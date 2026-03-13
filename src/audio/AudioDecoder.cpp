#include "AudioDecoder.h"
#include <spdlog/spdlog.h>
#include <QFileInfo>
#include <cstring>

namespace suno::audio {

AudioDecoder::AudioDecoder(QObject* parent)
: QObject(parent)
{
    m_floatBuffer.reserve(MAX_PCM_SAMPLES * 2);
    spdlog::debug("AudioDecoder created");
}

AudioDecoder::~AudioDecoder()
{
    unload();
    shutdownPulseAudio();
    spdlog::debug("AudioDecoder destroyed");
}

bool AudioDecoder::loadFile(const QString& filePath)
{
    unload();

    spdlog::info("Loading audio file: {}", filePath.toStdString());
    m_filePath = filePath;

    QByteArray pathBytes = filePath.toUtf8();

    if (avformat_open_input(&m_formatCtx, pathBytes.constData(), nullptr, nullptr) != 0) {
        spdlog::error("Failed to open audio file: {}", filePath.toStdString());
        emit errorOccurred("Failed to open audio file");
        return false;
    }

    if (avformat_find_stream_info(m_formatCtx, nullptr) < 0) {
        spdlog::error("Failed to find stream info");
        avformat_close_input(&m_formatCtx);
        m_formatCtx = nullptr;
        emit errorOccurred("Failed to find stream info");
        return false;
    }

    m_audioStreamIndex = -1;
    for (unsigned int i = 0; i < m_formatCtx->nb_streams; ++i) {
        if (m_formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            m_audioStreamIndex = static_cast<int>(i);
            break;
        }
    }

    if (m_audioStreamIndex < 0) {
        spdlog::error("No audio stream found");
        avformat_close_input(&m_formatCtx);
        m_formatCtx = nullptr;
        emit errorOccurred("No audio stream found");
        return false;
    }

    AVStream* audioStream = m_formatCtx->streams[m_audioStreamIndex];
    const AVCodec* codec = avcodec_find_decoder(audioStream->codecpar->codec_id);
    if (!codec) {
        spdlog::error("Failed to find decoder");
        avformat_close_input(&m_formatCtx);
        m_formatCtx = nullptr;
        emit errorOccurred("Failed to find decoder");
        return false;
    }

    m_codecCtx = avcodec_alloc_context3(codec);
    if (!m_codecCtx) {
        spdlog::error("Failed to allocate codec context");
        avformat_close_input(&m_formatCtx);
        m_formatCtx = nullptr;
        emit errorOccurred("Failed to allocate codec context");
        return false;
    }

    if (avcodec_parameters_to_context(m_codecCtx, audioStream->codecpar) < 0) {
        spdlog::error("Failed to copy codec parameters");
        avcodec_free_context(&m_codecCtx);
        avformat_close_input(&m_formatCtx);
        m_codecCtx = nullptr;
        m_formatCtx = nullptr;
        emit errorOccurred("Failed to copy codec parameters");
        return false;
    }

    if (avcodec_open2(m_codecCtx, codec, nullptr) < 0) {
        spdlog::error("Failed to open codec");
        avcodec_free_context(&m_codecCtx);
        avformat_close_input(&m_formatCtx);
        m_codecCtx = nullptr;
        m_formatCtx = nullptr;
        emit errorOccurred("Failed to open codec");
        return false;
    }

    m_audioFormat.sampleRate = m_codecCtx->sample_rate;
    m_audioFormat.channels = m_codecCtx->ch_layout.nb_channels;
    m_audioFormat.bitsPerSample = 32;
    m_audioFormat.isFloat = true;

    spdlog::info("Audio format: {} Hz, {} channels", m_audioFormat.sampleRate, m_audioFormat.channels);

    AVChannelLayout outLayout = AV_CHANNEL_LAYOUT_STEREO;
    AVSampleFormat outFormat = AV_SAMPLE_FMT_FLT;

    int ret = swr_alloc_set_opts2(&m_swrCtx,
                                   &outLayout, outFormat, m_audioFormat.sampleRate,
                                   &m_codecCtx->ch_layout, m_codecCtx->sample_fmt, m_codecCtx->sample_rate,
                                   0, nullptr);
    if (ret < 0 || !m_swrCtx) {
        spdlog::error("Failed to allocate resampler");
        unload();
        emit errorOccurred("Failed to allocate resampler");
        return false;
    }

    if (swr_init(m_swrCtx) < 0) {
        spdlog::error("Failed to initialize resampler");
        unload();
        emit errorOccurred("Failed to initialize resampler");
        return false;
    }

    m_durationMs = static_cast<qint64>(audioStream->duration * av_q2d(audioStream->time_base) * 1000);
    emit durationChanged(m_durationMs);

    m_isLoaded.store(true);
    spdlog::info("Audio file loaded: {} ms duration", m_durationMs);

    return true;
}

void AudioDecoder::unload()
{
    stop();

    m_isLoaded.store(false);

    if (m_swrCtx) {
        swr_free(&m_swrCtx);
        m_swrCtx = nullptr;
    }

    if (m_codecCtx) {
        avcodec_free_context(&m_codecCtx);
        m_codecCtx = nullptr;
    }

    if (m_formatCtx) {
        avformat_close_input(&m_formatCtx);
        m_formatCtx = nullptr;
    }

    m_audioStreamIndex = -1;
    m_durationMs = 0;
    m_positionMs.store(0);
    m_filePath.clear();
}

bool AudioDecoder::initializePulseAudio()
{
    spdlog::info("Initializing PulseAudio...");

    m_paMainloop = pa_threaded_mainloop_new();
    if (!m_paMainloop) {
        spdlog::error("Failed to create PulseAudio mainloop");
        emit errorOccurred("Failed to create PulseAudio mainloop");
        return false;
    }

    pa_mainloop_api* api = pa_threaded_mainloop_get_api(m_paMainloop);

    m_paContext = pa_context_new(api, "SunoVisualizer");
    if (!m_paContext) {
        spdlog::error("Failed to create PulseAudio context");
        pa_threaded_mainloop_free(m_paMainloop);
        m_paMainloop = nullptr;
        emit errorOccurred("Failed to create PulseAudio context");
        return false;
    }

    pa_context_connect(m_paContext, nullptr, PA_CONTEXT_NOFLAGS, nullptr);

    pa_threaded_mainloop_lock(m_paMainloop);
    if (pa_threaded_mainloop_start(m_paMainloop) < 0) {
        spdlog::error("Failed to start PulseAudio mainloop");
        pa_threaded_mainloop_unlock(m_paMainloop);
        pa_context_unref(m_paContext);
        pa_threaded_mainloop_free(m_paMainloop);
        m_paContext = nullptr;
        m_paMainloop = nullptr;
        emit errorOccurred("Failed to start PulseAudio mainloop");
        return false;
    }
    pa_threaded_mainloop_unlock(m_paMainloop);

    while (pa_context_get_state(m_paContext) != PA_CONTEXT_READY) {
        pa_context_state_t state = pa_context_get_state(m_paContext);
        if (state == PA_CONTEXT_FAILED || state == PA_CONTEXT_TERMINATED) {
            spdlog::error("PulseAudio context connection failed");
            shutdownPulseAudio();
            emit errorOccurred("PulseAudio context connection failed");
            return false;
        }
        pa_threaded_mainloop_lock(m_paMainloop);
        pa_threaded_mainloop_wait(m_paMainloop);
        pa_threaded_mainloop_unlock(m_paMainloop);
    }

    pa_sample_spec sampleSpec;
    sampleSpec.format = PA_SAMPLE_FLOAT32LE;
    sampleSpec.rate = static_cast<uint32_t>(m_audioFormat.sampleRate);
    sampleSpec.channels = static_cast<uint8_t>(m_audioFormat.channels);

    pa_channel_map channelMap;
    pa_channel_map_init_stereo(&channelMap);

    pa_threaded_mainloop_lock(m_paMainloop);
    m_paStream = pa_stream_new(m_paContext, "Audio Playback", &sampleSpec, &channelMap);
    if (!m_paStream) {
        pa_threaded_mainloop_unlock(m_paMainloop);
        spdlog::error("Failed to create PulseAudio stream");
        shutdownPulseAudio();
        emit errorOccurred("Failed to create PulseAudio stream");
        return false;
    }

    pa_stream_set_write_callback(m_paStream, paStreamWriteCallback, this);

    pa_buffer_attr bufferAttr;
    bufferAttr.maxlength = static_cast<uint32_t>(-1);
    bufferAttr.tlength = pa_usec_to_bytes(100000, &sampleSpec);
    bufferAttr.prebuf = static_cast<uint32_t>(-1);
    bufferAttr.minreq = static_cast<uint32_t>(-1);
    bufferAttr.fragsize = static_cast<uint32_t>(-1);

	pa_stream_flags_t streamFlags = static_cast<pa_stream_flags_t>(
		PA_STREAM_INTERPOLATE_TIMING | PA_STREAM_ADJUST_LATENCY | PA_STREAM_AUTO_TIMING_UPDATE);

	if (pa_stream_connect_playback(m_paStream, nullptr, &bufferAttr,
		streamFlags, nullptr, nullptr) < 0) {
        pa_threaded_mainloop_unlock(m_paMainloop);
        spdlog::error("Failed to connect PulseAudio stream for playback");
        shutdownPulseAudio();
        emit errorOccurred("Failed to connect PulseAudio stream for playback");
        return false;
    }

    while (pa_stream_get_state(m_paStream) != PA_STREAM_READY) {
        pa_stream_state_t state = pa_stream_get_state(m_paStream);
        if (state == PA_STREAM_FAILED || state == PA_STREAM_TERMINATED) {
            pa_threaded_mainloop_unlock(m_paMainloop);
            spdlog::error("PulseAudio stream setup failed");
            shutdownPulseAudio();
            emit errorOccurred("PulseAudio stream setup failed");
            return false;
        }
        pa_threaded_mainloop_wait(m_paMainloop);
    }

    pa_threaded_mainloop_unlock(m_paMainloop);

    spdlog::info("PulseAudio initialized successfully");
    return true;
}

void AudioDecoder::shutdownPulseAudio()
{
    if (m_paStream) {
        pa_threaded_mainloop_lock(m_paMainloop);
        pa_stream_disconnect(m_paStream);
        pa_stream_unref(m_paStream);
        pa_threaded_mainloop_unlock(m_paMainloop);
        m_paStream = nullptr;
    }

    if (m_paContext) {
        pa_threaded_mainloop_lock(m_paMainloop);
        pa_context_disconnect(m_paContext);
        pa_context_unref(m_paContext);
        pa_threaded_mainloop_unlock(m_paMainloop);
        m_paContext = nullptr;
    }

    if (m_paMainloop) {
        pa_threaded_mainloop_stop(m_paMainloop);
        pa_threaded_mainloop_free(m_paMainloop);
        m_paMainloop = nullptr;
    }

    spdlog::debug("PulseAudio shutdown complete");
}

void AudioDecoder::play()
{
    if (!m_isLoaded.load()) {
        spdlog::warn("No audio loaded");
        return;
    }

    if (!m_paMainloop && !initializePulseAudio()) {
        return;
    }

    m_stopRequested.store(false);
    m_isPaused.store(false);

    if (!m_isPlaying.load()) {
        m_isPlaying.store(true);
        decodeLoop();
        emit playbackStarted();
        spdlog::debug("Playback started");
    }
}

void AudioDecoder::pause()
{
    if (m_isPlaying.load() && !m_isPaused.load()) {
        m_isPaused.store(true);
        emit playbackPaused();
        spdlog::debug("Playback paused");
    }
}

void AudioDecoder::stop()
{
    if (m_isPlaying.load()) {
        m_stopRequested.store(true);
        m_isPlaying.store(false);
        m_isPaused.store(false);

        if (m_formatCtx) {
            av_seek_frame(m_formatCtx, m_audioStreamIndex, 0, AVSEEK_FLAG_BACKWARD);
        }
        m_positionMs.store(0);
        emit positionChanged(0);
        emit playbackStopped();
        spdlog::debug("Playback stopped");
    }
}

void AudioDecoder::seek(qint64 positionMs)
{
    if (!m_isLoaded.load() || positionMs < 0 || positionMs > m_durationMs) {
        return;
    }

    m_seekTarget.store(positionMs);
    m_seekRequested.store(true);
    spdlog::debug("Seek requested to {} ms", positionMs);
}

void AudioDecoder::setVolume(float volume)
{
    float clamped = qBound(0.0f, volume, 1.0f);
    if (!qFuzzyCompare(m_volume.load(), clamped)) {
        m_volume.store(clamped);
        emit volumeChanged(clamped);
    }
}

void AudioDecoder::decodeLoop()
{
    while (m_isPlaying.load() && !m_stopRequested.load()) {
        if (m_seekRequested.load()) {
            qint64 targetMs = m_seekTarget.load();
            int64_t targetPts = static_cast<int64_t>(targetMs / 1000.0 * AV_TIME_BASE);

            if (av_seek_frame(m_formatCtx, -1, targetPts, AVSEEK_FLAG_BACKWARD) >= 0) {
                m_positionMs.store(targetMs);
                emit positionChanged(targetMs);
                avcodec_flush_buffers(m_codecCtx);
            }

            m_seekRequested.store(false);
        }

        if (m_isPaused.load()) {
            QMutexLocker locker(&m_mutex);
            m_condition.wait(&m_mutex, 10);
            continue;
        }

        if (!decodeFrame()) {
            break;
        }
    }
}

bool AudioDecoder::decodeFrame()
{
    AVPacket* packet = av_packet_alloc();
    if (!packet) {
        return false;
    }

    int ret = av_read_frame(m_formatCtx, packet);
    if (ret < 0) {
        av_packet_free(&packet);
        if (ret == AVERROR_EOF) {
            spdlog::debug("End of audio stream");
            emit eofReached();
        }
        return false;
    }

    if (packet->stream_index != m_audioStreamIndex) {
        av_packet_free(&packet);
        return true;
    }

    ret = avcodec_send_packet(m_codecCtx, packet);
    if (ret < 0) {
        av_packet_free(&packet);
        if (ret != AVERROR(EAGAIN)) {
            return false;
        }
        return true;
    }

    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        av_packet_free(&packet);
        return false;
    }

    ret = avcodec_receive_frame(m_codecCtx, frame);
    if (ret < 0) {
        av_frame_free(&frame);
        av_packet_free(&packet);
        if (ret == AVERROR(EAGAIN)) {
            return true;
        }
        return false;
    }

    uint8_t* outData = nullptr;
    int outSamples = swr_convert(m_swrCtx, &outData, frame->nb_samples,
                                  const_cast<const uint8_t**>(frame->data), frame->nb_samples);

    if (outSamples > 0 && outData) {
        size_t dataSize = static_cast<size_t>(outSamples) * sizeof(float) * m_audioFormat.channels;
        m_resampleBuffer.resize(dataSize);
        memcpy(m_resampleBuffer.data(), outData, dataSize);

        float vol = m_volume.load();
        if (vol < 0.99f) {
            float* samples = reinterpret_cast<float*>(m_resampleBuffer.data());
            size_t sampleCount = m_resampleBuffer.size() / sizeof(float);
            for (size_t i = 0; i < sampleCount; ++i) {
                samples[i] *= vol;
            }
        }

        writeToPulseAudio(m_resampleBuffer.data(), m_resampleBuffer.size());

        size_t floatSamples = m_resampleBuffer.size() / sizeof(float);
        m_floatBuffer.assign(reinterpret_cast<float*>(m_resampleBuffer.data()),
                             reinterpret_cast<float*>(m_resampleBuffer.data()) + floatSamples);

        if (m_audioDataCallback) {
            m_audioDataCallback(m_floatBuffer.data(), floatSamples / m_audioFormat.channels, m_audioFormat.channels);
        }

        emit audioDataReady(m_floatBuffer.data(), floatSamples / m_audioFormat.channels, m_audioFormat.channels);

        if (m_audioStreamIndex >= 0 && m_formatCtx->streams[m_audioStreamIndex]) {
            int64_t pts = frame->pts;
            AVRational timeBase = m_formatCtx->streams[m_audioStreamIndex]->time_base;
            qint64 currentPos = static_cast<qint64>(pts * av_q2d(timeBase) * 1000);
            m_positionMs.store(currentPos);
            emit positionChanged(currentPos);
        }
    }

    av_frame_free(&frame);
    av_packet_free(&packet);
    return true;
}

void AudioDecoder::writeToPulseAudio(const uint8_t* data, size_t size)
{
    if (!m_paStream || !m_paMainloop) {
        return;
    }

    pa_threaded_mainloop_lock(m_paMainloop);

    size_t writable = pa_stream_writable_size(m_paStream);
    while (writable < size) {
        pa_threaded_mainloop_wait(m_paMainloop);
        writable = pa_stream_writable_size(m_paStream);
        if (!m_isPlaying.load()) {
            pa_threaded_mainloop_unlock(m_paMainloop);
            return;
        }
    }

    void* buffer = nullptr;
    size_t nbytes = size;
    if (pa_stream_begin_write(m_paStream, &buffer, &nbytes) < 0) {
        pa_threaded_mainloop_unlock(m_paMainloop);
        return;
    }

    memcpy(buffer, data, std::min(nbytes, size));
    pa_stream_write(m_paStream, buffer, nbytes, nullptr, 0, PA_SEEK_RELATIVE);

    pa_threaded_mainloop_unlock(m_paMainloop);
}

void AudioDecoder::paStreamWriteCallback(pa_stream* stream, size_t nbytes, void* userdata)
{
    Q_UNUSED(stream)
    Q_UNUSED(nbytes)
    Q_UNUSED(userdata)
}

void AudioDecoder::paStreamDrainCallback(pa_stream* stream, int success, void* userdata)
{
    Q_UNUSED(stream)
    Q_UNUSED(success)
    Q_UNUSED(userdata)
}

} // namespace suno::audio

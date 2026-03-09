#include "VideoCapture.h"
#include <spdlog/spdlog.h>
#include <QFileInfo>

namespace suno::video {

VideoCapture::VideoCapture(QObject* parent)
    : QObject(parent)
{
    // FFmpeg is like the Swiss Army knife of multimedia
    // Linus would approve of its everything-is-a-file philosophy
    spdlog::debug("VideoCapture instance created");
}

VideoCapture::~VideoCapture()
{
    shutdown();
}

bool VideoCapture::initialize(const QString& outputPath, int width, int height, int fps, int bitrate)
{
    if (m_initialized) {
        spdlog::warn("VideoCapture already initialized");
        return true;
    }
    
    spdlog::info("Initializing video capture: {}x{} @ {} fps, {} bps", 
                 width, height, fps, bitrate);
    
    m_outputPath = outputPath;
    m_width = width;
    m_height = height;
    m_fps = fps;
    m_bitrate = bitrate;
    
    emit outputPathChanged();
    
    m_initialized = true;
    spdlog::info("Video capture initialized");
    
    return true;
}

void VideoCapture::shutdown()
{
    if (!m_initialized) {
        return;
    }
    
    if (m_isRecording) {
        stopRecording();
    }
    
    m_initialized = false;
    spdlog::info("Video capture shutdown");
}

bool VideoCapture::startRecording()
{
    if (!m_initialized) {
        spdlog::error("VideoCapture not initialized");
        return false;
    }
    
    if (m_isRecording) {
        spdlog::warn("Already recording");
        return false;
    }
    
    spdlog::info("Starting video recording to: {}", m_outputPath.toStdString());
    
    if (!openOutputFile()) {
        return false;
    }
    
    m_isRecording = true;
    m_frameCount = 0;
    m_pts = 0;
    
    emit recordingStarted();
    emit recordingStateChanged();
    emit frameCountChanged(0);
    
    return true;
}

void VideoCapture::stopRecording()
{
    if (!m_isRecording) {
        return;
    }
    
    spdlog::info("Stopping video recording. Total frames: {}", m_frameCount);
    
    closeOutputFile();
    
    m_isRecording = false;
    
    emit recordingStopped();
    emit recordingStateChanged();
}

bool VideoCapture::openOutputFile()
{
    int ret;
    
    // Allocate output format context
    avformat_alloc_output_context2(&m_formatContext, nullptr, nullptr, m_outputPath.toUtf8().constData());
    if (!m_formatContext) {
        spdlog::error("Failed to allocate output format context");
        emit errorOccurred("Failed to create output format");
        return false;
    }
    
    // Find H.264 encoder
    const AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec) {
        spdlog::error("H.264 codec not found");
        emit errorOccurred("H.264 encoder not available");
        avformat_free_context(m_formatContext);
        m_formatContext = nullptr;
        return false;
    }
    
    // Create video stream
    m_videoStream = avformat_new_stream(m_formatContext, nullptr);
    if (!m_videoStream) {
        spdlog::error("Failed to create video stream");
        emit errorOccurred("Failed to create video stream");
        avformat_free_context(m_formatContext);
        m_formatContext = nullptr;
        return false;
    }
    
    // Allocate codec context
    m_codecContext = avcodec_alloc_context3(codec);
    if (!m_codecContext) {
        spdlog::error("Failed to allocate codec context");
        emit errorOccurred("Failed to allocate encoder context");
        avformat_free_context(m_formatContext);
        m_formatContext = nullptr;
        return false;
    }
    
    // Set codec parameters
    m_codecContext->codec_id = AV_CODEC_ID_H264;
    m_codecContext->codec_type = AVMEDIA_TYPE_VIDEO;
    m_codecContext->width = m_width;
    m_codecContext->height = m_height;
    m_codecContext->time_base = AVRational{1, m_fps};
    m_codecContext->framerate = AVRational{m_fps, 1};
    m_codecContext->gop_size = m_fps * 2; // Keyframe every 2 seconds
    m_codecContext->max_b_frames = 2;
    m_codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    m_codecContext->bit_rate = m_bitrate;
    
    // Set H.264 specific options for quality
    av_opt_set(m_codecContext->priv_data, "preset", "medium", 0);
    av_opt_set(m_codecContext->priv_data, "crf", "23", 0);
    
    // Global headers if needed
    if (m_formatContext->oformat->flags & AVFMT_GLOBALHEADER) {
        m_codecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    
    // Open codec
    ret = avcodec_open2(m_codecContext, codec, nullptr);
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        spdlog::error("Failed to open codec: {}", errbuf);
        emit errorOccurred("Failed to open video encoder");
        avcodec_free_context(&m_codecContext);
        avformat_free_context(m_formatContext);
        m_formatContext = nullptr;
        return false;
    }
    
    // Copy codec parameters to stream
    ret = avcodec_parameters_from_context(m_videoStream->codecpar, m_codecContext);
    if (ret < 0) {
        spdlog::error("Failed to copy codec parameters");
        emit errorOccurred("Failed to configure stream");
        avcodec_free_context(&m_codecContext);
        avformat_free_context(m_formatContext);
        m_formatContext = nullptr;
        return false;
    }
    
    m_videoStream->time_base = m_codecContext->time_base;
    
    // Allocate frame
    m_frame = av_frame_alloc();
    if (!m_frame) {
        spdlog::error("Failed to allocate frame");
        emit errorOccurred("Failed to allocate frame buffer");
        avcodec_free_context(&m_codecContext);
        avformat_free_context(m_formatContext);
        m_formatContext = nullptr;
        return false;
    }
    
    m_frame->format = m_codecContext->pix_fmt;
    m_frame->width = m_width;
    m_frame->height = m_height;
    
    ret = av_frame_get_buffer(m_frame, 0);
    if (ret < 0) {
        spdlog::error("Failed to allocate frame buffer");
        emit errorOccurred("Failed to allocate frame data");
        av_frame_free(&m_frame);
        avcodec_free_context(&m_codecContext);
        avformat_free_context(m_formatContext);
        m_formatContext = nullptr;
        return false;
    }
    
    // Allocate packet
    m_packet = av_packet_alloc();
    if (!m_packet) {
        spdlog::error("Failed to allocate packet");
        emit errorOccurred("Failed to allocate packet buffer");
        av_frame_free(&m_frame);
        avcodec_free_context(&m_codecContext);
        avformat_free_context(m_formatContext);
        m_formatContext = nullptr;
        return false;
    }
    
    // Open output file
    if (!(m_formatContext->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&m_formatContext->pb, m_outputPath.toUtf8().constData(), AVIO_FLAG_WRITE);
        if (ret < 0) {
            char errbuf[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(ret, errbuf, sizeof(errbuf));
            spdlog::error("Failed to open output file: {}", errbuf);
            emit errorOccurred("Failed to open output file");
            av_packet_free(&m_packet);
            av_frame_free(&m_frame);
            avcodec_free_context(&m_codecContext);
            avformat_free_context(m_formatContext);
            m_formatContext = nullptr;
            return false;
        }
    }
    
    // Write header
    ret = avformat_write_header(m_formatContext, nullptr);
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        spdlog::error("Failed to write header: {}", errbuf);
        emit errorOccurred("Failed to write file header");
        if (!(m_formatContext->oformat->flags & AVFMT_NOFILE)) {
            avio_closep(&m_formatContext->pb);
        }
        av_packet_free(&m_packet);
        av_frame_free(&m_frame);
        avcodec_free_context(&m_codecContext);
        avformat_free_context(m_formatContext);
        m_formatContext = nullptr;
        return false;
    }
    
    // Initialize scaler for RGB to YUV conversion
    m_swsContext = sws_getContext(
        m_width, m_height, AV_PIX_FMT_RGBA,
        m_width, m_height, AV_PIX_FMT_YUV420P,
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );
    
    if (!m_swsContext) {
        spdlog::error("Failed to initialize swscale context");
        emit errorOccurred("Failed to initialize color conversion");
        closeOutputFile();
        return false;
    }
    
    spdlog::info("Output file opened successfully");
    return true;
}

void VideoCapture::closeOutputFile()
{
    if (!m_formatContext) {
        return;
    }
    
    // Flush encoder
    if (m_codecContext) {
        avcodec_send_frame(m_codecContext, nullptr);
        while (avcodec_receive_packet(m_codecContext, m_packet) == 0) {
            av_packet_rescale_ts(m_packet, m_codecContext->time_base, m_videoStream->time_base);
            m_packet->stream_index = m_videoStream->index;
            av_interleaved_write_frame(m_formatContext, m_packet);
            av_packet_unref(m_packet);
        }
    }
    
    // Write trailer
    av_write_trailer(m_formatContext);
    
    // Cleanup
    if (m_swsContext) {
        sws_freeContext(m_swsContext);
        m_swsContext = nullptr;
    }
    
    if (m_packet) {
        av_packet_free(&m_packet);
    }
    
    if (m_frame) {
        av_frame_free(&m_frame);
    }
    
    if (m_codecContext) {
        avcodec_free_context(&m_codecContext);
    }
    
    if (!(m_formatContext->oformat->flags & AVFMT_NOFILE)) {
        avio_closep(&m_formatContext->pb);
    }
    
    avformat_free_context(m_formatContext);
    m_formatContext = nullptr;
    
    spdlog::info("Output file closed");
}

bool VideoCapture::captureFrame(const QImage& frame)
{
    if (!m_isRecording) {
        spdlog::warn("Not recording, frame ignored");
        return false;
    }
    
    if (frame.width() != m_width || frame.height() != m_height) {
        spdlog::error("Frame size mismatch: {}x{} expected, got {}x{}", 
                     m_width, m_height, frame.width(), frame.height());
        return false;
    }
    
    // Convert QImage to RGBA format
    QImage rgbaImage = frame.convertToFormat(QImage::Format_RGBA8888);
    
    // Prepare source data for swscale
    const uint8_t* srcData[1] = { rgbaImage.constBits() };
    int srcLinesize[1] = { static_cast<int>(rgbaImage.bytesPerLine()) };
    
    // Convert RGBA to YUV420P
    av_frame_make_writable(m_frame);
    sws_scale(m_swsContext, srcData, srcLinesize, 0, m_height,
              m_frame->data, m_frame->linesize);
    
    m_frame->pts = m_pts++;
    
    if (!writeFrame(m_frame)) {
        return false;
    }
    
    m_frameCount++;
    emit frameCountChanged(m_frameCount);
    
    if (m_frameCount % m_fps == 0) {
        spdlog::debug("Captured {} frames ({} seconds)", 
                     m_frameCount, m_frameCount / m_fps);
    }
    
    return true;
}

bool VideoCapture::writeFrame(AVFrame* frame)
{
    int ret;
    
    // Send frame to encoder
    ret = avcodec_send_frame(m_codecContext, frame);
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        spdlog::error("Failed to send frame to encoder: {}", errbuf);
        return false;
    }
    
    // Receive encoded packets
    while (ret >= 0) {
        ret = avcodec_receive_packet(m_codecContext, m_packet);
        
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        } else if (ret < 0) {
            char errbuf[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(ret, errbuf, sizeof(errbuf));
            spdlog::error("Failed to receive packet from encoder: {}", errbuf);
            return false;
        }
        
        // Rescale packet timestamps
        av_packet_rescale_ts(m_packet, m_codecContext->time_base, m_videoStream->time_base);
        m_packet->stream_index = m_videoStream->index;
        
        // Write packet
        ret = av_interleaved_write_frame(m_formatContext, m_packet);
        if (ret < 0) {
            char errbuf[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(ret, errbuf, sizeof(errbuf));
            spdlog::error("Failed to write packet: {}", errbuf);
            av_packet_unref(m_packet);
            return false;
        }
        
        av_packet_unref(m_packet);
    }
    
    return true;
}

} // namespace suno::video
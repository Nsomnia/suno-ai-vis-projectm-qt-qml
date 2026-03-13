#pragma once

#include <QObject>
#include <QString>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <memory>
#include <vector>
#include <atomic>
#include <functional>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <pulse/pulseaudio.h>
}

namespace suno::audio {

struct AudioFormat
{
    int sampleRate = 48000;
    int channels = 2;
    int bitsPerSample = 32;
    bool isFloat = true;
};

class AudioDecoder : public QObject
{
    Q_OBJECT

public:
    explicit AudioDecoder(QObject* parent = nullptr);
    ~AudioDecoder() override;

    bool loadFile(const QString& filePath);
    void unload();

    bool initializePulseAudio();
    void shutdownPulseAudio();

    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void seek(qint64 positionMs);

    bool isPlaying() const { return m_isPlaying.load(); }
    bool isPaused() const { return m_isPaused.load(); }
    bool isLoaded() const { return m_isLoaded.load(); }

    qint64 duration() const { return m_durationMs; }
    qint64 position() const { return m_positionMs.load(); }

    float volume() const { return m_volume.load(); }
    Q_INVOKABLE void setVolume(float volume);

    const AudioFormat& format() const { return m_audioFormat; }

    using AudioDataCallback = std::function<void(const float* data, size_t samples, int channels)>;
    void setAudioDataCallback(AudioDataCallback callback) { m_audioDataCallback = std::move(callback); }

signals:
    void playbackStarted();
    void playbackPaused();
    void playbackStopped();
    void positionChanged(qint64 positionMs);
    void durationChanged(qint64 durationMs);
    void volumeChanged(float volume);
    void audioDataReady(const float* data, size_t samples, int channels);
    void errorOccurred(const QString& error);
    void eofReached();

private:
    void decodeLoop();
    bool decodeFrame();
    void writeToPulseAudio(const uint8_t* data, size_t size);
    static void paStreamWriteCallback(pa_stream* stream, size_t nbytes, void* userdata);
    static void paStreamDrainCallback(pa_stream* stream, int success, void* userdata);

    QString m_filePath;
    std::atomic<bool> m_isPlaying{false};
    std::atomic<bool> m_isPaused{false};
    std::atomic<bool> m_isLoaded{false};
    std::atomic<bool> m_stopRequested{false};
    std::atomic<bool> m_seekRequested{false};
    std::atomic<qint64> m_seekTarget{0};
    std::atomic<float> m_volume{1.0f};
    std::atomic<qint64> m_positionMs{0};

    qint64 m_durationMs = 0;

    AVFormatContext* m_formatCtx = nullptr;
    AVCodecContext* m_codecCtx = nullptr;
    SwrContext* m_swrCtx = nullptr;
    int m_audioStreamIndex = -1;

    AudioFormat m_audioFormat;
    std::vector<uint8_t> m_resampleBuffer;
    std::vector<float> m_floatBuffer;

    pa_threaded_mainloop* m_paMainloop = nullptr;
    pa_context* m_paContext = nullptr;
    pa_stream* m_paStream = nullptr;

    QMutex m_mutex;
    QWaitCondition m_condition;

    AudioDataCallback m_audioDataCallback;

    static constexpr size_t MAX_PCM_SAMPLES = 2048;
};

} // namespace suno::audio

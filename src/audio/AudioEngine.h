#pragma once

#include <QObject>
#include <QAudioFormat>
#include <QAudioSink>
#include <QIODevice>
#include <memory>
#include <vector>

namespace suno::audio {

class AudioEngine : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY playbackStateChanged)
    Q_PROPERTY(qint64 position READ position NOTIFY positionChanged)
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(float volume READ volume WRITE setVolume NOTIFY volumeChanged)

public:
    explicit AudioEngine(QObject* parent = nullptr);
    ~AudioEngine() override;
    
    bool initialize();
    void shutdown();
    
    bool loadFile(const QString& filePath);
    void unload();
    
    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void seek(qint64 positionMs);
    
    bool isPlaying() const { return m_isPlaying; }
    qint64 position() const { return m_position; }
    qint64 duration() const { return m_duration; }
    float volume() const { return m_volume; }
    Q_INVOKABLE void setVolume(float volume);
    
    // Audio analysis data for visualization
    const std::vector<float>& getFrequencyData() const { return m_frequencyData; }
    const std::vector<float>& getWaveformData() const { return m_waveformData; }
    float getBassLevel() const { return m_bassLevel; }
    float getMidLevel() const { return m_midLevel; }
    float getTrebleLevel() const { return m_trebleLevel; }
    
signals:
    void playbackStateChanged();
    void positionChanged(qint64 position);
    void durationChanged(qint64 duration);
    void volumeChanged(float volume);
    void audioDataReady();
    void fileLoaded(const QString& filePath);
    void errorOccurred(const QString& error);

private slots:
    void updateAudioData();

private:
    void setupAudioFormat();
    void analyzeAudioFrame(const QByteArray& data);
    void calculateFrequencyBands();
    
    bool m_initialized = false;
    bool m_isPlaying = false;
    qint64 m_position = 0;
    qint64 m_duration = 0;
    float m_volume = 1.0f;
    
    QString m_currentFile;
    QAudioFormat m_audioFormat;
    std::unique_ptr<QAudioSink> m_audioSink;
    
    // Audio analysis data
    std::vector<float> m_frequencyData;
    std::vector<float> m_waveformData;
    float m_bassLevel = 0.0f;
    float m_midLevel = 0.0f;
    float m_trebleLevel = 0.0f;
    
    static constexpr int FREQUENCY_BINS = 512;
    static constexpr int WAVEFORM_SAMPLES = 2048;
};

} // namespace suno::audio
#pragma once

#include <QObject>
#include <QString>
#include <QProcess>
#include <memory>
#include <vector>

namespace suno::audio {

enum class StemType {
    Vocals,
    Bass,
    Drums,
    Other,
    Full
};

struct StemData {
    StemType type;
    QString filePath;
    std::vector<float> frequencyData;
    float level = 0.0f;
    bool ready = false;
};

class StemProcessor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isProcessing READ isProcessing NOTIFY processingStateChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)

public:
    explicit StemProcessor(QObject* parent = nullptr);
    ~StemProcessor() override = default;
    
    Q_INVOKABLE bool processStemsFromFile(const QString& audioFilePath);
    Q_INVOKABLE void cancelProcessing();
    
    bool isProcessing() const { return m_isProcessing; }
    int progress() const { return m_progress; }
    
    const StemData* getStem(StemType type) const;
    QList<StemType> availableStems() const;
    
    // For feeding stems to visualizer
    std::vector<float> getVocalFrequencies() const;
    std::vector<float> getBassFrequencies() const;
    std::vector<float> getDrumFrequencies() const;

signals:
    void processingStarted();
    void processingFinished();
    void processingStateChanged();
    void progressChanged(int progress);
    void stemReady(StemType type);
    void errorOccurred(const QString& error);

private slots:
    void onProcessOutput();
    void onProcessError();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    bool checkDemucsAvailable();
    void parseProcessOutput(const QString& output);
    void loadProcessedStems(const QString& outputDir);
    
    bool m_isProcessing = false;
    int m_progress = 0;
    QString m_currentInputFile;
    QString m_outputDirectory;
    
    std::unique_ptr<QProcess> m_demucsProcess;
    
    // Processed stems
    StemData m_vocals;
    StemData m_bass;
    StemData m_drums;
    StemData m_other;
    
    bool m_demucsAvailable = false;
};

} // namespace suno::audio
#include "StemProcessor.h"
#include <spdlog/spdlog.h>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>

namespace suno::audio {

StemProcessor::StemProcessor(QObject* parent)
    : QObject(parent)
{
    m_vocals.type = StemType::Vocals;
    m_bass.type = StemType::Bass;
    m_drums.type = StemType::Drums;
    m_other.type = StemType::Other;
    
    m_demucsAvailable = checkDemucsAvailable();
    
    if (!m_demucsAvailable) {
        spdlog::warn("Demucs not found - stem separation will be unavailable");
        spdlog::info("Install with: pip install demucs");
    } else {
        spdlog::info("Demucs stem separation available");
    }
}

bool StemProcessor::checkDemucsAvailable()
{
    QProcess process;
    process.start("demucs", QStringList{"--help"});
    process.waitForFinished(3000);
    
    return process.exitCode() == 0;
}

bool StemProcessor::processStemsFromFile(const QString& audioFilePath)
{
    if (m_isProcessing) {
        spdlog::warn("Stem processing already in progress");
        return false;
    }
    
    if (!m_demucsAvailable) {
        QString error = "Demucs not available - cannot process stems";
        std::string errStr = error.toStdString();
        spdlog::error("{}", errStr);
        emit errorOccurred(error);
        return false;
    }

    QFileInfo fileInfo(audioFilePath);
    if (!fileInfo.exists()) {
        QString error = QString("Audio file not found: %1").arg(audioFilePath);
        std::string errStr2 = error.toStdString();
        spdlog::error("{}", errStr2);
        emit errorOccurred(error);
        return false;
    }

    std::string filePathStr = audioFilePath.toStdString();
    spdlog::info("Starting stem separation for: {}", filePathStr);
    
    m_currentInputFile = audioFilePath;
    m_outputDirectory = QStandardPaths::writableLocation(QStandardPaths::TempLocation) 
                       + "/suno-visualizer/stems/" + fileInfo.baseName();
    
    QDir().mkpath(m_outputDirectory);
    
    m_demucsProcess = std::make_unique<QProcess>();
    
    connect(m_demucsProcess.get(), &QProcess::readyReadStandardOutput,
            this, &StemProcessor::onProcessOutput);
    connect(m_demucsProcess.get(), &QProcess::readyReadStandardError,
            this, &StemProcessor::onProcessError);
    connect(m_demucsProcess.get(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &StemProcessor::onProcessFinished);
    
    QStringList arguments;
    arguments << "-n" << "htdemucs";  // Use the htdemucs model
    arguments << "--two-stems=vocals"; // Separate vocals first (faster for visualization)
    arguments << "-o" << m_outputDirectory;
    arguments << audioFilePath;
    
    m_demucsProcess->start("demucs", arguments);
    
    if (!m_demucsProcess->waitForStarted(5000)) {
        QString error = "Failed to start Demucs process";
        std::string errStr = error.toStdString();
        spdlog::error("{}", errStr);
        emit errorOccurred(error);
        m_demucsProcess.reset();
        return false;
    }
    
    m_isProcessing = true;
    m_progress = 0;
    
    emit processingStarted();
    emit processingStateChanged();
    
    return true;
}

void StemProcessor::cancelProcessing()
{
    if (!m_isProcessing || !m_demucsProcess) {
        return;
    }
    
    spdlog::info("Cancelling stem processing");
    
    m_demucsProcess->kill();
    m_demucsProcess->waitForFinished(3000);
    m_demucsProcess.reset();
    
    m_isProcessing = false;
    m_progress = 0;
    
    emit processingStateChanged();
    emit progressChanged(0);
}

void StemProcessor::onProcessOutput()
{
    if (!m_demucsProcess) {
        return;
    }
    
    QString output = m_demucsProcess->readAllStandardOutput();
    parseProcessOutput(output);
}

void StemProcessor::onProcessError()
{
    if (!m_demucsProcess) {
        return;
    }

    QString error = m_demucsProcess->readAllStandardError();
    std::string errStr = error.toStdString();
    spdlog::debug("Demucs stderr: {}", errStr);
}

void StemProcessor::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    spdlog::info("Demucs process finished with exit code: {}", exitCode);
    
    m_isProcessing = false;
    emit processingStateChanged();
    
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        loadProcessedStems(m_outputDirectory);
        m_progress = 100;
        emit progressChanged(100);
        emit processingFinished();
    } else {
        QString error = QString("Stem processing failed with exit code: %1").arg(exitCode);
        std::string errStr = error.toStdString();
        spdlog::error("{}", errStr);
        emit errorOccurred(error);
    }
    
    m_demucsProcess.reset();
}

void StemProcessor::parseProcessOutput(const QString& output)
{
    // Parse Demucs progress output
    // Demucs typically outputs progress like: "30% 0:45"
    
    QRegularExpression progressRegex(R"((\d+)%)");
    auto match = progressRegex.match(output);
    
    if (match.hasMatch()) {
        int newProgress = match.captured(1).toInt();
        if (newProgress != m_progress) {
            m_progress = newProgress;
            emit progressChanged(m_progress);
            spdlog::debug("Stem processing progress: {}%", m_progress);
        }
    }
}

void StemProcessor::loadProcessedStems(const QString& outputDir)
{
    std::string outputDirStr = outputDir.toStdString();
    spdlog::info("Loading processed stems from: {}", outputDirStr);

    QFileInfo inputFile(m_currentInputFile);
    QString baseName = inputFile.baseName();

    QString stemsPath = outputDir + "/htdemucs/" + baseName;

    QStringList stemFiles = {"vocals.wav", "bass.wav", "drums.wav", "other.wav"};
    QList<StemData*> stems = {&m_vocals, &m_bass, &m_drums, &m_other};
    QList<StemType> stemTypes = {StemType::Vocals, StemType::Bass, StemType::Drums, StemType::Other};

    for (int i = 0; i < stemFiles.size(); ++i) {
        QString stemPath = stemsPath + "/" + stemFiles[i];
        QFileInfo stemInfo(stemPath);

        if (stemInfo.exists()) {
            stems[i]->filePath = stemPath;
            stems[i]->ready = true;
            std::string stemPathStr = stemPath.toStdString();
            spdlog::info("Loaded stem: {}", stemPathStr);
            emit stemReady(stemTypes[i]);
        } else {
            std::string stemPathStr2 = stemPath.toStdString();
            spdlog::warn("Stem file not found: {}", stemPathStr2);
        }
    }
}

const StemData* StemProcessor::getStem(StemType type) const
{
    switch (type) {
        case StemType::Vocals: return &m_vocals;
        case StemType::Bass: return &m_bass;
        case StemType::Drums: return &m_drums;
        case StemType::Other: return &m_other;
        default: return nullptr;
    }
}

QList<StemType> StemProcessor::availableStems() const
{
    QList<StemType> available;
    
    if (m_vocals.ready) available.append(StemType::Vocals);
    if (m_bass.ready) available.append(StemType::Bass);
    if (m_drums.ready) available.append(StemType::Drums);
    if (m_other.ready) available.append(StemType::Other);
    
    return available;
}

std::vector<float> StemProcessor::getVocalFrequencies() const
{
    if (m_vocals.ready) {
        return m_vocals.frequencyData;
    }
    return std::vector<float>();
}

std::vector<float> StemProcessor::getBassFrequencies() const
{
    if (m_bass.ready) {
        return m_bass.frequencyData;
    }
    return std::vector<float>();
}

std::vector<float> StemProcessor::getDrumFrequencies() const
{
    if (m_drums.ready) {
        return m_drums.frequencyData;
    }
    return std::vector<float>();
}

} // namespace suno::audio
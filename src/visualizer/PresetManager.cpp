#include "PresetManager.h"
#include <spdlog/spdlog.h>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonArray>

namespace suno::visualizer {

PresetManager::PresetManager(QObject* parent)
    : QObject(parent)
{
}

bool PresetManager::initialize(const QString& presetDirectory)
{
    m_presetDirectory = presetDirectory;
    
    QDir dir(m_presetDirectory);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            spdlog::error("Failed to create preset directory: {}", 
                         m_presetDirectory.toStdString());
            return false;
        }
    }
    
    loadPresetMetadata();
    scanPresets();
    
    spdlog::info("PresetManager initialized with {} presets", m_presets.size());
    
    return true;
}

void PresetManager::scanPresets()
{
    m_presets.clear();
    
    QDir dir(m_presetDirectory);
    QStringList filters;
    filters << "*.milk" << "*.prjm";
    
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
    
    for (const QFileInfo& fileInfo : files) {
        PresetInfo info;
        info.name = fileInfo.baseName();
        info.filePath = fileInfo.absoluteFilePath();
        info.created = fileInfo.birthTime();
        info.modified = fileInfo.lastModified();
        
        // Load additional metadata if available
        if (m_metadata.contains(info.name)) {
            QJsonObject presetMeta = m_metadata[info.name].toObject();
            info.author = presetMeta["author"].toString();
            info.description = presetMeta["description"].toString();
            info.rating = presetMeta["rating"].toInt();
            info.isFavorite = presetMeta["favorite"].toBool();
            info.gitCommitHash = presetMeta["commit"].toString();
        }
        
        m_presets.append(info);
    }
    
    emit presetsChanged();
}

void PresetManager::loadPresetMetadata()
{
    QString metaPath = metadataFilePath();
    QFile file(metaPath);
    
    if (!file.exists()) {
        spdlog::debug("No preset metadata file found");
        return;
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        spdlog::warn("Failed to open preset metadata file");
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    m_metadata = doc.object();
    
    spdlog::debug("Loaded preset metadata");
}

void PresetManager::savePresetMetadata()
{
    QString metaPath = metadataFilePath();
    QFile file(metaPath);
    
    if (!file.open(QIODevice::WriteOnly)) {
        spdlog::error("Failed to save preset metadata");
        return;
    }
    
    QJsonDocument doc(m_metadata);
    file.write(doc.toJson(QJsonDocument::Indented));
    
    spdlog::debug("Saved preset metadata");
}

QString PresetManager::metadataFilePath() const
{
    return m_presetDirectory + "/presets.json";
}

PresetInfo PresetManager::presetAt(int index) const
{
    if (index >= 0 && index < m_presets.size()) {
        return m_presets[index];
    }
    return PresetInfo();
}

PresetInfo PresetManager::presetByName(const QString& name) const
{
    for (const PresetInfo& info : m_presets) {
        if (info.name == name) {
            return info;
        }
    }
    return PresetInfo();
}

bool PresetManager::importPreset(const QString& filePath)
{
    QFileInfo sourceFile(filePath);
    
    if (!sourceFile.exists()) {
        emit errorOccurred("Source file not found");
        return false;
    }
    
    QString destPath = m_presetDirectory + "/" + sourceFile.fileName();
    
    if (QFile::exists(destPath)) {
        spdlog::warn("Preset already exists: {}", sourceFile.fileName().toStdString());
        emit errorOccurred("Preset with this name already exists");
        return false;
    }
    
    if (!QFile::copy(filePath, destPath)) {
        emit errorOccurred("Failed to copy preset file");
        return false;
    }
    
    spdlog::info("Imported preset: {}", sourceFile.fileName().toStdString());
    
    scanPresets();
    emit presetImported(sourceFile.baseName());
    
    return true;
}

bool PresetManager::exportPreset(const QString& name, const QString& destinationPath)
{
    PresetInfo info = presetByName(name);
    
    if (info.filePath.isEmpty()) {
        emit errorOccurred("Preset not found");
        return false;
    }
    
    if (!QFile::copy(info.filePath, destinationPath)) {
        emit errorOccurred("Failed to export preset");
        return false;
    }
    
    spdlog::info("Exported preset '{}' to {}", name.toStdString(), 
                 destinationPath.toStdString());
    
    emit presetExported(name);
    return true;
}

bool PresetManager::deletePreset(const QString& name)
{
    PresetInfo info = presetByName(name);
    
    if (info.filePath.isEmpty()) {
        emit errorOccurred("Preset not found");
        return false;
    }
    
    if (!QFile::remove(info.filePath)) {
        emit errorOccurred("Failed to delete preset file");
        return false;
    }
    
    // Remove metadata
    m_metadata.remove(name);
    savePresetMetadata();
    
    spdlog::info("Deleted preset: {}", name.toStdString());
    
    scanPresets();
    emit presetDeleted(name);
    
    return true;
}

void PresetManager::setFavorite(const QString& name, bool favorite)
{
    QJsonObject presetMeta = m_metadata[name].toObject();
    presetMeta["favorite"] = favorite;
    m_metadata[name] = presetMeta;
    
    savePresetMetadata();
    scanPresets();
}

void PresetManager::setRating(const QString& name, int rating)
{
    rating = qBound(0, rating, 5);
    
    QJsonObject presetMeta = m_metadata[name].toObject();
    presetMeta["rating"] = rating;
    m_metadata[name] = presetMeta;
    
    savePresetMetadata();
    scanPresets();
}

QString PresetManager::getPresetSource(const QString& name) const
{
    PresetInfo info = presetByName(name);
    
    if (info.filePath.isEmpty()) {
        return QString();
    }
    
    QFile file(info.filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }
    
    return QString::fromUtf8(file.readAll());
}

bool PresetManager::savePresetSource(const QString& name, const QString& source)
{
    PresetInfo info = presetByName(name);
    
    if (info.filePath.isEmpty()) {
        emit errorOccurred("Preset not found");
        return false;
    }
    
    QFile file(info.filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit errorOccurred("Failed to open preset for writing");
        return false;
    }
    
    file.write(source.toUtf8());
    file.close();
    
    spdlog::info("Saved preset source: {}", name.toStdString());
    
    emit presetModified(name);
    return true;
}

bool PresetManager::createNewPreset(const QString& name, const QString& source)
{
    QString filePath = m_presetDirectory + "/" + name + ".milk";
    
    if (QFile::exists(filePath)) {
        emit errorOccurred("Preset with this name already exists");
        return false;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit errorOccurred("Failed to create preset file");
        return false;
    }
    
    file.write(source.toUtf8());
    file.close();
    
    spdlog::info("Created new preset: {}", name.toStdString());
    
    scanPresets();
    return true;
}

bool PresetManager::commitPresetChange(const QString& name, const QString& message)
{
    // TODO: Integrate with GitController
    spdlog::info("Committing preset change: {} - {}", name.toStdString(), 
                 message.toStdString());
    return true;
}

QStringList PresetManager::getPresetHistory(const QString& name) const
{
    // TODO: Get history from GitController
    Q_UNUSED(name)
    return QStringList();
}

bool PresetManager::revertPresetToCommit(const QString& name, const QString& commitHash)
{
    // TODO: Revert using GitController
    spdlog::info("Reverting preset '{}' to commit {}", name.toStdString(), 
                 commitHash.toStdString());
    return false;
}

} // namespace suno::visualizer
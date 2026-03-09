#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QList>
#include <QJsonObject>
#include <QDateTime>

namespace suno::visualizer {

struct PresetInfo {
    QString name;
    QString filePath;
    QString author;
    QString description;
    QDateTime created;
    QDateTime modified;
    int rating = 0;
    bool isFavorite = false;
    QString gitCommitHash;
};

class PresetManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int presetCount READ presetCount NOTIFY presetsChanged)

public:
    explicit PresetManager(QObject* parent = nullptr);
    ~PresetManager() override = default;
    
    bool initialize(const QString& presetDirectory);
    
    int presetCount() const { return m_presets.size(); }
    QList<PresetInfo> presets() const { return m_presets; }
    
    PresetInfo presetAt(int index) const;
    PresetInfo presetByName(const QString& name) const;
    
    Q_INVOKABLE bool importPreset(const QString& filePath);
    Q_INVOKABLE bool exportPreset(const QString& name, const QString& destinationPath);
    Q_INVOKABLE bool deletePreset(const QString& name);
    
    Q_INVOKABLE void setFavorite(const QString& name, bool favorite);
    Q_INVOKABLE void setRating(const QString& name, int rating);
    
    // For the built-in shader IDE
    Q_INVOKABLE QString getPresetSource(const QString& name) const;
    Q_INVOKABLE bool savePresetSource(const QString& name, const QString& source);
    Q_INVOKABLE bool createNewPreset(const QString& name, const QString& source);
    
    // Git integration for preset versioning
    Q_INVOKABLE bool commitPresetChange(const QString& name, const QString& message);
    Q_INVOKABLE QStringList getPresetHistory(const QString& name) const;
    Q_INVOKABLE bool revertPresetToCommit(const QString& name, const QString& commitHash);

signals:
    void presetsChanged();
    void presetImported(const QString& name);
    void presetExported(const QString& name);
    void presetDeleted(const QString& name);
    void presetModified(const QString& name);
    void errorOccurred(const QString& error);

private:
    void scanPresets();
    void scanPath(const QString& path, bool recursive = true);
    void loadPresetMetadata();
    void savePresetMetadata();
    QString metadataFilePath() const;
    
    QString m_presetDirectory;
    QStringList m_systemPresetPaths;
    QList<PresetInfo> m_presets;
    QJsonObject m_metadata;
};

} // namespace suno::visualizer
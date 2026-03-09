#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <memory>
#include <toml++/toml.h>

namespace suno::core {

class ConfigManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentTheme READ currentTheme WRITE setCurrentTheme NOTIFY currentThemeChanged)
    Q_PROPERTY(bool autoSaveEnabled READ autoSaveEnabled WRITE setAutoSaveEnabled NOTIFY autoSaveEnabledChanged)
    Q_PROPERTY(QString gitUserName READ gitUserName WRITE setGitUserName NOTIFY gitUserNameChanged)
    Q_PROPERTY(QString gitUserEmail READ gitUserEmail WRITE setGitUserEmail NOTIFY gitUserEmailChanged)
    Q_PROPERTY(bool gitAutoCommit READ gitAutoCommit WRITE setGitAutoCommit NOTIFY gitAutoCommitChanged)

public:
    static ConfigManager* instance();
    
    Q_INVOKABLE bool load();
    Q_INVOKABLE bool save();
    
    // Theme settings
    QString currentTheme() const { return m_currentTheme; }
    Q_INVOKABLE void setCurrentTheme(const QString& theme);
    
    // General settings
    bool autoSaveEnabled() const { return m_autoSaveEnabled; }
    Q_INVOKABLE void setAutoSaveEnabled(bool enabled);
    
    int autoSaveInterval() const { return m_autoSaveInterval; }
    void setAutoSaveInterval(int seconds);
    
    // Video settings
    QString videoCodec() const { return m_videoCodec; }
    void setVideoCodec(const QString& codec);
    
    int videoBitrate() const { return m_videoBitrate; }
    void setVideoBitrate(int bitrate);
    
    int videoFps() const { return m_videoFps; }
    void setVideoFps(int fps);
    
    // Audio settings
    int audioSampleRate() const { return m_audioSampleRate; }
    void setAudioSampleRate(int rate);
    
    int audioBufferSize() const { return m_audioBufferSize; }
    void setAudioBufferSize(int size);
    
    // Visualizer settings
    int visualizerWidth() const { return m_visualizerWidth; }
    void setVisualizerWidth(int width);
    
    int visualizerHeight() const { return m_visualizerHeight; }
    void setVisualizerHeight(int height);
    
    int visualizerFps() const { return m_visualizerFps; }
    void setVisualizerFps(int fps);
    
    // Git settings
    QString gitUserName() const { return m_gitUserName; }
    Q_INVOKABLE void setGitUserName(const QString& name);
    
    QString gitUserEmail() const { return m_gitUserEmail; }
    Q_INVOKABLE void setGitUserEmail(const QString& email);
    
    bool gitAutoCommit() const { return m_gitAutoCommit; }
    Q_INVOKABLE void setGitAutoCommit(bool enabled);
    
    // Network settings
    QString sunoApiEndpoint() const { return m_sunoApiEndpoint; }
    void setSunoApiEndpoint(const QString& endpoint);
    
    QString githubToken() const { return m_githubToken; }
    void setGithubToken(const QString& token);
    
    // Get/Set generic values
    QVariant getValue(const QString& key, const QVariant& defaultValue = QVariant()) const;
    void setValue(const QString& key, const QVariant& value);
    
    ~ConfigManager() override = default;

signals:
    void currentThemeChanged();
    void autoSaveEnabledChanged();
    void gitUserNameChanged();
    void gitUserEmailChanged();
    void gitAutoCommitChanged();
    void configChanged();

private:
    ConfigManager();
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    
    void setDefaults();
    QString configFilePath() const;
    
    // Configuration values
    QString m_currentTheme = "dark";
    bool m_autoSaveEnabled = true;
    int m_autoSaveInterval = 300; // 5 minutes
    
    QString m_videoCodec = "libx264";
    int m_videoBitrate = 8000000; // 8 Mbps
    int m_videoFps = 60;
    
    int m_audioSampleRate = 48000;
    int m_audioBufferSize = 2048;
    
    int m_visualizerWidth = 1920;
    int m_visualizerHeight = 1080;
    int m_visualizerFps = 60;
    
    QString m_gitUserName = "Suno Visualizer User";
    QString m_gitUserEmail = "user@sunovisualizer.local";
    bool m_gitAutoCommit = true;
    
    QString m_sunoApiEndpoint = "https://api.suno.ai"; // Placeholder
    QString m_githubToken = "";
    
    QVariantMap m_customValues;
    
    static ConfigManager* s_instance;
};

} // namespace suno::core
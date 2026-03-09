#pragma once

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <memory>

namespace suno::network {

struct GistInfo {
    QString id;
    QString url;
    QString htmlUrl;
    QString description;
    QString filename;
    QString content;
    bool isPublic = true;
    QDateTime createdAt;
    QDateTime updatedAt;
};

class GistUploader : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isAuthenticated READ isAuthenticated NOTIFY authStateChanged)
    Q_PROPERTY(bool isBusy READ isBusy NOTIFY busyStateChanged)

public:
    explicit GistUploader(QObject* parent = nullptr);
    ~GistUploader() override = default;
    
    bool isAuthenticated() const { return !m_githubToken.isEmpty(); }
    bool isBusy() const { return m_busy; }
    
    void setGithubToken(const QString& token);
    
    // Upload presets as gists
    Q_INVOKABLE void uploadPreset(const QString& presetName, 
                                   const QString& presetContent,
                                   const QString& description = QString(),
                                   bool isPublic = true);
    
    // Fetch community presets
    Q_INVOKABLE void fetchPublicPresets(const QString& searchQuery = QString());
    Q_INVOKABLE void fetchGistById(const QString& gistId);
    Q_INVOKABLE void fetchUserGists();
    
    // Update existing gist
    Q_INVOKABLE void updateGist(const QString& gistId, 
                                 const QString& presetContent,
                                 const QString& description = QString());
    
    // Delete gist
    Q_INVOKABLE void deleteGist(const QString& gistId);
    
    // Download preset from gist URL
    Q_INVOKABLE void downloadPresetFromGist(const QString& gistUrl, 
                                             const QString& savePath);

signals:
    void authStateChanged();
    void busyStateChanged();
    
    void presetUploaded(const GistInfo& gist);
    void presetUpdated(const GistInfo& gist);
    void presetDeleted(const QString& gistId);
    
    void gistFetched(const GistInfo& gist);
    void publicPresetsFetched(const QList<GistInfo>& gists);
    void userGistsFetched(const QList<GistInfo>& gists);
    
    void presetDownloaded(const QString& savePath);
    
    void errorOccurred(const QString& error);

private:
    QNetworkRequest createRequest(const QString& endpoint) const;
    GistInfo parseGist(const QJsonObject& json) const;
    
    std::unique_ptr<QNetworkAccessManager> m_networkManager;
    
    QString m_githubToken;
    bool m_busy = false;
    
    static constexpr const char* GITHUB_API_URL = "https://api.github.com";
    static constexpr const char* PRESET_TAG = "[suno-visualizer-preset]";
};

} // namespace suno::network
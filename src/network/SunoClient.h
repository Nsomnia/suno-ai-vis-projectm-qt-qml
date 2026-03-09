#pragma once

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <QJsonArray>
#include <memory>

namespace suno::network {

struct SunoTrack {
    QString id;
    QString title;
    QString artist;
    QString audioUrl;
    QString imageUrl;
    QString lyrics;
    int durationMs = 0;
    QDateTime createdAt;
    QString genre;
    QString tags;
};

struct SunoGenerationRequest {
    QString prompt;
    QString style;
    QString lyrics;
    bool instrumental = false;
    int durationSeconds = 180;
};

class SunoClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isAuthenticated READ isAuthenticated NOTIFY authStateChanged)
    Q_PROPERTY(bool isBusy READ isBusy NOTIFY busyStateChanged)

public:
    explicit SunoClient(QObject* parent = nullptr);
    ~SunoClient() override = default;
    
    bool isAuthenticated() const { return m_authenticated; }
    bool isBusy() const { return m_busy; }
    
    // Authentication
    Q_INVOKABLE void authenticate(const QString& sessionToken);
    Q_INVOKABLE void logout();
    
    // Track fetching
    Q_INVOKABLE void fetchMyTracks(int limit = 50, int offset = 0);
    Q_INVOKABLE void fetchTrackById(const QString& trackId);
    Q_INVOKABLE void fetchTrackLyrics(const QString& trackId);
    Q_INVOKABLE void downloadTrackAudio(const QString& trackId, const QString& savePath);
    Q_INVOKABLE void downloadTrackImage(const QString& trackId, const QString& savePath);
    
    // Generation
    Q_INVOKABLE void generateTrack(const SunoGenerationRequest& request);
    Q_INVOKABLE void checkGenerationStatus(const QString& generationId);
    
    void setApiEndpoint(const QString& endpoint);

signals:
    void authStateChanged();
    void busyStateChanged();
    
    void tracksReceived(const QList<SunoTrack>& tracks);
    void trackReceived(const SunoTrack& track);
    void lyricsReceived(const QString& trackId, const QString& lyrics);
    
    void audioDownloaded(const QString& trackId, const QString& filePath);
    void imageDownloaded(const QString& trackId, const QString& filePath);
    
    void generationStarted(const QString& generationId);
    void generationProgress(const QString& generationId, int progress);
    void generationComplete(const QString& generationId, const SunoTrack& track);
    void generationFailed(const QString& generationId, const QString& error);
    
    void errorOccurred(const QString& error);

private:
    QNetworkRequest createRequest(const QString& endpoint) const;
    void handleTracksResponse(const QJsonObject& response);
    void handleTrackResponse(const QJsonObject& response);
    SunoTrack parseTrack(const QJsonObject& json);
    
    std::unique_ptr<QNetworkAccessManager> m_networkManager;
    
    QString m_apiEndpoint = "https://studio-api.suno.ai";
    QString m_sessionToken;
    bool m_authenticated = false;
    bool m_busy = false;
};

} // namespace suno::network
#include "SunoClient.h"
#include <spdlog/spdlog.h>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QFile>

namespace suno::network {

SunoClient::SunoClient(QObject* parent)
    : QObject(parent)
    , m_networkManager(std::make_unique<QNetworkAccessManager>())
{
    spdlog::debug("SunoClient initialized");
    
    // Note: This implementation targets the unofficial/reverse-engineered Suno API
    // Use at your own risk, and be respectful of rate limits
    spdlog::warn("SunoClient: Using unofficial API endpoints - use responsibly!");
}

void SunoClient::setApiEndpoint(const QString& endpoint)
{
    m_apiEndpoint = endpoint;
    spdlog::info("Suno API endpoint set to: {}", endpoint.toStdString());
}

void SunoClient::authenticate(const QString& sessionToken)
{
    if (sessionToken.isEmpty()) {
        emit errorOccurred("Session token cannot be empty");
        return;
    }
    
    spdlog::info("Authenticating with Suno API...");
    
    m_sessionToken = sessionToken;
    
    // Verify token by fetching user info
    m_busy = true;
    emit busyStateChanged();
    
    QNetworkRequest request = createRequest("/api/session");
    
    QNetworkReply* reply = m_networkManager->get(request);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        m_busy = false;
        emit busyStateChanged();
        
        if (reply->error() == QNetworkReply::NoError) {
            m_authenticated = true;
            emit authStateChanged();
            spdlog::info("Suno authentication successful");
        } else {
            m_sessionToken.clear();
            QString error = QString("Authentication failed: %1").arg(reply->errorString());
            spdlog::error("{}", error.toStdString());
            emit errorOccurred(error);
        }
        
        reply->deleteLater();
    });
}

void SunoClient::logout()
{
    m_sessionToken.clear();
    m_authenticated = false;
    emit authStateChanged();
    spdlog::info("Logged out from Suno");
}

QNetworkRequest SunoClient::createRequest(const QString& endpoint) const
{
    QNetworkRequest request;
    request.setUrl(QUrl(m_apiEndpoint + endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Accept", "application/json");
    
    if (!m_sessionToken.isEmpty()) {
        request.setRawHeader("Cookie", QString("__session=%1").arg(m_sessionToken).toUtf8());
    }
    
    // Mimic browser user agent
    request.setRawHeader("User-Agent", 
        "Mozilla/5.0 (X11; Linux x86_64; rv:120.0) Gecko/20100101 Firefox/120.0");
    
    return request;
}

void SunoClient::fetchMyTracks(int limit, int offset)
{
    if (!m_authenticated) {
        emit errorOccurred("Not authenticated");
        return;
    }
    
    spdlog::debug("Fetching tracks: limit={}, offset={}", limit, offset);
    
    m_busy = true;
    emit busyStateChanged();
    
    QString endpoint = QString("/api/feed?limit=%1&offset=%2").arg(limit).arg(offset);
    QNetworkRequest request = createRequest(endpoint);
    
    QNetworkReply* reply = m_networkManager->get(request);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        m_busy = false;
        emit busyStateChanged();
        
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            handleTracksResponse(doc.object());
        } else {
            QString error = QString("Failed to fetch tracks: %1").arg(reply->errorString());
            spdlog::error("{}", error.toStdString());
            emit errorOccurred(error);
        }
        
        reply->deleteLater();
    });
}

void SunoClient::fetchTrackById(const QString& trackId)
{
    if (!m_authenticated) {
        emit errorOccurred("Not authenticated");
        return;
    }
    
    spdlog::debug("Fetching track: {}", trackId.toStdString());
    
    QString endpoint = QString("/api/clip/%1").arg(trackId);
    QNetworkRequest request = createRequest(endpoint);
    
    QNetworkReply* reply = m_networkManager->get(request);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            handleTrackResponse(doc.object());
        } else {
            QString error = QString("Failed to fetch track: %1").arg(reply->errorString());
            spdlog::error("{}", error.toStdString());
            emit errorOccurred(error);
        }
        
        reply->deleteLater();
    });
}

void SunoClient::fetchTrackLyrics(const QString& trackId)
{
    // Lyrics are typically included in track metadata
    fetchTrackById(trackId);
}

void SunoClient::downloadTrackAudio(const QString& trackId, const QString& savePath)
{
    if (!m_authenticated) {
        emit errorOccurred("Not authenticated");
        return;
    }
    
    spdlog::info("Downloading audio for track: {} to {}", 
                 trackId.toStdString(), savePath.toStdString());
    
    // First fetch track to get audio URL
    QString endpoint = QString("/api/clip/%1").arg(trackId);
    QNetworkRequest request = createRequest(endpoint);
    
    QNetworkReply* reply = m_networkManager->get(request);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply, trackId, savePath]() {
        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred("Failed to get track info for download");
            reply->deleteLater();
            return;
        }
        
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject obj = doc.object();
        QString audioUrl = obj["audio_url"].toString();
        
        reply->deleteLater();
        
        if (audioUrl.isEmpty()) {
            emit errorOccurred("No audio URL available for track");
            return;
        }
        
        // Download the actual audio file
        QNetworkRequest audioRequest;
        audioRequest.setUrl(QUrl(audioUrl));
        
        QNetworkReply* audioReply = m_networkManager->get(audioRequest);
        
        connect(audioReply, &QNetworkReply::finished, this, 
                [this, audioReply, trackId, savePath]() {
            if (audioReply->error() == QNetworkReply::NoError) {
                QFile file(savePath);
                if (file.open(QIODevice::WriteOnly)) {
                    file.write(audioReply->readAll());
                    file.close();
                    spdlog::info("Audio downloaded: {}", savePath.toStdString());
                    emit audioDownloaded(trackId, savePath);
                } else {
                    emit errorOccurred("Failed to save audio file");
                }
            } else {
                emit errorOccurred("Failed to download audio");
            }
            
            audioReply->deleteLater();
        });
    });
}

void SunoClient::downloadTrackImage(const QString& trackId, const QString& savePath)
{
    // Similar pattern to audio download
    spdlog::debug("Downloading image for track: {}", trackId.toStdString());
    
    // Implementation would follow same pattern as downloadTrackAudio
    // Fetching image_url from track metadata and downloading it
}

void SunoClient::generateTrack(const SunoGenerationRequest& request)
{
    if (!m_authenticated) {
        emit errorOccurred("Not authenticated");
        return;
    }
    
    spdlog::info("Starting track generation with prompt: {}", request.prompt.toStdString());
    
    m_busy = true;
    emit busyStateChanged();
    
    QJsonObject body;
    body["prompt"] = request.prompt;
    body["mv"] = "chirp-v3-5";
    body["make_instrumental"] = request.instrumental;
    
    if (!request.lyrics.isEmpty()) {
        body["lyrics"] = request.lyrics;
    }
    
    QNetworkRequest netRequest = createRequest("/api/generate/v2/");
    QNetworkReply* reply = m_networkManager->post(netRequest, 
                                                   QJsonDocument(body).toJson());
    
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        m_busy = false;
        emit busyStateChanged();
        
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonObject obj = doc.object();
            
            QString generationId = obj["id"].toString();
            spdlog::info("Generation started: {}", generationId.toStdString());
            emit generationStarted(generationId);
        } else {
            QString error = QString("Generation failed: %1").arg(reply->errorString());
            spdlog::error("{}", error.toStdString());
            emit errorOccurred(error);
        }
        
        reply->deleteLater();
    });
}

void SunoClient::checkGenerationStatus(const QString& generationId)
{
    QString endpoint = QString("/api/feed/%1").arg(generationId);
    QNetworkRequest request = createRequest(endpoint);
    
    QNetworkReply* reply = m_networkManager->get(request);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply, generationId]() {
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonArray arr = doc.array();
            
            if (!arr.isEmpty()) {
                QJsonObject obj = arr.first().toObject();
                QString status = obj["status"].toString();
                
                if (status == "complete") {
                    SunoTrack track = parseTrack(obj);
                    emit generationComplete(generationId, track);
                } else if (status == "error") {
                    emit generationFailed(generationId, "Generation failed");
                } else {
                    // Still processing
                    int progress = static_cast<int>(obj["progress"].toDouble() * 100);
                    emit generationProgress(generationId, progress);
                }
            }
        }
        
        reply->deleteLater();
    });
}

void SunoClient::handleTracksResponse(const QJsonObject& response)
{
    QList<SunoTrack> tracks;
    
    QJsonArray items = response["clips"].toArray();
    for (const QJsonValue& val : items) {
        tracks.append(parseTrack(val.toObject()));
    }
    
    spdlog::info("Received {} tracks", tracks.size());
    emit tracksReceived(tracks);
}

void SunoClient::handleTrackResponse(const QJsonObject& response)
{
    SunoTrack track = parseTrack(response);
    
    if (!track.lyrics.isEmpty()) {
        emit lyricsReceived(track.id, track.lyrics);
    }
    
    emit trackReceived(track);
}

SunoTrack SunoClient::parseTrack(const QJsonObject& json)
{
    SunoTrack track;
    
    track.id = json["id"].toString();
    track.title = json["title"].toString();
    track.audioUrl = json["audio_url"].toString();
    track.imageUrl = json["image_url"].toString();
    track.lyrics = json["metadata"].toObject()["prompt"].toString();
    track.genre = json["metadata"].toObject()["tags"].toString();
    
    // Duration in seconds, convert to ms
    track.durationMs = static_cast<int>(json["metadata"].toObject()["duration"].toDouble() * 1000);
    
    return track;
}

} // namespace suno::network
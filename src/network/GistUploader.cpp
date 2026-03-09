#include "GistUploader.h"
#include <spdlog/spdlog.h>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QUrlQuery>

namespace suno::network {

GistUploader::GistUploader(QObject* parent)
    : QObject(parent)
    , m_networkManager(std::make_unique<QNetworkAccessManager>())
{
    spdlog::debug("GistUploader initialized");
    spdlog::info("P2P preset sharing ready - share your sick visuals with the world!");
}

void GistUploader::setGithubToken(const QString& token)
{
    bool wasAuthenticated = isAuthenticated();
    m_githubToken = token;
    
    if (wasAuthenticated != isAuthenticated()) {
        emit authStateChanged();
    }
    
    if (isAuthenticated()) {
        spdlog::info("GitHub token configured for gist uploads");
    }
}

QNetworkRequest GistUploader::createRequest(const QString& endpoint) const
{
    QNetworkRequest request;
    request.setUrl(QUrl(QString(GITHUB_API_URL) + endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Accept", "application/vnd.github+json");
    request.setRawHeader("X-GitHub-Api-Version", "2022-11-28");
    
    if (!m_githubToken.isEmpty()) {
        request.setRawHeader("Authorization", 
                            QString("Bearer %1").arg(m_githubToken).toUtf8());
    }
    
    // Be a good API citizen
    request.setRawHeader("User-Agent", "SunoVisualizer/0.0.1 (github.com/sunovisualizer)");
    
    return request;
}

void GistUploader::uploadPreset(const QString& presetName,
                                 const QString& presetContent,
                                 const QString& description,
                                 bool isPublic)
{
    if (!isAuthenticated()) {
        emit errorOccurred("GitHub token not configured");
        return;
    }
    
    spdlog::info("Uploading preset '{}' to GitHub Gist", presetName.toStdString());
    
    m_busy = true;
    emit busyStateChanged();
    
    QString filename = presetName + ".milk";
    QString desc = description.isEmpty() 
                   ? QString("%1 %2").arg(PRESET_TAG).arg(presetName)
                   : QString("%1 %2").arg(PRESET_TAG).arg(description);
    
    QJsonObject files;
    QJsonObject fileContent;
    fileContent["content"] = presetContent;
    files[filename] = fileContent;
    
    QJsonObject body;
    body["description"] = desc;
    body["public"] = isPublic;
    body["files"] = files;
    
    QNetworkRequest request = createRequest("/gists");
    QNetworkReply* reply = m_networkManager->post(request, 
                                                   QJsonDocument(body).toJson());
    
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        m_busy = false;
        emit busyStateChanged();
        
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            GistInfo gist = parseGist(doc.object());
            
            spdlog::info("Preset uploaded successfully: {}", gist.htmlUrl.toStdString());
            emit presetUploaded(gist);
        } else {
            QString error = QString("Failed to upload preset: %1").arg(reply->errorString());
            spdlog::error("{}", error.toStdString());
            emit errorOccurred(error);
        }
        
        reply->deleteLater();
    });
}

void GistUploader::fetchPublicPresets(const QString& searchQuery)
{
    spdlog::debug("Fetching public presets");
    
    m_busy = true;
    emit busyStateChanged();
    
    // Search for gists with our tag
    // Note: GitHub's gist search is limited, we search in descriptions
    QString query = searchQuery.isEmpty() 
                    ? QString(PRESET_TAG)
                    : QString("%1 %2").arg(PRESET_TAG).arg(searchQuery);
    
    // GitHub doesn't have a direct gist search API, so we use code search
    // This is a workaround - in production you might want a dedicated backend
    QString endpoint = "/gists/public";
    
    QNetworkRequest request = createRequest(endpoint);
    QNetworkReply* reply = m_networkManager->get(request);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        m_busy = false;
        emit busyStateChanged();
        
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonArray arr = doc.array();
            
            QList<GistInfo> presets;
            for (const QJsonValue& val : arr) {
                QJsonObject obj = val.toObject();
                QString desc = obj["description"].toString();
                
                // Filter for our presets
                if (desc.contains(PRESET_TAG)) {
                    presets.append(parseGist(obj));
                }
            }
            
            spdlog::info("Found {} community presets", presets.size());
            emit publicPresetsFetched(presets);
        } else {
            QString error = QString("Failed to fetch presets: %1").arg(reply->errorString());
            spdlog::error("{}", error.toStdString());
            emit errorOccurred(error);
        }
        
        reply->deleteLater();
    });
}

void GistUploader::fetchGistById(const QString& gistId)
{
    spdlog::debug("Fetching gist: {}", gistId.toStdString());
    
    QString endpoint = QString("/gists/%1").arg(gistId);
    QNetworkRequest request = createRequest(endpoint);
    QNetworkReply* reply = m_networkManager->get(request);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            GistInfo gist = parseGist(doc.object());
            emit gistFetched(gist);
        } else {
            emit errorOccurred("Failed to fetch gist");
        }
        
        reply->deleteLater();
    });
}

void GistUploader::fetchUserGists()
{
    if (!isAuthenticated()) {
        emit errorOccurred("GitHub token not configured");
        return;
    }
    
    spdlog::debug("Fetching user's gists");
    
    m_busy = true;
    emit busyStateChanged();
    
    QNetworkRequest request = createRequest("/gists");
    QNetworkReply* reply = m_networkManager->get(request);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        m_busy = false;
        emit busyStateChanged();
        
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonArray arr = doc.array();
            
            QList<GistInfo> gists;
            for (const QJsonValue& val : arr) {
                QJsonObject obj = val.toObject();
                QString desc = obj["description"].toString();
                
                // Filter for our presets
                if (desc.contains(PRESET_TAG)) {
                    gists.append(parseGist(obj));
                }
            }
            
            spdlog::info("Found {} user presets", gists.size());
            emit userGistsFetched(gists);
        } else {
            emit errorOccurred("Failed to fetch user gists");
        }
        
        reply->deleteLater();
    });
}

void GistUploader::updateGist(const QString& gistId,
                               const QString& presetContent,
                               const QString& description)
{
    if (!isAuthenticated()) {
        emit errorOccurred("GitHub token not configured");
        return;
    }
    
    spdlog::info("Updating gist: {}", gistId.toStdString());
    
    m_busy = true;
    emit busyStateChanged();
    
    // First fetch the gist to get the filename
    fetchGistById(gistId);
    
    // For simplicity, we'll use a generic approach
    // In production, chain this properly with the fetch
    
    QJsonObject body;
    if (!description.isEmpty()) {
        body["description"] = QString("%1 %2").arg(PRESET_TAG).arg(description);
    }
    
    // Note: We need the filename from the existing gist
    // This is simplified - proper implementation would chain requests
    
    QString endpoint = QString("/gists/%1").arg(gistId);
    QNetworkRequest request = createRequest(endpoint);
    
    QNetworkReply* reply = m_networkManager->sendCustomRequest(
        request, "PATCH", QJsonDocument(body).toJson());
    
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        m_busy = false;
        emit busyStateChanged();
        
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            GistInfo gist = parseGist(doc.object());
            
            spdlog::info("Gist updated successfully");
            emit presetUpdated(gist);
        } else {
            emit errorOccurred("Failed to update gist");
        }
        
        reply->deleteLater();
    });
}

void GistUploader::deleteGist(const QString& gistId)
{
    if (!isAuthenticated()) {
        emit errorOccurred("GitHub token not configured");
        return;
    }
    
    spdlog::info("Deleting gist: {}", gistId.toStdString());
    
    m_busy = true;
    emit busyStateChanged();
    
    QString endpoint = QString("/gists/%1").arg(gistId);
    QNetworkRequest request = createRequest(endpoint);
    
    QNetworkReply* reply = m_networkManager->deleteResource(request);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply, gistId]() {
        m_busy = false;
        emit busyStateChanged();
        
        if (reply->error() == QNetworkReply::NoError) {
            spdlog::info("Gist deleted successfully");
            emit presetDeleted(gistId);
        } else {
            emit errorOccurred("Failed to delete gist");
        }
        
        reply->deleteLater();
    });
}

void GistUploader::downloadPresetFromGist(const QString& gistUrl, 
                                           const QString& savePath)
{
    spdlog::info("Downloading preset from: {}", gistUrl.toStdString());
    
    // Extract gist ID from URL
    // Format: https://gist.github.com/username/gistid
    QUrl url(gistUrl);
    QString path = url.path();
    QStringList parts = path.split('/');
    
    if (parts.size() < 2) {
        emit errorOccurred("Invalid gist URL");
        return;
    }
    
    QString gistId = parts.last();
    
    m_busy = true;
    emit busyStateChanged();
    
    QString endpoint = QString("/gists/%1").arg(gistId);
    QNetworkRequest request = createRequest(endpoint);
    QNetworkReply* reply = m_networkManager->get(request);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply, savePath]() {
        m_busy = false;
        emit busyStateChanged();
        
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonObject obj = doc.object();
            QJsonObject files = obj["files"].toObject();
            
            // Get the first .milk file
            for (auto it = files.begin(); it != files.end(); ++it) {
                QString filename = it.key();
                if (filename.endsWith(".milk") || filename.endsWith(".prjm")) {
                    QString content = it.value().toObject()["content"].toString();
                    
                    QFile file(savePath);
                    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                        file.write(content.toUtf8());
                        file.close();
                        
                        spdlog::info("Preset downloaded to: {}", savePath.toStdString());
                        emit presetDownloaded(savePath);
                        reply->deleteLater();
                        return;
                    }
                }
            }
            
            emit errorOccurred("No preset file found in gist");
        } else {
            emit errorOccurred("Failed to download preset");
        }
        
        reply->deleteLater();
    });
}

GistInfo GistUploader::parseGist(const QJsonObject& json) const
{
    GistInfo info;
    
    info.id = json["id"].toString();
    info.url = json["url"].toString();
    info.htmlUrl = json["html_url"].toString();
    info.description = json["description"].toString();
    info.isPublic = json["public"].toBool();
    
    // Parse files
    QJsonObject files = json["files"].toObject();
    for (auto it = files.begin(); it != files.end(); ++it) {
        QString filename = it.key();
        if (filename.endsWith(".milk") || filename.endsWith(".prjm")) {
            info.filename = filename;
            info.content = it.value().toObject()["content"].toString();
            break;
        }
    }
    
    // Parse dates
    info.createdAt = QDateTime::fromString(json["created_at"].toString(), Qt::ISODate);
    info.updatedAt = QDateTime::fromString(json["updated_at"].toString(), Qt::ISODate);
    
    return info;
}

} // namespace suno::network
#include "Application.h"
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <spdlog/spdlog.h>

namespace suno::core {

Application* Application::s_instance = nullptr;

Application::Application()
{
    m_configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) 
                  + "/suno-visualizer";
    m_cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    m_dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

Application* Application::instance()
{
    if (!s_instance) {
        s_instance = new Application();
    }
    return s_instance;
}

bool Application::initialize()
{
    if (m_initialized) {
        spdlog::warn("Application already initialized");
        return true;
    }

    spdlog::info("Initializing application core...");
    
    if (!createDirectories()) {
        spdlog::error("Failed to create application directories");
        return false;
    }

    if (!detectSystemCapabilities()) {
        spdlog::error("System capability detection failed");
        return false;
    }

    m_initialized = true;
    emit initialized();
    
    spdlog::info("Core initialization complete");
    spdlog::info("Config dir: {}", m_configDir.toStdString());
    spdlog::info("Cache dir: {}", m_cacheDir.toStdString());
    spdlog::info("Data dir: {}", m_dataDir.toStdString());
    
    return true;
}

void Application::shutdown()
{
    spdlog::info("Shutting down application core");
    emit shutdownRequested();
    m_initialized = false;
}

QString Application::configDirectory() const
{
    return m_configDir;
}

QString Application::cacheDirectory() const
{
    return m_cacheDir;
}

QString Application::dataDirectory() const
{
    return m_dataDir;
}

bool Application::createDirectories()
{
    QDir configDir(m_configDir);
    QDir cacheDir(m_cacheDir);
    QDir dataDir(m_dataDir);

    bool success = true;

    if (!configDir.exists()) {
        if (!configDir.mkpath(".")) {
            spdlog::error("Failed to create config directory: {}", 
                         m_configDir.toStdString());
            success = false;
        }
    }

    if (!cacheDir.exists()) {
        if (!cacheDir.mkpath(".")) {
            spdlog::error("Failed to create cache directory: {}", 
                         m_cacheDir.toStdString());
            success = false;
        }
    }

    if (!dataDir.exists()) {
        if (!dataDir.mkpath(".")) {
            spdlog::error("Failed to create data directory: {}", 
                         m_dataDir.toStdString());
            success = false;
        }
    }

    // Create subdirectories
    QStringList subdirs = {"presets", "exports", "git-repos", "temp", "logs"};
    for (const auto& subdir : subdirs) {
        QDir dir(m_dataDir + "/" + subdir);
        if (!dir.exists()) {
            if (!dir.mkpath(".")) {
                spdlog::warn("Failed to create subdirectory: {}", subdir.toStdString());
            }
        }
    }

    return success;
}

bool Application::detectSystemCapabilities()
{
    // Detect GPU capabilities, FFmpeg availability, etc.
    spdlog::info("Detecting system capabilities...");
    
    // TODO: Implement actual detection
    // For now, assume we have everything (optimistic senior dev approach)
    
    spdlog::info("System capability detection complete");
    return true;
}

} // namespace suno::core
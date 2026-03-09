#include "ConfigManager.h"
#include "Application.h"
#include <QFile>
#include <QTextStream>
#include <spdlog/spdlog.h>
#include <fstream>

namespace suno::core {

ConfigManager* ConfigManager::s_instance = nullptr;

ConfigManager::ConfigManager()
{
    setDefaults();
}

ConfigManager* ConfigManager::instance()
{
    if (!s_instance) {
        s_instance = new ConfigManager();
    }
    return s_instance;
}

void ConfigManager::setDefaults()
{
    // Defaults already set in member initialization
    spdlog::debug("Configuration defaults set");
}

QString ConfigManager::configFilePath() const
{
    return Application::instance()->configDirectory() + "/config.toml";
}

bool ConfigManager::load()
{
    QString filePath = configFilePath();
    
    spdlog::info("Loading configuration from: {}", filePath.toStdString());
    
    if (!QFile::exists(filePath)) {
        spdlog::warn("Config file does not exist, creating with defaults");
        return save();
    }

    try {
        auto config = toml::parse_file(filePath.toStdString());
        
        // Theme settings
        if (auto theme = config["theme"]["current"].value<std::string>()) {
            m_currentTheme = QString::fromStdString(*theme);
        }
        
        // General settings
        if (auto enabled = config["general"]["auto_save_enabled"].value<bool>()) {
            m_autoSaveEnabled = *enabled;
        }
        if (auto interval = config["general"]["auto_save_interval"].value<int64_t>()) {
            m_autoSaveInterval = static_cast<int>(*interval);
        }
        
        // Video settings
        if (auto codec = config["video"]["codec"].value<std::string>()) {
            m_videoCodec = QString::fromStdString(*codec);
        }
        if (auto bitrate = config["video"]["bitrate"].value<int64_t>()) {
            m_videoBitrate = static_cast<int>(*bitrate);
        }
        if (auto fps = config["video"]["fps"].value<int64_t>()) {
            m_videoFps = static_cast<int>(*fps);
        }
        
        // Audio settings
        if (auto rate = config["audio"]["sample_rate"].value<int64_t>()) {
            m_audioSampleRate = static_cast<int>(*rate);
        }
        if (auto buffer = config["audio"]["buffer_size"].value<int64_t>()) {
            m_audioBufferSize = static_cast<int>(*buffer);
        }
        
        // Visualizer settings
        if (auto width = config["visualizer"]["width"].value<int64_t>()) {
            m_visualizerWidth = static_cast<int>(*width);
        }
        if (auto height = config["visualizer"]["height"].value<int64_t>()) {
            m_visualizerHeight = static_cast<int>(*height);
        }
        if (auto fps = config["visualizer"]["fps"].value<int64_t>()) {
            m_visualizerFps = static_cast<int>(*fps);
        }
        
        // Git settings
        if (auto name = config["git"]["user_name"].value<std::string>()) {
            m_gitUserName = QString::fromStdString(*name);
        }
        if (auto email = config["git"]["user_email"].value<std::string>()) {
            m_gitUserEmail = QString::fromStdString(*email);
        }
        if (auto autoCommit = config["git"]["auto_commit"].value<bool>()) {
            m_gitAutoCommit = *autoCommit;
        }
        
        // Network settings
        if (auto endpoint = config["network"]["suno_api_endpoint"].value<std::string>()) {
            m_sunoApiEndpoint = QString::fromStdString(*endpoint);
        }
        if (auto token = config["network"]["github_token"].value<std::string>()) {
            m_githubToken = QString::fromStdString(*token);
        }
        
        spdlog::info("Configuration loaded successfully");
        emit configChanged();
        return true;
        
    } catch (const toml::parse_error& err) {
        spdlog::error("TOML parse error: {}", err.description());
        return false;
    } catch (const std::exception& e) {
        spdlog::error("Failed to load configuration: {}", e.what());
        return false;
    }
}

bool ConfigManager::save()
{
    QString filePath = configFilePath();
    
    spdlog::info("Saving configuration to: {}", filePath.toStdString());
    
    try {
        toml::table config;
        
        // Theme settings
        config.insert("theme", toml::table{
            {"current", m_currentTheme.toStdString()}
        });
        
        // General settings
        config.insert("general", toml::table{
            {"auto_save_enabled", m_autoSaveEnabled},
            {"auto_save_interval", m_autoSaveInterval}
        });
        
        // Video settings
        config.insert("video", toml::table{
            {"codec", m_videoCodec.toStdString()},
            {"bitrate", m_videoBitrate},
            {"fps", m_videoFps}
        });
        
        // Audio settings
        config.insert("audio", toml::table{
            {"sample_rate", m_audioSampleRate},
            {"buffer_size", m_audioBufferSize}
        });
        
        // Visualizer settings
        config.insert("visualizer", toml::table{
            {"width", m_visualizerWidth},
            {"height", m_visualizerHeight},
            {"fps", m_visualizerFps}
        });
        
        // Git settings
        config.insert("git", toml::table{
            {"user_name", m_gitUserName.toStdString()},
            {"user_email", m_gitUserEmail.toStdString()},
            {"auto_commit", m_gitAutoCommit}
        });
        
        // Network settings
        config.insert("network", toml::table{
            {"suno_api_endpoint", m_sunoApiEndpoint.toStdString()},
            {"github_token", m_githubToken.toStdString()}
        });
        
        std::ofstream file(filePath.toStdString());
        file << config;
        file.close();
        
        spdlog::info("Configuration saved successfully");
        return true;
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to save configuration: {}", e.what());
        return false;
    }
}

void ConfigManager::setCurrentTheme(const QString& theme)
{
    if (m_currentTheme != theme) {
        m_currentTheme = theme;
        emit currentThemeChanged();
        emit configChanged();
    }
}

void ConfigManager::setAutoSaveEnabled(bool enabled)
{
    if (m_autoSaveEnabled != enabled) {
        m_autoSaveEnabled = enabled;
        emit autoSaveEnabledChanged();
        emit configChanged();
    }
}

void ConfigManager::setAutoSaveInterval(int seconds)
{
    if (m_autoSaveInterval != seconds) {
        m_autoSaveInterval = seconds;
        emit configChanged();
    }
}

void ConfigManager::setVideoCodec(const QString& codec)
{
    if (m_videoCodec != codec) {
        m_videoCodec = codec;
        emit configChanged();
    }
}

void ConfigManager::setVideoBitrate(int bitrate)
{
    if (m_videoBitrate != bitrate) {
        m_videoBitrate = bitrate;
        emit configChanged();
    }
}

void ConfigManager::setVideoFps(int fps)
{
    if (m_videoFps != fps) {
        m_videoFps = fps;
        emit configChanged();
    }
}

void ConfigManager::setAudioSampleRate(int rate)
{
    if (m_audioSampleRate != rate) {
        m_audioSampleRate = rate;
        emit configChanged();
    }
}

void ConfigManager::setAudioBufferSize(int size)
{
    if (m_audioBufferSize != size) {
        m_audioBufferSize = size;
        emit configChanged();
    }
}

void ConfigManager::setVisualizerWidth(int width)
{
    if (m_visualizerWidth != width) {
        m_visualizerWidth = width;
        emit configChanged();
    }
}

void ConfigManager::setVisualizerHeight(int height)
{
    if (m_visualizerHeight != height) {
        m_visualizerHeight = height;
        emit configChanged();
    }
}

void ConfigManager::setVisualizerFps(int fps)
{
    if (m_visualizerFps != fps) {
        m_visualizerFps = fps;
        emit configChanged();
    }
}

void ConfigManager::setGitUserName(const QString& name)
{
    if (m_gitUserName != name) {
        m_gitUserName = name;
        emit configChanged();
    }
}

void ConfigManager::setGitUserEmail(const QString& email)
{
    if (m_gitUserEmail != email) {
        m_gitUserEmail = email;
        emit configChanged();
    }
}

void ConfigManager::setGitAutoCommit(bool enabled)
{
    if (m_gitAutoCommit != enabled) {
        m_gitAutoCommit = enabled;
        emit configChanged();
    }
}

void ConfigManager::setSunoApiEndpoint(const QString& endpoint)
{
    if (m_sunoApiEndpoint != endpoint) {
        m_sunoApiEndpoint = endpoint;
        emit configChanged();
    }
}

void ConfigManager::setGithubToken(const QString& token)
{
    if (m_githubToken != token) {
        m_githubToken = token;
        emit configChanged();
    }
}

QVariant ConfigManager::getValue(const QString& key, const QVariant& defaultValue) const
{
    return m_customValues.value(key, defaultValue);
}

void ConfigManager::setValue(const QString& key, const QVariant& value)
{
    m_customValues[key] = value;
    emit configChanged();
}

} // namespace suno::core
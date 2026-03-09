#pragma once

#include <QObject>
#include <QString>
#include <memory>

namespace suno::core {

class Application : public QObject
{
    Q_OBJECT

public:
    static Application* instance();
    
    bool initialize();
    void shutdown();
    
    QString configDirectory() const;
    QString cacheDirectory() const;
    QString dataDirectory() const;
    
    ~Application() override = default;

signals:
    void initialized();
    void shutdownRequested();

private:
    Application();
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    bool createDirectories();
    bool detectSystemCapabilities();
    
    QString m_configDir;
    QString m_cacheDir;
    QString m_dataDir;
    bool m_initialized = false;
    
    static Application* s_instance;
};

} // namespace suno::core
#pragma once

#include <QObject>
#include <QString>
#include <QImage>
#include <memory>
#include <vector>

// Forward declarations for libprojectM
// "I'd just like to interject for a moment... what you're referring to as
// visualization, is in fact, projectM/MilkDrop visualization"
struct projectm;
typedef struct projectm projectm_handle;

namespace suno::visualizer {

class ProjectMWrapper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool initialized READ isInitialized NOTIFY initializedChanged)
    Q_PROPERTY(QString currentPreset READ currentPreset NOTIFY presetChanged)
    Q_PROPERTY(int fps READ fps WRITE setFps NOTIFY fpsChanged)

public:
    explicit ProjectMWrapper(QObject* parent = nullptr);
    ~ProjectMWrapper() override;
    
    bool initialize(int width, int height);
    void shutdown();
    
    bool isInitialized() const { return m_initialized; }
    
    // Rendering
    void renderFrame();
    QImage grabFrameBuffer();
    void resize(int width, int height);
    
    // Audio input
    void addPCMData(const float* data, size_t samples);
    void addPCMDataStereo(const float* left, const float* right, size_t samples);
    
    // Stem-specific audio routing
    void addVocalsPCM(const float* data, size_t samples);
    void addBassPCM(const float* data, size_t samples);
    void addDrumsPCM(const float* data, size_t samples);
    
    // Preset management
    QString currentPreset() const { return m_currentPreset; }
    QStringList presetList() const;
    
    Q_INVOKABLE void loadPreset(const QString& presetPath);
    Q_INVOKABLE void loadPresetByIndex(int index);
    Q_INVOKABLE void nextPreset();
    Q_INVOKABLE void previousPreset();
    Q_INVOKABLE void randomPreset();
    Q_INVOKABLE void lockPreset(bool lock);
    
    // Settings
    int fps() const { return m_fps; }
    void setFps(int fps);
    
    void setBeatSensitivity(float sensitivity);
    void setAspectCorrection(bool enabled);
    
    int width() const { return m_width; }
    int height() const { return m_height; }

signals:
    void initializedChanged();
    void presetChanged(const QString& presetName);
    void fpsChanged(int fps);
    void frameReady();
    void errorOccurred(const QString& error);

private:
    bool initializeOpenGLContext();
    void cleanupOpenGLContext();
    
    bool m_initialized = false;
    projectm_handle* m_projectM = nullptr;
    
    QString m_currentPreset;
    QString m_presetDirectory;
    int m_currentPresetIndex = 0;
    
    int m_width = 1920;
    int m_height = 1080;
    int m_fps = 60;
    
    float m_beatSensitivity = 1.0f;
    bool m_presetLocked = false;
    
    // OpenGL context resources
    unsigned int m_framebuffer = 0;
    unsigned int m_renderTexture = 0;
};

} // namespace suno::visualizer
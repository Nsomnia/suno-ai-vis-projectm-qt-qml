#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QImage>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <memory>
#include <vector>

// libprojectM v4 headers
#include <projectM-4/core.h>
#include <projectM-4/audio.h>
#include <projectM-4/parameters.h>
#include <projectM-4/render_opengl.h>
#include <projectM-4/playlist_core.h>
#include <projectM-4/playlist_items.h>
#include <projectM-4/playlist_playback.h>
#include <projectM-4/playlist_memory.h>

namespace suno::visualizer {

class ProjectMWrapper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool initialized READ isInitialized NOTIFY initializedChanged)
    Q_PROPERTY(QString currentPreset READ currentPreset NOTIFY presetChanged)
    Q_PROPERTY(int fps READ fps WRITE setFps NOTIFY fpsChanged)
    Q_PROPERTY(int presetCount READ presetCount NOTIFY presetsLoaded)
    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY presetChanged)

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
    void addPCMDataInterleaved(const float* data, size_t samples);
    
    // Stem-specific audio routing
    void addVocalsPCM(const float* data, size_t samples);
    void addBassPCM(const float* data, size_t samples);
    void addDrumsPCM(const float* data, size_t samples);
    
    // Preset management
    QString currentPreset() const { return m_currentPreset; }
    int presetCount() const { return m_presetCount; }
    int currentIndex() const { return m_currentPresetIndex; }
    QStringList presetList() const;
    
    Q_INVOKABLE void loadPreset(const QString& presetPath);
    Q_INVOKABLE void loadPresetByIndex(int index);
    Q_INVOKABLE void nextPreset(bool smooth = true);
    Q_INVOKABLE void previousPreset(bool smooth = true);
    Q_INVOKABLE void randomPreset();
    Q_INVOKABLE void lockPreset(bool lock);
    
    Q_INVOKABLE void addPresetPath(const QString& path, bool recursive = true);
    
    // Settings
    int fps() const { return m_fps; }
    void setFps(int fps);
    
    void setBeatSensitivity(float sensitivity);
    void setAspectCorrection(bool enabled);
    void setPresetDuration(float seconds);
    void setSoftCutDuration(float seconds);
    
    int width() const { return m_width; }
    int height() const { return m_height; }

signals:
    void initializedChanged();
    void presetChanged(const QString& presetName);
    void fpsChanged(int fps);
    void frameReady();
    void errorOccurred(const QString& error);
    void presetsLoaded(int count);

private:
    bool initializeOpenGLContext();
    void cleanupOpenGLContext();
    void setupDefaultParameters();
    
    bool m_initialized = false;
    projectm_handle m_projectM = nullptr;
    projectm_playlist_handle m_playlist = nullptr;
    
    QString m_currentPreset;
    QString m_presetDirectory;
    int m_currentPresetIndex = 0;
    int m_presetCount = 0;
    
    int m_width = 1920;
    int m_height = 1080;
    int m_fps = 60;
    
    float m_beatSensitivity = 1.0f;
    bool m_presetLocked = false;
    
    // OpenGL context resources
    unsigned int m_framebuffer = 0;
    unsigned int m_renderTexture = 0;
    unsigned int m_depthBuffer = 0;
    
    QOffscreenSurface* m_offscreenSurface = nullptr;
    QOpenGLContext* m_glContext = nullptr;
};

} // namespace suno::visualizer
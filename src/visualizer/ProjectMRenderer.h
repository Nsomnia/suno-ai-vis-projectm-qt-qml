#pragma once

#include <QQuickFramebufferObject>
#include <QOpenGLFunctions>
#include <QObject>
#include <QString>
#include <QTimer>
#include <memory>
#include <vector>

#include <projectM-4/core.h>
#include <projectM-4/audio.h>
#include <projectM-4/parameters.h>
#include <projectM-4/render_opengl.h>
#include <projectM-4/playlist_core.h>
#include <projectM-4/playlist_items.h>
#include <projectM-4/playlist_playback.h>
#include <projectM-4/playlist_memory.h>

namespace suno::visualizer {

class ProjectMRenderer;

class ProjectMRenderNode : public QQuickFramebufferObject::Renderer
{
public:
    ProjectMRenderNode();
    ~ProjectMRenderNode() override;

    void render() override;
    QOpenGLFramebufferObject* createFramebufferObject(const QSize& size) override;

    void setProjectMInstance(projectm_handle instance) { m_projectM = instance; }
    projectm_handle projectMInstance() const { return m_projectM; }

private:
    projectm_handle m_projectM = nullptr;
    bool m_initialized = false;
};

class ProjectMRenderer : public QQuickFramebufferObject
{
    Q_OBJECT
    Q_PROPERTY(bool initialized READ isInitialized NOTIFY initializedChanged)
    Q_PROPERTY(QString currentPreset READ currentPreset NOTIFY presetChanged)
    Q_PROPERTY(int presetCount READ presetCount NOTIFY presetsLoaded)
    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY presetChanged)
    Q_PROPERTY(int fps READ fps WRITE setFps NOTIFY fpsChanged)

public:
    explicit ProjectMRenderer(QQuickItem* parent = nullptr);
    ~ProjectMRenderer() override;

    Renderer* createRenderer() const override;

    bool isInitialized() const { return m_initialized; }
    QString currentPreset() const { return m_currentPreset; }
    int presetCount() const { return m_presetCount; }
    int currentIndex() const { return m_currentPresetIndex; }
    int fps() const { return m_fps; }
    void setFps(int fps);

    Q_INVOKABLE void loadPreset(const QString& presetPath);
    Q_INVOKABLE void loadPresetByIndex(int index);
    Q_INVOKABLE void nextPreset();
    Q_INVOKABLE void previousPreset();
    Q_INVOKABLE void randomPreset();
    Q_INVOKABLE void lockPreset(bool lock);

    void addPCMData(const float* data, size_t samples, bool stereo = false);
    void addPCMDataStereo(const float* left, const float* right, size_t samples);

    projectm_handle projectMInstance() const { return m_projectM; }

signals:
    void initializedChanged();
    void presetChanged(const QString& presetName);
    void fpsChanged(int fps);
    void presetsLoaded(int count);
    void errorOccurred(const QString& error);

protected:
    void geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) override;

private:
    void initializeProjectM();
    void loadPresets();
    void setupParameters();

    bool m_initialized = false;
    projectm_handle m_projectM = nullptr;
    projectm_playlist_handle m_playlist = nullptr;

    QString m_currentPreset;
    int m_currentPresetIndex = 0;
    int m_presetCount = 0;
    int m_fps = 60;

    std::vector<QString> m_presetPaths;
    bool m_presetLocked = false;

    static constexpr int DEFAULT_WIDTH = 1920;
    static constexpr int DEFAULT_HEIGHT = 1080;
};

} // namespace suno::visualizer

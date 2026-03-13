#include "ProjectMRenderer.h"
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLFramebufferObject>
#include <QStandardPaths>
#include <QDir>
#include <QRandomGenerator>
#include <QFileInfo>
#include <spdlog/spdlog.h>

namespace suno::visualizer {

ProjectMRenderNode::ProjectMRenderNode()
{
    spdlog::debug("ProjectMRenderNode created");
}

ProjectMRenderNode::~ProjectMRenderNode()
{
    spdlog::debug("ProjectMRenderNode destroyed");
}

QOpenGLFramebufferObject* ProjectMRenderNode::createFramebufferObject(const QSize& size)
{
    Q_UNUSED(size)
    return new QOpenGLFramebufferObject(size, QOpenGLFramebufferObject::CombinedDepthStencil);
}

void ProjectMRenderNode::render()
{
    if (!m_projectM) {
        return;
    }

    QOpenGLFunctions* gl = QOpenGLContext::currentContext()->functions();
    gl->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    projectm_opengl_render_frame(m_projectM);

    m_initialized = true;
}

ProjectMRenderer::ProjectMRenderer(QQuickItem* parent)
: QQuickFramebufferObject(parent)
{
    setFlag(ItemHasContents, true);
    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptHoverEvents(true);

    spdlog::debug("ProjectMRenderer created");
}

ProjectMRenderer::~ProjectMRenderer()
{
    if (m_playlist) {
        projectm_playlist_destroy(m_playlist);
    }
    if (m_projectM) {
        projectm_destroy(m_projectM);
    }
    spdlog::debug("ProjectMRenderer destroyed");
}

QQuickFramebufferObject::Renderer* ProjectMRenderer::createRenderer() const
{
    auto* renderer = new ProjectMRenderNode();
    if (m_projectM) {
        renderer->setProjectMInstance(m_projectM);
    }
    return renderer;
}

void ProjectMRenderer::geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry)
{
    QQuickFramebufferObject::geometryChange(newGeometry, oldGeometry);

    if (m_projectM && newGeometry.size() != oldGeometry.size()) {
        int width = static_cast<int>(newGeometry.width());
        int height = static_cast<int>(newGeometry.height());
        if (width > 0 && height > 0) {
            projectm_set_window_size(m_projectM, static_cast<size_t>(width), static_cast<size_t>(height));
            spdlog::debug("ProjectM resized to {}x{}", width, height);
        }
    }
}

void ProjectMRenderer::initializeProjectM()
{
    if (m_projectM) {
        return;
    }

    spdlog::info("Initializing projectM...");

    m_projectM = projectm_create();
    if (!m_projectM) {
        spdlog::error("Failed to create projectM instance");
        emit errorOccurred("Failed to create projectM instance");
        return;
    }

    m_playlist = projectm_playlist_create(m_projectM);
    if (!m_playlist) {
        spdlog::error("Failed to create projectM playlist");
        projectm_destroy(m_projectM);
        m_projectM = nullptr;
        emit errorOccurred("Failed to create playlist");
        return;
    }

    setupParameters();
    loadPresets();

    m_initialized = true;
    emit initializedChanged();
    emit presetsLoaded(m_presetCount);

    if (m_presetCount > 0) {
        loadPresetByIndex(0);
    }

    spdlog::info("ProjectM initialized with {} presets", m_presetCount);
}

void ProjectMRenderer::setupParameters()
{
    if (!m_projectM) return;

    projectm_set_fps(m_projectM, m_fps);
    projectm_set_mesh_size(m_projectM, 220, 125);
    projectm_set_aspect_correction(m_projectM, true);
    projectm_set_preset_duration(m_projectM, 30.0);
    projectm_set_soft_cut_duration(m_projectM, 3.0);
    projectm_set_hard_cut_enabled(m_projectM, true);
    projectm_set_hard_cut_duration(m_projectM, 20.0);
    projectm_set_hard_cut_sensitivity(m_projectM, 1.0f);
    projectm_set_beat_sensitivity(m_projectM, 1.0f);

    QStringList texturePaths = {
        "/usr/share/projectM/textures",
        "/usr/local/share/projectM/textures"
    };

    std::vector<QByteArray> pathStorage;
    std::vector<const char*> pathPtrs;
    for (const QString& p : texturePaths) {
        if (QDir(p).exists()) {
            pathStorage.push_back(p.toUtf8());
            pathPtrs.push_back(pathStorage.back().constData());
        }
    }
    if (!pathPtrs.empty()) {
        projectm_set_texture_search_paths(m_projectM, pathPtrs.data(), pathPtrs.size());
    }
}

void ProjectMRenderer::loadPresets()
{
    if (!m_playlist) return;

    QStringList presetDirs = {
        "/usr/share/projectM/presets",
        "/usr/share/projectM/presets/presets_projectM",
        "/usr/share/projectM/presets/presets_milkdrop",
        "/usr/share/projectM/presets/presets_stock",
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/presets"
    };

    for (const QString& dir : presetDirs) {
        if (QDir(dir).exists()) {
            QByteArray pathUtf8 = dir.toUtf8();
            uint32_t added = projectm_playlist_add_path(m_playlist, pathUtf8.constData(), 1, 0);
            spdlog::debug("Added {} presets from: {}", added, dir.toStdString());
        }
    }

    m_presetCount = static_cast<int>(projectm_playlist_size(m_playlist));
}

void ProjectMRenderer::setFps(int fps)
{
    if (m_fps != fps) {
        m_fps = fps;
        if (m_projectM) {
            projectm_set_fps(m_projectM, m_fps);
        }
        emit fpsChanged(fps);
    }
}

void ProjectMRenderer::loadPreset(const QString& presetPath)
{
    if (!m_projectM) return;

    QByteArray pathUtf8 = presetPath.toUtf8();
    projectm_load_preset_file(m_projectM, pathUtf8.constData(), true);

    m_currentPreset = QFileInfo(presetPath).baseName();
    emit presetChanged(m_currentPreset);
}

void ProjectMRenderer::loadPresetByIndex(int index)
{
    if (!m_playlist || m_presetCount == 0) return;

    index = qBound(0, index, m_presetCount - 1);
    m_currentPresetIndex = index;

    char* item = projectm_playlist_item(m_playlist, static_cast<uint32_t>(index));
    if (item) {
        QString presetPath = QString::fromUtf8(item);
        projectm_playlist_free_string(item);

        projectm_playlist_set_position(m_playlist, static_cast<uint32_t>(index), 1);
        loadPreset(presetPath);
    }
}

void ProjectMRenderer::nextPreset()
{
    if (!m_playlist || m_presetCount == 0) return;

    m_currentPresetIndex = (m_currentPresetIndex + 1) % m_presetCount;
    loadPresetByIndex(m_currentPresetIndex);
}

void ProjectMRenderer::previousPreset()
{
    if (!m_playlist || m_presetCount == 0) return;

    m_currentPresetIndex = (m_currentPresetIndex - 1 + m_presetCount) % m_presetCount;
    loadPresetByIndex(m_currentPresetIndex);
}

void ProjectMRenderer::randomPreset()
{
    if (!m_playlist || m_presetCount == 0) return;

    int randomIndex = QRandomGenerator::global()->bounded(m_presetCount);
    loadPresetByIndex(randomIndex);
}

void ProjectMRenderer::lockPreset(bool lock)
{
    m_presetLocked = lock;
    if (m_projectM) {
        projectm_set_preset_locked(m_projectM, lock);
    }
}

void ProjectMRenderer::addPCMData(const float* data, size_t samples, bool stereo)
{
    if (!m_projectM || !data || samples == 0) return;

    projectm_pcm_add_float(m_projectM, data, static_cast<uint32_t>(samples),
                           stereo ? PROJECTM_STEREO : PROJECTM_MONO);
}

void ProjectMRenderer::addPCMDataStereo(const float* left, const float* right, size_t samples)
{
    if (!m_projectM || !left || !right || samples == 0) return;

    std::vector<float> interleaved(samples * 2);
    for (size_t i = 0; i < samples; ++i) {
        interleaved[i * 2] = left[i];
        interleaved[i * 2 + 1] = right[i];
    }

    projectm_pcm_add_float(m_projectM, interleaved.data(), static_cast<uint32_t>(samples), PROJECTM_STEREO);
}

} // namespace suno::visualizer

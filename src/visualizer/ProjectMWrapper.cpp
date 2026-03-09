#include "ProjectMWrapper.h"
#include <spdlog/spdlog.h>
#include <QStandardPaths>
#include <QDir>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QRandomGenerator>
#include <QOpenGLFunctions>

namespace suno::visualizer {

ProjectMWrapper::ProjectMWrapper(QObject* parent)
    : QObject(parent)
{
    m_presetDirectory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                       + "/presets";
    
    spdlog::debug("ProjectMWrapper created, preset directory: {}", 
                 m_presetDirectory.toStdString());
}

ProjectMWrapper::~ProjectMWrapper()
{
    shutdown();
}

bool ProjectMWrapper::initialize(int width, int height)
{
    if (m_initialized) {
        spdlog::warn("ProjectMWrapper already initialized");
        return true;
    }
    
    spdlog::info("Initializing projectM visualizer: {}x{}", width, height);
    
    m_width = width;
    m_height = height;
    
    // Initialize OpenGL context for offscreen rendering
    if (!initializeOpenGLContext()) {
        spdlog::error("Failed to initialize OpenGL context");
        emit errorOccurred("OpenGL initialization failed");
        return false;
    }
    
    // Create projectM instance
    m_projectM = projectm_create();
    if (!m_projectM) {
        spdlog::error("Failed to create projectM instance");
        cleanupOpenGLContext();
        emit errorOccurred("projectM creation failed");
        return false;
    }
    
    // Create playlist
    m_playlist = projectm_playlist_create(m_projectM);
    if (!m_playlist) {
        spdlog::error("Failed to create projectM playlist");
        projectm_destroy(m_projectM);
        m_projectM = nullptr;
        cleanupOpenGLContext();
        emit errorOccurred("Playlist creation failed");
        return false;
    }
    
    // Set default parameters
    setupDefaultParameters();
    
    // Set window size
    projectm_set_window_size(m_projectM, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    
    // Set texture search paths
    QStringList texturePaths = {
        "/usr/share/projectM/textures",
        "/usr/local/share/projectM/textures",
        m_presetDirectory + "/textures"
    };
    
    std::vector<const char*> pathPtrs;
    std::vector<QByteArray> pathStorage;
    for (const QString& p : texturePaths) {
        if (QDir(p).exists()) {
            pathStorage.push_back(p.toUtf8());
            pathPtrs.push_back(pathStorage.back().constData());
        }
    }
    if (!pathPtrs.empty()) {
        projectm_set_texture_search_paths(m_projectM, pathPtrs.data(), static_cast<uint32_t>(pathPtrs.size()));
    }
    
    // Add preset paths
    addPresetPath(m_presetDirectory, false);
    addPresetPath("/usr/share/projectM/presets", true);
    addPresetPath("/usr/local/share/projectM/presets", true);
    
    // Create preset directory if it doesn't exist
    QDir presetDir(m_presetDirectory);
    if (!presetDir.exists()) {
        presetDir.mkpath(".");
        spdlog::info("Created preset directory: {}", m_presetDirectory.toStdString());
    }
    
    m_presetCount = static_cast<int>(projectm_playlist_size(m_playlist));
    spdlog::info("Loaded {} presets from playlist", m_presetCount);
    
    m_initialized = true;
    emit initializedChanged();
    emit presetsLoaded(m_presetCount);
    
    // Load first preset if available
    if (m_presetCount > 0) {
        loadPresetByIndex(0);
    }
    
    spdlog::info("ProjectM visualizer initialized successfully");
    
    return true;
}

void ProjectMWrapper::setupDefaultParameters()
{
    projectm_set_fps(m_projectM, static_cast<uint32_t>(m_fps));
    projectm_set_mesh_size(m_projectM, 220, 125);
    projectm_set_aspect_correction(m_projectM, true);
    projectm_set_preset_duration(m_projectM, 30.0f);
    projectm_set_soft_cut_duration(m_projectM, 3.0f);
    projectm_set_hard_cut_enabled(m_projectM, true);
    projectm_set_hard_cut_duration(m_projectM, 20.0f);
    projectm_set_hard_cut_sensitivity(m_projectM, 1.0f);
    projectm_set_beat_sensitivity(m_projectM, m_beatSensitivity);
}

void ProjectMWrapper::shutdown()
{
    if (!m_initialized) {
        return;
    }
    
    spdlog::info("Shutting down projectM visualizer");
    
    if (m_playlist) {
        projectm_playlist_destroy(m_playlist);
        m_playlist = nullptr;
    }
    
    if (m_projectM) {
        projectm_destroy(m_projectM);
        m_projectM = nullptr;
    }
    
    cleanupOpenGLContext();
    
    m_initialized = false;
    m_presetCount = 0;
    emit initializedChanged();
}

bool ProjectMWrapper::initializeOpenGLContext()
{
    // Create OpenGL context
    m_glContext = new QOpenGLContext(this);
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setAlphaBufferSize(8);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    m_glContext->setFormat(format);
    
    if (!m_glContext->create()) {
        spdlog::error("Failed to create OpenGL context");
        return false;
    }
    
    // Create offscreen surface
    m_offscreenSurface = new QOffscreenSurface(nullptr, this);
    m_offscreenSurface->setFormat(format);
    m_offscreenSurface->create();
    
    if (!m_offscreenSurface->isValid()) {
        spdlog::error("Failed to create offscreen surface");
        return false;
    }
    
    // Make context current
    if (!m_glContext->makeCurrent(m_offscreenSurface)) {
        spdlog::error("Failed to make OpenGL context current");
        return false;
    }
    
    // Create FBO for offscreen rendering
    QOpenGLFunctions gl(m_glContext);
    gl.glGenFramebuffers(1, &m_framebuffer);
    gl.glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    
    // Create render texture
    gl.glGenTextures(1, &m_renderTexture);
    gl.glBindTexture(GL_TEXTURE_2D, m_renderTexture);
    gl.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_renderTexture, 0);
    
    // Create depth buffer
    gl.glGenRenderbuffers(1, &m_depthBuffer);
    gl.glBindRenderbuffer(GL_RENDERBUFFER, m_depthBuffer);
    gl.glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, m_width, m_height);
    gl.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthBuffer);
    
    GLenum status = gl.glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        spdlog::error("Framebuffer not complete: {}", status);
        return false;
    }
    
    gl.glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    spdlog::debug("OpenGL FBO created: {}x{}", m_width, m_height);
    return true;
}

void ProjectMWrapper::cleanupOpenGLContext()
{
    if (m_glContext && m_glContext->isValid()) {
        m_glContext->makeCurrent(m_offscreenSurface);
        QOpenGLFunctions gl(m_glContext);
        
        if (m_framebuffer) {
            gl.glDeleteFramebuffers(1, &m_framebuffer);
            m_framebuffer = 0;
        }
        if (m_renderTexture) {
            gl.glDeleteTextures(1, &m_renderTexture);
            m_renderTexture = 0;
        }
        if (m_depthBuffer) {
            gl.glDeleteRenderbuffers(1, &m_depthBuffer);
            m_depthBuffer = 0;
        }
    }
    
    delete m_offscreenSurface;
    m_offscreenSurface = nullptr;
    
    delete m_glContext;
    m_glContext = nullptr;
}

void ProjectMWrapper::renderFrame()
{
    if (!m_initialized || !m_projectM) {
        return;
    }
    
    m_glContext->makeCurrent(m_offscreenSurface);
    
    QOpenGLFunctions gl(m_glContext);
    gl.glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    gl.glViewport(0, 0, m_width, m_height);
    
    projectm_opengl_render_frame(m_projectM);
    
    gl.glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    emit frameReady();
}

QImage ProjectMWrapper::grabFrameBuffer()
{
    if (!m_initialized || !m_projectM) {
        return QImage();
    }
    
    m_glContext->makeCurrent(m_offscreenSurface);
    
    QOpenGLFunctions gl(m_glContext);
    gl.glBindFramebuffer(GL_READ_FRAMEBUFFER, m_framebuffer);
    
    QImage frame(m_width, m_height, QImage::Format_RGBA8888);
    gl.glReadPixels(0, 0, m_width, m_height, GL_RGBA, GL_UNSIGNED_BYTE, frame.bits());
    gl.glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    
    // OpenGL images are bottom-up, flip vertically
    frame = frame.flipped(Qt::Vertical);
    
    return frame;
}

void ProjectMWrapper::resize(int width, int height)
{
    if (width == m_width && height == m_height) {
        return;
    }
    
    m_width = width;
    m_height = height;
    
    if (m_initialized && m_projectM) {
        projectm_set_window_size(m_projectM, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
        
        // Recreate framebuffer at new size
        cleanupOpenGLContext();
        initializeOpenGLContext();
    }
    
    spdlog::info("ProjectM resized to {}x{}", width, height);
}

void ProjectMWrapper::addPCMData(const float* data, size_t samples)
{
    if (!m_initialized || !m_projectM || !data || samples == 0) {
        return;
    }
    
    projectm_pcm_add_float(m_projectM, data, static_cast<uint32_t>(samples), PROJECTM_MONO);
}

void ProjectMWrapper::addPCMDataStereo(const float* left, const float* right, size_t samples)
{
    if (!m_initialized || !m_projectM || !left || !right || samples == 0) {
        return;
    }
    
    // Interleave stereo data
    std::vector<float> interleaved(samples * 2);
    for (size_t i = 0; i < samples; ++i) {
        interleaved[i * 2] = left[i];
        interleaved[i * 2 + 1] = right[i];
    }
    projectm_pcm_add_float(m_projectM, interleaved.data(), static_cast<uint32_t>(samples), PROJECTM_STEREO);
}

void ProjectMWrapper::addPCMDataInterleaved(const float* data, size_t samples)
{
    if (!m_initialized || !m_projectM || !data || samples == 0) {
        return;
    }
    
    projectm_pcm_add_float(m_projectM, data, static_cast<uint32_t>(samples), PROJECTM_STEREO);
}

void ProjectMWrapper::addVocalsPCM(const float* data, size_t samples)
{
    addPCMData(data, samples);
}

void ProjectMWrapper::addBassPCM(const float* data, size_t samples)
{
    addPCMData(data, samples);
}

void ProjectMWrapper::addDrumsPCM(const float* data, size_t samples)
{
    addPCMData(data, samples);
}

QStringList ProjectMWrapper::presetList() const
{
    QStringList result;
    if (!m_playlist) {
        return result;
    }
    
    uint32_t count = projectm_playlist_size(m_playlist);
    for (uint32_t i = 0; i < count; ++i) {
        char* item = projectm_playlist_item(m_playlist, i);
        if (item) {
            QString presetPath = QString::fromUtf8(item);
            QFileInfo fi(presetPath);
            result.append(fi.baseName());
            projectm_playlist_free_string(item);
        }
    }
    return result;
}

void ProjectMWrapper::addPresetPath(const QString& path, bool recursive)
{
    if (!m_playlist || !QDir(path).exists()) {
        return;
    }
    
    QByteArray pathUtf8 = path.toUtf8();
    uint32_t added = projectm_playlist_add_path(m_playlist, pathUtf8.constData(), recursive ? 1 : 0, 0);
    spdlog::debug("Added {} presets from path: {}", added, path.toStdString());
    
    m_presetCount = static_cast<int>(projectm_playlist_size(m_playlist));
}

void ProjectMWrapper::loadPreset(const QString& presetPath)
{
    if (!m_initialized || !m_projectM) {
        spdlog::warn("Cannot load preset - visualizer not initialized");
        return;
    }
    
    spdlog::info("Loading preset: {}", presetPath.toStdString());
    
    QByteArray pathUtf8 = presetPath.toUtf8();
    bool smooth = true;
    projectm_load_preset_file(m_projectM, pathUtf8.constData(), smooth ? 1 : 0);
    
    m_currentPreset = QFileInfo(presetPath).baseName();
    emit presetChanged(m_currentPreset);
}

void ProjectMWrapper::loadPresetByIndex(int index)
{
    if (!m_playlist) {
        return;
    }
    
    uint32_t count = projectm_playlist_size(m_playlist);
    if (count == 0) {
        spdlog::warn("No presets available");
        return;
    }
    
    index = qBound(0, index, static_cast<int>(count) - 1);
    m_currentPresetIndex = index;
    
    char* item = projectm_playlist_item(m_playlist, static_cast<uint32_t>(index));
    if (item) {
        QString presetPath = QString::fromUtf8(item);
        projectm_playlist_free_string(item);
        
        projectm_playlist_set_position(m_playlist, static_cast<uint32_t>(index), 1);
        loadPreset(presetPath);
    }
}

void ProjectMWrapper::nextPreset(bool smooth)
{
    if (!m_playlist || m_presetCount == 0) {
        return;
    }
    
    m_currentPresetIndex = (m_currentPresetIndex + 1) % m_presetCount;
    projectm_playlist_play_next(m_playlist, !smooth);
    
    char* item = projectm_playlist_item(m_playlist, static_cast<uint32_t>(m_currentPresetIndex));
    if (item) {
        m_currentPreset = QFileInfo(QString::fromUtf8(item)).baseName();
        projectm_playlist_free_string(item);
        emit presetChanged(m_currentPreset);
    }
}

void ProjectMWrapper::previousPreset(bool smooth)
{
    if (!m_playlist || m_presetCount == 0) {
        return;
    }
    
    m_currentPresetIndex = (m_currentPresetIndex - 1 + m_presetCount) % m_presetCount;
    projectm_playlist_play_previous(m_playlist, !smooth);
    
    char* item = projectm_playlist_item(m_playlist, static_cast<uint32_t>(m_currentPresetIndex));
    if (item) {
        m_currentPreset = QFileInfo(QString::fromUtf8(item)).baseName();
        projectm_playlist_free_string(item);
        emit presetChanged(m_currentPreset);
    }
}

void ProjectMWrapper::randomPreset()
{
    if (!m_playlist || m_presetCount == 0) {
        return;
    }
    
    int randomIndex = QRandomGenerator::global()->bounded(m_presetCount);
    loadPresetByIndex(randomIndex);
}

void ProjectMWrapper::lockPreset(bool lock)
{
    m_presetLocked = lock;
    
    if (m_projectM) {
        projectm_set_preset_locked(m_projectM, lock ? 1 : 0);
    }
    
    spdlog::debug("Preset lock: {}", lock ? "enabled" : "disabled");
}

void ProjectMWrapper::setFps(int fps)
{
    if (m_fps != fps) {
        m_fps = fps;
        
        if (m_projectM) {
            projectm_set_fps(m_projectM, static_cast<uint32_t>(fps));
        }
        
        emit fpsChanged(fps);
        spdlog::debug("Visualizer FPS set to {}", fps);
    }
}

void ProjectMWrapper::setBeatSensitivity(float sensitivity)
{
    m_beatSensitivity = qBound(0.0f, sensitivity, 5.0f);
    
    if (m_projectM) {
        projectm_set_beat_sensitivity(m_projectM, m_beatSensitivity);
    }
    
    spdlog::debug("Beat sensitivity set to {}", m_beatSensitivity);
}

void ProjectMWrapper::setAspectCorrection(bool enabled)
{
    if (m_projectM) {
        projectm_set_aspect_correction(m_projectM, enabled ? 1 : 0);
    }
    
    spdlog::debug("Aspect correction: {}", enabled ? "enabled" : "disabled");
}

void ProjectMWrapper::setPresetDuration(float seconds)
{
    if (m_projectM) {
        projectm_set_preset_duration(m_projectM, seconds);
    }
    spdlog::debug("Preset duration set to {} seconds", seconds);
}

void ProjectMWrapper::setSoftCutDuration(float seconds)
{
    if (m_projectM) {
        projectm_set_soft_cut_duration(m_projectM, seconds);
    }
    spdlog::debug("Soft cut duration set to {} seconds", seconds);
}

} // namespace suno::visualizer
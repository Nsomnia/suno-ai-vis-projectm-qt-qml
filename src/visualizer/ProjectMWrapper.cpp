#include "ProjectMWrapper.h"
#include <spdlog/spdlog.h>
#include <QStandardPaths>
#include <QDir>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QRandomGenerator>

// Note: Actual libprojectM v4.1.x headers would be included here
// This is a skeleton implementation showing the intended interface

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
    
    // Create preset directory if it doesn't exist
    QDir presetDir(m_presetDirectory);
    if (!presetDir.exists()) {
        presetDir.mkpath(".");
        spdlog::info("Created preset directory: {}", m_presetDirectory.toStdString());
    }
    
    /*
     * libprojectM v4.1.x initialization would look something like:
     * 
     * projectm_settings settings;
     * settings.window_width = width;
     * settings.window_height = height;
     * settings.fps = m_fps;
     * settings.mesh_x = 220;
     * settings.mesh_y = 125;
     * settings.aspect_correction = true;
     * settings.preset_duration = 30.0;
     * settings.soft_cut_duration = 3.0;
     * settings.hard_cut_enabled = true;
     * settings.hard_cut_duration = 20.0;
     * settings.hard_cut_sensitivity = 1.0;
     * settings.beat_sensitivity = m_beatSensitivity;
     * settings.preset_url = m_presetDirectory.toStdString();
     * 
     * m_projectM = projectm_create_settings(&settings);
     * 
     * if (!m_projectM) {
     *     spdlog::error("Failed to create projectM instance");
     *     return false;
     * }
     */
    
    // Placeholder: Mark as initialized for UI development
    m_initialized = true;
    emit initializedChanged();
    
    spdlog::info("ProjectM visualizer initialized successfully");
    spdlog::info("Ready to render some sick visuals, as the kids say (they don't say that)");
    
    return true;
}

void ProjectMWrapper::shutdown()
{
    if (!m_initialized) {
        return;
    }
    
    spdlog::info("Shutting down projectM visualizer");
    
    /*
     * if (m_projectM) {
     *     projectm_destroy(m_projectM);
     *     m_projectM = nullptr;
     * }
     */
    
    cleanupOpenGLContext();
    
    m_initialized = false;
    emit initializedChanged();
}

bool ProjectMWrapper::initializeOpenGLContext()
{
    // In a real implementation, we'd create an offscreen OpenGL context
    // For now, this is a placeholder
    
    spdlog::debug("OpenGL context initialization (placeholder)");
    
    // Create framebuffer for offscreen rendering
    // glGenFramebuffers(1, &m_framebuffer);
    // glGenTextures(1, &m_renderTexture);
    // ... configure FBO ...
    
    return true;
}

void ProjectMWrapper::cleanupOpenGLContext()
{
    // if (m_framebuffer) {
    //     glDeleteFramebuffers(1, &m_framebuffer);
    //     m_framebuffer = 0;
    // }
    // if (m_renderTexture) {
    //     glDeleteTextures(1, &m_renderTexture);
    //     m_renderTexture = 0;
    // }
}

void ProjectMWrapper::renderFrame()
{
    if (!m_initialized) {
        return;
    }
    
    /*
     * projectm_render_frame(m_projectM);
     */
    
    emit frameReady();
}

QImage ProjectMWrapper::grabFrameBuffer()
{
    if (!m_initialized) {
        return QImage();
    }
    
    // Read pixels from OpenGL framebuffer
    QImage frame(m_width, m_height, QImage::Format_RGBA8888);
    
    /*
     * glBindFramebuffer(GL_READ_FRAMEBUFFER, m_framebuffer);
     * glReadPixels(0, 0, m_width, m_height, GL_RGBA, GL_UNSIGNED_BYTE, frame.bits());
     * 
     * // OpenGL images are bottom-up, flip vertically
     * frame = frame.mirrored(false, true);
     */
    
    // Placeholder: fill with gradient for visual feedback during development
    for (int y = 0; y < m_height; ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(frame.scanLine(y));
        for (int x = 0; x < m_width; ++x) {
            int r = (x * 255) / m_width;
            int g = (y * 255) / m_height;
            int b = 128;
            line[x] = qRgba(r, g, b, 255);
        }
    }
    
    return frame;
}

void ProjectMWrapper::resize(int width, int height)
{
    if (width == m_width && height == m_height) {
        return;
    }
    
    m_width = width;
    m_height = height;
    
    if (m_initialized) {
        /*
         * projectm_set_window_size(m_projectM, width, height);
         */
        
        // Recreate framebuffer at new size
        // ...
    }
    
    spdlog::info("ProjectM resized to {}x{}", width, height);
}

void ProjectMWrapper::addPCMData(const float* data, size_t samples)
{
    if (!m_initialized || !data || samples == 0) {
        return;
    }
    
    /*
     * projectm_pcm_add_float(m_projectM, data, samples, PROJECTM_MONO);
     */
}

void ProjectMWrapper::addPCMDataStereo(const float* left, const float* right, size_t samples)
{
    if (!m_initialized || !left || !right || samples == 0) {
        return;
    }
    
    /*
     * // Interleave stereo data
     * std::vector<float> interleaved(samples * 2);
     * for (size_t i = 0; i < samples; ++i) {
     *     interleaved[i * 2] = left[i];
     *     interleaved[i * 2 + 1] = right[i];
     * }
     * projectm_pcm_add_float(m_projectM, interleaved.data(), samples, PROJECTM_STEREO);
     */
}

void ProjectMWrapper::addVocalsPCM(const float* data, size_t samples)
{
    // Route vocals to specific frequency bands for visualization
    // This would be used for lyric-reactive effects
    addPCMData(data, samples);
}

void ProjectMWrapper::addBassPCM(const float* data, size_t samples)
{
    // Route bass for beat-reactive visuals
    addPCMData(data, samples);
}

void ProjectMWrapper::addDrumsPCM(const float* data, size_t samples)
{
    // Route drums for rhythmic pulses
    addPCMData(data, samples);
}

QStringList ProjectMWrapper::presetList() const
{
    QDir presetDir(m_presetDirectory);
    QStringList filters;
    filters << "*.milk" << "*.prjm";
    
    return presetDir.entryList(filters, QDir::Files, QDir::Name);
}

void ProjectMWrapper::loadPreset(const QString& presetPath)
{
    if (!m_initialized) {
        spdlog::warn("Cannot load preset - visualizer not initialized");
        return;
    }
    
    spdlog::info("Loading preset: {}", presetPath.toStdString());
    
    /*
     * bool smooth = true;
     * projectm_load_preset_file(m_projectM, presetPath.toUtf8().constData(), smooth);
     */
    
    m_currentPreset = QFileInfo(presetPath).baseName();
    emit presetChanged(m_currentPreset);
}

void ProjectMWrapper::loadPresetByIndex(int index)
{
    QStringList presets = presetList();
    
    if (presets.isEmpty()) {
        spdlog::warn("No presets available");
        return;
    }
    
    index = qBound(0, index, presets.size() - 1);
    m_currentPresetIndex = index;
    
    QString presetPath = m_presetDirectory + "/" + presets[index];
    loadPreset(presetPath);
}

void ProjectMWrapper::nextPreset()
{
    QStringList presets = presetList();
    if (presets.isEmpty()) return;
    
    m_currentPresetIndex = (m_currentPresetIndex + 1) % presets.size();
    loadPresetByIndex(m_currentPresetIndex);
}

void ProjectMWrapper::previousPreset()
{
    QStringList presets = presetList();
    if (presets.isEmpty()) return;
    
    m_currentPresetIndex = (m_currentPresetIndex - 1 + presets.size()) % presets.size();
    loadPresetByIndex(m_currentPresetIndex);
}

void ProjectMWrapper::randomPreset()
{
    QStringList presets = presetList();
    if (presets.isEmpty()) return;
    
    int randomIndex = QRandomGenerator::global()->bounded(presets.size());
    loadPresetByIndex(randomIndex);
}

void ProjectMWrapper::lockPreset(bool lock)
{
    m_presetLocked = lock;
    
    /*
     * projectm_set_preset_locked(m_projectM, lock);
     */
    
    spdlog::debug("Preset lock: {}", lock ? "enabled" : "disabled");
}

void ProjectMWrapper::setFps(int fps)
{
    if (m_fps != fps) {
        m_fps = fps;
        emit fpsChanged(fps);
        spdlog::debug("Visualizer FPS set to {}", fps);
    }
}

void ProjectMWrapper::setBeatSensitivity(float sensitivity)
{
    m_beatSensitivity = qBound(0.0f, sensitivity, 5.0f);
    
    /*
     * projectm_set_beat_sensitivity(m_projectM, m_beatSensitivity);
     */
    
    spdlog::debug("Beat sensitivity set to {}", m_beatSensitivity);
}

void ProjectMWrapper::setAspectCorrection(bool enabled)
{
    /*
     * projectm_set_aspect_correction(m_projectM, enabled);
     */
    
    spdlog::debug("Aspect correction: {}", enabled ? "enabled" : "disabled");
}

} // namespace suno::visualizer
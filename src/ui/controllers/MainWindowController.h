#pragma once

#include <QObject>
#include <QString>
#include <memory>

#include "core/GitController.h"
#include "audio/AudioEngine.h"
#include "audio/StemProcessor.h"
#include "video/VideoCapture.h"
#include "video/FrameCompositor.h"
#include "visualizer/ProjectMWrapper.h"
#include "visualizer/PresetManager.h"
#include "network/SunoClient.h"
#include "network/GistUploader.h"
#include "ui/models/TimelineModel.h"

namespace suno::ui {

class MainWindowController : public QObject
{
    Q_OBJECT
    
    // Expose components to QML
    Q_PROPERTY(suno::audio::AudioEngine* audioEngine READ audioEngine CONSTANT)
    Q_PROPERTY(suno::audio::StemProcessor* stemProcessor READ stemProcessor CONSTANT)
    Q_PROPERTY(suno::video::VideoCapture* videoCapture READ videoCapture CONSTANT)
    Q_PROPERTY(suno::visualizer::ProjectMWrapper* visualizer READ visualizer CONSTANT)
    Q_PROPERTY(suno::visualizer::PresetManager* presetManager READ presetManager CONSTANT)
    Q_PROPERTY(suno::network::SunoClient* sunoClient READ sunoClient CONSTANT)
    Q_PROPERTY(suno::network::GistUploader* gistUploader READ gistUploader CONSTANT)
    Q_PROPERTY(TimelineModel* timelineModel READ timelineModel CONSTANT)
    
    // State properties
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY playbackStateChanged)
    Q_PROPERTY(bool isRecording READ isRecording NOTIFY recordingStateChanged)
    Q_PROPERTY(QString currentTrackTitle READ currentTrackTitle NOTIFY trackChanged)
    Q_PROPERTY(QString currentTrackArtist READ currentTrackArtist NOTIFY trackChanged)

public:
    explicit MainWindowController(QObject* parent = nullptr);
    ~MainWindowController() override;
    
    // Component accessors
    suno::audio::AudioEngine* audioEngine() const { return m_audioEngine.get(); }
    suno::audio::StemProcessor* stemProcessor() const { return m_stemProcessor.get(); }
    suno::video::VideoCapture* videoCapture() const { return m_videoCapture.get(); }
    suno::visualizer::ProjectMWrapper* visualizer() const { return m_visualizer.get(); }
    suno::visualizer::PresetManager* presetManager() const { return m_presetManager.get(); }
    suno::network::SunoClient* sunoClient() const { return m_sunoClient.get(); }
    suno::network::GistUploader* gistUploader() const { return m_gistUploader.get(); }
    TimelineModel* timelineModel() const { return m_timelineModel.get(); }
    
    bool isPlaying() const;
    bool isRecording() const;
    QString currentTrackTitle() const { return m_currentTrackTitle; }
    QString currentTrackArtist() const { return m_currentTrackArtist; }
    
    // Playback controls
    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void seek(qint64 positionMs);
    
    // File operations
    Q_INVOKABLE void loadAudioFile(const QString& filePath);
    Q_INVOKABLE void loadSunoTrack(const QString& trackId);
    
    // Recording controls
    Q_INVOKABLE void startRecording(const QString& outputPath);
    Q_INVOKABLE void stopRecording();
    
    // Project management
    Q_INVOKABLE void newProject();
    Q_INVOKABLE void openProject(const QString& projectPath);
    Q_INVOKABLE void saveProject();
    Q_INVOKABLE void saveProjectAs(const QString& projectPath);
    
    // Easter egg - system info
    Q_INVOKABLE QString getSystemWisdom() const;

signals:
    void playbackStateChanged();
    void recordingStateChanged();
    void trackChanged();
    void projectChanged();
    void errorOccurred(const QString& error);
    void statusMessage(const QString& message);

private slots:
    void onAudioDataReady();
    void onFrameReady();

private:
    void initializeComponents();
    void connectSignals();
    
    // Core components
    std::unique_ptr<suno::core::GitController> m_gitController;
    std::unique_ptr<suno::audio::AudioEngine> m_audioEngine;
    std::unique_ptr<suno::audio::StemProcessor> m_stemProcessor;
    std::unique_ptr<suno::video::VideoCapture> m_videoCapture;
    std::unique_ptr<suno::video::FrameCompositor> m_frameCompositor;
    std::unique_ptr<suno::visualizer::ProjectMWrapper> m_visualizer;
    std::unique_ptr<suno::visualizer::PresetManager> m_presetManager;
    std::unique_ptr<suno::network::SunoClient> m_sunoClient;
    std::unique_ptr<suno::network::GistUploader> m_gistUploader;
    std::unique_ptr<TimelineModel> m_timelineModel;
    
    // State
    QString m_currentTrackTitle;
    QString m_currentTrackArtist;
    QString m_currentProjectPath;
    bool m_projectModified = false;
};

} // namespace suno::ui
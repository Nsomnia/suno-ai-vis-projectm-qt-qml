#include "MainWindowController.h"
#include "core/Application.h"
#include "core/ConfigManager.h"
#include <spdlog/spdlog.h>
#include <QFile>
#include <QFileInfo>
#include <QRandomGenerator>

namespace suno::ui {

MainWindowController::MainWindowController(QObject* parent)
: QObject(parent)
{
    initializeComponents();
    connectSignals();

    spdlog::info("MainWindowController initialized and ready to rock");
}

MainWindowController::~MainWindowController()
{
    spdlog::debug("MainWindowController destroyed");
}

void MainWindowController::initializeComponents()
{
    auto* app = suno::core::Application::instance();
    auto* config = suno::core::ConfigManager::instance();

    m_gitController = std::make_unique<suno::core::GitController>(this);

    m_audioDecoder = std::make_unique<suno::audio::AudioDecoder>(this);
    m_audioEngine = std::make_unique<suno::audio::AudioEngine>(this);
    m_audioEngine->initialize();

    m_stemProcessor = std::make_unique<suno::audio::StemProcessor>(this);

    m_videoCapture = std::make_unique<suno::video::VideoCapture>(this);
    m_frameCompositor = std::make_unique<suno::video::FrameCompositor>(this);

    m_visualizer = std::make_unique<suno::visualizer::ProjectMWrapper>(this);
    m_visualizer->initialize(
        config->visualizerWidth(),
        config->visualizerHeight()
    );

    m_presetManager = std::make_unique<suno::visualizer::PresetManager>(this);
    m_presetManager->initialize(app->dataDirectory() + "/presets");

    m_sunoClient = std::make_unique<suno::network::SunoClient>(this);
    m_sunoClient->setApiEndpoint(config->sunoApiEndpoint());

    m_gistUploader = std::make_unique<suno::network::GistUploader>(this);
    if (!config->githubToken().isEmpty()) {
        m_gistUploader->setGithubToken(config->githubToken());
    }

    m_timelineModel = std::make_unique<TimelineModel>(this);

    m_audioDecoder->setAudioDataCallback([this](const float* data, size_t samples, int channels) {
        if (m_visualizer) {
            if (channels == 2) {
                m_visualizer->addPCMDataInterleaved(data, samples);
            } else {
                m_visualizer->addPCMData(data, samples);
            }
        }
    });

    spdlog::info("All components initialized successfully");
}

void MainWindowController::connectSignals()
{
    connect(m_audioDecoder.get(), &suno::audio::AudioDecoder::audioDataReady,
            this, &MainWindowController::onAudioDataReady);

    connect(m_visualizer.get(), &suno::visualizer::ProjectMWrapper::frameReady,
            this, &MainWindowController::onFrameReady);

    connect(m_audioDecoder.get(), &suno::audio::AudioDecoder::playbackStarted,
            this, [this]() { emit playbackStateChanged(); });
    connect(m_audioDecoder.get(), &suno::audio::AudioDecoder::playbackStopped,
            this, [this]() { emit playbackStateChanged(); });
    connect(m_audioDecoder.get(), &suno::audio::AudioDecoder::playbackPaused,
            this, [this]() { emit playbackStateChanged(); });

    connect(m_videoCapture.get(), &suno::video::VideoCapture::recordingStateChanged,
            this, &MainWindowController::recordingStateChanged);

    connect(m_audioDecoder.get(), &suno::audio::AudioDecoder::errorOccurred,
            this, &MainWindowController::errorOccurred);
    connect(m_visualizer.get(), &suno::visualizer::ProjectMWrapper::errorOccurred,
            this, &MainWindowController::errorOccurred);
    connect(m_sunoClient.get(), &suno::network::SunoClient::errorOccurred,
            this, &MainWindowController::errorOccurred);
}

bool MainWindowController::isPlaying() const
{
    return m_audioDecoder && m_audioDecoder->isPlaying();
}

bool MainWindowController::isRecording() const
{
    return m_videoCapture && m_videoCapture->isRecording();
}

void MainWindowController::play()
{
    if (m_audioDecoder) {
        m_audioDecoder->play();
        emit statusMessage("Playback started");
    }
}

void MainWindowController::pause()
{
    if (m_audioDecoder) {
        m_audioDecoder->pause();
        emit statusMessage("Playback paused");
    }
}

void MainWindowController::stop()
{
    if (m_audioDecoder) {
        m_audioDecoder->stop();
        emit statusMessage("Playback stopped");
    }
}

void MainWindowController::seek(qint64 positionMs)
{
    if (m_audioDecoder) {
        m_audioDecoder->seek(positionMs);
    }
}

void MainWindowController::loadAudioFile(const QString& filePath)
{
    spdlog::info("Loading audio file: {}", filePath.toStdString());

    if (!m_audioDecoder->loadFile(filePath)) {
        emit errorOccurred("Failed to load audio file");
        return;
    }

    QFileInfo fileInfo(filePath);
    m_currentTrackTitle = fileInfo.baseName();
    m_currentTrackArtist = "Local File";

    m_timelineModel->setDuration(m_audioDecoder->duration());

    emit trackChanged();
    emit statusMessage(QString("Loaded: %1").arg(m_currentTrackTitle));
}

void MainWindowController::loadSunoTrack(const QString& trackId)
{
    spdlog::info("Loading Suno track: {}", trackId.toStdString());

    QString cachePath = suno::core::Application::instance()->cacheDirectory()
        + "/tracks/" + trackId + ".mp3";

    m_sunoClient->downloadTrackAudio(trackId, cachePath);

    connect(m_sunoClient.get(), &suno::network::SunoClient::trackReceived,
            this, [this](const suno::network::SunoTrack& track) {
        m_currentTrackTitle = track.title;
        m_currentTrackArtist = track.artist;
        emit trackChanged();
    }, Qt::SingleShotConnection);

    connect(m_sunoClient.get(), &suno::network::SunoClient::audioDownloaded,
            this, [this](const QString&, const QString& filePath) {
        loadAudioFile(filePath);
    }, Qt::SingleShotConnection);

    m_sunoClient->fetchTrackById(trackId);
}

void MainWindowController::startRecording(const QString& outputPath)
{
    auto* config = suno::core::ConfigManager::instance();

    if (!m_videoCapture->initialize(
        outputPath,
        config->visualizerWidth(),
        config->visualizerHeight(),
        config->videoFps(),
        config->videoBitrate())) {
        emit errorOccurred("Failed to initialize video capture");
        return;
    }

    if (!m_videoCapture->startRecording()) {
        emit errorOccurred("Failed to start recording");
        return;
    }

    emit statusMessage("Recording started");
}

void MainWindowController::stopRecording()
{
    if (m_videoCapture) {
        m_videoCapture->stopRecording();
        emit statusMessage("Recording stopped");
    }
}

void MainWindowController::newProject()
{
    m_currentProjectPath.clear();
    m_projectModified = false;
    m_timelineModel->clear();

    emit projectChanged();
    emit statusMessage("New project created");
}

void MainWindowController::openProject(const QString& projectPath)
{
    spdlog::info("Opening project: {}", projectPath.toStdString());

    m_currentProjectPath = projectPath;
    m_projectModified = false;

    emit projectChanged();
    emit statusMessage(QString("Opened project: %1").arg(projectPath));
}

void MainWindowController::saveProject()
{
    if (m_currentProjectPath.isEmpty()) {
        emit errorOccurred("No project path set - use Save As");
        return;
    }

    saveProjectAs(m_currentProjectPath);
}

void MainWindowController::saveProjectAs(const QString& projectPath)
{
    spdlog::info("Saving project to: {}", projectPath.toStdString());

    m_currentProjectPath = projectPath;
    m_projectModified = false;

    emit statusMessage(QString("Project saved: %1").arg(projectPath));
}

void MainWindowController::onAudioDataReady()
{
}

void MainWindowController::onFrameReady()
{
    if (!isRecording()) {
        return;
    }

    QImage vizFrame = m_visualizer->grabFrameBuffer();

    qint64 currentPos = m_audioDecoder->position();
    QString lyric = m_timelineModel->activeLyricAtPosition(currentPos);
    if (!lyric.isEmpty()) {
        m_frameCompositor->setLyricText(lyric);
    }

    QImage finalFrame = m_frameCompositor->compositeFrame(vizFrame);
    m_videoCapture->captureFrame(finalFrame);
}

QString MainWindowController::getSystemWisdom() const
{
	QStringList wisdom = {
		"\"Talk is cheap. Show me the code.\" - Linus Torvalds",
		"\"I'd just like to interject for a moment. What you're referring to as Linux, "
		"is in fact, GNU/Linux...\" - Richard Stallman",
		"\"Given enough eyeballs, all bugs are shallow.\" - Linus's Law",
		"\"Software is like sex: it's better when it's free.\" - Linus Torvalds",
		"\"The only way to do great work is to love what you do.\" - Steve Jobs "
		"(but we prefer open source)",
		"\"Arch Linux: Because you secretly enjoy reading wikis at 3 AM\"",
		"\"Works on my machine\" - Every Developer Ever",
		"\"Have you tried turning it off and on again?\" - IT Proverb",
		"\"There are only two hard things in Computer Science: cache invalidation, "
		"naming things, and off-by-one errors.\"",
		"\"It's not a bug, it's a feature!\" - Marketing Department",
		"\"sudo make me a sandwich\" - XKCD 149",
		"\"In theory, theory and practice are the same. In practice, they're not.\"",
		"\"If it compiles, ship it!\" - Anonymous Senior Developer",
		"\"Real programmers use Vim. Or Emacs. The war continues...\"",
		"\"printf debugging: Because proper debuggers are for the weak\"",
		"\"I use Arch btw\" - Every Arch User in Any Conversation",
	};

	int index = QRandomGenerator::global()->bounded(wisdom.size());
	return wisdom[index];
}

} // namespace suno::ui
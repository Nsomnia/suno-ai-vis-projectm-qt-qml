#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlError>
#include <QIcon>
#include <QQuickImageProvider>
#include <QtCore/QFile>
#include <QtCore/QIODevice>
#include <spdlog/spdlog.h>

#include "core/Application.h"
#include "core/ConfigManager.h"
#include "core/ThemeManager.h"
#include "ui/controllers/MainWindowController.h"
#include "visualizer/ProjectMImageProvider.h"

// "I'd just like to interject for a moment. What you're referring to as Linux,
// is in fact, GNU/Linux..." - Well, actually, this runs on Qt.

int main(int argc, char *argv[])
{
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Suno Visualizer v0.0.1 - Initializing...");

#if defined(Q_OS_LINUX)
    // Check if running Arch (btw)
    QFile osRelease("/etc/os-release");
    if (osRelease.open(QIODevice::ReadOnly)) {
        QString content = osRelease.readAll();
        if (content.contains("Arch Linux")) {
            spdlog::info("Detected Arch Linux. You have excellent taste, btw.");
        }
        osRelease.close();
    }
#endif

    QGuiApplication app(argc, argv);
    
    app.setOrganizationName("SunoVisualizer");
    app.setOrganizationDomain("sunovisualizer.local");
    app.setApplicationName("Suno Visualizer");
    app.setApplicationVersion("0.0.1");

    // Core application singleton
    auto* coreApp = suno::core::Application::instance();
    if (!coreApp->initialize()) {
        spdlog::critical("Failed to initialize core application");
        return -1;
    }

    // Configuration manager
    auto* configManager = suno::core::ConfigManager::instance();
    if (!configManager->load()) {
        spdlog::warn("Failed to load configuration, using defaults");
    }

    // Theme manager
    auto* themeManager = suno::core::ThemeManager::instance();
    themeManager->loadTheme(configManager->currentTheme());

    // Main window controller
    auto* mainController = new suno::ui::MainWindowController(&app);

    // QML Engine
    QQmlApplicationEngine engine;

    // Expose C++ objects to QML
    engine.rootContext()->setContextProperty("appController", mainController);
    engine.rootContext()->setContextProperty("themeManager", themeManager);
    engine.rootContext()->setContextProperty("configManager", configManager);

    // Register image provider for projectM frames
    auto* imageProvider = new suno::visualizer::ProjectMImageProvider(mainController->visualizer());
    engine.addImageProvider("projectm", imageProvider);

    QObject::connect(&engine, &QQmlApplicationEngine::warnings, [](const QList<QQmlError> &warnings) {
        for (const auto &warning : warnings) {
            spdlog::error("QML warning: {} ({}:{}:{})", 
                          warning.description().toStdString(),
                          warning.url().toString().toStdString(),
                          warning.line(),
                          warning.column());
        }
    });

    using namespace Qt::StringLiterals;
    const QUrl mainQmlUrl(u"qrc:/SunoVisualizer/qml/main.qml"_s);
    
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated,
        &app, [mainQmlUrl](QObject *obj, const QUrl &objUrl) {
            if (!obj && mainQmlUrl == objUrl) {
                spdlog::critical("Failed to load QML");
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection
    );

    engine.load(mainQmlUrl);

    if (engine.rootObjects().isEmpty()) {
        spdlog::critical("No root QML objects created");
        return -1;
    }

    spdlog::info("Application started successfully. Talk is cheap, show me the visuals.");
    
    return app.exec();
}
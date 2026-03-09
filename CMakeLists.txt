cmake_minimum_required(VERSION 3.25)

project(suno-visualizer 
    VERSION 0.0.1
    DESCRIPTION "Professional A/V DAW Context Manager with Suno Integration"
    LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(cmake/CPM.cmake)

# Qt6 - The only reasonable GUI framework (fight me, GTK users)
find_package(Qt6 6.6 REQUIRED COMPONENTS 
    Core 
    Gui 
    Qml 
    Quick 
    QuickControls2
    Network
    Multimedia
)

# CPM Dependencies
CPMAddPackage(
    NAME tomlplusplus
    GITHUB_REPOSITORY marzer/tomlplusplus
    VERSION 3.4.0
    OPTIONS
        "TOML_BUILD_EXAMPLES OFF"
        "TOML_BUILD_TESTS OFF"
)

CPMAddPackage(
    NAME libgit2
    GITHUB_REPOSITORY libgit2/libgit2
    VERSION 1.7.1
    OPTIONS
        "BUILD_TESTS OFF"
        "BUILD_CLI OFF"
)

CPMAddPackage(
    NAME spdlog
    GITHUB_REPOSITORY gabime/spdlog
    VERSION 1.12.0
    OPTIONS
        "SPDLOG_BUILD_EXAMPLE OFF"
        "SPDLOG_BUILD_TESTS OFF"
)

CPMAddPackage(
    NAME nlohmann_json
    GITHUB_REPOSITORY nlohmann/json
    VERSION 3.11.3
    OPTIONS
        "JSON_BuildTests OFF"
)

# FFmpeg - Will be found via pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET
    libavcodec
    libavformat
    libavutil
    libswscale
    libswresample
)

# libprojectM v4.1.x - The visualization engine
pkg_check_modules(PROJECTM REQUIRED "projectM-4>=4.1.0")
pkg_check_modules(PROJECTM_PLAYLIST REQUIRED "projectM-4-playlist>=4.1.0")

find_library(PROJECTM_LIBRARY NAMES projectM-4 PATHS ${PROJECTM_LIBRARY_DIRS})
find_library(PROJECTM_PLAYLIST_LIBRARY NAMES projectM-4-playlist PATHS ${PROJECTM_PLAYLIST_LIBRARY_DIRS})
find_package(OpenGL REQUIRED)

include_directories(${PROJECTM_INCLUDE_DIRS})

# Source files
set(SOURCES
    src/main.cpp
    src/core/Application.cpp
    src/core/ConfigManager.cpp
    src/core/GitController.cpp
    src/core/ThemeManager.cpp
    src/audio/AudioEngine.cpp
    src/audio/StemProcessor.cpp
    src/video/VideoCapture.cpp
    src/video/FrameCompositor.cpp
    src/visualizer/ProjectMWrapper.cpp
    src/visualizer/ProjectMImageProvider.cpp
    src/visualizer/PresetManager.cpp
    src/network/SunoClient.cpp
    src/network/GistUploader.cpp
    src/ui/models/TimelineModel.cpp
    src/ui/controllers/MainWindowController.cpp
)

set(HEADERS
    src/core/Application.h
    src/core/ConfigManager.h
    src/core/GitController.h
    src/core/ThemeManager.h
    src/audio/AudioEngine.h
    src/audio/StemProcessor.h
    src/video/VideoCapture.h
    src/video/FrameCompositor.h
    src/visualizer/ProjectMWrapper.h
    src/visualizer/ProjectMImageProvider.h
    src/visualizer/PresetManager.h
    src/network/SunoClient.h
    src/network/GistUploader.h
    src/ui/models/TimelineModel.h
    src/ui/controllers/MainWindowController.h
)

qt_add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
)

qt_add_qml_module(${PROJECT_NAME}
    URI SunoVisualizer
    VERSION 1.0
    RESOURCE_PREFIX "/"
    QML_FILES
        qml/main.qml
        qml/components/AccordionDrawer.qml
        qml/components/VisualizerCanvas.qml
        qml/components/TimelineScrubber.qml
        qml/components/ThemeSwitch.qml
        qml/windows/SettingsWindow.qml
        qml/windows/SunoQueueWindow.qml
        qml/themes/LightTheme.qml
        qml/themes/DarkTheme.qml
    RESOURCES
        resources/icons/import.svg
        resources/icons/download.svg
        resources/icons/play.svg
        resources/icons/pause.svg
        resources/icons/chevron-down.svg
        resources/icons/chevron-up.svg
        resources/icons/chevron-left.svg
        resources/icons/chevron-right.svg
        resources/icons/code.svg
        resources/icons/text.svg
        resources/icons/users.svg
        resources/icons/volume.svg
        resources/icons/record.svg
        resources/icons/skip-forward.svg
        resources/icons/skip-back.svg
        resources/icons/plus.svg
        resources/icons/git-commit.svg
        resources/icons/maximize.svg
        resources/icons/sidebar-right.svg
        resources/icons/settings.svg
        resources/icons/menu.svg
        resources/icons/folder.svg
        resources/icons/folder-open.svg
        resources/icons/sparkles.svg
        resources/icons/music.svg
        resources/icons/waveform.svg
        resources/icons/file-plus.svg
        resources/icons/save.svg
        resources/icons/video.svg
        resources/icons/star.svg
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Network
    Qt6::Multimedia
    tomlplusplus::tomlplusplus
    git2
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    PkgConfig::FFMPEG
    ${PROJECTM_LIBRARY}
    ${PROJECTM_PLAYLIST_LIBRARY}
    OpenGL::GL
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

include(cmake/CompilerWarnings.cmake)
set_project_warnings(${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)
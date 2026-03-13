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

option(ENABLE_UNITY_BUILD "Enable unity build for faster compilation" OFF)
option(ENABLE_FAST_BUILD "Enable fast build mode (no optimizations)" ON)
option(USE_CCACHE "Use ccache for faster rebuilds" ON)

if(USE_CCACHE AND NOT CMAKE_CXX_COMPILER_LAUNCHER)
    find_program(CCACHE_PROGRAM ccache)
    if(CCACHE_PROGRAM)
        set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
        message(STATUS "Using ccache: ${CCACHE_PROGRAM}")
    endif()
endif()

if(ENABLE_UNITY_BUILD)
    set(CMAKE_UNITY_BUILD ON)
    set(CMAKE_UNITY_BUILD_BATCH_SIZE 16)
endif()

if(ENABLE_FAST_BUILD)
    if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_options(-O0 -g1 -fno-lto)
        string(REPLACE "-O2" "-O0" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
        string(REPLACE "-O3" "-O0" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
    endif()
    message(STATUS "Fast build mode enabled: -O0 -g1 -fno-lto")
endif()

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

find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET
  libavcodec
  libavformat
  libavutil
  libswscale
  libswresample
)

pkg_check_modules(PULSEAUDIO REQUIRED IMPORTED_TARGET
  libpulse
  libpulse-mainloop-glib
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
  src/audio/AudioDecoder.cpp
  src/audio/StemProcessor.cpp
  src/video/VideoCapture.cpp
  src/video/FrameCompositor.cpp
  src/visualizer/ProjectMWrapper.cpp
  src/visualizer/ProjectMRenderer.cpp
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
  src/audio/AudioDecoder.h
  src/audio/StemProcessor.h
  src/video/VideoCapture.h
  src/video/FrameCompositor.h
  src/visualizer/ProjectMWrapper.h
  src/visualizer/ProjectMRenderer.h
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
  PkgConfig::PULSEAUDIO
  ${PROJECTM_LIBRARY}
  ${PROJECTM_PLAYLIST_LIBRARY}
  OpenGL::GL
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_precompile_headers(${PROJECT_NAME} PRIVATE
    <vector>
    <string>
    <memory>
    <QtCore/QObject>
    <QtCore/QVariant>
    <QtCore/QString>
    <QtGui/QGuiApplication>
    <QtQml/QQmlEngine>
    <QtQuick/QQuickItem>
)

if(ENABLE_UNITY_BUILD)
    set_source_files_properties(${SOURCES} PROPERTIES SKIP_UNITY_BUILD_INCLUSION FALSE)
endif()

include(cmake/CompilerWarnings.cmake)
set_project_warnings(${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME}
  BUNDLE DESTINATION .
  RUNTIME DESTINATION bin
)

enable_testing()
find_package(Qt6 6.6 REQUIRED COMPONENTS Test)

add_executable(audio_decoder_test
  tests/test_audio_decoder.cpp
  src/audio/AudioDecoder.cpp
)
target_link_libraries(audio_decoder_test PRIVATE
  Qt6::Core
  Qt6::Test
  PkgConfig::FFMPEG
  PkgConfig::PULSEAUDIO
  spdlog::spdlog
)
target_include_directories(audio_decoder_test PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_test(NAME AudioDecoderTest COMMAND audio_decoder_test)
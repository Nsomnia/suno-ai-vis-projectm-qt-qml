# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.0.2] - 2026-03-13

### Added
- **AudioDecoder**: FFmpeg-based audio decoding with PulseAudio output
  - Supports MP3, FLAC, WAV, OGG, M4A, AAC formats
  - Real-time PCM data callback for visualizer integration
  - Volume control, seeking, and playback state management
  - Proper PulseAudio streaming (visible in pavucontrol)

- **ProjectMRenderer**: QQuickFramebufferObject-based OpenGL renderer
  - Proper GL context integration with QML scene graph
  - projectM v4 API implementation
  - Support for 4188+ MilkDrop presets

- **UI Enhancements**:
  - Menu bar with File, View, Help menus
  - Clean exit via Ctrl+Q or File → Quit
  - About dialog with application info
  - FileDialog for loading audio files
  - Dark mode icon colorization using MultiEffect

- **Testing**:
  - Unit tests for AudioDecoder (6 tests, all passing)
  - Qt6 Test framework integration

### Fixed
- OpenGL context issues causing blank visualizer
- Audio playback not appearing in pavucontrol
- qml_target_version CMake command not found
- PulseAudio pa_stream_flags_t type conversion
- QString escape sequences in C++ string literals
- Missing QOpenGLFramebufferObject include

### Changed
- Major architectural overhaul of audio pipeline
- Replaced placeholder AudioEngine with real AudioDecoder
- Updated README with architecture documentation

## [0.0.1] - 2025-01-XX

### Added
- Initial project setup
- Qt6/QML application structure
- Basic UI components (AccordionDrawer, VisualizerCanvas, TimelineScrubber)
- Theme system (Light/Dark)
- projectM integration skeleton
- Configuration management
- Git-backed version control

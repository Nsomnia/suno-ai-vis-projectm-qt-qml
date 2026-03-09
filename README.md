# Suno Visualizer - Professional A/V DAW Context Manager

A hyper-modern Qt6/C++23 application combining project management, Suno.com music integration, real-time audio visualization via libprojectM, and professional video rendering capabilities.

## Features

- **Git-Backed Version Control**: Every visualizer preset, lyric layer, and audio track change is versioned
- **Suno Integration**: Direct API access to fetch tracks, lyrics, and album art
- **Advanced Audio Processing**: AI-powered stem isolation feeding distinct visual channels
- **Real-time Visualization**: libprojectM v4.1.x with live MilkDrop preset editing
- **Video Export**: Capture QML/OpenGL surfaces with text overlays to FFmpeg
- **Timeline Scrubbing**: Git-blame inspired timeline for reverting visual changes
- **Preset Sharing**: P2P GitHub Gist integration for community presets
- **Modern UI**: Mobile-inspired accordion interface for desktop

## Build Requirements

- C++23 compatible compiler (GCC 13+, Clang 16+, MSVC 2022+)
- CMake 3.25+
- Qt 6.6+
- Arch Linux (recommended, btw)

## Building
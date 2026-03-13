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
- FFmpeg (libavcodec, libavformat, libavutil, libswscale, libswresample)
- PulseAudio (libpulse, libpulse-mainloop-glib)
- projectM 4.1.0+
- Arch Linux (recommended, btw)

## Building

```bash
mkdir build && cd build
cmake .. -DENABLE_FAST_BUILD=ON
make -j$(nproc)
```

## Running

```bash
./suno-visualizer
```

## Architecture

### Audio Pipeline
- **AudioDecoder**: FFmpeg-based audio decoding with PulseAudio output
  - Decodes any FFmpeg-supported format (MP3, FLAC, WAV, OGG, etc.)
  - Real-time PCM data callback for visualizer integration
  - PulseAudio streaming for system audio visibility in pavucontrol

### Visualizer Pipeline
- **ProjectMRenderer**: QQuickFramebufferObject-based OpenGL renderer
  - Proper GL context integration with QML scene graph
  - projectM v4 API usage for MilkDrop-style visualizations
  - Preset management with hot-reload support

### Data Flow
```
AudioDecoder --(PCM callback)--> ProjectMWrapper --(GL render)--> QML Scene
      |
      v
 PulseAudio (visible in pavucontrol)
```

## Testing

```bash
cd build
ctest --output-on-failure
```

## Dependencies

| Library | Version | Purpose |
|---------|---------|---------|
| Qt6 | 6.6+ | UI framework, QML, Quick |
| FFmpeg | 62.x | Audio decoding |
| PulseAudio | 17.x | Audio output |
| projectM | 4.1.x | Visualization engine |
| spdlog | 1.12.x | Logging |
| libgit2 | 1.7.x | Git integration |
| nlohmann_json | 3.11.x | JSON parsing |
| tomlplusplus | 3.4.x | Config files |
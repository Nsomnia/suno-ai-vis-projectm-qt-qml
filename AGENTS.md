# AI Agent Guidance

This document provides guidance for AI agents working on this codebase.

## Project Overview

- **Name**: Suno Visualizer
- **Type**: Qt6/C++23 desktop application
- **Purpose**: Professional A/V DAW context manager with Suno music integration, real-time audio visualization via libprojectM, and video rendering
- **License**: MIT

## Technology Stack

- **Language**: C++23
- **UI Framework**: Qt6 (QML/Quick)
- **Build System**: CMake 3.25+
- **Key Dependencies**:
  - libprojectM v4.1.x (visualization)
  - FFmpeg (video capture)
  - libgit2 (version control)
  - tomlplusplus (config)
  - spdlog (logging)
  - nlohmann_json (JSON parsing)

## Code Standards

### C++ Guidelines

1. **C++23 Standard**: Use C++23 features where appropriate
2. **Single Responsibility**: Each class should have one clear purpose
3. **No Monolithic Dependencies**: Keep proprietary code independent
4. **Header-Only where sensible**: Use header-only libraries for small utilities
5. **Strict Type Safety**: Prefer strong types over raw primitives

### Naming Conventions

- **Classes**: PascalCase (e.g., `AudioEngine`, `ConfigManager`)
- **Functions**: PascalCase (e.g., `loadConfiguration()`, `processFrame()`)
- **Variables**: camelCase (e.g., `audioBuffer`, `frameCount`)
- **Constants**: kPascalCase or UPPER_SNAKE_CASE
- **Files**: PascalCase for C++, snake_case for CMake/python
- **QML Components**: PascalCase (matching component names)

### Code Organization

```
src/
  audio/      - Audio processing and stem isolation
  core/       - Application, config, themes, git integration
  network/    - Suno API client, Gist upload
  ui/         - Controllers and models (MVC pattern)
  video/      - FFmpeg capture and frame compositing
  visualizer/ - libprojectM wrapper and preset management

qml/
  components/ - Reusable QML components
  themes/     - Theme definitions (light/dark)
  windows/    - Window-level QML files

cmake/        - CMake modules and utilities
docs/         - Documentation
resources/    - Icons, assets
```

### Build Instructions

```zsh
# Standard build
./build.zsh

# Debug build
./build.zsh -d

# Clean build with specific jobs
./build.zsh -c -j4
```

## Git Workflow

1. **Commit Messages**: Use conventional commits format
   - `feat: add new feature`
   - `fix: resolve issue`
   - `docs: update documentation`
   - `refactor: restructure code`
   - `chore: maintenance tasks`

2. **Branch Naming**: `feature/description`, `fix/description`, `docs/description`

3. **Changelog**: Update `CHANGELOG.md` with each release (see Keep a Changelog format)

## QML Guidelines

1. **Component Structure**: One component per file, filename matches component name
2. **Property Naming**: camelCase, use proper QML types
3. **Signals**: Past-tense naming (e.g., `onLoadingChanged`)
4. **Imports**: Use QtQuick and QtQuick.Controls 2.x imports

## Configuration

- Config stored in: `~/.config/suno-visualizer/`
- Format: TOML
- Default config created on first run

## Testing

- Use Catch2 for unit tests (when added)
- Build tests with `-DCMAKE_BUILD_TESTING=ON`

## Security Considerations

- Never commit secrets, tokens, or credentials
- Use environment variables or secure storage outside of the repository root directory for API keys and other secrets.

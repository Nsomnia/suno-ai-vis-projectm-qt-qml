# TODO - Suno Visualizer

> **Warning:** When moved out of build/../.. it crashes again

---

## 1. Core Infrastructure

### 1.1 Application Bootstrap
- [ ] Fix binary relocation crash (runs only from build directory)
- [ ] Implement graceful startup with splash screen
- [ ] Add application icon and branding
- [ ] Implement single-instance enforcement (prevent multiple instances)
- [ ] Add system tray support for background operation
- [ ] Implement auto-update mechanism
- [ ] Add crash handler with stack trace logging

### 1.2 Configuration System
- [ ] Config hot-reload without restart
- [ ] Config migration between versions
- [ ] Config backup/restore functionality
- [ ] Environment variable overrides for config
- [ ] Per-project config files
- [ ] Config validation on startup

---

## 2. Audio Engine

### 2.1 Playback System
- [ ] Implement gapless playback between tracks
- [ ] Add crossfade support with configurable duration
- [ ] Support for more audio formats (FLAC, WAV, AIFF, OGG)
- [ ] Audio normalization/leveling
- [ ] ReplayGain support
- [ ] Queue management (upcoming, history)
- [ ] Shuffle and repeat modes
- [ ] Tempo/pitch shifting without affecting visualization sync
- [ ] Auto-detect audio device and sample rate
- [ ] Support for external audio input (microphone, loopback)

### 2.2 Stem Separation
- [ ] Integrate Demucs or Spleeter for stem separation
- [ ] Real-time stem processing pipeline
- [ ] Per-stem visualization channels (vocals, drums, bass, other)
- [ ] Stem isolation toggle for each channel
- [ ] Stem export functionality
- [ ] Cache processed stems for faster loading
- [ ] Progress indicator for stem processing

### 2.3 Audio Analysis
- [ ] Beat detection and BPM calculation
- [ ] Frequency spectrum analysis (FFT)
- [ ] Onset detection for reactive visuals
- [ ] Loudness metering (LUFS)
- [ ] Genre detection (basic)
- [ ] Mood/energy analysis
- [ ] Chord progression detection
- [ ] Vocal/instrumental classification

---

## 3. Visualization Engine

### 3.1 libprojectM Integration
- [ ] Multiple projectM instances for different stem channels
- [ ] Real-time preset switching without audio interruption
- [ ] Preset blending/mixing between two presets
- [ ] Custom parametric presets (generated from audio analysis)
- [ ] GPU memory management and optimization
- [ ] Resolution scaling for performance
- [ ] Frame rate limiting for low-end systems
- [ ] Stereo/3D audio reactive modes

### 3.2 Preset Management
- [ ] Preset browser with thumbnails/previews
- [ ] Favorite presets system
- [ ] Preset tagging and categorization
- [ ] Smart playlists based on audio characteristics
- [ ] Preset metadata editor
- [ ] Import/export preset collections
- [ ] Preset rating system
- [ ] Automatic preset selection based on genre/mood
- [ ] Preset versioning with3.3 Custom git

###  Visuals
- [ ] Shader editor with live preview
- [ ] Built-in GLSL/HLSL shader library
- [ ] Particle system integration
- [ ] 3D model rendering in visualizations
- [ ] Audio-reactive particle effects
- [ ] Shape primitives library
- [ ] Custom texture support
- [ ] Procedural background generation

### 3.4 Multi-Display
- [ ] Fullscreen on secondary monitor
- [ ] Different visualization per monitor
- [ ] Mirror or extended modes
- [ ] Bezel compensation for multi-projector setups
- [ ] EDID-aware resolution selection

---

## 4. Video Capture & Export

### 4.1 Recording
- [ ] Record visualization to video file
- [ ] Configurable resolution (up to 4K/8K)
- [ ] Configurable frame rate (24, 30, 60, 120 fps)
- [ ] HDR recording support
- [ ] Lossless recording option (DNxHR, ProRes)
- [ ] Recording queue (batch export)
- [ ] Pause/resume recording
- [ ] Scheduled recording

### 4.2 Streaming
- [ ] RTMP streaming to Twitch/YouTube
- [ ] OBS integration/control
- [ ] Custom RTMP server support
- [ ] Stream quality presets
- [ ] Chat overlay integration

### 4.3 Frame Compositing
- [ ] Text overlays with custom fonts
- [ ] Image/logo watermarking
- [ ] Lower thirds generation
- [ ] Countdown timer overlay
- [ ] Dynamic lyric overlay (synced to Suno)
- [ ] Picture-in-picture support
- [ ] Transition effects between scenes

---

## 5. Suno API Integration

### 5.1 Authentication
- [ ] OAuth2 flow for Suno login
- [ ] Token refresh handling
- [ ] Secure credential storage
- [ ] Multi-account support

### 5.2 Track Management
- [ ] Fetch user's created tracks
- [ ] Fetch liked/favorited tracks
- [ ] Search Suno library
- [ ] Download tracks with quality selection
- [ ] Automatic track metadata fetching
- [ ] Album art download and caching
- [ ] Lyrics fetching and display

### 5.3 Playlist Features
- [ ] Create/edit Suno playlists
- [ ] Smart playlists based on listening history
- [ ] Playlist sync with local queue
- [ ] Collaborative playlist support

### 5.4 Queue System
- [ ] Create queues from Suno links
- [ ] Auto-add related tracks
- [ ] Queue persistence across sessions
- [ ] Drag-and-drop reordering
- [ ] Smart shuffle (not completely random)

---

## 6. UI/UX Enhancements

### 6.1 Main Window
- [ ] Collapsible side panels
- [ ] Mini player mode (compact)
- [ ] Resizable panels
- [ ] Panel layout presets (save/load)
- [ ] Keyboard shortcuts for all actions
- [ ] Mouse gesture support

### 6.2 Visualizer Canvas
- [ ] Multiple canvas sizes/aspect ratios
- [ ] Windowed mode with transparency
- [ ] Always-on-top option
- [ ] Click-through mode for overlays
- [ ] Custom border/window chrome
- [ ] Multi-window support

### 6.3 Themes
- [ ] System theme detection (follow OS light/dark)
- [ ] Theme creator/editor
- [ ] Import community themes
- [ ] Per-element theme customization
- [ ] Animated theme transitions

### 6.4 Accessibility
- [ ] Screen reader support
- [ ] High contrast mode
- [ ] Scalable UI (DPI awareness)
- [ ] Keyboard navigation highlights
- [ ] Reduced motion option

### 6.5 Animations & Effects
- [ ] Smooth panel animations
- [ ] Micro-interactions on hover/click
- [ ] Loading skeletons
- [ ] Toast notifications
- [ ] Modal dialogs for important actions

---

## 7. Timeline & History

### 7.1 Timeline Model
- [ ] Visual timeline with waveform
- [ ] Bookmark specific moments
- [ ] Annotation/comments on timeline
- [ ] Export timeline as image/PDF
- [ ] Timeline zoom levels

### 7.2 History
- [ ] Listening history with timestamps
- [ ] Statistics dashboard
- [ ] Most played tracks/artists
- [ ] Time-based listening reports
- [ ] Export history data

---

## 8. Git Integration

### 8.1 Version Control
- [ ] Auto-commit on preset changes
- [ ] Commit message templates
- [ ] Branch for different project configurations
- [ ] Diff viewer for preset changes
- [ ] Merge conflict resolution UI

### 8.2 Preset Sharing
- [ ] Publish presets to GitHub Gist
- [ ] Browse community presets from Gist
- [ ] One-click preset import
- [ ] Preset fork functionality
- [ ] Star/favorite community presets

---

## 9. Network Features

### 9.1 Sync Features
- [ ] Cloud config sync
- [ ] Cross-device preset sync
- [ ] Share visualization state via URL
- [ ] Real-time collaboration (multi-user editing)

### 9.2 API Extensions
- [ ] WebSocket server for remote control
- [ ] REST API for integrations
- [ ] Home Assistant integration
- [ ] Philips Hue sync
- [ ] MIDI controller mapping
- [ ] OSC (Open Sound Control) support

---

## 10. Performance & Optimization

### 10.1 CPU Optimization
- [ ] Multi-threaded audio processing
- [ ] Thread affinity settings
- [ ] Process priority adjustment
- [ ] Idle detection for auto-pause

### 10.2 GPU Optimization
- [ ] Vulkan rendering backend (optional)
- [ ] GPU selection for multi-GPU systems
- [ ] Texture compression
- [ ] Adaptive quality based on FPS

### 10.3 Memory Management
- [ ] Memory usage monitoring UI
- [ ] Automatic cache cleanup
- [ ] Streaming large audio files
- [ ] Memory profiling tools

### 10.4 Startup Performance
- [ ] Lazy loading of modules
- [ ] Background initialization
- [ ] Startup time optimization

---

## 11. Platform Features

### 11.1 Linux
- [ ] PulseAudio/PipeWire support
- [ ] .desktop file integration
- [ ] System notifications
- [ ] Native file dialogs
- [ ] X11/Wayland detection

### 11.2 Windows
- [ ] WASAPI exclusive mode
- [ ] Windows store packaging
- [ ] MSI installer
- [ ] Start menu integration
- [ ] Windows notifications

### 11.3 macOS
- [ ] Core Audio support
- [ ] DMG distribution
- [ ] App Store preparation
- [ ] Touch Bar support
- [ ] Native menu bar

---

## 12. Testing & Quality Assurance

### 12.1 Unit Tests
- [ ] Test audio processing algorithms
- [ ] Test configuration parsing
- [ ] Test network client responses
- [ ] Test git operations

### 12.2 Integration Tests
- [ ] End-to-end playback test
- [ ] Visualization rendering test
- [ ] Video export verification

### 12.3 Performance Tests
- [ ] Memory leak detection
- [ ] Stress testing
- [ ] FPS stability tests

---

## 13. Documentation

### 13.1 User Documentation
- [ ] Getting started guide
- [ ] Video tutorials
- [ ] API documentation
- [ ] Troubleshooting guide
- [ ] FAQ section

### 13.2 Developer Documentation
- [ ] Architecture overview
- [ ] Plugin SDK documentation
- [ ] Contribution guidelines
- [ ] Release process

---

## 14. Plugin System

### 14.1 Plugin Architecture
- [ ] Plugin API specification
- [ ] Plugin loader/unloader
- [ ] Plugin sandboxing
- [ ] Plugin marketplace

### 14.2 Official Plugins
- [ ] Spotify integration plugin
- [ ] SoundCloud integration plugin
- [ ] YouTube integration plugin
- [ ] MIDI controller plugin
- [ ] Game audio capture plugin

---

## 15. AI & Machine Learning

### 15.1 Audio Understanding
- [ ] AI-powered genre classification
- [ ] Mood detection for visualization matching
- [ ] Beat prediction for smoother transitions
- [ ] Lyrics sentiment analysis

### 15.2 Visualization AI
- [ ] Neural style transfer for presets
- [ ] AI-generated procedural textures
- [ ] Semantic search for presets
- [ ] Recommendation system for presets

### 15.3 Voice Control
- [ ] Voice command recognition
- [ ] Natural language playlist queries
- [ ] Voice activity detection

---

## 16. Community & Social

### 16.1 Preset Sharing Platform
- [ ] In-app preset marketplace
- [ ] User profiles and portfolios
- [ ] Preset of the day/week
- [ ] Community challenges/contests

### 16.2 Social Features
- [ ] Share to social media (video clips)
- [ ] Collaboration rooms
- [ ] Live visualization events
- [ ] Discord bot integration

---

## 17. Mobile Companion

### 17.1 Mobile App
- [ ] iOS/Android companion app
- [ ] Remote playback control
- [ ] Queue management
- [ ] Preset browsing
- [ ] Mobile as second display

### 17.2 Web Interface
- [ ] Web-based remote control
- [ ] Stream preview
- [ ] Basic controls only

---

## 18. Advanced Features

### 18.1 Audio-Visual Sync
- [ ] DJ mix visualization mode
- [ ] Karaoke mode with vocal visualization
- [ ] Concert mode (reacts to crowd noise via mic)
- [ ] Ambient mode (reacts to room audio)

### 18.2 Custom Scenes
- [ ] Scene builder with drag-and-drop
- [ ] Scene transitions with effects
- [ ] Scene scheduling/automation
- [ ] Scene templates

### 18.3 MIDI/OSC Mapping
- [ ] Visual MIDI mapper
- [ ] OSC address browser
- [ ] MIDI learn mode
- [ ] Save/load MIDI mappings

---

## 19. Internationalization

### 19.1 Localization
- [ ] Translation system (Qt Linguist)
- [ ] Supported languages: English, Spanish, French, German, Japanese, Chinese, Korean
- [ ] RTL language support preparation
- [ ] Locale-aware date/time formatting
- [ ] Locale-aware number formatting

---

## 20. Security & Privacy

### 20.1 Data Protection
- [ ] Local-first data storage
- [ ] Encryption for sensitive data
- [ ] Anonymized analytics option
- [ ] Data export/deletion tools

### 20.2 Network Security
- [ ] Certificate pinning
- [ ] HTTPS-only API calls
- [ ] Request signing
- [ ] Rate limiting

---

## Priority Order

### Phase 1: Core Functionality
- Fix crash on relocation
- Audio playback stability
- Basic visualization
- Configuration persistence

### Phase 2: User Experience
- Better UI/UX
- Theme system completion
- Keyboard shortcuts
- Settings window

### Phase 3: Feature Complete
- Stem separation
- Video export
- Suno API integration
- Git preset management

### Phase 4: Polish
- Performance optimization
- Cross-platform support
- Testing and bug fixes

### Phase 5: Community
- Plugin system
- Preset marketplace
- Mobile companion
- AI features

---

## Notes

- Items marked with [ ] are pending
- Check issues for detailed discussions
- Priority may shift based on user feedback
- Some features require external library integration

---

**Last Updated:** 2026-03-09
**Version:** 0.0.1

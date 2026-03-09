# Contributing to Suno Visualizer

Thank you for your interest in contributing!

## Code of Conduct

Be respectful, constructive, and collaborative. We welcome contributors of all experience levels.

## How to Contribute

### Reporting Bugs

1. Check existing issues to avoid duplicates
2. Use the bug report template
3. Include:
   - Clear title and description
   - Steps to reproduce
   - Expected vs actual behavior
   - Environment details (OS, Qt version, etc.)

### Suggesting Features

1. Open a discussion first for major features
2. Describe the use case clearly
3. Consider implementation approach

### Pull Requests

1. **Fork** the repository
2. **Create** a feature branch: `feature/description` or `fix/description`
3. **Follow** the code standards (see `AGENTS.md`)
4. **Test** your changes thoroughly
5. **Commit** using conventional commits format
6. **Push** and open a PR

### Code Standards

- C++23 with strict type safety
- Qt6/QML for UI
- Single Responsibility Principle
- See `AGENTS.md` for detailed guidelines

### Commit Messages

Use conventional commits:

```
feat: add timeline scrubbing
fix: resolve audio buffer overflow
docs: update build instructions
refactor: restructure audio pipeline
chore: update dependencies
```

### Build & Test

```zsh
# Build
./build.zsh

# Debug build
./build.zsh -d

# Clean build
./build.zsh -c
```

## Review Process

1. All PRs require review
2. Address feedback promptly
3. Squash commits before merge

## License

By contributing, you agree your code will be licensed under the MIT License.

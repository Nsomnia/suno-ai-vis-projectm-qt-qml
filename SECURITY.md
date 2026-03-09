# Security Policy

## Supported Versions

| Version | Supported          |
| ------- | ------------------ |
| 0.0.x   | :white_check_mark: |

## Reporting a Vulnerability

If you discover a security vulnerability, please report it responsibly:

1. **DO NOT** create a public GitHub issue
2. **DO** email the maintainer directly
3. **INCLUDE**:
   - Description of the vulnerability
   - Steps to reproduce
   - Potential impact
   - Suggested fix (if any)

## Security Best Practices

### API Keys and Credentials

- Never commit API keys, tokens, or secrets to the repository
- Use environment variables for sensitive configuration
- Store credentials in `~/.config/suno-visualizer/secrets.toml` (not tracked by git)
- The `.gitignore` already excludes common secret file patterns

### Network Security

- Validate all API responses
- Use HTTPS for all network requests
- Sanitize user input before making API calls

### Input Validation

- Validate all file paths before reading/writing
- Sanitize user-provided paths to prevent directory traversal
- Use Qt's built-in validation where possible

### Build Security

- Review third-party dependencies
- Use pinned versions for critical dependencies
- Run security audits periodically

## Dependencies

Key dependencies are monitored for security advisories:

- Qt6
- libgit2
- FFmpeg
- nlohmann_json
- spdlog
- tomlplusplus

## Acknowledgments

Thank you for helping keep this project secure!

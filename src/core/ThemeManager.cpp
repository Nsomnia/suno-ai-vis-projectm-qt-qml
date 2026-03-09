#include "ThemeManager.h"
#include <spdlog/spdlog.h>

namespace suno::core {

ThemeManager* ThemeManager::s_instance = nullptr;

ThemeManager::ThemeManager()
{
    loadDarkTheme(); // Default theme
}

ThemeManager* ThemeManager::instance()
{
    if (!s_instance) {
        s_instance = new ThemeManager();
    }
    return s_instance;
}

void ThemeManager::loadTheme(const QString& themeName)
{
    spdlog::info("Loading theme: {}", themeName.toStdString());
    
    if (themeName == "light") {
        loadLightTheme();
    } else if (themeName == "dark") {
        loadDarkTheme();
    } else {
        spdlog::warn("Unknown theme '{}', falling back to dark", themeName.toStdString());
        loadDarkTheme();
    }
    
    m_currentTheme = themeName;
    emit currentThemeChanged();
    emit themeColorsChanged();
}

void ThemeManager::loadLightTheme()
{
    // Light theme for those who hate their eyes
    m_primaryColor = QColor("#2196F3");      // Blue
    m_secondaryColor = QColor("#03A9F4");    // Light Blue
    m_backgroundColor = QColor("#FAFAFA");   // Very light gray
    m_surfaceColor = QColor("#FFFFFF");      // White
    m_textColor = QColor("#212121");         // Almost black
    m_accentColor = QColor("#FF5722");       // Deep Orange
    
    spdlog::debug("Light theme loaded (eye damage mode activated)");
}

void ThemeManager::loadDarkTheme()
{
    // Dark theme for professionals who code at 3 AM
    m_primaryColor = QColor("#BB86FC");      // Purple
    m_secondaryColor = QColor("#03DAC6");    // Teal
    m_backgroundColor = QColor("#121212");   // Almost black
    m_surfaceColor = QColor("#1E1E1E");      // Dark gray
    m_textColor = QColor("#FFFFFF");          // White for maximum contrast
    m_accentColor = QColor("#CF6679");       // Pink
    
    spdlog::debug("Dark theme loaded (superior choice confirmed)");
}

QColor ThemeManager::getColor(const QString& key) const
{
    if (m_customColors.contains(key)) {
        return m_customColors.value(key);
    }
    
    // Return accent as fallback
    return m_accentColor;
}

void ThemeManager::setColor(const QString& key, const QColor& color)
{
    m_customColors[key] = color;
    emit themeColorsChanged();
}

} // namespace suno::core
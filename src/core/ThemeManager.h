#pragma once

#include <QObject>
#include <QString>
#include <QColor>
#include <QHash>

namespace suno::core {

class ThemeManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentTheme READ currentTheme NOTIFY currentThemeChanged)
    Q_PROPERTY(QColor primaryColor READ primaryColor NOTIFY themeColorsChanged)
    Q_PROPERTY(QColor secondaryColor READ secondaryColor NOTIFY themeColorsChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor NOTIFY themeColorsChanged)
    Q_PROPERTY(QColor surfaceColor READ surfaceColor NOTIFY themeColorsChanged)
    Q_PROPERTY(QColor textColor READ textColor NOTIFY themeColorsChanged)
    Q_PROPERTY(QColor accentColor READ accentColor NOTIFY themeColorsChanged)

public:
    static ThemeManager* instance();
    
    Q_INVOKABLE void loadTheme(const QString& themeName);
    
    QString currentTheme() const { return m_currentTheme; }
    
    QColor primaryColor() const { return m_primaryColor; }
    QColor secondaryColor() const { return m_secondaryColor; }
    QColor backgroundColor() const { return m_backgroundColor; }
    QColor surfaceColor() const { return m_surfaceColor; }
    QColor textColor() const { return m_textColor; }
    QColor accentColor() const { return m_accentColor; }
    
    Q_INVOKABLE QColor getColor(const QString& key) const;
    Q_INVOKABLE void setColor(const QString& key, const QColor& color);
    
    ~ThemeManager() override = default;

signals:
    void currentThemeChanged();
    void themeColorsChanged();

private:
    ThemeManager();
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;
    
    void loadLightTheme();
    void loadDarkTheme();
    
    QString m_currentTheme;
    
    QColor m_primaryColor;
    QColor m_secondaryColor;
    QColor m_backgroundColor;
    QColor m_surfaceColor;
    QColor m_textColor;
    QColor m_accentColor;
    
    QHash<QString, QColor> m_customColors;
    
    static ThemeManager* s_instance;
};

} // namespace suno::core
#pragma once

#include <QObject>
#include <QImage>
#include <QPainter>
#include <QString>
#include <QFont>
#include <QColor>
#include <QHash>

namespace suno::video {

struct TextOverlay {
    QString text;
    QPointF position;
    QFont font;
    QColor color;
    Qt::Alignment alignment = Qt::AlignCenter;
    qreal opacity = 1.0;
    bool enabled = true;
};

class FrameCompositor : public QObject
{
    Q_OBJECT

public:
    explicit FrameCompositor(QObject* parent = nullptr);
    ~FrameCompositor() override = default;
    
    void setCanvasSize(int width, int height);
    
    // Layer composition
    QImage compositeFrame(const QImage& visualizerFrame);
    
    // Text overlay management
    void addTextOverlay(const QString& id, const TextOverlay& overlay);
    void removeTextOverlay(const QString& id);
    void updateTextOverlay(const QString& id, const TextOverlay& overlay);
    void clearTextOverlays();
    
    // Background
    void setBackgroundColor(const QColor& color);
    void setBackgroundImage(const QImage& image);
    
    // Utility
    Q_INVOKABLE void setLyricText(const QString& text);
    Q_INVOKABLE void setTrackInfo(const QString& title, const QString& artist);

signals:
    void frameComposited(const QImage& frame);

private:
    void drawTextOverlays(QPainter& painter);
    
    int m_width = 1920;
    int m_height = 1080;
    
    QColor m_backgroundColor = Qt::black;
    QImage m_backgroundImage;
    bool m_useBackgroundImage = false;
    
    QHash<QString, TextOverlay> m_textOverlays;
    
    // Predefined overlay IDs
    static constexpr const char* OVERLAY_LYRICS = "lyrics";
    static constexpr const char* OVERLAY_TITLE = "title";
    static constexpr const char* OVERLAY_ARTIST = "artist";
};

} // namespace suno::video
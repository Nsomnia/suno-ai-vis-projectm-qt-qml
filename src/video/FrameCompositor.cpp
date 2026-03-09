#include "FrameCompositor.h"
#include <spdlog/spdlog.h>

namespace suno::video {

FrameCompositor::FrameCompositor(QObject* parent)
    : QObject(parent)
{
    spdlog::debug("FrameCompositor created");
}

void FrameCompositor::setCanvasSize(int width, int height)
{
    if (m_width != width || m_height != height) {
        m_width = width;
        m_height = height;
        spdlog::info("Frame compositor canvas size set to {}x{}", width, height);
    }
}

QImage FrameCompositor::compositeFrame(const QImage& visualizerFrame)
{
    QImage outputFrame(m_width, m_height, QImage::Format_RGBA8888);
    QPainter painter(&outputFrame);
    
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    
    // Draw background
    if (m_useBackgroundImage && !m_backgroundImage.isNull()) {
        painter.drawImage(outputFrame.rect(), m_backgroundImage);
    } else {
        painter.fillRect(outputFrame.rect(), m_backgroundColor);
    }
    
    // Draw visualizer frame
    if (!visualizerFrame.isNull()) {
        QImage scaledViz = visualizerFrame.scaled(
            m_width, m_height, 
            Qt::KeepAspectRatio, 
            Qt::SmoothTransformation
        );
        
        int x = (m_width - scaledViz.width()) / 2;
        int y = (m_height - scaledViz.height()) / 2;
        
        painter.drawImage(x, y, scaledViz);
    }
    
    // Draw text overlays
    drawTextOverlays(painter);
    
    painter.end();
    
    emit frameComposited(outputFrame);
    
    return outputFrame;
}

void FrameCompositor::drawTextOverlays(QPainter& painter)
{
    for (auto it = m_textOverlays.constBegin(); it != m_textOverlays.constEnd(); ++it) {
        const TextOverlay& overlay = it.value();
        
        if (!overlay.enabled) {
            continue;
        }
        
        painter.save();
        
        painter.setFont(overlay.font);
        painter.setPen(overlay.color);
        painter.setOpacity(overlay.opacity);
        
        QFontMetrics fm(overlay.font);
        QRect textRect = fm.boundingRect(overlay.text);
        
        QPointF drawPos = overlay.position;
        
        // Apply alignment
        if (overlay.alignment & Qt::AlignHCenter) {
            drawPos.setX(drawPos.x() - textRect.width() / 2.0);
        }
        if (overlay.alignment & Qt::AlignVCenter) {
            drawPos.setY(drawPos.y() + textRect.height() / 2.0);
        }
        if (overlay.alignment & Qt::AlignRight) {
            drawPos.setX(drawPos.x() - textRect.width());
        }
        if (overlay.alignment & Qt::AlignBottom) {
            drawPos.setY(drawPos.y() + textRect.height());
        }
        
        painter.drawText(drawPos, overlay.text);
        
        painter.restore();
    }
}

void FrameCompositor::addTextOverlay(const QString& id, const TextOverlay& overlay)
{
    m_textOverlays[id] = overlay;
    spdlog::debug("Added text overlay: {}", id.toStdString());
}

void FrameCompositor::removeTextOverlay(const QString& id)
{
    if (m_textOverlays.remove(id)) {
        spdlog::debug("Removed text overlay: {}", id.toStdString());
    }
}

void FrameCompositor::updateTextOverlay(const QString& id, const TextOverlay& overlay)
{
    if (m_textOverlays.contains(id)) {
        m_textOverlays[id] = overlay;
    } else {
        addTextOverlay(id, overlay);
    }
}

void FrameCompositor::clearTextOverlays()
{
    m_textOverlays.clear();
    spdlog::debug("Cleared all text overlays");
}

void FrameCompositor::setBackgroundColor(const QColor& color)
{
    m_backgroundColor = color;
    m_useBackgroundImage = false;
}

void FrameCompositor::setBackgroundImage(const QImage& image)
{
    m_backgroundImage = image;
    m_useBackgroundImage = !image.isNull();
}

void FrameCompositor::setLyricText(const QString& text)
{
    TextOverlay overlay;
    overlay.text = text;
    overlay.position = QPointF(m_width / 2.0, m_height * 0.85);
    overlay.font = QFont("Arial", 36, QFont::Bold);
    overlay.color = Qt::white;
    overlay.alignment = Qt::AlignCenter;
    overlay.opacity = 0.95;
    
    updateTextOverlay(OVERLAY_LYRICS, overlay);
}

void FrameCompositor::setTrackInfo(const QString& title, const QString& artist)
{
    // Title overlay
    TextOverlay titleOverlay;
    titleOverlay.text = title;
    titleOverlay.position = QPointF(m_width / 2.0, 60);
    titleOverlay.font = QFont("Arial", 28, QFont::Bold);
    titleOverlay.color = Qt::white;
    titleOverlay.alignment = Qt::AlignCenter;
    titleOverlay.opacity = 0.9;
    
    updateTextOverlay(OVERLAY_TITLE, titleOverlay);
    
    // Artist overlay
    TextOverlay artistOverlay;
    artistOverlay.text = artist;
    artistOverlay.position = QPointF(m_width / 2.0, 100);
    artistOverlay.font = QFont("Arial", 20);
    artistOverlay.color = QColor(200, 200, 200);
    artistOverlay.alignment = Qt::AlignCenter;
    artistOverlay.opacity = 0.8;
    
    updateTextOverlay(OVERLAY_ARTIST, artistOverlay);
}

} // namespace suno::video
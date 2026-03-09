#include "ProjectMImageProvider.h"
#include "ProjectMWrapper.h"

namespace suno::visualizer {

ProjectMImageProvider::ProjectMImageProvider(ProjectMWrapper* visualizer)
    : QQuickImageProvider(QQuickImageProvider::Image)
    , m_visualizer(visualizer)
{
}

QImage ProjectMImageProvider::requestImage(const QString& id, QSize* size, const QSize& requestedSize)
{
    if (!m_visualizer || !m_visualizer->isInitialized()) {
        return QImage();
    }
    
    QImage frame = m_visualizer->grabFrameBuffer();
    
    if (frame.isNull()) {
        return QImage();
    }
    
    if (size) {
        *size = frame.size();
    }
    
    if (requestedSize.isValid() && requestedSize != frame.size()) {
        frame = frame.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    
    return frame;
}

} // namespace suno::visualizer
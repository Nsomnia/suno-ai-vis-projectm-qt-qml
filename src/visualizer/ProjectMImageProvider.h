#pragma once

#include <QQuickImageProvider>
#include <QImage>
#include <QQuickImageResponse>
#include <QRunnable>

namespace suno::visualizer {

class ProjectMWrapper;

class ProjectMImageProvider : public QQuickImageProvider
{
public:
    explicit ProjectMImageProvider(ProjectMWrapper* visualizer);
    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;

private:
    ProjectMWrapper* m_visualizer;
};

} // namespace suno::visualizer
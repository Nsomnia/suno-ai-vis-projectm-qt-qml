#pragma once

#include <QObject>
#include <QString>
#include <QImage>
#include <memory>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

namespace suno::video {

class VideoCapture : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isRecording READ isRecording NOTIFY recordingStateChanged)
    Q_PROPERTY(int frameCount READ frameCount NOTIFY frameCountChanged)
    Q_PROPERTY(QString outputPath READ outputPath NOTIFY outputPathChanged)

public:
    explicit VideoCapture(QObject* parent = nullptr);
    ~VideoCapture() override;
    
    bool initialize(const QString& outputPath, int width, int height, int fps, int bitrate);
    void shutdown();
    
    Q_INVOKABLE bool startRecording();
    Q_INVOKABLE void stopRecording();
    
    bool captureFrame(const QImage& frame);
    
    bool isRecording() const { return m_isRecording; }
    int frameCount() const { return m_frameCount; }
    QString outputPath() const { return m_outputPath; }

signals:
    void recordingStarted();
    void recordingStopped();
    void recordingStateChanged();
    void frameCountChanged(int count);
    void outputPathChanged();
    void errorOccurred(const QString& error);

private:
    bool openOutputFile();
    void closeOutputFile();
    bool writeFrame(AVFrame* frame);
    
    bool m_initialized = false;
    bool m_isRecording = false;
    int m_frameCount = 0;
    
    QString m_outputPath;
    int m_width = 1920;
    int m_height = 1080;
    int m_fps = 60;
    int m_bitrate = 8000000;
    
    AVFormatContext* m_formatContext = nullptr;
    AVCodecContext* m_codecContext = nullptr;
    AVStream* m_videoStream = nullptr;
    AVFrame* m_frame = nullptr;
    AVPacket* m_packet = nullptr;
    SwsContext* m_swsContext = nullptr;
    
    int64_t m_pts = 0;
};

} // namespace suno::video
#ifndef CVCAMERAVIEWER_H
#define CVCAMERAVIEWER_H

#include <functional>

#include <QObject>
#include <QTimer>

#include <QAbstractVideoSurface>

#include <opencv2/core/mat.hpp>

#include "videoprocessthread.h"

class CvCameraViewer : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QAbstractVideoSurface* videoSurface
               READ videoSurface
               WRITE setVideoSurface
               NOTIFY videoSurfaceChanged)
    Q_PROPERTY(bool connected
               READ connected
               // WRITE setConnected
               NOTIFY connectedChanged)

public:
    explicit CvCameraViewer(
            const QString& cameraAddress,
            std::function<void(cv::Mat, cv::Mat&)> task,
            QObject* parent = nullptr
            );
    explicit CvCameraViewer(
            int cameraIndex,
            std::function<void(cv::Mat, cv::Mat&)> task,
            QObject* parent = nullptr
            );
    void show();

    VideoProcessThread* videoThread() const;

public slots:
    void setVideoSurface(QAbstractVideoSurface *);
    QAbstractVideoSurface* videoSurface();

    bool connected() const;

    void processFrameSizeChange(int width, int height);

signals:
    void videoSurfaceChanged();
    void connectedChanged(bool connected);

private:
    void setConnected(bool connected);
    void initialize();

private:
    VideoProcessThread *_videoThread;
    QAbstractVideoSurface *_videoSurface;
    QVideoFrame _frame;
    cv::Mat _cvAliasToFrame;
    QTimer* _reconnectTimer;


    bool _connected;
};

#endif // CVCAMERAVIEWER_H

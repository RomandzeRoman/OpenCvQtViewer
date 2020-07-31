#include "cvcameraviewer.h"

#include <opencv2/opencv.hpp>

#include <QVideoSurfaceFormat>

#include "QtDebugPrint/debugoutput.h"

const int reconnectTimeout = 1000; // ms
const QVideoFrame::PixelFormat FRAME_FORMAT = QVideoFrame::Format_RGB32;

CvCameraViewer::CvCameraViewer(
        const QString& cameraAddress,
        std::function<void(cv::Mat input, cv::Mat& output)> task,
        QObject* parent
        )
    : QObject(parent)
    , _videoThread{new VideoProcessThread(cameraAddress, task, this)}
    , _frame()
    , _cvAliasToFrame()
    , _reconnectTimer{ new QTimer(this) }
    , _connected{false}
{
    initialize();
}

CvCameraViewer::CvCameraViewer(
        int cameraIndex,
        std::function<void(cv::Mat input, cv::Mat& output)> task,
        QObject* parent
        )
    : QObject(parent)
    , _videoThread{new VideoProcessThread(cameraIndex, task, this)}
    , _frame()
    , _cvAliasToFrame()
    , _reconnectTimer{ new QTimer(this) }
    , _connected{false}
{
    initialize();
}

/********************************************************************
 * Public
 ********************************************************************
 */

void CvCameraViewer::show() {
    Q_ASSERT(_videoSurface != nullptr);
    if (!_videoSurface->isActive()) {
        //dbg << "not active";

        if (!_videoSurface->start(
                QVideoSurfaceFormat(
                    QSize(_frame.width(),_frame.height()),
                    FRAME_FORMAT
                    )
                )
            )
        {
            dbg << "start error" << _videoSurface->error();
        }
    }

    _videoThread->copyReceivedFrameTo(_cvAliasToFrame);

    _reconnectTimer->start();

    if (Q_LIKELY(_videoSurface)) {
        _videoSurface->present(_frame);
    }

}

VideoProcessThread* CvCameraViewer::videoThread() const {
    return _videoThread;
}

/********************************************************************
 * Slots
 ********************************************************************
 */

void CvCameraViewer::setVideoSurface(
        QAbstractVideoSurface *vs
        )
{
    _videoSurface = vs;
}

QAbstractVideoSurface* CvCameraViewer::videoSurface() {
    return _videoSurface;
}

bool CvCameraViewer::connected() const {
    return _connected;
}


void CvCameraViewer::processFrameSizeChange(
        int width, int height
        )
{
    _frame = QVideoFrame(
                width*height*4,
                QSize(width,height),
                width*4,FRAME_FORMAT
                );
    if (_frame.isMapped()) {
        _frame.unmap();
    }
    _frame.map(QAbstractVideoBuffer::ReadOnly);

    _cvAliasToFrame = cv::Mat(
                cv::Size(_frame.width(), _frame.height()),
                CV_8UC4, _frame.bits()
                );
}

/********************************************************************
 * Private
 ********************************************************************
 */

void CvCameraViewer::setConnected(bool connected) {
    if (connected != _connected) {
        _connected = connected;
        emit connectedChanged(connected);
    }
}

void CvCameraViewer::initialize() {
    _reconnectTimer->setSingleShot(true);
    _reconnectTimer->setInterval(reconnectTimeout);
    connect(_reconnectTimer,
            &QTimer::timeout,
            _videoThread,
            &VideoProcessThread::processReconnectTimeout);

    connect(_videoThread, &VideoProcessThread::frameReceived,
            this, &CvCameraViewer::show);
    connect(_videoThread, &VideoProcessThread::connectedChanged,
            this, &CvCameraViewer::setConnected);
    connect(_videoThread, &VideoProcessThread::frameSizeChanged,
            this, &CvCameraViewer::processFrameSizeChange);
    _videoThread->start();
}

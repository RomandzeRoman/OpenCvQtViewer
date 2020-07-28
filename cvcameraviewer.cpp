#include "cvcameraviewer.h"

//#include <opencv2/imgproc.hpp> // cvtColor
//#include <opencv2/imgproc/types_c.h> //cv_bgr2rgb
#include <opencv2/opencv.hpp>
//#include <opencv2/core/hal/interface.h> // cv_8uc3
//#include <opencv2/core/cvdef.h>

//#include <opencv2/highgui.hpp>

#include <QVideoSurfaceFormat>

//#include "debugoutput.h"

#define FRAME_FORMAT QVideoFrame::Format_RGB32

CvCameraViewer::CvCameraViewer(
        const QString &cameraAddress,
        QObject *parent
        )
    : QObject(parent)
    , _videoThread{new VideoProcessThread(cameraAddress, this)}
    //, _frame(width*height*4, QSize(width,height),width*4,FRAME_FORMAT)
    , _frame()
    , _cvAliasToFrame()
    , _reconnectTimer{ new QTimer(this) }
    , _connected{false}
{
    initialize();
}

CvCameraViewer::CvCameraViewer(
        int cameraIndex,
        QObject *parent
        )
    : QObject(parent)
    , _videoThread{new VideoProcessThread(cameraIndex, this)}
    //, _frame(width*height*4, QSize(width,height),width*4,FRAME_FORMAT)
    , _frame()
    , _cvAliasToFrame()
    , _reconnectTimer{ new QTimer(this) }
    , _connected{false}
{
    initialize();
}

/** *****************************************************************
 * Public
 * ******************************************************************
 */

void CvCameraViewer::show() {
    Q_ASSERT(_videoSurface != nullptr);
    if (!_videoSurface->isActive()) {
        //dbg << "not active";

        if (!_videoSurface->start(
                    QVideoSurfaceFormat(
                        QSize(
                            _frame.width(),
                            _frame.height()
                            ),
                        FRAME_FORMAT
                        )
                    )
                )
        {
            //dbg << "start error" << _videoSurface->error();
            //
        }
    }

    //cv::Mat cvAliasToFrame(cv::Size(img.cols, img.rows), CV_8UC4, _frame.bits());

    //cv::cvtColor(, _cvAliasToFrame, cv::COLOR_RGB2RGBA);

    //cv::Mat img = _videoThread->receivedFrame();

    _videoThread->copyReceivedFrameTo(_cvAliasToFrame);

    _reconnectTimer->start();

    if (Q_LIKELY(_videoSurface)) {
        _videoSurface->present(_frame);
    }

}

VideoProcessThread* CvCameraViewer::videoThread() const {
    return _videoThread;
}

/* ******************************************************************
 * Slots
 * ******************************************************************
 */

void CvCameraViewer::setVideoSurface(
        QAbstractVideoSurface *vs
        )
{
    _videoSurface = vs;
    //dbg << "Was set";
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
                cv::Size(
                    _frame.width(), _frame.height()
                    ),
                CV_8UC4, _frame.bits()
                );
}

/* ******************************************************************
 * Private
 * ******************************************************************
 */

void CvCameraViewer::setConnected(bool connected) {
    //dbg << "set camera connected" << connected;
    if (connected != _connected) {
        _connected = connected;
        emit connectedChanged(connected);
    }
}

void CvCameraViewer::initialize() {
    _reconnectTimer->setSingleShot(true);
    _reconnectTimer->setInterval(1000);
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

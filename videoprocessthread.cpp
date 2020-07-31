#include "videoprocessthread.h"

//#include <opencv2/imgproc.hpp> // cvtColor
#include <opencv2/opencv.hpp>
//#include <opencv2/core/hal/interface.h> // cv_8uc3

#include "QtDebugPrint/debugoutput.h"

cv::Scalar randomColor(cv::RNG& rng);

VideoProcessThread::VideoProcessThread(
        const QString& cameraAddress,
        std::function<void(cv::Mat, cv::Mat&)> task,
        QObject* parent
        )
    : QThread(parent)
    , _mutex()
    , _capture(new cv::VideoCapture(qPrintable(cameraAddress)))
    , _receivedFrame{}
    , _processedFrame{}
    , _task{ task }
    , _cameraAddress(cameraAddress)
    , _cameraIndex{ -1 }
    , _width{ 0 }
    , _height{ 0 }
    , _laserCount{ 0 }
    , _abort{ false }
    , _connected{false}
{

}

VideoProcessThread::VideoProcessThread(
        int cameraIndex,
        std::function<void(cv::Mat, cv::Mat&)> task,
        QObject* parent)
    : QThread(parent)
    , _mutex()
    , _capture(new cv::VideoCapture(cameraIndex))
    , _receivedFrame{}
    , _processedFrame{}
    , _task{ task }
    , _cameraAddress()
    , _cameraIndex{cameraIndex}
    , _width{0}
    , _height{0}
    , _laserCount{0}
    , _abort{ false }
    , _connected{false}
{

}

VideoProcessThread::~VideoProcessThread() {
    delete _capture;
    _mutex.lock();
    _abort = true;
    _mutex.unlock();

    wait();
}

bool VideoProcessThread::connected() const {
    return _connected;
}

void VideoProcessThread::copyReceivedFrameTo(cv::OutputArray frame) {
    //dbg << "processing frame";
    _mutex.lock();
    _processedFrame.copyTo(frame);
    _mutex.unlock();
    //dbg << "frame processed";
}

void VideoProcessThread::run() {
    cv::Mat blured, binarizedFrame, processed, small;
    int64 tick = cv::getTickCount();
    double t(0.0);
    int fps, momentFps = 0;
    forever {
        if (_abort) {
            return;
        }
        //dbg << "cycle";
        if (_capture == nullptr) {
            //dbg << "nullptr";
            continue;
        }
        if (!_capture->isOpened()) {
            //dbg << "not opened";
            setConnected(false);
            if (_cameraIndex == -1) {
                _capture->open(qPrintable(_cameraAddress));
            }
            else {
                _capture->open(_cameraIndex);
            }
            continue;
        }
        setConnected(true);
        if (_capture->grab()) {
            //dbg << "retrieving";
            _capture->retrieve(_receivedFrame);
            if (_receivedFrame.cols != _width
                || _receivedFrame.rows != _height
                    )
            {
                _width = _receivedFrame.cols;
                _height = _receivedFrame.rows;
                emit frameSizeChanged(_width, _height);
            }

            _task(_receivedFrame, processed);

//            emit laserCountChanged(contoursSize);

            t = double(cv::getTickCount() - tick);
            tick = cv::getTickCount();
            momentFps = int(cv::getTickFrequency()/t);
            fps = 0.9*fps + 0.1*momentFps;
            cv::putText(processed, std::to_string(fps), cv::Point(0,50), cv::FONT_HERSHEY_SIMPLEX, 1.4, cv::Scalar(0,255,255), 3);

            _mutex.lock();
            _processedFrame = processed.clone();
            _mutex.unlock();
            emit frameReceived();
        }
    }
}


void VideoProcessThread::processReconnectTimeout() {
    setConnected(false);
    //return;
    dbg << "reconnect";
    delete _capture;
    _capture = nullptr;
    dbg << "released";
    if (_cameraIndex == -1) {
        _capture = new cv::VideoCapture(qPrintable(_cameraAddress));
    }
    else {
        _capture = new cv::VideoCapture(_cameraIndex);
    }
    dbg << "created";
}


/********************************************************************
 * Private
 ********************************************************************
 */

void VideoProcessThread::setConnected(bool connected) {
    if (connected != _connected) {
        _mutex.lock();
        _connected = connected;
        _mutex.unlock();
        emit connectedChanged(connected);
    }
}

void VideoProcessThread::setLaserCount(int laserCount) {
    if (_laserCount != laserCount) {
        _laserCount = laserCount;
        emit laserCountChanged(laserCount);
    }
}


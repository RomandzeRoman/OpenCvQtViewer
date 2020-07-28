#include "videoprocessthread.h"

//#include <opencv2/imgproc.hpp> // cvtColor
#include <opencv2/opencv.hpp>
//#include <opencv2/core/hal/interface.h> // cv_8uc3

#include "debugoutput.h"

const int threshold = 190;//208;//190;//174;
const int blur = 3;//5;//3;
const double scale = 2.0;

cv::Scalar randomColor(cv::RNG& rng);

VideoProcessThread::VideoProcessThread(
        const QString &cameraAddress,
        QObject *parent
        )
    : QThread(parent)
    , _mutex()
    , _capture(new cv::VideoCapture(qPrintable(cameraAddress)/*qPrintable(cameraAddress)*/))
    , _receivedFrame{}
    , _processedFrame{}
    , _cameraAddress(cameraAddress)
    , _cameraIndex{-1}
    , _width{0}
    , _height{0}
    , _laserCount{0}
    , _abort{ false }
    , _connected{false}
{

}

VideoProcessThread::VideoProcessThread(int cameraIndex, QObject* parent)
    : QThread(parent)
    , _mutex()
    , _capture(new cv::VideoCapture(cameraIndex))
    , _receivedFrame{}
    , _processedFrame{}
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
    int fps = 0;
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

            // Delete noise for small
            double fx = 1 / scale;
            //cv::resize( _receivedFrame, small, cv::Size(), fx, fx, cv::INTER_LINEAR );
            //cv::medianBlur(small, blured, blur);

            // Delete noise for big
            cv::medianBlur(_receivedFrame, blured, blur);

            cv::cvtColor(blured, binarizedFrame, cv::COLOR_RGB2GRAY );
            cv::threshold(binarizedFrame, binarizedFrame, threshold, 255.0, cv::THRESH_BINARY);

            std::vector<std::vector<cv::Point>> contours;
            cv::findContours(binarizedFrame, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

            processed = _receivedFrame.clone();
            t = double(cv::getTickCount() - tick);
            tick = cv::getTickCount();
            fps = cv::getTickFrequency()/t;//timeInMs = t*1000/getTickFrequency();
            //fps = t*1000/cv::getTickFrequency();
            cv::putText(processed, std::to_string(fps), cv::Point(0,50), cv::FONT_HERSHEY_SIMPLEX, 1.4, cv::Scalar(0,255,255), 3);


            int contoursSize = int(contours.size());
            cv::putText(processed, std::to_string(contoursSize),
                        cv::Point(0,100), cv::FONT_HERSHEY_SIMPLEX, 1.4, cv::Scalar(0,0,255), 3);

            cv::RNG rng(2);
            for (int i = 0; i < contoursSize; ++i) {
                cv::drawContours(processed, contours, i, cv::Scalar(0,0,255),4);
            }
            emit laserCountChanged(contoursSize);

            _mutex.lock();
            //cv::cvtColor(binarizedFrame, _processedFrame, cv::COLOR_RGB2RGBA);
            cv::cvtColor(processed, _processedFrame, cv::COLOR_RGB2RGBA);
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

//const cv::Mat &VideoProcessThread::receivedFrame() const {
//    return _processedFrame;
//}

/** *****************************************************************
 * Private
 * ******************************************************************
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

cv::Scalar randomColor(cv::RNG& rng) {
    return cv::Scalar(rng.uniform(0.0,255.0), rng.uniform(0.0,255.0), rng.uniform(0.0,255.0));
}

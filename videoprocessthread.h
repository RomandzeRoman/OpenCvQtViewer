#ifndef VIDEOPROCESSTHREAD_H
#define VIDEOPROCESSTHREAD_H

#include <QObject>

#include <QThread>
#include <QMutex>

#include <opencv2/core/mat.hpp>
#include <opencv2/videoio/videoio.hpp>

class VideoProcessThread : public QThread
{
    Q_OBJECT

    Q_PROPERTY(bool connected
               READ connected
               NOTIFY connectedChanged)

public:
    explicit VideoProcessThread(
            const QString& cameraAddress,
            std::function<void(cv::Mat input, cv::Mat& output)> task,
            QObject* parent = nullptr
            );
    explicit VideoProcessThread(
            int cameraIndex,
            std::function<void(cv::Mat input, cv::Mat& output)> task,
            QObject* parent = nullptr
            );
    ~VideoProcessThread() override;

    void copyReceivedFrameTo(cv::OutputArray frame);

public slots:
    bool connected() const;
    void processReconnectTimeout();

signals:
    void frameReceived();
    void connectedChanged(bool connected);

    void frameSizeChanged(int width, int height);
    void laserCountChanged(int laserCount);

protected:
    virtual void run() override final;

private slots:

private:
    void setConnected(bool connected);
    void setLaserCount(int laserCount);

private:
    QMutex _mutex;
    cv::VideoCapture* _capture;
    cv::Mat _receivedFrame;
    cv::Mat _processedFrame;

    std::function<void(cv::Mat input, cv::Mat& output)> _task;
    const QString _cameraAddress;
    const int _cameraIndex;
    int _width;
    int _height;

    int _laserCount;

    bool _abort;
    bool _connected;
};

#endif // VIDEOPROCESSTHREAD_H

# Install
Add module to .pro file
```
include(OpenCvQtViewer/OpenCvQtViewer.pri)
```
# Usage
in main.cpp
```
...
QQmlApplicationEngine engine;

CvCameraViewer *viewer0 = new CvCameraViewer(0);
engine.rootContext()->setContextProperty("camera0Viewer", viewer0);
CvCameraViewer *viewer1 = new CvCameraViewer(1);
engine.rootContext()->setContextProperty("camera1Viewer", viewer1);
...
```
In .qml:
```
VideoViewer {
    id: video0
    anchors {
        left: parent.left;
        top: parent.top;
        bottom: parent.bottom;
        right: parent.horizontalCenter;
    }
    source: camera0Viewer
}
VideoViewer {
    id: video1
    anchors {
        right: parent.right;
        top: parent.top;
        bottom: parent.bottom;
        left: parent.horizontalCenter;
    }
    source: camera1Viewer
}
```
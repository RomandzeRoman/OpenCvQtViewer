QML_IMPORT_PATH += $$PWD/../
OTHER_FILES += \
    $$PWD/README.md \
    $$PWD/VideoViewer.qml \
    $$PWD/qmldir

RESOURCES += \
    $$PWD/VideoViewer.qml \
    $$PWD/qmldir

HEADERS += \
    $$PWD/cvcameraviewer.h \
    $$PWD/videoprocessthread.h

SOURCES += \
    $$PWD/cvcameraviewer.cpp \
    $$PWD/videoprocessthread.cpp

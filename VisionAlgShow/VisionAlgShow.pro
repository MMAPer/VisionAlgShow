#-------------------------------------------------
#
# Project created by QtCreator 2018-03-02T15:38:38
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VisionAlgShow
TEMPLATE = app

DESTDIR += ./bin
OBJECTS_DIR += ./tmp

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        visionalgmain.cpp \
    realplay.cpp \
    frameplaywnd.cpp \
    offline.cpp \
    tracking.cpp \
    camera/camera.cpp \
    camera/channeldata.cpp \
    camera/devicedata.cpp \
    camera/treeitem.cpp \
    camera/treemodel.cpp \
    utils/iconhelper.cpp \
    utils/myapp.cpp \
    detect/mydetect.cpp \
    sottracker/sottracker.cpp \
    mottracker/mottracker.cpp \
    algorithms/detect.cpp

HEADERS  += visionalgmain.h \
    realplay.h \
    frameplaywnd.h \
    Public.h \
    offline.h \
    tracking.h \
    camera/camera.h \
    camera/channeldata.h \
    camera/devicedata.h \
    camera/treeitem.h \
    camera/treemodel.h \
    utils/iconhelper.h \
    utils/myapp.h \
    utils/myhelper.h \
    detect/mydetect.h \
    sottracker/sottracker.h \
    mottracker/mottracker.h \
    utils/parsehelper.h \
    algorithms/detect.h

FORMS    += visionalgmain.ui \
    realplay.ui \
    frameplaywnd.ui \
    offline.ui \
    tracking.ui

win32:{
RC_FILE=main.rc
}

unix:{
DISTFILES +=
}

RESOURCES += \
    rc.qrc

#HCNetSDK
INCLUDEPATH += ../linux64/incCn
LIBS += -L../linux64/lib/ -Wl,-rpath=./:./HCNetSDKCom:../linux64/lib -lhcnetsdk -lPlayCtrl -lAudioRender -lSuperRender

#OpenCV3.4.1
INCLUDEPATH += /usr/local/opencv3.4.1/include \
               /usr/local/opencv3.4.1/include/opencv \
               /usr/local/opencv3.4.1/include/opencv2
LIBS += /usr/local/opencv3.4.1/lib/libopencv_*.so




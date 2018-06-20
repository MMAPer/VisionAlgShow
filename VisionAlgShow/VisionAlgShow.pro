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
    myapp.cpp \
    iconhelper.cpp \
    realplay.cpp \
    frameplaywnd.cpp \
    channeldata.cpp \
    devicedata.cpp \
    treeitem.cpp \
    treemodel.cpp \
    offline.cpp \
    tracking.cpp \
    camera.cpp

HEADERS  += visionalgmain.h \
    myapp.h \
    myhelper.h \
    iconhelper.h \
    realplay.h \
    frameplaywnd.h \
    channeldata.h \
    devicedata.h \
    Public.h \
    treeitem.h \
    treemodel.h \
    offline.h \
    tracking.h \
    camera.h

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




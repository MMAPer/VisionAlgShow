#ifndef OFFLINE_H
#define OFFLINE_H

#include <QDialog>
#include <QMenu>
#include <QTimer>
#include <QList>
#include <QComboBox>
#include <QModelIndex>
#include <QAction>
#include <QString>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QStandardItem>
#include <QLabel>

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/dnn/all_layers.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/dnn/shape_utils.hpp>
#include <opencv2/dpm.hpp>
#include <opencv2/core.hpp>
#include <opencv2/video/background_segm.hpp>
//#include<opencv2/cudaimgproc.hpp>
//#include <opencv2/cudacodec.hpp>

#include <vector>
#include <string>
#include <iostream>
#include <time.h>

#include "public.h"
#include "camera/devicedata.h"
#include "camera/channeldata.h"
#include "camera/treeitem.h"
#include "camera/treemodel.h"
#include "algorithms/yolov2.h"
#include "algorithms/mot/IOU/iou.h"
#include "utils/iconhelper.h"
#include "utils/myapp.h"
#include "algorithms/detect.h"



namespace Ui {
class offline;
}

class offline : public QDialog
{
    Q_OBJECT

public:
    explicit offline(QWidget *parent = 0);
    ~offline();
    int videoFlag=0;  //0:image  1:video
    QString filePath;
    QString fileSuffix;

    YOLO_V2 *yoloDetector;
    IOUTracker *iouTracker;
    //cv::cuda::GpuMat g_frame;
    //cv::Ptr<cv::cudacodec::VideoReader> g_reader;

protected:
    bool eventFilter(QObject *obj, QEvent *event);


private slots:
    void InitStyle();
    void InitEvent();
    void od_alg_clicked(const QString od_alg);

    void on_btn_open_clicked();
    void playbyframe();

    void playYoloV2Detect();

    //object detection algorithms
    void od_alg_hog();
    void od_alg_dpm();
    void od_alg_faster_rcnn();
    void od_alg_ssd();
    void od_alg_yolo();

    void bgSubtraction();

    void show_video_1();   //切换到1画面
    void show_video_4();   //切换到4画面

private:
    Ui::offline *ui;

    //2018.3.21 by hbc
    cv::VideoCapture capture;
    QImage Mat2QImage(cv::Mat cvImage);
    cv::Mat frame;
    QImage  image;
    QTimer *timer;
    double rate; //FPS
    cv::VideoWriter writer;   //make a video record

    QComboBox *cbox_bg;  //背景建模下拉按钮
    QComboBox *cbox_od;  //目标检测下拉按钮
    QComboBox *cbox_sot;  //单目标跟踪下拉按钮
    QComboBox *cbox_sm;  //衣物解析下拉按钮

    void removeLayout();
    void change_video_1(int index=0);  //改变1画面布局
    void change_video_4(int index=0);  //改变4画面布局

    void InitOfflineVideo();  //初始化视频布局载体数据
    bool video_max;
    int offlineWindowNum;  //窗口数
    QLabel *offlineTempLabel;  //临时播放视频的标签
    QList<QLabel *> offlineVideoLabel;  //通道显示视频lab载体
    QList<QLayout *> offlineVideoLayout;  //通道视频所在lab的layout
    QMenu *offlineMenu; //鼠标右键菜单

    void selectScreen(QImage im,int sc);
    int screenCount=0;  // 多屏播放时指定是哪块屏
};

#endif // OFFLINE_H


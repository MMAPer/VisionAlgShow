#ifndef OFFLINE_H
#define OFFLINE_H

#include <QDialog>
#include "public.h"
#include <QDialog>
#include <QList>
#include <QComboBox>
#include <QModelIndex>
#include <vector>
#include <string>
#include "camera/devicedata.h"
#include "camera/channeldata.h"
#include "camera/treeitem.h"
#include "camera/treemodel.h"
#include "realplay.h"
#include "opencv2/opencv.hpp"
#include <QLabel>
#include "algorithms/yolov2.h"
#include "tracker/IOU/iou.h"



namespace Ui {
class offline;
}

class offline : public QDialog
{
    Q_OBJECT

public:
    explicit offline(QWidget *parent = 0);
    ~offline();
    int m_gsdkinit;  //sdk是否初始化标志
    int m_bislogin = FALSE;  //是否登录
    int m_iposttreelevel;  //点击的树节点的层级
    RealPlay *m_realplay;  //中间的播放窗口RealPlay
    TreeModel *m_gmodel;  //自定义数据类型
    QModelIndex m_qtreemodelindex;  //树点击的索引
    QList<DeviceData> m_qlistdevicedata;  //DeviceData链表
    int m_gcurrentuserid;  //当前登录的用户ID
    int m_guseridbackup;  //用户ID备份
    NET_DVR_DEVICEINFO_V30 m_gcurrentdeviceinfo;  //海康官方返回的设备信息，注意设备和通道的区别
    int m_gcurrentchannelnum;  //当前通道
    int m_gchannelnumbackup;  //当前通道备份，用于作比较的
    int m_gcurrentchannellinkmode;  //通道链接模式
    int m_rpwindownumindex;  //窗口数量选择按钮的索引
    int videoFlag=0;
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
    int screenCount=0;




};

#endif // OFFLINE_H


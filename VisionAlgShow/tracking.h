#ifndef TRACKING_H
#define TRACKING_H

#include <QDialog>
#include "Public.h"
#include <QDialog>
#include <QList>
#include <QComboBox>
#include <QModelIndex>
#include <vector>
#include <string>
#include "devicedata.h"
#include "channeldata.h"
#include "treeitem.h"
#include "treemodel.h"
#include "realplay.h"
#include "opencv2/opencv.hpp"


namespace Ui {
class tracking;
}

class tracking : public QDialog
{
    Q_OBJECT

public:
    explicit tracking(QWidget *parent = 0);
    ~tracking();
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
    QString fileName;
    QString fileSuffix;


private slots:
    void InitStyle();
    void InitEvent();

    void on_btn_back_clicked();
    void on_btn_open_clicked();
    void playbyframe();
    void on_btn_faster_rcnn_clicked();

    void on_btn_ssd_clicked();

    void on_btn_hog_clicked();

    void on_btn_yolo_clicked();

    void on_btn_dpm_clicked();

    void bgModel();

private:
    Ui::tracking *ui;

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

};

#endif // OFFLINE_H


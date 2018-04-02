#ifndef OFFLINE_H
#define OFFLINE_H

#include <QDialog>
#include "Public.h"
#include <QDialog>
#include <QList>
#include <QModelIndex>
#include "devicedata.h"
#include "channeldata.h"
#include "treeitem.h"
#include "treemodel.h"
#include "realplay.h"
#include "opencv2/opencv.hpp"


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


private slots:
    void receiveofflinehandle();
    void InitStyle();

    void on_btn_back_clicked();
    void on_btn_open_clicked();
    void playbyframe();
    void on_btn_faster_rcnn_clicked();

signals:
    void backmain();
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
};

#endif // OFFLINE_H


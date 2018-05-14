#ifndef VISIONALGMAIN_H
#define VISIONALGMAIN_H

#include <QDialog>
#include <QFileDialog>
#include <QLabel>
#include <QModelIndex>
#include <QMenu>
#include <QLayout>
#include <QAction>
#include <QString>
#include <QStandardItem>
#include "Public.h"
#include "devicedata.h"
#include "channeldata.h"
#include "treeitem.h"
#include "treemodel.h"
#include "realplay.h"
#include <QDebug>
#include <QMessageBox>

/* 说明:主程序
 * 功能:系统运行主程序
 * 作者:张精制  QQ:852370792
 * 时间:2018-03-10  检查:2018-3-10
 */


namespace Ui {
class VisionAlgMain;
}

class VisionAlgMain : public QDialog
{
    Q_OBJECT

public:
    explicit VisionAlgMain(QWidget *parent = 0);
    ~VisionAlgMain();

    //控制标识符
    int m_gsdkinit;  //sdk是否初始化标志
    int m_bislogin = FALSE;  //是否登录
    int m_iposttreelevel;  //点击的树节点的层级
    int m_rpwindownumindex;  //窗口数量选择按钮的索引
    QModelIndex m_qtreemodelindex;  //树点击的索引

    //用户及摄像头设备信息
    int m_gcurrentuserid;  //当前登录的用户ID
    int m_guseridbackup;  //用户ID备份
    int m_gcurrentchannelnum;  //当前通道
    int m_gchannelnumbackup;  //当前通道备份，用于作比较的
    int m_gcurrentchannellinkmode;  //通道链接模式
    NET_DVR_DEVICEINFO_V30 m_gcurrentdeviceinfo;  //当前设备信息

    TreeModel *m_gmodel;  //自定义数据类型，包括设备及通道数据的封装
    QList<DeviceData> m_qlistdevicedata;  //DeviceData链表

    void showDeviceTree(const QString &nodedata);  //根据字符串显示左侧树结构
    QString getStringFromList(QList<DeviceData> &data);

    //实时播放控制
    int m_rpstartstopflag;  //停止播放为0，开始播放为1
    int m_rpuseridbackup;  //实时userid备份
    int m_rpfirstrealhandle;  //第一次播放窗口句柄
    int m_rpcurrentrealhandle;  //当前播放窗口句柄

//槽函数
private slots:
    void login();  //登录
    void on_btnMenu_Full_clicked();  //进入全屏模式
    void screen_normal();  //普通模式
    void on_btnMenu_Setting_clicked();  //系统设置
    void on_btnMenu_Logout_clicked();  //注销
    void on_btnMenu_Min_clicked();  //最小化
    void on_btnMenu_Close_clicked();  //关闭
    void offlinehandle();  //离线处理

    //切换屏幕
    void show_video_1();  //切换到1画面
    void show_video_4();   //切换到4画面
    void show_video_9();   //切换到9画面
    void show_video_16();   //切换到16画面

    //左侧树形结构
    void pressedTreeView(const QModelIndex &);  //用户点击树的结点时，要对相应的索引标识符做改变
    void OnDoubleClickTree(const QModelIndex &);  //双击播放

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void keyPressEvent(QKeyEvent *event);  //键盘事件

private:
    Ui::VisionAlgMain *ui;

    void InitData();  //初始化用户及摄像头设备数据
    void InitStyle();  //初始化样式
    void InitVideo(); //初始化视频布局载体数据
    void InitSdk();  //初始化海康SDK
    void InitSlot();  //初始化事件绑定
    void removelayout();  //移除所有布局
    void change_video_1(int index=0);  //改变1画面布局
    void change_video_4(int index=0);  //改变4画面布局
    void change_video_9(int index=0);  //改变9画面布局
    void change_video_16(int index=0);  //改变16画面布局

    void realplay();  //双击树结点执行的函数
    void startRealPlay();  //开始实时播放
    void stopRealPlay();  //停止实时播放
    int realPlayEncapseInterface(int devicerow, int channelrow, NET_DVR_CLIENTINFO *clientinfo);
    void stopRealPlayEncapseInterface();

    bool video_max;
    int windowNum;  //窗口数
    QMenu *menu;  //鼠标右键菜单对象
    QLabel *tempLab;  //临时播放视频的标签
    QList<QLabel *> VideoLab;  //通道显示视频lab载体
    QList<QLayout *>VideoLay;  //通道视频所在lab的layout

};

#endif // VISIONALGMAIN_H

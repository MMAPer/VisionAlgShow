#ifndef VISIONALGMAIN_H
#define VISIONALGMAIN_H

#include <QDialog>
#include <QFileDialog>
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

class QLabel;
class QModelIndex;
class QMenu;
class QLayout;
class QAction;
class QString;
class QStandardItem;


namespace Ui {
class VisionAlgMain;
}

class VisionAlgMain : public QDialog
{
    Q_OBJECT

public:
    explicit VisionAlgMain(QWidget *parent = 0);
    ~VisionAlgMain();

    void showDeviceTree(const QString &nodedata);
    QString getStringFromList(QList<DeviceData> &data);


    int m_gsdkinit;  //sdk是否初始化标志
    int m_bislogin = FALSE;  //是否登录
    int m_iposttreelevel;  //点击的树节点的层级

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
    void on_btnMenu_Close_clicked();  //关闭
    void on_btnMenu_Min_clicked();  //最小化
    void on_btnMenu_Login_clicked();  //登录
    void on_btnMenu_Logout_clicked();  //注销
    void receiveback(); //接收返回

    //切换屏幕
    void show_video_1();  //切换到1画面
    void show_video_4();   //切换到4画面
    void show_video_9();   //切换到9画面
    void show_video_16();   //切换到16画面

    void screen_full();  //进入全屏模式
    void screen_normal();  //普通模式


    //左侧树形结构
    void pressedTreeView(const QModelIndex &);  //用户点击树的结点时，要对相应的索引标识符做改变
    void OnDoubleClickTree(const QModelIndex &);  //双击播放


    void on_btn_offlinehandle_clicked();

signals:
    void offlineHandle();   //跳转到离线处理界面

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void keyPressEvent(QKeyEvent *event);

private:
    Ui::VisionAlgMain *ui;

    void InitStyle();
    void InitVideo();               //初始化视频布局载体数据
    void InitSdk();
    void InitSlot();
    void removelayout();  //移除所有布局
    void change_video_1(int index=0);  //改变1画面布局
    void change_video_4(int index=0);  //改变4画面布局
    void change_video_9(int index=0);  //改变9画面布局
    void change_video_16(int index=0);  //改变16画面布局

    bool video_max;
    QMenu *menu;  //鼠标右键菜单对象
    QLabel *tempLab;  //临时播放视频的标签
    QList<QLabel *> VideoLab;  //通道显示视频lab载体
    QList<QLayout *>VideoLay;  //通道视频所在lab的layout

    QMenu* menu_window_num;
    QAction* m_one;
    QAction* m_four;
    QAction* m_nine;
    QAction* m_sixteen;
};

#endif // VISIONALGMAIN_H

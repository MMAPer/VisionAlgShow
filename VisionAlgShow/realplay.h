#ifndef REALPLAY_H
#define REALPLAY_H

#include <QtGui>
#include <QObject>
#include <QWidget>
#include "Public.h"
#include <QMenu>
#include <QDebug>
#include <QList>
#include "frameplaywnd.h"
#include "treemodel.h"
#include "devicedata.h"
#include "channeldata.h"

//Max num of windows used to preview
#define REALPLAY_MAX_NUM 16

/* 说明:播放区域
 * 功能:播放部分比较复杂，所以单独分出来，该部分处理屏幕切换等功能
 * 作者:张精制  QQ:852370792
 * 时间:2018-03-10  检查:2018-3-10
 */
namespace Ui {
class RealPlay;
}

class RealPlay : public QWidget
{
    Q_OBJECT

public:
    friend class VisionAlgMain;
    RealPlay(TreeModel **model, const QModelIndex *index, QList<DeviceData> *tree,
                      int *userid, NET_DVR_DEVICEINFO_V30 *devinfo, int *channelnum, int *channelmode,
                      QWidget *parent = 0);
    ~RealPlay();

    //play
    void startRealPlay(int index);
    void stopRealPlay();
    int realPlayEncapseInterface(int devicerow, int channelrow, NET_DVR_CLIENTINFO *clientinfo);
    void stopRealPlayEncapseInterface();

signals:
    //有播放的，告诉主界面切换成暂停按钮；不播放，告诉主界面切换成播放按钮
    //flag=1有播放；flag=0不播放了
    void PlayOrPauseSig(int flag);

private slots:
    void winNumChanged(int winNumIndex);  //更换屏幕数

    //click play windows.
    void OnSingleClickPlayWindow();
//    void RealPlayFullFrameRep();
//    void RealPlayFullScreenRep();
//    void RealPlayNormalFrameRep();
//    void RealPlayContextResponse(const QPoint &pos);

    void on_pushButton_realplay_clicked();


private:
    Ui::RealPlay *ui;

    QFrame *m_pframePlay;  //Window which is used to contain playWnd or is used to play.
    FramePlayWnd *m_pSelectedPlayWnd;
    FramePlayWnd *m_framePlayWnd[REALPLAY_MAX_NUM];


#ifdef __linux__
    //draw area initial
    INITINFO m_rpintinfo;
#endif
    QMenu *m_rpmenu;
    QList<DeviceData> *m_qlistdevicetree;
    int *m_rpuserid;
    NET_DVR_DEVICEINFO_V30 *m_rpdeviceinfo;
    int *m_rpchannelnum;
    int *m_channellinkmode;
    //after begin value is 1 while stop is 0
    int m_rpstartstopflag;
    //realplay window num equal (index+1)*(index+1)
    int m_rpwindownumindex;
    int m_rpwindownumindexbackup;
    //used to close realplay handle point to device
    TreeModel **m_rpmodel;
    const QModelIndex *m_rpmodelindex;
    int m_rpuseridbackup;
    int m_rpfirstrealhandle;
    int m_rpcurrentrealhandle;

};

#endif // REALPLAY_H

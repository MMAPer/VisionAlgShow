#include "realplay.h"
#include "ui_realplay.h"
#include "visionalgmain.h"
#include <QRect>
#include <QMessageBox>

extern VisionAlgMain* visionalgmain;

enum DeviceTreeNodeType
{
    DeviceTreeNode_ROOT = 1,
    DeviceTreeNode_Device,
    DeviceTreeNode_Channel
};

RealPlay::RealPlay(TreeModel **model, const QModelIndex *index, QList<DeviceData> *tree,
                   int *userid, NET_DVR_DEVICEINFO_V30 *devinfo, int *channelnum, int *channelmode,
                   QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RealPlay)
{
    ui->setupUi(this);

    m_pSelectedPlayWnd = NULL;  //选中的窗口
    m_pframePlay = ui->m_pframPlay;

    int i;
    for(i = 0; i < REALPLAY_MAX_NUM; i++)
    {
        m_framePlayWnd[i] = new FramePlayWnd();
        m_framePlayWnd[i]->setParent(m_pframePlay);

        connect(m_framePlayWnd[i], SIGNAL(SingleClickSig()), this, SLOT(OnSingleClickPlayWindow()));
    }

    m_pSelectedPlayWnd = m_framePlayWnd[0];
    m_pSelectedPlayWnd->setStyleSheet("border: 2px solid blue;");

    m_rpmodel = model;
    m_rpdeviceinfo = devinfo;
    m_rpchannelnum = channelnum;
    m_channellinkmode = channelmode;
    m_qlistdevicetree = tree;

    //Display the video frame to display real data
    m_rpmenu = NULL;

    winNumChanged(0);
}

RealPlay::~RealPlay()
{
    delete ui;
    if(m_rpstartstopflag == 1)
    {
        stopRealPlay();
    }
}

//改变屏幕数量
void RealPlay::winNumChanged(int winNumIndex)
{
    m_rpwindownumindex = winNumIndex;
    //Relayout playing windows according to the number and size
    int width = ui->m_pframPlay->width();
    int height = ui->m_pframPlay->height();
    qDebug()<<"width="<<width<<"height="<<height;
    int windowNum;
    int lineWndNum = winNumIndex + 1;
    windowNum = lineWndNum * lineWndNum;

    if (windowNum > REALPLAY_MAX_NUM)
    {
        return;
    }

    int i;
    int playWndWidth = width / lineWndNum;
    int playWndHeight = height / lineWndNum;

    for(i = 0; i < REALPLAY_MAX_NUM; i++)
    {
        m_framePlayWnd[i]->hide();

        if(i < windowNum)
        {
            m_framePlayWnd[i]->setGeometry((i%lineWndNum)*playWndWidth, (i%lineWndNum)*playWndHeight, playWndWidth, playWndHeight);
            m_framePlayWnd[i]->show();
        }
    }

}

//开始实时播放， int index为播放窗口数的索引值
void RealPlay::startRealPlay(int index)
{
    //上次被点击树节点的节点类型： 2设备树 3设备 4通道
    int nodetype = 0;
    //点击设备在qlistdevice中的序列号
    int devicerow = 0;
    //设备拥有的通道数
    int devicesubchannelnum = 0;
    //点击通道在qlistchannel中的序列号，点击设备时为0 其他大于0
    int channelrow = 0;

    QModelIndex tmpindex = (*m_rpmodelindex);

    while(tmpindex.isValid()==1)
    {
        //获得当前有效节点类型
        nodetype++;
        tmpindex = tmpindex.parent();
    }
    if(nodetype == 2)
    {
        //获取设备序列号，并置通道序列号为0
        devicerow = m_rpmodelindex->row();
        channelrow = 0;
        QModelIndex tmpsubindex = (*m_rpmodelindex).child(0, 0);
        while(tmpsubindex.isValid()==1)
        {
            //获取当前设备的通道数
            devicesubchannelnum++;
            tmpsubindex = (*m_rpmodelindex).child(devicesubchannelnum, 0);
        }
    }
    else if(nodetype == 3)
    {
        //获取设备和通道序列号
        devicerow = m_rpmodelindex->parent().row();
        channelrow = m_rpmodelindex->row();
    }
    //目前播放的窗口数
    int realplaytotalnum = (index+1)*(index+1);
    int returnvalue = 0;

    //树节点为设备，播放所有通道
    if(nodetype==2)
    {
        if(realplaytotalnum >= (devicesubchannelnum+1))
        {
            //该设备的所有通道预览都打开
            for(int i=0; i<devicesubchannelnum;i++)
            {
                NET_DVR_CLIENTINFO  tmpclientinfo;
                tmpclientinfo.hPlayWnd = (HWND)m_framePlayWnd[i]->GetPlayWndId();
                int ret = realPlayEncapseInterface(devicerow, i, &tmpclientinfo);
                if(ret==1)
                {
                    returnvalue = 1;
                }
            }
        }
        else
        {
            //该设备的前realplaytotalnum个通道打开
            for(int i = 0; i < realplaytotalnum; i++)
            {
                NET_DVR_CLIENTINFO tmpclientinfo;
                tmpclientinfo.hPlayWnd = (HWND)m_framePlayWnd[i]->GetPlayWndId();
                int ret = realPlayEncapseInterface(devicerow, i, &tmpclientinfo);
                if(ret==1)
                {
                    returnvalue = 1;
                }
            }
        }
        if(returnvalue==1)
        {
            m_rpstartstopflag = 1;
            emit PlayOrPauseSig(1);
        }
    }
    //树节点为通道，仅播放这一路通道
    if (nodetype == DeviceTreeNode_Channel)
    {
        NET_DVR_CLIENTINFO tmpclientinfo;
        tmpclientinfo.hPlayWnd = (HWND)m_pSelectedPlayWnd->GetPlayWndId();

        int returnvalue = realPlayEncapseInterface(devicerow, channelrow, &tmpclientinfo);
        if (returnvalue == 1)
        {
            m_rpstartstopflag = 1;
            emit PlayOrPauseSig(1);
        }
    }
}


//关掉已经打开的预览句柄
void RealPlay::stopRealPlay()
{
    stopRealPlayEncapseInterface();
    m_pframePlay->hide(); //If don't do this, the window is black.
    m_pframePlay->show();
}


///**  @fn  void __stdcall  RealDataCallBack(LONG lRealHandle,DWORD dwDataType,BYTE *pBuffer,DWORD  dwBufSize, void* dwUser)
// *   @brief data callback funtion
// *   @param (OUT) LONG lRealHandle
// *   @param (OUT) DWORD dwDataType
// *   @param (OUT) BYTE *pBuffer
// *   @param (OUT) DWORD  dwBufSize
// *   @param (OUT) void* dwUser
// *   @return none
// */
//void __stdcall  RealDataCallBack(LONG lRealHandle,DWORD dwDataType,BYTE *pBuffer,DWORD  dwBufSize, void* dwUser)
//{
//    if (dwUser != NULL)
//    {
//        qDebug("Demmo lRealHandle[%d]: Get StreamData! Type[%d], BufSize[%d], pBuffer:%p\n", lRealHandle, dwDataType, dwBufSize, pBuffer);
//    }
//}

/*******************************************************************
      Function:   RealPlay::realPlayEncapseInterface
   Description:   播放一路通道的数据
     Parameter:   (IN)   int devicerow     要播放的设备在qlistdevice中的序列号
                  (IN)   int channelrow    要播放的通道在某个设备节点下的序号.
                  (IN)   NET_DVR_CLIENTINFO *clientinfo  Device Infomation
        Return:   0--成功，-1--失败。
**********************************************************************/
int RealPlay::realPlayEncapseInterface(int devicerow, int channelrow, NET_DVR_CLIENTINFO *clientinfo)
{
    QList<DeviceData>::iterator it;
    QList<ChannelData>::iterator it_channel;
    //在设备列表中寻找设备节点
    int i = 0;
    for ( it = (*m_qlistdevicetree).begin(); it != (*m_qlistdevicetree).end(); ++it)
    {
        if (i == devicerow)
        {
            break;
        }
        i++;
    }
    //在设备节点下的通道列表中寻找通道节点
    int j = 0;
    for ( it_channel= (*it).m_qlistchanneldata.begin();
        it_channel != (*it).m_qlistchanneldata.end(); ++it_channel)
    {
        if (j == channelrow)
        {
            break;
        }
        j++;
    }

    //设置预览打开的相关参数
    clientinfo->lChannel = (*it_channel).getChannelNum();
    clientinfo->lLinkMode = (*it_channel).getLinkMode();
    char tmp[128] = {0};
    sprintf(tmp, "%s", (*it).getMultiCast().toLatin1().data());
    clientinfo->sMultiCastIP = tmp;
    //取流显示
//    int realhandle = NET_DVR_RealPlay_V30((*it).getUsrID(), clientinfo, VisionAlgMain::RealDataCallBack,NULL,1);
    int realhandle = 1;
    qDebug("Demo---Protocal:%d", clientinfo->lLinkMode);
    if (realhandle < 0)
    {
        QMessageBox::information(this, tr("NET_DVR_RealPlay error"), tr("SDK_LASTERROR=%1").arg(NET_DVR_GetLastError()));
        return 0;
    }
    else
    {
        if ((*it_channel).getChannelNum()>32)
        {
            NET_DVR_IPPARACFG ipcfg;
            DWORD Bytesreturned;
            if (!NET_DVR_GetDVRConfig((*it).getUsrID(), NET_DVR_GET_IPPARACFG,0,
                &ipcfg, sizeof(NET_DVR_IPPARACFG),&Bytesreturned))
            {
                QMessageBox::information(this, tr("NET_DVR_GetDVRConfig"), \
                    tr("SDK_LAST_ERROR=%1").arg(NET_DVR_GetLastError()));

                return 0;
            }
            if (ipcfg.struIPChanInfo[(*it_channel).getChannelNum()-32-1].byEnable == 0)
            {
                QMessageBox::information(this,tr("NET_DVR_RealPlay error"), \
                    tr("该通道不在线，预览失败"));

                return 0;
            }
        }
        //QMessageBox::information(this,tr("realPlayEncapseInterface"),tr("realhandle =%1").arg(realhandle));
        m_rpuseridbackup = (*it).getUsrID();

        //设置通道预览句柄
        (*it_channel).setRealhandle(realhandle);

        //备份预览句柄作为其他接口使用
        m_rpcurrentrealhandle = realhandle;
        if (clientinfo->lChannel == 1)
        {
            m_rpfirstrealhandle = realhandle;
        }

        //设置设备是否在预览状态标签值
        (*it).setRealPlayLabel(1);

        //初始为0
        int nodetype = 0;
        QModelIndex tmpindex = (*m_rpmodelindex);
        while (tmpindex.isValid() == 1)
        {
            nodetype ++ ;
            tmpindex = tmpindex.parent();
        }

        QStandardItem *item = NULL;
        if (nodetype == 2)
        {

            QModelIndex specialindex = m_rpmodelindex->child(channelrow,0);
            item = (*m_rpmodel)->itemFromIndex(specialindex);
            item->setIcon(QIcon(":/images/play.bmp"));

        }
        else if (nodetype == 3)
        {
            QModelIndex  parentindex = m_rpmodelindex->parent();
            item = (*m_rpmodel)->itemFromIndex(parentindex.child(channelrow,0));
            item->setIcon(QIcon(":/images/play.bmp"));
        }
        return 1;
    }

}

void RealPlay::stopRealPlayEncapseInterface()
{//QMessageBox::information(this,tr("stopRealPlayEncapseInterface"),tr("673"));
    QList<DeviceData>::iterator it;
    QList<ChannelData>::iterator it_channel;
    //在设备列表中寻找设备节点
    int i = 0;
    for ( it = (*m_qlistdevicetree).begin(); it != (*m_qlistdevicetree).end(); ++it)
    {
        if (m_rpuseridbackup == (*it).getUsrID())
        {
        //QMessageBox::information(this,tr("stopRealPlayEncapseInterface"),tr("715 i=%1").arg(i));
            break;
        }
        //QMessageBox::information(this,tr("stopRealPlayEncapseInterface"),tr("718 i=%1").arg(i));
        i++;
    }
//QMessageBox::information(this,tr("stopRealPlayEncapseInterface"),tr("686"));
    //在设备节点下的通道列表中寻找通道节点
    int j = 0;
    for ( it_channel= (*it).m_qlistchanneldata.begin();
        it_channel != (*it).m_qlistchanneldata.end(); ++it_channel)
    {
        if ((*it_channel).getRealhandle()!=-1)
        {
//QMessageBox::information(this,tr("stopRealPlayEncapseInterface"),tr("694 j=%1").arg(j));
            NET_DVR_StopRealPlay((*it_channel).getRealhandle());
            (*it_channel).setRealhandle(-1);

            QModelIndex tmpindex = (*m_rpmodel)->index(0,0).child(i,0).child(j,0);
            QStandardItem *item = (*m_rpmodel)->itemFromIndex(tmpindex);
            item->setIcon(QIcon(":/images/camera.bmp"));

        }
        j++;
    }
//QMessageBox::information(this,tr("stopRealPlayEncapseInterface"),tr("738"));
    //修改设备节点的预览状态标记
    (*it).setRealPlayLabel(0);
}

void RealPlay::OnSingleClickPlayWindow()
{
    //who send the signal
    FramePlayWnd *pWnd = qobject_cast<FramePlayWnd *>(sender());
    if(pWnd != m_pSelectedPlayWnd)
    {
        m_pSelectedPlayWnd->setStyleSheet("border: 2px solid gray");
    }

    //Set the border of selected window
    m_pSelectedPlayWnd = pWnd;
    m_pSelectedPlayWnd->setStyleSheet("border: 2px solid blue");

//    QRect qRect;
//    qRect = m_pSelectedPlayWnd->geometry();
//    qRect.setWidth(qRect.width()+100);
//    qRect.setHeight(qRect.height()+100);

//    m_pSelectedPlayWnd->setGeometry(qRect);
}

void RealPlay::on_pushButton_realplay_clicked()
{
    //User must login.
    if ( *m_rpuserid < 0)
    {
        QMessageBox::information(this,tr("have no login yet"),tr("not login"));
        return;
    }

    //当前处于停止播放阶段，就开启播放功能
    if (m_rpstartstopflag == 0)
    {
        m_pframePlay->show();
        startRealPlay(m_rpwindownumindex);
    }
    else
    {
        //当前处于播放中，就停止播放
        stopRealPlay();
        //置标记位
        m_rpstartstopflag = 0;
        //主界面显示为播放字符
        emit PlayOrPauseSig(0);
    }
}



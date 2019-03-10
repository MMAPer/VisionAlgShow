#include "visionalgmain.h"
#include "ui_visionalgmain.h"
#include "utils/iconhelper.h"
#include "utils/myapp.h"
#include "camera/camera.h"
#include "offline.h"
#include "tracking.h"
#include <QDesktopWidget>
#include <opencv2/opencv.hpp>
#include <QSize>
#include "pthread.h"
#include "algorithms/common.h"
extern VisionAlgMain* visionalgmain;


using namespace cv;

enum DeviceTreeNodeType
{
    DeviceTreeNode_ROOT = 0,
    DeviceTreeNode_Device,
    DeviceTreeNode_Channel
};

VisionAlgMain::VisionAlgMain(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::VisionAlgMain)
{
    ui->setupUi(this);
    timer1 = new QTimer();
    timer2 = new QTimer();
    timer3 = new QTimer();
    timer4 = new QTimer();
    yoloDetector = new YOLO_V2("/home/mmap/work/HCNetTest/models/detection/yolo/yolov2-tiny.cfg", "/home/mmap/work/HCNetTest/models/detection/yolo/yolov2-tiny.weights");

    this->InitCamera();  //初始化SDK
    this->InitData();
    this->InitStyle();  //初始化样式
    this->InitSlot();  //连接信号与槽
    this->InitVideo();  //初始化视频布局载体数据
}

VisionAlgMain::~VisionAlgMain()
{
    delete ui;
}

QImage Mat2QImage(cv::Mat cvImg)
{
    QImage qImg;
    if(cvImg.channels()==3)                             //3 channels color image
    {
        cv::cvtColor(cvImg,cvImg,CV_BGR2RGB);
        qImg =QImage((const unsigned char*)(cvImg.data),
                    cvImg.cols, cvImg.rows,
                    cvImg.cols*cvImg.channels(),
                    QImage::Format_RGB888);
    }
    else if(cvImg.channels()==1)                    //grayscale image
    {
        qImg =QImage((const unsigned char*)(cvImg.data),
                    cvImg.cols,cvImg.rows,
                    cvImg.cols*cvImg.channels(),
                    QImage::Format_Indexed8);
    }
    else
    {
        qImg =QImage((const unsigned char*)(cvImg.data),
                    cvImg.cols,cvImg.rows,
                    cvImg.cols*cvImg.channels(),
                    QImage::Format_RGB888);
    }
    return qImg;
}

//初始化摄像头对象及海康SDK
void VisionAlgMain::InitCamera()
{
    camera = Camera::getCamera();
    if(camera->initSDK())
    {
        qDebug() << "SDK init successful";
        QString strHCNetSDK;
        strHCNetSDK.sprintf("HCNetSDK V%d.%d.%d.%d", camera->getMainVersion(),camera->getSubVersion(),
                            camera->getBuildVersion(), camera->getBuildNUmber());
//        dwVersion = PlayM4_GetSdkVersion();
//        QString strPlaySDKVersion;
//        strPlaySDKVersion.sprintf("PlayCtrl V%d.%d.%d.%d", (0xff000000 & dwVersion)>>24, (0x00ff0000 & dwVersion)>>16,\
//                            (0x0000ff00 & dwVersion)>>8, (0x000000ff & dwVersion));
        ui->label_HCNetSDK->setText("海康威视SDK版本："+strHCNetSDK);
    }
    else
    {
            QMessageBox::information(this, tr("SDK初始化失败，"), \
                                     tr("SDK_LAST_ERROR=%1").arg(NET_DVR_GetLastError()));
    }
}

//初始化用户及摄像头设备数据
void VisionAlgMain::InitData()
{
    currentWinIndex = 0;  //默认当前选择第一块屏,自定义窗口数组，从0开始计数
    m_gcurrentchannelnum = 1;  //当前选择通道,通道数是从1开始计数，由海康SDK决定的
    m_gmodel = NULL;  //当前的自定义TreeModel数据  重写了QStandardItemModel
}

//初始化应用的全局样式
void VisionAlgMain::InitStyle()
{
    this->setStyleSheet("QGroupBox#gboxMain{border-width:0px;}");
    this->setProperty("Form", true);
    this->resize(QSize(1280,720));
    ui->widget_menu->move(0,0);

    ui->widget_menu->setStyleSheet("background-color:#3C3C3C;");
    ui->widget_alg->setStyleSheet("background-color:#3C3C3C;");

    ui->btnMenu_Full->setIcon(QIcon(":images/fullscreen.png"));
    ui->btnMenu_Full->setIconSize(QSize(32,32));
    ui->btnMenu_Full->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    ui->btnMenu_Full->setText("全屏");
    ui->btnMenu_Full->setFixedSize(64,64);


    ui->btnMenu_Setting->setIcon(QIcon(":images/setting.png"));
    ui->btnMenu_Setting->setIconSize(QSize(32,32));
    ui->btnMenu_Setting->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    ui->btnMenu_Setting->setText("设置");
    ui->btnMenu_Setting->setFixedSize(64,64);

    ui->btnMenu_Login->setIcon(QIcon(":images/login.png"));
    ui->btnMenu_Login->setIconSize(QSize(32,32));
    ui->btnMenu_Login->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    ui->btnMenu_Login->setText("登录 ");
    ui->btnMenu_Login->setFixedSize(64,64);

    ui->btnMenu_Logout->setIcon(QIcon(":images/logout.png"));
    ui->btnMenu_Logout->setIconSize(QSize(32,32));
    ui->btnMenu_Logout->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    ui->btnMenu_Logout->setText("注销");
    ui->btnMenu_Logout->setFixedSize(64,64);

    ui->btn_offlinehandle->setIcon(QIcon(":images/offline.png"));
    ui->btn_offlinehandle->setIconSize(QSize(32,32));
    ui->btn_offlinehandle->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    ui->btn_offlinehandle->setText("离线处理");
    ui->btn_offlinehandle->setFixedSize(64,64);

    ui->btn_onlinehandle->setIcon(QIcon(":images/online.png"));
    ui->btn_onlinehandle->setIconSize(QSize(32,32));
    ui->btn_onlinehandle->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    ui->btn_onlinehandle->setText("在线处理");
    ui->btn_onlinehandle->setFixedSize(64,64);

    //设置窗体标题栏隐藏--Qt::WindowStaysOnTopHint |
//    this->setWindowFlags(Qt::FramelessWindowHint |
//                         Qt::WindowSystemMenuHint |
//                         Qt::WindowMinMaxButtonsHint);

    //IconHelper::Instance()->SetIcon(ui->btnMenu_Close, QChar(0xf00d), 10);
    //IconHelper::Instance()->SetIcon(ui->btnMenu_Min, QChar(0xf068), 10);
    //IconHelper::Instance()->SetIcon(ui->label_ico, QChar(0xf03d), 11);

    //ui->label_title->setText(myApp::AppTitle);
    this->setWindowTitle(myApp::AppTitle);
    //ui->widget_title->setStyleSheet("background-color:#eeeeee;");
}


//初始化播放窗口
void VisionAlgMain::InitVideo()
{
    tempLab = 0;
    video_max = false;

    VideoLab.append(ui->labVideo1);  //QList<QLabel *> VideoLab;
    VideoLab.append(ui->labVideo2);
    VideoLab.append(ui->labVideo3);
    VideoLab.append(ui->labVideo4);
    VideoLab.append(ui->labVideo5);
    VideoLab.append(ui->labVideo6);
    VideoLab.append(ui->labVideo7);
    VideoLab.append(ui->labVideo8);
    VideoLab.append(ui->labVideo9);
    VideoLab.append(ui->labVideo10);
    VideoLab.append(ui->labVideo11);
    VideoLab.append(ui->labVideo12);
    VideoLab.append(ui->labVideo13);
    VideoLab.append(ui->labVideo14);
    VideoLab.append(ui->labVideo15);
    VideoLab.append(ui->labVideo16);

    VideoLay.append(ui->lay1);  //QList<QLayout *>VideoLay;
    VideoLay.append(ui->lay2);
    VideoLay.append(ui->lay3);
    VideoLay.append(ui->lay4);

    for (int i = 0; i < 16; i++) {
        VideoLab[i]->installEventFilter(this);
        VideoLab[i]->setProperty("labVideo", true);
        VideoLab[i]->setText(QString("屏幕%1").arg(i + 1));
    }

    menu = new QMenu(this);
    menu->setStyleSheet("font: 10pt \"微软雅黑\";");
    menu->addAction("切换到1画面", this, SLOT(show_video_1()));
    menu->addAction("切换到4画面", this, SLOT(show_video_4()));
    menu->addAction("切换到9画面", this, SLOT(show_video_9()));
    menu->addAction("切换到16画面", this, SLOT(show_video_16()));
    tempLab = VideoLab[0];
    show_video_4();
}


void VisionAlgMain::InitSlot()
{
    //添加事件监听器
    //左侧树监听事件
    ui->DVRsets_treeView->setMouseTracking(1);
    connect(ui->DVRsets_treeView, SIGNAL(pressed(const QModelIndex &)),
            this, SLOT(pressedTreeView(const QModelIndex &)));  //单击左侧摄像头树结构
    connect(ui->DVRsets_treeView, SIGNAL(doubleClicked(const QModelIndex &)),
            this, SLOT(OnDoubleClickTree(const QModelIndex &)));  //双击
    //按钮点击事件
    connect(ui->btnMenu_Full, SIGNAL(clicked(bool)), this, SLOT(fullScreen()));  //进入全屏
    connect(ui->btnMenu_Login, SIGNAL(clicked(bool)), this, SLOT(login()));  //登录
    connect(ui->btnMenu_Setting, SIGNAL(clicked(bool)), this, SLOT(systemSetting()));  //系统设置
    connect(ui->btnMenu_Logout, SIGNAL(clicked(bool)), this, SLOT(logoff()));  //注销
    connect(ui->btn_offlinehandle, SIGNAL(clicked(bool)), this, SLOT(offlinehandle()));  //离线处理

    connect(timer1,SIGNAL(timeout()),this,SLOT(playTimer()));
    connect(timer2,SIGNAL(timeout()),this,SLOT(playTimer()));
    connect(timer3,SIGNAL(timeout()),this,SLOT(playTimer()));
    connect(timer4,SIGNAL(timeout()),this,SLOT(playTimer()));

}

void VisionAlgMain::playTimer()
{
    int index = sender()->property("index").toInt();
    Mat image = camera->getFrame(index);
    cv::resize(image, image, cv::Size(490, 320), (0, 0), (0, 0), cv::INTER_LINEAR);

    vector<BoundingBox> boxes = yoloDetector->Detect_yolov2(image);
    for (int i = 0; i < boxes.size(); i++) {
        int x = (int) boxes[i].x;
        int y = (int) boxes[i].y;
        int w = (int) boxes[i].w;
        int h = (int) boxes[i].h;
        cv::rectangle(image, cv::Rect(x, y, w, h), cv::Scalar(255, 0, 0), 2);
    }

    VideoLab[index]->setPixmap(QPixmap::fromImage(Mat2QImage(image).scaled(490,320)));
}

//通过注册事件监听器绑定事件
bool VisionAlgMain::eventFilter(QObject *obj, QEvent *event)
{
    QMouseEvent *MouseEvent = static_cast<QMouseEvent *>(event);
    if((event->type()) == QEvent::MouseButtonPress)
    {
       if(MouseEvent->buttons() == Qt::RightButton)
        {
            tempLab = qobject_cast<QLabel *>(obj);
            menu->exec(QCursor::pos());
            return true;
        } else if(MouseEvent->buttons() == Qt::LeftButton)
       {
           tempLab = qobject_cast<QLabel *>(obj);
           for(int i=0; i<VideoLab.size(); i++)
           {
               if(tempLab == VideoLab[i])
               {
                   currentWinIndex = i;
               }
           }
       }
    }
    return QObject::eventFilter(obj, event);
}

//键盘事件
void VisionAlgMain::keyPressEvent(QKeyEvent *event)
{
    //F1键进入全屏,esc键退出全屏
    switch(event->key()) {
    case Qt::Key_F1:
        fullScreen();
        break;
    case Qt::Key_Escape:
        screen_normal();
        break;
    default:
        QDialog::keyPressEvent(event);
        break;
    }
}


//单击树结点，对一些状态索引进行改变
void VisionAlgMain::pressedTreeView(const QModelIndex &index)
{
    //目的是设置当前有效地设备(DVR)和通道信息(IP Device)，方案是先区分是设备树，设备还是通道
    //然后通过index的parent()和row()值来定位在qlistdevice中的位置和实际内容
//    QMessageBox::information(this, tr("pressedTreeView"),tr("row=%1 colum=%2 data=%3").arg(
//                                 index.row()).arg(index.column()).arg(
//                                 index.data().toString().toLatin1().data()));
    int level = 0;
    QModelIndex tempindex = index;
    while(tempindex.parent().isValid()==1)
    {
        level++;  //为了判断level是根节点、设备结点、通道结点
        tempindex = tempindex.parent();
    }
    if(level==0)  //设备
    {
        m_iposttreelevel = 0;
    }
    else if(level==1)  //通道
    {
        m_qtreemodelindex = index;
        m_iposttreelevel = 1;
    }
    ui->labMessage->setText(tr("当前选中:%1").arg(index.data().toString()));
}



//双击树节点
void VisionAlgMain::OnDoubleClickTree(const QModelIndex &index)
{
    int level = 0;
    QModelIndex tmpindex = index;
    while (tmpindex.parent().isValid()==1)
    {
        level ++ ;
        tmpindex = tmpindex.parent();
    }
    if (level == 0)  //设备结点
    {
        m_iposttreelevel= 0;
    }
    else if (level == 1)  //通道结点
    {
        m_qtreemodelindex = index;  //QModelIndex m_qtreemodelindex;  //树点击的索引
        m_iposttreelevel = 1;
    }

    //双击的是设备
    if (m_iposttreelevel == 0)
    {
        return;
    }
    //当前点击的是一个通道
    else if (m_iposttreelevel == 1)
    {
        int deviceindex = index.parent().row();  //点击通道所属设备在树中的索引，目前只有一台设备，所以为0
        int channelindex = index.row();  //点击通道的索引  目前37台设备（13室内+22楼道+2台考勤机），channelIndex是0~36

        //need first find device then channel then set the channel num
        QList<DeviceData>::iterator it;
        QList<ChannelData>::iterator it_channel;

        int i=0;
        //目前只有一台NVR，所以i最后为1
        for ( it = camera->listDeviceData.begin(); it != camera->listDeviceData.end(); ++it)
        {
            if (i == deviceindex)
            {
                break;
            }
            i++;
        }

        memcpy(&camera->myDeviceInfo.struDeviceV30, &(*it).m_deviceinfo,sizeof(NET_DVR_DEVICEINFO_V30));

        i=0;

        for (it_channel = (*it).m_qlistchanneldata.begin(); it_channel !=
                    (*it).m_qlistchanneldata.end();++it_channel)  //这样遍历的目的是为了获取相应的通道（又称IP设备）
        {
            if (i == channelindex)
            {

                // play logic--------------------------------------------------------------------
                if((*it).m_qlistchanneldata[i].getRealPlaying())
                {
                    QMessageBox::information(this, tr("information"),tr("该设备正在播放"));
                }
                else
                {
                    int index = currentWinIndex;

                    if(labPlayStatus[index])  // replace
                    {
                        (*it).m_qlistchanneldata[(labPlayChannelBackup[index]-1)].setRealPlaying(false);
                        camera->pauseOneChannel(index);

                        int channelNum = (*it).m_qlistchanneldata[i].getChannelNum();
                        camera->playOneChannel(channelNum, index);
                        (*it).m_qlistchanneldata[i].setRealPlaying(true);
                        labPlayStatus[index] = 1;
                        switch(index)
                        {
                        case 0:
                            timer1->stop();
                            timer1->setProperty("index", 0);
                            timer1->start(1000/25);
                            break;
                        case 1:
                            timer2->stop();
                            timer2->setProperty("index", 1);
                            timer2->start(1000/25);
                            break;
                        case 2:
                            timer3->stop();
                            timer3->setProperty("index", 2);
                            timer3->start(1000/25);
                            break;
                        case 3:
                            timer4->stop();
                            timer4->setProperty("index", 3);
                            timer4->start(1000/25);
                            break;
                        }
                    }

                    else  // play
                    {
                        int channelNum = (*it).m_qlistchanneldata[i].getChannelNum();
                        camera->playOneChannel(channelNum, index);
                        (*it).m_qlistchanneldata[i].setRealPlaying(true);
                        labPlayStatus[index] = 1;
                        labPlayChannelBackup[index] = channelNum;
                        switch(index)
                        {
                        case 0:
                            timer1->setProperty("index", 0);
                            timer1->start(1000/25);
                            break;
                        case 1:
                            timer2->setProperty("index", 1);
                            timer2->start(1000/25);
                            break;
                        case 2:
                            timer3->setProperty("index", 2);
                            timer3->start(1000/25);
                            break;
                        case 3:
                            timer4->setProperty("index", 3);
                            timer4->start(1000/25);
                            break;
                        }
                    }

//                        while(1)
//                        {
//                            Mat image = camera->getFrame(index);
//                            cv::resize(image, image, cv::Size(490, 320), (0, 0), (0, 0), cv::INTER_LINEAR);
//                            VideoLab[index]->setPixmap(QPixmap::fromImage(Mat2QImage(image).scaled(490,320)));
//                            imshow("test1", image);
//                            waitKey(1);
//                        }

                }
            break;
            }
            //没找到则i++
            i++;
        }
    return;
    }
}




void VisionAlgMain::startRealPlay()
{
    //上次被点击树节点的节点类型： 1设备 2通道
    int nodetype = 0;
    //点击设备在qlistdevice中的序列号
    int devicerow = 0;
    //设备拥有的通道数
    int devicesubchannelnum = 0;
    //点击通道在qlistchannel中的序列号，点击设备时为0 其他大于0
    int channelrow = 0;

    QModelIndex tmpindex = m_qtreemodelindex;

    while(tmpindex.isValid()==1)
    {
        //获得当前有效节点类型
        nodetype++;
        tmpindex = tmpindex.parent();
    }

    if(nodetype == 1)
    {
        //获取设备序列号，并置通道序列号为0
        devicerow = m_qtreemodelindex.row();
        channelrow = 0;
        QModelIndex tmpsubindex = m_qtreemodelindex.child(0, 0);
        while(tmpsubindex.isValid()==1)
        {
            //获取当前设备的通道数
            devicesubchannelnum++;
            tmpsubindex = m_qtreemodelindex.child(devicesubchannelnum, 0);
        }
        qDebug()<<"devicesubchannelnum = " << devicesubchannelnum;
    }
    else if(nodetype == 2)
    {
        //获取设备和通道序列号
        devicerow = m_qtreemodelindex.parent().row();
        channelrow = m_qtreemodelindex.row();
    }
    //目前播放的窗口数
    windowNum = 4;
    int realplaytotalnum = windowNum;
    int returnvalue = 0;

    //树节点为设备，播放所有通道
    if(nodetype==1)
    {
        if(realplaytotalnum >= (devicesubchannelnum+1))
        {
            //该设备的所有通道预览都打开
            for(int i=0; i<devicesubchannelnum;i++)
            {
                NET_DVR_CLIENTINFO  tmpclientinfo;
                tmpclientinfo.hPlayWnd = (HWND)VideoLab[i]->winId();
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
                tmpclientinfo.hPlayWnd = (HWND)VideoLab[i]->winId();
                int ret = realPlayEncapseInterface(devicerow, i, &tmpclientinfo);
                if(ret==1)
                {
                    returnvalue = 1;
                }
            }
        }
        if(returnvalue==1)
        {
//            m_rpstartstopflag = 1;
        }
    }
    //树节点为通道，仅播放这一路通道
    if (nodetype == DeviceTreeNode_Channel)
    {
        NET_DVR_CLIENTINFO tmpclientinfo;
        tmpclientinfo.hPlayWnd = (HWND)tempLab->winId();

        int returnvalue = realPlayEncapseInterface(devicerow, channelrow, &tmpclientinfo);
        if (returnvalue == 1)
        {
//            m_rpstartstopflag = 1;
        }
    }
}

void VisionAlgMain::stopRealPlay()
{

}


/**  @fn  void __stdcall  RealDataCallBack(LONG lRealHandle,DWORD dwDataType,BYTE *pBuffer,DWORD  dwBufSize, void* dwUser)
 *   @brief data callback funtion
 *   @param (OUT) LONG lRealHandle
 *   @param (OUT) DWORD dwDataType
 *   @param (OUT) BYTE *pBuffer
 *   @param (OUT) DWORD  dwBufSize
 *   @param (OUT) void* dwUser
 *   @return none
 */
void __stdcall  RealDataCallBack(LONG lRealHandle,DWORD dwDataType,BYTE *pBuffer,DWORD  dwBufSize, void* dwUser)
{


}

/*******************************************************************
      Function:   RealPlay::realPlayEncapseInterface
   Description:   播放一路通道的数据
     Parameter:   (IN)   int devicerow     要播放的设备在qlistdevice中的序列号
                  (IN)   int channelrow    要播放的通道在某个设备节点下的序号.
                  (IN)   NET_DVR_CLIENTINFO *clientinfo  Device Infomation
        Return:   0--成功，-1--失败。
**********************************************************************/
int VisionAlgMain::realPlayEncapseInterface(int devicerow, int channelrow, NET_DVR_CLIENTINFO *clientinfo)
{
    qDebug() << "devicerow = " << devicerow << " channelrow = " << channelrow;
    QList<DeviceData>::iterator it;
    QList<ChannelData>::iterator it_channel;
    //在设备列表中寻找设备节点
    int i = 0;
    for ( it = camera->listDeviceData.begin(); it != camera->listDeviceData.end(); ++it)
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
    qDebug() << "i = " << i << " j = " << j;

    //设置预览打开的相关参数
    clientinfo->lChannel = (*it_channel).getChannelNum();
    clientinfo->lLinkMode = (*it_channel).getLinkMode();
    char tmp[128] = {0};
    sprintf(tmp, "%s", (*it).getMultiCast().toLatin1().data());
    clientinfo->sMultiCastIP = tmp;

    qDebug() << "clientinfo->lChannel = " << clientinfo->lChannel << " clientinfo->lLinkMode = " << clientinfo->lLinkMode
             << " clientinfo->sMultiCastIP = " << clientinfo->sMultiCastIP;

    //取流显示
    int realhandle = NET_DVR_RealPlay_V30((*it).getUsrID(), clientinfo, RealDataCallBack,NULL,1);
    qDebug("Demo---Protocal:%d  realhandle=%d", clientinfo->lLinkMode, realhandle);

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
//        m_rpuseridbackup = (*it).getUsrID();

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
        QModelIndex tmpindex = m_qtreemodelindex;
        while (tmpindex.isValid() == 1)
        {
            nodetype ++ ;
            tmpindex = tmpindex.parent();
        }

        QStandardItem *item = NULL;
        if (nodetype == 1)
        {

            QModelIndex specialindex = m_qtreemodelindex.child(channelrow,0);
            item = m_gmodel->itemFromIndex(specialindex);
            item->setIcon(QIcon(":/images/play.bmp"));

        }
        else if (nodetype == 2)
        {
            QModelIndex  parentindex = m_qtreemodelindex.parent();
            item = m_gmodel->itemFromIndex(parentindex.child(channelrow,0));
            item->setIcon(QIcon(":/images/play.bmp"));
        }
        return 1;
    }

}

void VisionAlgMain::stopRealPlayEncapseInterface()
{//QMessageBox::information(this,tr("stopRealPlayEncapseInterface"),tr("673"));
    QList<DeviceData>::iterator it;
    QList<ChannelData>::iterator it_channel;
    //在设备列表中寻找设备节点
    int i = 0;
    for ( it = camera->listDeviceData.begin(); it != camera->listDeviceData.end(); ++it)
    {
//        if (m_rpuseridbackup == (*it).getUsrID())
//        {
//        //QMessageBox::information(this,tr("stopRealPlayEncapseInterface"),tr("715 i=%1").arg(i));
//            break;
//        }
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

            QModelIndex tmpindex = m_gmodel->index(0,0).child(i,0).child(j,0);
            QStandardItem *item = m_gmodel->itemFromIndex(tmpindex);
            item->setIcon(QIcon(":/images/camera.bmp"));

        }
        j++;
    }
//QMessageBox::information(this,tr("stopRealPlayEncapseInterface"),tr("738"));
    //修改设备节点的预览状态标记
    (*it).setRealPlayLabel(0);
}



//系统设置
void VisionAlgMain::systemSetting()
{
    qDebug()<<"系统设置"<<endl;
}

//离线处理
void VisionAlgMain::offlinehandle()
{
    myOffline = new offline(ui->widget_show);
    myOffline->setAttribute(Qt::WA_DeleteOnClose);
    ui->widget_alg->hide();
    ui->widget_main->hide();
    //ui->widget_show->hide();
    myOffline->show();
    myOffline->move(0,0);
    connect(ui->btn_onlinehandle, SIGNAL(clicked(bool)), this, SLOT(onlinehandle()));   //在线处理
    ui->btnMenu_Full->blockSignals(true); //禁用全屏
    //connect(ui->btnMenu_Full,&QPushButton::clicked,this,&VisionAlgMain::offline_full);
    //offline o;
    //o.setGeometry(qApp->desktop()->availableGeometry());
    //o.exec();
    //qDebug()<<"离线处理"<<endl;
    //tracking t;
    //t.setGeometry(qApp->desktop()->availableGeometry());
    //t.exec();
}

void VisionAlgMain::onlinehandle()
{
    myOffline->close();
   // ui->widget_show->show();
    ui->widget_alg->show();
    ui->widget_main->show();
}

void VisionAlgMain::offline_full()
{

    this->setGeometry(qApp->desktop()->geometry());
    this->layout()->setContentsMargins(0, 0, 0, 0);
    ui->widget_main->layout()->setContentsMargins(0, 0, 0, 0);
    ui->widget_menu->setVisible(false);
    ui->widget_camera->setVisible(false);
    ui->widget_alg->setVisible(false);
}

void VisionAlgMain::removelayout()
{
    for (int i = 0; i < 4; i++) {
        VideoLay[0]->removeWidget(VideoLab[i]);
        VideoLab[i]->setVisible(false);
    }

    for (int i = 4; i < 8; i++) {
        VideoLay[1]->removeWidget(VideoLab[i]);
        VideoLab[i]->setVisible(false);
    }

    for (int i = 8; i < 12; i++) {
        VideoLay[2]->removeWidget(VideoLab[i]);
        VideoLab[i]->setVisible(false);
    }

    for (int i = 12; i < 16; i++) {
        VideoLay[3]->removeWidget(VideoLab[i]);
        VideoLab[i]->setVisible(false);
    }
}

void VisionAlgMain::show_video_1()
{
    removelayout();
    myApp::VideoType = "1";
    video_max = true;
    change_video_1();
    windowNum = 1;
}

void VisionAlgMain::change_video_1(int index)
{
    for (int i = (index + 0); i < (index + 1) ; i++) {
        VideoLay[0]->addWidget(VideoLab[i]);
        VideoLab[i]->setVisible(true);
    }
}

void VisionAlgMain::show_video_4()
{
    removelayout();
    video_max = false;
    myApp::VideoType = "4";
    windowNum = 4;
    change_video_4();
}

void VisionAlgMain::change_video_4(int index)
{
    for (int i = (index + 0); i < (index + 2); i++) {
        VideoLay[0]->addWidget(VideoLab[i]);
        VideoLab[i]->setVisible(true);
    }

    for (int i = (index + 2); i < (index + 4); i++) {
        VideoLay[1]->addWidget(VideoLab[i]);
        VideoLab[i]->setVisible(true);
    }
}

void VisionAlgMain::show_video_9()
{
    removelayout();
    video_max = false;
    myApp::VideoType = "9";
    windowNum = 9;
    change_video_9(0);
}

void VisionAlgMain::change_video_9(int index)
{
    for (int i = (index + 0); i < (index + 3); i++) {
        VideoLay[0]->addWidget(VideoLab[i]);
        VideoLab[i]->setVisible(true);
    }

    for (int i = (index + 3); i < (index + 6); i++) {
        VideoLay[1]->addWidget(VideoLab[i]);
        VideoLab[i]->setVisible(true);
    }

    for (int i = (index + 6); i < (index + 9); i++) {
        VideoLay[2]->addWidget(VideoLab[i]);
        VideoLab[i]->setVisible(true);
    }
}

void VisionAlgMain::show_video_16()
{
    removelayout();
    myApp::VideoType = "16";
    video_max = false;
    windowNum = 16;
    change_video_16(0);
}

void VisionAlgMain::change_video_16(int index)
{
    for (int i = (index + 0); i < (index + 4); i++) {
        VideoLay[0]->addWidget(VideoLab[i]);
        VideoLab[i]->setVisible(true);
    }

    for (int i = (index + 4); i < (index + 8); i++) {
        VideoLay[1]->addWidget(VideoLab[i]);
        VideoLab[i]->setVisible(true);
    }

    for (int i = (index + 8); i < (index + 12); i++) {
        VideoLay[2]->addWidget(VideoLab[i]);
        VideoLab[i]->setVisible(true);
    }

    for (int i = (index + 12); i < (index + 16); i++) {
        VideoLay[3]->addWidget(VideoLab[i]);
        VideoLab[i]->setVisible(true);
    }
}



//登录
void VisionAlgMain::login()
{
    //防止重复登录
    if(camera->getIsLogin()==true)
    {
        QMessageBox::information(this, tr("login information"), tr("您已经登录"));
        return;
    }
    //登录
    if(!camera->login())
    {
        QMessageBox::information(this, tr("login fail"), tr("请检查网络是否在同一网段,error code=%1").arg(NET_DVR_GetLastError()));
        return;
    }
    //通过SDK获取设备信息
    if(!camera->setDeviceData())
    {
        QMessageBox::information(this, tr("device data fail"), tr("无法获取设备信息，请检查网络,error code=%1").arg(NET_DVR_GetLastError()));
        return;
    }
    ui->labMessage->setText("登录成功");
    //设备信息写成字符串，供左侧树解析
    QString treedatastring;
    treedatastring = getStringFromList(camera->listDeviceData);
    showDeviceTree(treedatastring);
}

//注销
void VisionAlgMain::logoff()
{
    int logOffRes = camera->logoff();
    if(logOffRes==1)  //不存在已登录的用户
    {
        QMessageBox::information(this, tr("logout information"),tr("未存在已登录的设备"));
    }
    else if(logOffRes==2)  //其他原因
    {
        QMessageBox::information(this, tr("login information"),  tr("SDK_LAST_ERROR=%1").arg(NET_DVR_GetLastError()));
    }
    else  //注销成功
    {
       //加载登出设备标记图片和收起通道
        if (m_gmodel != NULL)
        {
            delete m_gmodel;
            m_gmodel = NULL;
            ui->DVRsets_treeView->clearSelection();
            ui->labMessage->setText("注销成功");
        }
    }
}

//全屏
void VisionAlgMain::fullScreen()
{
    this->setGeometry(qApp->desktop()->geometry());
    this->layout()->setContentsMargins(0, 0, 0, 0);
    ui->widget_main->layout()->setContentsMargins(0, 0, 0, 0);
    ui->widget_menu->setVisible(false);
    ui->widget_camera->setVisible(false);
    ui->widget_alg->setVisible(false);
}

//正常屏
void VisionAlgMain::screen_normal()
{
    this->setGeometry(qApp->desktop()->availableGeometry());
    this->layout()->setContentsMargins(0, 0, 0, 0);
    ui->widget_main->layout()->setContentsMargins(0, 0, 0, 0);
    ui->widget_menu->setVisible(true);
    ui->widget_camera->setVisible(true);
    ui->widget_alg->setVisible(true);
}

//从设备列表中获取设备字符串,该函数主要是为了今后写入文件，保存信息
QString VisionAlgMain::getStringFromList(QList<DeviceData> & data)
{
    QString dataInString;
    QStringList lines;

    QString deviceBegin("<device>");
    QString deviceEnd("</device>");
    QString channelBegin("<channel>");
    QString channelEnd("</channel>");

    DeviceData device;
    ChannelData chann;
    QString port;
    QString channum;
    for ( QList<DeviceData>::iterator it = data.begin(); it != data.end();
    ++it)
    {
        lines << deviceBegin;
        device =(*it);
        lines << device.getDeviceName();
        lines << device.getIP();
        lines << port.number(device.getPort());
        lines << device.getUsrName();
        lines << device.getPasswd();

        for ( QList<ChannelData>::iterator it_Chann =
        device.m_qlistchanneldata.begin(); \
            it_Chann !=  device.m_qlistchanneldata.end(); ++it_Chann)
        {
            lines << channelBegin;
            chann = (*it_Chann);
            lines << chann.getChannelName();
            lines << channum.number(chann.getChannelNum());
            lines << chann.getProtocolTypeQstring();
            lines << chann.getStreamTypeQstring();
            lines << channelEnd;
        }
        lines << deviceEnd;
    }
    for (QStringList::iterator it_string = lines.begin();\
        it_string != lines.end(); ++it_string)
    {
        dataInString.append(*it_string);
        dataInString.append("\n");
    }
    return dataInString;
}

//显示设备树信息
void VisionAlgMain::showDeviceTree(const QString &nodedata)
{
    if (m_gmodel != NULL)
    {
        delete m_gmodel;
        m_gmodel = NULL;
        ui->DVRsets_treeView->clearSelection();
        QMessageBox::information(this,tr("system message"),tr("add or delete device success"));
    }

    m_gmodel = new TreeModel(nodedata);  //TreeModel继承QStandardItemModel，内部会解析xml字符串
    ui->DVRsets_treeView->header()->hide();
    ui->DVRsets_treeView->setModel(m_gmodel);

    //ui.DVRsets_treeView->setUpdatesEnabled (TRUE);
    //ui.DVRsets_treeView->update();
    //ui.DVRsets_treeView->repaint();

    //setUpdatesEnabled (TRUE);
    //update();
    //repaint();

    ui->DVRsets_treeView->expand(m_gmodel->index(0,0));
    ui->DVRsets_treeView->setExpanded(m_gmodel->index(0,0), 1);

    //更新设备状态和通道状态 先找设备树节点然后递归
    QModelIndex tmpindex = m_gmodel->index(0,0);  //根节点，参数为行和列
    if (tmpindex.isValid()==1)
    {
        ui->DVRsets_treeView->setExpanded(tmpindex, 1);
    }

    QModelIndex deviceindex = tmpindex;  //根节点  NERCMS_NVR
    QList<DeviceData>::iterator it;  //设备迭代器
    QList<ChannelData>::iterator it_channel;  //通道迭代器
    int i=0;
    it = camera->listDeviceData.begin();
    while((deviceindex.isValid()==1)&&(it != camera->listDeviceData.end()))  //目前就一台NVR
    {
        QModelIndex channelindex = deviceindex.child(0,0);
        it_channel = (*it).m_qlistchanneldata.begin();
        int j=0;
        while((channelindex.isValid()==1)&&(it_channel != (*it).m_qlistchanneldata.end()))
        {
            if (it_channel->getRealhandle()!=-1)
            {
                //加载上线标记图片和展开通道
                QStandardItem *item = m_gmodel->itemFromIndex(channelindex);
                item->setIcon(QIcon(":/images/play.bmp"));
                ui->DVRsets_treeView->expand(channelindex);
                ui->DVRsets_treeView->setExpanded(channelindex, 1);
            }
            j++;
            ++it_channel;
            channelindex = deviceindex.child(j,0);

        }
        i++;
        ++it;
        deviceindex = tmpindex.child(i,0);
    }
}

#include "visionalgmain.h"
#include "ui_visionalgmain.h"
#include "iconhelper.h"
#include "myapp.h"
#include <QDesktopWidget>
#include <opencv2/opencv.hpp>
extern VisionAlgMain* visionalgmain;

using namespace cv;

VisionAlgMain::VisionAlgMain(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::VisionAlgMain)
{
    ui->setupUi(this);

    m_gcurrentuserid = -1;  //当前登录用户
    m_gcurrentchannelnum = 0;  //当前选择通道
    m_gcurrentchannellinkmode = 0x0;  //当前通道链接模式
    m_gmodel = NULL;  //当前的自定义TreeModel数据  重写了QStandardItemModel
    this->InitStyle();  //初始化样式
    this->InitSlot();  //连接信号与槽
    this->InitSdk();  //初始化SDK
    this->InitVideo();  //初始化视频布局载体数据
}

VisionAlgMain::~VisionAlgMain()
{
    delete ui;
    m_bislogin = FALSE;
    NET_DVR_Cleanup();
}



//init the global style of the application
void VisionAlgMain::InitStyle()
{
    this->setStyleSheet("QGroupBox#gboxMain{border-width:0px;}");
    this->setProperty("Form", true);
    //设置窗体标题栏隐藏--Qt::WindowStaysOnTopHint |
    this->setWindowFlags(Qt::FramelessWindowHint |
                         Qt::WindowSystemMenuHint |
                         Qt::WindowMinMaxButtonsHint);

    IconHelper::Instance()->SetIcon(ui->btnMenu_Close, QChar(0xf00d), 10);
    IconHelper::Instance()->SetIcon(ui->btnMenu_Min, QChar(0xf068), 10);
    IconHelper::Instance()->SetIcon(ui->label_ico, QChar(0xf03d), 11);

    ui->label_title->setText(myApp::AppTitle);
    this->setWindowTitle(myApp::AppTitle);
    ui->widget_title->setStyleSheet("background-color:#78b4e3;");

}

//init the SDK of Hikvision
void VisionAlgMain::InitSdk()
{
    if(!NET_DVR_Init())
    {
        QMessageBox::information(this, tr("SDK初始化失败，"), \
                                 tr("SDK_LAST_ERROR=%1").arg(NET_DVR_GetLastError()));
    }
    //初始化成功，Title上显示SDK版本
    else
    {
        DWORD dwVersion = NET_DVR_GetSDKBuildVersion();
        QString strHCNetSDK;
        strHCNetSDK.sprintf("HCNetSDK V%d.%d.%d.%d", (0xff000000 & dwVersion)>>24, (0x00ff0000 & dwVersion)>>16,\
            (0x0000ff00 & dwVersion)>>8, (0x000000ff & dwVersion));

        dwVersion = PlayM4_GetSdkVersion();
        QString strPlaySDKVersion;
        strPlaySDKVersion.sprintf("PlayCtrl V%d.%d.%d.%d", (0xff000000 & dwVersion)>>24, (0x00ff0000 & dwVersion)>>16,\
                            (0x0000ff00 & dwVersion)>>8, (0x000000ff & dwVersion));
        ui->label_HCNetSDK->setText("SDK版本："+strHCNetSDK+"  "+strPlaySDKVersion);
    }
}

//init the play lab
void VisionAlgMain::InitVideo()
{
    tempLab = 0;
    video_max = false;

    VideoLab.append(ui->labVideo1);
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

    VideoLay.append(ui->lay1);
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
    show_video_1();
}

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
        }
    }
}

void VisionAlgMain::InitSlot()
{
    //左侧树监听事件
    ui->DVRsets_treeView->setMouseTracking(1);
    connect(ui->DVRsets_treeView, SIGNAL(pressed(const QModelIndex &)),
            this, SLOT(pressedTreeView(const QModelIndex &)));  //单击
    connect(ui->DVRsets_treeView, SIGNAL(doubleClicked(const QModelIndex &)),
            this, SLOT(OnDoubleClickTree(const QModelIndex &)));  //双击


    connect(ui->btnMenu_Full, SIGNAL(clicked()), this, SLOT(screen_full()));
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

void VisionAlgMain::keyPressEvent(QKeyEvent *event)
{
    //空格键进入全屏,esc键退出全屏
    switch(event->key()) {
    case Qt::Key_F1:
        screen_full();
        break;
    case Qt::Key_Escape:
        screen_normal();
        break;
    default:
        QDialog::keyPressEvent(event);
        break;
    }
}

//关闭
void VisionAlgMain::on_btnMenu_Close_clicked()
{
    exit(0);
}

//最小化
void VisionAlgMain::on_btnMenu_Min_clicked()
{
    this->showMinimized();
}

//登录
void VisionAlgMain::on_btnMenu_Login_clicked()
{
    if(m_bislogin==TRUE)
    {
        QMessageBox::information(this, tr("login information"), tr("您已经登录"));
        return;
    }
    DeviceData *newDNode = new DeviceData;
    QString treedatastring;
    newDNode->m_qdevicename = myApp::DeviceNodeName;
    newDNode->m_qip = myApp::DeviceIp;
    newDNode->m_qiport = myApp::DevicePort;
    newDNode->m_qusername = myApp::DeviceUserName;
    newDNode->m_qpassword = myApp::DevicePwd;

    //先要尝试登陆设备，登陆成功后才能加入到队列中，然后还要判断通道情况
    newDNode->m_iuserid = NET_DVR_Login_V30(newDNode->getIP().toLatin1().data(),
                                            newDNode->getPort(),
                                            newDNode->getUsrName().toLatin1().data(),
                                            newDNode->getPasswd().toLatin1().data(),
                                            &newDNode->m_deviceinfo);
    //登录失败
    if (newDNode->m_iuserid < 0)
    {
        QMessageBox::information(this, tr("login fail"), tr("请检查网络是否在同一网段,error code=%1").arg(NET_DVR_GetLastError()));
        newDNode = NULL;
        return;
    }
    else  //登录成功
    {
        qDebug()<<"设备登陆成功，用户ID为"<<newDNode->m_iuserid;
        m_bislogin = TRUE;
        //测试通道信息，填充通道数据
        NET_DVR_IPPARACFG ipcfg;
        DWORD Bytesreturned;
        //获取IP接入配置参数
        if(!NET_DVR_GetDVRConfig(newDNode->m_iuserid, NET_DVR_GET_IPPARACFG,0,&ipcfg,sizeof(NET_DVR_IPPARACFG),&Bytesreturned))
        {
            //8000 devices begin
            NET_DVR_DEVICECFG devicecfg;
            DWORD Bytesreturned;
            //获取设备参数
            if(!NET_DVR_GetDVRConfig(newDNode->m_iuserid, NET_DVR_GET_DEVICECFG,0,&devicecfg,sizeof(NET_DVR_DEVICECFG),&Bytesreturned))
            {
                QMessageBox::information(this, tr("NET_DVR_GetDVRConfig"),tr("SDK_LAST_ERROR=%1").arg(NET_DVR_GetLastError()));
                delete newDNode;
                newDNode = NULL;
                return;
            }
            for (int i=devicecfg.byStartChan;i<=devicecfg.byChanNum ;i++)  //byStartChan起始通道号
            {
                ChannelData *newChannel = new ChannelData;
                QString name="Cameral";
                QString num = QString::number(i, 10) ;  //以十进制的数字形式转为字符串
                name.append(num);
                //填充通道初始属性内容,初始设为TCP+主码流
                newChannel->setChannelName(name);
                newChannel->setChannelNum(i);
                newChannel->setProtocolType(TCP);
                newChannel->setStreamType(MAINSTREAM);
                //添加进设备节点
                newDNode->m_qlistchanneldata.append(*newChannel);
                delete newChannel;
                newChannel =NULL;
            }
            NET_DVR_Logout_V30(newDNode->m_iuserid);
            newDNode->m_iuserid = -1;
            m_qlistdevicedata.append(*newDNode);
            delete newDNode;
            newDNode =NULL;
            treedatastring =getStringFromList(m_qlistdevicedata);
            qDebug()<<"treedatastring = "<<treedatastring;
            showDeviceTree(treedatastring);
            return;
            //8000 devices end
        }

        //9000 IPC接入模拟通道
        for (int i=0;i< MAX_ANALOG_CHANNUM;i++)
        {
            if (1== ipcfg.byAnalogChanEnable[i])
            {
                ChannelData *newChannel = new ChannelData;
                QString name="Cameral";
                QString num = QString::number(i+1, 10) ;
                name.append(num);
                //填充通道初始属性内容,初始设为TCP+主码流
                newChannel->setChannelName(name);
                newChannel->setChannelNum(i+1);
                newChannel->setProtocolType(TCP);
                newChannel->setStreamType(MAINSTREAM);

                //添加进设备节点
                newDNode->m_qlistchanneldata.append(*newChannel);
            }
        }

        //9000 IPC接入IP通道  这是我们要用到的信息，上面的都是针对其他设备的
        for (int i=0;i< MAX_IP_CHANNEL;i++)
        {
//QMessageBox::information(this,tr("ipcfg"), tr("ipcfg.struIPChanInfo[%1].byIPID=%2").arg(i).arg(ipcfg.struIPChanInfo[i].byIPID));

            if (0 != ipcfg.struIPChanInfo[i].byIPID)  //IP设备ID低8位，当设备ID为0时表示通道不可用
            {
                ChannelData *newChannel = new ChannelData;
                QString name="IPCameral";
                QString num = QString::number(ipcfg.struIPChanInfo[i].byIPID, 10) ;
                name.append(num);
                //填充通道初始属性内容,初始设为TCP+主码流
                newChannel->setChannelName(name);
                newChannel->setChannelNum(32+ipcfg.struIPChanInfo[i].byIPID);
                newChannel->setProtocolType(TCP);
                newChannel->setStreamType(MAINSTREAM);
                //添加进设备节点
                newDNode->m_qlistchanneldata.append(*newChannel);
                delete newChannel;
                newChannel =NULL;
            }
        }
        NET_DVR_Logout_V30(newDNode->m_iuserid);
        newDNode->m_iuserid = -1;
        m_qlistdevicedata.append(*newDNode);
        delete newDNode;
        treedatastring =getStringFromList(m_qlistdevicedata);
        qDebug()<<"treedatastring = "<<treedatastring;
        showDeviceTree(treedatastring);
        m_bislogin = TRUE;
        return;
    }
}

//单击树结点，对一些状态索引进行改变
void VisionAlgMain::pressedTreeView(const QModelIndex &index)
{
    //目的是设置当前有效地设备和通道信息，方案是先区分是设备树，设备还是通道
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
    if(level==0)  //根节点
    {
        m_iposttreelevel = 0;
    }
    else if (level==1)  //设备
    {
        m_qtreemodelindex = index;
        m_iposttreelevel = 1;
    }
    else if(level==2)  //通道
    {
        m_qtreemodelindex = index;
        m_iposttreelevel = 2;
    }
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
    if (level == 0)  //根节点
    {
        m_iposttreelevel= 0;
    }
    else if (level == 1)  //设备结点
    {
        m_qtreemodelindex = index;
        m_iposttreelevel = 1;
    }
    else if (level == 2)  //通道结点
    {
        m_qtreemodelindex = index;
        m_iposttreelevel = 2;
    }

    //双击的是根节点
    if (m_iposttreelevel == 0)
    {
        return;
    }
    //当前点击的是一台设备，点击设备时播放第一个通道
    else if (m_iposttreelevel == 1)
    {
        int devicerow = index.row();  //获取设备在左侧树的索引
        int i=0;
        QList<DeviceData>::iterator it;
        for ( it = m_qlistdevicedata.begin(); it != m_qlistdevicedata.end();++it)
        {
            if (i == devicerow)  //点击第一台的时候就是初始为0的情况，无需做多余操作
            {
                break;//以设别名称作为关键字，区别不同设备。
            }
            i++;
        }

        m_gcurrentuserid =(*it).getUsrID();

//        QMessageBox::information(this,tr("treeBeenClicked"),tr("device"));

        //如果被点击的设备还没有登录
        if (m_gcurrentuserid < 0)
        {
            int i = -1;
            //登录
            i=NET_DVR_Login_V30((*it).getIP().toLatin1().data(), \
                (*it).getPort(), \
                (*it).getUsrName().toLatin1().data(), \
                (*it).getPasswd().toLatin1().data(), \
                &(*it).m_deviceinfo);
            //登录失败
            if (-1 == i )
            {
                QMessageBox::information(this,tr("NET_DVR_Login_V30"),tr("设备登录失败，SDK_LAST_ERROR=%1").arg(NET_DVR_GetLastError()));
                //将索引引用的项目设置为折叠或展开，具体取决于展开的值
                ui->DVRsets_treeView->setExpanded(index, 1);
                return;
            }
            else  //登陆成功
            {
                m_gcurrentuserid = i;
                (*it).setUsrID(i);
                QStandardItem *item = m_gmodel->itemFromIndex(index);
                item->setIcon(QIcon(":/image/login.bmp"));
            }
        }
        m_gcurrentchannelnum = 0;
        memcpy(&m_gcurrentdeviceinfo, &(*it).m_deviceinfo,sizeof(NET_DVR_DEVICEINFO_V30));
        //preview
//        //播放视频
//        if ((m_realplay->m_rpstartstopflag == 1)&&(m_guseridbackup != m_gcurrentuserid))
//        {
//            m_realplay->on_pushButton_realplay_clicked();
//        }
//        m_realplay->on_pushButton_realplay_clicked();

        //Change the user id of parameters configuring.
        m_guseridbackup = m_gcurrentuserid;
        m_gchannelnumbackup =0;
        ui->DVRsets_treeView->setExpanded(index,0);
        return;
    }

    //当前点击的是一个通道
    else if (m_iposttreelevel == 2)
    {
        int deviceindex = index.parent().row();  //点击通道所属设备在树中的索引
        int channelindex = index.row();  //点击通道的索引

        //need first find device then channel then set the channel num
        QList<DeviceData>::iterator it;
        QList<ChannelData>::iterator it_channel;

        int i=0;
        for ( it = m_qlistdevicedata.begin(); it != m_qlistdevicedata.end(); ++it)
        {
            if (i ==deviceindex)
            {
                break;
            }
            i++;
        }
        m_gcurrentuserid =(*it).getUsrID();
        //如果设备没有登录先登录
        if (m_gcurrentuserid < 0)
        {
            int ret=-1;
            ret=NET_DVR_Login_V30((*it).getIP().toLatin1().data(), (*it).getPort(),
                    (*it).getUsrName().toLatin1().data(),
                    (*it).getPasswd().toLatin1().data(), &(*it).m_deviceinfo) ;
            //登录失败
            if (-1 == ret )
            {
                QMessageBox::information(this,tr("NET_DVR_Login_V30"),tr(\
                    "SDK_LAST_ERROR=%1").arg(NET_DVR_GetLastError()));
                return;
            }
            else  //登录成功
            {
                m_gcurrentuserid = ret;
                (*it).setUsrID(ret);
                QStandardItem *item = m_gmodel->itemFromIndex(index.parent());
                item->setIcon(QIcon(":/image/login.bmp"));
            }
        }
        //目前设备已经登陆成功，开始寻找点击的通道
        memcpy(&m_gcurrentdeviceinfo, &(*it).m_deviceinfo,sizeof(NET_DVR_DEVICEINFO_V30));
        i=0;
        for (it_channel = (*it).m_qlistchanneldata.begin(); it_channel !=
                    (*it).m_qlistchanneldata.end();++it_channel)
        {
            if (i == channelindex)
            {
                //找到了通道 与上次点击的设备进行比较，处理会不相同
                if (m_guseridbackup != m_gcurrentuserid)
                {
                    //如果在播放状态先关闭之前的预览
//                    if (m_realplay->m_rpstartstopflag == 1)
//                    {
//                        m_realplay->on_pushButton_realplay_clicked();
//                    }
//                    m_realplay->on_pushButton_realplay_clicked();

                }
                else
                {
                //同一台设备的通道被连续点击两次
                    if (m_gchannelnumbackup != (*it_channel).getChannelNum())
                    {
//                        if ((m_gchannelnumbackup != 0)&&(m_realplay->m_rpstartstopflag ==1))
//                        {
//                            m_realplay->on_pushButton_realplay_clicked();
//                        }

                    }

//                    m_realplay->on_pushButton_realplay_clicked();

                }

            m_gchannelnumbackup = (*it_channel).getChannelNum();
            m_gcurrentchannelnum = (*it_channel).getChannelNum();
            m_guseridbackup = m_gcurrentuserid;
            break;
        }
        //没找到则i++
        i++;
    }

    return;
    }
}

//从设备列表中获取设备字符串
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

    m_gmodel = new TreeModel(nodedata);
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
    QModelIndex tmpindex = m_gmodel->index(0,0);
    if (tmpindex.isValid()==1)
    {
        ui->DVRsets_treeView->setExpanded(tmpindex, 1);
    }

    QModelIndex devicetreeindex = tmpindex;
    QModelIndex deviceindex = tmpindex.child(0,0);


    QList<DeviceData>::iterator it;
    QList<ChannelData>::iterator it_channel;
    int i=0;
    it = m_qlistdevicedata.begin();
    while((deviceindex.isValid()==1)&&(it != m_qlistdevicedata.end()))
    {
        if (it->getUsrID()!=-1)
        {

            QStandardItem *item = m_gmodel->itemFromIndex(deviceindex);
            item->setIcon(QIcon(":/image/login.bmp"));
            ui->DVRsets_treeView->expand(deviceindex);
            ui->DVRsets_treeView->setExpanded(deviceindex, 1);
        }
        QModelIndex channelindex = deviceindex.child(0,0);
        it_channel = (*it).m_qlistchanneldata.begin();
        int j=0;
        while((channelindex.isValid()==1)&&(it_channel != (*it).m_qlistchanneldata.end()))
        {
            if (it_channel->getRealhandle()!=-1)
            {
                //加载上线标记图片和展开通道
                QStandardItem *item = m_gmodel->itemFromIndex(channelindex);
                item->setIcon(QIcon(":/image/play.bmp"));
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

void VisionAlgMain::on_btnMenu_Logout_clicked()
{
    if(m_gcurrentuserid<0)
    {
        QMessageBox::information(this, tr("logout information"),tr("未存在已登录的设备，双击设备"
                                                                   "或通道进行登录及播放"));
    }
    else
    {
       int deviceindex = m_qtreemodelindex.row();
       int i = 0;
       QList<DeviceData>::iterator it;
       for (it = m_qlistdevicedata.begin(); it != m_qlistdevicedata.end(); ++it)
       {
           if(i==deviceindex)
           {
               break;
           }
           i++;
       }
       if((*it).getUsrID()<0)
       {
           QMessageBox::information(this, tr("have no login yet"), tr("请点击登录按钮"));
       }
       else
       {
           if((*it).getRealPlayLabel() == 1)
           {
               QMessageBox::information(this, tr("logout error"), tr("请先停止预览"));
               return;
           }
           int ret = NET_DVR_Logout_V30((*it).m_iuserid);
           (*it).setUsrID(-1);
           m_gcurrentuserid = -1;

           //加载登出设备标记图片和收起通道
           QStandardItem *item = m_gmodel->itemFromIndex(m_qtreemodelindex);
           item->setIcon(QIcon(":/image/logout.bmp"));
           ui->DVRsets_treeView->setExpanded(m_qtreemodelindex, 0);
           QMessageBox::information(this, tr("NET_DVR_Logout_V30"), tr("注销成功"));
           return;
       }
    }
}


void VisionAlgMain::screen_full()
{
    this->setGeometry(qApp->desktop()->geometry());
    this->layout()->setContentsMargins(0, 0, 0, 0);
    ui->widget_main->layout()->setContentsMargins(0, 0, 0, 0);
    ui->widget_title->setVisible(false);
    ui->widget_left->setVisible(false);
    ui->widget_right->setVisible(false);
}

void VisionAlgMain::screen_normal()
{
    this->setGeometry(qApp->desktop()->availableGeometry());
    this->layout()->setContentsMargins(1, 1, 1, 1);
    ui->widget_main->layout()->setContentsMargins(5, 5, 5, 5);
    ui->widget_title->setVisible(true);
    ui->widget_left->setVisible(true);
    ui->widget_right->setVisible(true);
}

void VisionAlgMain::on_btn_offlinehandle_clicked()
{
    this->hide();
    emit offlineHandle();    //激活信号，让信号传送到特定界面
}

void VisionAlgMain::receiveback(){
    this->show();
}

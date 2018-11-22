#include "camera/camera.h"
#include <QDebug>
#include "utils/myapp.h"
#include <stdio.h>

Camera::Camera(void)
{
    qDebug() << "init";
}

Camera::Camera(const Camera &)
{

}

Camera& Camera::operator=(const Camera&)
{

}

Camera* Camera::camera = new Camera();
Camera* Camera::getCamera()
{
    return camera;
}

Camera::~Camera()
{
    this->isLogin = false;
    if(!this->userId<0)
    {
        this->userId=-1;
        NET_DVR_Logout(this->userId);
        NET_DVR_Cleanup();
    }
}

//初始化SDK
bool Camera::initSDK()
{
    if(NET_DVR_Init())
    {
        DWORD dwVersion = NET_DVR_GetSDKBuildVersion();  //typedef unsigned long DWORD
        this->mainVersion = (0xff000000 & dwVersion)>>24;  //主版本号
        this->subVersion = (0x00ff0000 & dwVersion)>>16;  //次版本号
        this->buildVersion = (0x0000ff00 & dwVersion)>>8;  //build版本号
        this->buildNumber = (0x000000ff & dwVersion);  //build号
        return true;
    }
    return false;
}

//登录
bool Camera::login()
{
    this->myLoginInfo.bUseAsynLogin = 0;  //同步登录方式，返回-1表示登录失败
    strcpy(this->myLoginInfo.sDeviceAddress, myApp::DeviceIp.toLatin1().data());  //IP
    this->myLoginInfo.wPort = myApp::DevicePort;  //Port
    strcpy(this->myLoginInfo.sUserName, myApp::DeviceUserName.toLatin1().data());  //UserName
    strcpy(this->myLoginInfo.sPassword, myApp::DevicePwd.toLatin1().data());  //Password

    this->userId = NET_DVR_Login_V40(&this->myLoginInfo, &this->myDeviceInfo);

    qDebug()<<"登录设备的用户ID="<<this->userId;
    qDebug()<<"设备支持的最大IP通道数="<<myDeviceInfo.struDeviceV30.byIPChanNum+myDeviceInfo.struDeviceV30.byHighDChanNum*256;
    qDebug()<<"数字通道起始通道号="<<myDeviceInfo.struDeviceV30.byStartChan;
    BOOL isEncrypt = FALSE;
    BOOL test = NET_DVR_InquestGetEncryptState(this->userId, 1, &isEncrypt);
    qDebug()<<"码流是否加密："<<test;
    if(this->userId >= 0)
    {
        this->isLogin = true;
    }
    else
    {
        this->isLogin = false;
    }
    return this->isLogin;
}

//注销摄像头  1表示未登录  2表示注销失败  0表示注销成功
int Camera::logoff()
{
    if(this->getUserId()<0)
    {
        return 1;
    }
    if(NET_DVR_Logout(this->userId))
    {
        this->isLogin = false;
        if(!this->userId<0)
        {
            this->userId=-1;
        }
        return 0;
    }
    else
    {
        return 2;
    }
}

//SDK版本号
int Camera::getMainVersion() const
{
    return mainVersion;
}
int Camera::getSubVersion() const
{
    return subVersion;
}
//build信息
int Camera::getBuildVersion() const
{
    return buildVersion;
}
int Camera::getBuildNUmber() const
{
    return buildNumber;
}
//返回是否登录
bool Camera::getIsLogin() const
{
    return this->isLogin;
}
//返回登录用户ID
int Camera::getUserId() const
{
    return this->userId;
}
//返回设备信息
NET_DVR_DEVICEINFO_V40 Camera::getMyDeviceInfo() const
{
    return this->myDeviceInfo;
}

void deviceInfo(NET_DVR_IPPARACFG_V40 ipcfg)
{
    //        qDebug()<<"通道信息->"<<"IP设备低8位："<<ipcfg.struIPChanInfo[3].byIPID<<
    //                  " IP设备高8位:"<<ipcfg.struIPChanInfo[3].byIPIDHigh<<
    //                  " 通道号:"<<ipcfg.struIPChanInfo[3].byChannel;
    //        qDebug()<<"IP通道个数："<<ipcfg.dwDChanNum<<"起始通道号："<<ipcfg.dwStartDChan;
            qDebug()<<"目前有37台设备，第一个IP设备地址："<<ipcfg.struIPDevInfo[0].struIP.sIpV4;
    //        for(int i=0; i<sizeof(ipcfg.struIPDevInfo)/sizeof(NET_DVR_IPDEVINFO_V31); i++)
    //        {
    //            qDebug()<<ipcfg.struIPDevInfo[i].struIP.sIpV4;
    //        }
    //        qDebug()<<"IP通道取流模式："<<ipcfg.struStreamMode[0].byGetStreamType<<"0为从设备取流，1为从流媒体取流";
}

map<string, string> Camera::cameraIp2Name()
{
    map<string, string> ip2Name;
    ip2Name["172.16.27.101"] = "6楼南619";
    ip2Name["172.16.27.102"] = "6楼北603";
    ip2Name["172.16.27.103"] = "6楼北605";
    ip2Name["172.16.27.104"] = "6楼北606";
    ip2Name["172.16.27.105"] = "6楼北608-1";
    ip2Name["172.16.27.106"] = "6楼北608-2";
    ip2Name["172.16.27.107"] = "6楼北608-3";
    ip2Name["172.16.27.109"] = "6楼北611";
    ip2Name["172.16.27.111"] = "7楼南705";
    ip2Name["172.16.27.112"] = "7楼南706";
    ip2Name["172.16.27.113"] = "7楼南712";
    ip2Name["172.16.27.113"] = "7楼南715";
    ip2Name["172.16.27.113"] = "7楼北702";

//    ip2Name["172.16.27.120"] = "7楼北702";
//    ip2Name["172.16.27.121"] = "7楼北702";
//    ip2Name["172.16.27.122"] = "7楼北702";
//    ip2Name["172.16.27.123"] = "7楼北702";
//    ip2Name["172.16.27.124"] = "7楼北702";
//    ip2Name["172.16.27.125"] = "7楼北702";
//    ip2Name["172.16.27.126"] = "7楼北702";
//    ip2Name["172.16.27.127"] = "7楼北702";
//    ip2Name["172.16.27.128"] = "7楼北702";
//    ip2Name["172.16.27.129"] = "7楼北702";
//    ip2Name["172.16.27.130"] = "7楼北702";
//    ip2Name["172.16.27.131"] = "7楼北702";
//    ip2Name["172.16.27.132"] = "7楼北702";
//    ip2Name["172.16.27.133"] = "7楼北702";
//    ip2Name["172.16.27.134"] = "7楼北702";
//    ip2Name["172.16.27.135"] = "7楼北702";
//    ip2Name["172.16.27.136"] = "7楼北702";
//    ip2Name["172.16.27.137"] = "7楼北702";
//    ip2Name["172.16.27.138"] = "7楼北702";
//    ip2Name["172.16.27.139"] = "7楼北702";
    return ip2Name;
}

//封装自定义设备信息
bool Camera::setDeviceData()
{
    this->deviceData = new DeviceData;
    this->deviceData->m_qdevicename = myApp::DeviceNodeName;  //设备名称
    this->deviceData->m_qip = myApp::DeviceIp;  //设备IP
    this->deviceData->m_qiport = myApp::DevicePort;  //设备端口
    this-> deviceData->m_qusername = myApp::DeviceUserName;  //用户名
    this->deviceData->m_qpassword = myApp::DevicePwd;  //密码
    this->deviceData->m_iuserid = userId;  //用户ID
    this->deviceData->m_deviceinfo = myDeviceInfo.struDeviceV30;  //NET_DVR_Login_V30()参数结构
    int max_channel = myDeviceInfo.struDeviceV30.byIPChanNum+myDeviceInfo.struDeviceV30.byHighDChanNum*256;
    NET_DVR_IPPARACFG_V40 ipcfg;  //新版SDK取设备详细的IP资源信息
//    NET_DVR_IPPARACFG ipcfg;
    DWORD Bytesreturned;  //实际收到的数据长度指针
    //通过远程参数配置接口NET_DVR_GetDVRConfig获取设备详细的IP资源信息的结构体，获取成功返回1，否则返回0
    //输入参数：用户ID、设备配置命令、通道号、接收数据的缓冲指针、接收数据的缓冲长度（单位：字节）、实际收到的数据长度指针
    int status = NET_DVR_GetDVRConfig(this->deviceData->m_iuserid, NET_DVR_GET_IPPARACFG_V40,0,&ipcfg,sizeof(NET_DVR_IPPARACFG_V40),&Bytesreturned);
//    int status = NET_DVR_GetDVRConfig(this->deviceData->m_iuserid, NET_DVR_GET_IPPARACFG,0,&ipcfg,sizeof(NET_DVR_IPPARACFG),&Bytesreturned);
    if (status)
    {
        deviceInfo(ipcfg);  //debug信息
        map<string, string> ip2Name = this->cameraIp2Name();
        for (int i=0;i< max_channel;i++)
        {
            if (0 != ipcfg.struStreamMode[i].uGetStream.struChanInfo.byIPID)  //IP设备ID低8位，当设备ID为0时表示通道不可用,目前是从1到37
            {
                ChannelData *newChannel = new ChannelData;
//                QString name="IPCameral";
//                QString num = QString::number(ipcfg.struStreamMode[i].uGetStream.struChanInfo.byIPID, 10) ;
                string ip = ipcfg.struIPDevInfo[i].struIP.sIpV4;
                QString name="";
                if(ip2Name.find(ip)!=ip2Name.end())
                {
                    name=QString::fromStdString(ip2Name[ip]);
                }else {
                    name=QString::fromStdString(ip);
                }
//                name.append(num);
                //填充通道初始属性内容,初始设为TCP+主码流
                newChannel->setChannelName(name);
                newChannel->setChannelNum(ipcfg.struStreamMode[i].uGetStream.struChanInfo.byIPID);  //目前是从1到37
                newChannel->setProtocolType(TCP);
                newChannel->setStreamType(MAINSTREAM);
                //添加进设备节点
                deviceData->m_qlistchanneldata.append(*newChannel);
                delete newChannel;
                newChannel =NULL;
            }
        }
    }
    listDeviceData.append(*(this->deviceData));
    return true;
}



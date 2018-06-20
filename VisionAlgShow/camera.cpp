#include "camera.h"
#include <QDebug>
#include "myapp.h"
#include <stdio.h>

Camera::Camera(void)
{
    qDebug() << "init";
}

Camera::~Camera()
{
    this->isLogin = false;
    NET_DVR_Logout(this->userId);
    NET_DVR_Cleanup();
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
    strcpy(this->myLoginInfo.sDeviceAddress, myApp::DeviceIp.toLatin1().data());
    this->myLoginInfo.wPort = myApp::DevicePort;
    strcpy(this->myLoginInfo.sUserName, myApp::DeviceUserName.toLatin1().data());
    strcpy(this->myLoginInfo.sPassword, myApp::DevicePwd.toLatin1().data());
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
    this->deviceData->m_deviceinfo = myDeviceInfo.struDeviceV30;
    int max_channel = myDeviceInfo.struDeviceV30.byIPChanNum+myDeviceInfo.struDeviceV30.byHighDChanNum*256;
    NET_DVR_IPPARACFG_V40 ipcfg;
//    NET_DVR_IPPARACFG ipcfg;
    DWORD Bytesreturned;
    //通过远程参数配置接口NET_DVR_GetDVRConfig获取设备详细的IP资源信息，获取成功返回1，否则返回0
    int status = NET_DVR_GetDVRConfig(this->deviceData->m_iuserid, NET_DVR_GET_IPPARACFG_V40,0,&ipcfg,sizeof(NET_DVR_IPPARACFG_V40),&Bytesreturned);
//    int status = NET_DVR_GetDVRConfig(this->deviceData->m_iuserid, NET_DVR_GET_IPPARACFG,0,&ipcfg,sizeof(NET_DVR_IPPARACFG),&Bytesreturned);
    if (status)
    {
//        qDebug()<<"通道信息->"<<"IP设备低8位："<<ipcfg.struIPChanInfo[3].byIPID<<
//                  " IP设备高8位:"<<ipcfg.struIPChanInfo[3].byIPIDHigh<<
//                  " 通道号:"<<ipcfg.struIPChanInfo[3].byChannel;
        qDebug()<<"IP通道个数："<<ipcfg.dwDChanNum<<"起始通道号："<<ipcfg.dwStartDChan;
        qDebug()<<"目前有36台设备，第一个IP设备地址："<<ipcfg.struIPDevInfo[0].struIP.sIpV4;
        qDebug()<<"IP通道取流模式："<<ipcfg.struStreamMode[0].byGetStreamType<<"0为从设备取流，1为从流媒体取流";
        for (int i=0;i< max_channel;i++)
        {
            if (0 != ipcfg.struStreamMode[i].uGetStream.struChanInfo.byIPID)  //IP设备ID低8位，当设备ID为0时表示通道不可用
            {
                ChannelData *newChannel = new ChannelData;
                QString name="IPCameral";
                QString num = QString::number(ipcfg.struStreamMode[i].uGetStream.struChanInfo.byIPID, 10) ;
                name.append(num);
                //填充通道初始属性内容,初始设为TCP+主码流
                newChannel->setChannelName(name);
                newChannel->setChannelNum(ipcfg.struStreamMode[i].uGetStream.struChanInfo.byIPID);
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

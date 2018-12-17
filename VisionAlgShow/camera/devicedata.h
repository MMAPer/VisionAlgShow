#ifndef DEVICEDATA_H
#define DEVICEDATA_H

#include <QString>
#include <QList>

#include "channeldata.h"
#include "public.h"

class ChannelData;

class DeviceData
{
public:
    friend class VisionAlgMain;
    friend class Camera;
    friend class RealPlay;
    DeviceData();
    ~DeviceData();
    void setRealPlayLabel(int value);
    int getRealPlayLabel();

    void setUsrID(int id);
    int getUsrID();

    void setDeviceName(QString devicename);
    QString getDeviceName();

    void setIP(QString  ip);
    QString getIP();

    void setPort(int port);
    int getPort();

    void setUsrName(QString usrname);
    QString getUsrName();

    void setPasswd(QString passwd);
    QString getPasswd();

    void setMultiCast(QString multiCast);
    QString getMultiCast();

    NET_DVR_DEVICEINFO_V30 getDeviceInfo();

    void setDeployState(int deployed);
    int  getDeployState();

    int modifyChannelDataChild(ChannelData *channel);

private:
    //正在预览或录像0假1真
    int     m_irealplaying;
    //登陆设备后返回的用户ID；不存入文件
    int     m_iuserid;
    //登陆设备后返回设备信息；不存入文件
    NET_DVR_DEVICEINFO_V30 m_deviceinfo;
    //设备名称，存入文件
    QString m_qdevicename;
    //设备IP，存入文件
    QString m_qip;
    //设备端口,存入文件
    int     m_qiport;
    //用户名，存入文件
    QString m_qusername;
    //用户密码，存入文件
    QString m_qpassword;
    //布防中 >=0  否则 -1
    int m_ideployed;
    //多播ip地址
    QString m_multiCast;
    //子属性通道节点列表，其内容也要存入文件；
    QList<ChannelData> m_qlistchanneldata;
};
#endif // DEVICEDATA_H

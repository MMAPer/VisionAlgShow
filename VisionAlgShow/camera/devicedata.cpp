#include "devicedata.h"

DeviceData::DeviceData()
{
    m_irealplaying =0;
    m_iuserid = -1;
    memset(&m_deviceinfo,0,sizeof(NET_DVR_DEVICEINFO_V30));
    m_qdevicename = "";
    m_qip = "";
    m_qiport = 0;
    m_qusername = "";
    m_qpassword = "";
    m_ideployed = -1;
    m_multiCast = "234.5.6.7";
}


DeviceData::~DeviceData()
{

}


void DeviceData::setRealPlayLabel(int value)
{
    m_irealplaying =value;
}


int DeviceData::getRealPlayLabel()
{
    return m_irealplaying;
}


void DeviceData::setUsrID(int id)
{
    m_iuserid = id;
}


int DeviceData::getUsrID()
{
    return m_iuserid;
}


void DeviceData::setDeviceName(QString devicename)
{
    m_qdevicename = devicename;
}


QString DeviceData::getDeviceName()
{
    return m_qdevicename;
}


void DeviceData::setIP(QString  ip)
{
    m_qip = ip;
}


QString DeviceData::getIP()
{
    return m_qip;
}


void DeviceData::setPort(int port)
{
    m_qiport = port;
}


int DeviceData::getPort()
{
    return m_qiport;
}


void DeviceData::setUsrName(QString usrname)
{
    m_qusername = usrname;
}

QString DeviceData::getUsrName()
{
    return m_qusername;
}


void DeviceData::setPasswd(QString passwd)
{
    m_qpassword = passwd;
}


QString DeviceData::getPasswd()
{
    return m_qpassword;
}


int DeviceData::modifyChannelDataChild(ChannelData *channel)
{
    for ( QList<ChannelData >::Iterator it = m_qlistchanneldata.begin(); it !=
    m_qlistchanneldata.end(); ++it )
    {
        if ((*it).m_ichannelnum == channel->m_ichannelnum)
        {
            (*it).m_qchannelname = channel->m_qchannelname;
            (*it).m_eprotocoltype = channel->m_eprotocoltype;
            (*it).m_estreamtype = channel->m_estreamtype;
            return 0;
        }
    }
    return -1;
}

NET_DVR_DEVICEINFO_V30 DeviceData::getDeviceInfo()
{

    return m_deviceinfo;
}

void DeviceData::setDeployState(int deployed)
{
    m_ideployed = deployed;
}

int  DeviceData::getDeployState()
{
    return m_ideployed;
}


void DeviceData::setMultiCast(QString multiCast)
{
    m_multiCast = multiCast;
}


QString DeviceData::getMultiCast()
{
    return m_multiCast;
}

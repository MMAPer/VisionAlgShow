#include "channeldata.h"


ChannelData::ChannelData()
{
    m_qchannelname = "";
    m_ichannelnum = -1;
	m_irealhandle = -1;
    m_eprotocoltype = TCP;
    m_estreamtype = MAINSTREAM;
}


ChannelData::~ChannelData()
{

}


void ChannelData::setChannelName(QString name)
{
    m_qchannelname = name;
}


QString ChannelData::getChannelName()
{
    return m_qchannelname;
}


void ChannelData::setChannelNum(int num)
{
    m_ichannelnum = num;
}


int ChannelData::getChannelNum()
{
    return m_ichannelnum;
}

void ChannelData::setProtocolType(PROTOCOL type)
{
    m_eprotocoltype = type;
}


PROTOCOL ChannelData::getProtocolType()
{
    return m_eprotocoltype;
}


QString ChannelData::getProtocolTypeQstring()
{
    QString protocol;
    switch (m_eprotocoltype)
    {
    case TCP:
        protocol="TCP";
        break;
    case UDP:
        protocol="UDP";
        break;
    case MCAST :
        protocol="MCAST";
        break;
    case RTP:
        protocol="RTP";
        break;
    case RTP_RTSP:
        protocol = "RTP/RTSP";
    default :
        protocol = "error";
        break;
    }
    return protocol;
}


void ChannelData::setStreamType(STREAMTYPE type)
{
    m_estreamtype = type;
}


STREAMTYPE ChannelData::getStreamType()
{
    return m_estreamtype;
}


QString ChannelData::getStreamTypeQstring()
{
    QString type;

    switch (m_estreamtype)
    {
    case MAINSTREAM:
        type="MAINSTREAM";
        break;
    case SUBSTREAM:
        type="SUBSTREAM";
        break;
    default :
        type = "error";
        break;
    }
    return type;
}


void ChannelData::setLinkMode()
{
    switch (m_estreamtype)
    {
    case MAINSTREAM:
            switch (m_eprotocoltype)
            {
            case TCP:
                m_ilinkmode =0x0;
                break;
            case UDP:
                m_ilinkmode=0x1;
                break;
            case MCAST :
                m_ilinkmode=0x2;
                break;
            case RTP:
                m_ilinkmode=0x3;
                break;
            case RTP_RTSP:
                m_ilinkmode=0x4;
                break;
            default :
                m_ilinkmode=0x0;
                break;
            }
            break;
    case SUBSTREAM:
            switch (m_eprotocoltype)
            {
            case TCP:
                m_ilinkmode =0x80000000;
                break;
            case UDP:
                m_ilinkmode=0x80000001;
                break;
            case MCAST :
                m_ilinkmode=0x80000002;
                break;
            case RTP:
                m_ilinkmode=0x80000003;
                break;
            case RTP_RTSP:
                m_ilinkmode=0x80000004;
                break;
            default :
                m_ilinkmode=0x80000000;
                break;
            }
            break;
        break;
    default :
        break;
    }
}

int ChannelData::getLinkMode()
{
    setLinkMode();
    return m_ilinkmode;
}


void ChannelData::setRealhandle(int num)
{
    m_irealhandle = num;
}

int ChannelData::getRealhandle()
{
    return m_irealhandle;
}
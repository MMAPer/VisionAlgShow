#ifndef CHANNELDATA_H
#define CHANNELDATA_H

#include <QString>
#include "devicedata.h"

enum PROTOCOL{TCP=0, UDP, MCAST, RTP, RTP_RTSP};
enum STREAMTYPE{MAINSTREAM = 0, SUBSTREAM};


//存储通道相关的有效数据，用于存取文件和界面显示，实时修改通道信息
class ChannelData
{
public:
    friend class DeviceData;
    friend class VisionAlgMain;
    friend class RealPlay;

    ChannelData();
    ~ChannelData();

    void setChannelName(QString name);
    QString getChannelName();

    void setChannelNum(int num);
    int getChannelNum();

    void setProtocolType(PROTOCOL type);
    PROTOCOL getProtocolType();
    QString getProtocolTypeQstring();

    void setStreamType(STREAMTYPE type);
    STREAMTYPE getStreamType();
    QString getStreamTypeQstring();

    void setLinkMode();
    int getLinkMode();

    void setRealhandle(int num);
    int getRealhandle();

private:
    //通道名称
    QString m_qchannelname;
    //通道号
    int m_ichannelnum;
    //传输协议，有TCP，UDP，多播和RTP等
    PROTOCOL m_eprotocoltype;
    //码流类型，分为主码流和子码流
    STREAMTYPE m_estreamtype;
    int m_ilinkmode;
    int m_irealhandle;
};

#endif // CHANNELDATA_H

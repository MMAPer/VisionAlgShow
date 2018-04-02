#ifndef MYAPP_H
#define MYAPP_H

class QString;

/* 说明:系统本身的信息及通用功能
 * 功能:关于该平台的全局属性，以及与算法无关的功能模块（如读取文件等）
 * 作者:张精制  QQ:852370792
 * 时间:2018-03-10  检查:2018-3-10
 */
class myApp
{
public:
    static QString AppPath;                 //应用程序路径
    static int DeskWidth;                   //桌面可用宽度
    static int DeskHeight;                  //桌面可用高度

    static QString AppTitle;                //界面标题
    static QString NVRType;                 //硬盘录像机厂家
    static QString IPCType;                 //网络摄像机厂家
    static QString AppStyle;                //界面样式
    static QString VideoType;             //当前画面展示类型

    static QString DeviceIp;
    static int DevicePort;
    static QString DeviceUserName;
    static QString DevicePwd;
    static QString DeviceNodeName;
    static QString DeviceStreamPwd;  //码流密码

    static void ReadConfig();               //读取配置文件
    static void WriteConfig();              //写入配置文件

};

#endif // MYAPP_H

#ifndef CAMERA_H
#define CAMERA_H

#include <opencv2/opencv.hpp>
#include "Public.h"
#include <iostream>
#include "devicedata.h"
#include "channeldata.h"
#include <QList>
using namespace cv;

class Camera
{
public:
    friend class VisionAlgMain;
    Camera();
    ~Camera();
    //初始化SDK
    bool initSDK();
    //登录
    bool login();
//    Mat getInitFrame();
//    bool getCurrentFrame();
    //SDK版本号
    int getMainVersion() const;\
    int getSubVersion() const;
    //build信息
    int getBuildVersion() const;
    int getBuildNUmber() const;
    //返回是否登录
    bool getIsLogin() const;
    //返回登录ID
    int getUserId() const;
    //返回设备信息
    NET_DVR_DEVICEINFO_V40 getMyDeviceInfo() const;
    //封装设备信息
    bool setDeviceData();

private:
    //SDK信息
    int mainVersion;  //主版本号
    int subVersion;  //次版本号
    int buildVersion;  //build版本号
    int buildNumber;  //build号
    int userId;  //登录设备的用户ID
    bool isLogin;  //是否正在登录
    DeviceData *deviceData;  //设备信息
    QList<DeviceData> listDeviceData;  //DeviceData链表
    NET_DVR_USER_LOGIN_INFO myLoginInfo;  //登录参数，包括设备地址、登录用户、密码等
    NET_DVR_DEVICEINFO_V40 myDeviceInfo;  //设备信息，
};

#endif // CAMERA_H

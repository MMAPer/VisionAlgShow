#include "camera/camera.h"
#include <QDebug>
#include "utils/myapp.h"
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>


#define USECOLOR 1
#define WINAPI

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

LONG nPort[4] = {-1};
HWND hWnd[4] = {NULL};
pthread_mutex_t g_cs_frameList[4];
list<Mat> g_frameList_1, g_frameList_2, g_frameList_3, g_frameList_4;
LONG lUserID;
NET_DVR_AES_KEY_INFO keyInfo;  //新版SDK取设备详细的IP资源信息
LONG lRealPlayHandle = -1;
char *aesKey;


struct PlayChannel
{
    int channelNum;
};

void yv12toYUV(char *outYuv, char *inYv12, int width, int height, int widthStep)
{
    int col, row;
    unsigned int Y, U, V;
    int tmp;
    int idx;
    //printf("widthStep=%d.\n",widthStep);
    for (row = 0; row<height; row++)
    {
        idx = row * widthStep;
        int rowptr = row*width;
        for (col = 0; col<width; col++)
        {
            //int colhalf=col>>1;
            tmp = (row / 2)*(width / 2) + (col / 2);
            //         if((row==1)&&( col>=1400 &&col<=1600))
            //         { //          printf("col=%d,row=%d,width=%d,tmp=%d.\n",col,row,width,tmp);
            //          printf("row*width+col=%d,width*height+width*height/4+tmp=%d,width*height+tmp=%d.\n",row*width+col,width*height+width*height/4+tmp,width*height+tmp);
            //         }
            Y = (unsigned int)inYv12[row*width + col];
            U = (unsigned int)inYv12[width*height + width*height / 4 + tmp];
            V = (unsigned int)inYv12[width*height + tmp];
            //         if ((col==200))
            //         {
            //         printf("col=%d,row=%d,width=%d,tmp=%d.\n",col,row,width,tmp);
            //         printf("width*height+width*height/4+tmp=%d.\n",width*height+width*height/4+tmp);
            //         return ;
            //         }
            if ((idx + col * 3 + 2)> (1200 * widthStep))
            {
                //printf("row * widthStep=%d,idx+col*3+2=%d.\n",1200 * widthStep,idx+col*3+2);
            }
            outYuv[idx + col * 3] = Y;
            outYuv[idx + col * 3 + 1] = U;
            outYuv[idx + col * 3 + 2] = V;
        }
    }
    //printf("col=%d,row=%d.\n",col,row);
}

//解码回调 视频为YUV数据(YV12)，音频为PCM数据
void CALLBACK DecCBFun1(int nPort, char * pBuf, int nSize, FRAME_INFO * pFrameInfo, void * nReserved1, int nReserved2)
{
    long lFrameType = pFrameInfo->nType;
//            cout << "nType=" << lFrameType << endl;
    if (lFrameType == T_YV12)
    {
        #if USECOLOR
        //int start = clock();
        static IplImage* pImgYCrCb = cvCreateImage(cvSize(pFrameInfo->nWidth, pFrameInfo->nHeight), 8, 3);
        yv12toYUV(pImgYCrCb->imageData, pBuf, pFrameInfo->nWidth, pFrameInfo->nHeight, pImgYCrCb->widthStep);
        static IplImage* pImg = cvCreateImage(cvSize(pFrameInfo->nWidth, pFrameInfo->nHeight), 8, 3);
        cvCvtColor(pImgYCrCb, pImg, CV_YCrCb2RGB);
        //int end = clock();
        #else
        static IplImage* pImg = cvCreateImage(cvSize(pFrameInfo->nWidth, pFrameInfo->nHeight), 8, 1);
        memcpy(pImg->imageData, pBuf, pFrameInfo->nWidth*pFrameInfo->nHeight);
        #endif
        //printf("%d\n",end-start);
        //Mat frametemp(pImg), frame;
        //frametemp.copyTo(frame);
        //      cvShowImage("IPCamera",pImg);
        //      cvWaitKey(1);
        pthread_mutex_lock(&g_cs_frameList[0]);
        Mat mat=cvarrToMat(pImg);

        g_frameList_1.push_back(mat);
        pthread_mutex_unlock(&g_cs_frameList[0]);

        #if USECOLOR
        //      cvReleaseImage(&pImgYCrCb);
        //      cvReleaseImage(&pImg);
        #else
        /*cvReleaseImage(&pImg);*/
        #endif
        //此时是YV12格式的视频数据，保存在pBuf中，可以fwrite(pBuf,nSize,1,Videofile);
        //fwrite(pBuf,nSize,1,fp);
    }
}

///实时流回调
void CALLBACK fRealDataCallBack1(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser)
{
    DWORD dRet;
    switch (dwDataType)
    {
    case NET_DVR_SYSHEAD: //系统头
        if (!PlayM4_GetPort(&nPort[0])) //获取播放库未使用的通道号
        {
            break;
        }
        if (dwBufSize > 0)
        {
            int res = PlayM4_SetSecretKey(nPort[0], 1, aesKey, 128);
            cout << "res = " << res << endl;
            if (!PlayM4_OpenStream(nPort[0], pBuffer, dwBufSize, 1024 * 1024))
            {
                dRet = PlayM4_GetLastError(nPort[0]);
                break;
            }
            //设置解码回调函数 只解码不显示
            if (!PlayM4_SetDecCallBack(nPort[0], DecCBFun1))
            {
                dRet = PlayM4_GetLastError(nPort[0]);
                break;
            }
            //设置解码回调函数 解码且显示
            //if (!PlayM4_SetDecCallBackEx(nPort,DecCBFun,NULL,NULL))
            //{
            //  dRet=PlayM4_GetLastError(nPort);
            //  break;
            //}
            //打开视频解码
            if (!PlayM4_Play(nPort[0], hWnd[0]))
            {
                dRet = PlayM4_GetLastError(nPort[0]);
                break;
            }
            //打开音频解码, 需要码流是复合流
            //          if (!PlayM4_PlaySound(nPort))
            //          {
            //              dRet=PlayM4_GetLastError(nPort);
            //              break;
            //          }
        }
        break;

    case NET_DVR_STREAMDATA: //码流数据
        if (dwBufSize > 0 && nPort[0] != -1)
        {
            BOOL inData = PlayM4_InputData(nPort[0], pBuffer, dwBufSize);
            while (!inData)
            {
                sleep(10);
                inData = PlayM4_InputData(nPort[0], pBuffer, dwBufSize);
                cout << (L"PlayM4_InputData failed \n") << endl;
            }
        }
        break;
    }
}

//void CALLBACK g_ExceptionCallBack1(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser)
//{
//    char tempbuf[256] = { 0 };
//    switch (dwType)
//    {
//    case EXCEPTION_RECONNECT: //预览时重连
//        printf("----------reconnect--------%d\n",
//               time(NULL));
//        break;
//    default: break;
//    }
//}

void * ReadCamera1(void* IpParameter)
{
    //---------------------------------------
    //设置异常消息回调函数
//    NET_DVR_SetExceptionCallBack_V30(0, NULL, g_ExceptionCallBack1, NULL);

    PlayChannel *playChannel = (PlayChannel *)IpParameter;
    int channelNum = playChannel->channelNum;
    //启动预览并设置回调数据流
    NET_DVR_CLIENTINFO ClientInfo;
    ClientInfo.lChannel = channelNum; //Channel number 设备通道号
    ClientInfo.hPlayWnd = NULL; //窗口为空，设备SDK不解码只取流
    ClientInfo.lLinkMode = 1; //Main Stream
    ClientInfo.sMultiCastIP = NULL;
    LONG lRealPlayHandle;
    lRealPlayHandle = NET_DVR_RealPlay_V30(lUserID, &ClientInfo, fRealDataCallBack1, NULL, TRUE);
    if (lRealPlayHandle<0)
    {
        printf("NET_DVR_RealPlay_V30 failed! Error number: %d\n", NET_DVR_GetLastError());
        //return -1;
    }
    else
        cout << "码流回调成功！" << endl;
    sleep(-1);
    //fclose(fp);
    //---------------------------------------
    //关闭预览
    if (!NET_DVR_StopRealPlay(lRealPlayHandle))
    {
        printf("NET_DVR_StopRealPlay error! Error number: %d\n", NET_DVR_GetLastError());
        return 0;
    }
    //注销用户
    NET_DVR_Logout(lUserID);
    NET_DVR_Cleanup();
    //return 0;
}


//---------------------------------------------NO 2--------------------------------------------------------------------
//解码回调 视频为YUV数据(YV12)，音频为PCM数据
void CALLBACK DecCBFun2(int nPort, char * pBuf, int nSize, FRAME_INFO * pFrameInfo, void * nReserved1, int nReserved2)
{
    long lFrameType = pFrameInfo->nType;
//            cout << "nType=" << lFrameType << endl;
    if (lFrameType == T_YV12)
    {
        #if USECOLOR
        //int start = clock();
        static IplImage* pImgYCrCb2 = cvCreateImage(cvSize(pFrameInfo->nWidth, pFrameInfo->nHeight), 8, 3);
        yv12toYUV(pImgYCrCb2->imageData, pBuf, pFrameInfo->nWidth, pFrameInfo->nHeight, pImgYCrCb2->widthStep);
        static IplImage* pImg2 = cvCreateImage(cvSize(pFrameInfo->nWidth, pFrameInfo->nHeight), 8, 3);
        cvCvtColor(pImgYCrCb2, pImg2, CV_YCrCb2RGB);
        //int end = clock();
        #else
        static IplImage* pImg2 = cvCreateImage(cvSize(pFrameInfo->nWidth, pFrameInfo->nHeight), 8, 1);
        memcpy(pImg2->imageData, pBuf, pFrameInfo->nWidth*pFrameInfo->nHeight);
        #endif
        pthread_mutex_lock(&g_cs_frameList[1]);
        Mat mat=cvarrToMat(pImg2);

        g_frameList_2.push_back(mat);
        pthread_mutex_unlock(&g_cs_frameList[1]);

        #if USECOLOR
        //      cvReleaseImage(&pImgYCrCb);
        //      cvReleaseImage(&pImg);
        #else
        /*cvReleaseImage(&pImg2);*/
        #endif
        //此时是YV12格式的视频数据，保存在pBuf中，可以fwrite(pBuf,nSize,1,Videofile);
        //fwrite(pBuf,nSize,1,fp);
    }
}

///实时流回调
void CALLBACK fRealDataCallBack2(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser)
{
    DWORD dRet;
    switch (dwDataType)
    {
    case NET_DVR_SYSHEAD: //系统头
        if (!PlayM4_GetPort(&nPort[1])) //获取播放库未使用的通道号
        {
            break;
        }
        if (dwBufSize > 0)
        {
            int res = PlayM4_SetSecretKey(nPort[1], 1, aesKey, 128);
            cout << "res = " << res << endl;
            if (!PlayM4_OpenStream(nPort[1], pBuffer, dwBufSize, 1024 * 1024))
            {
                dRet = PlayM4_GetLastError(nPort[1]);
                break;
            }
            //设置解码回调函数 只解码不显示
            if (!PlayM4_SetDecCallBack(nPort[1], DecCBFun2))
            {
                dRet = PlayM4_GetLastError(nPort[1]);
                break;
            }
            //打开视频解码
            if (!PlayM4_Play(nPort[1], hWnd[1]))
            {
                dRet = PlayM4_GetLastError(nPort[1]);
                break;
            }
        }
        break;

    case NET_DVR_STREAMDATA: //码流数据
        if (dwBufSize > 0 && nPort[1] != -1)
        {
            BOOL inData = PlayM4_InputData(nPort[1], pBuffer, dwBufSize);
            while (!inData)
            {
                sleep(10);
                inData = PlayM4_InputData(nPort[1], pBuffer, dwBufSize);
                cout << (L"PlayM4_InputData failed \n") << endl;
            }
        }
        break;
    }
}

void CALLBACK g_ExceptionCallBack2(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser)
{
    char tempbuf[256] = { 0 };
    switch (dwType)
    {
    case EXCEPTION_RECONNECT: //预览时重连
        printf("----------reconnect--------%d\n",
               time(NULL));
        break;
    default: break;
    }
}

void * ReadCamera2(void* IpParameter)
{
    //---------------------------------------
    //设置异常消息回调函数
    NET_DVR_SetExceptionCallBack_V30(0, NULL, g_ExceptionCallBack2, NULL);

    PlayChannel *playChannel = (PlayChannel *)IpParameter;
    int channelNum = playChannel->channelNum;
    //启动预览并设置回调数据流
    NET_DVR_CLIENTINFO ClientInfo;
    ClientInfo.lChannel = channelNum; //Channel number 设备通道号
    ClientInfo.hPlayWnd = NULL; //窗口为空，设备SDK不解码只取流
    ClientInfo.lLinkMode = 1; //Main Stream
    ClientInfo.sMultiCastIP = NULL;
    LONG lRealPlayHandle;
    lRealPlayHandle = NET_DVR_RealPlay_V30(lUserID, &ClientInfo, fRealDataCallBack2, NULL, TRUE);
    if (lRealPlayHandle<0)
    {
        printf("NET_DVR_RealPlay_V30 failed! Error number: %d\n", NET_DVR_GetLastError());
        //return -1;
    }
    else
        cout << "码流回调成功！" << endl;
    sleep(-1);
    //fclose(fp);
    //---------------------------------------
    //关闭预览
    if (!NET_DVR_StopRealPlay(lRealPlayHandle))
    {
        printf("NET_DVR_StopRealPlay error! Error number: %d\n", NET_DVR_GetLastError());
        return 0;
    }
    //注销用户
    NET_DVR_Logout(lUserID);
    NET_DVR_Cleanup();
    //return 0;
}



//---------------------------------------------NO 3--------------------------------------------------------------------
//解码回调 视频为YUV数据(YV12)，音频为PCM数据
void CALLBACK DecCBFun3(int nPort, char * pBuf, int nSize, FRAME_INFO * pFrameInfo, void * nReserved1, int nReserved2)
{
    long lFrameType = pFrameInfo->nType;
//            cout << "nType=" << lFrameType << endl;
    if (lFrameType == T_YV12)
    {
        #if USECOLOR
        //int start = clock();
        static IplImage* pImgYCrCb3 = cvCreateImage(cvSize(pFrameInfo->nWidth, pFrameInfo->nHeight), 8, 3);
        yv12toYUV(pImgYCrCb3->imageData, pBuf, pFrameInfo->nWidth, pFrameInfo->nHeight, pImgYCrCb3->widthStep);
        static IplImage* pImg3 = cvCreateImage(cvSize(pFrameInfo->nWidth, pFrameInfo->nHeight), 8, 3);
        cvCvtColor(pImgYCrCb3, pImg3, CV_YCrCb2RGB);
        //int end = clock();
        #else
        static IplImage* pImg3 = cvCreateImage(cvSize(pFrameInfo->nWidth, pFrameInfo->nHeight), 8, 1);
        memcpy(pImg3->imageData, pBuf, pFrameInfo->nWidth*pFrameInfo->nHeight);
        #endif
        pthread_mutex_lock(&g_cs_frameList[2]);
        Mat mat=cvarrToMat(pImg3);

        g_frameList_3.push_back(mat);
        pthread_mutex_unlock(&g_cs_frameList[2]);

        #if USECOLOR
        //      cvReleaseImage(&pImgYCrCb);
        //      cvReleaseImage(&pImg);
        #else
        /*cvReleaseImage(&pImg3);*/
        #endif
        //此时是YV12格式的视频数据，保存在pBuf中，可以fwrite(pBuf,nSize,1,Videofile);
        //fwrite(pBuf,nSize,1,fp);
    }
}

///实时流回调
void CALLBACK fRealDataCallBack3(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser)
{
    DWORD dRet;
    switch (dwDataType)
    {
    case NET_DVR_SYSHEAD: //系统头
        if (!PlayM4_GetPort(&nPort[2])) //获取播放库未使用的通道号
        {
            break;
        }
        if (dwBufSize > 0)
        {
            int res = PlayM4_SetSecretKey(nPort[2], 1, aesKey, 128);
            cout << "res = " << res << endl;
            if (!PlayM4_OpenStream(nPort[2], pBuffer, dwBufSize, 1024 * 1024))
            {
                dRet = PlayM4_GetLastError(nPort[2]);
                break;
            }
            //设置解码回调函数 只解码不显示
            if (!PlayM4_SetDecCallBack(nPort[2], DecCBFun3))
            {
                dRet = PlayM4_GetLastError(nPort[2]);
                break;
            }
            //打开视频解码
            if (!PlayM4_Play(nPort[2], hWnd[2]))
            {
                dRet = PlayM4_GetLastError(nPort[2]);
                break;
            }
        }
        break;

    case NET_DVR_STREAMDATA: //码流数据
        if (dwBufSize > 0 && nPort[2] != -1)
        {
            BOOL inData = PlayM4_InputData(nPort[2], pBuffer, dwBufSize);
            while (!inData)
            {
                sleep(10);
                inData = PlayM4_InputData(nPort[2], pBuffer, dwBufSize);
                cout << (L"PlayM4_InputData failed \n") << endl;
            }
        }
        break;
    }
}

void CALLBACK g_ExceptionCallBack3(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser)
{
    char tempbuf[256] = { 0 };
    switch (dwType)
    {
    case EXCEPTION_RECONNECT: //预览时重连
        printf("----------reconnect--------%d\n",
               time(NULL));
        break;
    default: break;
    }
}

void * ReadCamera3(void* IpParameter)
{
    //---------------------------------------
    //设置异常消息回调函数
    NET_DVR_SetExceptionCallBack_V30(0, NULL, g_ExceptionCallBack3, NULL);

    PlayChannel *playChannel = (PlayChannel *)IpParameter;
    int channelNum = playChannel->channelNum;
    //启动预览并设置回调数据流
    NET_DVR_CLIENTINFO ClientInfo;
    ClientInfo.lChannel = channelNum; //Channel number 设备通道号
    ClientInfo.hPlayWnd = NULL; //窗口为空，设备SDK不解码只取流
    ClientInfo.lLinkMode = 1; //Main Stream
    ClientInfo.sMultiCastIP = NULL;
    LONG lRealPlayHandle;
    lRealPlayHandle = NET_DVR_RealPlay_V30(lUserID, &ClientInfo, fRealDataCallBack3, NULL, TRUE);
    if (lRealPlayHandle<0)
    {
        printf("NET_DVR_RealPlay_V30 failed! Error number: %d\n", NET_DVR_GetLastError());
        //return -1;
    }
    else
        cout << "码流回调成功！" << endl;
    sleep(-1);
    //fclose(fp);
    //---------------------------------------
    //关闭预览
    if (!NET_DVR_StopRealPlay(lRealPlayHandle))
    {
        printf("NET_DVR_StopRealPlay error! Error number: %d\n", NET_DVR_GetLastError());
        return 0;
    }
    //注销用户
    NET_DVR_Logout(lUserID);
    NET_DVR_Cleanup();
    //return 0;
}


//----------------------------------------------NO 4------------------------------------------------
//解码回调 视频为YUV数据(YV12)，音频为PCM数据
void CALLBACK DecCBFun4(int nPort, char * pBuf, int nSize, FRAME_INFO * pFrameInfo, void * nReserved1, int nReserved2)
{
    long lFrameType = pFrameInfo->nType;
//            cout << "nType=" << lFrameType << endl;
    if (lFrameType == T_YV12)
    {
        #if USECOLOR
        //int start = clock();
        static IplImage* pImgYCrCb4 = cvCreateImage(cvSize(pFrameInfo->nWidth, pFrameInfo->nHeight), 8, 3);
        yv12toYUV(pImgYCrCb4->imageData, pBuf, pFrameInfo->nWidth, pFrameInfo->nHeight, pImgYCrCb4->widthStep);
        static IplImage* pImg4 = cvCreateImage(cvSize(pFrameInfo->nWidth, pFrameInfo->nHeight), 8, 3);
        cvCvtColor(pImgYCrCb4, pImg4, CV_YCrCb2RGB);
        //int end = clock();
        #else
        static IplImage* pImg4 = cvCreateImage(cvSize(pFrameInfo->nWidth, pFrameInfo->nHeight), 8, 1);
        memcpy(pImg4->imageData, pBuf, pFrameInfo->nWidth*pFrameInfo->nHeight);
        #endif
        pthread_mutex_lock(&g_cs_frameList[3]);
        Mat mat=cvarrToMat(pImg4);

        g_frameList_4.push_back(mat);
        pthread_mutex_unlock(&g_cs_frameList[3]);

        #if USECOLOR
        //      cvReleaseImage(&pImgYCrCb);
        //      cvReleaseImage(&pImg);
        #else
        /*cvReleaseImage(&pImg);*/
        #endif

    }
}

///实时流回调
void CALLBACK fRealDataCallBack4(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser)
{
    DWORD dRet;
    switch (dwDataType)
    {
    case NET_DVR_SYSHEAD: //系统头
        if (!PlayM4_GetPort(&nPort[3])) //获取播放库未使用的通道号
        {
            break;
        }
        if (dwBufSize > 0)
        {
            int res = PlayM4_SetSecretKey(nPort[3], 1, aesKey, 128);
            cout << "res = " << res << endl;
            if (!PlayM4_OpenStream(nPort[3], pBuffer, dwBufSize, 1024 * 1024))
            {
                dRet = PlayM4_GetLastError(nPort[3]);
                break;
            }
            //设置解码回调函数 只解码不显示
            if (!PlayM4_SetDecCallBack(nPort[3], DecCBFun4))
            {
                dRet = PlayM4_GetLastError(nPort[3]);
                break;
            }
            //打开视频解码
            if (!PlayM4_Play(nPort[3], hWnd[3]))
            {
                dRet = PlayM4_GetLastError(nPort[3]);
                break;
            }
        }
        break;

    case NET_DVR_STREAMDATA: //码流数据
        if (dwBufSize > 0 && nPort[3] != -1)
        {
            BOOL inData = PlayM4_InputData(nPort[3], pBuffer, dwBufSize);
            while (!inData)
            {
                sleep(10);
                inData = PlayM4_InputData(nPort[3], pBuffer, dwBufSize);
                cout << (L"PlayM4_InputData failed \n") << endl;
            }
        }
        break;
    }
}

void CALLBACK g_ExceptionCallBack4(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser)
{
    char tempbuf[256] = { 0 };
    switch (dwType)
    {
    case EXCEPTION_RECONNECT: //预览时重连
        printf("----------reconnect--------%d\n",
               time(NULL));
        break;
    default: break;
    }
}

void * ReadCamera4(void* IpParameter)
{
    //---------------------------------------
    //设置异常消息回调函数
    NET_DVR_SetExceptionCallBack_V30(0, NULL, g_ExceptionCallBack4, NULL);
    PlayChannel *playChannel = (PlayChannel *)IpParameter;
    int channelNum = playChannel->channelNum;
    //启动预览并设置回调数据流
    NET_DVR_CLIENTINFO ClientInfo;
    ClientInfo.lChannel = channelNum; //Channel number 设备通道号
    ClientInfo.hPlayWnd = NULL; //窗口为空，设备SDK不解码只取流
    ClientInfo.lLinkMode = 1; //Main Stream
    ClientInfo.sMultiCastIP = NULL;
    LONG lRealPlayHandle;
    lRealPlayHandle = NET_DVR_RealPlay_V30(lUserID, &ClientInfo, fRealDataCallBack4, NULL, TRUE);
    if (lRealPlayHandle<0)
    {
        printf("NET_DVR_RealPlay_V30 failed! Error number: %d\n", NET_DVR_GetLastError());
    }
    else
        cout << "码流回调成功！" << endl;
    sleep(-1);
    //关闭预览
    if (!NET_DVR_StopRealPlay(lRealPlayHandle))
    {
        printf("NET_DVR_StopRealPlay error! Error number: %d\n", NET_DVR_GetLastError());
        return 0;
    }
    //注销用户
    NET_DVR_Logout(lUserID);
    NET_DVR_Cleanup();
}

void Camera::playOneChannel(int channelNum, int windowNo)
{
    cout << "channelNum = " << channelNum << " windowNum = " << windowNo << endl;

    PlayChannel * playChannel = new PlayChannel();
    playChannel->channelNum = channelNum;
    switch(windowNo)
    {
    case 0:
        pthread_mutex_init(&g_cs_frameList[0], NULL);
        pthread_create(&hThread1, NULL, ReadCamera1, playChannel);
        break;
    case 1:
        pthread_mutex_init(&g_cs_frameList[1], NULL);
        pthread_create(&hThread2, NULL, ReadCamera2, playChannel);
        break;
    case 2:
        pthread_mutex_init(&g_cs_frameList[2], NULL);
        pthread_create(&hThread3, NULL, ReadCamera3, playChannel);
        break;
    case 3:
        pthread_mutex_init(&g_cs_frameList[3], NULL);
        pthread_create(&hThread4, NULL, ReadCamera4, playChannel);
        break;
    }

}

void Camera::pauseOneChannel(int windowNo)
{
    int status;
    int *p = &status;
    switch(windowNo)
    {
    case 0:
        pthread_mutex_unlock(&g_cs_frameList[0]);
        pthread_cancel(hThread1);
        pthread_join(hThread1, (void**)&p);
        printf("thread exit code %d\n", status);
        break;
    case 1:
        pthread_mutex_unlock(&g_cs_frameList[1]);
        pthread_cancel(hThread2);
        pthread_join(hThread2, (void**)&p);
        break;
    case 2:
        pthread_mutex_unlock(&g_cs_frameList[2]);
        pthread_cancel(hThread3);
        pthread_join(hThread3, (void**)&p);
        break;
    case 3:
        pthread_mutex_unlock(&g_cs_frameList[3]);
        pthread_cancel(hThread4);
        pthread_join(hThread4, (void**)&p);
        break;
    }
}

Mat Camera::getFrame(int windowNo)
{
    Mat frame;
    list<Mat>::iterator it;
    Mat dbgframe;
    pthread_mutex_lock(&g_cs_frameList[windowNo]);  //pthread_mutex_t g_cs_frameList;
    switch(windowNo)
    {
    case 0:
        while (!g_frameList_1.size())
        {
            pthread_mutex_unlock(&g_cs_frameList[windowNo]);
            pthread_mutex_lock(&g_cs_frameList[windowNo]);
        }
        it = g_frameList_1.end();
        it--;
        dbgframe = (*(it));
        (*g_frameList_1.begin()).copyTo(frame);
        frame = dbgframe;
        g_frameList_1.pop_front();
        break;
    case 1:
        while (!g_frameList_2.size())
        {
            pthread_mutex_unlock(&g_cs_frameList[windowNo]);
            pthread_mutex_lock(&g_cs_frameList[windowNo]);
        }
        it = g_frameList_2.end();
        it--;
        dbgframe = (*(it));
        (*g_frameList_2.begin()).copyTo(frame);
        frame = dbgframe;
        g_frameList_2.pop_front();
        break;
    case 2:
        while (!g_frameList_3.size())
        {
            pthread_mutex_unlock(&g_cs_frameList[windowNo]);
            pthread_mutex_lock(&g_cs_frameList[windowNo]);
        }
        it = g_frameList_3.end();
        it--;
        dbgframe = (*(it));
        (*g_frameList_3.begin()).copyTo(frame);
        frame = dbgframe;
        g_frameList_3.pop_front();
        break;
    case 3:
        while (!g_frameList_4.size())
        {
            pthread_mutex_unlock(&g_cs_frameList[windowNo]);
            pthread_mutex_lock(&g_cs_frameList[windowNo]);
        }
        it = g_frameList_4.end();
        it--;
        dbgframe = (*(it));
        (*g_frameList_4.begin()).copyTo(frame);
        frame = dbgframe;
        g_frameList_4.pop_front();
        break;
    }

    pthread_mutex_unlock(&g_cs_frameList[windowNo]);
    return(frame);
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

    if(this->userId >= 0)
    {
        this->isLogin = true;

        lUserID = this->userId;

        qDebug()<<"登录设备的用户ID="<<this->userId;
        qDebug()<<"设备支持的最大IP通道数="<<myDeviceInfo.struDeviceV30.byIPChanNum+myDeviceInfo.struDeviceV30.byHighDChanNum*256;
        qDebug()<<"数字通道起始通道号="<<myDeviceInfo.struDeviceV30.byStartChan;
    //    BOOL isEncrypt = FALSE;
    //    BOOL test = NET_DVR_InquestGetEncryptState(this->userId, 1, &isEncrypt);
    //    qDebug()<<"码流是否加密："<<isEncrypt;
        DWORD Bytesreturned;  //实际收到的数据长度指针
        //通过远程参数配置接口NET_DVR_GetDVRConfig获取设备详细的IP资源信息的结构体，获取成功返回1，否则返回0
        //输入参数：用户ID、设备配置命令、通道号、接收数据的缓冲指针、接收数据的缓冲长度（单位：字节）、实际收到的数据长度指针
        int status = NET_DVR_GetDVRConfig(lUserID, NET_DVR_GET_AES_KEY,1,&keyInfo,sizeof(NET_DVR_AES_KEY_INFO),&Bytesreturned);
        cout << "status = " << status << endl;
        if(status==1)
        {
            aesKey = (char *)keyInfo.sAESKey;
            cout << "aesKey = " << aesKey << endl;
        }
        else
            cout << "failed to get AES key!" << endl;
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
    ip2Name["172.16.27.101"] = "6楼北611";
    ip2Name["172.16.27.102"] = "7楼北702";
    ip2Name["172.16.27.103"] = "6楼北605";
    ip2Name["172.16.27.104"] = "6楼北606";
    ip2Name["172.16.27.105"] = "7楼南715";
//    ip2Name["172.16.27.106"] = "7楼南703";
    ip2Name["172.16.27.107"] = "6楼南619";
    ip2Name["172.16.27.108"] = "7楼南706";
    ip2Name["172.16.27.109"] = "6楼北603";
    ip2Name["172.16.27.110"] = "7楼南712";
    ip2Name["172.16.27.111"] = "7楼南705";
    ip2Name["172.16.27.112"] = "6楼北608";
    ip2Name["172.16.27.113"] = "6楼北608科研1";
    ip2Name["172.16.27.114"] = "6楼北608科研2";

    ip2Name["172.16.27.120"] = "7楼东南走廊西";
    ip2Name["172.16.27.121"] = "7楼北通道";
    ip2Name["172.16.27.122"] = "6楼西南走廊东";
    ip2Name["172.16.27.123"] = "6楼东北走廊西";
    ip2Name["172.16.27.124"] = "6楼西南走廊西";
    ip2Name["172.16.27.125"] = "6楼东北走廊东";
    ip2Name["172.16.27.126"] = "6楼东南走廊东";
    ip2Name["172.16.27.127"] = "7楼西北走廊西";
    ip2Name["172.16.27.128"] = "6楼东南走廊西";
    ip2Name["172.16.27.129"] = "6楼西北走廊西";

    ip2Name["172.16.27.130"] = "6楼西北走廊东";
    ip2Name["172.16.27.131"] = "7楼东南走廊东";
    ip2Name["172.16.27.132"] = "6楼门厅北";
    ip2Name["172.16.27.133"] = "6楼门厅南";
    ip2Name["172.16.27.134"] = "7楼东北走廊东";
    ip2Name["172.16.27.135"] = "7楼南通道";
    ip2Name["172.16.27.136"] = "7楼西南走廊东";
    ip2Name["172.16.27.137"] = "7楼西南走廊西";
    ip2Name["172.16.27.138"] = "7楼西北楼梯";
    ip2Name["172.16.27.139"] = "7楼东北楼梯";

    ip2Name["172.16.27.140"] = "6楼南通道";
    ip2Name["172.16.27.141"] = "6楼北通道";
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
                string ip = ipcfg.struIPDevInfo[i].struIP.sIpV4;
                if(ip=="172.16.27.115" || ip=="172.16.27.153")
                {
                    continue;
                }
                ChannelData *newChannel = new ChannelData;
//                QString name="IPCameral";
//                QString num = QString::number(ipcfg.struStreamMode[i].uGetStream.struChanInfo.byIPID, 10) ;

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
                newChannel->setRealPlaying(false);
                //添加进设备节点
                deviceData->m_qlistchanneldata.append(*newChannel);
                delete newChannel;
                newChannel =NULL;
            }
        }
    } else {
        cout << "error code = " << NET_DVR_GetLastError() << endl;
    }
    listDeviceData.append(*(this->deviceData));
    return true;
}



bool YV12_to_RGB24( char* pYV12,  unsigned char* pRGB24, int iWidth, int iHeight)
{
    if(!pYV12 || !pRGB24)
        return false;
    const long nYLen = long(iHeight * iWidth);
    const int nHfWidth = (iWidth>>1);
    if(nYLen<1 || nHfWidth<1)
        return false;
    // yv12数据格式，其中Y分量长度为width * height, U和V分量长度都为width * height / 4
    // |WIDTH |
    // y......y--------
    // y......y   HEIGHT
    // y......y
    // y......y--------
    // v..v
    // v..v
    // u..u
    // u..u
    char* yData = pYV12;
    char* vData = &yData[nYLen];
    char* uData = &vData[nYLen>>2];
    if(!uData || !vData)
        return false;
    // Convert YV12 to RGB24
    //
    // formula
    //                                       [1            1                        1             ]
    // [r g b] = [y u-128 v-128] [0            0.34375             0             ]
    //                                       [1.375      0.703125          1.734375]
    // another formula
    //                                       [1                   1                      1            ]
    // [r g b] = [y u-128 v-128] [0                   0.698001         0            ]
    //                                       [1.370705      0.703125         1.732446]
    int rgb[3];
    int i, j, m, n, x, y;
    m = -iWidth;
    n = -nHfWidth;
    for(y=0; y < iHeight; y++)
    {
        m += iWidth;
        if(!(y % 2))
            n += nHfWidth;
        for(x=0; x < iWidth; x++)
        {
            i = m + x;
            j = n + (x>>1);
            rgb[2] = int(yData[i] + 1.370705 * (vData[j] - 128)); // r分量值
            rgb[1] = int(yData[i] - 0.698001 * (uData[j] - 128)  - 0.703125 * (vData[j] - 128)); // g分量值
            rgb[0] = int(yData[i] + 1.732446 * (uData[j] - 128)); // b分量值
            j = nYLen - iWidth - m + x;
            i = (j<<1) + j;
            for(j=0; j<3; j++)
            {
                if(rgb[j]>=0 && rgb[j]<=255)
                    pRGB24[i + j] = rgb[j];
                else
                    pRGB24[i + j] = (rgb[j] < 0) ? 0 : 255;
            }
        }
    }
    return true;
}


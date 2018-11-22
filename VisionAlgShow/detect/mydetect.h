#ifndef _MYDETECT_H
#define _MYDETECT_H

#include "opencv2/opencv.hpp"
#include <opencv2/dnn.hpp>
#include <opencv2/dnn/all_layers.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/dnn/shape_utils.hpp>
#include <opencv2/dpm.hpp>
#include <opencv2/core.hpp>
#include <opencv2/video/background_segm.hpp>
#include <vector>

using namespace std;
using namespace cv;

struct Position
{
    int x;
    int y;
    int w;
    int h;
};

class MyDetector
{
public:
    virtual int detect(Mat &inputImg, vector<Position> &result)
    {

    }
};

class SSDDetector : public MyDetector
{
public:
    int detect(Mat &inputImg, vector<Position> &result) override;
};

class YOLODetector : public MyDetector
{
public:
    int detect(Mat &inputImg, vector<Position> &result) override;
    int loadNet();
private:
    String modelConfiguration = "../../models/detection/yolo/yolov2.cfg";  //网络结构配置文件
    String modelBinary = "../../models/detection/yolo/yolov2.weights";  //网络权重参数文件
    String classNames ="../../models/detection/yolo/coco.names";
    //String source = filePath.toStdString();
    //String  out = "../../images/output.jpg";
    String object_roi_style = "box";   // box or line style draw
    dnn::Net net;
};

class FasterRCNNDetector : public MyDetector
{
public:
    int detect(Mat &inputImg, vector<Position> &result) override;
};

class DPMDetector : public MyDetector
{
public:
    int detect(Mat &inputImg, vector<Position> &result) override;
};

class HOGSVMDetector : public MyDetector
{
public:
    int detect(Mat &inputImg, vector<Position> &result) override;
};

#endif

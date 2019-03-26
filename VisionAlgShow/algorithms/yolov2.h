#ifndef YOLOV2_H
#define YOLOV2_H

#include <fstream>
#include <iostream>
#include "common.h"
#include "opencv2/opencv.hpp"
#include <opencv2/dnn.hpp>
#include <opencv2/dnn/all_layers.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/dnn/shape_utils.hpp>
#include <opencv2/core.hpp>
#include <opencv2/video/background_segm.hpp>
#include <vector>
#include "detect.h"


using namespace std;
using namespace cv;
using namespace dnn;
using namespace cv::dnn;


class YOLO_V2 : public Detector
{
public:
    YOLO_V2();
    YOLO_V2(const string& cfgPath, const string& modelPath);
    vector<BoundingBox> detect(const cv::Mat& img) override;

public:
    dnn::Net yolov2_net;
};

#endif // YOLOV3_H

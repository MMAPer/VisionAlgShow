#ifndef _DETECT_H
#define _DETECT_H

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
#include "common.h"

using namespace cv;
using namespace dnn;
using namespace std;
using namespace cv::dnn;
using namespace cv::dpm;


class Detector
{
public:
    virtual vector<BoundingBox> detect(const cv::Mat& img)
    {

    }
};

class FasterRCNNDetector : public Detector
{
public:
    int loadNet();
    vector<BoundingBox> detect(const cv::Mat& img) override;
private:

    String protoPath = "../../models/detection/faster_rcnn/faster_rcnn_vgg16.prototxt";
    String modelPath = "../../models/detection/faster_rcnn/VGG16_faster_rcnn_final.caffemodel";
    dnn::Net net;
};

class SSDDetector : public Detector
{
public:
    int loadNet();
    vector<BoundingBox> detect(const cv::Mat& img) override;
private:

    String protoPath = "../../models/detection/ssd/deploy.prototxt";
    String modelPath = "../../models/detection/ssd/VGG_VOC0712_SSD_300x300_iter_120000.caffemodel";
    dnn::Net net;
};

#endif

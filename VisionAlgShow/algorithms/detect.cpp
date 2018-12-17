#include "detect.h"

int FasterRCNNDetector::loadNet(){

    net = readNetFromCaffe(protoPath, modelPath);
    if(net.empty()){
        return -1;
    }else{
        return 0;
    }
}

void FasterRCNNDetector::detect(Mat &img, vector<BoundingBox> &bbox){

    float confThreshold = 0.8;
    CV_Assert(!protoPath.empty(), !modelPath.empty());
    Mat blob = blobFromImage(img, 1.0, Size(), Scalar(102.9801, 115.9465, 122.7717), false, false);
    Mat imInfo = (Mat_<float>(1, 3) << img.rows, img.cols, 1.6f);
    net.setInput(blob, "data");
    net.setInput(imInfo, "im_info");

    // Output results.
    Mat detections = net.forward();
    const float* data = (float*)detections.data;
    for (size_t i = 0; i < detections.total(); i += 7)
    {
        // An every detection is a vector [id, classId, confidence, left, top, right, bottom]
        float confidence = data[i + 2];
        if (confidence > confThreshold)
        {
            //int classId = (int)data[i + 1];
            int left = max(0, min((int)data[i + 3], img.cols - 1));
            int top = max(0, min((int)data[i + 4], img.rows - 1));
            int right = max(0, min((int)data[i + 5], img.cols - 1));
            int bottom = max(0, min((int)data[i + 6], img.rows - 1));

            // Bounding box info
            bbox[i].x = left;
            bbox[i].y = top;
            bbox[i].w = right - left;
            bbox[i].h = top - bottom;
            bbox[i].confidence = confidence;
        }
    }

}

int SSDDetector::loadNet(){

    net = readNetFromCaffe(protoPath, modelPath);
    if(net.empty()){
        return -1;
    }else{
        return 0;
    }
}

void SSDDetector::detect(Mat &img, vector<BoundingBox> &bbox){



}

int YOLODetector::loadNet()
{
    net = readNetFromDarknet(cfgPath, modelPath);
    if(net.empty())
    {
        return -1;
    }else{
        return 0;
    }
}

void YOLODetector::detect(Mat &img, vector<BoundingBox> &bbox)
{


}

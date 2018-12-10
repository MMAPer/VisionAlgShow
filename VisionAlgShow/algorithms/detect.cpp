#include "detect.h"

int FasterRCNNDetector::loadNet(){
    net = readNetFromCaffe(protoPath, modelPath);
    if(net.empty()){
        return -1;
    }else{
        return 0;
    }
}

int FasterRCNNDetector::detect(Mat &img, vector<BoundingBox> &bbox){
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

            // bounding box
            bbox[i].x = left;
            bbox[i].y = top;
            bbox[i].w = right - left;
            bbox[i].h = top - bottom;
        }
    }
    return 0;
}

int SSDDetector::loadNet(){
    net = readNetFromCaffe(protoPath, modelPath);
    if(net.empty()){
        return -1;
    }else{
        return 0;
    }
}

int SSDDetector::detect(Mat &inputImg, vector<BoundingBox> &bbox){
    float min_confidence = 0.5;
    Mat img = imread(inputImg);
    if(img.channels() == 4){
    cvtColor(img, img, COLOR_BGRA2BGR);
    }
    //! [Prepare blob]
    Mat inputBlob = blobFromImage(img, 1.0f, Size(300, 300), Scalar(104, 117, 123), false, false); //Convert Mat to batch of images
    //! [Prepare blob]
    //! [Set input blob]
    net.setInput(inputBlob, "data"); //set the network input
    //! [Set input blob]

    //! [Make forward pass]
    Mat detection = net.forward("detection_out"); //compute output
    //! [Make forward pass]
    vector<double> layersTimings;
    double freq = getTickFrequency() / 1000;
    double time = net.getPerfProfile(layersTimings) / freq;
    ostringstream ss;
    ss << "FPS: " << 1000/time << " ; time: " << time << "ms";
    putText(img, ss.str(), Point(20,20), 0, 0.5, Scalar(0,0,255));

    Mat detectionMat(detection.size[2], detection.size[3], CV_32F, detection.ptr<float>());

    float confidenceThreshold = min_confidence;
    for(int i = 0; i < detectionMat.rows; i++)
    {
        float confidence = detectionMat.at<float>(i, 2);
        if(confidence > confidenceThreshold)
        {
            size_t objectClass = (size_t)(detectionMat.at<float>(i, 1));

            int xLeftBottom = static_cast<int>(detectionMat.at<float>(i, 3) * img.cols);
            int yLeftBottom = static_cast<int>(detectionMat.at<float>(i, 4) * img.rows);
            int xRightTop = static_cast<int>(detectionMat.at<float>(i, 5) * img.cols);
            int yRightTop = static_cast<int>(detectionMat.at<float>(i, 6) * img.rows);

            ss.str("");
            ss << confidence;
            String conf(ss.str());

            Rect object(xLeftBottom, yLeftBottom,
                        xRightTop - xLeftBottom,
                        yRightTop - yLeftBottom);

            rectangle(img, object, Scalar(0, 255, 0));
            String label = String(classNames[objectClass]) + ": " + conf;
            int baseLine = 0;
            Size labelSize = getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
            rectangle(img, Rect(Point(xLeftBottom, yLeftBottom - labelSize.height),
                                  Size(labelSize.width, labelSize.height + baseLine)),
                      Scalar(255, 255, 255), FILLED);
            putText(img, label, Point(xLeftBottom, yLeftBottom),
                    FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0,0,0));
        }
    }

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

int YOLODetector::detect(Mat &inputImg, vector<BoundingBox> &bbox)
{

    //int cameraDevice = 0;
    //double fps = 3;
    float confidenceThreshold = 0.24;

    vector<String> classNamesVec;
    ifstream classNamesFile(classNames.c_str());
    if (classNamesFile.is_open())
    {
        string className = "";
        while (std::getline(classNamesFile, className))
            classNamesVec.push_back(className);
    }
    Mat yoloImage;

    if (inputImg.channels() == 4)
    cvtColor(yoloImage, yoloImage, COLOR_BGRA2BGR);

    //! [Prepare blob]
    Mat inputBlob = blobFromImage(inputImg, 1 / 255.F, Size(416, 416), Scalar(), true, false); //Convert Mat to batch of images
    //! [Prepare blob]

    //! [Set input blob]
    net.setInput(inputBlob, "data");                   //set the network input
    //! [Set input blob]

    //! [Make forward pass]
    Mat detectionMat = net.forward("detection_out");   //compute output
    //! [Make forward pass]

    vector<double> layersTimings;
    double tick_freq = getTickFrequency();
    double time_ms = net.getPerfProfile(layersTimings) / tick_freq * 1000;
    putText(yoloImage, format("FPS: %.2f ; time: %.2f ms", 1000.f / time_ms, time_ms),
           Point(20, 20), 0, 0.5, Scalar(0, 0, 255));
    for (int i = 0; i < detectionMat.rows; i++)
    {
       const int probability_index = 5;
       const int probability_size = detectionMat.cols - probability_index;
       float *prob_array_ptr = &detectionMat.at<float>(i, probability_index);

       size_t objectClass = max_element(prob_array_ptr, prob_array_ptr + probability_size) - prob_array_ptr;
       float confidence = detectionMat.at<float>(i, (int)objectClass + probability_index);

       if (confidence > confidenceThreshold)
       {
           float x_center = detectionMat.at<float>(i, 0) * yoloImage.cols;
           float y_center = detectionMat.at<float>(i, 1) * yoloImage.rows;
           float width = detectionMat.at<float>(i, 2) * yoloImage.cols;
           float height = detectionMat.at<float>(i, 3) * yoloImage.rows;
           Point p1(cvRound(x_center - width / 2), cvRound(y_center - height / 2));
           Point p2(cvRound(x_center + width / 2), cvRound(y_center + height / 2));
           Rect object(p1, p2);

           Scalar object_roi_color(0, 255, 0);

           if (object_roi_style == "box")
           {
               rectangle(yoloImage, object, object_roi_color);
           }
           else
           {
               Point p_center(cvRound(x_center), cvRound(y_center));
               line(yoloImage, object.tl(), p_center, object_roi_color, 1);
           }

           String className = objectClass < classNamesVec.size() ? classNamesVec[objectClass] : cv::format("unknown(%d)", objectClass);
           String label = format("%s: %.2f", className.c_str(), confidence);
           int baseLine = 0;
           Size labelSize = getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
           rectangle(yoloImage, Rect(p1, Size(labelSize.width, labelSize.height + baseLine)),
                     object_roi_color, FILLED);
           putText(yoloImage, label, p1 + Point(0, labelSize.height),
                   FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0,0,0));
       }
    }
}
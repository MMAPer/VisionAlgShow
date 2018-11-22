#include "mydetect.h"

int YOLODetector::loadNet()
{
    net = readNetFromDarknet(modelConfiguration, modelBinary);
    if(net.empty())
    {
        return -1;
    } else {
        return 0;
    }
}

int YOLODetector::detect(Mat &inputImg, vector<Position> &result)
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
       //cv::resize(yoloImage, yoloImage, Size(480, 290));

//       QImage yoloQImage=offline::Mat2QImage(yoloImage);
//       selectScreen(yoloQImage,screenCount);
       //ui->labelVideo4->setPixmap(QPixmap::fromImage(yoloQImage));
       //ui->labelVideo4->setAlignment(Qt::AlignCenter);

}

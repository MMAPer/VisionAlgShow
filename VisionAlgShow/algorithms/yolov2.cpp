#include "yolov2.h"
#include "time.h"


// "../../models/detection/yolo/yolov2-tiny.cfg"   "../../models/detection/yolo/yolov2-tiny.weights"
YOLO_V2::YOLO_V2(const string& cfgPath, const string& modelPath) {
    yolov2_net = readNetFromDarknet(cfgPath, modelPath);
    if(yolov2_net.empty())
    {
        cout << "yolo_v2 network load error!" << endl;
    }
    else
    {
        cout << "yolo_v2 network load success!" << endl;
    }
}


String classNames ="../../models/detection/yolo/coco.names";
String object_roi_style = "box";   // box or line style draw
float confidenceThreshold = 0.24;

vector<BoundingBox> YOLO_V2::Detect_yolov2(const Mat &img)
{
    vector<BoundingBox> boxes;
    vector<String> classNamesVec;
    ifstream classNamesFile(classNames.c_str());
    if (classNamesFile.is_open())
    {
        string className = "";
        while (std::getline(classNamesFile, className))
            classNamesVec.push_back(className);
    }

    if (img.channels() == 4)
        cvtColor(img, img, COLOR_BGRA2BGR);

    //! [Prepare blob]
    Mat inputBlob = blobFromImage(img, 1 / 255.F, Size(416, 416), Scalar(), true, false); //Convert Mat to batch of images
    //! [Prepare blob]

    //! [Set input blob]
    yolov2_net.setInput(inputBlob, "data");                   //set the network input
    //! [Set input blob]

    //! [Make forward pass]
    Mat detectionMat = yolov2_net.forward("detection_out");   //compute output
    //! [Make forward pass]


     vector<double> layersTimings;
    double tick_freq = getTickFrequency();
    double time_ms = yolov2_net.getPerfProfile(layersTimings) / tick_freq * 1000;
    std::cout << "speed:" << 1000.f / time_ms << std::endl;


    //float confidenceThreshold = parser.get<float>("min_confidence");
    for (int i = 0; i < detectionMat.rows; i++)
    {
        const int probability_index = 5;
        const int probability_size = detectionMat.cols - probability_index;
        float *prob_array_ptr = &detectionMat.at<float>(i, probability_index);

        size_t objectClass = max_element(prob_array_ptr, prob_array_ptr + probability_size) - prob_array_ptr;
        float confidence = detectionMat.at<float>(i, (int)objectClass + probability_index);

        if (confidence > confidenceThreshold)
        {
            float x_center = detectionMat.at<float>(i, 0) * img.cols;
            float y_center = detectionMat.at<float>(i, 1) * img.rows;
            float width = detectionMat.at<float>(i, 2) * img.cols;
            float height = detectionMat.at<float>(i, 3) * img.rows;
//            Point p1(cvRound(x_center - width / 2), cvRound(y_center - height / 2));
//            Point p2(cvRound(x_center + width / 2), cvRound(y_center + height / 2));

            BoundingBox box;
            box.x = x_center - width / 2;
            box.y = y_center - height / 2;
            box.w = width;
            box.h = height;
            box.confidence = confidence;

            boxes.push_back(box);
        }
    }
    return boxes;
}




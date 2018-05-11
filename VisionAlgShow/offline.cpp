#include "offline.h"
#include "ui_offline.h"
#include "iconhelper.h"
#include "myapp.h"
#include "QMenu"
#include "QAction"
#include "QString"
#include <QDesktopWidget>
#include "QFileDialog"
#include "QDebug"
#include "QMessageBox"
#include "QStandardItem"
#include "opencv2/opencv.hpp"
#include <opencv2/dnn.hpp>
#include <opencv2/dnn/all_layers.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/dnn/shape_utils.hpp>
#include <opencv2/dpm.hpp>
#include <opencv2/core.hpp>
#include <opencv2/video/background_segm.hpp>
#include <iostream>
using namespace cv;
using namespace dnn;
using namespace std;
using namespace cv::dnn;
using namespace cv::dpm;

void addCboxItem(QComboBox *target, vector<string> items);

offline::offline(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::offline)
{

    ui->setupUi(this);
    this->InitStyle();  //初始化样式
    this->InitEvent();  //事件信号与槽绑定

}

offline::~offline()
{
    delete ui;
}

void offline::InitStyle()
{

    this->setStyleSheet("QGroupBox#gboxMain{border-width:0px;}");
    this->setProperty("Form", true);
    this->setWindowFlags(Qt::FramelessWindowHint |
                         Qt::WindowSystemMenuHint |
                         Qt::WindowMinMaxButtonsHint);

    IconHelper::Instance()->SetIcon(ui->label_ico, QChar(0xf03d), 11);

    ui->label_title->setText("本地视频文件处理");
    this->setWindowTitle("本地视频文件处理");

    //背景建模
    cbox_bg = ui->cbox_bg;
    string str_bg[] = {"", "KNN", "MOG2"};
    vector<string> alg_bg(str_bg, str_bg+3);
    addCboxItem(cbox_bg, alg_bg);

    //目标检测
    cbox_od = ui->cbox_od;
    string str_od[] = {"", "HOG+SVM", "DPM", "Faster R-CNN", "YOLO", "SSD"};
    vector<string> alg_od(str_od, str_od+6);
    addCboxItem(cbox_od, alg_od);

    //ui->label_play->setStyleSheet("border: 1px solid #000000;");

}

void addCboxItem(QComboBox *target, vector<string> items)
{
    for(int i=0; i<items.size(); i++)
    {
        target->addItem(QString::fromStdString(items[i]));
    }
}

void offline::InitEvent()
{
    connect(cbox_bg, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(bgModel()));
    connect(cbox_od, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(on_btn_ssd_clicked()));
}

void offline::receiveofflinehandle(){
    this->show();
}

void offline::on_btn_back_clicked()
{
    this->hide();
    emit backmain();

}

//open file
void offline::on_btn_open_clicked()
{


    if (capture.isOpened()){
        capture.release();     //decide if capture is already opened; if so,close it
        fileName = "";
    }
        fileName =QFileDialog::getOpenFileName(NULL,tr("选择文件"),".",tr("Image or Video Files(*.jpg *.png *.JPEG *.avi *.mp4 *.flv *.mkv)"));
        ui->text_dir->setText(fileName);
        QFileInfo fileinfo=QFileInfo(fileName);
        fileSuffix=fileinfo.suffix();
        //ui->text_dir->setText(fileSuffix);
        if(fileSuffix == "jpg" || fileSuffix == "png" || fileSuffix == "JPEG" ){
            videoFlag = 0;
            //static const int kInpWidth = 960;
            //static const int kInpHeight = 576;
            cv::Mat image = cv::imread(fileName.toLatin1().data());
           // cv::resize(image, image, Size(kInpWidth, kInpHeight));
            QImage img = offline::Mat2QImage(image);
            ui->label_play->setPixmap(QPixmap::fromImage(img));
            ui->label_play->setAlignment(Qt::AlignCenter);
            //ui->label_play->setAlignment(Qt::AlignVCenter);

        }
        else{
            videoFlag = 1;
            capture.open(fileName.toLocal8Bit().data());
            if (capture.isOpened())
            {
                rate= capture.get(CV_CAP_PROP_FPS);
                capture >> frame;
                if (!frame.empty())
                {
                    image = Mat2QImage(frame);
                    ui->label_play->setPixmap(QPixmap::fromImage(image));
                    ui->label_play->setAlignment(Qt::AlignCenter);
                    timer = new QTimer(this);
                    timer->setInterval(1000/rate);   //set timer match with FPS
                    connect(timer, SIGNAL(timeout()), this, SLOT(playbyframe()));
                    timer->start();


                }
            }
        }

}

//auto play by frame
void offline::playbyframe()
{
    capture >> frame;
        if (!frame.empty())
        {
            image = Mat2QImage(frame);
            ui->label_play->setPixmap(QPixmap::fromImage(image));
            //this->update();
        }
}

// opencv Mat to Image
QImage offline::Mat2QImage(cv::Mat cvImg)
{
    QImage qImg;
    if(cvImg.channels()==3)                             //3 channels color image
    {

        cv::cvtColor(cvImg,cvImg,CV_BGR2RGB);
        qImg =QImage((const unsigned char*)(cvImg.data),
                    cvImg.cols, cvImg.rows,
                    cvImg.cols*cvImg.channels(),
                    QImage::Format_RGB888);
    }
    else if(cvImg.channels()==1)                    //grayscale image
    {
        qImg =QImage((const unsigned char*)(cvImg.data),
                    cvImg.cols,cvImg.rows,
                    cvImg.cols*cvImg.channels(),
                    QImage::Format_Indexed8);
    }
    else
    {
        qImg =QImage((const unsigned char*)(cvImg.data),
                    cvImg.cols,cvImg.rows,
                    cvImg.cols*cvImg.channels(),
                    QImage::Format_RGB888);
    }

    return qImg;

}

void offline::bgModel()
{

    if (videoFlag==1){
        timer->stop();
        Mat fgmask;
        while(capture.read(frame)){
        if (frame.empty())
        {
            waitKey();
            break;
        }

        if (frame.channels() == 4)
            cvtColor(frame, frame, COLOR_BGRA2BGR);


        Ptr<BackgroundSubtractor> bg_model = createBackgroundSubtractorMOG2();
        bg_model->apply(frame, fgmask);

        image=offline::Mat2QImage(fgmask);
        ui->label_play->setPixmap(QPixmap::fromImage(image));
        ui->label_play->setAlignment(Qt::AlignCenter);
        if (waitKey(1) >= 0) break;
        }
    }
}

void offline::on_btn_faster_rcnn_clicked()
{
    const char* classNames[] = {
        "__background__",
        "aeroplane", "bicycle", "bird", "boat",
        "bottle", "bus", "car", "cat", "chair",
        "cow", "diningtable", "dog", "horse",
        "motorbike", "person", "pottedplant",
        "sheep", "sofa", "train", "tvmonitor"
    };



    if(fileName == NULL)
        QMessageBox::information(this, QString::fromLocal8Bit("警告"),QString::fromLocal8Bit("当前路径为空,请打开一张图片"));
    else if(videoFlag == 1){
        QMessageBox::information(this, QString::fromLocal8Bit("警告"),QString::fromLocal8Bit("当前路径为视频文件,请打开一张图片"));
    }
    else{
        String protoPath = "../../models/detection/faster_rcnn/faster_rcnn_vgg16.prototxt";
        String modelPath = "../../models/detection/faster_rcnn/VGG16_faster_rcnn_final.caffemodel";
        String imagePath = fileName.toStdString();
        float confThreshold = 0.8;
        CV_Assert(!protoPath.empty(), !modelPath.empty(), !imagePath.empty());

        // Load a model.
        Net net = readNetFromCaffe(protoPath, modelPath);

        Mat img = imread(imagePath);
        Mat blob = blobFromImage(img, 1.0, Size(), Scalar(102.9801, 115.9465, 122.7717), false, false);
        Mat imInfo = (Mat_<float>(1, 3) << img.rows, img.cols, 1.6f);

        net.setInput(blob, "data");
        net.setInput(imInfo, "im_info");

        // Draw detections.
        Mat detections = net.forward();
        const float* data = (float*)detections.data;
        for (size_t i = 0; i < detections.total(); i += 7)
        {
            // An every detection is a vector [id, classId, confidence, left, top, right, bottom]
            float confidence = data[i + 2];
            if (confidence > confThreshold)
            {
                int classId = (int)data[i + 1];
                int left = max(0, min((int)data[i + 3], img.cols - 1));
                int top = max(0, min((int)data[i + 4], img.rows - 1));
                int right = max(0, min((int)data[i + 5], img.cols - 1));
                int bottom = max(0, min((int)data[i + 6], img.rows - 1));

                // Draw a bounding box.
                rectangle(img, Point(left, top), Point(right, bottom), Scalar(0, 255, 0));

                // Put a label with a class name and confidence.
                String label = cv::format("%s, %.3f", classNames[classId], confidence);
                int baseLine;
                Size labelSize = cv::getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);

                top = max(top, labelSize.height);
                rectangle(img, Point(left, top - labelSize.height),
                          Point(left + labelSize.width, top + baseLine),
                          Scalar(255, 255, 255), FILLED);
                putText(img, label, Point(left, top), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0));
            }
        }
        image=offline::Mat2QImage(img);
        ui->label_play->setPixmap(QPixmap::fromImage(image));
        ui->label_play->setAlignment(Qt::AlignCenter);

    }
}

void offline::on_btn_ssd_clicked()
{

    const char* classNames[] = {"background",
                                "aeroplane", "bicycle", "bird", "boat",
                                "bottle", "bus", "car", "cat", "chair",
                                "cow", "diningtable", "dog", "horse",
                                "motorbike", "person", "pottedplant",
                                "sheep", "sofa", "train", "tvmonitor"};



    String modelConfiguration = "../../models/detection/ssd/deploy.prototxt";
    String modelBinary = "../../models/detection/ssd/VGG_VOC0712_SSD_300x300_iter_120000.caffemodel";
    //String filePath = "../../videos/original.mp4";
    String filePath = fileName.toStdString();
    //int cameraDevice = 0;
    float min_confidence = 0.5;
    //! [Initialize network]
    dnn::Net net = readNetFromCaffe(modelConfiguration, modelBinary);
    //! [Initialize network]

    if (net.empty())
    {
        QMessageBox::information(this,QString::fromLocal8Bit("警告"),QString::fromLocal8Bit("无法加载配置文件"));
    }


    if (videoFlag==1){
        timer->stop();
        for(;;){
        capture>>frame;
        if (frame.empty())
        {
            waitKey();
            break;
        }

        if (frame.channels() == 4)
            cvtColor(frame, frame, COLOR_BGRA2BGR);


        //! [Prepare blob]
        Mat inputBlob = blobFromImage(frame, 1.0f, Size(300, 300), Scalar(104, 117, 123), false, false); //Convert Mat to batch of images
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
        ss << "FPS: " << 1000/time << " ; time: " << time << " ms";
        putText(frame, ss.str(), Point(20,20), 0, 0.5, Scalar(0,0,255));

        Mat detectionMat(detection.size[2], detection.size[3], CV_32F, detection.ptr<float>());

        float confidenceThreshold = min_confidence;
        for(int i = 0; i < detectionMat.rows; i++)
        {
            float confidence = detectionMat.at<float>(i, 2);

            if(confidence > confidenceThreshold)
            {
                size_t objectClass = (size_t)(detectionMat.at<float>(i, 1));

                int xLeftBottom = static_cast<int>(detectionMat.at<float>(i, 3) * frame.cols);
                int yLeftBottom = static_cast<int>(detectionMat.at<float>(i, 4) * frame.rows);
                int xRightTop = static_cast<int>(detectionMat.at<float>(i, 5) * frame.cols);
                int yRightTop = static_cast<int>(detectionMat.at<float>(i, 6) * frame.rows);

                ss.str("");
                ss << confidence;
                String conf(ss.str());

                Rect object(xLeftBottom, yLeftBottom,
                            xRightTop - xLeftBottom,
                            yRightTop - yLeftBottom);

                rectangle(frame, object, Scalar(0, 255, 0));
                String label = String(classNames[objectClass]) + ": " + conf;
                int baseLine = 0;
                Size labelSize = getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
                rectangle(frame, Rect(Point(xLeftBottom, yLeftBottom - labelSize.height),
                                      Size(labelSize.width, labelSize.height + baseLine)),
                          Scalar(255, 255, 255), FILLED);
                putText(frame, label, Point(xLeftBottom, yLeftBottom),
                        FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0,0,0));
            }
        }

        //imshow("detections", frame);
        //if (waitKey(1) >= 0) break;
        //cv::resize(frame, frame, Size(kInpWidth, kInpHeight));
        image=offline::Mat2QImage(frame);
        ui->label_play->setPixmap(QPixmap::fromImage(image));
        ui->label_play->setAlignment(Qt::AlignCenter);
        if (waitKey(1) >= 0) break;

        }

    }
    else {
        Mat ssd_frame;
        ssd_frame = imread(filePath);

        if(!ssd_frame.data)
            {
                QMessageBox::information(this,QString::fromLocal8Bit("警告"),QString::fromLocal8Bit("当前路径为空，请打开一张图片或一段视频"));
                //QMessageBox msgBox;
                //msgBox.setText(tr("Image Data Is Null"));
                //msgBox.exec();
            }
            else {
            if (ssd_frame.channels() == 4)
                cvtColor(ssd_frame, ssd_frame, COLOR_BGRA2BGR);


            //! [Prepare blob]
            Mat inputBlob = blobFromImage(ssd_frame, 1.0f, Size(300, 300), Scalar(104, 117, 123), false, false); //Convert Mat to batch of images
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
            ss << "FPS: " << 1000/time << " ; time: " << time << " ms";
            putText(ssd_frame, ss.str(), Point(20,20), 0, 0.5, Scalar(0,0,255));

            Mat detectionMat(detection.size[2], detection.size[3], CV_32F, detection.ptr<float>());

            float confidenceThreshold = min_confidence;
            for(int i = 0; i < detectionMat.rows; i++)
            {
                float confidence = detectionMat.at<float>(i, 2);

                if(confidence > confidenceThreshold)
                {
                    size_t objectClass = (size_t)(detectionMat.at<float>(i, 1));

                    int xLeftBottom = static_cast<int>(detectionMat.at<float>(i, 3) * ssd_frame.cols);
                    int yLeftBottom = static_cast<int>(detectionMat.at<float>(i, 4) * ssd_frame.rows);
                    int xRightTop = static_cast<int>(detectionMat.at<float>(i, 5) * ssd_frame.cols);
                    int yRightTop = static_cast<int>(detectionMat.at<float>(i, 6) * ssd_frame.rows);

                    ss.str("");
                    ss << confidence;
                    String conf(ss.str());

                    Rect object(xLeftBottom, yLeftBottom,
                                xRightTop - xLeftBottom,
                                yRightTop - yLeftBottom);

                    rectangle(ssd_frame, object, Scalar(0, 255, 0));
                    String label = String(classNames[objectClass]) + ": " + conf;
                    int baseLine = 0;
                    Size labelSize = getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
                    rectangle(ssd_frame, Rect(Point(xLeftBottom, yLeftBottom - labelSize.height),
                                          Size(labelSize.width, labelSize.height + baseLine)),
                              Scalar(255, 255, 255), FILLED);
                    putText(ssd_frame, label, Point(xLeftBottom, yLeftBottom),
                            FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0,0,0));
                }
            }
            QImage ssd_image;
            ssd_image=offline::Mat2QImage(ssd_frame);
            ui->label_play->setPixmap(QPixmap::fromImage(ssd_image));
            ui->label_play->setAlignment(Qt::AlignCenter);

            }
    }

}

void offline::on_btn_hog_clicked()
{
    String file = fileName.toStdString();
    cv::Mat image = cv::imread(file);
    if (image.empty())
    {
        QMessageBox::information(this, QString::fromLocal8Bit("警告"),QString::fromLocal8Bit("当前路径为空,请打开一张图片"));
    }
    else{
    //【1】定义hog描述符
    cv::HOGDescriptor hog;
    //【2】设置SVM分类器
    hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
    //【3】在测试图像上检测行人区域
    std::vector<cv::Rect> regions;
    hog.detectMultiScale(image, regions, 0, cv::Size(8, 8), cv::Size(32, 32), 1.05, 1);

    for (size_t i = 0; i < regions.size(); i++)
    {
        cv::rectangle(image, regions[i], cv::Scalar(0, 0, 255), 2);
    }
    //cv::imshow("行人检测", image);
    QImage img=offline::Mat2QImage(image);
    ui->label_play->setPixmap(QPixmap::fromImage(img));
    ui->label_play->setAlignment(Qt::AlignCenter);

    }

}

void offline::on_btn_yolo_clicked()
{
    String modelConfiguration = "/home/hbc/dev/projects/VisionAlgShow/detection/yolo/yolov2.cfg";
    String modelBinary = "/home/hbc/dev/projects/VisionAlgShow/detection/yolo/yolov2.weights";
    String class_names ="/home/hbc/dev/projects/VisionAlgShow/detection/yolo/coco.names";
    String source = fileName.toStdString();
    String  out = "/home/hbc/dev/projects/VisionAlgShow/images/output.jpg";
    String object_roi_style = "box";   // box or line style draw
    int cameraDevice = 0;
    double fps = 3;
    float confidenceThreshold = 0.24;

    //! [Initialize network]
    dnn::Net net = readNetFromDarknet(modelConfiguration, modelBinary);
    //! [Initialize network]

    if (net.empty())
    {
        QMessageBox::information(this, QString::fromLocal8Bit("警告"),QString::fromLocal8Bit("无法加载模型及配置文件"));

    }

    VideoCapture cap;
    VideoWriter writer;
    int codec = CV_FOURCC('M', 'J', 'P', 'G');

    if (source.empty())
    {

        cap = VideoCapture(cameraDevice);
        if(!cap.isOpened())
        {
           QMessageBox::information(this, QString::fromLocal8Bit("警告"),QString::fromLocal8Bit("无法打开camera 0"));
           //QMessageBox::information(this, QString::fromLocal8Bit("错误")),QString::fromLocal8Bit("无法打开camera 0"));
        }
    }
    else
    {

        cap.open(source);
        if(!cap.isOpened())
        {
           QMessageBox::information(this, QString::fromLocal8Bit("警告"),QString::fromLocal8Bit("无法打开当前路径文件"));
           //QMessageBox::information(this,QString::fromLocal8Bit("错误")),QString::fromLocal8Bit("无法打开当前路径文件"));
    }

    if(out.empty())
    {
        writer.open(out, codec, fps, Size((int)cap.get(CAP_PROP_FRAME_WIDTH),(int)cap.get(CAP_PROP_FRAME_HEIGHT)), 1);
    }

    vector<String> classNamesVec;

    ifstream classNamesFile(class_names.c_str());
    if (classNamesFile.is_open())
    {
        string className = "";
        while (std::getline(classNamesFile, className))
            classNamesVec.push_back(className);
    }



    for(;;)
    {
        Mat frame;
        cap >> frame; // get a new frame from camera/video or read image

        if (frame.empty())
        {
            waitKey();
            break;
        }

        if (frame.channels() == 4)
            cvtColor(frame, frame, COLOR_BGRA2BGR);

        //! [Prepare blob]
        Mat inputBlob = blobFromImage(frame, 1 / 255.F, Size(416, 416), Scalar(), true, false); //Convert Mat to batch of images
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
        putText(frame, format("FPS: %.2f ; time: %.2f ms", 1000.f / time_ms, time_ms),
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
                float x_center = detectionMat.at<float>(i, 0) * frame.cols;
                float y_center = detectionMat.at<float>(i, 1) * frame.rows;
                float width = detectionMat.at<float>(i, 2) * frame.cols;
                float height = detectionMat.at<float>(i, 3) * frame.rows;
                Point p1(cvRound(x_center - width / 2), cvRound(y_center - height / 2));
                Point p2(cvRound(x_center + width / 2), cvRound(y_center + height / 2));
                Rect object(p1, p2);

                Scalar object_roi_color(0, 255, 0);

                if (object_roi_style == "box")
                {
                    rectangle(frame, object, object_roi_color);
                }
                else
                {
                    Point p_center(cvRound(x_center), cvRound(y_center));
                    line(frame, object.tl(), p_center, object_roi_color, 1);
                }

                String className = objectClass < classNamesVec.size() ? classNamesVec[objectClass] : cv::format("unknown(%d)", objectClass);
                String label = format("%s: %.2f", className.c_str(), confidence);
                int baseLine = 0;
                Size labelSize = getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
                rectangle(frame, Rect(p1, Size(labelSize.width, labelSize.height + baseLine)),
                          object_roi_color, FILLED);
                putText(frame, label, p1 + Point(0, labelSize.height),
                        FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0,0,0));
            }
        }
        if(writer.isOpened())
        {
            writer.write(frame);
        }

        QImage img=offline::Mat2QImage(frame);
        ui->label_play->setPixmap(QPixmap::fromImage(img));
        ui->label_play->setAlignment(Qt::AlignCenter);
    }

}
}

void offline::on_btn_dpm_clicked()
{
    String dpm_model_path = "../../models/detection/dpm/inriaperson.xml";
    String dpm_image_dir = fileName.toStdString();

    if (dpm_model_path.empty() || dpm_image_dir.empty()){
        QMessageBox::information(this,QString::fromLocal8Bit("警告"),QString::fromLocal8Bit("无法加载配置文件"));
    }
    cv::Ptr<DPMDetector> detector = DPMDetector::create(vector<string>(1, dpm_model_path));

    Scalar color(0, 255, 255); // yellow
    Mat frame;

    Mat dpm_image = imread(dpm_image_dir);
    vector<DPMDetector::ObjectDetection> ds;

    frame = dpm_image.clone();
    if(dpm_image.empty()){
        QMessageBox::information(this,QString::fromLocal8Bit("警告"),QString::fromLocal8Bit("当前路径为空，请打开一张图片"));
    }
    //detection
    detector->detect(dpm_image,ds);
    // compute frame per second (fps)
    //double t = ((double) getTickCount() - t)/getTickFrequency();//elapsed time
    //draw boxes
    for (unsigned int i = 0; i < ds.size(); i++)
        {
            rectangle(frame, ds[i].rect, color, 2);
        }

    // draw text on image
    //String text =format("%0.1f fps",1.0/t);
    //Scalar textColor(0,0,250);
    //putText(frame, text, Point(10,50), FONT_HERSHEY_PLAIN, 2, textColor, 2);

    QImage img=offline::Mat2QImage(frame);
    ui->label_play->setPixmap(QPixmap::fromImage(img));
    ui->label_play->setAlignment(Qt::AlignCenter);
}





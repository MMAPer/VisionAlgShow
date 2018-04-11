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
#include <iostream>
using namespace cv;
using namespace dnn;
using namespace std;
using namespace cv::dnn;

offline::offline(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::offline)
{

    ui->setupUi(this);
    this->InitStyle();  //初始化样式

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

    ui->label_title->setText("离线处理");
    this->setWindowTitle("离线处理");
    ui->widge_title->setStyleSheet("background-color:#78b4e3;");
    //ui->label_play->setStyleSheet("border: 1px solid #000000;");

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


    if (capture.isOpened())
        capture.release();     //decide if capture is already opened; if so,close it
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
            videoFlag=1;
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
        String protoPath = "../../detection/faster_rcnn/faster_rcnn_vgg16.prototxt";
        String modelPath = "../../detection/faster_rcnn/VGG16_faster_rcnn_final.caffemodel";
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
       // ui->label_play->setAlignment(Qt::AlignHCenter);
        //ui->label_play->setAlignment(Qt::AlignVCenter);

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



    String modelConfiguration = "../../detection/ssd/deploy.prototxt";
    String modelBinary = "../../detection/ssd/VGG_VOC0712_SSD_300x300_iter_120000.caffemodel";
    //String filePath = "../../videos/1026.mp4";
    String filePath = fileName.toStdString();
    int cameraDevice = 0;
    float min_confidence = 0.5;
    //! [Initialize network]
    dnn::Net net = readNetFromCaffe(modelConfiguration, modelBinary);
    //! [Initialize network]

    if (net.empty())
    {
        cerr << "Can't load network by using the following files: " << endl;
        cerr << "prototxt:   " << modelConfiguration << endl;
        cerr << "caffemodel: " << modelBinary << endl;
        cerr << "Models can be downloaded here:" << endl;
        cerr << "https://github.com/weiliu89/caffe/tree/ssd#models" << endl;
        exit(-1);
    }

    VideoCapture cap;
    if (filePath.empty())
    {
        //int cameraDevice = parser.get<int>("camera_device");
        cap = VideoCapture(cameraDevice);
        if(!cap.isOpened())
        {
            cout << "Couldn't find camera: " << cameraDevice << endl;
            //return -1;
        }
    }
    else
    {
        cap.open(filePath);
        if(!cap.isOpened())
        {
            cout << "Couldn't open image or video: " << filePath << endl;
            //return -1;
        }
    }

    for (;;)
    {
        cv::Mat frame;
        cap >> frame; // get a new frame from camera/video or read image
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
        //cv::resize(frame, frame, Size(kInpWidth, kInpHeight));
        image=offline::Mat2QImage(frame);
        ui->label_play->setPixmap(QPixmap::fromImage(image));
        ui->label_play->setAlignment(Qt::AlignCenter);

    }

}

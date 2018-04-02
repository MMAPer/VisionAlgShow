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
using namespace cv;
using namespace dnn;
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
    ui->label_play->setStyleSheet("border: 1px solid #000000;");

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
        QString filename =QFileDialog::getOpenFileName(this,tr("Open Video File"),".",tr("Video Files(*.avi *.mp4 *.flv *.mkv)"));
        capture.open(filename.toLocal8Bit().data());
        if (capture.isOpened())
        {
            rate= capture.get(CV_CAP_PROP_FPS);
            capture >> frame;
            if (!frame.empty())
            {

                image = Mat2QImage(frame);
                ui->label_play->setPixmap(QPixmap::fromImage(image));
                timer = new QTimer(this);
                timer->setInterval(1000/rate);   //set timer match with FPS
                connect(timer, SIGNAL(timeout()), this, SLOT(playbyframe()));
                timer->start();
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

    static const int kInpWidth = 960;
    static const int kInpHeight = 576;

        String protoPath = "../../detection/faster_rcnn/faster_rcnn_vgg16.prototxt";
        String modelPath = "../../detection/faster_rcnn/VGG16_faster_rcnn_final.caffemodel";
        String imagePath = "../../detection/faster_rcnn/test.jpg";
        float confThreshold = 0.8;
        CV_Assert(!protoPath.empty(), !modelPath.empty(), !imagePath.empty());

        // Load a model.
        Net net = readNetFromCaffe(protoPath, modelPath);

        Mat img = imread(imagePath);
        cv::resize(img, img, Size(kInpWidth, kInpHeight));
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
}

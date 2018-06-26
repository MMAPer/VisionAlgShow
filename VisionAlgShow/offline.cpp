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
//#include <opencv2/cudacodec.hpp>
#include <iostream>
#include <time.h>
//#include<opencv2/cudaimgproc.hpp>
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
    this->InitOfflineVideo();

}

offline::~offline()
{
    delete ui;
}

void offline::InitStyle()
{

    this->setStyleSheet("QGroupBox#gboxMain{border-width:0px;}");
    this->setProperty("Form", true);
//  this->setWindowFlags(Qt::FramelessWindowHint |
//                         Qt::WindowSystemMenuHint |
//                         Qt::WindowMinMaxButtonsHint);
    this->setWindowFlags(Qt::Widget);

    //IconHelper::Instance()->SetIcon(ui->label_ico, QChar(0xf03d), 11);
    //ui->label_title->setText("本地视频文件处理");
    this->setWindowTitle("本地视频文件处理");

    //背景建模
    cbox_bg = ui->cbox_bg;
    string str_bg[] = {"背景建模", "KNN", "MOG2"};
    vector<string> alg_bg(str_bg, str_bg+3);
    addCboxItem(cbox_bg, alg_bg);

    //目标检测
    cbox_od = ui->cbox_od;
    string str_od[] = {"目标检测", "HOG+SVM", "DPM", "Faster R-CNN", "YOLO", "SSD"};
    vector<string> alg_od(str_od, str_od+6);
    addCboxItem(cbox_od, alg_od);

    //目标跟踪
    cbox_sot = ui->cbox_sot;
    string str_sot[] = {"目标跟踪"};
    vector<string> alg_sot(str_sot, str_sot+1);
    addCboxItem(cbox_sot, alg_sot);

    //衣服解析
    cbox_sm = ui->cbox_sm;
    string str_sm[] = {"衣服解析"};
    vector<string> alg_sm(str_sm, str_sm+1);
    addCboxItem(cbox_sm, alg_sm);

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
    connect(cbox_bg, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(bgSubtraction()));
    connect(cbox_od, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(od_alg_clicked(const  QString &)));

    //connect(this,SIGNAL(od_alg_clicked(const QString od_alg)),this,SLOT(od_alg_hog()));
    //connect(this,SIGNAL(od_alg_clicked(const QString od_alg)),this,SLOT(od_alg_dpm()));
    //connect(this,SIGNAL(od_alg_clicked(const QString od_alg)),this,SLOT(od_alg_faster_rcnn()));
    //connect(this,SIGNAL(od_alg_clicked(const QString od_alg)),this,SLOT(od_alg_ssd()));
    //connect(this,SIGNAL(od_alg_clicked(const QString od_alg)),this,SLOT(od_alg_yolo()));
}

//init the offline play lab
void offline::InitOfflineVideo()
{
    offlineTempLabel = 0;
    video_max = false;

    offlineVideoLabel.append(ui->labelVideo1);
    offlineVideoLabel.append(ui->labelVideo2);
    offlineVideoLabel.append(ui->labelVideo3);
    offlineVideoLabel.append(ui->labelVideo4);

    offlineVideoLayout.append(ui->layoutVideo1);
    offlineVideoLayout.append(ui->layoutVideo2);

    for (int i = 0; i < 4; i++) {
        offlineVideoLabel[i]->installEventFilter(this);
        offlineVideoLabel[i]->setProperty("labVideo", true);
        offlineVideoLabel[i]->setText(QString("屏幕%1").arg(i + 1));
    }

    offlineMenu = new QMenu(this);
    offlineMenu->setStyleSheet("font: 10pt \"微软雅黑\";");
    offlineMenu->addAction("切换到1画面", this, SLOT(show_video_1()));
    offlineMenu->addAction("切换到4画面", this, SLOT(show_video_4()));
    offlineTempLabel = offlineVideoLabel[0];
    show_video_4();
}

void offline::removeLayout()
{
    for(int i=0; i<2; i++)
    {
        offlineVideoLayout[0]->removeWidget(offlineVideoLabel[i]);
        offlineVideoLabel[i]->setVisible(false);
    }

    for(int i=2; i<4; i++)
    {
        offlineVideoLayout[0]->removeWidget(offlineVideoLabel[i]);
        offlineVideoLabel[i]->setVisible(false);
    }
}
void offline::show_video_1()
{
    removeLayout();
    myApp::VideoType="1";
    video_max= true;
    change_video_1();
    offlineWindowNum=1;
}

void offline::change_video_1(int index)
{
    for (int i = (index + 0); i < (index + 1) ; i++)
    {
        offlineVideoLayout[0]->addWidget(offlineVideoLabel[i]);
        offlineVideoLabel[i]->setVisible(true);
    }
}

void offline::show_video_4()
{
    removeLayout();
    myApp::VideoType="4";
    change_video_4();
    offlineWindowNum=4;
}

void offline::change_video_4(int index)
{
    for (int i = (index + 0); i < (index + 2); i++) {
        offlineVideoLayout[0]->addWidget(offlineVideoLabel[i]);
        //offlineVideoLabel[i]->resize(QSize(480,290));
        offlineVideoLabel[i]->setVisible(true);
    }

    for (int i = (index + 2); i < (index + 4); i++) {
        offlineVideoLayout[1]->addWidget(offlineVideoLabel[i]);
        offlineVideoLabel[i]->setVisible(true);
    }
}

//通过注册事件监听器绑定事件
bool offline::eventFilter(QObject *obj, QEvent *event)
{
    QMouseEvent *MouseEvent = static_cast<QMouseEvent *>(event);
    if((event->type()) == QEvent::MouseButtonPress)
    {
       if(MouseEvent->buttons() == Qt::RightButton)
        {
            offlineTempLabel = qobject_cast<QLabel *>(obj);
            offlineMenu->exec(QCursor::pos());
            return true;
        }
    }
    return QObject::eventFilter(obj, event);
}

void offline::od_alg_clicked(const QString od_alg)
{

    if(od_alg=="HOG+SVM")
    {
        emit offline::od_alg_hog();
    }
    else if(od_alg=="DPM")
    {
        emit offline::od_alg_dpm();
    }
    else if(od_alg=="Faster R-CNN")
    {
        emit offline::od_alg_faster_rcnn();
    }
    else if(od_alg=="SSD")
    {
        emit offline::od_alg_ssd();
    }
    else if(od_alg=="YOLO")
    {
        //QMessageBox::information(this, QString::fromLocal8Bit("警告"),QString::fromLocal8Bit("yolo"));
        emit offline::od_alg_yolo();
    }
}

//open file
void offline::on_btn_open_clicked()
{
    if (screenCount >= 4)
        QMessageBox::information(this, QString::fromLocal8Bit("警告"),QString::fromLocal8Bit("当前通道已满！"));
    else
    {
        if (capture.isOpened())
        {
            capture.release();     //decide if capture is already opened; if so,close it
            //filePath = "";
        }
            filePath =QFileDialog::getOpenFileName(NULL,tr("选择文件"),".",tr("Image or Video Files(*.jpg *.png *.JPEG *.avi *.mp4 *.flv *.mkv)"));

            ui->text_dir->setText(filePath);


            QFileInfo fileInfo=QFileInfo(filePath);
            fileSuffix=fileInfo.suffix();
            //ui->text_dir->setText(fileSuffix);
            if(fileSuffix == "jpg" || fileSuffix == "png" || fileSuffix == "JPEG" )
            {
                videoFlag = 0;
                //static const int kInpWidth = 960;
                //static const int kInpHeight = 576;
                cv::Mat image = cv::imread(filePath.toStdString());
                //cv::resize(image, image, Size(480, 290));
                QImage img = offline::Mat2QImage(image);
                selectScreen(img,screenCount);
                screenCount++;
                //ui->labelVideo1->setPixmap(QPixmap::fromImage(img));
                //ui->labelVideo1->setAlignment(Qt::AlignCenter);
                //ui->label_play->setAlignment(Qt::AlignVCenter);

            }
            else
            {
                videoFlag = 1;
                capture.open(filePath.toStdString());
                //GPU
                //g_reader = cv::cudacodec::createVideoReader(filePath.toStdString());
                //g_reader->nextFrame(g_frame);
                //rate = 30;
                if (capture.isOpened())
                {
                    rate= capture.get(CV_CAP_PROP_FPS);
                    capture >> frame;
                    if (!frame.empty())
                    {
                        image = Mat2QImage(frame);
                        selectScreen(image,screenCount);
                        //ui->labelVideo1->setPixmap(QPixmap::fromImage(image));
                        //ui->labelVideo1->setAlignment(Qt::AlignCenter);
                        timer = new QTimer(this);
                        timer->setInterval(1000/rate);   //set timer match with FPS
                        connect(timer, SIGNAL(timeout()), this, SLOT(playbyframe()));
                        timer->start();
                    }
                    screenCount++;
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
            selectScreen(image,screenCount);
            //ui->labelVideo1->setPixmap(QPixmap::fromImage(image));
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

void offline::selectScreen(QImage im,int sc)
{
    //判断单屏还是多屏
    if(myApp::VideoType == "4")
    {
        //offlineVideoLabel[sc]->resize(QSize(480,290));
        offlineVideoLabel[sc]->setPixmap(QPixmap::fromImage(im.scaled(480,290)));
            //screenCount++;

    }
    else
    {
        offlineVideoLabel[0]->setPixmap(QPixmap::fromImage(im));
        offlineVideoLabel[0]->setAlignment(Qt::AlignCenter);
    }

}
void offline::bgSubtraction()
{
    if (videoFlag==1)
    {
        timer->stop();
        Mat fgmask;
        while(capture.read(frame))
        {
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
            selectScreen(image,screenCount);
            //ui->labelVideo1->setPixmap(QPixmap::fromImage(image));
            //ui->labelVideo1->setAlignment(Qt::AlignCenter);
            if (waitKey(1) >= 0) break;
        }
    }
}

void offline::od_alg_hog()
{
    String file = filePath.toStdString();
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
    //cv::resize(image, image, Size(480, 290));
    QImage img=offline::Mat2QImage(image);
    selectScreen(img,screenCount);
    //ui->labelVideo1->setPixmap(QPixmap::fromImage(img));
    //ui->labelVideo1->setAlignment(Qt::AlignCenter);

    }

}

void offline::od_alg_dpm()
{
    String dpm_model_path = "../../models/detection/dpm/inriaperson.xml";
    String dpm_image_dir = filePath.toStdString();

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
    //cv::resize(frame, frame, Size(480, 290));
    QImage img=offline::Mat2QImage(frame);
    selectScreen(img,screenCount);
    //ui->labelVideo2->setPixmap(QPixmap::fromImage(img));
    //ui->labelVideo2->setAlignment(Qt::AlignCenter);
}

void offline::od_alg_faster_rcnn()
{
    const char* classNames[] = {
        "____",
        "aeroplane", "bicycle", "bird", "boat",
        "bottle", "bus", "car", "cat", "chair",
        "cow", "diningtable", "dog", "horse",
        "motorbike", "person", "pottedplant",
        "sheep", "sofa", "train", "tvmonitor"
    };



    if(filePath == NULL)
        QMessageBox::information(this, QString::fromLocal8Bit("警告"),QString::fromLocal8Bit("当前路径为空,请打开一张图片"));
    else if(videoFlag == 1){
        QMessageBox::information(this, QString::fromLocal8Bit("警告"),QString::fromLocal8Bit("当前路径为视频文件,请打开一张图片"));
    }
    else{
        String protoPath = "../../models/detection/faster_rcnn/faster_rcnn_vgg16.prototxt";
        String modelPath = "../../models/detection/faster_rcnn/VGG16_faster_rcnn_final.caffemodel";
        String imagePath = filePath.toStdString();
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
        //cv::resize(img, img, Size(480, 290));
        image=offline::Mat2QImage(img);
        selectScreen(image,screenCount);
        //ui->labelVideo1->setPixmap(QPixmap::fromImage(image));
        //ui->labelVideo1->setAlignment(Qt::AlignCenter);

    }
}

void offline::od_alg_ssd()
{

    const char* classNames[] = {"",
                                "aeroplane", "bicycle", "bird", "boat",
                                "bottle", "bus", "car", "cat", "chair",
                                "cow", "diningtable", "dog", "horse",
                                "motorbike", "person", "pottedplant",
                                "sheep", "sofa", "train", "tvmonitor"};
    String modelConfiguration = "../../models/detection/ssd/deploy.prototxt";
    String modelBinary = "../../models/detection/ssd/VGG_VOC0712_SSD_300x300_iter_120000.caffemodel";
    //String filePath = "../../videos/original.mp4";
    //int cameraDevice = 0;
    float min_confidence = 0.5;
    //! [Initialize network]
    dnn::Net net = readNetFromCaffe(modelConfiguration, modelBinary);
    //! [Initialize network]

    if (net.empty())
    {
        QMessageBox::information(this,QString::fromLocal8Bit("警告"),QString::fromLocal8Bit("无法加载配置文件"));
    }
    if (videoFlag==1)
    {
        timer->stop();
        for(;;)
        {
            Mat ssdFrame;
            capture>>ssdFrame;
           //g_reader->nextFrame(frame);
            if (ssdFrame.empty())
            {
            waitKey();
            break;      
            }        
            if (ssdFrame.channels() == 4)
            cvtColor(ssdFrame, ssdFrame, COLOR_BGRA2BGR);
            //! [Prepare blob]
            Mat inputBlob = blobFromImage(ssdFrame, 1.0f, Size(300, 300), Scalar(104, 117, 123), false, false); //Convert Mat to batch of images
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
            ss << "SSD FPS: " << 1000/time << " ; time: " << time << " ms";
            putText(ssdFrame, ss.str(), Point(20,20), 0, 0.5, Scalar(0,0,255));

            Mat detectionMat(detection.size[2], detection.size[3], CV_32F, detection.ptr<float>());

            float confidenceThreshold = min_confidence;
            for(int i = 0; i < detectionMat.rows; i++)
            {
                float confidence = detectionMat.at<float>(i, 2);

                if(confidence > confidenceThreshold)
                {
                    size_t objectClass = (size_t)(detectionMat.at<float>(i, 1));

                    int xLeftBottom = static_cast<int>(detectionMat.at<float>(i, 3) * ssdFrame.cols);
                    int yLeftBottom = static_cast<int>(detectionMat.at<float>(i, 4) * ssdFrame.rows);
                    int xRightTop = static_cast<int>(detectionMat.at<float>(i, 5) * ssdFrame.cols);
                    int yRightTop = static_cast<int>(detectionMat.at<float>(i, 6) * ssdFrame.rows);

                    ss.str("");
                    ss << confidence;
                    String conf(ss.str());

                    Rect object(xLeftBottom, yLeftBottom,
                            xRightTop - xLeftBottom,
                            yRightTop - yLeftBottom);

                    rectangle(ssdFrame, object, Scalar(0, 255, 0));
                    String label = String(classNames[objectClass]) + ": " + conf;
                    int baseLine = 0;
                    Size labelSize = getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
                    rectangle(ssdFrame, Rect(Point(xLeftBottom, yLeftBottom - labelSize.height),
                                      Size(labelSize.width, labelSize.height + baseLine)),
                          Scalar(255, 255, 255), FILLED);
                    putText(ssdFrame, label, Point(xLeftBottom, yLeftBottom),
                        FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0,0,0));
                }
            }

            //imshow("detections", ssdFrame);
            //if (waitKey(1) >= 0) break;
            //cv::resize(ssdFrame, ssdFrame, Size(kInpWidth, kInpHeight));
            QImage ssdQImage;
            //cv::resize(ssdFrame, ssdFrame, Size(480, 290));
            ssdQImage=offline::Mat2QImage(ssdFrame);
            selectScreen(ssdQImage,screenCount);
            //ui->labelVideo3->setPixmap(QPixmap::fromImage(ssdQImage));
            //ui->labelVideo3->setAlignment(Qt::AlignCenter);
            if (waitKey(1) >= 0) break;
            }
        }
        else {
            Mat ssdImage;
            String ssdImagePath = filePath.toStdString();
            ssdImage = imread(ssdImagePath);
            if(!ssdImage.data)
            {
                QMessageBox::information(this,QString::fromLocal8Bit("警告"),QString::fromLocal8Bit("当前路径为空，请打开一张图片或一段视频"));
                //QMessageBox msgBox;
                //msgBox.setText(tr("Image Data Is Null"));
                //msgBox.exec();
            }

            else
            {
                if (ssdImage.channels() == 4)
                cvtColor(ssdImage, ssdImage, COLOR_BGRA2BGR);
                //clock_t t_ssd_1 = clock();
                //! [Prepare blob]
                Mat inputBlob = blobFromImage(ssdImage, 1.0f, Size(300, 300), Scalar(104, 117, 123), false, false); //Convert Mat to batch of images
                //! [Prepare blob]
                //clock_t t_ssd_2 =clock();
                //! [Set input blob]
                net.setInput(inputBlob, "data"); //set the network input
                //! [Set input blob]
                //clock_t t_ssd_3 =clock();

                //! [Make forward pass]
                Mat detection = net.forward("detection_out"); //compute output
                //! [Make forward pass]
                //clock_t t_ssd_4 =clock();
                //double time_resize=(double)(t_ssd_2 - t_ssd_1);
                //double time_intput=(double)(t_ssd_3 - t_ssd_2);
                //double time_detection=(double)(t_ssd_4 - t_ssd_3);

                vector<double> layersTimings;
                double freq = getTickFrequency() / 1000;
                double time = net.getPerfProfile(layersTimings) / freq;
                ostringstream ss;
                ss << "FPS: " << 1000/time << " ; time: " << time << "ms";
                putText(ssdImage, ss.str(), Point(20,20), 0, 0.5, Scalar(0,0,255));

                Mat detectionMat(detection.size[2], detection.size[3], CV_32F, detection.ptr<float>());

                float confidenceThreshold = min_confidence;
                for(int i = 0; i < detectionMat.rows; i++)
                {
                    float confidence = detectionMat.at<float>(i, 2);
                    if(confidence > confidenceThreshold)
                    {
                        size_t objectClass = (size_t)(detectionMat.at<float>(i, 1));

                        int xLeftBottom = static_cast<int>(detectionMat.at<float>(i, 3) * ssdImage.cols);
                        int yLeftBottom = static_cast<int>(detectionMat.at<float>(i, 4) * ssdImage.rows);
                        int xRightTop = static_cast<int>(detectionMat.at<float>(i, 5) * ssdImage.cols);
                        int yRightTop = static_cast<int>(detectionMat.at<float>(i, 6) * ssdImage.rows);

                        ss.str("");
                        ss << confidence;
                        String conf(ss.str());

                        Rect object(xLeftBottom, yLeftBottom,
                                    xRightTop - xLeftBottom,
                                    yRightTop - yLeftBottom);

                        rectangle(ssdImage, object, Scalar(0, 255, 0));
                        String label = String(classNames[objectClass]) + ": " + conf;
                        int baseLine = 0;
                        Size labelSize = getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
                        rectangle(ssdImage, Rect(Point(xLeftBottom, yLeftBottom - labelSize.height),
                                              Size(labelSize.width, labelSize.height + baseLine)),
                                  Scalar(255, 255, 255), FILLED);
                        putText(ssdImage, label, Point(xLeftBottom, yLeftBottom),
                                FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0,0,0));
                    }
                }
                QImage ssdQImage;
                //cv::resize(ssdImage, ssdImage, Size(480, 290));
                ssdQImage=offline::Mat2QImage(ssdImage);
                selectScreen(ssdQImage,screenCount);
                //ui->labelVideo3->setPixmap(QPixmap::fromImage(ssdQImage));
                //ui->labelVideo3->setAlignment(Qt::AlignCenter);

//              ui->label_info->setText("time_resize="+QString::number(time_resize,'f',6)+'\n'+
//                                    "time_input="+QString::number(time_intput,'f',6)+'\n'+
//                                    "time_detection="+QString::number(time_detection,'f',6)+'\n');

            }
    }

}

void offline::od_alg_yolo()
{
    String modelConfiguration = "../../models/detection/yolo/yolov2.cfg";
    String modelBinary = "../../models/detection/yolo/yolov2.weights";
    String classNames ="../../models/detection/yolo/coco.names";
    //String source = filePath.toStdString();
    //String  out = "../../images/output.jpg";
    String object_roi_style = "box";   // box or line style draw
    //int cameraDevice = 0;
    //double fps = 3;
    float confidenceThreshold = 0.24;


    //! [Initialize network]
    dnn::Net net = readNetFromDarknet(modelConfiguration, modelBinary);
    //! [Initialize network]

    if (net.empty())
    {
        QMessageBox::information(this, QString::fromLocal8Bit("警告"),QString::fromLocal8Bit("无法加载模型及配置文件"));

    }

    vector<String> classNamesVec;
    ifstream classNamesFile(classNames.c_str());
    if (classNamesFile.is_open())
    {
        string className = "";
        while (std::getline(classNamesFile, className))
            classNamesVec.push_back(className);
    }
    if(videoFlag==1)
    {
        timer->stop();
        //capture.release();
        //VideoCapture cap;
        //cap.open("/home/mmap/work/VisionAlgShow/videos/1026.mp4");
        for(;;)
        {
            Mat yoloFrame;
            capture >> yoloFrame; // get a new frame from camera/video or read image

            if (yoloFrame.empty())
            {
                waitKey();
                break;
            }

            if (yoloFrame.channels() == 4)
                cvtColor(yoloFrame, yoloFrame, COLOR_BGRA2BGR);

            //! [Prepare blob]
            Mat inputBlob = blobFromImage(yoloFrame, 1 / 255.F, Size(416, 416), Scalar(), true, false); //Convert Mat to batch of images
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
            putText(yoloFrame, format("YOLO FPS: %.2f ; time: %.2f ms", 1000.f / time_ms, time_ms),
                    Point(20, 20), 0, 0.5, Scalar(0, 0, 255));

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
                    float x_center = detectionMat.at<float>(i, 0) * yoloFrame.cols;
                    float y_center = detectionMat.at<float>(i, 1) * yoloFrame.rows;
                    float width = detectionMat.at<float>(i, 2) * yoloFrame.cols;
                    float height = detectionMat.at<float>(i, 3) * yoloFrame.rows;
                    Point p1(cvRound(x_center - width / 2), cvRound(y_center - height / 2));
                    Point p2(cvRound(x_center + width / 2), cvRound(y_center + height / 2));
                    Rect object(p1, p2);

                    Scalar object_roi_color(0, 255, 0);

                    if (object_roi_style == "box")
                    {
                        rectangle(yoloFrame, object, object_roi_color);
                    }
                    else
                    {
                        Point p_center(cvRound(x_center), cvRound(y_center));
                        line(yoloFrame, object.tl(), p_center, object_roi_color, 1);
                    }

                    String className = objectClass < classNamesVec.size() ? classNamesVec[objectClass] : cv::format("unknown(%d)", objectClass);
                    String label = format("%s: %.2f", className.c_str(), confidence);
                    int baseLine = 0;
                    Size labelSize = getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
                    rectangle(yoloFrame, Rect(p1, Size(labelSize.width, labelSize.height + baseLine)),
                              object_roi_color, FILLED);
                    putText(yoloFrame, label, p1 + Point(0, labelSize.height),
                            FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0,0,0));
                }
            }

            //imshow("YOLO: Detections", frame);
            //if (waitKey(1) >= 0) break;
            //cv::resize(yoloFrame, yoloFrame, Size(480, 290));

            QImage yoloQImage=offline::Mat2QImage(yoloFrame);
            selectScreen(yoloQImage,screenCount);
            //ui->labelVideo4->setPixmap(QPixmap::fromImage(yoloQImage));
            //ui->labelVideo4->setAlignment(Qt::AlignCenter);
            if (waitKey(1) >= 0) break;


        }
    }

    else
    {

       Mat yoloImage;
       String yoloImagePath=filePath.toStdString();
       yoloImage=imread(yoloImagePath);

       if (yoloImage.channels() == 4)
       cvtColor(yoloImage, yoloImage, COLOR_BGRA2BGR);

       //! [Prepare blob]
       Mat inputBlob = blobFromImage(yoloImage, 1 / 255.F, Size(416, 416), Scalar(), true, false); //Convert Mat to batch of images
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

       QImage yoloQImage=offline::Mat2QImage(yoloImage);
       selectScreen(yoloQImage,screenCount);
       //ui->labelVideo4->setPixmap(QPixmap::fromImage(yoloQImage));
       //ui->labelVideo4->setAlignment(Qt::AlignCenter);
    }


}







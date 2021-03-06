#include "offline.h"
#include "ui_offline.h"

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

void offline::od_alg_yolo()
{
//    yoloDetector = new YOLO_V2("/home/mmap/work/VisionAlgShow/models/detection/yolo/yolo_v3.cfg","/home/mmap/work/VisionAlgShow/models/detection/yolo/yolov3.weights");
    yoloDetector = new YOLO_V2();
    iouTracker = new IOUTracker();
    if(videoFlag==1)
    {
        timer->stop();
        disconnect(timer, SIGNAL(timeout()), this, SLOT(playbyframe()));
        connect(timer, SIGNAL(timeout()), this, SLOT(playYoloV2Detect()));
        timer->setInterval(1000/20);   //set timer match with FPS
        timer->start();
    }
    else
    {
       Mat yoloImage;
       String yoloImagePath=filePath.toStdString();
       yoloImage=imread(yoloImagePath);
       if (yoloImage.channels() == 4)
       cvtColor(yoloImage, yoloImage, COLOR_BGRA2BGR);
       vector<BoundingBox> boxes = yoloDetector->detect(yoloImage);
       for (int i = 0; i < boxes.size(); i++) {
           int x = (int) boxes[i].x;
           int y = (int) boxes[i].y;
           int w = (int) boxes[i].w;
           int h = (int) boxes[i].h;
           cv::rectangle(yoloImage, cv::Rect(x, y, w, h), cv::Scalar(255, 0, 0), 2);
       }
       QImage yoloQImage=offline::Mat2QImage(yoloImage);
       selectScreen(yoloQImage,screenCount-1);
    }
}

void offline::playYoloV2Detect()
{
    Mat yoloFrame;
    capture >> yoloFrame; // get a new frame from camera/video or read image

    if(yoloFrame.empty())
    {
        timer->stop();
        disconnect(timer, SIGNAL(timeout()), this, SLOT(playYoloV2Detect()));
        return;
    }

//    std::cout << yoloFrame.rows << "   "  << yoloFrame.cols << std::endl;

    if (yoloFrame.channels() == 4)
        cvtColor(yoloFrame, yoloFrame, COLOR_BGRA2BGR);

    vector<BoundingBox> boxes = yoloDetector->detect(yoloFrame);
    vector<BoundingBox> outputs = iouTracker->track_iou(boxes);
    for (int i = 0; i < outputs.size(); i++) {
        int x = (int) outputs[i].x;
        int y = (int) outputs[i].y;
        int w = (int) outputs[i].w;
        int h = (int) outputs[i].h;
        cv::rectangle(yoloFrame, cv::Rect(x, y, w, h), cv::Scalar(255, 0, 0), 2);
        cout << "id = " << outputs[i].id << endl;
        cv::putText(yoloFrame, std::to_string(outputs[i].id), cv::Point(outputs[i].x+outputs[i].w-outputs[i].w/2, outputs[i].y), 1, 2, cv::Scalar(0,255,255), 2);
    }

    QImage yoloQImage=offline::Mat2QImage(yoloFrame);
    selectScreen(yoloQImage,screenCount-1);
}


void offline::InitStyle()
{

    this->setStyleSheet("QGroupBox#gboxMain{border-width:0px;}");
    this->setProperty("Form", true);
//  this->setWindowFlags(Qt::FramelessWindowHint |
//                         Qt::WindowSystemMenuHint |
//                         Qt::WindowMinMaxButtonsHint);
    this->setWindowFlags(Qt::Widget);
    this->setWindowTitle("本地视频文件处理");

    //背景建模
    cbox_bg = ui->cbox_bg;
    string str_bg[] = {"背景建模", "KNN", "MOG2"};
    vector<string> alg_bg(str_bg, str_bg+3);
    addCboxItem(cbox_bg, alg_bg);

    //目标检测
    cbox_od = ui->cbox_od;
    string str_od[] = {"行人检测", "HOG+SVM", "DPM", "Faster R-CNN", "YOLO_V2", "SSD"};
    vector<string> alg_od(str_od, str_od+6);
    addCboxItem(cbox_od, alg_od);

    //目标跟踪
    cbox_sot = ui->cbox_sot;
    string str_sot[] = {"目标跟踪"};
    vector<string> alg_sot(str_sot, str_sot+1);
    addCboxItem(cbox_sot, alg_sot);

    //衣服解析
    cbox_sm = ui->cbox_sm;
    string str_sm[] = {"行人解析"};
    vector<string> alg_sm(str_sm, str_sm+1);
    addCboxItem(cbox_sm, alg_sm);
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
    show_video_1();
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
    else if(od_alg=="YOLO_V2")
    {
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
            filePath = "";
        }
        filePath =QFileDialog::getOpenFileName(NULL,tr("选择文件"),".",tr("Image or Video Files(*.jpg *.png *.JPEG *.avi *.mp4 *.flv *.mkv)"));
        ui->text_dir->setText(filePath);
        QFileInfo fileInfo=QFileInfo(filePath);
        fileSuffix=fileInfo.suffix();
        if(fileSuffix == "jpg" || fileSuffix == "png" || fileSuffix == "JPEG" )
        {
            videoFlag = 0;
            //static const int kInpWidth = 960;
            //static const int kInpHeight = 580;
            cv::Mat image = cv::imread(filePath.toStdString());
            //cv::resize(image, image, Size(480, 290));
            QImage img = offline::Mat2QImage(image);
            selectScreen(img,screenCount);
            screenCount++;
        }
            else
            {
                videoFlag = 1;
                capture.open(filePath.toStdString());
                //screenCount++;
                //GPU
                //g_reader = cv::cudacodec::createVideoReader(filePath.toStdString());
                //g_reader->nextFrame(g_frame);
                //rate = 30;
                if (capture.isOpened())
                {
                    rate= capture.get(CAP_PROP_FPS);
                    capture >> frame;
                    if (!frame.empty())
                    {
                        image = Mat2QImage(frame);
                        selectScreen(image,screenCount++);
                        timer = new QTimer(this);
                        timer->setInterval(1000/rate);   //set timer match with FPS
                        connect(timer, SIGNAL(timeout()), this, SLOT(playbyframe()));
                        //qDebug("screenCount:%d", screenCount);

                        timer->start();
                    }

                }
                //screenCount++;
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
        selectScreen(image,screenCount-1);
    }
}

// opencv Mat to Image
QImage offline::Mat2QImage(cv::Mat cvImg)
{
    QImage qImg;
    if(cvImg.channels()==3)                             //3 channels color image
    {

        cv::cvtColor(cvImg,cvImg,COLOR_BGR2RGB);
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
    if(offlineWindowNum == 4)
    {
        offlineVideoLabel[0]->setPixmap(QPixmap::fromImage(im.scaled(480,290)));
    }
    else
    {
        offlineVideoLabel[0]->setPixmap(QPixmap::fromImage(im.scaled(960,580)));
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
            selectScreen(image,screenCount-1);
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
    selectScreen(img,screenCount-1);
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

    // draw text on imagevivviv
    //String text =format("%0.1f fps",1.0/t);
    //Scalar textColor(0,0,250);
    //putText(frame, text, Point(10,50), FONT_HERSHEY_PLAIN, 2, textColor, 2);
    //cv::resize(frame, frame, Size(480, 290));
    QImage img=offline::Mat2QImage(frame);
    selectScreen(img,screenCount-1);
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
        FasterRCNNDetector *fasterRCNNDetector = new FasterRCNNDetector();
        if(fasterRCNNDetector->loadNet()==-1)
        {
            QMessageBox::information(this, QString::fromLocal8Bit("警告"),QString::fromLocal8Bit("无法加载模型及配置文件"));
            return;
        }
        vector<BoundingBox> bbox ;
        String imgPath = filePath.toStdString();
        Mat img = imread(imgPath);
        bbox = fasterRCNNDetector->detect(img);

        // Draw detections.
        for (size_t i = 0; i < bbox.size(); i++){

            //An every bbox is a vector [confidence,x,y,w,h]
            int left = bbox[i].x;
            int top = bbox[i].y;
            int right = bbox[i].x + bbox[i].w;
            int bottom = bbox[i].y - bbox[i].h;

            // Draw a bounding box.
            rectangle(img, Point(left, top), Point(right, bottom), Scalar(0, 255, 0));

            // Put a label with a class name and confidence.
            String label = cv::format("%s, %.3f", bbox[i].confidence);
            int baseLine;
            Size labelSize = cv::getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);

            top = max(top, labelSize.height);
            rectangle(img, Point(left, top - labelSize.height),
                      Point(left + labelSize.width, top + baseLine),
                      Scalar(255, 255, 255), FILLED);
            putText(img, label, Point(left, top), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0));
            }
        //cv::resize(img, img, Size(480, 290));
        QImage qImg;
        qImg=offline::Mat2QImage(img);
        selectScreen(qImg,screenCount-1);
        //ui->labelVideo1->setPixmap(QPixmap::fromImage(image));
        //ui->labelVideo1->setAlignment(Qt::AlignCenter);

        }

//        String protoPath = "../../models/detection/faster_rcnn/faster_rcnn_vgg16.prototxt";
//        String modelPath = "../../models/detection/faster_rcnn/VGG16_faster_rcnn_final.caffemodel";
//        String imagePath = filePath.toStdString();
//        float confThreshold = 0.8;
//        CV_Assert(!protoPath.empty(), !modelPath.empty(), !imagePath.empty());

//        // Load a model.
//        Net net = readNetFromCaffe(protoPath, modelPath);

//        Mat img = imread(imagePath);
//        Mat blob = blobFromImage(img, 1.0, Size(), Scalar(102.9801, 115.9465, 122.7717), false, false);
//        Mat imInfo = (Mat_<float>(1, 3) << img.rows, img.cols, 1.6f);

//        net.setInput(blob, "data");
//        net.setInput(imInfo, "im_info");

//        // Draw detections.
//        Mat detections = net.forward();
//        const float* data = (float*)detections.data;
//        for (size_t i = 0; i < detections.total(); i += 7)
//        {
//            // An every detection is a vector [id, classId, confidence, left, top, right, bottom]
//            float confidence = data[i + 2];
//            if (confidence > confThreshold)
//            {
//                int classId = (int)data[i + 1];
//                int left = max(0, min((int)data[i + 3], img.cols - 1));
//                int top = max(0, min((int)data[i + 4], img.rows - 1));
//                int right = max(0, min((int)data[i + 5], img.cols - 1));
//                int bottom = max(0, min((int)data[i + 6], img.rows - 1));

//                // Draw a bounding box.
//                rectangle(img, Point(left, top), Point(right, bottom), Scalar(0, 255, 0));

//                // Put a label with a class name and confidence.
//                String label = cv::format("%s, %.3f", classNames[classId], confidence);
//                int baseLine;
//                Size labelSize = cv::getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);

//                top = max(top, labelSize.height);
//                rectangle(img, Point(left, top - labelSize.height),
//                          Point(left + labelSize.width, top + baseLine),
//                          Scalar(255, 255, 255), FILLED);
//                putText(img, label, Point(left, top), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0));
//            }
//        }
//    }
}

void offline::od_alg_ssd()
{
    const char* classNames[] = {
        "____",
        "aeroplane", "bicycle", "bird", "boat",
        "bottle", "bus", "car", "cat", "chair",
        "cow", "diningtable", "dog", "horse",
        "motorbike", "person", "pottedplant",
        "sheep", "sofa", "train", "tvmonitor"
    };
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
            selectScreen(ssdQImage,screenCount-1);
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
                selectScreen(ssdQImage,screenCount-1);
                //ui->labelVideo3->setPixmap(QPixmap::fromImage(ssdQImage));
                //ui->labelVideo3->setAlignment(Qt::AlignCenter);

//              ui->label_info->setText("time_resize="+QString::number(time_resize,'f',6)+'\n'+
//                                    "time_input="+QString::number(time_intput,'f',6)+'\n'+
//                                    "time_detection="+QString::number(time_detection,'f',6)+'\n');

            }
    }

}








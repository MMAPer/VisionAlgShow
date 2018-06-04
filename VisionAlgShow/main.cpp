#include "visionalgmain.h"
#include "myhelper.h"
#include "myapp.h"
#include "offline.h"
#include "tracking.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("VisionAlgShow");         //设置应用程序名称
    a.setApplicationVersion("V201805"); //设置应用程序版本
    a.setWindowIcon(QIcon("image/v.ico"));
    myHelper::SetUTF8Code();            //设置程序编码为UTF-8,兼容QT4
    myHelper::SetChinese();             //设置程序为中文字库

    //赋值当前应用程序路径和桌面宽度高度
    myApp::AppPath = QApplication::applicationDirPath() + "/";
    myApp::DeskWidth = qApp->desktop()->availableGeometry().width();
    myApp::DeskHeight = qApp->desktop()->availableGeometry().height();
    qDebug()<<"AppPath ="<<myApp::AppPath<<" DeskWidth ="<<myApp::DeskWidth<<" DeskHeight ="<<myApp::DeskHeight;

    //加载和应用样式
    //myHelper::SetStyle(myApp::AppStyle);

    VisionAlgMain w;
    w.show();
    w.setGeometry(qApp->desktop()->availableGeometry());

    return a.exec();
}

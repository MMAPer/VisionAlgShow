#include "visionalgmain.h"
#include "myhelper.h"
#include "myapp.h"
#include "offline.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("VM");         //设置应用程序名称
    a.setApplicationVersion("V201412"); //设置应用程序版本
    myHelper::SetUTF8Code();            //设置程序编码为UTF-8
    myHelper::SetChinese();             //设置程序为中文字库

    //赋值当前应用程序路径和桌面宽度高度
    myApp::AppPath = QApplication::applicationDirPath() + "/";
    myApp::DeskWidth = qApp->desktop()->availableGeometry().width();
    myApp::DeskHeight = qApp->desktop()->availableGeometry().height();

    //加载和应用样式
    myHelper::SetStyle(myApp::AppStyle);

    VisionAlgMain w;
    w.show();
    w.setGeometry(qApp->desktop()->availableGeometry());

    offline o;
    o.setGeometry(qApp->desktop()->availableGeometry());
    QObject::connect(&w,SIGNAL(offlineHandle()),&o,SLOT(receiveofflinehandle()));
    QObject::connect(&o,SIGNAL(backmain()),&w,SLOT(receiveback()));


    return a.exec();
}

#ifndef MYHELPER_H
#define MYHELPER_H

#include <QtCore>
#include <QtGui>
#if (QT_VERSION > QT_VERSION_CHECK(5,0,0))
#include <QtWidgets>
#endif

/* 说明:全局辅助操作类实现文件
 * 功能:窗体显示/编码设置/开机启动/文件选择与复制/对话框等
 * 作者:张精制  QQ:852370792
 * 时间:2018-03-10  检查:2018-3-10
 */
class myHelper: public QObject
{

public:

    //此函数为了兼容QT4，设置编码为UTF8
    static void SetUTF8Code() {
        #if (QT_VERSION <= QT_VERSION_CHECK(5,0,0))
                QTextCodec *codec = QTextCodec::codecForName("UTF-8");
                QTextCodec::setCodecForLocale(codec);
                QTextCodec::setCodecForCStrings(codec);
                QTextCodec::setCodecForTr(codec);
        #endif
    }

    //设置为中文字符
    static void SetChinese() {
        QTranslator *translator = new QTranslator(qApp);
        translator->load(":/image/qt_zh_CN.qm");
        qApp->installTranslator(translator);
    }

    //设置指定样式
    static void SetStyle(const QString &qssFile) {
        QFile file(qssFile);
        if (file.open(QFile::ReadOnly)) {
            QString qss = QLatin1String(file.readAll());
            qApp->setStyleSheet(qss);
            //QString PaletteColor = qss.mid(20, 7);
            //qApp->setPalette(QPalette(QColor(PaletteColor)));
            file.close();
        }
    }

    //窗体居中显示
    static void FormInCenter(QWidget *frm, int deskWidth, int deskHeigth) {
        int frmX = frm->width();
        int frmY = frm->height();
        QPoint movePoint(deskWidth / 2 - frmX / 2, deskHeigth / 2 - frmY / 2);
        frm->move(movePoint);
    }
};

#endif // MYHELPER_H

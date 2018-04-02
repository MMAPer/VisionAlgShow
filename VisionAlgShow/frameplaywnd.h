#ifndef FRAMEPLAYWND_H
#define FRAMEPLAYWND_H

#include <QFrame>
#include "ui_frameplaywnd.h"
#include "Public.h"
#include "QResizeEvent"
#include "QMouseEvent"

/* 说明:播放窗口类
 * 功能:播放区域包含多个播放窗口，所以抽象出单独的类
 * 作者:张精制  QQ:852370792
 * 时间:2018-03-10  检查:2018-3-10
 */
namespace Ui {
class FramePlayWnd;
}

//播放区域
typedef struct tagPlayRect
{
    int iX;
    int iY;
    int iWidth;
    int iHeight;
}CLIENT_PLAY_RECT, *LPCLIENT_PLAY_RECT;

class FramePlayWnd : public QFrame
{
    Q_OBJECT

public:
    explicit FramePlayWnd(QWidget *parent = 0);
    ~FramePlayWnd();

    WId GetPlayWndId();
    int GetPlayRect(CLIENT_PLAY_RECT *pPlayRect);

public slots:

protected:
    void resizeEvent(QResizeEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

signals:
    //sigle click signal
    void SingleClickSig();

private:
    Ui::FramePlayWnd *ui;
    QFrame *m_pframePlay;  //windows下作为播放窗口

    int m_iBorderWidth;
    int m_iBorderHeight;

    int m_bSelected;
};

#endif // FRAMEPLAYWND_H

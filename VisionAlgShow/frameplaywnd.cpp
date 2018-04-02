#include "frameplaywnd.h"
#include "ui_frameplaywnd.h"
#include "QEvent"
#include "QDebug"


FramePlayWnd::FramePlayWnd(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::FramePlayWnd)
{
    ui->setupUi(this);

    m_iBorderWidth = 2;
    m_iBorderHeight = 2;
    m_bSelected = FALSE;

    setStyleSheet("border: 2px solid gray;");

    m_pframePlay = ui->playWnd;
    m_pframePlay->move(m_iBorderWidth, m_iBorderHeight);  //和上面的border保持一致
    m_pframePlay->setStyleSheet("border: none");
}

FramePlayWnd::~FramePlayWnd()
{
    delete ui;
}

/********************************************
Function: FramePlayWnd::resizeEvent
Description:  new size
Parameter:  (IN) QResizeEvent *event Size event.
Return:  none
*********************************************/
void FramePlayWnd::resizeEvent(QResizeEvent* event)
{
    int iTempWidth;
    int iTempHeight;
    iTempWidth = event->size().width();
    iTempHeight = event->size().height();
    qDebug()<<"iTempWidth = "<<iTempWidth<<"iTempHeight"<<iTempHeight;
    m_pframePlay->resize(iTempWidth-2*2, iTempHeight-2*2);
}

/********************************************
Function:  FramePlayWnd::GetPlayWndId
Description:  Get play HWND in Windows
Parameter:  none
Return:  Window ID
*********************************************/
WId FramePlayWnd::GetPlayWndId()
{
    return m_pframePlay->winId();
}

/********************************************
Function:  FramePlayWnd::GetPlayRect
Description:  Get play rect in windows and linux
Parameter:  (IN) CLIENT_PLAY_RECT* pPlayRect
Return: 0-success, -1-fail
*********************************************/
int FramePlayWnd::GetPlayRect(CLIENT_PLAY_RECT *pPlayRect)
{
    if(NULL == pPlayRect)
    {
        return -1;
    }
    pPlayRect->iX = x() + m_iBorderWidth;
    pPlayRect->iY = y() + m_iBorderHeight;

    //计算尺寸
    pPlayRect->iWidth = m_pframePlay->width();
    pPlayRect->iHeight = m_pframePlay->height();
    return 0;
}

void FramePlayWnd::mouseReleaseEvent(QMouseEvent *event)
{
    emit SingleClickSig();
}









#include "VideoCallForm.h"
#include "Utils.h"
#include "Commands.h"

#include <QtGui/QVBoxLayout>
#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtCore/QDir>

VideoCallForm::VideoCallForm(QWidget *parent)
    : CallHandlerWidget(parent)
{
    RECTANGLE   tmprect;

    QWidget*    widget  = loadForm("qss/forms/videoCall.ui");

    QVBoxLayout*layout  = new QVBoxLayout;

    layout->addWidget(widget);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    this->setLayout(layout);

    assign(this, m_end,     "videoCall_end");
    assign(this, m_sms,     "videoCall_sms");
    assign(this, m_chat,    "videoCall_chat");
    assign(this, m_file,    "videoCall_file");
    assign(this, m_resume,  "videoCall_resume");
    assign(this, m_hold,    "videoCall_hold");
    assign(this, m_add,     "videoCall_add");
    assign(this, m_pip,     "videoCall_pip");
    assign(this, m_capture, "videoCall_capture");
    assign(this, m_video,   "videoCall_video");
    assign(this, m_me,      "videoCall_me");

    assign(this, m_talking, "videoCall_talking");   m_talking-> setEnabled(false);
    assign(this, m_datetime,"videoCall_datetime");  m_datetime-> setEnabled(false);

    assign(this, m_party,   "videoCall_party");     m_party-> setEnabled(false);

    connect(m_end,      SIGNAL(clicked()),  this, SLOT(onEndClicked()));
    connect(m_hold,     SIGNAL(clicked()),  this, SLOT(onHoldClicked()));
    connect(m_resume,   SIGNAL(clicked()),  this, SLOT(onResumeClicked()));
    connect(m_capture,  SIGNAL(clicked()),  this, SLOT(onCaptureClicked()));
    connect(m_add,      SIGNAL(clicked()),  this, SLOT(onAddClicked()));
    connect(m_pip,      SIGNAL(clicked()),  this, SLOT(onPipClicked()));

    connect(m_sms,      SIGNAL(clicked()),  this, SLOT(onSmsClicked()));
    connect(m_chat,     SIGNAL(clicked()),  this, SLOT(onChatClicked()));
    connect(m_file,     SIGNAL(clicked()),  this, SLOT(onFileClicked()));

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(onTimerTick()));
    m_timer.start(1000);
    m_counter   = 0;

    img_video = new videowidget(m_video);
    
    LOGI("img_video = widget %p\n",img_video);
//    vpstack()->SetVideoDataFactory(vpvideoFactory(m_video));
    vpstack()->SetVideoDataFactory(vpvideoFactory(img_video));
//    vpstack()->SetVideoWindowData(0,(void *) img_video, 'chld', &tmprect,true);
    LOGI("VideoForm Setdatafactory\n");

    vpMessageSubject()->addObserver(new VpMessageObserverFilterByBearer(
                                            this,
                                            (1 << BC_VIDEO) | (1 << BC_AUDIOVIDEO)));
    LOGI("VideoForm AddObserver\n");


    m_vpvideo   = NULL;
//#ifdef ANDROID_VIDEO
    m_me = new videowidget(m_me);
    LOGI("me_video = %p\n",m_me);
    m_vfactory  = vpvideoFactory(m_me);
//#endif
    m_fullscreen= false;
    LOGI("VideoForm Created\n");
}

//  VideoCallForm::updateUI
//  -------------------------------------------------------------------------
void    VideoCallForm::updateUI()
{

    if(m_calls.size() == 1)
    {
        VPCALL  call    = (VPCALL)*m_calls.begin();

        if(vpstack()->IsHeld(call))
        {
            m_resume->show();
            m_hold->hide();
        }
        else
        {
            m_resume->hide();
            m_hold->show();
        }

        RECTANGLE   rect;
        QRect       qrect;
        bool        held    = vpstack()->IsHeld(call);

        if(held)
        {
            m_video->setStyleSheet(
                        QString("#%1{background-image : url(%2);}")
                            .arg(m_video->objectName())
                            .arg(getFilename("qss/videoCallForm/held.png")));
        }
        else
        {
            m_video->setStyleSheet(
                        QString("#%1{background-image : url(%2);}")
                            .arg(m_video->objectName())
                            .arg(getFilename("qss/videoCallForm/waiting.png")));
        }


        if(m_vpvideo == NULL)
        {
            char    name[MAXNAMELEN + 1];
            vpstack()->GetCallRemoteName(call, name);
            m_party->setText(name);

            //  set up PIP window
            //  -------------------------------------------------------------
            qrect       = m_me->rect();
            rect.top    = qrect.top();
            rect.left   = qrect.left();
            rect.bottom = qrect.bottom();
            rect.right  = qrect.right();

            unsigned    fourcc;
            unsigned    w;
            unsigned    h;
            unsigned    fr;
            unsigned    q;
//no local video during call
            vpstack()->GetDefaultVideoParameters(&fourcc, &w, &h, &fr, &q);
            LOGI("Default Video parameters for local video %i %i %i\n", w, h, fourcc);
            m_vpvideo   = m_vfactory->New(0, 0, fourcc, w, h, fourcc, w, h, fr, q);
#ifdef ANDROID_VERSION
            m_vpvideo->SetVideoWindowData((void *)m_me, 'abcd', &rect, false);
#else
            m_vpvideo->SetVideoWindowData(m_me->winId(), 'abcd', &rect, false);
#endif        
        }

        //  set up video window
        //  -------------------------------------------------------------
        LOGI("VideoForm set up video window\n");

        qrect       = m_video->rect();
        rect.top    = qrect.top();
        rect.left   = qrect.left();
        rect.bottom = qrect.bottom();
        rect.right  = qrect.right();
#ifdef ANDROID_VERSION
//        vpstack()->SetVideoWindowData(call,(void *) m_video, 'chld', &rect, vpstack()->IsHeld(call));
        vpstack()->SetVideoWindowData(call,(void *) img_video, 'chld', &rect, vpstack()->IsHeld(call));
        LOGI("vpstack()->SetVideoWindowData(call,(void *) img_video=%p, 'chld', &rect, vpstack()->IsHeld(call));\n",img_video);

#else
        vpstack()->SetVideoWindowData(call, m_video->winId(), 'chld', &rect, vpstack()->IsHeld(call));
#endif
        showMe(this);
    }
    else
    {
        if(m_vpvideo)
        {
            delete  m_vpvideo;
            m_vpvideo   = NULL;
        }

        hideMe(this);
    }
}

//  VideoCallForm::call
//  -------------------------------------------------------------------------
void    VideoCallForm::call(
                        const QString&      address)
{
    emit calling(0, qPrintable(address), BC_AUDIOVIDEO);
}

//  VideoCallForm::onEndClicked
//  -------------------------------------------------------------------------
void    VideoCallForm::onEndClicked()
{
    if(!m_calls.empty())
    {
        VPCALL  call    = (VPCALL)*m_calls.begin();

        vpstack()->Disconnect(call, REASON_NORMAL);
    }
}

//  VideoCallForm::onHoldClicked
//  -------------------------------------------------------------------------
void    VideoCallForm::onHoldClicked()
{
    Q_ASSERT(m_calls.size() == 1);

    VPCALL  call    = (VPCALL)*m_calls.begin();

    vpstack()->Hold(call);

    updateUI();
}

//  VideoCallForm::onAddClicked
//  -------------------------------------------------------------------------
void    VideoCallForm::onAddClicked()
{
    sendCommand(commandContactList);
}

//  VideoCallForm::onSmsClicked
//  -------------------------------------------------------------------------
void    VideoCallForm::onSmsClicked()
{
    VPCALL  call    = (VPCALL)*m_calls.begin();

    char    name[MAXNAMELEN + 1];
    vpstack()->GetCallRemoteName(call, name);

    emit makeCall(name, BC_SMS);
}

//  VideoCallForm::onChatClicked
//  -------------------------------------------------------------------------
void    VideoCallForm::onChatClicked()
{
    VPCALL  call    = (VPCALL)*m_calls.begin();

    char    name[MAXNAMELEN + 1];
    vpstack()->GetCallRemoteName(call, name);

    emit makeCall(name, BC_CHAT);
}

//  VideoCallForm::onFileClicked
//  -------------------------------------------------------------------------
void    VideoCallForm::onFileClicked()
{
    VPCALL  call    = (VPCALL)*m_calls.begin();

    char    name[MAXNAMELEN + 1];
    vpstack()->GetCallRemoteName(call, name);

    emit makeCall(name, BC_FILE);
}

//  VideoCallForm::onPipClicked
//  -------------------------------------------------------------------------
void    VideoCallForm::onPipClicked()
{
//    m_fullscreen    = !m_fullscreen;
    vpstack()->SetVideoWindowFullScreen((VPCALL)*m_calls.begin(), true);
}

//  VideoCallForm::onCaptureClicked
//  -------------------------------------------------------------------------
void    VideoCallForm::onCaptureClicked()
{
    QString datetime= QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
    VPCALL  call    = (VPCALL)*m_calls.begin();
    int     rc;
    void*   image;
    int     w;
    int     h;
#ifdef HOME_PATH_SDCARD
    QString filename    = QDir("/sdcard").path() + "/"PROJECTNAME"/Video Capture/" + datetime + ".bmp";
#else
    QString filename    = QDir::homePath() + "/"PROJECTNAME"/Video Capture/" + datetime + ".bmp";
#endif

    rc  = vpstack()->CaptureVideoImage(call, &image, &w, &h);
    if(!rc)
    {
        QFile   file(filename);
        file.open(QIODevice::WriteOnly);

        char    bm[2];
#pragma pack(push, 2)
        struct {
            unsigned        size,
                            reserved,
                            bits;
            unsigned        biSize;
            unsigned        biWidth,
                            biHeight;
            unsigned short  biPlanes,
                            biBitCount;
            unsigned        biCompression,
                            biSizeImage;
            unsigned        notused[4];
        }       bmpheader;
#pragma pack(pop)

        bm[0] = 'B';
        bm[1] = 'M';

        memset(&bmpheader, 0, sizeof bmpheader);

        bmpheader.size          = sizeof(bmpheader) + 2 + 3 * w * h;
        bmpheader.bits          = sizeof(bmpheader) + 2;
        bmpheader.biSize        = 40;
        bmpheader.biWidth       = w;
        bmpheader.biHeight      = h;
        bmpheader.biPlanes      = 1;
        bmpheader.biBitCount    = 24;
        bmpheader.biSizeImage   = w * h;

        file.write(bm,                      sizeof(bm));
        file.write((const char*)&bmpheader, sizeof(bmpheader));
        file.write((const char*)image,          w * h * 3);
        file.close();

        VPFreeBuffer(image);

        sendCommand(commandVideoCaptured, filename);
    }
}

//  VideoCallForm::onResumeClicked
//  -------------------------------------------------------------------------
void    VideoCallForm::onResumeClicked()
{
    Q_ASSERT(m_calls.size() == 1);

    VPCALL  call    = (VPCALL)*m_calls.begin();

    vpstack()->Resume(call);

    updateUI();
}

//  VideoCallForm::onTimerTick
//  -------------------------------------------------------------------------
void    VideoCallForm::onTimerTick()
{
    ++m_counter;

    int     h   = m_counter / (60 * 60);
    int     m   = (m_counter / 60) % 60;
    int     s   = m_counter % 60;

    m_talking->setText(QString(tr("talking %1:%2:%3"))
                        .arg(h, 2, 10, QChar('0'))
                        .arg(m, 2, 10, QChar('0'))
                        .arg(s, 2, 10, QChar('0')));
    m_datetime->setText(QDateTime::currentDateTime().toString());
}

//  VideoCallForm::onSendFileReq
//  -------------------------------------------------------------------------
void    VideoCallForm::onVpStackMessage(
                                unsigned        msg,
                                VPCALL          call,
                                unsigned        param)
{
    switch(msg)
    {
        //  VPMSG_NEWCALL
        //  -----------------------------------------------------------------
        case    VPMSG_NEWCALL   :
        {
            m_calls.insert((int)call);
            vpstack()->AnswerCall(call);

            updateUI();

            m_timer.start(1000);
            m_counter   = 0;

        } break;

        //  VPMSG_CONFERENCEREQ
        //  -----------------------------------------------------------------
        case    VPMSG_CONFERENCEREQ :
        {
            vpstack()->AcceptConference(call, true);
        } break;

        //  VPMSG_CALLENDED
        //  -----------------------------------------------------------------
        case    VPMSG_CALLENDED :
        {

            m_calls.remove((int)call);

            if(m_calls.empty())
            {
                m_timer.stop();
            }

            updateUI();
        } break;

        //  VPMSG_CALLACCEPTED
        //  -----------------------------------------------------------------
        case    VPMSG_CALLACCEPTED  :
        {

            m_calls.insert((int)call);

            updateUI();

            m_timer.start(1000);
            m_counter   = 0;
        } break;
    }
}

//  VideoCallForm::message
//  -------------------------------------------------------------------------
void    VideoCallForm::message(VpMessageSubject*   subject,
                    unsigned            msg,
                    unsigned            pcall,
                    unsigned            param)
{
//    onVpStackMessage(msg, pcall, param);
}

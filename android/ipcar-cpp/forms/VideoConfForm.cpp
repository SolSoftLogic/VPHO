#include "VideoConfForm.h"
#include "Utils.h"
#include "Commands.h"

#include <QtGui/QVBoxLayout>
#include <QtCore/QVariant>

//  VideoConfForm::VideoConfForm
//  -------------------------------------------------------------------------
VideoConfForm::VideoConfForm(QWidget *parent)
    : QWidget(parent)
{
    QWidget*    widget  = loadForm("qss/forms/videoConf.ui");

    QVBoxLayout*layout  = new QVBoxLayout;

    layout->addWidget(widget);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    this->setLayout(layout);

    for(int i = 0; i < MAX_NUM_OF_PARTIES; ++i)
    {
        assign(this, m_partner[i].title,    qPrintable(QString("videoConf_name%1").arg(i + 1)));

        assign(this, m_partner[i].end,      qPrintable(QString("videoConf_end%1").arg(i + 1)));
        assign(this, m_partner[i].hold,     qPrintable(QString("videoConf_hold%1").arg(i + 1)));
        assign(this, m_partner[i].resume,   qPrintable(QString("videoConf_resume%1").arg(i + 1)));
        m_partner[i].resume->hide();
        assign(this, m_partner[i].chat,     qPrintable(QString("videoConf_chat%1").arg(i + 1)));
        assign(this, m_partner[i].file,     qPrintable(QString("videoConf_file%1").arg(i + 1)));
        assign(this, m_partner[i].sms,      qPrintable(QString("videoConf_sms%1").arg(i + 1)));

        assign(this, m_partner[i].video,    qPrintable(QString("videoConf_video%1").arg(i + 1)));

	m_partner[i].video = new videowidget(m_partner[i].video);
	//printf("img_video = %p\n",img_video);

        m_partner[i].video->setStyleSheet(
                                QString("#%1{background-image : url(%2);}")
                                    .arg(m_partner[i].video->objectName())
                                    .arg(getFilename("qss/videoConfForm/waiting.png")));

        m_partner[i].title->setReadOnly(true);

        m_partner[i].end->setProperty("index",      QVariant(i));
        m_partner[i].hold->setProperty("index",     QVariant(i));
        m_partner[i].resume->setProperty("index",   QVariant(i));
        m_partner[i].file->setProperty("index",     QVariant(i));
        m_partner[i].sms->setProperty("index",      QVariant(i));
        m_partner[i].chat->setProperty("index",     QVariant(i));

        connect(m_partner[i].end,   SIGNAL(clicked()),  this,   SLOT(onButttonClicked()));
        connect(m_partner[i].hold,  SIGNAL(clicked()),  this,   SLOT(onButttonClicked()));
        connect(m_partner[i].resume,SIGNAL(clicked()),  this,   SLOT(onButttonClicked()));
        connect(m_partner[i].sms,   SIGNAL(clicked()),  this,   SLOT(onButttonClicked()));
        connect(m_partner[i].file,  SIGNAL(clicked()),  this,   SLOT(onButttonClicked()));
        connect(m_partner[i].chat,  SIGNAL(clicked()),  this,   SLOT(onButttonClicked()));

        m_partner[i].end->setDisabled(true);
        m_partner[i].hold->setDisabled(true);
        m_partner[i].resume->setDisabled(true);
        m_partner[i].sms->setDisabled(true);
        m_partner[i].file->setDisabled(true);
        m_partner[i].chat->setDisabled(true);

        m_partner[i].incall = false;
    }

    vpMessageSubject()->addObserver(new VpMessageObserverFilterByBearer(
                                            this,
                                            (1 << BC_VIDEO) | (1 << BC_AUDIOVIDEO)));
}

//  VideoConfForm::onButttonClicked
//  -------------------------------------------------------------------------
void        VideoConfForm::onButttonClicked()
{
    QPushButton*    button  = qobject_cast<QPushButton*>(sender());
    int             index   = button->property("index").toInt();
    VPCALL          call    = m_partner[index].call;

    if(button == m_partner[index].end)
    {
        vpstack()->Disconnect(call, REASON_NORMAL);
        updateUI();
    }
    else if(button == m_partner[index].hold)
    {
        vpstack()->Hold(m_partner[index].call);
        updateUI();
    }
    else if(button == m_partner[index].resume)
    {
        vpstack()->Resume(m_partner[index].call);
        updateUI();
    }
    else if(button == m_partner[index].sms)
    {
        emit    makeCall(m_partner[index].name, BC_SMS);
    }
    else if(button == m_partner[index].file)
    {
        emit    makeCall(m_partner[index].name, BC_FILE);
    }
    else if(button == m_partner[index].chat)
    {
        emit    makeCall(m_partner[index].name, BC_CHAT);
    }
}

//  VideoConfForm::message
//  -------------------------------------------------------------------------
void    VideoConfForm::message(
                    VpMessageSubject*   subject,
                    unsigned            msg,
                    unsigned            pcall,
                    unsigned            param)
{
    switch(msg)
    {
        //  VPMSG_CONFERENCEREQ
        //  -----------------------------------------------------------------
        case    VPMSG_CONFERENCEREQ :
        {
            vpstack()->AcceptConference((VPCALL)pcall, true);
        } break;

        case    VPMSG_NEWCALL:
        {
            m_calls.insert(pcall);

            updateUI();
        } break;

        case    VPMSG_CALLACCEPTED  :
        {
            m_calls.insert(pcall);

            if(m_calls.size() > 1)
            {
                VPCALL  calls[MAX_NUM_OF_PARTIES];
                int     i   = 0;

                foreach(int call, m_calls)
                {
                    calls[i]    = (VPCALL)call;
                    ++i;
                }
                vpstack()->ConferenceCalls(calls, m_calls.size(), false);
            }

            updateUI();
        } break;

        case    VPMSG_CALLENDED :
        {
            m_calls.remove(pcall);

            updateUI();
        } break;
    }
}

//  VideoConfForm::updateUI
//  -------------------------------------------------------------------------
void    VideoConfForm::updateUI()
{
    if(m_calls.size() <= 1)
    {
        hideMe(this);
        return;
    }

    setUpdatesEnabled(false);

    for(int i = m_calls.size(); i < MAX_NUM_OF_PARTIES; ++i)
    {
        QWidget*video;

        video   = m_partner[i].video;

        video->setStyleSheet(
                    QString("#%1{background-image : url(%2);}")
                        .arg(video->objectName())
                        .arg(getFilename("qss/videoConfForm/waiting.png")));
        m_partner[i].incall  = false;
        m_partner[i].title->setText("");

        m_partner[i].end->setDisabled(true);
        m_partner[i].hold->setDisabled(true);
        m_partner[i].resume->setDisabled(true);
        m_partner[i].sms->setDisabled(true);
        m_partner[i].file->setDisabled(true);
        m_partner[i].chat->setDisabled(true);
    }

    int     i   = 0;
    foreach(int call, m_calls)
    {
        char        username[MAXNAMELEN + 1];

        vpstack()->GetCallRemoteName((VPCALL)call, username);

        QWidget*    video;

        video   = m_partner[i].video;

        m_partner[i].incall  = true;
        m_partner[i].title->setText(username);

        m_partner[i].end->setEnabled(true);
        m_partner[i].hold->setEnabled(true);
        m_partner[i].resume->setEnabled(true);
        m_partner[i].sms->setEnabled(true);
        m_partner[i].file->setEnabled(true);
        m_partner[i].chat->setEnabled(true);

        QRect       qrect;
        RECTANGLE   rect;
        bool        hidden  = false;

        if(vpstack()->IsHeld((VPCALL)call))
        {
            m_partner[i].video->setStyleSheet(
                                    QString("#%1{background-image : url(%2);}")
                                        .arg(m_partner[i].video->objectName())
                                        .arg(getFilename("qss/videoConfForm/held.png")));
            m_partner[i].hold->hide();
            m_partner[i].resume->show();

            hidden  = true;
        }
        else
        {
            m_partner[i].hold->show();
            m_partner[i].resume->hide();
        }

        qrect   = m_partner[i].video->rect();
        rect.left   = qrect.left();
        rect.right  = qrect.right();
        rect.bottom = qrect.bottom();
        rect.top    = qrect.top();

        vpstack()->SetVideoWindowData(
                                    (VPCALL)call,
#ifdef ANDROID_VERSION
//                                    (void *)m_partner[i].video->winId(),
                                    (void *)m_partner[i].video,
#else
                                    m_partner[i].video->winId(),
#endif
                                    'chld',
                                    &rect,
                                    hidden);

        m_partner[i].name   = username;
        m_partner[i].title->setText(username);
        m_partner[i].call   = (VPCALL)call;
        m_partner[i].incall = true;

        ++i;
    }

    setUpdatesEnabled(true);

    showMe(this);
}

//  VideoConfForm::showEvent
//  -------------------------------------------------------------------------
void        VideoConfForm::showEvent(QShowEvent* event)
{
    updateUI();
}

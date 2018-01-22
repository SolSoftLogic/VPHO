#include "MmsForm.h"
#include "Utils.h"
#include "Commands.h"
#include "LogEntry.h"
#include "Utils.h"

#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QTextEdit>
#include <QtGui/QCheckBox>
#include <QtGui/QPushButton>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtCore/QTime>
#include <QtCore/QDir>

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QtCore/QDebug>

#include <QtCore/QTemporaryFile>

//  MmsForm::MmsForm
//  -------------------------------------------------------------------------
MmsForm::MmsForm(QWidget *parent) 
    : CallHandlerWidget(parent)
{
    QWidget*    widget  = loadForm("qss/forms/mms.ui");

    QVBoxLayout*layout  = new QVBoxLayout;

    layout->addWidget(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    this->setLayout(layout);

    assign(this, m_username,    "mms_username");
    assign(this, m_vpnumber,    "mms_vpnumber");
    assign(this, m_video,       "mms_video");
    assign(this, m_videoin,     "mms_videoIn");

    assign(this, m_recStart,    "mms_recStart");
    assign(this, m_recStop,     "mms_recStop");
    assign(this, m_playStart,   "mms_playStart");
    assign(this, m_playStop,    "mms_playStop");
    assign(this, m_send,        "mms_send");
    assign(this, m_close,       "mms_close");

    assign(this, m_speed,       "mms_speed");   //m_speed->hide();
    assign(this, m_remain,      "mms_remain");  //m_remain->hide();

    assign(this, m_progress,    "mms_progress");

    connect(m_recStart, SIGNAL(clicked()),  this,   SLOT(onRecStartClicked()));
    connect(m_recStop,  SIGNAL(clicked()),  this,   SLOT(onRecStopClicked()));
    connect(m_playStart,SIGNAL(clicked()),  this,   SLOT(onPlayStartClicked()));
    connect(m_playStop, SIGNAL(clicked()),  this,   SLOT(onPlayStopClicked()));
    connect(m_close,    SIGNAL(clicked()),  this,   SLOT(onCloseClicked()));
    connect(m_send,     SIGNAL(clicked()),  this,   SLOT(onSendClicked()));

    m_aviplayer = NULL;
    m_playStart->setEnabled(false);
    m_recStop->hide();
    m_playStop->hide();
    m_send->setEnabled(false);

    m_progress->setValue(0);
    m_progress->setRange(0, 1000);

    vpMessageSubject()->addObserver(new VpMessageObserverFilterByBearer(
                                            this,
                                            (1 << BC_VIDEOMSG)));
    m_playTimer = new QTimer;
    connect(m_playTimer, SIGNAL(timeout()), this, SLOT(onPlayTimerTick()));
}


//  MmsForm::message
//  -------------------------------------------------------------------------
void    MmsForm::message(VpMessageSubject*   subject,
                    unsigned            msg,
                    unsigned            pcall,
                    unsigned            param)
{
//    onVpStackMessage(msg, (VPCALL)pcall, param);
}

//  MmsForm::onVpStackMessage
//  -------------------------------------------------------------------------
void    MmsForm::onVpStackMessage(
                        unsigned        msg,
                        VPCALL          call,
                        unsigned        param)
{
    switch(msg)
    {
        case    VPMSG_NEWCALL       :
        {
            m_call  =   call;

            unsigned int    count;
            unsigned int    nbytes;
            char            text[MAXTEXTLEN + 1];
            
            vpstack()->AnswerCall(call);
            vpstack()->GetCallNFiles(call, &count, &nbytes);
            vpstack()->GetCallText(call, text);

            m_send->setEnabled(false);
            m_recStart->setEnabled(false);
            showMe(this);
        } break;

        case    VPMSG_CALLACCEPTED  :
        {
            m_start.start();
            m_progress->setValue(0);

            vpstack()->SendFile(m_call, qPrintable(m_avifile));
        } break;

        case    VPMSG_CALLREFUSED   :
        {
        } break;

        case    VPMSG_SENDFILEFAILED:
        {
            emit mmsSent(false, m_party);
        } break;

        case    VPMSG_FTTXCOMPLETE   :
        {
            m_playStart->setEnabled(false);
            m_send->setEnabled(false);
            m_recStart->setEnabled(true);

            vpstack()->Disconnect(call, REASON_NORMAL);

            emit mmsSent(true, m_party);
        } break;

        case    VPMSG_SENDFILEREQ   :
        {
            char        file[MAX_PATH + 1];
            QStringList list;

            vpstack()->GetCallFilePath(call, file);

            list = QString(file).split(QRegExp("[/\\\\]"));
#ifdef HOME_PATH_SDCARD
            m_avifile  = QDir("/sdcard").path() + "/"PROJECTNAME"/Received video messages/" + list.back();
#else
            m_avifile  = QDir::homePath() + "/"PROJECTNAME"/Received video messages/" + list.back();
#endif
            vpstack()->SetCallFilePath(call, qPrintable(m_avifile));

            vpstack()->AcceptFile(call, 1);
            m_start.start();
        } break;

        case    VPMSG_FTPROGRESS    :
        {
            unsigned int    current;
            unsigned int    total;

            vpstack()->GetFileProgress(call, &current, &total);

            m_progress->setValue(current / m_filesize);

            int     elapsed = m_start.elapsed();

            if(elapsed != 0)
            {
                double  speed   = (double)current / ((double)elapsed / 1000);

                int     remain  = (double)(total - current) / speed;

                m_remain->setText(QString(tr("%1 s").arg(remain)));
            }

            m_speed->setText(getSpeed(current, elapsed));
        } break;

        case    VPMSG_FTRXCOMPLETE  :
        {
            setupPlayer();
        } break;
    }
}

//  MmsForm::call
//  -------------------------------------------------------------------------
void    MmsForm::call(const QString&  username)
{
    m_party     = username;

    m_username->setText(username);
    m_vpnumber->hide();

    m_recStart->setEnabled(true);
    m_recStart->show();
    m_recStop->hide();
    m_playStart->show();
    m_playStop->hide();

    showMe(this);

//    Q_ASSERT(m_inCall == false);

//    m_inCall    = true;
//    m_call      = vpstack()->CreateCall();
}

//  MmsForm::onShowMms
//  -------------------------------------------------------------------------
void    MmsForm::onShowMms(int id)
{
    QSqlQuery   query;

    query.exec(QString("UPDATE log SET type = type | %1 WHERE id = '%2'")
                .arg(logtRead)
                .arg(id));

    LOG_SQL_QUERY(query);
    query.exec(QString("SELECT file.file, log.username FROM file JOIN log ON log.id == file.owner WHERE log.id = '%1'")
                    .arg(id));
    LOG_SQL_QUERY(query);

    query.first();

    m_recStart->setEnabled(false);
    m_recStart->show();
    m_recStop->hide();

    setupPlayer();

    m_send->setEnabled(false);

    m_avifile   = query.record().value("file").toString();
    m_username->setText(query.record().value("username").toString());


    showMe(this);
}


//  MmsForm::onRecStartClicked
//  -------------------------------------------------------------------------
void    MmsForm::onRecStartClicked()
{
    QString     datetime    = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");

    QRect       qrect;
    RECTANGLE   rect;
#ifdef HOME_PATH_SDCARD
    m_avifile   = QDir("/sdcard").path() + "/"PROJECTNAME"/Sent video messages/" + datetime + ".avi";
#else
    m_avifile   = QDir::homePath() + "/"PROJECTNAME"/Sent video messages/" + datetime + ".avi";
#endif
#ifdef ANDROID_VERSION
    vpstack()->RecordAVIFile(qPrintable(m_avifile));
#endif
    VPCALL      call;
    vpstack()->GetRecorderCall(&call);

    qrect       = m_video->rect();
    rect.top    = qrect.top();
    rect.left   = qrect.left();
    rect.bottom = qrect.bottom();
    rect.right  = qrect.right();
#ifdef ANDROID_VERSION
    vpstack()->SetVideoWindowData(call,(void *) m_video->winId(), 'rcrd', &rect, false);
#else
    vpstack()->SetVideoWindowData(call, m_video->winId(), 'rcrd', &rect, false);
#endif
    m_recStart->hide();
    m_recStop->show();
    m_playStart->setEnabled(false);
}

//  MmsForm::onRecStopClicked
//  -------------------------------------------------------------------------
void    MmsForm::onRecStopClicked()
{
    vpstack()->StopAVIRecord();
    m_recStart->show();
    m_recStop->hide();

    m_send->setEnabled(true);

    setupPlayer();
}

//  MmsForm::setupPlayer
//  -------------------------------------------------------------------------
void    MmsForm::setupPlayer()
{
    if(m_aviplayer)
        delete  m_aviplayer;

    RECTANGLE   rect;
    QRect       qrect;
#ifdef ANDROID_VIDEO
    m_aviplayer = CreateAVIPLAYER();
#endif
    qrect       = m_video->rect();
    rect.top    = qrect.top();
    rect.left   = qrect.left();
    rect.bottom = qrect.bottom();
    rect.right  = qrect.right();
#ifdef ANDROID_VERSION
    m_aviplayer->SetWindowData((void *)m_video->winId(), &rect);
#else
    m_aviplayer->SetWindowData((HWND)m_video->winId(), &rect);
#endif
    m_aviplayer->Pause(1);
    m_aviplayer->PlayAVIFile(qPrintable(m_avifile));

    m_playStart->setEnabled(true);
    m_playStart->show();
    m_playStop->hide();

    m_progress->setValue(0);
}

//  MmsForm::onPlayStartClicked
//  -------------------------------------------------------------------------
void    MmsForm::onPlayStartClicked()
{
    m_aviplayer->Pause(0);

    m_playTimer->start(100);
    m_progress->setValue(0);

    m_playStart->hide();
    m_playStop->show();
    m_recStart->setEnabled(false);
}

//  MmsForm::onPlayTimerTick
//  -------------------------------------------------------------------------
void    MmsForm::onPlayTimerTick()
{
    int progress    = m_aviplayer->Progress();

    if(progress == -1)
    {
        onPlayStopClicked();
    }
    else
    {
        m_progress->setValue(progress);
    }
}

//  MmsForm::onPlayStopClicked
//  -------------------------------------------------------------------------
void    MmsForm::onPlayStopClicked()
{
    if(m_aviplayer)
    {
        m_aviplayer->Stop();
        delete m_aviplayer;
        m_aviplayer = 0;
    }
    
    setupPlayer();

    m_playTimer->stop();
}

//  MmsForm::onSendClicked
//  -------------------------------------------------------------------------
void    MmsForm::onSendClicked()
{
    QFile   avifile(m_avifile);
    avifile.open(QIODevice::ReadOnly);

    m_call      = vpstack()->CreateCall();
    m_filesize  = avifile.size();

    vpstack()->SetCallNFiles(m_call, 1, m_filesize);
    vpstack()->SetCallText(m_call, qPrintable(m_avifile));

    emit calling((int)m_call, qPrintable(m_party), BC_VIDEOMSG);
}

//  MmsForm::onCloseClicked
//  -------------------------------------------------------------------------
void    MmsForm::onCloseClicked()
{
    vpstack()->StopAVIRecord();

    if(m_aviplayer)
    {
        m_aviplayer->Stop();
        delete  m_aviplayer;
        m_aviplayer = 0;
    }

    m_remain->setText("");
    m_speed->setText("");
    m_playTimer->stop();

    hideMe(this);
}

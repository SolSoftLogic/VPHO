#include "FileTransferForm.h"
#include "Utils.h"
#include "Commands.h"

#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QLineEdit>
#include <QtGui/QTableWidget>
#include <QtGui/QFileDialog>
#include <QtGui/QHeaderView>
#include <QtCore/QUrl>

//  FileTransferForm::FileTransferForm
//  -------------------------------------------------------------------------
FileTransferForm::FileTransferForm(QWidget *parent)
    : CallHandlerWidget(parent)
    //, m_table
{
    QWidget*    widget  = loadForm("qss/forms/ftp.ui");

    QVBoxLayout*layout  = new QVBoxLayout;

    layout->addWidget(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    this->setLayout(layout);

    assign(this, m_table,   "ftp_table");
    assign(this, m_add,     "ftp_add");
    assign(this, m_del,     "ftp_del");
    assign(this, m_open,    "ftp_open");
    assign(this, m_send,    "ftp_send");
    assign(this, m_disc,    "ftp_disc");
    assign(this, m_hold,    "ftp_hold");
    assign(this, m_resume,  "ftp_resume");
    assign(this, m_current, "ftp_current");
    assign(this, m_total,   "ftp_total");
    assign(this, m_speed,   "ftp_speed");
    assign(this, m_username,"ftp_username");
    assign(this, m_coc,     "ftp_coc");

    connect(m_add,      SIGNAL(clicked()), this, SLOT(onAddClicked()));
    connect(m_del,      SIGNAL(clicked()), this, SLOT(onDelClicked()));
    connect(m_send,     SIGNAL(clicked()), this, SLOT(onSendClicked()));
    connect(m_disc,     SIGNAL(clicked()), this, SLOT(onDiscClicked()));
    connect(m_hold,     SIGNAL(clicked()), this, SLOT(onHoldClicked()));
    connect(m_resume,   SIGNAL(clicked()), this, SLOT(onResumeClicked()));

    m_table->setContentsMargins(0, 0, 0, 0);
    m_table->resizeColumnsToContents();
    m_table->horizontalHeader()->setResizeMode( 0, QHeaderView::Stretch );

    m_calltype  = NoCall;
}

//  FileTransferForm::onAddClicked
//  -------------------------------------------------------------------------
void        FileTransferForm::onAddClicked()
{
    setProperty("resident", true);
    sendCommand(commandSelectFile);
}

//  FileTransferForm::onDelClicked
//  -------------------------------------------------------------------------
void    FileTransferForm::onDelClicked()
{
    int     row = m_table->currentIndex().row();

    if(row != -1)
    {
        m_table->removeRow(row);
        m_files.removeAt(row);
    }

    m_send->setDisabled(m_files.isEmpty());
}

//  FileTransferForm::onHoldClicked
//  -------------------------------------------------------------------------
void    FileTransferForm::onHoldClicked()
{
    vpstack()->Hold(m_call);

    updateUI();
}

//  FileTransferForm::onResumeClicked
//  -------------------------------------------------------------------------
void    FileTransferForm::onResumeClicked()
{
    vpstack()->Resume(m_call);

    updateUI();
}

//  FileTransferForm::call
//  -------------------------------------------------------------------------
void    FileTransferForm::call(  const QString&  username)
{
    m_username->setText(username);

    m_hold->show();
    m_resume->hide();

    m_open->hide();
    m_disc->hide();

    showMe(this);

    for(int i = m_table->rowCount() - 1; i >= 0; --i)
        m_table->removeRow(i);

    m_files.clear();

    m_send->setDisabled(true);

    updateUI();

}

//  FileTransferForm::onNewCall
//  -------------------------------------------------------------------------
void    FileTransferForm::onNewCall(
                        VPCALL          call)
{
    if(m_calltype != NoCall)
    {
        vpstack()->Disconnect(call, REASON_BUSY);
        return;
    }

    char    username[MAXNAMELEN + 1];
    char    text[MAXTEXTLEN + 1];

    vpstack()->AnswerCall(call);

    for(int i = m_table->rowCount() - 1; i >= 0; --i)
        m_table->removeRow(i);

    vpstack()->GetCallRemoteName(call, username);
    m_username->setText(username);
    m_call  = call;

    unsigned int     count;
    unsigned int     nbytes;

    vpstack()->GetCallNFiles(call, &count, &nbytes);
    vpstack()->GetCallText(call, text);

    m_total->setRange(0, nbytes);

    m_time.start();

    m_receivedBytes = 0;
    m_cfile         = 0;

    showMe(this);

    m_calltype  = IncomingCall;

    updateUI();
}

//  FileTransferForm::onCallAccepted
//  -------------------------------------------------------------------------
void    FileTransferForm::onCallAccepted(
                        VPCALL          call)
{
    m_cfile     = 0;
    m_time.start();

    vpstack()->SendFile(call, qPrintable(m_files[m_cfile]));

    m_calltype  = OutgoingCall;

    updateUI();
}

//  FileTransferForm::onCallRefused
//  -------------------------------------------------------------------------
void    FileTransferForm::onCallRefused(
                        VPCALL          call,
                        int             reason)
{
    m_calltype  = NoCall;
}

//  FileTransferForm::onDiscClicked
//  -------------------------------------------------------------------------
void    FileTransferForm::onDiscClicked()
{
    vpstack()->Disconnect(m_call, REASON_NORMAL);

    m_calltype  = NoCall;

    updateUI();
}


//  FileTransferForm::onSendClicked
//  -------------------------------------------------------------------------
void    FileTransferForm::onSendClicked()
{
    int total   = 0;

    m_call  = vpstack()->CreateCall();
    m_send->hide();
    m_disc->show();

    QString path;
    for(int i = 0; i < m_files.count(); ++i)
    {
        QString filename    = m_files[i];

        path    += filename.split("/").last() + ":";

        QFile   file(filename);
        file.open(QIODevice::ReadOnly);

        total   += file.size();
    }

    m_receivedBytes = 0;
    m_total->setRange(0, total);

    vpstack()->SetCallNFiles(m_call, m_table->rowCount(), total);
    vpstack()->SetCallText(m_call, qPrintable(path));

    emit calling((int)m_call, qPrintable(m_username->text()), BC_FILE);
}

//  FileTransferForm::onVpStackMessage
//  -------------------------------------------------------------------------
void    FileTransferForm::onVpStackMessage(
                       unsigned        msg,
                       VPCALL          call,
                       unsigned        param)
{
    char    file[MAX_PATH + 1];

    switch(msg)
    {
        case    VPMSG_NEWCALL   :
        {
            setProperty("resident", true);
            onNewCall(call);
        } break;

        case    VPMSG_CALLENDED :
        {
            setProperty("resident", false);

            m_send->setDisabled(true);
            m_calltype  = NoCall;

            m_current->setValue(0);
            m_total->setValue(0);

            if(m_coc->isChecked())
            {
                hideMe(this);
            }

            updateUI();

        } break;

        //  VPMSG_CALLACCEPTED
        //  -----------------------------------------------------------------
        case    VPMSG_CALLACCEPTED  :
        {
            onCallAccepted(call);
        } break;

        //  VPMSG_CALLREFUSED
        //  -----------------------------------------------------------------
        case    VPMSG_CALLREFUSED   :
        {
            onCallRefused(call, param);
        } break;

        //  VPMSG_SENDFILEREQ
        //  -----------------------------------------------------------------
        case    VPMSG_SENDFILEREQ   :
        {
            vpstack()->GetCallFilePath(call, file);

            if(m_cfile <= m_table->rowCount())
            {
                m_table->insertRow(m_cfile);
                m_table->setCellWidget(m_cfile, 0, new QLabel(file));
            }

            m_table->setCellWidget(m_cfile, 1, new QLabel(tr("downloading")));
#ifdef HOME_PATH_SDCARD
            vpstack()->SetCallFilePath(call, qPrintable(QDir("/sdcard").path() + "/"PROJECTNAME"/Received Files/" + file));
#else
            vpstack()->SetCallFilePath(call, qPrintable(QDir::homePath() + "/"PROJECTNAME"/Received Files/" + file));
#endif
            vpstack()->AcceptFile(call, 1);
        } break;

        //  VPMSG_FTPROGRESS
        //  -----------------------------------------------------------------
        case    VPMSG_FTPROGRESS    :
        {
            unsigned int    current;
            unsigned int    total;

            vpstack()->GetFileProgress(call, &current, &total);

            m_current->setRange(0, total);
            m_current->setValue(current);
            m_total->setValue(m_receivedBytes + current);

            if(current == total)
            {
                m_receivedBytes += total;
            }

            m_speed->setText(getSpeed(m_receivedBytes + current,   m_time.elapsed()));

        } break;

        //  VPMSG_FTTXCOMPLETE
        //  -----------------------------------------------------------------
        case    VPMSG_FTTXCOMPLETE  :
        {
            ++m_cfile;

            if(m_cfile < m_files.size())
            {
                vpstack()->SendFile(call, qPrintable(m_files[m_cfile]));
            }

            if(m_cfile == m_files.size())
            {
                vpstack()->Disconnect(call, REASON_NORMAL);
            }
        } break;

        //  VPMSG_FTRXCOMPLETE
        //  -----------------------------------------------------------------
        case    VPMSG_FTRXCOMPLETE  :
        {
            ++m_cfile;

        } break;

        //  VPMSG_SENDFILESUCCESS
        //  -----------------------------------------------------------------
        case    VPMSG_SENDFILESUCCESS:
        {
            m_table->setCellWidget(m_cfile, 1, new QLabel(tr("success")));
        } break;

        case    VPMSG_SENDFILEFAILED    :
        {
            m_table->setCellWidget(m_cfile, 1, new QLabel(tr("failure")));
        } break;
    }
}

//  FileTransferForm::onFileSelected
//  -------------------------------------------------------------------------
void    FileTransferForm::onFileSelected(
                        const QString&  file)
{
    int     row     = m_table->rowCount();
    m_table->insertRow(row);

    m_table->setCellWidget(row, 0, new QLabel(file));
    m_table->setCellWidget(row, 1, new QLabel(tr("waiting")));

    m_files.append(file);

    updateUI();

    setProperty("resident", false);

    m_send->setDisabled(m_files.isEmpty());
}

//  FileTransferForm::showEvent
//  -------------------------------------------------------------------------
void    FileTransferForm::showEvent(  QShowEvent*     event)
{
    updateUI();
}

//  FileTransferForm::updateUI
//  -------------------------------------------------------------------------
void    FileTransferForm::updateUI()
{
    setUpdatesEnabled(false);

    m_add   ->setEnabled(m_calltype == NoCall);
    m_del   ->setEnabled(m_calltype == NoCall);
    m_open  ->setEnabled(m_calltype == NoCall);

    m_hold  ->setDisabled(m_calltype == NoCall);
    m_resume->setDisabled(m_calltype == NoCall);

    if(m_calltype != NoCall)
    {
        if(vpstack()->IsHeld(m_call))
        {
            m_hold->hide();
            m_resume->show();
        }
        else
        {
            m_hold->show();
            m_resume->hide();
        }
    }

    setUpdatesEnabled(true);
}

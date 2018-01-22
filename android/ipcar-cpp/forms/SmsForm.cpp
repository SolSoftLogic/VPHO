#include "SmsForm.h"
#include "Utils.h"
#include "Keyboard.h"
#include "Commands.h"
#include "LogEntry.h"

#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QTextEdit>
#include <QtGui/QCheckBox>
#include <QtGui/QPushButton>

#include <QtSql/QSqlQuery>

#define MAX_SMS_LENGTH  160

//  SmsForm::SmsForm
//  -------------------------------------------------------------------------
SmsForm::SmsForm(QWidget *parent)
    : CallHandlerWidget(parent)
{
    QWidget*    widget  = loadForm("qss/forms/sms.ui");

    QVBoxLayout*layout  = new QVBoxLayout;
    Keyboard*   keyboard= new Keyboard;

    layout->addWidget(widget);
    layout->addWidget(keyboard);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    this->setLayout(layout);

    assign(this, m_input,       "sms_input");
    assign(this, m_username,    "sms_username");
    assign(this, m_forward,     "sms_forward");
    assign(this, m_reply,       "sms_reply");
    assign(this, m_backcounter, "sms_backcounter");
    assign(this, m_tomobile,    "sms_tomobile");

    connect(keyboard,   SIGNAL(sendClicked()),  this,   SLOT(onSendClicked()));
    connect(m_forward,  SIGNAL(clicked()),      this,   SLOT(onForwardClicked()));
    connect(m_reply,    SIGNAL(clicked()),      this,   SLOT(onReplyClicked()));

    connect(m_input,    SIGNAL(textChanged()),  this,   SLOT(onTextChanged()));
    connect(m_tomobile, SIGNAL(toggled(bool)),  this,   SLOT(onToggled(bool)));

    m_backcounter->setReadOnly(true);
    m_backcounter->hide();

    updateUI();
}

//  SmsForm::onSendSms
//  -------------------------------------------------------------------------
void    SmsForm::call(const QString&  username)
{
    QString     mobile;
    QSqlQuery   query;

    query.exec(QString("SELECT contact.mobile FROM contact JOIN user ON contact.owner = user.id WHERE user.username = '%1' AND contact.username = '%2'")
                    .arg(getUsername())
                    .arg(username));
    LOG_SQL_QUERY(query);

    query.first();

    m_address   = username;
    m_mobile    = query.value(0).toString();

    m_username->setText(username);
    m_input->clear();
    m_reply->hide();
    m_forward->hide();

    m_tomobile->setChecked(false);
    m_tomobile->setEnabled(!m_mobile.isEmpty());

    showMe(this);
}

//  SmsForm::sendMessage
//  -------------------------------------------------------------------------
void    SmsForm::sendMessage(
                        const QString&  username,
                        const QString&  message)
{
    VPCALL  call;

    call    = vpstack()->CreateCall();

    if(call)
    {
        vpstack()->SetCallText(call, qPrintable(message));

        emit calling((int)call, qPrintable(username), BC_SMS);
    }
}

//  SmsForm::onSendClicked
//  -------------------------------------------------------------------------
void    SmsForm::onSendClicked()
{
    if(m_tomobile->checkState() == Qt::Checked)
    {
        sendMessage(m_mobile, m_input->toPlainText());
    }
    else
    {
        sendMessage(m_address, m_input->toPlainText());
    }

    m_input->clear();

    hideMe(this);
}

//  SmsForm::onShowSms
//  -------------------------------------------------------------------------
void    SmsForm::onShowSms(int id)
{
    QSqlQuery   query;
    QString     message;
    QString     username;

    int         type;

    m_currId    = id;

    query.exec(QString("UPDATE log SET type = type | %1 WHERE id = '%2'")
                .arg(logtRead)
                .arg(id));

    LOG_SQL_QUERY(query);

    query.exec(QString("SELECT message, username FROM log WHERE id = '%1'")
                    .arg(id));
    LOG_SQL_QUERY(query);

    query.first();

    message = query.value(0).toString();
    username= query.value(1).toString();

    m_username->setText(username);
    m_input->setText(message);

    m_reply->show();
    m_forward->show();

    updateUI();

    showMe(this);
}

//  SmsForm::onReplyClicked
//  -------------------------------------------------------------------------
void    SmsForm::onReplyClicked()
{
    m_input->clear();
    m_reply->hide();
    m_forward->hide();

    showMe(this);
}

//  SmsForm::onContactSelected
//  -------------------------------------------------------------------------
void    SmsForm::onContactSelected(
                        const QString&  username)
{
    QString     message;
    QSqlQuery   query;

    query.exec(QString("SELECT message FROM log WHERE id = '%1'")
                    .arg(m_currId));
    LOG_SQL_QUERY(query);

    query.first();

    message = query.value(0).toString();

    sendMessage(username, message);
}


//  SmsForm::onForwardClicked
//  -------------------------------------------------------------------------
void    SmsForm::onForwardClicked()
{
    m_username->setText("");
    m_reply->hide();
    m_forward->hide();

    emit    selectContact();
//    emit forward();

//    showMe(this);
}

//  SmsForm::onTextChanged
//  -------------------------------------------------------------------------
void    SmsForm::onTextChanged()
{
    updateUI();
}

//  SmsForm::onToggled
//  -------------------------------------------------------------------------
void    SmsForm::onToggled(bool state)
{
    updateUI();
}

//  SmsForm::onVpStackMessage
//  -------------------------------------------------------------------------
void    SmsForm::onVpStackMessage(   unsigned        msg,
                                    VPCALL          pcall,
                                    unsigned        param)
{
    switch(msg)
    {
        case    VPMSG_CALLACCEPTED  :
        {
            hideMe(this);
            showMessage(NULL, tr("Message has been send"));
        } break;

        case    VPMSG_CALLREFUSED  :
        {
            hideMe(this);
            showMessage(NULL, tr("Message was not sent"));
        } break;

        case    VPMSG_CALLENDED     :
        {
            hideMe(this);
        } break;
    }
}

//  SmsForm::updateUI
//  -------------------------------------------------------------------------
void    SmsForm::updateUI()
{
    if(m_tomobile->isChecked())
    {
        QString text    = m_input->toPlainText();

        if(m_backcounter->isHidden())
        {
            m_backcounter->show();
            m_username->setText(QString("%1").arg(m_mobile));
        }

        if(text.length() > MAX_SMS_LENGTH)
        {
            QTextCursor cursor;
            int         position;

            cursor  = m_input->textCursor();
            position= cursor.position();

            text.truncate(MAX_SMS_LENGTH);

            if(position > MAX_SMS_LENGTH)
                position    = MAX_SMS_LENGTH;

            cursor.setPosition(position);

            m_input->setPlainText(text);
            m_input->setTextCursor(cursor);
        }
    
        m_backcounter->setText(QString(tr("%1 left")).arg(MAX_SMS_LENGTH - text.length()));
    }
    else
    {
        if(m_backcounter->isVisible())
        {
            m_backcounter->hide();
            m_username->setText(m_address);
        }
    }
}

#include "ChatForm.h"
#include "Keyboard.h"
#include "Utils.h"
#include "Commands.h"

#include <QtGui/QGridLayout>
#include <QtGui/QScrollBar>
#include <QtCore/QTime>

ChatForm::ChatForm(QWidget *parent)
    : CallHandlerWidget(parent)
{
    QWidget*    widget  = loadForm("qss/forms/chat.ui");

    QVBoxLayout*layout  = new QVBoxLayout;
//#ifdef SOFTKEYBOARD
    Keyboard*   keyboard= new Keyboard;
//#endif
    layout->addWidget(widget);

    layout->addWidget(keyboard);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    this->setLayout(layout);

    assign(this, m_label,    "chat_username");
    assign(this, m_textEdit, "chat_text");
    assign(this, m_lineEdit, "chat_line");

    connect(m_lineEdit, SIGNAL(returnPressed()),this,   SLOT(onReturnPressed()));
    connect(keyboard,   SIGNAL(sendClicked()),  this,   SLOT(onSendClicked()));

    m_inCall    = false;
}

//  ChatForm::onSendClicked
//  -------------------------------------------------------------------------
void    ChatForm::onSendClicked()
{
    onReturnPressed();
}

//  ChatForm::onReturnPressed
//  -------------------------------------------------------------------------
void    ChatForm::onReturnPressed()
{
    if(m_inCall)
    {
        QString text;

        text    = m_lineEdit->text();

        vpstack()->SendChat(m_call, qPrintable(text));
        m_lineEdit->setText("");
    }
}

//  ChatForm::hideEvent
//  -------------------------------------------------------------------------
void    ChatForm::hideEvent( QHideEvent * event)
{
    if(m_inCall)
    {
        vpstack()->Disconnect(m_call, REASON_NORMAL);
        m_inCall    = false;
    }
}

//  ChatForm::showEvent
//  -------------------------------------------------------------------------
void    ChatForm::showEvent( QShowEvent * event)
{
    m_lineEdit->setFocus();
}

//  ChatForm::call
//  -------------------------------------------------------------------------
void    ChatForm::call(
                        const QString&      username)
{
    if(m_inCall)
        return;

    emit    calling(0, qPrintable(username), BC_CHAT);
}

//  ChatForm::onCallAccepted
//  -------------------------------------------------------------------------
void    ChatForm::onCallAccepted(
                        VPCALL           call)
{
    Q_ASSERT(m_inCall == false);

    m_inCall    = true;
    m_call      = call;

    char    name[MAXNAMELEN + 1];
    vpstack()->GetCallRemoteName(call, name);
    m_username  = name;
    m_label->setText(m_username);

    m_textEdit->append(tr("<font color=\"red\"><i>Connected<i></font>"));

    showMe(this);
}

//  ChatForm::onSendFileReq
//  -------------------------------------------------------------------------
void    ChatForm::onVpStackMessage(
                                unsigned        msg,
                                VPCALL          call,
                                unsigned        param)
{
    switch(msg)
    {
        case    VPMSG_NEWCALL   :
        {
            char    name[MAXNAMELEN + 1];

            Q_ASSERT(m_inCall == false);

            m_inCall    = true;
            m_call      = call;

            vpstack()->AnswerCall(call);
            vpstack()->GetCallRemoteName(call, name);
            m_username  = name;
            m_label->setText(m_username);

            m_textEdit->append(tr("<font color=\"red\"><i>Connected<i></font>"));

            showMe(this);
        } break;

        case    VPMSG_CALLACCEPTED  :
        {
            onCallAccepted(call);
        } break;

        case    VPMSG_CALLENDED:
        {
            m_textEdit->append(tr("<font color=\"red\">Disconnected</font>"));

            m_inCall    = false;

            hideMe(this);
        } break;

        case    VPMSG_CHAT  :
        {
            Q_ASSERT(m_inCall == true);
            Q_ASSERT(m_call == call);

            QTime   time    = QTime::currentTime();
            char    text[MAXTEXTLEN];
            vpstack()->GetChatText(call, text, sizeof(text));

            m_textEdit->append(QString("<font color=\"red\"><b>%1 (%2)</b></font>:")
                                .arg(m_username)
                                .arg(time.toString()));
            m_textEdit->append(text);
        } break;

        case    VPMSG_CHATACK  :
        {
            Q_ASSERT(m_inCall == true);
            Q_ASSERT(m_call == call);

            QTime   time    = QTime::currentTime();
            char    text[MAXTEXTLEN];
            vpstack()->GetChatText(call, text, sizeof(text));

            m_textEdit->append(QString("<font color=\"blue\"><b>%1 (%2)</b></font>:")
                                    .arg(getUsername())
                                    .arg(time.toString()));
            m_textEdit->append(text);
        } break;
    }
}

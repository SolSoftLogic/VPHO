#include "MessageForm.h"
#include "Utils.h"

#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>
#include <QtCore/QVariant>

#include "Commands.h"

#define OK_MESSAGE_TIMEOUT  5000

MessageForm::MessageForm(QWidget *parent)
    : QWidget(parent)
{
    QWidget*    widget  = loadForm("qss/forms/message.ui");

    QVBoxLayout*layout  = new QVBoxLayout;

    layout->addWidget(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    this->setLayout(layout);

    assign(this, m_label,   "message_text");

    assign(this, m_ok,      "message_ok");

    assign(this, m_yes,     "message_yes");
    assign(this, m_no,      "message_no");

    assign(this, m_accept,  "message_accept");
    assign(this, m_refuse,  "message_refuse");

    connect(m_ok,       SIGNAL(clicked()),  this,   SLOT(onOkClick()));
    connect(m_yes,      SIGNAL(clicked()),  this,   SLOT(onYesAcceptClick()));
    connect(m_no,       SIGNAL(clicked()),  this,   SLOT(onNoRefuseClick()));
    connect(m_accept,   SIGNAL(clicked()),  this,   SLOT(onYesAcceptClick()));
    connect(m_refuse,   SIGNAL(clicked()),  this,   SLOT(onNoRefuseClick()));

    connect(&m_timer,   SIGNAL(timeout()),  this,   SLOT(onTimerTimeout()));
}

//  MessageForm::setMessage
//  -------------------------------------------------------------------------
void        MessageForm::showMessage(
                        const QString&  message,
                        MessageType     type,
                        void*           userdata)
{
    m_yes->hide();
    m_no->hide();
    m_accept->hide();
    m_refuse->hide();
    m_ok->hide();

    m_userdata  = userdata;
    m_type      = type;

    m_timer.stop();

    switch(type)
    {
        case    MessageYesNo    :
        {
            m_yes->show();
            m_no->show();
        } break;

        case    MessageAcceptRefuse    :
        {
            m_accept->show();
            m_refuse->show();
        } break;

        case    MessageOk       :
        {
            m_timer.start(OK_MESSAGE_TIMEOUT);
            m_ok->show();
        } break;
    }

    m_label->setText(message);
}

//  -------------------------------------------------------------------------
void        MessageForm::onOkClick()
{
    m_timer.stop();
    emit result(m_type, true, m_userdata);
}
//  -------------------------------------------------------------------------
void        MessageForm::onYesAcceptClick()
{
    emit result(m_type, true, m_userdata);
}

//  -------------------------------------------------------------------------
void        MessageForm::onNoRefuseClick()
{
    emit result(m_type, false, m_userdata);
}

//  MessageForm::onTimerTimeout
//  -------------------------------------------------------------------------
void        MessageForm::onTimerTimeout()
{
    onOkClick();
}

//  MessageForm::hideEvent
//  -------------------------------------------------------------------------
void        MessageForm::hideEvent(QHideEvent *)
{
    m_timer.stop();
}

#include "ConnectingForm.h"
#include "Commands.h"
#include "Utils.h"

#include <QtGui/QVBoxLayout>

ConnectingForm::ConnectingForm(QWidget *parent)
    : QWidget(parent)
{
    QWidget*    widget  = loadForm("qss/forms/progress.ui");

    QVBoxLayout*layout  = new QVBoxLayout;

    layout->addWidget(widget);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    this->setLayout(layout);

    assign(this, m_bar,     "progress_bar");
    assign(this, m_text,    "progress_text");
    assign(this, m_user,    "progress_username");
    assign(this, m_cancel,  "progress_end");

    m_bar->setRange(0, 0);

    connect(&m_progressTimer, SIGNAL(timeout()), this, SLOT(onProgressTick()));
    connect(m_cancel, SIGNAL(clicked()), this, SLOT(onCancelClicked()));
}

//  ConnectingForm::onCancelClicked
//  -------------------------------------------------------------------------
void    ConnectingForm::onCancelClicked()
{
    emit cancel(m_call);
}

//  ConnectingForm::showEvent
//  -------------------------------------------------------------------------
void    ConnectingForm::showEvent(      QShowEvent* )
{
    m_progressTimer.start(50);
}

//  ConnectingForm::hideEvent
//  -------------------------------------------------------------------------
void    ConnectingForm::hideEvent(      QHideEvent* )
{
    m_progressTimer.stop();
    m_bar->reset();
}

//  ConnectingForm::onTick
//  -------------------------------------------------------------------------
void    ConnectingForm::onProgressTick()
{
    int value   = m_bar->value() + 1;

//    value      %= 32;

    m_bar->setValue(value);
}

//  ConnectingForm::onConnecting
//  -------------------------------------------------------------------------
void    ConnectingForm::onConnecting(   int         call)
{
    char    username[MAXNAMELEN + 1];
    vpstack()->GetCallRemoteName((VPCALL)call, username);

    m_user->setText(QString(tr("calling %1")).arg(username));
    m_call  = call;
}

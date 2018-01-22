#include "CallManagementForm.h"
#include "Utils.h"

#include <QtGui/QVBoxLayout>

//  CallManagementForm::CallManagementForm
//  -------------------------------------------------------------------------
CallManagementForm::CallManagementForm(QWidget *parent)
    : QWidget(parent)
{
    QWidget*    widget  = loadForm("qss/forms/settings/call.ui");

    QVBoxLayout*layout  = new QVBoxLayout;

    layout->addWidget(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    this->setLayout(layout);

    assign(this,    m_ring,         "callConf_ring");
    assign(this,    m_refuse,       "callConf_refuse");
    assign(this,    m_refuseText,   "callConf_refuseText");
    assign(this,    m_deflect,      "callConf_deflect");
    assign(this,    m_deflectText,  "callConf_deflectNumber");
    assign(this,    m_accept,       "callConf_accept");
    assign(this,    m_answer,       "callConf_answer");

    assign(this,    m_always,       "callConf_always");
    assign(this,    m_offline,      "callConf_offline");
    assign(this,    m_forwardTo,    "callConf_forwardTo");

    assign(this,    m_wait,         "callConf_wait");

    connect(m_ring,         SIGNAL(toggled(bool)),          this,   SLOT(onRadiobuttonToggle(bool)));
    connect(m_refuse,       SIGNAL(toggled(bool)),          this,   SLOT(onRadiobuttonToggle(bool)));
    connect(m_deflect,      SIGNAL(toggled(bool)),          this,   SLOT(onRadiobuttonToggle(bool)));
    connect(m_accept,       SIGNAL(toggled(bool)),          this,   SLOT(onRadiobuttonToggle(bool)));
    connect(m_answer,       SIGNAL(toggled(bool)),          this,   SLOT(onRadiobuttonToggle(bool)));

    connect(m_always,       SIGNAL(toggled(bool)),          this,   SLOT(onCheckboxToggle(bool)));
    connect(m_offline,      SIGNAL(toggled(bool)),          this,   SLOT(onCheckboxToggle(bool)));

    connect(m_refuseText,   SIGNAL(editingFinished()),      this,   SLOT(onRefuseTextChanged()));
    connect(m_deflectText,  SIGNAL(editingFinished()),      this,   SLOT(onDeflectNumberChanged()));
    connect(m_forwardTo,    SIGNAL(editingFinished()),      this,   SLOT(onForwardToTextChanged()));

    connect(m_wait,         SIGNAL(valueChanged(double)),   this,   SLOT(onWaitChanged(double)));
}

//  CallManagementForm::showEvent
//  -------------------------------------------------------------------------
void    CallManagementForm::showEvent(QShowEvent *)
{
    QString     username= getUsername();

    QString     action  = getUserData(username, "newcall/action");
    QString     text    = getUserData(username, "refuse/text");
    QString     number  = getUserData(username, "deflect/number");
    int         wait    = getUserData(username, "action/wait").toInt();

//    QString     oper    = getUserData(username, "forward/op");
//    QString     forward = getUserData(username, "forward/number");

    if(action == "refuse")
    {
        m_refuse->toggle();
    }
    else if(action == "deflect")
    {
        m_deflect->toggle();
    }
    else if(action == "accept")
    {
        m_accept->toggle();
    }
    else if(action == "answer")
    {
        m_answer->toggle();
    }
    else
    {
        m_ring->toggle();
    }

    m_refuseText->setText(text);
    m_deflectText->setText(number);

    char    forward[MAXNUMBERLEN + 1];

    vpstack()->GetCallForwardingStatus(&m_forwardOp, forward);

    m_forwardTo->setText(forward);

    if(m_forwardOp == OPERATION_ACTIVATE)
    {
        m_always->setChecked(true);
    }

    if(m_forwardOp == OPERATION_ACTIVATE_ONOFFLINE)
    {
        m_offline->setChecked(true);
    }

    m_wait->setValue(wait);
}

//  CallManagementForm::onRadiobuttonToggle
//  -------------------------------------------------------------------------
void    CallManagementForm::onRadiobuttonToggle(bool    state)
{
    QString     username= getUsername();

    if(state)
    {
        QObject*    widget  = sender();

        if(widget == m_ring)
        {
            ::setUserData(username, "newcall/action", "ring");
        }
        else if(widget == m_refuse)
        {
            ::setUserData(username, "newcall/action", "refuse");
        }
        else if(widget == m_deflect)
        {
            ::setUserData(username, "newcall/action", "deflect");
        }
        else if(widget == m_accept)
        {
            ::setUserData(username, "newcall/action", "accept");
        }
        else if(widget == m_answer)
        {
            ::setUserData(username, "newcall/action", "answer");
        }
    }
}

//  CallManagementForm::onRefuseTextChanged
//  -------------------------------------------------------------------------
void    CallManagementForm::onRefuseTextChanged()
{
    QString     username= getUsername();

    ::setUserData(username, "refuse/text", m_refuseText->text());
}

//  CallManagementForm::onDeflectNumberChanged
//  -------------------------------------------------------------------------
void    CallManagementForm::onDeflectNumberChanged()
{
    QString     username= getUsername();

    ::setUserData(username, "deflect/number", m_deflectText->text());
}

//  CallManagementForm::onForwardToTextChanged
//  -------------------------------------------------------------------------
void    CallManagementForm::onForwardToTextChanged()
{
    vpstack()->CallForwardingRequest(m_forwardOp, qPrintable(m_forwardTo->text()));
}

//  CallManagementForm::onWaitChanged
//  -------------------------------------------------------------------------
void    CallManagementForm::onWaitChanged(double)
{
    QString     username= getUsername();
    
    ::setUserData(username, "action/wait", m_wait->text());
}

//  CallManagementForm::onCheckboxToggle
//  -------------------------------------------------------------------------
void    CallManagementForm::onCheckboxToggle(   bool    toggle)
{
    if(toggle)
    {
        QObject*    widget  = sender();

        if(widget == m_always)
        {
            m_offline->setChecked(false);
            m_forwardOp = OPERATION_ACTIVATE;
        }
        else if(widget == m_offline)
        {
            m_always->setChecked(false);
            m_forwardOp = OPERATION_ACTIVATE_ONOFFLINE;
        }

        if(!m_forwardTo->text().isEmpty())
        {
            vpstack()->CallForwardingRequest(m_forwardOp, qPrintable(m_forwardTo->text()));
        }
    }
    else if(    m_always->isChecked()  == false
            &&  m_offline->isChecked() == false)
    {
        vpstack()->CallForwardingRequest(OPERATION_DEACTIVATE, qPrintable(m_forwardTo->text()));
    }
}

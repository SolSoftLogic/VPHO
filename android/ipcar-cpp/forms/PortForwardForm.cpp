#include "PortForwardForm.h"
#include <QtGui/QVBoxLayout>

//  PortForwardForm::PortForwardForm
//  -------------------------------------------------------------------------
PortForwardForm::PortForwardForm(QWidget* parent)
    : QWidget(parent)
{
    QWidget*    widget  = loadForm("qss/forms/portForward.ui");
    QVBoxLayout*layout  = new QVBoxLayout;

    layout->addWidget(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    this->setLayout(layout);

    assign(this,    m_check,    "portForward_Check");
    assign(this,    m_message,  "portForward_Message");
    assign(this,    m_port,     "portForward_Port");

    m_port->setReadOnly(true);

    vpMessageSubject()->addObserver(this);

    connect(m_check, SIGNAL(clicked()), this, SLOT(onCheckClicked()));
}

//  PortForwardForm::onCheckClicked
//  -------------------------------------------------------------------------
void    PortForwardForm::onCheckClicked()
{
    vpstack()->MeasureBandwidth();
    m_message->setText(tr("Network type is being descovered"));
    m_check->setDisabled(true);
}

//  PortForwardForm::message
//  -------------------------------------------------------------------------
void    PortForwardForm::message(
                    VpMessageSubject*   subject,
                    unsigned            msg,
                    unsigned            pcall,
                    unsigned            param)
{
    switch(msg)
    {
        //  VPMSG_NATTYPE
        //  -----------------------------------------------------------------
        case    VPMSG_NATTYPE   :
        {
            int     natType = param;
            QString message;

            switch(natType)
            {
                case    NAT_UNKNOWN     :
                    message = tr("Not logged on yet");
                    break;
                case    NAT_NONAT       :
                    message = tr("You are directly connected to the Internet with no NAT or firewall blocking anything. Your network is fully compatible with v-Phone.");
                    break;
                case    NAT_NONAT_PACKETLOSS  :
                    message = tr("You are directly connected to the Internet with no NAT, but some firewall is blocking packets. Your network is not fully compatible with v-Phone and you will probably experience trouble.");
                    break;
                case    NAT_FIREWALL    :
                    message = tr("You are having trouble with the network. No packets have been received from the network and you will not be able to use v-Phone.");
                    break;
                case    NAT_FULLCONE    :
                    message = tr("You are behind a full cone NAT or you have enabled port forwarding for v-Phone. Your network is fully compatible with v-Phone.");
                    break;
                case    NAT_RESTRICTEDCONE      :
                    message = tr("You are behind a restricted cone NAT. Your network is fully compatible with v-Phone.");
                    break;
                case    NAT_PORTRESTRICTEDCONE  :
                    message = tr("You are behind a port restricted cone NAT. Your network is not fully compatible with v-Phone. You will not be able to talk with people behind a symmetric NAT.");
                    break;
                case    NAT_SYMMETRIC   :
                    message = tr("You are behind a symmetric NAT. Your network is not fully compatible with v-Phone. You will not be able to talk with people behind a symmetric NAT like you and people behind a port restricted cone NAT.");
                    break;
                case    NAT_INCONSISTENT        :
                    message = tr("You are having trouble with the network. The results are inconsistent. Please retry this test in ten minutes.");
                    break;
                default:
                    message = "";
            }

            m_message->setText(message);
            m_port->setText(QString("%1").arg(vpstack()->GetBindPort()));
            m_check->setDisabled(false);
        } break;

        //  VPMSG_MEASUREBANDWITH
        //  -----------------------------------------------------------------
        case    VPMSG_MEASUREBANDWIDTH  :
        {
            if(param == 0)
            {
                m_message->setText(tr("I have no idea"));
            }
            m_port->setText(QString("%1").arg(vpstack()->GetBindPort()));
            m_check->setDisabled(false);
        } break;
    }
}

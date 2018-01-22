#include "VpStackAnsweringMachineDecorator.h"
#include "Utils.h"
#include "Commands.h"
#include <QtCore/QVariant>

//  VpStackAnsweringMachineDecorator::VpStackAnsweringMachineDecorator
//  -------------------------------------------------------------------------
VpStackAnsweringMachineDecorator::VpStackAnsweringMachineDecorator(
                    IVPSTACK*           base,
                    QObject*            parent)
    : VpStackDecorator(base, parent)
{
    m_base->CallForwardingRequest(OPERATION_ASK, NULL);
}

//  VpStackAnsweringMachineDecorator::onTimeout
//  -------------------------------------------------------------------------
void            VpStackAnsweringMachineDecorator::onTimeout()
{
    VPCALL      call    = (VPCALL)sender()->property("call").toUInt();
    QString     action  = sender()->property("action").toString();
    QString     username= m_base->LogonName();
    int         bc      = m_base->GetCallBearer(call);

    delete  m_call2timer[call];
    m_call2timer.erase(m_call2timer.find(call));

    if(action == "refuse")
    {
        QString text    = getUserData(username, "refuse/text");

        m_base->SetCallDisconnectMessage(call, qPrintable(text));
        m_base->Disconnect(call, REASON_NORMAL);

        m_calls.insert(call);
    }
    else if(action == "deflect")
    {
        QString     number  = getUserData(username, "deflect/number");
        ADDRESSTYPE type    = AddressType(qPrintable(number));

        if(type == ADDRT_VNUMBER || type == ADDRT_VNAME)
        {
            m_base->SetCallDisconnectMessage(call, qPrintable(number));
            m_base->Disconnect(call, REASON_CALLDEFLECTION);
        }
        else if(type == ADDRT_PSTN)
        {
            VPCALL  dst = m_base->CreateCall();

            m_deflect[call] = dst;
            m_calls.insert(dst);

            m_base->Connect(dst, qPrintable(number), bc);
            m_base->AnswerCall(call);
        }

        m_calls.insert(call);
    }
    else if(action == "accept")
    {
        sendCommand(commandAccept, QString("%1").arg((int)call));
    }
    else if(action == "answer")
    {
        // TODO:: ???
    }
}

//  VpStackAnsweringMachineDecorator::AnswerCall
//  -------------------------------------------------------------------------
int             VpStackAnsweringMachineDecorator::AnswerCall(VPCALL vpcall)
{
    if(m_call2timer.contains(vpcall))
    {
        delete  m_call2timer[vpcall];
        m_call2timer.erase(m_call2timer.find(vpcall));
    }

    return  m_base->AnswerCall(vpcall);
}

//  VpStackAnsweringMachineDecorator::Disconnect
//  -------------------------------------------------------------------------
int             VpStackAnsweringMachineDecorator::Disconnect(VPCALL vpcall, int reason)
{
    if(m_call2timer.contains(vpcall))
    {
        delete  m_call2timer[vpcall];
        m_call2timer.erase(m_call2timer.find(vpcall));
    }

    return  m_base->Disconnect(vpcall, reason);
}

//  VpStackAnsweringMachineDecorator::vpstackNotificationRoutine
//  -------------------------------------------------------------------------
void            VpStackAnsweringMachineDecorator::vpstackNotificationRoutine(
                            unsigned int        msg,
                            unsigned int        pcall,
                            unsigned int        param)
{
    VPCALL  call    = (VPCALL)pcall;
    int     bc      = m_base->GetCallBearer(call);

    if(msg == VPMSG_NEWCALL && (bc == BC_VOICE || bc == BC_VIDEO || bc == BC_AUDIOVIDEO || bc == BC_SMS))
    {
        QString     username= m_base->LogonName();
        QString     action  = getUserData(username, "newcall/action");

        if(action == "refuse" || action == "deflect" || action == "accept")
        {
            QString wait    = getUserData(username, "action/wait");

            QTimer*     timer   = new QTimer;
            m_call2timer[call]  = timer;

            connect(timer,  SIGNAL(timeout()), this, SLOT(onTimeout()));
            timer->start(wait.toInt() * 1000);
            timer->setProperty("call",  QVariant((int)call));
            timer->setProperty("action",QVariant(action));
        }

        DefNotificationRoutine(msg, pcall, param);
    }
    else if(msg == VPMSG_CALLESTABLISHED && m_deflect.contains(call))
    {
        VPCALL  calls[2];

        calls[0]    = call;
        calls[1]    = m_deflect[call];

        m_base->ConferenceCalls(calls, 2, true);

        m_deflect.remove(call);
    }
    else if(msg == VPMSG_CALLENDED && m_calls.contains(call))
    {
        //  this message is not propagated
        m_calls.remove(call);
    }
    else if(msg == VPMSG_CALLFORWARDING)
    {
        int         op;
        char        name[MAXNAMELEN + 1];
        QString     username= m_base->LogonName();
        m_base->GetCallForwardingStatus(&op, name);
    }
    else if(msg == VPMSG_QUERYONLINEACK)
    {
        DefNotificationRoutine(msg, pcall, param);
    }
    else if(msg == VPMSG_CALLENDED && m_call2timer.contains(call))
    {
        delete  m_call2timer[call];
        m_call2timer.erase(m_call2timer.find(call));

        DefNotificationRoutine(msg, pcall, param);
    }
    else 
    {
        if(!m_calls.contains(call))
        {
            DefNotificationRoutine(msg, pcall, param);
        }
    }
}

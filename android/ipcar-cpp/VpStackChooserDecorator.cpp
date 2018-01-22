#include "Utils.h"
#include <QtCore/QThread>
#include <QtCore/QPair>
#include "VpStackChooserDecorator.h"
#include <QtCore/QSettings>
#include <QtGui/qmessagebox.h>

#define Q_ASSERT(x)                                             \
    if(!(x))                                                    \
        QMessageBox::critical(                                  \
                        0,                                      \
                        QObject::tr(PROJECTNAME),                \
                        QString(QObject::tr("%1:%2 - %3 - %4")) \
                            .arg(__FILE__)                      \
                            .arg(__LINE__)                      \
                            .arg(__FUNCTION__)                  \
                            .arg(# x),                          \
                        QMessageBox::Ok);

QMutex          VpStackChooserDecorator::m_mutex;
QWaitCondition  VpStackChooserDecorator::m_done;
int             VpStackChooserDecorator::m_syncResult   = 0;


//  VpStackChooserDecorator::VpStackChooserDecorator
//  -------------------------------------------------------------------------
VpStackChooserDecorator::VpStackChooserDecorator(
                    IVPSTACK*           netStack,
                    IVPSTACK*           gsmStack,
                    QObject*            parent)
    : QObject(parent)
{
    m_call  = 0;

    Q_ASSERT(netStack && netStack->GetStackType() == STACKTYPE_VP);
    Q_ASSERT(gsmStack && gsmStack->GetStackType() == STACKTYPE_GSM);

    m_netStack      = netStack;
    m_gsmStack      = gsmStack;
    m_username[0]   = 0;
}


//  VpStackChooserDecorator::getStack
//  -------------------------------------------------------------------------
IVPSTACK*   VpStackChooserDecorator::getStack()
{
    char        name[128];
    unsigned    flags       = 0;

    name[0] = 0;

    m_gsmStack->GetOperatorName(name, sizeof(name), &flags);

    if(m_netStack->IsLoggedOn())
        return  m_netStack;
    else
        return  m_gsmStack;
}

//  VpStackChooserDecorator::getStack
//  -------------------------------------------------------------------------
IVPSTACK*   VpStackChooserDecorator::getStack(   VPCALL              gcall)
{
    if(m_g2l.contains((int)gcall))
    {
        return  m_g2l[(int)gcall].first;
    }

    return  m_netStack;
}

//  VpStackChooserDecorator::getCall
//  -------------------------------------------------------------------------
VPCALL      VpStackChooserDecorator::getCall(    VPCALL              gcall)
{
    return  (VPCALL)global2local((int)gcall);
}

int         VpStackChooserDecorator::Init()
{
    int a;
    int b;

    a   = m_netStack->Init();

    if(a = 0)
        return  a;

    b   = m_gsmStack->Init();

    return  b;
}

int         VpStackChooserDecorator::SetNotifyRoutine(
                    void                (*notify)(void *param, unsigned, unsigned, unsigned),
                    void*               param)
{
    m_notify        = notify;
    m_notifyParam   = param;

    m_netStack->SetNotifyRoutine(&VpStackChooserDecorator::netVpstackNotificationRoutine, static_cast<void*>(this));
    m_gsmStack->SetNotifyRoutine(&VpStackChooserDecorator::gsmVpstackNotificationRoutine, static_cast<void*>(this));

    return  0;
}

int         VpStackChooserDecorator::SetSyncNotifyRoutine(
                    int                 (*notify)(void *param, unsigned, unsigned, unsigned),
                    void*               param)
{
    m_sync      = notify;
    m_syncParam = param;

    m_netStack->SetSyncNotifyRoutine(&VpStackChooserDecorator::netVpstackSyncNotifyRoutine, static_cast<void*>(this));
    m_gsmStack->SetSyncNotifyRoutine(&VpStackChooserDecorator::gsmVpstackSyncNotifyRoutine, static_cast<void*>(this));

    return  0;
}


VPCALL      VpStackChooserDecorator::CreateCall()
{
    IVPSTACK*   stack   = getStack();
    int         local   = (int)stack->CreateCall();
    int         global;

    addCall(stack, local);
    global  = local2global(stack, local);

    return  (VPCALL)global;
}

void        VpStackChooserDecorator::FreeCall(
                    VPCALL              vpcall)
{
    getStack(vpcall)->FreeCall(getCall(vpcall));

    delCall((int)vpcall);
}

int         VpStackChooserDecorator::Connect(
                    VPCALL*             vpcall,
                    const char*         address,
                    int                 bc)
{
    IVPSTACK*   stack   = (     bc == BC_NOTHING 
                            || (    bc != BC_SMS 
                                &&  bc != BC_VOICE))
                        ? m_netStack 
                        : getStack();
    int         result;
    int         local;

    result  = stack->Connect(
                        (VPCALL*)&local,
                        address,
                        bc);

    if(result == 0)
    {
        addCall(stack, local);

        if(vpcall)
            *vpcall = (VPCALL)local2global(stack, local);
    }

    return  result;
}

int         VpStackChooserDecorator::Connect(
                    VPCALL              vpcall,
                    const char*         address,
                    int                 bc)
{
    return  getStack(vpcall)->Connect(
                        getCall(vpcall),
                        address,
                        bc);
}

int         VpStackChooserDecorator::SetCallAddress(
                    VPCALL              vpcall,
                    const char*         address)
{
    return  getStack(vpcall)->SetCallAddress(
                        getCall(vpcall),
                        address);
}

int         VpStackChooserDecorator::SetCallLocalName(
                    VPCALL              vpcall,
                    const char*         username)
{
    return  getStack(vpcall)->SetCallLocalName(
                        getCall(vpcall),
                        username);
}

int         VpStackChooserDecorator::SetCallLocalNumber(
                    VPCALL              vpcall,
                    const char*         number)
{
    return  getStack(vpcall)->SetCallLocalNumber(
                        getCall(vpcall),
                        number);
}

int         VpStackChooserDecorator::Disconnect(
                    VPCALL              vpcall,
                    int                 reason)
{
    return  getStack(vpcall)->Disconnect(
                        getCall(vpcall),
                        reason);
}

int         VpStackChooserDecorator::AnswerCall(
                    VPCALL              vpcall)
{
    return  getStack(vpcall)->AnswerCall(
                        getCall(vpcall));
}

int         VpStackChooserDecorator::Hold(
                    VPCALL              vpcall)
{
    return  getStack(vpcall)->Hold(
                        getCall(vpcall));
}

int         VpStackChooserDecorator::Resume(
                    VPCALL              vpcall)
{
    return  getStack(vpcall)->Resume(
                        getCall(vpcall));
}

int         VpStackChooserDecorator::ConferenceCalls(
                    VPCALL*             vpcalls,
                    int                 ncalls,
                    bool                detach)
{
    VPCALL* calls   = new VPCALL[ncalls];
    int     count   = 0;

    for(int i = 0; i < ncalls; ++i)
    {
        QPair<IVPSTACK*, int>   pair    = m_g2l[(int)vpcalls[i]];

        if(pair.first == m_netStack)
        {
            calls[count++]  = (VPCALL)pair.second;
        }
    }

    int result  = 0;

    if(count != 0)
    {

        result  = m_netStack->ConferenceCalls(calls, count, detach);

        delete [] calls;
    }

    return  result;
}

int         VpStackChooserDecorator::AddToConference(
                    VPCALL              vpcall)
{
    return getStack(vpcall)->AddToConference(
                        getCall(vpcall));
}

int         VpStackChooserDecorator::RemoveFromConference(
                    VPCALL              vpcall)
{
    return getStack(vpcall)->RemoveFromConference(
                        getCall(vpcall));
}

int         VpStackChooserDecorator::ConferenceAll()
{
    return m_netStack->ConferenceAll();
}

int         VpStackChooserDecorator::StopConference()
{
    return m_netStack->StopConference();
}

int         VpStackChooserDecorator::SendAudioMessage(
                    VPCALL              vpcall,
                    const char*         file,
                    unsigned            msgid)
{
    return  getStack(vpcall)->SendAudioMessage(
                        getCall(vpcall),
                        file,
                        msgid);
}

int         VpStackChooserDecorator::SetServers(
                    const char*         list)
{
    QSettings       settings;
    QString         comports;
    char            opname[128];
    unsigned int    flags;

    comports    = QString("\\\\.\\%1;\\\\.\\%2")
                      .arg(settings.value("network/cport").toString())
                      .arg(settings.value("network/aport").toString());

    m_netStack->SetServers(list);
    m_gsmStack->SetServers(qPrintable(comports));


    m_gsmStack->GetOperatorName(opname, sizeof(opname), &flags);

    return  0;
}

int         VpStackChooserDecorator::Logon(
                    const char*         username,
                    const char*         password)
{
    if(username != NULL)
    {
        m_netStack->Logon(username, password);
        strncpy(m_username, username, MAXNAMELEN);
    }
    else
        m_gsmStack->Logon(NULL,     password);

    return  0;
}

int         VpStackChooserDecorator::GetLogonData(
                    char*               serveraccesscode,
                    unsigned char*      accountsecret)
{
    serveraccesscode[0] = 0;
    accountsecret[0] = 0;
    return  m_netStack->GetLogonData(serveraccesscode, accountsecret);
}

int         VpStackChooserDecorator::Logon(
                    const char*         serveraccesscode,
                    const unsigned char*accountsecret)
{
    m_netStack->Logon(serveraccesscode, accountsecret);
//    m_gsmStack->Logon(serveraccesscode, accountsecret);

    return  0;
}

int         VpStackChooserDecorator::Logoff()
{
    return  m_netStack->Logoff();
}

int         VpStackChooserDecorator::IsLoggedOn()
{
    return  m_netStack->IsLoggedOn();
}

const char* VpStackChooserDecorator::LogonName()
{
    if(m_username[0] == 0)
    {
        char const* name    = m_netStack->LogonName();

        strncpy(m_username, name, MAXNAMELEN);
    }

    return  m_username;
}

const char* VpStackChooserDecorator::LogonNumber()
{
    return  m_netStack->LogonNumber();
}

void        VpStackChooserDecorator::SetSupportedBearersMask(
                    unsigned            bc_mask)
{
    m_netStack->SetSupportedBearersMask(bc_mask);
//    m_gsmStack->SetSupportedBearersMask(bc_mask & ((1 << BC_SMS) | (1 << BC_VOICE)));
}

void        VpStackChooserDecorator::SetAudioDataFactory(
                    VPAUDIODATAFACTORY* af)
{
    m_netStack->SetAudioDataFactory(af);
    m_gsmStack->SetAudioDataFactory(af);
}

void        VpStackChooserDecorator::SetVideoDataFactory(
                    VPVIDEODATAFACTORY* vf)
{
    m_netStack->SetVideoDataFactory(vf);
    m_gsmStack->SetVideoDataFactory(vf);
}

int         VpStackChooserDecorator::SendKeypad(
                    VPCALL              vpcall,
                    int                 key)
{
    return  getStack(vpcall)->SendKeypad(
                        getCall(vpcall),
                        key);
}

int         VpStackChooserDecorator::GetStackType()
{
    return  getStack()->GetStackType();
}

int         VpStackChooserDecorator::EnumCalls(
                    VPCALL*             vpcalls,
                    unsigned            maxvpcalls,
                    unsigned            mask)
{
    int countA;
    int countB;

    countA  = m_netStack->EnumCalls(vpcalls, maxvpcalls, mask);
    for(int i = 0; i < countA && i < maxvpcalls; ++i)
    {
        *vpcalls    = (VPCALL)local2global(m_netStack, (int)*vpcalls);
        ++vpcalls;
        --maxvpcalls;
    }

    countB  = m_gsmStack->EnumCalls(vpcalls, maxvpcalls - countA, mask);
    for(int i = 0; i < countB && i < maxvpcalls; ++i)
    {
        *vpcalls    = (VPCALL)local2global(m_gsmStack, (int)*vpcalls);
        ++vpcalls;
        --maxvpcalls;
    }

    return  countA + countB;
}

int         VpStackChooserDecorator::NetworkQuality()
{
    return  m_gsmStack->NetworkQuality();
}

int         VpStackChooserDecorator::GetQosData(
                    VPCALL              vpcall,
                    QOSDATA*            qos)
{
    return  getStack(vpcall)->GetQosData(
                        getCall(vpcall),
                        qos);
}

int         VpStackChooserDecorator::GetAudioParameters(
                    VPCALL              vpcall,
                    int*                codec,
                    int*                framesperpacket)
{
    return  getStack(vpcall)->GetAudioParameters(
                        getCall(vpcall),
                        codec,
                        framesperpacket);
}

int         VpStackChooserDecorator::SetCodecsMask(
                    unsigned            mask)
{
    m_gsmStack->SetCodecsMask(mask);
    m_netStack->SetCodecsMask(mask);

    return  0;
}

char*       VpStackChooserDecorator::StackName()
{
    static
    char const* name    = "+++";

    return  const_cast<char*>(name);
}

void        VpStackChooserDecorator::SetBindPort(
                    int                 port)
{
    m_netStack->SetBindPort(port);
}

int         VpStackChooserDecorator::GetBindPort()
{
    return  m_netStack->GetBindPort();
}

void        VpStackChooserDecorator::SetBandwidth(
                    int                 kbps)
{
    m_netStack->SetBandwidth(kbps);
    m_gsmStack->SetBandwidth(kbps);
}

int         VpStackChooserDecorator::GetCallStatus(
                    VPCALL              vpcall)
{
    return  getStack(vpcall)->GetCallStatus(
                        getCall(vpcall));
}

int         VpStackChooserDecorator::GetTransceiverBandwidths(
                    int*                rx,
                    int*                tx)
{
    return  m_gsmStack->GetTransceiverBandwidths(
                        rx,
                        tx);
}

int         VpStackChooserDecorator::GetConferencePeers(
                    VPCALL              vpcall,
                    VPCALL*             vpcalls,
                    int*                ncalls)
{
    return  getStack(vpcall)->GetConferencePeers(
                        getCall(vpcall),
                        vpcalls,
                        ncalls);
}

int         VpStackChooserDecorator::GetLinkedCall(
                    VPCALL              vpcall,
                    VPCALL*             linkedcall)
{
    return  getStack(vpcall)->GetLinkedCall(
                        getCall(vpcall),
                        linkedcall);
}

int         VpStackChooserDecorator::GetCallRemoteName(
                    VPCALL              vpcall,
                    char*               username)
{
    username[0] = 0;
    return  getStack(vpcall)->GetCallRemoteName(
                        getCall(vpcall),
                        username);
}

int         VpStackChooserDecorator::GetCallRemoteNumber(
                    VPCALL              vpcall,
                    char*               number)
{
    number[0] = 0;
    return  getStack(vpcall)->GetCallRemoteNumber(
                        getCall(vpcall),
                        number);
}

int         VpStackChooserDecorator::GetCallRemoteAddress(
                    VPCALL              vpcall,
                    char*               address)
{
    address[0] = 0;
    return  getStack(vpcall)->GetCallRemoteAddress(
                        getCall(vpcall),
                        address);
}

int         VpStackChooserDecorator::GetCallRemoteSubAddr(
                    VPCALL              vpcall,
                    char*               subaddr)
{
    subaddr[0] = 0;
    return  getStack(vpcall)->GetCallRemoteSubAddr(
                        getCall(vpcall),
                        subaddr);
}

int         VpStackChooserDecorator::GetCallLocalName(
                    VPCALL              vpcall,
                    char*               username)
{
    username[0] = 0;
    return  getStack(vpcall)->GetCallLocalName(
                        getCall(vpcall),
                        username);
}

int         VpStackChooserDecorator::GetCallLocalNumber(
                    VPCALL              vpcall,
                    char*               number)
{
    number[0] = 0;
    return  getStack(vpcall)->GetCallLocalNumber(
                        getCall(vpcall),
                        number);
}

int         VpStackChooserDecorator::GetCallLocalSubAddr(
                    VPCALL              vpcall,
                    char*               subaddr)
{
    subaddr[0] = 0;
    return  getStack(vpcall)->GetCallLocalSubAddr(
                        getCall(vpcall),
                        subaddr);
}

int         VpStackChooserDecorator::GetCallBearer(
                    VPCALL              vpcall)
{
    if(m_g2l.contains((int)vpcall))
        return  getStack(vpcall)->GetCallBearer(
                            getCall(vpcall));

    return  -1; //  this is not a call
}

int         VpStackChooserDecorator::GetCallCodec(
                    VPCALL              vpcall)
{
    return  getStack(vpcall)->GetCallCodec(
                        getCall(vpcall));
}

int         VpStackChooserDecorator::GetCallTimes(
                    VPCALL              vpcall,
                    time_t*             start_time,
                    unsigned*           length_ms)
{
    return  getStack(vpcall)->GetCallTimes(
                        getCall(vpcall),
                        start_time,
                        length_ms);
}

int         VpStackChooserDecorator::IsHeld(
                    VPCALL              vpcall)
{
    return  getStack(vpcall)->IsHeld(
                        getCall(vpcall));
}

int         VpStackChooserDecorator::IsConferenced(
                    VPCALL              vpcall)
{
    return  getStack(vpcall)->IsConferenced(
                        getCall(vpcall));
}


int         VpStackChooserDecorator::SetCallDisconnectMessage(
                    VPCALL              vpcall,
                    const char*         text)
{
    return getStack(vpcall)->SetCallDisconnectMessage(
                        getCall(vpcall),
                        text);
}
int         VpStackChooserDecorator::CallForwardingRequest(
                    int                 op,
                    const char*         number)
{
    return  m_netStack->CallForwardingRequest(op, number);
}

int         VpStackChooserDecorator::GetCallForwardingStatus(
                    int*                op,
                    char*               number)
{
    number[0] = 0;
    return  m_netStack->GetCallForwardingStatus(op, number);
}

int         VpStackChooserDecorator::MeasureBandwidth()
{
    return  m_netStack->MeasureBandwidth();
//    return  m_gsmStack->MeasureBandwidth();
}

int         VpStackChooserDecorator::SendChat(
                    VPCALL              vpcall,
                    const char*         text)
{
    return  getStack(vpcall)->SendChat(
                        getCall(vpcall),
                        text);
}

int         VpStackChooserDecorator::GetChatText(
                    VPCALL              vpcall,
                    char*               text,
                    int                 textsize)
{
    text[0] = 0;
    return  getStack(vpcall)->GetChatText(
                        getCall(vpcall),
                        text,
                        textsize);
}

// SMS data
int         VpStackChooserDecorator::GetMissedCallsData(
                    VPCALL              vpcall,
                    unsigned*           missedcallscount,
                    unsigned*           missedcalltime,
                    unsigned*           missedcallbc)
{
    return  getStack(vpcall)->GetMissedCallsData(
                        getCall(vpcall),
                        missedcallscount,
                        missedcalltime,
                        missedcallbc);
}

int         VpStackChooserDecorator::GetCallText(
                    VPCALL              vpcall,
                    char*               text)
{
    text[0] = 0;
    return  getStack(vpcall)->GetCallText(
                        getCall(vpcall),
                        text);
}

int         VpStackChooserDecorator::SetCallText(
                    VPCALL              vpcall,
                    const char*         text)
{
    return  getStack(vpcall)->SetCallText(
                        getCall(vpcall),
                        text);
}

int         VpStackChooserDecorator::SetMissedCallBC(
                    VPCALL              vpcall,
                    int                 bc)
{
    return  getStack(vpcall)->SetMissedCallBC(
                        getCall(vpcall),
                        bc);
}

int         VpStackChooserDecorator::SendAudio(
                    VPCALL              vpcall,
                    int                 codec,
                    unsigned            timestamp,
                    const void*         buf,
                    int                 len,
                    bool                wait)
{
    return  getStack(vpcall)->SendAudio(
                        getCall(vpcall),
                        codec,
                        timestamp,
                        buf,
                        len,
                        wait);
}

int         VpStackChooserDecorator::SendVideo(
                    VPCALL              vpcall,
                    unsigned short      timestamp,
                    void*               data,
                    int                 size,
                    bool                keyframe)
{
    return  getStack(vpcall)->SendVideo(
                        getCall(vpcall),
                        timestamp,
                        data,
                        size,
                        keyframe);
}

int         VpStackChooserDecorator::AskOnline(
                    AOL*                aol)
{
    return  m_netStack->AskOnline(aol);
}

int         VpStackChooserDecorator::QueryAccountInfo()
{
    return  m_netStack->QueryAccountInfo();
}

int         VpStackChooserDecorator::ServerTransfer(
                    int                 op,
                    const char*         path,
                    bool                amactive)
{
    return  m_netStack->ServerTransfer(op, path, amactive);
}


int         VpStackChooserDecorator::SendFile(
                    VPCALL              vpcall,
                    const char*         path)
{
    return  getStack(vpcall)->SendFile(
                        getCall(vpcall),
                        path);
}

int         VpStackChooserDecorator::AbortFileTransfer(
                    VPCALL              vpcall)
{
    return  getStack(vpcall)->AbortFileTransfer(
                        getCall(vpcall));
}

int         VpStackChooserDecorator::AcceptFile(
                    VPCALL              vpcall,
                    int                 accept)
{
    return  getStack(vpcall)->AcceptFile(
                        getCall(vpcall),
                        accept);
}

int         VpStackChooserDecorator::GetCallFilePath(
                    VPCALL              vpcall,
                    char*               path)
{
    path[0] = 0;
    return  getStack(vpcall)->GetCallFilePath(
                        getCall(vpcall),
                        path);
}

int         VpStackChooserDecorator::SetCallFilePath(
                    VPCALL              vpcall,
                    const char*         path)
{
    return  getStack(vpcall)->SetCallFilePath(
                        getCall(vpcall),
                        path);
}

int         VpStackChooserDecorator::SetCallNFiles(
                    VPCALL              vpcall,
                    unsigned            nfiles,
                    unsigned            nbytes)
{
    return  getStack(vpcall)->SetCallNFiles(
                        getCall(vpcall),
                        nfiles,
                        nbytes);
}

int         VpStackChooserDecorator::GetCallNFiles(
                    VPCALL              vpcall,
                    unsigned*           nfiles,
                    unsigned*           nbytes)
{
    return  getStack(vpcall)->GetCallNFiles(
                        getCall(vpcall),
                        nfiles,
                        nbytes);
}

int         VpStackChooserDecorator::GetFileProgress(
                    VPCALL              vpcall,
                    unsigned*           current,
                    unsigned*           total)
{
    return  getStack(vpcall)->GetFileProgress(
                        getCall(vpcall),
                        current,
                        total);
}

//  VpStackChooserDecorator::netVpstackNotificationRoutine
//  -------------------------------------------------------------------------
void        VpStackChooserDecorator::netVpstackNotificationRoutine(
                            void*               uparam,
                            unsigned            msg,
                            unsigned            pcall,
                            unsigned            param)
{
    VpStackChooserDecorator*   self    = static_cast<VpStackChooserDecorator*>(uparam);

    if(self->thread() != QThread::currentThread())
    {
        QMetaObject::invokeMethod(
                        self,
                        "netVpstackNotificationRoutineSlot",
                        Qt::QueuedConnection,
                        Q_ARG(uint, msg),
                        Q_ARG(uint, pcall),
                        Q_ARG(uint, param));
    }
    else
    {
        self->netVpstackNotificationRoutine(msg, pcall, param);
    }
}

//  VpStackChooserDecorator::gsmVpstackNotificationRoutine
//  -------------------------------------------------------------------------
void        VpStackChooserDecorator::gsmVpstackNotificationRoutine(
                            void*               uparam,
                            unsigned            msg,
                            unsigned            pcall,
                            unsigned            param)
{
    VpStackChooserDecorator*   self    = static_cast<VpStackChooserDecorator*>(uparam);

    if(self->thread() != QThread::currentThread())
    {
        QMetaObject::invokeMethod(
                        self,
                        "gsmVpstackNotificationRoutineSlot",
                        Qt::QueuedConnection,
                        Q_ARG(uint, msg),
                        Q_ARG(uint, pcall),
                        Q_ARG(uint, param));
    }
    else
    {
        self->gsmVpstackNotificationRoutine(msg, pcall, param);
    }
}


//  VpStackChooserDecorator::netVpstackSyncNotifyRoutine
//  -------------------------------------------------------------------------
int         VpStackChooserDecorator::netVpstackSyncNotifyRoutine(
                            void*               uparam,
                            unsigned            msg,
                            unsigned            param1,
                            unsigned            param2)
{
    VpStackChooserDecorator*   self    = static_cast<VpStackChooserDecorator*>(uparam);
    if(self->thread() != QThread::currentThread())
    {
        m_mutex.lock();

        QMetaObject::invokeMethod(
                        self,
                        "netVpstackSyncNotifyRoutineSlot",
                        Qt::QueuedConnection,
                        Q_ARG(uint, msg),
                        Q_ARG(uint, param1),
                        Q_ARG(uint, param2));

        m_done.wait(&m_mutex);
        m_mutex.unlock();
        return  m_syncResult;
    }
    else
    {
        return  self->netVpstackSyncNotifyRoutine(msg, param1, param2);
    }
}

//  VpStackChooserDecorator::gsmVpstackSyncNotifyRoutine
//  -------------------------------------------------------------------------
int         VpStackChooserDecorator::gsmVpstackSyncNotifyRoutine(
                            void*               uparam,
                            unsigned            msg,
                            unsigned            param1,
                            unsigned            param2)
{
    VpStackChooserDecorator*   self    = static_cast<VpStackChooserDecorator*>(uparam);
    if(self->thread() != QThread::currentThread())
    {
        m_mutex.lock();

        QMetaObject::invokeMethod(
                        self,
                        "gsmVpstackSyncNotifyRoutineSlot",
                        Qt::QueuedConnection,
                        Q_ARG(uint, msg),
                        Q_ARG(uint, param1),
                        Q_ARG(uint, param2));

        m_done.wait(&m_mutex);
        m_mutex.unlock();
        return  m_syncResult;
    }
    else
    {
        return  self->gsmVpstackSyncNotifyRoutine(msg, param1, param2);
    }
}

//  VpStackChooserDecorator::netVpstackNotificationRoutineSlot
//  -------------------------------------------------------------------------
void        VpStackChooserDecorator::netVpstackNotificationRoutineSlot(
                            unsigned            msg,
                            unsigned            pcall,
                            unsigned            param)
{
    netVpstackNotificationRoutine(msg, pcall, param);
}

//  VpStackChooserDecorator::gsmVpstackNotificationRoutineSlot
//  -------------------------------------------------------------------------
void        VpStackChooserDecorator::gsmVpstackNotificationRoutineSlot(
                            unsigned            msg,
                            unsigned            pcall,
                            unsigned            param)
{
    gsmVpstackNotificationRoutine(msg, pcall, param);
}

//  VpStackChooserDecorator::netVpstackSyncNotifyRoutineSlot
//  -------------------------------------------------------------------------
void        VpStackChooserDecorator::netVpstackSyncNotifyRoutineSlot(
                            unsigned            msg,
                            unsigned            param1,
                            unsigned            param2)
{
    m_syncResult    = netVpstackSyncNotifyRoutine(msg, param1, param2);
    m_done.wakeAll();
}

//  VpStackChooserDecorator::gsmVpstackSyncNotifyRoutineSlot
//  -------------------------------------------------------------------------
void        VpStackChooserDecorator::gsmVpstackSyncNotifyRoutineSlot(
                            unsigned            msg,
                            unsigned            param1,
                            unsigned            param2)
{
    m_syncResult    = gsmVpstackSyncNotifyRoutine(msg, param1, param2);
    m_done.wakeAll();
}

//  VpStackChooserDecorator::netVpstackNotificationRoutine
//  -------------------------------------------------------------------------
void        VpStackChooserDecorator::netVpstackNotificationRoutine(
                            unsigned            msg,
                            unsigned            pcall,
                            unsigned            param)
{
    netDefNotificationRoutine(msg, pcall, param);
}

//  VpStackChooserDecorator::gsmVpstackNotificationRoutine
//  -------------------------------------------------------------------------
void        VpStackChooserDecorator::gsmVpstackNotificationRoutine(
                            unsigned            msg,
                            unsigned            pcall,
                            unsigned            param)
{
    gsmDefNotificationRoutine(msg, pcall, param);
}

//  VpStackChooserDecorator::netvpstackSyncNotifyRoutine
//  -------------------------------------------------------------------------
int             VpStackChooserDecorator::netVpstackSyncNotifyRoutine(
                            unsigned            msg,
                            unsigned            param1,
                            unsigned            param2)
{
    return  netDefSyncNotifyRoutine(msg, param1, param2);
}

//  VpStackChooserDecorator::gsmvpstackSyncNotifyRoutine
//  -------------------------------------------------------------------------
int             VpStackChooserDecorator::gsmVpstackSyncNotifyRoutine(
                            unsigned            msg,
                            unsigned            param1,
                            unsigned            param2)
{
    return  gsmDefSyncNotifyRoutine(msg, param1, param2);
}


//  VpStackChooserDecorator::netDefNotificationRoutine
//  -------------------------------------------------------------------------
void            VpStackChooserDecorator::netDefNotificationRoutine(
                            unsigned int        msg,
                            unsigned int        pcall,
                            unsigned int        param)
{
    if(m_notify)
    {
        if(msg == VPMSG_NEWCALL)
        {
            addCall(m_netStack, pcall);
        }

        if(msg >= VPMSG_INVALIDADDR && msg <= VPMSG_REQUESTREFUSED)
        {
            pcall   = local2global(m_netStack, (int)pcall);
        }

        m_notify(m_notifyParam, msg, pcall, param);
    }
}

//  VpStackChooserDecorator::gsmDefNotificationRoutine
//  -------------------------------------------------------------------------
void            VpStackChooserDecorator::gsmDefNotificationRoutine(
                            unsigned int        msg,
                            unsigned int        pcall,
                            unsigned int        param)
{
    if(m_notify)
    {
        if(msg == VPMSG_NEWCALL)
        {
            addCall(m_gsmStack, pcall);
        }

        if(msg >= VPMSG_INVALIDADDR && msg <= VPMSG_REQUESTREFUSED)
        {
            pcall   = local2global(m_gsmStack, (int)pcall);
        }

        m_notify(m_notifyParam, msg, pcall, param);
    }
}

//  VpStackChooserDecorator::netDefSyncNotifyRoutine
//  -------------------------------------------------------------------------
int            VpStackChooserDecorator::netDefSyncNotifyRoutine(
                            unsigned int        msg,
                            unsigned int        pcall,
                            unsigned int        param)
{
    if(m_sync)
    {
        return  m_sync(m_syncParam, msg, pcall, param);
    }

    return  0;
}

//  VpStackChooserDecorator::gsmDefSyncNotifyRoutine
//  -------------------------------------------------------------------------
int            VpStackChooserDecorator::gsmDefSyncNotifyRoutine(
                            unsigned int        msg,
                            unsigned int        pcall,
                            unsigned int        param)
{
    if(m_sync)
    {
        return  m_sync(m_syncParam, msg, pcall, param);
    }

    return  0;
}

int         VpStackChooserDecorator::SetDefaultVideoParameters(
                    unsigned            fourcc,
                    unsigned            width,
                    unsigned            height,
                    unsigned            framerate,
                    unsigned            quality)
{
    return  m_netStack->SetDefaultVideoParameters(
                        fourcc,
                        width,
                        height,
                        framerate,
                        quality);
}

int         VpStackChooserDecorator::GetDefaultVideoParameters(
                    unsigned*           fourcc,
                    unsigned*           width,
                    unsigned*           height,
                    unsigned*           framerate,
                    unsigned*           quality)
{
    return  m_netStack->GetDefaultVideoParameters(
                        fourcc,
                        width,
                        height,
                        framerate,
                        quality);
}

int         VpStackChooserDecorator::RecordAVIFile(
                    const char*         path)
{
    return  m_netStack->RecordAVIFile(path);
}

int         VpStackChooserDecorator::StopAVIRecord()
{
    return  m_netStack->StopAVIRecord();
}

int         VpStackChooserDecorator::GetRecorderCall(
                    VPCALL*             vpcall)
{
    VPCALL      local;
    int         result;


    result  = m_netStack->GetRecorderCall(
                        &local);

    if(!m_l2g.contains(qMakePair(m_netStack, (int)local)))
    {
        addCall(m_netStack, (int)local);
    }

    if(vpcall)
    {
        *vpcall = (VPCALL)local2global(m_netStack, (int)local);
    }

    return  result;
}

int         VpStackChooserDecorator::SetVideoWindowData(
                    VPCALL              vpcall,
                    void*               hParent,
                    int                 childid,
                    const RECTANGLE*    rect,
                    bool                hidden)
{
    return  getStack(vpcall)->SetVideoWindowData(
                        getCall(vpcall),
                        hParent,
                        childid,
                        rect,
                        hidden);
}


int         VpStackChooserDecorator::SetVideoWindowFullScreen(
                    VPCALL              vpcall,
                    int                 fs)
{
    return  getStack(vpcall)->SetVideoWindowFullScreen(
                        getCall(vpcall),
                        fs);
}

int         VpStackChooserDecorator::CaptureVideoImage(
                    VPCALL              vpcall,
                    void**              image,
                    int*                w,
                    int*                h)
{
    return  getStack(vpcall)->CaptureVideoImage(
                        getCall(vpcall),
                        image,
                        w,
                        h);
}

int         VpStackChooserDecorator::SetCallVideoParameters(
                    VPCALL              vpcall,
                    unsigned            fourcc,
                    unsigned            width,
                    unsigned            height,
                    unsigned            framerate,
                    unsigned            quality)
{
    return  getStack(vpcall)->SetCallVideoParameters(
                        getCall(vpcall),
                        fourcc,
                        width,
                        height,
                        framerate,
                        quality);
}

int         VpStackChooserDecorator::GetCallVideoParameters(
                    VPCALL              vpcall,
                    unsigned*           fourcc,
                    unsigned*           width,
                    unsigned*           height,
                    unsigned*           framerate,
                    unsigned*           quality)
{
    return  getStack(vpcall)->GetCallVideoParameters(
                        getCall(vpcall),
                        fourcc,
                        width,
                        height,
                        framerate,
                        quality);
}

int         VpStackChooserDecorator::AcceptConference(
                    VPCALL              vpcall,
                    bool                accept)
{
    return getStack(vpcall)->AcceptConference(
                        getCall(vpcall),
                        accept);
}


//  VpStackChooserDecorator::addCall
//  -------------------------------------------------------------------------
void            VpStackChooserDecorator::addCall(
                            IVPSTACK*           stack,
                            int                 lcall)
{
    Q_ASSERT(!m_l2g.contains(qMakePair(stack, (int)lcall)));

    m_mutex.lock();

    QPair<IVPSTACK*, int>   local   = qMakePair(stack, lcall);
    int                     global  = ++m_call;

    m_l2g[local]    = global;
    m_g2l[global]   = local;

    m_mutex.unlock();
}

//  VpStackChooserDecorator::delCall
//  -------------------------------------------------------------------------
void            VpStackChooserDecorator::delCall(
                            IVPSTACK*           stack,
                            int                 lcall)
{
    Q_ASSERT(m_l2g.contains(qMakePair(stack, lcall)));

    m_mutex.lock();

    QPair<IVPSTACK*, int>   local   = qMakePair(stack, lcall);
    int                     global  = m_l2g[local];

    m_l2g.remove(local);
    m_g2l.remove(global);

    m_mutex.unlock();

}

//  VpStackChooserDecorator::delCall
//  -------------------------------------------------------------------------
void            VpStackChooserDecorator::delCall(
                            int                 gcall)
{
    Q_ASSERT(m_g2l.contains(gcall));

    m_mutex.lock();

    int                     global  = gcall;
    QPair<IVPSTACK*, int>   local   = m_g2l[global];

    m_l2g.remove(local);
    m_g2l.remove(global);

    m_mutex.unlock();
}

//  VpStackChooserDecorator::local2global
//  -------------------------------------------------------------------------
int             VpStackChooserDecorator::local2global(
                            IVPSTACK*           stack,
                            int                 call)
{
    Q_ASSERT(m_l2g.contains(qMakePair(stack, call)));

    m_mutex.lock();

    QPair<IVPSTACK*, int>   local   = qMakePair(stack, call);
    int                     global  = m_l2g[local];

    m_mutex.unlock();

    return  global;
}

//  VpStackChooserDecorator::global2local
//  -------------------------------------------------------------------------
int             VpStackChooserDecorator::global2local(
                            int                 call)
{
    Q_ASSERT(m_g2l.contains(call));

    m_mutex.lock();

    int                     global  = call;
    QPair<IVPSTACK*, int>   local   = m_g2l[global];

    m_mutex.unlock();

    return  local.second;
}

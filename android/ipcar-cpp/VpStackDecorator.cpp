#include "VpStackDecorator.h"
#include <QtCore/QThread>

QMutex          VpStackDecorator::m_mutex;
QWaitCondition  VpStackDecorator::m_done;
int             VpStackDecorator::m_syncResult;

VpStackDecorator::VpStackDecorator(
                    IVPSTACK*           base,
                    QObject*            parent)
    : QObject(parent)
    , m_base(base)
{
}

int         VpStackDecorator::Init()
{
    return  m_base->Init();
}

int         VpStackDecorator::SetNotifyRoutine(
                    void                (*notify)(void *param, unsigned, unsigned, unsigned),
                    void*               param)
{
    m_notify        = notify;
    m_notifyParam   = param;

    return  m_base->SetNotifyRoutine(&VpStackDecorator::vpstackNotificationRoutine, static_cast<void*>(this));
}

int         VpStackDecorator::SetSyncNotifyRoutine(
                    int                 (*notify)(void *param, unsigned, unsigned, unsigned),
                    void*               param)
{
    m_sync      = notify;
    m_syncParam = param;

    return  m_base->SetSyncNotifyRoutine(&VpStackDecorator::vpstackSyncNotifyRoutine, static_cast<void*>(this));
}

VPCALL      VpStackDecorator::CreateCall()
{

    return  m_base->CreateCall();
}

void        VpStackDecorator::FreeCall(
                    VPCALL              vpcall)
{
    return  m_base->FreeCall(
                    vpcall);
}

int         VpStackDecorator::Connect(
                    VPCALL*             vpcall,
                    const char*         address,
                    int                 bc)
{
    return  m_base->Connect(
                        vpcall,
                        address,
                        bc);
}

int         VpStackDecorator::Connect(
                    VPCALL              vpcall,
                    const char*         address,
                    int                 bc)
{
    return  m_base->Connect(
                        vpcall,
                        address,
                        bc);
}

int         VpStackDecorator::SetCallAddress(
                    VPCALL              vpcall,
                    const char*         address)
{
    return  m_base->SetCallAddress(
                        vpcall,
                        address);
}

int         VpStackDecorator::SetCallLocalName(
                    VPCALL              vpcall,
                    const char*         username)
{
    return  m_base->SetCallLocalName(
                        vpcall,
                        username);
}

int         VpStackDecorator::SetCallLocalNumber(
                    VPCALL              vpcall,
                    const char*         number)
{
    return  m_base->SetCallLocalNumber(
                        vpcall,
                        number);
}

int         VpStackDecorator::Disconnect(
                    VPCALL              vpcall,
                    int                 reason)
{
    return  m_base->Disconnect(
                        vpcall,
                        reason);
}

int         VpStackDecorator::AnswerCall(
                    VPCALL              vpcall)
{
    return  m_base->AnswerCall(
                        vpcall);
}

int         VpStackDecorator::Hold(
                    VPCALL              vpcall)
{
    return  m_base->Hold(
                        vpcall);
}

int         VpStackDecorator::Resume(
                    VPCALL              vpcall)
{
    return  m_base->Resume(
                        vpcall);
}

int         VpStackDecorator::ConferenceCalls(
                    VPCALL*             vpcalls,
                    int                 ncalls,
                    bool                detach)
{
    return  m_base->ConferenceCalls(
                        vpcalls,
                        ncalls,
                        detach);
}

int         VpStackDecorator::AddToConference(
                    VPCALL              vpcall)
{
    return m_base->AddToConference(vpcall);
}

int         VpStackDecorator::RemoveFromConference(
                    VPCALL              vpcall)
{
    return m_base->RemoveFromConference(vpcall);
}

int         VpStackDecorator::ConferenceAll()
{
    return m_base->ConferenceAll();
}

int         VpStackDecorator::StopConference()
{
    return m_base->StopConference();
}

int         VpStackDecorator::SendAudioMessage(
                    VPCALL              vpcall,
                    const char*         file,
                    unsigned            msgid)
{
    return  m_base->SendAudioMessage(
                        vpcall,
                        file,
                        msgid);
}

int         VpStackDecorator::SetServers(
                    const char*         list)
{
    return  m_base->SetServers(
                        list);
}

int         VpStackDecorator::Logon(
                    const char*         username,
                    const char*         password)
{
    return  m_base->Logon(
                        username,
                        password);
}

int         VpStackDecorator::GetLogonData(
                    char*               serveraccesscode,
                    unsigned char*      accountsecret)
{
    return m_base->GetLogonData(serveraccesscode, accountsecret);
}

int         VpStackDecorator::Logon(
                    const char*         serveraccesscode,
                    const unsigned char*accountsecret)
{
    return m_base->Logon(serveraccesscode, accountsecret);
}

int         VpStackDecorator::Logoff()
{
    return  m_base->Logoff();
}

int         VpStackDecorator::IsLoggedOn()
{
    return  m_base->IsLoggedOn();
}

const char* VpStackDecorator::LogonName()
{
    return  m_base->LogonName();
}

const char* VpStackDecorator::LogonNumber()
{
    return  m_base->LogonNumber();
}

void        VpStackDecorator::SetSupportedBearersMask(
                    unsigned            bc_mask)
{
    return  m_base->SetSupportedBearersMask(
                        bc_mask);
}

void        VpStackDecorator::SetAudioDataFactory(
                    VPAUDIODATAFACTORY* af)
{
    return  m_base->SetAudioDataFactory(
                        af);
}

void        VpStackDecorator::SetVideoDataFactory(
                    VPVIDEODATAFACTORY* vf)
{
    return  m_base->SetVideoDataFactory(
                        vf);
}

int         VpStackDecorator::SendKeypad(
                    VPCALL              vpcall,
                    int                 key)
{
    return  m_base->SendKeypad(
                        vpcall,
                        key);
}

int         VpStackDecorator::GetStackType()
{
    return  m_base->GetStackType();
}

int         VpStackDecorator::EnumCalls(
                    VPCALL*             vpcalls,
                    unsigned            maxvpcalls,
                    unsigned            mask)
{
    return  m_base->EnumCalls(
                        vpcalls,
                        maxvpcalls,
                        mask);
}

int         VpStackDecorator::NetworkQuality()
{
    return  m_base->NetworkQuality();
}

int         VpStackDecorator::GetQosData(
                    VPCALL              vpcall,
                    QOSDATA*            qos)
{
    return  m_base->GetQosData(
                        vpcall,
                        qos);
}

int         VpStackDecorator::GetAudioParameters(
                    VPCALL              vpcall,
                    int*                codec,
                    int*                framesperpacket)
{
    return  m_base->GetAudioParameters(
                        vpcall,
                        codec,
                        framesperpacket);
}

int         VpStackDecorator::SetCodecsMask(
                    unsigned            mask)
{
    return  m_base->SetCodecsMask(
                        mask);
}

char*       VpStackDecorator::StackName()
{
    return  m_base->StackName();
}

void        VpStackDecorator::SetBindPort(
                    int                 port)
{
    m_base->SetBindPort(port);
}

int         VpStackDecorator::GetBindPort()
{
    return  m_base->GetBindPort();
}

void        VpStackDecorator::SetBandwidth(
                    int                 kbps)
{
    return  m_base->SetBandwidth(
                        kbps);
}

int         VpStackDecorator::GetCallStatus(
                    VPCALL              vpcall)
{
    return  m_base->GetCallStatus(
                        vpcall);
}

int         VpStackDecorator::GetTransceiverBandwidths(
                    int*                rx,
                    int*                tx)
{
    return  m_base->GetTransceiverBandwidths(
                        rx,
                        tx);
}

int         VpStackDecorator::GetConferencePeers(
                    VPCALL              vpcall,
                    VPCALL*             vpcalls,
                    int*                ncalls)
{
    return  m_base->GetConferencePeers(
                        vpcall,
                        vpcalls,
                        ncalls);
}

int         VpStackDecorator::GetLinkedCall(
                    VPCALL              vpcall,
                    VPCALL*             linkedcall)
{
    return  m_base->GetLinkedCall(
                        vpcall,
                        linkedcall);
}

int         VpStackDecorator::GetCallRemoteName(
                    VPCALL              vpcall,
                    char*               username)
{
    return  m_base->GetCallRemoteName(
                        vpcall,
                        username);
}

int         VpStackDecorator::GetCallRemoteNumber(
                    VPCALL              vpcall,
                    char*               number)
{
    return  m_base->GetCallRemoteNumber(
                        vpcall,
                        number);
}

int         VpStackDecorator::GetCallRemoteAddress(
                    VPCALL              vpcall,
                    char*               address)
{
    return  m_base->GetCallRemoteAddress(
                        vpcall,
                        address);
}

int         VpStackDecorator::GetCallRemoteSubAddr(
                    VPCALL              vpcall,
                    char*               subaddr)
{
    return  m_base->GetCallRemoteSubAddr(
                        vpcall,
                        subaddr);
}

int         VpStackDecorator::GetCallLocalName(
                    VPCALL              vpcall,
                    char*               username)
{
    return  m_base->GetCallLocalName(
                        vpcall,
                        username);
}

int         VpStackDecorator::GetCallLocalNumber(
                    VPCALL              vpcall,
                    char*               number)
{
    return  m_base->GetCallLocalNumber(
                        vpcall,
                        number);
}

int         VpStackDecorator::GetCallLocalSubAddr(
                    VPCALL              vpcall,
                    char*               subaddr)
{
    return  m_base->GetCallLocalSubAddr(
                        vpcall,
                        subaddr);
}

int         VpStackDecorator::GetCallBearer(
                    VPCALL              vpcall)
{
    return  m_base->GetCallBearer(
                        vpcall);
}

int         VpStackDecorator::GetCallCodec(
                    VPCALL              vpcall)
{
    return  m_base->GetCallCodec(
                        vpcall);
}

int         VpStackDecorator::GetCallTimes(
                    VPCALL              vpcall,
                    time_t*             start_time,
                    unsigned*           length_ms)
{
    return  m_base->GetCallTimes(vpcall, start_time, length_ms);
}

int         VpStackDecorator::IsHeld(
                    VPCALL              vpcall)
{
    return  m_base->IsHeld(
                        vpcall);
}

int         VpStackDecorator::IsConferenced(
                    VPCALL              vpcall)
{
    return  m_base->IsConferenced(vpcall);
}


int         VpStackDecorator::SetCallDisconnectMessage(
                    VPCALL              vpcall,
                    const char*         text)
{
    return m_base->SetCallDisconnectMessage(
                        vpcall,
                        text);
}
int         VpStackDecorator::CallForwardingRequest(
                    int                 op,
                    const char*         number)
{
    return  m_base->CallForwardingRequest(op, number);
}

int         VpStackDecorator::GetCallForwardingStatus(
                    int*                op,
                    char*               number)
{
    return  m_base->GetCallForwardingStatus(op, number);
}

int         VpStackDecorator::MeasureBandwidth()
{
    return  m_base->MeasureBandwidth();
}

int         VpStackDecorator::SendChat(
                    VPCALL              vpcall,
                    const char*         text)
{
    return  m_base->SendChat(
                        vpcall,
                        text);
}

int         VpStackDecorator::GetChatText(
                    VPCALL              vpcall,
                    char*               text,
                    int                 textsize)
{
    return  m_base->GetChatText(
                        vpcall,
                        text,
                        textsize);
}

// SMS data
int         VpStackDecorator::GetMissedCallsData(
                    VPCALL              vpcall,
                    unsigned*           missedcallscount,
                    unsigned*           missedcalltime,
                    unsigned*           missedcallbc)
{
    return  m_base->GetMissedCallsData(
                        vpcall,
                        missedcallscount,
                        missedcalltime,
                        missedcallbc);
}

int         VpStackDecorator::GetCallText(
                    VPCALL              vpcall,
                    char*               text)
{
    return  m_base->GetCallText(vpcall, text);
}

int         VpStackDecorator::SetCallText(
                    VPCALL              vpcall,
                    const char*         text)
{
    return  m_base->SetCallText(vpcall, text);
}

int         VpStackDecorator::SetMissedCallBC(
                    VPCALL              vpcall,
                    int                 bc)
{
    return  m_base->SetMissedCallBC(vpcall, bc);
}

int         VpStackDecorator::SendAudio(
                    VPCALL              vpcall,
                    int                 codec,
                    unsigned            timestamp,
                    const void*         buf,
                    int                 len,
                    bool                wait)
{
    return  m_base->SendAudio(vpcall, codec, timestamp, buf, len, wait);
}

int         VpStackDecorator::SendVideo(
                    VPCALL              vpcall,
                    unsigned short      timestamp,
                    void*               data,
                    int                 size,
                    bool                keyframe)
{
    return  m_base->SendVideo(vpcall, timestamp, data, size, keyframe);
}

int         VpStackDecorator::AskOnline(
                    AOL*                aol)
{
    return  m_base->AskOnline(aol);
}

int         VpStackDecorator::QueryAccountInfo()
{
    return  m_base->QueryAccountInfo();
}

int         VpStackDecorator::ServerTransfer(
                    int                 op,
                    const char*         path,
                    bool                amactive)
{
    return  m_base->ServerTransfer(op, path, amactive);
}


int         VpStackDecorator::SendFile(
                    VPCALL              vpcall,
                    const char*         path)
{
    return  m_base->SendFile(   vpcall,
                        path);
}

int         VpStackDecorator::AbortFileTransfer(
                    VPCALL              vpcall)
{
    return  m_base->AbortFileTransfer(
                        vpcall);
}

int         VpStackDecorator::AcceptFile(
                    VPCALL              vpcall,
                    int                 accept)
{
    return  m_base->AcceptFile( vpcall,
                        accept);
}

int         VpStackDecorator::GetCallFilePath(
                    VPCALL              vpcall,
                    char*               path)
{
    return  m_base->GetCallFilePath(
                        vpcall,
                        path);
}

int         VpStackDecorator::SetCallFilePath(
                    VPCALL              vpcall,
                    const char*         path)
{
    return  m_base->SetCallFilePath(
                        vpcall,
                        path);
}

int         VpStackDecorator::SetCallNFiles(
                    VPCALL              vpcall,
                    unsigned            nfiles,
                    unsigned            nbytes)
{
    return  m_base->SetCallNFiles(
                        vpcall,
                        nfiles,
                        nbytes);
}

int         VpStackDecorator::GetCallNFiles(
                    VPCALL              vpcall,
                    unsigned*           nfiles,
                    unsigned*           nbytes)
{
    return  m_base->GetCallNFiles(
                        vpcall,
                        nfiles,
                        nbytes);
}

int         VpStackDecorator::GetFileProgress(
                    VPCALL              vpcall,
                    unsigned*           current,
                    unsigned*           total)
{
    return  m_base->GetFileProgress(
                        vpcall,
                        current,
                        total);
}

//  VpStackDecorator::vpstackNotificationRoutine
//  -------------------------------------------------------------------------
void        VpStackDecorator::vpstackNotificationRoutine(
                            void*               uparam,
                            unsigned            msg,
                            unsigned            pcall,
                            unsigned            param)
{
    VpStackDecorator*   self    = static_cast<VpStackDecorator*>(uparam);

    if(self->thread() != QThread::currentThread())
    {
        QMetaObject::invokeMethod(
                        self,
                        "vpstackNotificationRoutineSlot",
                        Qt::QueuedConnection,
                        Q_ARG(uint, msg),
                        Q_ARG(uint, pcall),
                        Q_ARG(uint, param));
    }
    else
    {
        self->vpstackNotificationRoutine(msg, pcall, param);
    }
}

//  VpStackDecorator::vpstackSyncNotifyRoutine
//  -------------------------------------------------------------------------
int         VpStackDecorator::vpstackSyncNotifyRoutine(
                            void*               uparam,
                            unsigned            msg,
                            unsigned            param1,
                            unsigned            param2)
{
    VpStackDecorator*   self    = static_cast<VpStackDecorator*>(uparam);
    if(self->thread() != QThread::currentThread())
    {
        m_mutex.lock();

        QMetaObject::invokeMethod(
                        self,
                        "vpstackSyncNotifyRoutineSlot",
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
        return  self->vpstackSyncNotifyRoutine(msg, param1, param2);
    }
}

//  VpStackDecorator::vpstackNotificationRoutineSlot
//  -------------------------------------------------------------------------
void        VpStackDecorator::vpstackNotificationRoutineSlot(
                            unsigned            msg,
                            unsigned            pcall,
                            unsigned            param)
{
    vpstackNotificationRoutine(msg, pcall, param);
}

//  VpStackDecorator::vpstackSyncNotifyRoutineSlot
//  -------------------------------------------------------------------------
void        VpStackDecorator::vpstackSyncNotifyRoutineSlot(
                            unsigned            msg,
                            unsigned            param1,
                            unsigned            param2)
{
    m_syncResult    = vpstackSyncNotifyRoutine(msg, param1, param2);
    m_done.wakeAll();
}

//  VpStackDecorator::vpstackNotificationRoutine
//  -------------------------------------------------------------------------
void        VpStackDecorator::vpstackNotificationRoutine(
                            unsigned            msg,
                            unsigned            pcall,
                            unsigned            param)
{
    DefNotificationRoutine(msg, pcall, param);
}

//  VpStackDecorator::vpstackSyncNotifyRoutine
//  -------------------------------------------------------------------------
int             VpStackDecorator::vpstackSyncNotifyRoutine(
                            unsigned            msg,
                            unsigned            param1,
                            unsigned            param2)
{
    return  DefSyncNotifyRoutine(msg, param1, param2);
}


//  VpStackDecorator::DefNotificationRoutine
//  -------------------------------------------------------------------------
void            VpStackDecorator::DefNotificationRoutine(
                            unsigned int        msg,
                            unsigned int        pcall,
                            unsigned int        param)
{
    if(m_notify)
    {
        m_notify(m_notifyParam, msg, pcall, param);
    }
}

//  VpStackDecorator::DefSyncNotifyRoutine
//  -------------------------------------------------------------------------
int            VpStackDecorator::DefSyncNotifyRoutine(
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

int         VpStackDecorator::SetDefaultVideoParameters(
                    unsigned            fourcc,
                    unsigned            width,
                    unsigned            height,
                    unsigned            framerate,
                    unsigned            quality)
{
    return  m_base->SetDefaultVideoParameters(
                        fourcc,
                        width,
                        height,
                        framerate,
                        quality);
}

int         VpStackDecorator::GetDefaultVideoParameters(
                    unsigned*           fourcc,
                    unsigned*           width,
                    unsigned*           height,
                    unsigned*           framerate,
                    unsigned*           quality)
{
    return  m_base->GetDefaultVideoParameters(
                        fourcc,
                        width,
                        height,
                        framerate,
                        quality);
}

int         VpStackDecorator::RecordAVIFile(
                    const char*         path)
{
    return  m_base->RecordAVIFile(path);
}

int         VpStackDecorator::StopAVIRecord()
{
    return  m_base->StopAVIRecord();
}

int         VpStackDecorator::GetRecorderCall(
                    VPCALL*             vpcall)
{
    return  m_base->GetRecorderCall(vpcall);
}

int         VpStackDecorator::SetVideoWindowData(
                    VPCALL              vpcall,
                    void*               hParent,
                    int                 childid,
                    const RECTANGLE*    rect,
                    bool                hidden)
{
    return  m_base->SetVideoWindowData(
                        vpcall,
                        hParent,
                        childid,
                        rect,
                        hidden);
}


int         VpStackDecorator::SetVideoWindowFullScreen(
                    VPCALL              vpcall,
                    int                 fs)
{
    return  m_base->SetVideoWindowFullScreen(vpcall, fs);
}

int         VpStackDecorator::CaptureVideoImage(
                    VPCALL              vpcall,
                    void**              image,
                    int*                w,
                    int*                h)
{
    return  m_base->CaptureVideoImage(vpcall, image, w, h);
}

int         VpStackDecorator::SetCallVideoParameters(
                    VPCALL              vpcall,
                    unsigned            fourcc,
                    unsigned            width,
                    unsigned            height,
                    unsigned            framerate,
                    unsigned            quality)
{
    return  m_base->SetCallVideoParameters(
            vpcall,
            fourcc,
            width,
            height,
            framerate,
            quality);
}

int         VpStackDecorator::GetCallVideoParameters(
                    VPCALL              vpcall,
                    unsigned*           fourcc,
                    unsigned*           width,
                    unsigned*           height,
                    unsigned*           framerate,
                    unsigned*           quality)
{
    return  m_base->GetCallVideoParameters(
            vpcall,
            fourcc,
            width,
            height,
            framerate,
            quality);
}

int         VpStackDecorator::AcceptConference(
                    VPCALL              vpcall,
                    bool                accept)
{
    return m_base->AcceptConference(vpcall, accept);
}

int         VpStackDecorator::SignalLevel(
                    int*                dBm)
{
    return  m_base->SignalLevel(dBm);
}

int         VpStackDecorator::GetOperatorName(
                    char*               name,
                    int                 namesize,
                    unsigned*           flags)
{
    return  m_base->GetOperatorName(name, namesize, flags);
}

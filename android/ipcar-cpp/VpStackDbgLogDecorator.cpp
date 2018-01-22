#include "VpStackDbgLogDecorator.h"
#include "Utils.h"

#include <QtCore/QDateTime>
#include <QtCore/QString>
#include <QtCore/QDir>

#define ADD_MESSAGE(x)  m_msg2str[x]    = # x

QFile*              VpStackDbgLogDecorator::s_file  = NULL;
QTextStream         VpStackDbgLogDecorator::s_stream;
QSemaphore          VpStackDbgLogDecorator::s_semaphore(1);


//  VpStackDbgLogDecorator::VpStackDbgLogDecorator
//  -------------------------------------------------------------------------
VpStackDbgLogDecorator::VpStackDbgLogDecorator(
            IVPSTACK*           base,
            QObject*            parent)
    : VpStackDecorator(base, parent)
{
    QString     datetime    = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");

    s_semaphore.acquire();
    if(s_file == NULL)
    {
#ifdef HOME_PATH_SDCARD
	QString filename    = QDir("/sdcard").path() + "/"PROJECTNAME"/Data/vpstack-%1.log";
#else
        QString filename    = QDir::homePath() + "/"PROJECTNAME"/Data/vpstack-%1.log";
#endif
        s_file  = new QFile(filename.arg(datetime));
        s_file->open(QIODevice::WriteOnly);
    
        s_stream.setDevice(s_file);
    }
    s_semaphore.release();

    char const* stackname   = base->StackName();

    m_name  = stackname ? QString(stackname) : QString("%1").arg((unsigned)base, 8, 16, QChar('0'));

    ADD_MESSAGE(VPMSG_SERVERSLISTCHANGED);
    ADD_MESSAGE(VPMSG_SERVERSTATUS);
    ADD_MESSAGE(VPMSG_CALLFORWARDING);
    ADD_MESSAGE(VPMSG_SERVERTRANSFERFINISHED);
    ADD_MESSAGE(VPMSG_INVALIDADDR);
    ADD_MESSAGE(VPMSG_SERVERNOTRESPONDING);
    ADD_MESSAGE(VPMSG_RESOLVED);
    ADD_MESSAGE(VPMSG_USERALERTED);
    ADD_MESSAGE(VPMSG_CALLREFUSED);
    ADD_MESSAGE(VPMSG_CALLACCEPTED);
    ADD_MESSAGE(VPMSG_CALLACCEPTEDBYUSER);
    ADD_MESSAGE(VPMSG_CONNECTFINISHED);
    ADD_MESSAGE(VPMSG_CALLESTABLISHED);
    ADD_MESSAGE(VPMSG_CALLSETUPFAILED);
    ADD_MESSAGE(VPMSG_CONNECTTIMEOUT);
    ADD_MESSAGE(VPMSG_CALLDISCONNECTED);
    ADD_MESSAGE(VPMSG_CALLENDED);
    ADD_MESSAGE(VPMSG_NEWCALL);
    ADD_MESSAGE(VPMSG_KEYPAD);
    ADD_MESSAGE(VPMSG_AUDIOMSGCOMPLETE);
    ADD_MESSAGE(VPMSG_REMOTELYCONFERENCED);
    ADD_MESSAGE(VPMSG_REMOTELYUNCONFERENCED);
    ADD_MESSAGE(VPMSG_REMOTELYHELD);
    ADD_MESSAGE(VPMSG_REMOTELYRESUMED);
    ADD_MESSAGE(VPMSG_CONFERENCEESTABLISHED);
    ADD_MESSAGE(VPMSG_CONFERENCEFAILED);
    ADD_MESSAGE(VPMSG_CONFERENCEREQ);
    ADD_MESSAGE(VPMSG_CONFERENCECALLESTABLISHED);
    ADD_MESSAGE(VPMSG_LOCALCONFERENCEREQ);
    ADD_MESSAGE(VPMSG_CONFERENCEEND);
    ADD_MESSAGE(VPMSG_CALLTRANSFERRED);
    ADD_MESSAGE(VPMSG_CHAT);
    ADD_MESSAGE(VPMSG_CHATACK);
    ADD_MESSAGE(VPMSG_SENDFILEACK_REFUSED);
    ADD_MESSAGE(VPMSG_SENDFILEACK_USERALERTED);
    ADD_MESSAGE(VPMSG_SENDFILEACK_ACCEPTED);
    ADD_MESSAGE(VPMSG_FTPROGRESS);
    ADD_MESSAGE(VPMSG_FTTIMEOUT);
    ADD_MESSAGE(VPMSG_FTTXCOMPLETE);
    ADD_MESSAGE(VPMSG_FTRXCOMPLETE);
    ADD_MESSAGE(VPMSG_SENDFILEREQ);
    ADD_MESSAGE(VPMSG_SENDFILESUCCESS);
    ADD_MESSAGE(VPMSG_SENDFILEFAILED);
    ADD_MESSAGE(VPMSG_HELD);
    ADD_MESSAGE(VPMSG_RESUMED);
    ADD_MESSAGE(VPMSG_REQUESTREFUSED);
    ADD_MESSAGE(VPMSG_QUERYONLINEACK);
    ADD_MESSAGE(VPMSG_CREDIT);
    ADD_MESSAGE(VPMSG_DISPLAYINFO);
    ADD_MESSAGE(VPMSG_ISDNINFO);
    ADD_MESSAGE(VPMSG_SSERROR);
    ADD_MESSAGE(VPMSG_NOTIFYLOGON);
    ADD_MESSAGE(VPMSG_NEWVOICEMSG);
    ADD_MESSAGE(VPMSG_ABUPDATE);
}

//  VpStackDbgLogDecorator::~VpStackDbgLogDecorator
//  -------------------------------------------------------------------------
VpStackDbgLogDecorator::~VpStackDbgLogDecorator()
{
//    s_file->close();

//    delete s_file;
}

//  VpStackDbgLogDecorator::vpstackNotificationRoutine
//  -------------------------------------------------------------------------
void    VpStackDbgLogDecorator::vpstackNotificationRoutine(
                            unsigned            msg,
                            unsigned            pcall,
                            unsigned            param)
{
    QString     datetime    = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString     text;
    QTextStream stream(&text);

    stream    << datetime << " " << m_name << "\t";
    if(m_msg2str.find(msg) != m_msg2str.end())
    {
        stream    << m_msg2str[msg] << "\t";
    }
    else
    {
        stream    << "UNKNOWN(" << msg << ")" << "\t";
    }

    if(     msg < VPMSG_INVALIDADDR
       ||   msg > VPMSG_REQUESTREFUSED)
    {
        stream    << pcall << "\t" << param << endl;
    }
    else
    {
        stream    << pcall << "\treason = " << getReason(param) << endl;
    }

    s_semaphore.acquire();
    s_stream    << text;
    s_stream.flush();
    s_semaphore.release();

    DefNotificationRoutine(msg, pcall, param);
}

//  VpStackDbgLogDecorator::vpstackSyncNotifyRoutine
//  -------------------------------------------------------------------------
int     VpStackDbgLogDecorator::vpstackSyncNotifyRoutine(
                            unsigned            msg,
                            unsigned            param1,
                            unsigned            param2)
{
    QString     datetime    = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString     text;
    QTextStream stream(&text);

    stream    << datetime << " " << m_name << "\t";
    if(m_msg2str.find(msg) != m_msg2str.end())
    {
        stream    << m_msg2str[msg] << "\t";
    }
    else
    {
        stream    << "UNKNOWN(" << msg << ")" << "\t";
    }

    stream    << param1 << "\t" << param2 << endl;

    s_semaphore.acquire();
    s_stream    << text;
    s_stream.flush();
    s_semaphore.release();

    return  DefSyncNotifyRoutine(msg, param1, param2);
}

//template<typename T>
//struct  getval  {static     T   value(T t)  {return t;}};
template<typename T>
struct  typetrait
{
        typetrait(T t)
            : m_t(t)
        {

        }

        T       deref()
        {
            return  m_t;
        }

    private:
        T   m_t;
};

template<>
struct  typetrait<int*>
{
        typetrait(int*   t)
            : m_t(t)
        {

        }

        int         deref()
        {
            if(m_t)
                return  *m_t;
            else
                return  0;
        }

    private:
        int*        m_t;
};

template<>
struct  typetrait<const RECTANGLE*>
{
        typetrait(const RECTANGLE*   t)
            : m_t(t)
        {

        }

        QString     deref()
        {
            if(m_t)
            {
                return  QString("<%1, %2, %3, %4>")
                        .arg(m_t->left)
                        .arg(m_t->top)
                        .arg(m_t->right)
                        .arg(m_t->bottom);
            }
            else
                return  0;
        }

    private:
        const RECTANGLE*    m_t;
};

template<>
struct  typetrait<unsigned*>
{
        typetrait(unsigned*   t)
            : m_t(t)
        {

        }

        unsigned         deref()
        {
            if(m_t)
                return  *m_t;
            else
                return  0;
        }

    private:
        unsigned*        m_t;
};

//  VpStackDbgLogDecorator::logCall
//  -------------------------------------------------------------------------
template<typename R>
R               VpStackDbgLogDecorator::logCall0(
                            const char*         name,
                            IVPSTACK*           object,
                            R                   (IVPSTACK::*member)())
{
    QString     datetime    = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    R           result;

    QString     text;
    QTextStream stream(&text);

    stream  <<  datetime << " " << m_name << "\t" << name    << "\t= ";
    result  = (object->*member)();
    stream  <<  result << endl;

    s_semaphore.acquire();
    s_stream    << text;
    s_stream.flush();
    s_semaphore.release();

    return  result;
}

template<typename R, typename T1>
R               VpStackDbgLogDecorator::logCall1(
                            const char*         name,
                            IVPSTACK*           object,
                            R                   (IVPSTACK::*member)(T1),
                            T1                  t1)
{
    QString     datetime    = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    R           result;

    QString     text;
    QTextStream stream(&text);

    stream    <<  datetime << " " << m_name << "\t" << name;
    result  = (object->*member)(t1);
    stream    << "\t"     << typetrait<T1>(t1).deref()
              << "\t= "   << result << endl;

    s_semaphore.acquire();
    s_stream    << text;
    s_stream.flush();
    s_semaphore.release();

    return  result;
}

void            VpStackDbgLogDecorator::logCallv0(
                            const char*         name,
                            IVPSTACK*           object,
                            void                (IVPSTACK::*member)())
{
    QString     datetime    = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    QString     text;
    QTextStream stream(&text);

    stream    <<  datetime << " " << m_name << "\t" << name;
    (object->*member)();
    stream    << endl;

    s_semaphore.acquire();
    s_stream    << text;
    s_stream.flush();
    s_semaphore.release();
}

template<typename T1>
void            VpStackDbgLogDecorator::logCallv1(
                            const char*         name,
                            IVPSTACK*           object,
                            void                (IVPSTACK::*member)(T1),
                            T1                  t1)
{
    QString     datetime    = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    QString     text;
    QTextStream stream(&text);

    stream    <<  datetime << " " << m_name << "\t" << name;
    (object->*member)(t1);
    stream    << "\t"     << typetrait<T1>(t1).deref()
              << endl;

    s_semaphore.acquire();
    s_stream    << text;
    s_stream.flush();
    s_semaphore.release();
}

template<typename R, typename T1, typename T2>
R               VpStackDbgLogDecorator::logCall2(
                            const char*         name,
                            IVPSTACK*           object,
                            R                   (IVPSTACK::*member)(T1, T2),
                            T1                  t1,
                            T2                  t2)
{
    QString     datetime    = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    R           result;

    QString     text;
    QTextStream stream(&text);

    stream    <<  datetime << " " << m_name << "\t" << name;
    result  = (object->*member)(t1, t2);
    stream    << "\t"     << typetrait<T1>(t1).deref()
              << "\t"     << typetrait<T2>(t2).deref()
              << "\t= "   <<  result << endl;

    s_semaphore.acquire();
    s_stream    << text;
    s_stream.flush();
    s_semaphore.release();

    return  result;
}

template<typename R, typename T1, typename T2, typename T3>
R               VpStackDbgLogDecorator::logCall3(
                            const char*         name,
                            IVPSTACK*           object,
                            R                   (IVPSTACK::*member)(T1, T2, T3),
                            T1                  t1,
                            T2                  t2,
                            T3                  t3)
{
    QString     datetime   = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    R           result;

    QString     text;
    QTextStream stream(&text);

    stream    <<  datetime << " " << m_name << "\t" << name;
    result  = (object->*member)(t1, t2, t3);
    stream    << "\t"     << typetrait<T1>(t1).deref()
              << "\t"     << typetrait<T2>(t2).deref()
              << "\t"     << typetrait<T3>(t3).deref()
              << "\t= "   <<  result << endl;

    s_semaphore.acquire();
    s_stream    << text;
    s_stream.flush();
    s_semaphore.release();

    return  result;
}

template<typename R, typename T1, typename T2, typename T3, typename T4>
R               VpStackDbgLogDecorator::logCall4(
                            const char*         name,
                            IVPSTACK*           object,
                            R                   (IVPSTACK::*member)(T1, T2, T3, T4),
                            T1                  t1,
                            T2                  t2,
                            T3                  t3,
                            T4                  t4)
{
    QString     datetime    = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    R           result;

    QString     text;
    QTextStream stream(&text);

    stream    <<  datetime << " " << m_name << "\t" << name;
    result  = (object->*member)(t1, t2, t3, t4);
    stream    << "\t"     << typetrait<T1>(t1).deref()
              << "\t"     << typetrait<T2>(t2).deref()
              << "\t"     << typetrait<T3>(t3).deref()
              << "\t"     << typetrait<T4>(t4).deref()
              << "\t= "   <<  result << endl;

    s_semaphore.acquire();
    s_stream    << text;
    s_stream.flush();
    s_semaphore.release();

    return  result;
}

template<typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
R               VpStackDbgLogDecorator::logCall5(
                            const char*         name,
                            IVPSTACK*           object,
                            R                   (IVPSTACK::*member)(T1, T2, T3, T4, T5),
                            T1                  t1,
                            T2                  t2,
                            T3                  t3,
                            T4                  t4,
                            T5                  t5)
{
    QString     datetime    = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    R           result;

    QString     text;
    QTextStream stream(&text);

    stream    <<  datetime << " " << m_name << "\t" << name;
    result  = (object->*member)(t1, t2, t3, t4, t5);
    stream    << "\t"     << typetrait<T1>(t1).deref()
              << "\t"     << typetrait<T2>(t2).deref()
              << "\t"     << typetrait<T3>(t3).deref()
              << "\t"     << typetrait<T4>(t4).deref()
              << "\t"     << typetrait<T5>(t5).deref()
              << "\t= "   <<  result << endl;

    s_semaphore.acquire();
    s_stream    << text;
    s_stream.flush();
    s_semaphore.release();

    return  result;
}

template<typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
R               VpStackDbgLogDecorator::logCall6(
                            const char*         name,
                            IVPSTACK*           object,
                            R                   (IVPSTACK::*member)(T1, T2, T3, T4, T5, T6),
                            T1                  t1,
                            T2                  t2,
                            T3                  t3,
                            T4                  t4,
                            T5                  t5,
                            T6                  t6)
{
    QString     datetime    = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    R           result;

    QString     text;
    QTextStream stream(&text);

    stream    <<  datetime << " " << m_name << "\t" << name;
    result  = (object->*member)(t1, t2, t3, t4, t5, t6);
    stream    << "\t"     << typetrait<T1>(t1).deref()
              << "\t"     << typetrait<T2>(t2).deref()
              << "\t"     << typetrait<T3>(t3).deref()
              << "\t"     << typetrait<T4>(t4).deref()
              << "\t"     << typetrait<T5>(t5).deref()
              << "\t"     << typetrait<T6>(t6).deref()
              << "\t= "   <<  result << endl;

    s_semaphore.acquire();
    s_stream    << text;
    s_stream.flush();
    s_semaphore.release();

    return  result;
}

//  -------------------------------------------------------------------------
int         VpStackDbgLogDecorator::SetNotifyRoutine(
                    void                (*notify)(void *param, unsigned, unsigned, unsigned),
                    void*               param)
{
    return  VpStackDecorator::SetNotifyRoutine(notify, param);
}

int         VpStackDbgLogDecorator::SetSyncNotifyRoutine(
                    int                 (*notify)(void *param, unsigned, unsigned, unsigned),
                    void*               param)
{
    return  VpStackDecorator::SetSyncNotifyRoutine(notify, param);
}

VPCALL      VpStackDbgLogDecorator::CreateCall()
{

    return  logCall0("CreateCall", m_base, &IVPSTACK::CreateCall);
}

void        VpStackDbgLogDecorator::FreeCall(
                    VPCALL              vpcall)
{
    logCallv1("FreeCall", m_base, &IVPSTACK::FreeCall, 
                    vpcall);
}

int         VpStackDbgLogDecorator::Connect(
                    VPCALL*             vpcall,
                    const char*         address,
                    int                 bc)
{
    return  logCall3<int, VPCALL*, const char*, int>("Connect", m_base, &IVPSTACK::Connect, 
                        vpcall,
                        address,
                        bc);
}

int         VpStackDbgLogDecorator::Connect(
                    VPCALL              vpcall,
                    const char*         address,
                    int                 bc)
{
    return  logCall3<int, VPCALL, const char*, int>("Connect", m_base, &IVPSTACK::Connect, 
                        vpcall,
                        address,
                        bc);
}

int         VpStackDbgLogDecorator::SetCallAddress(
                    VPCALL              vpcall,
                    const char*         address)
{
    return  logCall2("SetCallAddress", m_base, &IVPSTACK::SetCallAddress, 
                        vpcall,
                        address);
}

int         VpStackDbgLogDecorator::SetCallLocalName(
                    VPCALL              vpcall,
                    const char*         username)
{
    return  logCall2("SetCallLocalName", m_base, &IVPSTACK::SetCallLocalName, 
                        vpcall,
                        username);
}

int         VpStackDbgLogDecorator::SetCallLocalNumber(
                    VPCALL              vpcall,
                    const char*         number)
{
    return  logCall2("SetCallLocalNumber", m_base, &IVPSTACK::SetCallLocalNumber, 
                        vpcall,
                        number);
}

int         VpStackDbgLogDecorator::Disconnect(
                    VPCALL              vpcall,
                    int                 reason)
{
    return  logCall2("Disconnect", m_base, &IVPSTACK::Disconnect, 
                        vpcall,
                        reason);
}

int         VpStackDbgLogDecorator::AnswerCall(
                    VPCALL              vpcall)
{
    return  logCall1("AnswerCall", m_base, &IVPSTACK::AnswerCall, 
                        vpcall);
}

int         VpStackDbgLogDecorator::Hold(
                    VPCALL              vpcall)
{
    return  logCall1("Hold", m_base, &IVPSTACK::Hold, 
                        vpcall);
}

int         VpStackDbgLogDecorator::Resume(
                    VPCALL              vpcall)
{
    return  logCall1("Resume", m_base, &IVPSTACK::Resume, 
                        vpcall);
}

int         VpStackDbgLogDecorator::ConferenceCalls(
                    VPCALL*             vpcalls,
                    int                 ncalls,
                    bool                detach)
{
    return  logCall3("ConferenceCalls", m_base, &IVPSTACK::ConferenceCalls, 
                        vpcalls,
                        ncalls,
                        detach);
}

int         VpStackDbgLogDecorator::AddToConference(
                    VPCALL              vpcall)
{
    return  logCall1("AddToConference", m_base, &IVPSTACK::AddToConference,
                        vpcall);
}

int         VpStackDbgLogDecorator::RemoveFromConference(
                    VPCALL              vpcall)
{
    return  logCall1("RemoveFromConference", m_base, &IVPSTACK::RemoveFromConference,
                        vpcall);
}

int         VpStackDbgLogDecorator::ConferenceAll()
{
    return  logCall0("ConferenceAll", m_base, &IVPSTACK::ConferenceAll);
}

int         VpStackDbgLogDecorator::StopConference()
{
    return  logCall0("StopConference", m_base, &IVPSTACK::StopConference);
}

int         VpStackDbgLogDecorator::SendAudioMessage(
                    VPCALL              vpcall,
                    const char*         file,
                    unsigned            msgid)
{
    return  logCall3("SendAudioMessage", m_base, &IVPSTACK::SendAudioMessage, 
                        vpcall,
                        file,
                        msgid);
}

int         VpStackDbgLogDecorator::SetServers(
                    const char*         list)
{
    return  logCall1("SetServers", m_base, &IVPSTACK::SetServers, 
                        list);
}

int         VpStackDbgLogDecorator::Logon(
                    const char*         username,
                    const char*         password)
{
    return  logCall2<int, const char*, const char*>(
                        "Logon", m_base, &IVPSTACK::Logon, 
                        username,
                        password);
}

int         VpStackDbgLogDecorator::Logon(
                    const char*         serveraccesscode,
                    const unsigned char*accountsecret)
{
    return  logCall2<int, const char*, const unsigned char*>(
                        "Logon", m_base, &IVPSTACK::Logon,
                        serveraccesscode,
                        accountsecret);
}

int         VpStackDbgLogDecorator::GetLogonData(
                    char*               serveraccesscode,
                    unsigned char*      accountsecret)
{
    return  logCall2("GetLogonData", m_base, &IVPSTACK::GetLogonData,
                        serveraccesscode,
                        accountsecret);
}

int         VpStackDbgLogDecorator::Logoff()
{
    return  logCall0("Logoff", m_base, &IVPSTACK::Logoff);
}

int         VpStackDbgLogDecorator::IsLoggedOn()
{
    return  logCall0("IsLoggedOn", m_base, &IVPSTACK::IsLoggedOn);
}

const char* VpStackDbgLogDecorator::LogonName()
{
    return  logCall0("LogonName", m_base, &IVPSTACK::LogonName);
}

const char* VpStackDbgLogDecorator::LogonNumber()
{
    return  logCall0("LogonNumber", m_base, &IVPSTACK::LogonNumber);
}

void        VpStackDbgLogDecorator::SetSupportedBearersMask(
                    unsigned            bc_mask)
{
    logCallv1("SetSupportedBearersMask", m_base, &IVPSTACK::SetSupportedBearersMask, 
                        bc_mask);
}

void        VpStackDbgLogDecorator::SetAudioDataFactory(
                    VPAUDIODATAFACTORY* af)
{
    logCallv1("SetAudioDataFactory", m_base, &IVPSTACK::SetAudioDataFactory, 
                        af);
}

void        VpStackDbgLogDecorator::SetVideoDataFactory(
                    VPVIDEODATAFACTORY* vf)
{
    logCallv1("SetVideoDataFactory", m_base, &IVPSTACK::SetVideoDataFactory, 
                        vf);
}

int         VpStackDbgLogDecorator::SendKeypad(
                    VPCALL              vpcall,
                    int                 key)
{
    return  logCall2("SendKeypad", m_base, &IVPSTACK::SendKeypad, 
                        vpcall,
                        key);
}

int         VpStackDbgLogDecorator::GetStackType()
{
    return  logCall0("GetStackType", m_base, &IVPSTACK::GetStackType);
}

int         VpStackDbgLogDecorator::EnumCalls(
                    VPCALL*             vpcalls,
                    unsigned            maxvpcalls,
                    unsigned            mask)
{
    return  logCall3("EnumCalls", m_base, &IVPSTACK::EnumCalls, 
                        vpcalls,
                        maxvpcalls,
                        mask);
}

int         VpStackDbgLogDecorator::NetworkQuality()
{
    return  logCall0("NetworkQuality", m_base, &IVPSTACK::NetworkQuality);
}

int         VpStackDbgLogDecorator::GetQosData(
                    VPCALL              vpcall,
                    QOSDATA*            qos)
{
    return  logCall2("GetQosData", m_base, &IVPSTACK::GetQosData, 
                        vpcall,
                        qos);
}

int         VpStackDbgLogDecorator::GetAudioParameters(
                    VPCALL              vpcall,
                    int*                codec,
                    int*                framesperpacket)
{
    return  logCall3("GetAudioParameters", m_base, &IVPSTACK::GetAudioParameters, 
                        vpcall,
                        codec,
                        framesperpacket);
}

int         VpStackDbgLogDecorator::SetCodecsMask(
                    unsigned            mask)
{
    return  logCall1("SetCodecsMask", m_base, &IVPSTACK::SetCodecsMask, 
                        mask);
}

char*       VpStackDbgLogDecorator::StackName()
{
    return  logCall0("StackName", m_base, &IVPSTACK::StackName);
}

void        VpStackDbgLogDecorator::SetBindPort(
                    int                 port)
{
    logCallv1("SetBindPort", m_base, &IVPSTACK::SetBindPort,
                        port);
}

int         VpStackDbgLogDecorator::GetBindPort()
{
    return  logCall0("GetBindPort", m_base, &IVPSTACK::GetBindPort);
}

void        VpStackDbgLogDecorator::SetBandwidth(
                    int                 kbps)
{
    logCallv1("SetBandwidth", m_base, &IVPSTACK::SetBandwidth, 
                        kbps);
}

int         VpStackDbgLogDecorator::GetCallStatus(
                    VPCALL              vpcall)
{
    return  logCall1("GetCallStatus", m_base, &IVPSTACK::GetCallStatus, 
                        vpcall);
}

int         VpStackDbgLogDecorator::GetTransceiverBandwidths(
                    int*                rx,
                    int*                tx)
{
    return  logCall2("GetTransceiverBandwidths", m_base, &IVPSTACK::GetTransceiverBandwidths, 
                        rx,
                        tx);
}

int         VpStackDbgLogDecorator::GetConferencePeers(
                    VPCALL              vpcall,
                    VPCALL*             vpcalls,
                    int*                ncalls)
{
    return  logCall3("GetConferencePeers", m_base, &IVPSTACK::GetConferencePeers, 
                        vpcall,
                        vpcalls,
                        ncalls);
}

int         VpStackDbgLogDecorator::GetLinkedCall(
                    VPCALL              vpcall,
                    VPCALL*             linkedcall)
{
    return  logCall2("GetLinkedCall", m_base, &IVPSTACK::GetLinkedCall, 
                        vpcall,
                        linkedcall);
}

int         VpStackDbgLogDecorator::GetCallRemoteName(
                    VPCALL              vpcall,
                    char*               username)
{
    return  logCall2("GetCallRemoteName", m_base, &IVPSTACK::GetCallRemoteName, 
                        vpcall,
                        username);
}

int         VpStackDbgLogDecorator::GetCallRemoteNumber(
                    VPCALL              vpcall,
                    char*               number)
{
    return  logCall2("GetCallRemoteNumber", m_base, &IVPSTACK::GetCallRemoteNumber, 
                        vpcall,
                        number);
}

int         VpStackDbgLogDecorator::GetCallRemoteAddress(
                    VPCALL              vpcall,
                    char*               address)
{
    return  logCall2("GetCallRemoteAddress", m_base, &IVPSTACK::GetCallRemoteAddress, 
                        vpcall,
                        address);
}

int         VpStackDbgLogDecorator::GetCallRemoteSubAddr(
                    VPCALL              vpcall,
                    char*               subaddr)
{
    return  logCall2("GetCallRemoteSubAddr", m_base, &IVPSTACK::GetCallRemoteSubAddr, 
                        vpcall,
                        subaddr);
}

int         VpStackDbgLogDecorator::GetCallLocalName(
                    VPCALL              vpcall,
                    char*               username)
{
    return  logCall2("GetCallLocalName", m_base, &IVPSTACK::GetCallLocalName, 
                        vpcall,
                        username);
}

int         VpStackDbgLogDecorator::GetCallLocalNumber(
                    VPCALL              vpcall,
                    char*               number)
{
    return  logCall2("GetCallLocalNumber", m_base, &IVPSTACK::GetCallLocalNumber, 
                        vpcall,
                        number);
}

int         VpStackDbgLogDecorator::GetCallLocalSubAddr(
                    VPCALL              vpcall,
                    char*               subaddr)
{
    return  logCall2("GetCallLocalSubAddr", m_base, &IVPSTACK::GetCallLocalSubAddr, 
                        vpcall,
                        subaddr);
}

int         VpStackDbgLogDecorator::GetCallBearer(
                    VPCALL              vpcall)
{
    return  logCall1("GetCallBearer", m_base, &IVPSTACK::GetCallBearer, 
                        vpcall);
}

int         VpStackDbgLogDecorator::GetCallCodec(
                    VPCALL              vpcall)
{
    return  logCall1("GetCallCodec", m_base, &IVPSTACK::GetCallCodec, 
                        vpcall);
}

int         VpStackDbgLogDecorator::IsHeld(
                    VPCALL              vpcall)
{
    return  logCall1("IsHeld", m_base, &IVPSTACK::IsHeld, 
                        vpcall);
}

int         VpStackDbgLogDecorator::IsConferenced(
                    VPCALL              vpcall)
{
    return  logCall1("IsConferenced", m_base, &IVPSTACK::IsConferenced,
                        vpcall);
}


int         VpStackDbgLogDecorator::SetCallDisconnectMessage(
                    VPCALL              vpcall,
                    const char*         text)
{
    return logCall2("SetCallDisconnectMessage", m_base, &IVPSTACK::SetCallDisconnectMessage, 
                        vpcall,
                        text);
}

int         VpStackDbgLogDecorator::CallForwardingRequest(
                    int                 op,
                    const char*         number)
{
    return  logCall2("CallForwardingRequest", m_base, &IVPSTACK::CallForwardingRequest,
                        op,
                        number);
}

int         VpStackDbgLogDecorator::GetCallForwardingStatus(
                    int*                op,
                    char*               number)
{
    return  logCall2("GetCallForwardingStatus", m_base, &IVPSTACK::GetCallForwardingStatus,
                        op,
                        number);
}
int         VpStackDbgLogDecorator::SendChat(
                    VPCALL              vpcall,
                    const char*         text)
{
    return  logCall2("SendChat", m_base, &IVPSTACK::SendChat, 
                        vpcall,
                        text);
}

int         VpStackDbgLogDecorator::GetChatText(
                    VPCALL              vpcall,
                    char*               text,
                    int                 textsize)
{
    return  logCall3("GetChatText", m_base, &IVPSTACK::GetChatText, 
                        vpcall,
                        text,
                        textsize);
}

// SMS data
int         VpStackDbgLogDecorator::GetMissedCallsData(
                    VPCALL              vpcall,
                    unsigned*           missedcallscount,
                    unsigned*           missedcalltime,
                    unsigned*           missedcallbc)
{
    return  logCall4("GetMissedCallsData", m_base, &IVPSTACK::GetMissedCallsData, 
                        vpcall,
                        missedcallscount,
                        missedcalltime,
                        missedcallbc);
}

int         VpStackDbgLogDecorator::GetCallText(
                    VPCALL              vpcall,
                    char*               text)
{
    return  logCall2("GetCallText", m_base, &IVPSTACK::GetCallText, vpcall, text);
}

int         VpStackDbgLogDecorator::SetCallText(
                    VPCALL              vpcall,
                    const char*         text)
{
    return  logCall2("SetCallText", m_base, &IVPSTACK::SetCallText, vpcall, text);
}

int         VpStackDbgLogDecorator::SetMissedCallBC(
                    VPCALL              vpcall,
                    int                 bc)
{
    return  logCall2("SetMissedCallBC", m_base, &IVPSTACK::SetMissedCallBC, vpcall, bc);
}

int         VpStackDbgLogDecorator::SendAudio(
                    VPCALL              vpcall,
                    int                 codec,
                    unsigned            timestamp,
                    const void*         buf,
                    int                 len,
                    bool                wait)
{
    return  logCall6("SendAudio", m_base, &IVPSTACK::SendAudio, vpcall, codec, timestamp, buf, len, wait);
}

int         VpStackDbgLogDecorator::SendVideo(
                    VPCALL              vpcall,
                    unsigned short      timestamp,
                    void*               data,
                    int                 size,
                    bool                keyframe)
{
    return  logCall5("SendVideo", m_base, &IVPSTACK::SendVideo, 
                        vpcall, 
                        timestamp, 
                        data, 
                        size, 
                        keyframe);
}

int         VpStackDbgLogDecorator::AskOnline(
                    AOL*                aol)
{
    return  logCall1("AskOnline", m_base, &IVPSTACK::AskOnline, aol);
}

int         VpStackDbgLogDecorator::QueryAccountInfo()
{
    return  logCall0("QueryAccountInfo", m_base, &IVPSTACK::QueryAccountInfo);
}

int         VpStackDbgLogDecorator::ServerTransfer(
                    int                 op,
                    const char*         path,
                    bool                amactive)
{
    return  logCall3("ServerTransfer", m_base, &IVPSTACK::ServerTransfer,
                        op,
                        path,
                        amactive);
}

int         VpStackDbgLogDecorator::SendFile(
                    VPCALL              vpcall,
                    const char*         path)
{
    return  logCall2("SendFile", m_base, &IVPSTACK::SendFile,    vpcall,
                        path);
}

int         VpStackDbgLogDecorator::AbortFileTransfer(
                    VPCALL              vpcall)
{
    return  logCall1("AbortFileTransfer", m_base, &IVPSTACK::AbortFileTransfer, 
                        vpcall);
}

int         VpStackDbgLogDecorator::AcceptFile(
                    VPCALL              vpcall,
                    int                 accept)
{
    return  logCall2("AcceptFile", m_base, &IVPSTACK::AcceptFile,  vpcall,
                        accept);
}

int         VpStackDbgLogDecorator::GetCallFilePath(
                    VPCALL              vpcall,
                    char*               path)
{
    return  logCall2("GetCallFilePath", m_base, &IVPSTACK::GetCallFilePath, 
                        vpcall,
                        path);
}

int         VpStackDbgLogDecorator::SetCallFilePath(
                    VPCALL              vpcall,
                    const char*         path)
{
    return  logCall2("SetCallFilePath", m_base, &IVPSTACK::SetCallFilePath, 
                        vpcall,
                        path);
}

int         VpStackDbgLogDecorator::SetCallNFiles(
                    VPCALL              vpcall,
                    unsigned            nfiles,
                    unsigned            nbytes)
{
    return  logCall3("SetCallNFiles", m_base, &IVPSTACK::SetCallNFiles, 
                        vpcall,
                        nfiles,
                        nbytes);
}

int         VpStackDbgLogDecorator::GetCallNFiles(
                    VPCALL              vpcall,
                    unsigned*           nfiles,
                    unsigned*           nbytes)
{
    return  logCall3("GetCallNFiles", m_base, &IVPSTACK::GetCallNFiles, 
                        vpcall,
                        nfiles,
                        nbytes);
}

int         VpStackDbgLogDecorator::GetFileProgress(
                    VPCALL              vpcall,
                    unsigned*           current,
                    unsigned*           total)
{
    return  logCall3("GetFileProgress", m_base, &IVPSTACK::GetFileProgress, 
                        vpcall,
                        current,
                        total);
}

int         VpStackDbgLogDecorator::SetDefaultVideoParameters(
                    unsigned            fourcc,
                    unsigned            width,
                    unsigned            height,
                    unsigned            framerate,
                    unsigned            quality)
{
    return  logCall5("SetDefaultVideoParameters", m_base, &IVPSTACK::SetDefaultVideoParameters,
                        fourcc,
                        width,
                        height,
                        framerate,
                        quality);
}

int         VpStackDbgLogDecorator::GetDefaultVideoParameters(
                    unsigned*           fourcc,
                    unsigned*           width,
                    unsigned*           height,
                    unsigned*           framerate,
                    unsigned*           quality)
{
    return  logCall5("GetDefaultVideoParameters", m_base, &IVPSTACK::GetDefaultVideoParameters,
                        fourcc,
                        width,
                        height,
                        framerate,
                        quality);
}

int         VpStackDbgLogDecorator::SetCallVideoParameters(
                    VPCALL              vpcall,
                    unsigned            fourcc,
                    unsigned            width,
                    unsigned            height,
                    unsigned            framerate,
                    unsigned            quality)
{
    return  logCall6("SetCallVideoParameters", m_base, &IVPSTACK::SetCallVideoParameters,
                        vpcall,
                        fourcc,
                        width,
                        height,
                        framerate,
                        quality);
}

int         VpStackDbgLogDecorator::GetCallVideoParameters(
                    VPCALL              vpcall,
                    unsigned*           fourcc,
                    unsigned*           width,
                    unsigned*           height,
                    unsigned*           framerate,
                    unsigned*           quality)
{
    return  logCall6("GetCallVideoParameters", m_base, &IVPSTACK::GetCallVideoParameters,
                        vpcall,
                        fourcc,
                        width,
                        height,
                        framerate,
                        quality);
}

int         VpStackDbgLogDecorator::RecordAVIFile(
                    const char*         path)
{
    return  logCall1("RecordAVIFile", m_base, &IVPSTACK::RecordAVIFile,
                        path);
}

int         VpStackDbgLogDecorator::StopAVIRecord()
{
    return  logCall0("StopAVIRecord", m_base, &IVPSTACK::StopAVIRecord);
}

int         VpStackDbgLogDecorator::GetRecorderCall(
                    VPCALL*             vpcall)
{
    return  logCall1("GetRecorderCall", m_base, &IVPSTACK::GetRecorderCall,
                        vpcall);
}

int         VpStackDbgLogDecorator::SetVideoWindowData(
                    VPCALL              vpcall,
                    void*               hParent,
                    int                 childid,
                    const RECTANGLE*    rect,
                    bool                hidden)
{
    return  logCall5("SetVideoWindowData", m_base, &IVPSTACK::SetVideoWindowData,
                        vpcall,
                        hParent,
                        childid,
                        rect,
                        hidden);
}

int         VpStackDbgLogDecorator::SetVideoWindowFullScreen(
                    VPCALL              vpcall,
                    int                 fs)
{
    return  logCall2("SetVideoWindowFullScreen", m_base, &IVPSTACK::SetVideoWindowFullScreen,
                        vpcall,
                        fs);
}

int         VpStackDbgLogDecorator::CaptureVideoImage(
                    VPCALL              vpcall,
                    void**              image,
                    int*                w,
                    int*                h)
{
    return  logCall4("CaptureVideoImage", m_base, &IVPSTACK::CaptureVideoImage,
                        vpcall,
                        image,
                        w,
                        h);
}

int         VpStackDbgLogDecorator::AcceptConference(
                    VPCALL              vpcall,
                    bool                accept)
{
    return  logCall2("AcceptConference", m_base, &IVPSTACK::AcceptConference,
                        vpcall,
                        accept);
}

int         VpStackDbgLogDecorator::SignalLevel(
                    int*                dBm)
{
    return  logCall1("SignalLevel", m_base, &IVPSTACK::SignalLevel,
                        dBm);
}

int         VpStackDbgLogDecorator::GetOperatorName(
                    char*               name,
                    int                 namesize,
                    unsigned*           flags)
{
    return  logCall3("GetOperatorName", m_base, &IVPSTACK::GetOperatorName,
                        name,
                        namesize,
                        flags);
}

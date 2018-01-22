#ifndef VPSTACKDBGLOGDECORATOR_H
#define VPSTACKDBGLOGDECORATOR_H

#include "VpStackDecorator.h"
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QSemaphore>

class VpStackDbgLogDecorator
    : public VpStackDecorator
{
    public:
        explicit        VpStackDbgLogDecorator(
                            IVPSTACK*           base,
                            QObject*            parent  = 0);
                        ~VpStackDbgLogDecorator();

    //  the interface
    //  ---------------------------------------------------------------------
        int         SetNotifyRoutine(
                            void                (*notify)(void *param, unsigned, unsigned, unsigned),
                            void*               param);
        int         SetSyncNotifyRoutine(
                            int                 (*notify)(void *param, unsigned, unsigned, unsigned),
                            void*               param);
        VPCALL      CreateCall();

        void        FreeCall(
                            VPCALL              vpcall);
        int         Connect(VPCALL*             vpcall,
                            const char*         address,
                            int                 bc);
        int         Connect(VPCALL              vpcall,
                            const char*         address,
                            int                 bc);
        int         SetCallAddress(
                            VPCALL              vpcall,
                            const char*         address);
        int         SetCallLocalName(
                            VPCALL              vpcall,
                            const char*         username);
        int         SetCallLocalNumber(
                            VPCALL              vpcall,
                            const char*         number);
        int         Disconnect(
                            VPCALL              vpcall,
                            int                 reason);
        int         AnswerCall(
                            VPCALL              vpcall);
        int         Hold(   VPCALL              vpcall);
        int         Resume( VPCALL              vpcall);

        int         ConferenceCalls(
                            VPCALL*             vpcalls,
                            int                 ncalls,
                            bool                detach);
        int         AddToConference(
                            VPCALL              vpcall);
        int         RemoveFromConference(
                            VPCALL              vpcall);
        int         ConferenceAll();
        int         StopConference();

        int         AcceptConference(
                            VPCALL              vpcall,
                            bool                accept);

        int         SendAudioMessage(
                            VPCALL              vpcall,
                            const char*         file,
                            unsigned            msgid);
        int         SetServers(
                            const char*         list);
        int         Logon(  const char*         username,
                            const char*         password);
        int         Logon(  const char*         serveraccesscode,
                            const unsigned char*accountsecret);
        int         GetLogonData(
                            char*               serveraccesscode,
                            unsigned char*      accountsecret);
        int         Logoff();
        int         IsLoggedOn();
        const char* LogonName();
        const char* LogonNumber();

        void        SetSupportedBearersMask(
                            unsigned            bc_mask);
        void        SetAudioDataFactory(
                            VPAUDIODATAFACTORY* af);
        void        SetVideoDataFactory(
                            VPVIDEODATAFACTORY* vf);

        int         SendKeypad(
                            VPCALL              vpcall,
                            int                 key);
        int         GetStackType();
        int         EnumCalls(
                            VPCALL*             vpcalls,
                            unsigned            maxvpcalls,
                            unsigned            mask);
        int         NetworkQuality();
        int         GetQosData(
                            VPCALL              vpcall,
                            QOSDATA*            qos);
        int         GetAudioParameters(
                            VPCALL              vpcall,
                            int*                codec,
                            int*                framesperpacket);
        int         SetCodecsMask(
                            unsigned            mask);
        char*       StackName();
        void        SetBindPort(
                            int                 port);
        int         GetBindPort();
        void        SetBandwidth(
                            int                 kbps);
        int         GetCallStatus(
                            VPCALL              vpcall);
        int         GetTransceiverBandwidths(
                            int*                rx,
                            int*                tx);

        int         GetConferencePeers(
                            VPCALL              vpcall,
                            VPCALL*             vpcalls,
                            int*                ncalls);
        int         GetLinkedCall(
                            VPCALL              vpcall,
                            VPCALL*             linkedcall);
        int         GetCallRemoteName(
                            VPCALL              vpcall,
                            char*               username);
        int         GetCallRemoteNumber(
                            VPCALL              vpcall,
                            char*               number);
        int         GetCallRemoteAddress(
                            VPCALL              vpcall,
                            char*               address);
        int         GetCallRemoteSubAddr(
                            VPCALL              vpcall,
                            char*               subaddr);
        int         GetCallLocalName(
                            VPCALL              vpcall,
                            char*               username);
        int         GetCallLocalNumber(
                            VPCALL              vpcall,
                            char*               number);
        int         GetCallLocalSubAddr(
                            VPCALL              vpcall,
                            char*               subaddr);
        int         GetCallBearer(
                            VPCALL              vpcall);
        int         GetCallCodec(
                            VPCALL              vpcall);
        int         IsHeld( VPCALL              vpcall);
        int         IsConferenced(
                            VPCALL              vpcall);
        int         SetCallDisconnectMessage(
                            VPCALL              vpcall,
                            const char*         text);
        int         CallForwardingRequest(
                            int                 op,
                            const char*         number);
        int         GetCallForwardingStatus(
                            int*                op,
                            char*               number);
        int         SendChat(
                            VPCALL              vpcall,
                            const char*         text);
        int         GetChatText(
                            VPCALL              vpcall,
                            char*               text,
                            int                 textsize);

        int         GetMissedCallsData(
                            VPCALL              vpcall,
                            unsigned*           missedcallscount,
                            unsigned*           missedcalltime,
                            unsigned*           missedcallbc);
        int         GetCallText(
                            VPCALL              vpcall,
                            char*               text);
        int         SetCallText(
                            VPCALL              vpcall,
                            const char*         text);
        int         SetMissedCallBC(
                            VPCALL              vpcall,
                            int                 bc);

        int         SendAudio(
                            VPCALL              vpcall,
                            int                 codec,
                            unsigned            timestamp,
                            const void*         buf,
                            int                 len,
                            bool                wait);
        int         SendVideo(
                            VPCALL              vpcall,
                            unsigned short      timestamp,
                            void*               data,
                            int                 size,
                            bool                keyframe);
        int         AskOnline(
                            AOL*                aol);
        int         QueryAccountInfo();
        int         ServerTransfer(
                            int                 op,
                            const char*         path,
                            bool                amactive);
        int         SendFile(
                            VPCALL              vpcall,
                            const char*         path);
        int         AbortFileTransfer(
                            VPCALL              vpcall);
        int         AcceptFile(
                            VPCALL              vpcall,
                            int                 accept);
        int         GetCallFilePath(
                            VPCALL              vpcall,
                            char*               path);
        int         SetCallFilePath(
                            VPCALL              vpcall,
                            const char*         path);
        int         SetCallNFiles(
                            VPCALL              vpcall,
                            unsigned            nfiles,
                            unsigned            nbytes);
        int         GetCallNFiles(
                            VPCALL              vpcall,
                            unsigned*           nfiles,
                            unsigned*           nbytes);
        int         GetFileProgress(
                            VPCALL              vpcall,
                            unsigned*           current,
                            unsigned*           total);
        int         SetDefaultVideoParameters(
                            unsigned            fourcc,
                            unsigned            width,
                            unsigned            height,
                            unsigned            framerate,
                            unsigned            quality);
        int         GetDefaultVideoParameters(
                            unsigned*           fourcc,
                            unsigned*           width,
                            unsigned*           height,
                            unsigned*           framerate,
                            unsigned*           quality);
        int         SetCallVideoParameters(
                            VPCALL              vpcall,
                            unsigned            fourcc,
                            unsigned            width,
                            unsigned            height,
                            unsigned            framerate,
                            unsigned            quality);
        int         GetCallVideoParameters(
                            VPCALL              vpcall,
                            unsigned*           fourcc,
                            unsigned*           width,
                            unsigned*           height,
                            unsigned*           framerate,
                            unsigned*           quality);
        int         RecordAVIFile(
                            const char*         path);
        int         StopAVIRecord();
        int         GetRecorderCall(
                            VPCALL*             vpcall);
        int         SetVideoWindowData(
                            VPCALL              vpcall,
                            void*               hParent,
                            int                 childid,
                            const RECTANGLE*    rect,
                            bool                hidden);
        int         SetVideoWindowFullScreen(
                            VPCALL              vpcall,
                            int                 fs);
        int         CaptureVideoImage(
                            VPCALL              vpcall,
                            void**              image,
                            int*                w,
                            int*                h);
        int         SignalLevel(
                            int*                dBm);
        int         GetOperatorName(
                            char*               name,
                            int                 namesize,
                            unsigned*           flags);



    private:
        template<typename R>
        R               logCall0(   const char*         name,
                                    IVPSTACK*           object,
                                    R                   (IVPSTACK::*member)());
        void            logCallv0(  const char*         name,
                                    IVPSTACK*           object,
                                    void                (IVPSTACK::*member)());
        template<typename R, typename T1>
        R               logCall1(   const char*         name,
                                    IVPSTACK*           object,
                                    R                   (IVPSTACK::*member)(T1),
                                    T1                  t1);
        template<typename T1>
        void            logCallv1(  const char*         name,
                                    IVPSTACK*           object,
                                    void                (IVPSTACK::*member)(T1),
                                    T1                  t1);
        template<typename R, typename T1, typename T2>
        R               logCall2(   const char*         name,
                                    IVPSTACK*           object,
                                    R                   (IVPSTACK::*member)(T1, T2),
                                    T1                  t1,
                                    T2                  t2);
        template<typename R, typename T1, typename T2, typename T3>
        R               logCall3(   const char*         name,
                                    IVPSTACK*           object,
                                    R                   (IVPSTACK::*member)(T1, T2, T3),
                                    T1                  t1,
                                    T2                  t2,
                                    T3                  t3);

        template<typename R, typename T1, typename T2, typename T3, typename T4>
        R               logCall4(   const char*         name,
                                    IVPSTACK*           object,
                                    R                   (IVPSTACK::*member)(T1, T2, T3, T4),
                                    T1                  t1,
                                    T2                  t2,
                                    T3                  t3,
                                    T4                  t4);

        template<typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
        R               logCall5(   const char*         name,
                                    IVPSTACK*           object,
                                    R                   (IVPSTACK::*member)(T1, T2, T3, T4, T5),
                                    T1                  t1,
                                    T2                  t2,
                                    T3                  t3,
                                    T4                  t4,
                                    T5                  t5);

        template<typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
        R               logCall6(   const char*         name,
                                    IVPSTACK*           object,
                                    R                   (IVPSTACK::*member)(T1, T2, T3, T4, T5, T6),
                                    T1                  t1,
                                    T2                  t2,
                                    T3                  t3,
                                    T4                  t4,
                                    T5                  t5,
                                    T6                  t6);

        void            vpstackNotificationRoutine(
                                    unsigned            msg,
                                    unsigned            pcall,
                                    unsigned            param);
        int             vpstackSyncNotifyRoutine(
                                    unsigned            msg,
                                    unsigned            param1,
                                    unsigned            param2);
    private:
        static
        QFile*              s_file;
        static
        QTextStream         s_stream;
        static
        QSemaphore          s_semaphore;

        QMap<int, QString>  m_msg2str;
        QString             m_name;
};

#endif // VPSTACKDBGLOGDECORATOR_H

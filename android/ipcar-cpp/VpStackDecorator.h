#ifndef VPSTACKDECORATOR_H
#define VPSTACKDECORATOR_H

#include "../vpstack/vpstack.h"

#include <QtCore/QObject>

#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>

class VpStackDecorator
    : public QObject
    , public IVPSTACK
{
    Q_OBJECT

    public:
        explicit    VpStackDecorator(
                            IVPSTACK*           base,
                            QObject*            parent  = 0);
        virtual     ~VpStackDecorator()
        {
            if(m_base)
                delete m_base;
        }

        int         Init();
        int         SetNotifyRoutine(
                            void                (*notify)(void *param, unsigned, unsigned, unsigned),
                            void*               param);
        int         SetSyncNotifyRoutine(
                            int                 (*notify)(void *param, unsigned, unsigned, unsigned),
                            void*               param);
        // Create a call
        VPCALL      CreateCall();
        // Free a call
        void        FreeCall(
                            VPCALL              vpcall);
        // Attempt a connection with address, bearer bc, on success returns a VPCALL
        int         Connect(VPCALL*             vpcall,
                            const char*         address,
                            int                 bc);
        // Attempt a connection with an already allocated call
        int         Connect(VPCALL              vpcall,
                            const char*         address,
                            int                 bc);
        // Set the destination address of a call
        int         SetCallAddress(
                            VPCALL              vpcall,
                            const char*         address);
        // Set the source of the call
        int         SetCallLocalName(
                            VPCALL              vpcall,
                            const char*         username);
        int         SetCallLocalNumber(
                            VPCALL              vpcall,
                            const char*         number);
        // Disconnect the call
        int         Disconnect(
                            VPCALL              vpcall,
                            int                 reason);
        // Answer a call in alerted state
        int         AnswerCall(
                            VPCALL              vpcall);
        int         Hold(   VPCALL              vpcall);
        int         Resume( VPCALL              vpcall);

        // Conferences and call transfers, detach detaches the calls from us after setup complete (for call transfer)
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

        // Auto attendant
        int         SendAudioMessage(
                            VPCALL              vpcall,
                            const char*         file,
                            unsigned            msgid);
        // Servers
        int         SetServers(
                            const char*         list);
        int         Logon(  const char*         username,
                            const char*         password);
        int         GetLogonData(
                            char*               serveraccesscode,
                            unsigned char*      accountsecret);
        int         Logon(  const char*         serveraccesscode,
                            const unsigned char*accountsecret);

        int         Logoff();
        int         IsLoggedOn();
        const char* LogonName();
        const char* LogonNumber();

        // Setup
        void        SetSupportedBearersMask(
                            unsigned            bc_mask);
        void        SetAudioDataFactory(
                            VPAUDIODATAFACTORY* af);
        void        SetVideoDataFactory(
                            VPVIDEODATAFACTORY* vf);

        // Misc
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

        // Get information about calls, ncalls contains the size of the vpcalls array on input,
        // the number of filled entries on output
        int         GetConferencePeers(
                            VPCALL              vpcall,
                            VPCALL*             vpcalls,
                            int*                ncalls);
        // Get the call that will be disconnected when vpcall is disconnected; when linkedcall
        // becomes established, it is connected to vpcall and then the two calls detached from us
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
        int         GetCallTimes(
                            VPCALL              vpcall,
                            time_t*             start_time,
                            unsigned*           length_ms);
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
        int         MeasureBandwidth();
        int         SendChat(
                            VPCALL              vpcall,
                            const char*         text);
        int         GetChatText(
                            VPCALL              vpcall,
                            char*               text,
                            int                 textsize);

        // SMS data
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

    protected:
        virtual
        void            vpstackNotificationRoutine(
                                    unsigned            msg,
                                    unsigned            pcall,
                                    unsigned            param);
        virtual
        int             vpstackSyncNotifyRoutine(
                                    unsigned            msg,
                                    unsigned            param1,
                                    unsigned            param2);

    private slots:
        void            vpstackNotificationRoutineSlot(
                                    unsigned            msg,
                                    unsigned            pcall,
                                    unsigned            param);
        void            vpstackSyncNotifyRoutineSlot(
                                    unsigned            msg,
                                    unsigned            param1,
                                    unsigned            param2);

    private:
        static
        void            vpstackNotificationRoutine(
                                    void*               uparam,
                                    unsigned            msg,
                                    unsigned            pcall,
                                    unsigned            param);
        static
        int             vpstackSyncNotifyRoutine(
                                    void*               uparam,
                                    unsigned            msg,
                                    unsigned            param1,
                                    unsigned            param2);

        void            addCall(    VPCALL              call);
        void            updateCallType(
                                    VPCALL              call,
                                    int                 type);

    protected:
        void            DefNotificationRoutine(
                                    unsigned int        msg,
                                    unsigned int        pcall,
                                    unsigned int        param);
        int             DefSyncNotifyRoutine(
                                    unsigned int        msg,
                                    unsigned int        param1,
                                    unsigned int        param2);

    protected:
        IVPSTACK*       m_base;

    private:
        void            (*m_notify)(void*, unsigned, unsigned, unsigned);
        void*           m_notifyParam;

        static
        QMutex          m_mutex;
        static
        QWaitCondition  m_done;
        static
        int             m_syncResult;
        int             (*m_sync)(  void*, unsigned, unsigned, unsigned);
        void*           m_syncParam;
};

#endif // VPSTACKDECORATOR_H

#include "VpStackSoundDecorator.h"
#include "Utils.h"

//  VpStackSoundDecorator::VpStackSoundDecorator
//  -------------------------------------------------------------------------
VpStackSoundDecorator::VpStackSoundDecorator(
                    IVPSTACK*           base,
                    QObject*            parent)
    : VpStackDecorator(base, parent)
    , m_dialing(getFilename("sounds/dialing.wav"))

    , m_ring(getFilename("sounds/ring.wav"))
    , m_chatRing(getFilename("sounds/chatRing.wav"))
    , m_ftRing(getFilename("sounds/ftRing.wav"))
    , m_videoRing(getFilename("sounds/videoRing.wav"))

    , m_callHeld(getFilename("sounds/callHeld.wav"))
    , m_callResumed(getFilename("sounds/callResumed.wav"))
{
    m_dialing.setLoops(-1);

    m_ring.setLoops(-1);
    m_chatRing.setLoops(-1);
    m_ftRing.setLoops(-1);
    m_videoRing.setLoops(-1);

    m_callHeld.setLoops(-1);
    m_callResumed.setLoops(1);
}

//  VpStackSoundDecorator::vpstackNotificationRoutine
//  -------------------------------------------------------------------------
void        VpStackSoundDecorator::vpstackNotificationRoutine(
                            unsigned            msg,
                            unsigned            pcall,
                            unsigned            param)
{
    VPCALL  call    = (VPCALL)pcall;
    int     bc      = m_base->GetCallBearer(call);

    switch(msg)
    {
        case    VPMSG_NEWCALL       :
        {
            switch(bc)
            {
                case    BC_CHAT :
                {
                    m_chatRing.play();
                } break;

                case    BC_FILE :
                {
                    m_ftRing.play();
                } break;

                case    BC_VIDEO:
                case    BC_AUDIOVIDEO   :
                {
                    m_videoRing.play();
                } break;

                case    BC_SMS  :
                break;

                default         :
                {
                    m_ring.play();
                }
            };
        } break;

        case    VPMSG_CALLENDED :
        {
            switch(bc)
            {
                case    BC_CHAT :
                {
                    m_chatRing.stop();
                } break;

                case    BC_FILE :
                {
                    m_ftRing.stop();
                } break;

                case    BC_VIDEO:
                case    BC_AUDIOVIDEO   :
                {
                    m_videoRing.stop();
                } break;

                case    BC_SMS  :
                break;

                default         :
                {
                    m_ring.stop();
                }
            };
        } break;

        case    VPMSG_CALLACCEPTED  :
        {
            m_dialing.stop();
        } break;

        case    VPMSG_CALLREFUSED   :
        {
            m_dialing.stop();
        } break;

        case    VPMSG_REMOTELYHELD  :
        {
            m_callHeld.play();
        } break;

        case    VPMSG_REMOTELYRESUMED   :
        {
            m_callResumed.play();
        } break;
    }

    DefNotificationRoutine(msg, pcall, param);
}

//  VpStackSoundDecorator::vpstackSyncNotifyRoutine
//  -------------------------------------------------------------------------
int         VpStackSoundDecorator::vpstackSyncNotifyRoutine(
                            unsigned            msg,
                            unsigned            param1,
                            unsigned            param2)
{
    return      DefSyncNotifyRoutine(msg, param1, param2);
}

//  VpStackSoundDecorator::Connect
//  -------------------------------------------------------------------------
int         VpStackSoundDecorator::Connect(
                    VPCALL*             vpcall,
                    const char*         address,
                    int                 bc)
{
    int result  = 0;

    result  = m_base->Connect(vpcall, address, bc);

    if(result == 0 && bc != BC_NOTHING && bc != BC_SMS)
    {
        m_dialing.play();
    }

    return  result;
}

//  VpStackSoundDecorator::Connect
//  -------------------------------------------------------------------------
int         VpStackSoundDecorator::Connect(
                    VPCALL              vpcall,
                    const char*         address,
                    int                 bc)
{
    int result  = 0;

    result  = m_base->Connect(vpcall, address, bc);

    if(result == 0 && bc != BC_NOTHING && bc != BC_SMS)
    {
        m_dialing.play();
    }

    return  result;
}

//  VpStackSoundDecorator::AnswerCall
//  -------------------------------------------------------------------------
int         VpStackSoundDecorator::AnswerCall(
                    VPCALL              vpcall)
{
    m_ring.stop();
    m_chatRing.stop();
    m_ftRing.stop();
    m_videoRing.stop();

    return  m_base->AnswerCall(vpcall);
}

//  VpStackSoundDecorator::Disconnect
//  -------------------------------------------------------------------------
int         VpStackSoundDecorator::Disconnect(
                    VPCALL              vpcall,
                    int                 reason)
{
    m_dialing.stop();

    m_ring.stop();
    m_chatRing.stop();
    m_ftRing.stop();
    m_videoRing.stop();

    m_callHeld.stop();

    return  m_base->Disconnect(vpcall, reason);
}



#ifndef VPSTACKSOUNDDECORATOR_H
#define VPSTACKSOUNDDECORATOR_H

#include "VpStackDecorator.h"

#ifndef   ANDROID_VERSION
#include <QtGui/QSound>
#else
#include "QSound.h"
#endif

class VpStackSoundDecorator
    : public VpStackDecorator
{
    public:
        explicit    VpStackSoundDecorator(
                            IVPSTACK*           base,
                            QObject*            parent  = 0);

        int         Connect(VPCALL*             vpcall,
                            const char*         address,
                            int                 bc);
        int         Connect(VPCALL              vpcall,
                            const char*         address,
                            int                 bc);

        int         AnswerCall(
                            VPCALL              vpcall);
        int         Disconnect(
                            VPCALL              vpcall,
                            int                 reason);

    private:
        void            vpstackNotificationRoutine(
                                    unsigned            msg,
                                    unsigned            pcall,
                                    unsigned            param);
        int             vpstackSyncNotifyRoutine(
                                    unsigned            msg,
                                    unsigned            param1,
                                    unsigned            param2);
    private:
        QSound      m_dialing;

        QSound      m_ring;
        QSound      m_chatRing;
        QSound      m_ftRing;
        QSound      m_videoRing;

        QSound      m_callHeld;
        QSound      m_callResumed;
};

#endif // VPSTACKSOUNDDECORATOR_H

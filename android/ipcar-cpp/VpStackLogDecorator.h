#ifndef VPSTACKLOGDECORATOR_H
#define VPSTACKLOGDECORATOR_H

#include "VpStackDecorator.h"
#include "LogEntry.h"

#include <QtCore/QObject>
#include <QtCore/QMap>

class VpStackLogDecorator
    : public VpStackDecorator
{
    public:
        explicit VpStackLogDecorator(
                            IVPSTACK*           base,
                            QObject*            parent = 0);

        // Disconnect the call
        int         Disconnect(
                            VPCALL              vpcall,
                            int                 reason);
        // Answer a call in alerted state
        int         AnswerCall(
                            VPCALL              vpcall);
        int         SetCallFilePath(
                            VPCALL              vpcall,
                            const char*         path);

    signals:

    public slots:

    private:
        void            vpstackNotificationRoutine(
                                    unsigned            msg,
                                    unsigned            pcall,
                                    unsigned            param);

    private:
        void            addCall(    VPCALL              call);
        void            updateCallType(
                                    VPCALL              call,
                                    int                 type);

    private:
        QMap<VPCALL, int>
                        m_callid;
        int             m_userid;

//        QString         m_password;
//        bool            m_loggedon;

};

#endif // VPSTACKLOGDECORATOR_H

#ifndef VPSTACKANSWERINGMACHINEDECORATOR_H
#define VPSTACKANSWERINGMACHINEDECORATOR_H
#include "VpStackDecorator.h"

#include <QtCore/QSet>
#include <QtCore/QMap>
#include <QtCore/QTimer>

class VpStackAnsweringMachineDecorator
    : public VpStackDecorator
{
    Q_OBJECT

    public:
        explicit    VpStackAnsweringMachineDecorator(
                            IVPSTACK*           base,
                            QObject*            parent  = 0);

    private:
        void            vpstackNotificationRoutine(
                                    unsigned int        msg,
                                    unsigned int        pcall,
                                    unsigned int        param);

        int             AnswerCall(VPCALL vpcall);
        int             Disconnect(VPCALL vpcall, int reason);

    private slots:
        void            onTimeout();

    private:
        QSet<VPCALL>            m_calls;
        QMap<VPCALL, VPCALL>    m_deflect;
        QMap<VPCALL, QTimer*>   m_call2timer;
};

#endif // VPSTACKANSWERINGMACHINEDECORATOR_H

#ifndef VPSTACKAOLDECORATOR_H
#define VPSTACKAOLDECORATOR_H

#include "VpStackDecorator.h"

#include <QtCore/QList>

class VpStackAolDecorator
    : public VpStackDecorator
{
    public:
        explicit    VpStackAolDecorator(
                            IVPSTACK*           base,
                            QObject*            parent = 0);
        int         AskOnline(
                            AOL*                aol);


    protected:
        void        vpstackNotificationRoutine(
                                    unsigned            msg,
                                    unsigned            pcall,
                                    unsigned            param);
        QList<AOL*> m_fifo;
};

#endif // VPSTACKAOLDECORATOR_H

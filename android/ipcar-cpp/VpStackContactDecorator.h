#ifndef VPSTACKContactDECORATOR_H
#define VPSTACKContactDECORATOR_H
#include "VpStackDecorator.h"

#include <QtCore/QString>
#include <QtCore/QList>

typedef struct
{
    int     op;
    QString path;
    bool    amactive;
} serverTransfer_t;

class VpStackContactDecorator
    : public VpStackDecorator
{
    public:
                    VpStackContactDecorator(
                                    IVPSTACK*           base,
                                    QObject*            parent  = 0);

        int         ServerTransfer(
                                    int                 op,
                                    const char*         path,
                                    bool                amactive);

    private:
        void        vpstackNotificationRoutine(
                                    unsigned            msg,
                                    unsigned            pcall,
                                    unsigned            param);


        void        rxAbook(        const QString&      file);

    private:
        QList<serverTransfer_t> m_rxlist;
        QList<serverTransfer_t> m_txlist;
};

#endif // VPSTACKContactDECORATOR_H

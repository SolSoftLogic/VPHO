#ifndef CALLHANDLERWIDGET_H
#define CALLHANDLERWIDGET_H

#include "CallHandler.h"

#include <QtGui/QWidget>
#include <QtCore/QString>

class   CallHandlerWidget
    : public QWidget
    , public CallHandler
{
    Q_OBJECT
    
    public:
        explicit    CallHandlerWidget(  
                        QWidget*            parent = 0);

    public:
        virtual
        void    call(   const QString&      username) = 0;

    signals:
        void    makeCall(
                        int                 call,
                        const QString&      username,
                        int                 bearer);
        void    makeCall(
                        const QString&      username,
                        int                 bearer);

        void    calling(int                 call,   // 0 if do not care
                        const QString&      username,
                        int                 bearer);

};

#endif // CALLHANDLER_H

#ifndef CALLSESSIONTHREAD_H
#define CALLSESSIONTHREAD_H

#include <QtCore/QThread>
#include <QtCore/QSemaphore>

class CallSessionThread
    : public QThread
{
    Q_OBJECT
    public:
        explicit CallSessionThread(QObject *parent = 0);

    signals:
        void    connected();
        void    disconnected();
        void    timeout();

    public slots:
        void    connect();
        void    disconnect();

    protected:
        QSemaphore  m_start;

        void        run();


};

#endif // CALLSESSIONTHREAD_H

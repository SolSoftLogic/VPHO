#include "CallSessionThread.h"
#include <QtCore/QMutex>

int     g_counter = 0;
QMutex  g_mutex;

CallSessionThread::CallSessionThread(QObject *parent)
    : QThread(parent)
    , m_start(0)
{

}

//  CallSessionThread::run
//  -------------------------------------------------------------------------
void    CallSessionThread::run()
{

    while(1)
    {
        g_mutex.lock();
        int counter = g_counter++;
        g_mutex.unlock();

        m_start.acquire();

        sleep(2);

        if(counter % 2 == 0)
        {
            emit connected();
        }
        else
        {
            emit timeout();
        }
    }
}

//  CallSessionThread::connect
//  -------------------------------------------------------------------------
void    CallSessionThread::connect()
{
    m_start.release();
}

//  CallSessionThread::disconnect
//  -------------------------------------------------------------------------
void    CallSessionThread::disconnect()
{
}

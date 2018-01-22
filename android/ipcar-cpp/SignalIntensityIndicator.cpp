#include "SignalIntensityIndicator.h"
#include "Utils.h"

//  SignalIntensityThread::SignalIntensityThread
//  -------------------------------------------------------------------------
SignalIntensityThread::SignalIntensityThread(QObject *parent)
    : QThread(parent)
{
}

//  SignalIntensityThread::run
//  -------------------------------------------------------------------------
void    SignalIntensityThread::run()
{
    while(1)
    {
        sleep(1);

        changed(vpstack()->NetworkQuality());
    }
}

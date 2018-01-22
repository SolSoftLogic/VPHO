#include "ConnectionStatusIndicator.h"
#include "Utils.h"

//  ConnectionStatusIndicator::ConnectionStatusIndicator
//  -------------------------------------------------------------------------
ConnectionStatusIndicator::ConnectionStatusIndicator(QWidget *parent)
    : QWidget(parent)
{
    bool    result;

    this->setObjectName(QString::number((uint)this));

    ConnectionStatusThread*  thread  = new ConnectionStatusThread;

    result  = connect(thread, SIGNAL(changed(uint)), this, SLOT(onChanged(uint)));
    Q_ASSERT(result);

    thread->start();
}

//  ConnectionStatusIndicator::setSignalIntenisty
//  -------------------------------------------------------------------------
void    ConnectionStatusIndicator::onChanged(uint led)
{
    QString path[]  =
    {
        getFilename("qss/connectionStatusIndicator/ledOnline.png"),
        getFilename("qss/connectionStatusIndicator/ledActive.png"),
        getFilename("qss/connectionStatusIndicator/ledOffline.png"),
        getFilename("qss/connectionStatusIndicator/ledPrivacy.png"),
        getFilename("qss/connectionStatusIndicator/ledOff.png")
    };
    Q_ASSERT(led < 5);

    setStyleSheet(QString("#%1{background-image : url(%2);}").arg(QString::number((uint)this)).arg(path[led]));
    repaint();
}

//  ConnectionStatusThread::ConnectionStatusThread
//  -------------------------------------------------------------------------
ConnectionStatusThread::ConnectionStatusThread(QObject *parent)
    : QThread(parent)
{
}

//  SignalIntensityThread::run
//  -------------------------------------------------------------------------
void    ConnectionStatusThread::run()
{
    int status   = 0;
    while(1)
    {
        sleep(1);
        changed(status++);

        status  %= 5;
    }
}

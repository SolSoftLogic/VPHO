#ifndef CONNECTIONSTATUSINDICATOR_H
#define CONNECTIONSTATUSINDICATOR_H
#include <QtGui/QFrame>

enum    Led
{
    ledOnline   = 0,
    ledActive,
    ledOffline,
    ledPrivacy,
    ledOff
};

class ConnectionStatusIndicator
    : public QWidget
{
    Q_OBJECT
    public:
        explicit ConnectionStatusIndicator(QWidget* parent = 0);

    signals:

    public slots:
        void        onChanged( uint led);
};

#include <QtCore/QThread>
class   ConnectionStatusThread
    : public QThread
{
    Q_OBJECT
    public:
        explicit    ConnectionStatusThread(QObject *parent = 0);

    signals:
        void        changed(    uint led);

    private:
        void        run();
};

#endif // CONNECTIONSTATUSINDICATOR_H

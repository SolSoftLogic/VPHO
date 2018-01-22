#ifndef SIGNALINTENSITYINDICATOR_H
#define SIGNALINTENSITYINDICATOR_H

#include <QtCore/QThread>
class   SignalIntensityThread
    : public QThread
{
    Q_OBJECT
    public:
        explicit    SignalIntensityThread(QObject *parent = 0);

    signals:
        void        changed(    uint     intensity);

    private:
        void        run();
};

#endif // SIGNALINTENSITYINDICATOR_H

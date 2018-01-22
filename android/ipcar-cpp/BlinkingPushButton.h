#ifndef BLINKINGPUSHBUTTON_H
#define BLINKINGPUSHBUTTON_H

#include <QtGui/QPushButton>
#include <QtCore/QTimer>

class BlinkingPushButton
    : public QPushButton
{
    Q_OBJECT
    public:
        explicit BlinkingPushButton(QWidget *parent = 0);

    signals:

    public slots:
    private slots:
        void        onTimerTick();

    private:
        void        showEvent( QShowEvent * event);
        void        hideEvent( QHideEvent * event);

        int         m_interval;
        QTimer      m_timer;

};

#endif // BLINKINGPUSHBUTTON_H

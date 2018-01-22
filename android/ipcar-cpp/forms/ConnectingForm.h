#ifndef CONNECTINGPANE_H
#define CONNECTINGPANE_H

#include <QtGui/QFrame>
#include <QtCore/QTimer>
#include <QtGui/QProgressBar>
#include <QtGui/QTextEdit>
#include <QtGui/QLineEdit>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>

class ConnectingForm
    : public QWidget
{
    Q_OBJECT
    public:
        explicit ConnectingForm(QWidget *parent = 0);

    signals:
        void    cancel(         int         call);

    public slots:
        void    onProgressTick();
        void    onConnecting(   int         call);

    private slots:
        void    onCancelClicked();
    
    protected:
        void    showEvent(      QShowEvent* );
        void    hideEvent(      QHideEvent* );

    private:
        QTimer          m_progressTimer;
        QProgressBar*   m_bar;
        QLabel*         m_text;
        QLineEdit*      m_user;
        QPushButton*    m_cancel;

        int             m_call;
};

#endif // CONNECTINGPANE_H

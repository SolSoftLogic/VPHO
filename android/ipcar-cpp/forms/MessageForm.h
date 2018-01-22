#ifndef MESSAGEPANE_H
#define MESSAGEPANE_H

#include <QtGui/QFrame>
#include <QtGui/QPushButton>
#include <QtCore/QTimer>
class   QLabel;

typedef enum    tag_MessageType
{
    MessageYesNo   = 0,
    MessageAcceptRefuse,
    MessageOk

} MessageType;

class MessageForm
    : public QWidget

{
    Q_OBJECT
    public:
        explicit    MessageForm(QWidget *parent = 0);
        void        showMessage(const QString&  message,
                                MessageType     type,
                                void*           userdata    = 0);

    signals:
        void        result(     int             type,
                                bool            yesAccept,
                                void*           userdata);

    private slots:
        void        onOkClick();
        void        onYesAcceptClick();
        void        onNoRefuseClick();
        void        onTimerTimeout();

    private:
        void        hideEvent(QHideEvent *);

    private:
        QLabel*         m_label;

        QPushButton*    m_ok;

        QPushButton*    m_yes;
        QPushButton*    m_no;

        QPushButton*    m_accept;
        QPushButton*    m_refuse;

        MessageType     m_type;
        void*           m_userdata;

        QTimer          m_timer;
};

#endif // MESSAGEPANE_H

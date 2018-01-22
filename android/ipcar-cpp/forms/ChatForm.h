#ifndef CHATPANE_H
#define CHATPANE_H

#include "Utils.h"
#include "CallHandlerWidget.h"


#include <QtGui/QFrame>
#include <QtGui/QTextEdit>
#include <QtGui/QLineEdit>
#include <QtGui/QLabel>

class ChatForm
    : public CallHandlerWidget
{
    Q_OBJECT
    public:
        explicit ChatForm(QWidget *parent = 0);

        void    onCallAccepted( VPCALL          call);
        void    onVpStackMessage(
                                unsigned        msg,
                                VPCALL          call,
                                unsigned        param);

    public:
        void    call(    const QString&  username);


    public slots:
        void    onSendClicked();
    private slots:
        void    onReturnPressed();

    private:
        void        showEvent( QShowEvent * event);
        void        hideEvent( QHideEvent * event);

        QTextEdit*  m_textEdit;
        QLineEdit*  m_lineEdit;
        QLabel*     m_label;
        QScrollBar* m_scroller;

        VPCALL      m_call;
        QString     m_username;
        bool        m_inCall;
};

#endif // CHATPANE_H

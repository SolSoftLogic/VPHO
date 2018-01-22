#ifndef SMSPANE_H
#define SMSPANE_H

#include "CallHandlerWidget.h"
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>
#include <QtGui/QCheckBox>

class SmsForm
    : public CallHandlerWidget
{
    Q_OBJECT
    public:
        explicit SmsForm(QWidget *parent = 0);

        void    call(       const QString&  username);

    signals:
        void    selectContact();

    public slots:
        void    onSendClicked();
        void    onShowSms(      int             id);
        void    onReplyClicked();
        void    onContactSelected(
                                const QString&  username);


    private slots:
        void    onForwardClicked();
        void    onTextChanged();
        void    onToggled(      bool            state);

    private:
        void    onVpStackMessage(
                                unsigned        msg,
                                VPCALL          pcall,
                                unsigned        param);
        void    sendMessage(    const QString&  username,
                                const QString&  message);
        void    updateUI();

    private:
        QString         m_address;
        QString         m_mobile;
        int             m_currId;

        QLabel*         m_username;
        QTextEdit*      m_input;
        QPushButton*    m_reply;
        QPushButton*    m_forward;
        QLineEdit*      m_backcounter;
        QCheckBox*      m_tomobile;
};

#endif // SMSPANE_H

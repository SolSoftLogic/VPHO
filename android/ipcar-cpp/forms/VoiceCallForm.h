#ifndef CALLPANE_H
#define CALLPANE_H

#include "CallHandlerWidget.h"

#include <QtGui/QWidget>
#include <QtGui/QPushButton>
#include <QtGui/QScrollBar>
#include <QtGui/QLabel>
#include <QtGui/QTableWidget>
#include <QtGui/QStandardItemModel>
#include <QtCore/QTimer>
#include <QtGui/QSlider>


class VoiceCallForm
    : public CallHandlerWidget
    , public VpMessageObserver

{
    Q_OBJECT
    public:
        explicit VoiceCallForm(QWidget *parent = 0);

        void    onCallAccepted( VPCALL          call);
        void    onCallRefused(  VPCALL          call,
                                int             reason);
        void    onVpStackMessage(
                                unsigned        msg,
                                VPCALL          call,
                                unsigned        param);

    private slots:
        void    onEndClicked();

        void    onHoldClicked();
        void    onResumeClicked();

        void    onVideoClicked();
        void    onSmsClicked();
        void    onChatClicked();
        void    onFileClicked();

        void    onTimerTick();
        void    onItemSelectionChanged();

        void    onPartyHoldClicked();
        void    onPartyResumeClicked();

        void    onConfAllClicked();
        void    onHoldAllClicked();

        void    onMinVolumeClicked();
        void    onVolumeChanged(int);
        void    onMaxVolumeClicked();

    public:
        void    call(           const QString&  username);

    private:
        void    updateButtons();
        void    UpdateParties();
        void    AddCall(        VPCALL          call,
                                const QString&  name);
        void    DelCall(        VPCALL          call);

        QString getUsername();

        void        message(VpMessageSubject*   subject,
                            unsigned            msg,
                            unsigned            pcall,
                            unsigned            param);


    private:
        QPushButton*            m_end;
        QPushButton*            m_hold;
        QPushButton*            m_resume;
        QPushButton*            m_video;
        QPushButton*            m_chat;
        QPushButton*            m_sms;
        QPushButton*            m_file;
        QTableView*             m_parties;
        QLabel*                 m_time;
        QPushButton*            m_holdAll;
        QPushButton*            m_confAll;
        QSlider*                m_volume;
        QPushButton*            m_minVolume;
        QPushButton*            m_maxVolume;

        QMap<VPCALL, QString>   m_call2name;
        QMap<QString, VPCALL>   m_name2call;
        QMap<int, VPCALL>       m_row2call;

        QTimer                  m_timer;
        int                     m_counter;

        CallType                m_calltype;
        QString                 m_currname;

        QStandardItemModel      m_model;

        QIcon*                  m_ipCallIcon;
        QIcon*                  m_mblCallIcon;

};

#endif // CALLPANE_H

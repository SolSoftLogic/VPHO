#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QtGui/QPushButton>
#include <QtGui/QGridLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QStackedLayout>
#include <QtGui/QStackedWidget>
#include <QtCore/QVector>

#include "HostDependent/WebBrowser.h"
#include "DialPad.h"

#include "forms/InfoForm.h"
#include "forms/Settings/SettingsForm.h"
#include "forms/Settings/AudioForm.h"
#include "forms/Settings/CallManagementForm.h"
#include "forms/Settings/NetworkForm.h"
#include "forms/Settings/VideoForm.h"
#include "forms/AddContactForm.h"
#include "forms/CallHistoryForm.h"
#include "forms/ContactListForm.h"
#include "forms/ConnectingForm.h"
#include "forms/MessageForm.h"
#include "forms/NavigationForm.h"
#include "forms/SmsForm.h"
#include "forms/FileTransferForm.h"
#include "forms/ChatForm.h"
#include "forms/MmsForm.h"
#include "forms/SlidesForm.h"
#include "forms/VideoConfForm.h"
#include "forms/CaptureForm.h"
#include "forms/DirectoryForm.h"
#include "forms/PortForwardForm.h"

#include "Keyboard.h"

#include "SignalIntensityIndicator.h"
#include "ConnectionStatusIndicator.h"

#include "BlinkingPushButton.h"

#include "forms/LoginWindow.h"
#include "forms/VoiceCallForm.h"
#include "forms/VideoCallForm.h"

#include "Utils.h"


class MainWindow
    : public QMainWindow
{
        Q_OBJECT

    public:
        MainWindow(QWidget *parent = 0);
        ~MainWindow();

        void            showMessage(
                                    const QString&      message,
                                    MessageType         type,
                                    void*               userdata);

    public slots:
        void            execute(    int             command,
                                    const QString&  param);
        void            showMe(     QWidget*        widget);
        void            hideForm(   QWidget*        widget);
        int             call(       const QString&  address,
                                    int             bearer);
    private slots:
        void            execute(    int             command);

    protected:
        void            closeEvent( QCloseEvent*    event);
        void            showEvent(  QShowEvent*     event);

    private slots:
        void            autoLogon();

        void            onBackClicked();
        void            onCalling(  int                 call,
                                    const QString&      username,
                                    int                 bearer);
        void            onDeleting();
        void            onClearing();
        void            onCancel(   int                 call);
        void            onMmsSent(  bool                success,
                                    const QString&      username);
        void            onButtonClicked();
        void            onSendClicked();
        void            onSignalChanged(
                                    uint                value);


        void            onMessageResult(
                                    int                 type,
                                    bool                result,
                                    void*               userdata);

        void            connectingStarted();
        void            connectingFailed(const QString&  text);
        void            connectionEstablished();

        void            vpstackNotificationRoutine(
                                    unsigned            msg,
                                    unsigned            pcall,
                                    unsigned            param);
        void            syncNotifyRoutine(
                                    unsigned            message,
                                    unsigned            param1,
                                    unsigned            param2);

        void            onShowNewSms();
        void            onShowNewMms();
        void            onShowNewVoice();
        void            onShowNewVideo();
        void            onMakeCall( const QString&      username,
                                    int                 bearer);


    private:
        void            updateUI();
        void            showForm(   QWidget*            form);

        static
        void            vpstackNotificationRoutine(
                                    void*               uparam,
                                    unsigned            msg,
                                    unsigned            pcall,
                                    unsigned            param);
        static
        int             syncNotifyRoutine(
                                    void*               param,
                                    unsigned            message,
                                    unsigned            param1,
                                    unsigned            param2);
        QWidget*        visibleWidget()
        {
            return  m_clientLayout->currentWidget();
        }



    private:
        QPushButton*        m_quit;

        QPushButton*        m_gmail;
        QPushButton*        m_facebook;
        QPushButton*        m_twitter;
        QPushButton*        m_gpsmenu;
        QPushButton*        m_settings;
        QPushButton*        m_vc;

        QPushButton*        m_abook;
        QPushButton*        m_log;
        QPushButton*        m_add;
        QPushButton*        m_back;

        QPushButton*        m_showKeyboard;
        QPushButton*        m_showDial;

        QLabel*             m_username;
        QLabel*             m_number;
        QLabel*             m_version;

        QWidget*            m_client;
        QStackedLayout*     m_clientLayout;

        QPushButton*        m_missedVoice;
        QPushButton*        m_missedVideo;
        QPushButton*        m_missedSms;
        QPushButton*        m_missedMms;

        QLabel*             m_online;
        QLabel*             m_offline;
        QLabel*             m_talking;

        LoginWindow*        m_login;
        InfoForm*           m_infoForm;
        SettingsForm*       m_settingsForm;

        Browser*            m_browser;
        AudioForm*          m_audioForm;
        CallManagementForm* m_callManagementForm;
        NetworkForm*        m_networkForm;
        VideoForm*          m_videoForm;
        AddContactForm*     m_addContactForm;
        CallHistoryForm*    m_callHistoryForm;
        ContactListForm*    m_contactListForm;
        ConnectingForm*     m_connectingForm;
        MessageForm*        m_messageForm;
        NavigationForm*     m_navigationForm;
        SmsForm*            m_smsForm;
        MmsForm*            m_mmsForm;
        FileTransferForm*   m_ftpForm;
        ChatForm*           m_chatForm;
        VoiceCallForm*      m_voiceCallForm;
        VideoCallForm*      m_videoCallForm;
        SlidesForm*         m_slidesForm;
        VideoConfForm*      m_videoConfForm;
        CaptureForm*        m_captureForm;
        DirectoryForm*      m_directoryForm;
        PortForwardForm*    m_portForwardForm;

        CallHandlerWidget*  m_callHandler[BC_LAST + 1];

        ConnectionStatusIndicator*
                            m_connectionStatusIndicator;
        SignalIntensityThread*
                            m_intensityThread;
        QLabel*             m_intensity[6];

        VPCALL              m_vpcall;

        DialPad             m_dialPad;
        Keyboard            m_keyboard;

        QVector<QWidget*>   m_backstack;

        QTimer              m_autoLogonTimer;

        int                 m_incomingCall;
};

#endif // MAINWINDOW_H

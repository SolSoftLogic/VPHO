#include "mainwindow.h"
#include "Commands.h"
#include "Utils.h"
#include "forms/LoginWindow.h"
#include "Version.h"

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlError>
#include <QtCore/QSettings>
#include <QtCore/QByteArray>
#include <QtCore/QDebug>

//  MainWindow::MainWindow
//  -------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_dialPad(this)
    , m_keyboard(this)
    , m_incomingCall(-1)
{
#ifdef ANDROID_VERSION
    QSettings settings("/sdcard/"PROJECTNAME"/"PROJECTNAME".ini",QSettings::IniFormat);
#else
    QSettings   settings;
#endif

    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setDocumentMode(false);
    this->move(0, 0);

    QWidget*    widget  = loadForm("qss/forms/main.ui");

    vpstack()->SetServers("271.epserver.net;383.epserver.net");
    vpstack()->SetSupportedBearersMask((1 << (BC_LAST + 1)) - 1);	//	all

    this->setCentralWidget(widget);

    assign(this, m_client,      "main_client");

    m_clientLayout  = new QStackedLayout(m_client);

    m_clientLayout->addWidget(m_infoForm            = new InfoForm);
    m_clientLayout->addWidget(m_login               = new LoginWindow);
        m_login->setProperty("_modal", true);
    m_clientLayout->addWidget(m_settingsForm        = new SettingsForm);
    m_clientLayout->addWidget(m_browser             = new Browser);
    m_clientLayout->addWidget(m_audioForm           = new AudioForm);
    m_clientLayout->addWidget(m_callManagementForm  = new CallManagementForm);
#if defined _WIN32 && !defined _WIN32_WCE
    m_clientLayout->addWidget(m_networkForm         = new NetworkForm);
#endif
    m_clientLayout->addWidget(m_videoForm           = new VideoForm);
    m_clientLayout->addWidget(m_addContactForm      = new AddContactForm);
    m_clientLayout->addWidget(m_callHistoryForm     = new CallHistoryForm);
    m_clientLayout->addWidget(m_contactListForm     = new ContactListForm);
    m_clientLayout->addWidget(m_connectingForm      = new ConnectingForm);
        m_connectingForm->setProperty("_modal", true);
    m_clientLayout->addWidget(m_messageForm         = new MessageForm);
        m_messageForm->setProperty("_modal", true);
    m_clientLayout->addWidget(m_navigationForm      = new NavigationForm);
    m_clientLayout->addWidget(m_smsForm             = new SmsForm);
        m_smsForm->setProperty("bearer", BC_SMS);
    m_clientLayout->addWidget(m_mmsForm             = new MmsForm);
        m_mmsForm->setProperty("bearer", BC_VIDEOMSG);
    m_clientLayout->addWidget(m_ftpForm             = new FileTransferForm);
        m_ftpForm->setProperty("bearer", BC_FILE);
    m_clientLayout->addWidget(m_chatForm            = new ChatForm);
        m_chatForm->setProperty("bearer", BC_CHAT);
    m_clientLayout->addWidget(m_voiceCallForm       = new VoiceCallForm);
        m_voiceCallForm->setProperty("bearer", BC_VOICE);
        m_voiceCallForm->setProperty("resident", true);
    m_clientLayout->addWidget(m_videoCallForm       = new VideoCallForm);
        m_videoCallForm->setProperty("bearer", BC_AUDIOVIDEO);
        m_videoCallForm->setProperty("resident", true);
    m_clientLayout->addWidget(m_slidesForm          = new SlidesForm);
    m_clientLayout->addWidget(m_videoConfForm       = new VideoConfForm);
        m_videoConfForm->setProperty("bearer", BC_AUDIOVIDEO);
        m_videoConfForm->setProperty("resident", true);
    m_clientLayout->addWidget(m_captureForm         = new CaptureForm);
    m_clientLayout->addWidget(m_directoryForm       = new DirectoryForm);
    m_clientLayout->addWidget(m_portForwardForm     = new PortForwardForm);

    m_client->setLayout(m_clientLayout);
    m_clientLayout->setContentsMargins(0, 0, 0, 0);
    m_clientLayout->setSpacing(0);

    memset(m_callHandler,   0, sizeof(m_callHandler));

    m_callHandler[BC_CHAT]          = m_chatForm;
    m_callHandler[BC_VOICE]         = m_voiceCallForm;
    m_callHandler[BC_VIDEO]         = m_videoCallForm;
    m_callHandler[BC_AUDIOVIDEO]    = m_videoCallForm;
    m_callHandler[BC_FILE]          = m_ftpForm;
    m_callHandler[BC_VIDEOMSG]      = m_mmsForm;
    m_callHandler[BC_SMS]           = m_smsForm;

    m_keyboard.hide();
    m_dialPad.hide();

    //  ---------------------------------------------------------------------
    assign(this, m_quit,        "main_quit");       connect(m_quit,         SIGNAL(clicked()),  this,   SLOT(onButtonClicked()));
    assign(this, m_gmail,       "main_gmail");      connect(m_gmail,        SIGNAL(clicked()),  this,   SLOT(onButtonClicked()));
    assign(this, m_facebook,    "main_facebook");   connect(m_facebook,     SIGNAL(clicked()),  this,   SLOT(onButtonClicked()));
    assign(this, m_twitter,     "main_twitter");    connect(m_twitter,      SIGNAL(clicked()),  this,   SLOT(onButtonClicked()));
    assign(this, m_gpsmenu,     "main_gpsmenu");    connect(m_gpsmenu,      SIGNAL(clicked()),  this,   SLOT(onButtonClicked()));
    assign(this, m_settings,    "main_settings");   connect(m_settings,     SIGNAL(clicked()),  this,   SLOT(onButtonClicked()));
    assign(this, m_vc,          "main_vc");         connect(m_vc,           SIGNAL(clicked()),  this,   SLOT(onButtonClicked()));

    assign(this, m_abook,       "main_abook");      connect(m_abook,        SIGNAL(clicked()),  this,   SLOT(onButtonClicked()));
    assign(this, m_log,         "main_log");        connect(m_log,          SIGNAL(clicked()),  this,   SLOT(onButtonClicked()));
    assign(this, m_add,         "main_add");        connect(m_add,          SIGNAL(clicked()),  this,   SLOT(onButtonClicked()));
    assign(this, m_back,        "main_back");       connect(m_back,         SIGNAL(clicked()),  this,   SLOT(onBackClicked()));

    assign(this, m_showKeyboard,"main_openKeyboard");
                                                    connect(m_showKeyboard, SIGNAL(clicked()),  this,   SLOT(onButtonClicked()));
    assign(this, m_showDial,    "main_openDialpad");connect(m_showDial,     SIGNAL(clicked()),  this,   SLOT(onButtonClicked()));

    assign(this, m_username,    "main_username");
    assign(this, m_number,      "main_number");
    assign(this, m_version,     "main_version");

    assign(this, m_missedVoice, "main_missedVoice");connect(m_missedVoice,  SIGNAL(clicked()),  this,   SLOT(onShowNewVoice()));
        m_missedVoice->hide();
    assign(this, m_missedVideo, "main_missedVideo");connect(m_missedVideo,  SIGNAL(clicked()),  this,   SLOT(onShowNewVideo()));
        m_missedVideo->hide();
    assign(this, m_missedSms,   "main_missedSms");  connect(m_missedSms,    SIGNAL(clicked()),  this,   SLOT(onShowNewSms()));
        m_missedSms->hide();
    assign(this, m_missedMms,   "main_missedMms");  connect(m_missedMms,    SIGNAL(clicked()),  this,   SLOT(onShowNewMms()));
        m_missedMms->hide();

    assign(this, m_offline,     "main_offline");
    assign(this, m_online,      "main_online");
    assign(this, m_talking,     "main_talking");
    m_offline->raise();

    for(int i = 0; i < 6; i++)
    {
        m_intensity[i]  = new QLabel;
        assign<QLabel>(this, m_intensity[i],    qPrintable(QString("main_intensity%1").arg(i)));
    }
    m_intensityThread   = new SignalIntensityThread;
    connect(m_intensityThread, SIGNAL(changed(uint)), this, SLOT(onSignalChanged(uint)));

//    m_connectionStatusIndicator = new ConnectionStatusIndicator();
//    placeholder(this, m_connectionStatusIndicator, "ledOnline");

    //  connect signals
    //  ---------------------------------------------------------------------
    connect(m_chatForm,         SIGNAL(calling(int, QString, int)),  this,               SLOT(onCalling(int, QString, int)));
    connect(m_voiceCallForm,    SIGNAL(calling(int, QString, int)),  this,               SLOT(onCalling(int, QString, int)));
    connect(m_videoCallForm,    SIGNAL(calling(int, QString, int)),  this,               SLOT(onCalling(int, QString, int)));
    connect(m_videoConfForm,    SIGNAL(calling(int, QString, int)),  this,               SLOT(onCalling(int, QString, int)));
    connect(&m_dialPad,         SIGNAL(calling(int, QString, int)),  this,               SLOT(onCalling(int, QString, int)));
    connect(m_mmsForm,          SIGNAL(calling(int, QString, int)),  this,               SLOT(onCalling(int, QString, int)));
    connect(m_smsForm,          SIGNAL(calling(int, QString, int)),  this,               SLOT(onCalling(int, QString, int)));
    connect(m_smsForm,          SIGNAL(selectContact(int)),          m_contactListForm,  SLOT(onSelectContact(int)));
    connect(m_ftpForm,          SIGNAL(calling(int, QString, int)),  this,               SLOT(onCalling(int, QString, int)));

    connect(m_smsForm,          SIGNAL(selectContact()),            m_contactListForm,  SLOT(onSelectContact()));
    connect(m_contactListForm,  SIGNAL(contactSelected(QString)),   m_smsForm,          SLOT(onContactSelected(QString)));

    connect(m_mmsForm,          SIGNAL(mmsSent(bool, QString)), this,               SLOT(onMmsSent(bool,QString)));
    connect(m_connectingForm,   SIGNAL(cancel(int)),            this,               SLOT(onCancel(int)));

    connect(m_contactListForm,  SIGNAL(editContact(int)),       m_addContactForm,   SLOT(onEditContact(int)));
    connect(m_contactListForm,  SIGNAL(makeCall(QString,int)),  this,               SLOT(onMakeCall(QString,int)));

    connect(m_callHistoryForm,  SIGNAL(showSms(int)),           m_smsForm,          SLOT(onShowSms(int)));
    connect(m_callHistoryForm,  SIGNAL(showMms(int)),           m_mmsForm,          SLOT(onShowMms(int)));
    connect(m_callHistoryForm,  SIGNAL(clearing()),             this,               SLOT(onClearing()));

    connect(m_voiceCallForm,    SIGNAL(makeCall(QString,int)),  this,               SLOT(onMakeCall(QString,int)));
    connect(m_videoCallForm,    SIGNAL(makeCall(QString,int)),  this,               SLOT(onMakeCall(QString,int)));
    connect(m_videoConfForm,    SIGNAL(makeCall(QString,int)),  this,               SLOT(onMakeCall(QString,int)));

    connect(m_addContactForm,   SIGNAL(deleting()),             this,               SLOT(onDeleting()));
    connect(m_messageForm,      SIGNAL(result(int,bool,void*)), this,               SLOT(onMessageResult(int,bool,void*)));

    connect(m_quit,             SIGNAL(clicked()),              this,               SLOT(close()));

    connect(m_directoryForm,    SIGNAL(selected(QString)),      m_ftpForm,          SLOT(onFileSelected(QString)));

    connect(&m_keyboard,        SIGNAL(sendClicked()),          this,               SLOT(onSendClicked()));

    connect(&m_autoLogonTimer,  SIGNAL(timeout()),              this,               SLOT(autoLogon()));
#if 0
    m_version->setText(QString("v:%1.%2-%3")
                            .arg(VERSION_MAJOR)
                            .arg(VERSION_MINOR)
                            .arg(VERSION_BUILD, 4, 10, QChar('0')));
#endif

    if(     settings.value("login/username").isValid() && settings.value("login/username").toString() != ""
        &&  settings.value("login/vpnumber").isValid() && settings.value("login/vpnumber").toString() != ""
        &&  settings.value("login/serveraccesscode").isValid()
        &&  settings.value("login/accountsecret").isValid())
    {
        //  it seems that we have all the needed data
        //  we can skip the login windows
        //  but we should activate auto login

        setUsername(settings.value("login/username").toString());
        setVpNumber(settings.value("login/vpnumber").toString());

        //  while we are not logged on
        //  the user can see his name but not his number
        m_username->setText(getUsername());

        OpenDatabase(getUsername(), getVpNumber(), getVpNumber());

        showForm(m_infoForm);

        m_autoLogonTimer.setSingleShot(false);
        m_autoLogonTimer.start(5000);
    }
    else
    {
        //  some data is not set
        //  there is noway we can get it.
        //  so we should present login windows to the user
        showForm(m_login);
    }

    updateUI();

    //  now we are ready
    //  ---------------------------------------------------------------------
    vpstack()->SetNotifyRoutine(vpstackNotificationRoutine, this);
    vpstack()->SetSyncNotifyRoutine(syncNotifyRoutine,      this);
}

//  MainWindow::onSignalChanged
//  -------------------------------------------------------------------------
void    MainWindow::onSignalChanged(uint value)
{
    if(value < 0)
        value   = 0;
    if(value > 5)
        value   = 5;

    m_intensity[value]->raise();
}

//  MainWindow::autoLogon
//  -------------------------------------------------------------------------
void        MainWindow::autoLogon()
{
#ifdef ANDROID_VERSION
    QSettings settings("/sdcard/"PROJECTNAME"/"PROJECTNAME".ini",QSettings::IniFormat);
#else
    QSettings   settings;
#endif

    if(!vpstack()->IsLoggedOn())
    {
        char            serveraccesscode[MAXSERVERACCESSCODELEN + 1];
        unsigned char   accountsecret[16];
        QByteArray  bytearray;

        bytearray   = settings.value("login/serveraccesscode").toByteArray();
        memcpy(serveraccesscode, bytearray.data(), sizeof(serveraccesscode));

        bytearray   = settings.value("login/accountsecret").toByteArray();
        memcpy(accountsecret, bytearray.data(), sizeof(accountsecret));

        vpstack()->Logon(serveraccesscode, accountsecret);
    }
}

//  MainWindow::~MainWindow
//  -------------------------------------------------------------------------
MainWindow::~MainWindow()
{

}

//  MainWindow::onCalling
//  -------------------------------------------------------------------------
void    MainWindow::onCalling(int                 call,
                              const QString&      username,
                              int                 bearer)
{
    int     rc;

    if(call == 0)
    {
        call    = (int)vpstack()->CreateCall();
    }

    rc  = vpstack()->Connect((VPCALL)call, qPrintable(username), bearer);

    if(rc == 0)
    {
        if(bearer != BC_SMS)
        {
            m_connectingForm->onConnecting(call);
            showForm(m_connectingForm);
        }
    }
    else
    {
        showMessage(QString(tr("cannot call '%1'. rc = %2")
                        .arg(username)
                        .arg(rc)), MessageOk, m_infoForm);
    
        vpMessageSubject()->message(VPMSG_CALLENDED, call, REASON_SYNTAXERROR);
        vpstack()->FreeCall((VPCALL)call);
    }
}

//  MainWindow::onDeleting
//  -------------------------------------------------------------------------
void    MainWindow::onDeleting()
{
    showMessage(tr("Are you sure you want to delete this user?"),
                MessageYesNo,
                m_addContactForm);
}

//  MainWindow::onDeleting
//  -------------------------------------------------------------------------
void    MainWindow::onClearing()
{
    showMessage(tr("Are you sure you want to clear the log?"),
                MessageYesNo,
                m_callHistoryForm);
}

//  MainWindow::call
//  -------------------------------------------------------------------------
int     MainWindow::call(   const QString&  address,
                            int             bearer)
{
    VPCALL  _call   = vpstack()->CreateCall();
    vpstack()->Connect(_call, qPrintable(address), bearer);

    if(bearer != BC_SMS)
    {
        m_connectingForm->onConnecting((int)_call);
        showForm(m_connectingForm);
    }

    return  (int)_call;
}

//  MainWindow::showMe
//  -------------------------------------------------------------------------
void    MainWindow::showMe(QWidget *widget)
{
    showForm(widget);
}

//  MainWindow::hideForm
//  -------------------------------------------------------------------------
void    MainWindow::hideForm(QWidget *form)
{
    QWidget*    current = visibleWidget();

    if(current == form)
    {
        if(m_backstack.empty())
            m_backstack.push_back(m_infoForm);

        current = m_backstack.back();
        m_backstack.pop_back();

        m_clientLayout->setCurrentWidget(current);

        m_keyboard.hide();
    }
    else
    {
        int index   = m_backstack.indexOf(form);

        if(index != -1)
        {
            m_backstack.remove(index);
        }
    }

    LogWrite(QString("hide(%1)  - [%2] ")
                .arg(form->metaObject()->className())
                .arg(visibleWidget()->metaObject()->className()));
    foreach(QWidget*widget, m_backstack)
    {
        LogWrite(QString("<%1> ").arg(widget->metaObject()->className()));
    }
    LogWrite("\n");

    updateUI();
}

void    MainWindow::onSendClicked()
{
    if(visibleWidget() == m_chatForm)
    {
        m_chatForm->onSendClicked();
    }
}

//  MainWindow::onButtonClicked
//  -------------------------------------------------------------------------
void    MainWindow::onButtonClicked()
{
    QWidget*    widget  = static_cast<QWidget*>(sender());
    QWidget*    current = visibleWidget();
    QVariant    modal   = current->property("_modal");
    
    if(widget == m_quit)
    {
        close();
    }

    else if(modal.isValid() && modal.toBool() == true)
    {
        return;
    }

    else if(widget == m_gmail)
    {
        showForm(m_browser);
        m_browser->Navigate("http://www.gmail.com");
    }

    else if(widget == m_facebook)
    {
        showForm(m_browser);
        m_browser->Navigate("http://www.facebook.com");
    }

    else if(widget == m_twitter)
    {
        showForm(m_browser);
        m_browser->Navigate("http://www.twitter.com");
    }

    else if(widget == m_gpsmenu)
    {
        showForm(m_navigationForm);
    }

    else if(widget == m_settings)
    {
        showForm(m_settingsForm);
    }

    else if(widget == m_vc)
    {
        showForm(m_portForwardForm);
    }

    else if(widget == m_abook)
    {
        showForm(m_contactListForm);
    }

    else if(widget == m_log)
    {
        showForm(m_callHistoryForm);
    }

    else if(widget == m_add)
    {
        m_addContactForm->onAddContact();
        showForm(m_addContactForm);
    }

    else if(widget == m_showKeyboard)
    {
        QWidget*    widget  = m_clientLayout->currentWidget();

        if(m_keyboard.isHidden())
        {
#ifdef SOFTKEYBOARD

            QWidget*    focus   = widget->focusWidget();

            QSize   a   = m_keyboard.size();
            QSize   b   = m_client->size();
            QPoint  c   = m_client->pos();
            m_keyboard.move(c.x(), c.y() + b.height() - a.height());
            m_keyboard.show();
            m_keyboard.raise();

            if(focus)
            {
                QPoint  one = m_keyboard.mapToGlobal(QPoint(0,0));
                QPoint  two = focus->mapToGlobal(QPoint(0,0));

                if(one.y() < two.y())
                {
                    widget->move(0, -m_keyboard.height());
                }
            }
#endif
        }
        else
        {
            widget->move(0, 0);
            m_keyboard.hide();
        }
    }

    else if(widget == m_showDial)
    {
        if(m_dialPad.isHidden())
        {
            QSize   a   = m_dialPad.size();
            QSize   b   = m_client->size();
            QPoint  c   = m_client->pos();
            m_dialPad.move(c.x(), c.y() + b.height() - a.height());
            m_dialPad.move(c.x() + b.width() - a.width(), c.y() + b.height() - a.height());
            m_dialPad.show();
            m_dialPad.raise();
        }
        else
            m_dialPad.hide();
    }
}

//  MainWindow::execute
//  -------------------------------------------------------------------------
void    MainWindow::execute(int command)
{
    execute(command, "");
}

//  MainWindow::Execute
//  -------------------------------------------------------------------------
void    MainWindow::execute(int command, const QString& param)
{
    QWidget*    current = visibleWidget();
    QVariant    modal   = current->property("_modal");

    //  if there is a modal window, drop it
    if(modal.isValid() && modal.toBool() == true && command != commandAccept)
    {
        return;
    }

    switch(command)
    {
        case    commandSlides   :
        {
            showForm(m_slidesForm);
        } break;

        case    commandHome      :
        {
            showForm(m_browser);
            m_browser->Navigate("http://www.google.com");
        } break;

        case commandContactList     :
        {
            showForm(m_contactListForm);
        } break;
        case    commandSettings :
        {
            showForm(m_settingsForm);
        } break;

        case    commandInternet :
        {
            showForm(m_browser);
            m_browser->NavigateHome();
        } break;

        case    commandAudio    :
        {
            showForm(m_audioForm);
        } break;

        case    commandCallManagement   :
        {
            showForm(m_callManagementForm);
        } break;
#if defined _WIN32 && !defined _WIN32_WCE
        case    commandNetwork          :
        {
            showForm(m_networkForm);
        } break;
#endif

        case    commandVideo            :
        {
            showForm(m_videoForm);
        } break;

        case    commandVideoCaptured    :
        {
            m_captureForm->setFile(param);
            showForm(m_captureForm);
        } break;

        case    commandSelectFile       :
        {
            showForm(m_directoryForm);
        } break;

        case    commandNavigate         :
        {
            showForm(m_browser);
            m_browser->Navigate(param);
        } break;

        case    commandAccept           :
        {
            VPCALL          call    = (VPCALL)param.toInt();
            int             bc      = vpstack()->GetCallBearer(call);
            CallHandler*    handler = m_callHandler[bc];

            if(handler)
            {
                hideMe(m_messageForm);
                handler->onVpStackMessage(VPMSG_NEWCALL, (VPCALL)call, 0);
            }
        } break;
    }
}

//  MainWindow::showEvent
//  -------------------------------------------------------------------------
void    MainWindow::showEvent(QShowEvent *event)
{
    updateUI();
}

//  MainWindow::closeEvent
//  -------------------------------------------------------------------------
void    MainWindow::closeEvent(QCloseEvent *event)
{
    m_dialPad.close();
    m_keyboard.close();
}

//  MainWindow::connectingStarted
//  -------------------------------------------------------------------------
void    MainWindow::connectingStarted()
{
    //m_statusCommandBar.show();
    showForm(m_connectingForm);
}

//  MainWindow::showMessage
//  -------------------------------------------------------------------------
void    MainWindow::showMessage(
                            const QString&      message,
                            MessageType         type,
                            void*               userdata)
{
    m_messageForm->showMessage(message, type, userdata);
    showForm(m_messageForm);
}

//  MainWindow::connectingFailed
//  -------------------------------------------------------------------------
void    MainWindow::connectingFailed(const QString&  text)
{
    showMessage(text, MessageOk, m_infoForm);
}

//  MainWindow::connectionEstablished
//  -------------------------------------------------------------------------
void    MainWindow::connectionEstablished()
{
    showMessage(tr("Connection established"), MessageOk, m_voiceCallForm);
}

//  MainWindow::showForm
//  -------------------------------------------------------------------------
void    MainWindow::showForm(QWidget*   form)
{
    QWidget*    current = visibleWidget();

    if(current != form)
    {
        QVariant    modal   = current->property("_modal");
        QVariant    resident= current->property("resident");

        if(modal.isValid() && modal.toBool())
        {
            m_backstack.push_back(form);
        }
        else if(resident.isValid() && resident.toBool())
        {
            int index   = m_backstack.indexOf(current);
            if(index != -1)
                m_backstack.remove(index);

            m_backstack.push_back(current);
            m_clientLayout->setCurrentWidget(form);
        }
        else
        {
            m_clientLayout->setCurrentWidget(form);
        }

        m_keyboard.hide();
    }

    LogWrite(QString("show(%1)  - [%2] ")
                .arg(form->metaObject()->className())
                .arg(visibleWidget()->metaObject()->className()));
    foreach(QWidget*widget, m_backstack)
    {
        LogWrite(QString("<%1> ").arg(widget->metaObject()->className()));
    }
    LogWrite("\n");

    updateUI();
}

//	MainWindow::vpstackNotificationRoutine
//  -------------------------------------------------------------------------
void    MainWindow::vpstackNotificationRoutine(
                                void*				uparam,
                                unsigned			msg,
                                unsigned			pcall,
                                unsigned			param)
{
    MainWindow* self    = static_cast<MainWindow*>(uparam);

    if(self->thread() != QThread::currentThread())
    {
        QMetaObject::invokeMethod(
                        self,
                        "vpstackNotificationRoutine",
                        Qt::QueuedConnection,
                        Q_ARG(uint, msg),
                        Q_ARG(uint, pcall),
                        Q_ARG(uint, param));
    }
    else
    {
        self->vpstackNotificationRoutine(msg, pcall, param);
    }
}

//  MainWindow::syncNotifyRoutine
//  -------------------------------------------------------------------------
int     MainWindow::syncNotifyRoutine(
                            void*               uparam,
                            unsigned            message,
                            unsigned            param1,
                            unsigned            param2)
{
    MainWindow* self    = static_cast<MainWindow*>(uparam);

    if(self->thread() != QThread::currentThread())
    {
        QMetaObject::invokeMethod(
                        self,
                        "syncNotifyRoutine",
                        Qt::QueuedConnection,
                        Q_ARG(uint, message),
                        Q_ARG(uint, param1),
                        Q_ARG(uint, param2));
    }
    else
    {
        self->syncNotifyRoutine(message, param1, param2);
    }

    return  0;
}

//  MainWindow::syncNotifyRoutine
//  -------------------------------------------------------------------------
void    MainWindow::syncNotifyRoutine(
                            unsigned            message,
                            unsigned            param1,
                            unsigned            param2)
{
    switch(message)
    {
        //  VPMSG_NOTIFYLOGON
        //  -----------------------------------------------------------------
        case    VPMSG_NOTIFYLOGON   :
        {
            m_contactListForm->onLogon();
        } break;

        //  VPMSG_ABUPDATE
        //  -----------------------------------------------------------------
        case    VPMSG_ABUPDATE	:
        {
            const char* name    = reinterpret_cast<char*>(param1);
            const char* number	= reinterpret_cast<char*>(param2);

            QSqlQuery   query;
            query.exec(QString("UPDATE contact SET vpnumber = '%1' WHERE owner = (SELECT id FROM user WHERE username = '%2') AND username = '%3' AND vpnumber = ''")
                       .arg(number)
                       .arg(getUsername())
                       .arg(name));
            LOG_SQL_QUERY(query);

            query.exec(QString("UPDATE contact SET username = '%1' WHERE owner = (SELECT id FROM user WHERE username = '%2') AND username = vpnumber AND vpnumber = '%3'")
                       .arg(name)
                       .arg(getUsername())
                       .arg(number));
            LOG_SQL_QUERY(query);

            m_contactListForm->onAbUpdate(name, number);
        } break;
    }
}

//	MainWindow::vpstackNotificationRoutine
//  -------------------------------------------------------------------------
void    MainWindow::vpstackNotificationRoutine(
                                unsigned                msg,
                                unsigned                pcall,
                                unsigned                param)
{
    VPCALL      call    = (VPCALL)pcall;
    int         bearer  = vpstack()->GetCallBearer(call);
    bool        invokeCallHandler  = false;

    switch(msg)
    {
        //  VPMSG_CALLDISCONNECTED
        //  -----------------------------------------------------------------
        case    VPMSG_CALLDISCONNECTED   :
        {
            if(bearer != BC_FILE && bearer != BC_VIDEOMSG && param != REASON_VOICE2VIDEO)
            {
                showMessage(
                        tr("The remote user has terminated the call"),
                        MessageOk,
                        0);
            }
        } break;

        //  VPMSG_SERVERTRANSFERFINISHED
        //  -----------------------------------------------------------------
        case    VPMSG_SERVERTRANSFERFINISHED    :
        {
            if(pcall == TCPFT_RXAB)
            {
                m_contactListForm->update();
            }
        } break;

        //  VPMSG_SERVERSTATUS
        //  -----------------------------------------------------------------
        case	VPMSG_SERVERSTATUS  :
        {
#ifdef ANDROID_VERSION
    QSettings settings("/sdcard/"PROJECTNAME"/"PROJECTNAME".ini",QSettings::IniFormat);
#else
    QSettings   settings;
#endif
            int         reason  = pcall;
            QString     vpnumber= vpstack()->LogonNumber();
            QString     username= vpstack()->LogonName();
            bool        remember= settings.value("login/remember").toBool();

            //  the user should see his number only when he is logged on
            m_number->setText(vpnumber);

            //  ok, check the reason
            switch(reason)
            {
                //  REASON_LOGGEDON
                //  ---------------------------------------------------------
                case    REASON_LOGGEDON :
                {
#ifdef ANDROID_VERSION
    QSettings settings("/sdcard/"PROJECTNAME"/"PROJECTNAME".ini",QSettings::IniFormat);
#else
    QSettings   settings;
#endif

                    setUsername(username);
                    setVpNumber(vpnumber);

                    m_username->setText(username);

                    OpenDatabase(username, vpnumber, vpnumber);
                    
                    hideForm(m_messageForm);
                    hideForm(m_login);

                    vpstack()->QueryAccountInfo();
                    vpstack()->CallForwardingRequest(OPERATION_ASK, "");

                    if(remember)
                    {
                        if(settings.value("login/username").toString() == "")
                        {
                            char            serveraccesscode[MAXSERVERACCESSCODELEN + 1];
                            unsigned char   accountsecret[16];
                            QByteArray  bytearray;

                            vpstack()->GetLogonData(serveraccesscode, accountsecret);

                            //  save "username"
                            settings.setValue("login/username", vpstack()->LogonName());

                            //  save "vpnumber"
                            settings.setValue("login/vpnumber", vpstack()->LogonName());


                            //  save "account secret"
                            bytearray   = QByteArray((const char*)accountsecret, sizeof(accountsecret));
                            settings.setValue("login/accountsecret", bytearray);

                            //  seve "server access code"
                            bytearray   = QByteArray((const char*)serveraccesscode, sizeof(serveraccesscode));
                            settings.setValue("login/serveraccesscode", bytearray);
                        }
                    }
                    else
                    {
                        settings.remove("login/serveraccesscode");
                        settings.remove("login/accountsecret");
                        settings.remove("login/vpnumber");
                        settings.remove("login/username");
                    }

                    m_online->raise();
                    m_contactListForm->onLogon();
                } break;

                //  REASON_NOTFOUND
                //  ---------------------------------------------------------
                case    REASON_NOTFOUND :
                {
                    //  there might be a list of server (currently two)
                    //  the user might exist on only one server
                    //  other servers will report this error
                    //  the user should be notified in case it is not
                    //  auto login. if this is auto login it means
                    //  the user succedded to login once i.e. the user
                    //  is present
                    if(m_clientLayout->currentWidget() == m_login && !vpstack()->IsLoggedOn())
                    {
                        showMessage(tr("Error registering on server: Unknown user"),
                                    MessageOk,
                                    m_login);
                    }
                } break;

                //  REASON_AUTHERROR
                //  ---------------------------------------------------------
                case    REASON_AUTHERROR    :
                {
                    //  see comment above
                    if(m_clientLayout->currentWidget() == m_login && !vpstack()->IsLoggedOn())
                    {
                        showMessage(tr("Error registering on server:\nAuthorization failed"),
                                    MessageOk,
                                    m_login);
                    }
                } break;

                //  REASON_DISCONNECTED
                //  ---------------------------------------------------------
                case    REASON_DISCONNECTED :
                {
                    //  NOTE: should we try to reconect in this case ???

                    showMessage(tr("You have been disconnected from the server\n"
                                   "by a user haiving the same account"),
                                MessageOk,
                                m_login);
                    m_username->setText("");
                    m_offline->raise();
                } break;

                //  REASON_SERVERNOTRESPONDING
                //  ---------------------------------------------------------
                case    REASON_SERVERNOTRESPONDING  :
                {
                } break;

                //  REASON_VERSIONOLD
                //  ---------------------------------------------------------
                case    REASON_VERSIONOLD   :
                {
                } break;

                //  REASON_SERVERRESET
                //  ---------------------------------------------------------
                case    REASON_SERVERRESET:
                {
                }

                //  REASON_ACCOUNTBLOCKED
                //  ---------------------------------------------------------
                case    REASON_ACCOUNTBLOCKED   :
                {
                    showMessage(tr("Your account is blocked\n"
                                   "please contact support department"),
                                MessageOk,
                                m_login);
                } break;
            }
        } break;

        case    VPMSG_SERVERNOTRESPONDING   :
        {
//            showMessage(tr("No response from the server"), MessageOk, visibleWidget());
        } break;

        //  VPMSG_QUERYONLINEACK
        //  -----------------------------------------------------------------
        case    VPMSG_QUERYONLINEACK        :
        {
            m_contactListForm->aolAck(pcall, (AOL*)param);
        } break;

        //  call processing
        //  -----------------------------------------------------------------
        //  VPMSG_NEWCALL
        //  -----------------------------------------------------------------
        case    VPMSG_NEWCALL       :
        {
            char    name[MAXNAMELEN + 1];
            int     bc      = vpstack()->GetCallBearer(call);

            vpstack()->GetCallRemoteName(call, name);

            if(bc != BC_SMS && bc <= BC_LAST)
            {
                QString         action  = getUserData(getUsername(), "newcall/action");
                CallHandler*    handler = m_callHandler[bc];
                static
                QString         bc2message[BC_LAST + 1]    = {
                    tr("Incomming voice call from %1."),
                    tr("Incomming video call from %1."),
                    tr("Incomming chat from %1."),
                    tr("Incomming video message from %1."),
                    tr("Incomming file transfere call from %1."),
                    tr("Incomming video call from %1."),
                    tr("n/a")
                };

                m_incomingCall  = pcall;
                showMessage(
                    tr(qPrintable((QString(bc2message[bc]) + QString(tr(" Do you want to accept?"))).arg(name))),
                    MessageAcceptRefuse,
                    (void*)pcall);
            }
            else
            {
                updateUI();
            }
        } break;

        //  VPMSG_CONNECTTIMEOUT
        //  ---------------------------------------------------------
        case    VPMSG_CONNECTTIMEOUT:
        {
            char    name[MAXNAMELEN + 1];
            int     bc      = vpstack()->GetCallBearer(call);

            vpstack()->GetCallRemoteName(call, name);

            showMessage(
                tr(qPrintable(QString(tr("%1 is unreachable, network timeout")).arg(name))),
                MessageOk,
                (void*)m_infoForm);
        } break;

        //  VPMSG_CALLACCEPTED
        //  ---------------------------------------------------------
        case    VPMSG_CALLACCEPTED:
        {
            int     bearer  = vpstack()->GetCallBearer(call);

            hideForm(m_connectingForm);
            showForm(m_callHandler[bearer]);

            invokeCallHandler   = true;
        } break;

        //  VPMSG_INVALIDADDR
        //  ---------------------------------------------------------
        case    VPMSG_INVALIDADDR:
        {
            if(m_connectingForm->isVisible())
            {
                showMessage(
                    tr(qPrintable(QString(tr("an invalid address dialed")))),
                    MessageOk,
                    (void*)m_infoForm);
            }
        } break;

        //  VPMSG_CALLREFUSED
        //  ---------------------------------------------------------
        case    VPMSG_CALLREFUSED:
        {
            int     bearer  = vpstack()->GetCallBearer(call);
            char    name[MAXNAMELEN + 1];

            hideForm(m_connectingForm);
            invokeCallHandler   = true;

            vpstack()->GetCallRemoteName(call, name);

            showMessage(
                tr(qPrintable(QString("%1 did not accept your call. Reason: %2")
                                .arg(name)
                                .arg(getReason(param)))),
                MessageOk,
                (void*)visibleWidget());

            invokeCallHandler   = true;
        } break;

        case    VPMSG_CALLENDED                 : 
        case    VPMSG_REMOTELYHELD              :
        case    VPMSG_REMOTELYRESUMED           :
        case    VPMSG_CHAT                      :
        case    VPMSG_CHATACK                   :
        case    VPMSG_SENDFILEREQ               :
        case    VPMSG_FTPROGRESS                :
        case    VPMSG_SENDFILESUCCESS           :
        case    VPMSG_SENDFILEFAILED            :
        case    VPMSG_FTTXCOMPLETE              :
        case    VPMSG_FTRXCOMPLETE              :
        {
            invokeCallHandler   = true;
        } break;
    }

    if(invokeCallHandler && bearer != BC_NOTHING)
    {
        int             bearer  = vpstack()->GetCallBearer(call);
        CallHandler*    handler = m_callHandler[bearer];

        if(handler)
        {
            handler->onVpStackMessage(msg, call, param);
        }
    }

    vpMessageSubject()->message(msg, pcall, param);

    if(msg == VPMSG_CALLENDED)
    {
        if(m_incomingCall == pcall)
        {
            //  it seems that the user did not answer the call
            m_incomingCall  = -1;
        }

        hideForm(m_connectingForm);

        vpstack()->FreeCall((VPCALL)pcall);

        updateUI();
    }
}

//  MainWindow::onMessageResult
//  -------------------------------------------------------------------------
void    MainWindow::onMessageResult(
                            int                 type,
                            bool                result,
                            void*               userdata)
{
    setUpdatesEnabled(false);

    hideForm(m_messageForm);

    switch(type)
    {
        case    MessageOk   :
        {
            if(userdata)
            {
                showMe((QWidget*)userdata);
            }
        } break;
        
        case    MessageAcceptRefuse :
        {
            VPCALL      call    = userdata;
            int         bearer  = vpstack()->GetCallBearer((VPCALL)userdata);
            CallHandler*handler = bearer != -1 ? m_callHandler[bearer] : NULL;

            m_incomingCall  = -1;

            if(result)
            {
                vpstack()->AnswerCall(call);
            }
            else
            {
                vpstack()->Disconnect(call, REASON_NORMAL);
            }

            if(handler && result)
            {
                handler->onVpStackMessage(VPMSG_NEWCALL, call, 0);
            }
            else
            {
                showForm(m_infoForm);
            }
        } break;

        case    MessageYesNo    :
        {
            if(userdata == m_addContactForm)
            {
                if(result)
                {
                    m_addContactForm->doDelete();
                    showForm(m_contactListForm);
                }
                else
                {
                    showForm(m_addContactForm);
                }
            }

            if(userdata == m_callHistoryForm)
            {
                if(result)
                {
                    m_callHistoryForm->doClear();
                    showForm(m_infoForm);
                }
                else
                {
                    showForm(m_callHistoryForm);
                }
            }
        } break;
    }

    hideForm(m_messageForm);

    setUpdatesEnabled(true);
}

//  MainWindow::onMmsSent
//  -------------------------------------------------------------------------
void    MainWindow::onMmsSent(
                            bool                success,
                            const QString&      username)

{
    if(success)
    {
        showMessage(QString(tr("Video message has been successfully sent to %1")
                        .arg(username)),
                    MessageOk,
                    m_contactListForm);
    }
    else
    {
        showMessage(QString(tr("Sending video message to %1 failed")
                        .arg(username)),
                    MessageOk,
                    m_contactListForm);
    }
}

//  MainWindow::onCancel
//  -------------------------------------------------------------------------
void    MainWindow::onCancel(int   call)
{

    vpstack()->Disconnect((VPCALL)call, REASON_NORMAL);
    
    hideForm(m_connectingForm);
}

//  MainWindow::onShowNewSms
//  -------------------------------------------------------------------------
void    MainWindow::onShowNewSms()
{
    QSqlQuery   query;

    query.exec(QString("SELECT id FROM log WHERE (type & %1) == %2 AND (type & %3) != %3")
                    .arg(logBcMask)
                    .arg(logtSms)
                    .arg(logtRead));
    LOG_SQL_QUERY(query);

    int     id;

    if(query.next())
    {
        id  = query.value(0).toInt();

        m_smsForm->onShowSms(id);
    }

    updateUI();
}

//  MainWindow::onShowNewMms
//  -------------------------------------------------------------------------
void    MainWindow::onShowNewMms()
{
    QSqlQuery   query;

    query.exec(QString("SELECT id FROM log WHERE (type & %1) == %2 AND (type & %3) != %3")
                    .arg(logBcMask)
                    .arg(logtVideoMsg)
                    .arg(logtRead));
    LOG_SQL_QUERY(query);

    int     id;

    if(query.next())
    {
        id  = query.value(0).toInt();

        m_mmsForm->onShowMms(id);
    }

    updateUI();
}

//  MainWindow::onShowNewVoice
//  -------------------------------------------------------------------------
void    MainWindow::onShowNewVoice()
{
    QSqlQuery   query;

    query.exec(QString("SELECT id FROM log WHERE (type & %1) == %2 AND (type & %3) != %3")
                    .arg(logBcMask)
                    .arg(logtVoice)
                    .arg(logtRead));
    LOG_SQL_QUERY(query);

    if(query.next())
    {
        showForm(m_callHistoryForm);
    }

    updateUI();
}

//  MainWindow::onShowNewVideo
//  -------------------------------------------------------------------------
void    MainWindow::onShowNewVideo()
{
    QSqlQuery   query;

    query.exec(QString("SELECT id FROM log WHERE (type & %1) == %2 AND (type & %3) != %3")
                    .arg(logBcMask)
                    .arg(logtAudioVideo)
                    .arg(logtRead));
    LOG_SQL_QUERY(query);

    if(query.next())
    {
		showForm(m_callHistoryForm);
    }

    updateUI();
}

//  MainWindow::onMakeCall
//  -------------------------------------------------------------------------
void    MainWindow::onMakeCall(
                            const QString&      name,
                            int                 bc)
{
    if(     getUsername() == name
       ||   vpstack()->LogonNumber() == name)
    {
        showMessage(tr("You cannot call yourself"), MessageOk, m_infoForm);
    }
    else
    {
        VPCALL  calls[10];
        int     count;

        count   = vpstack()->EnumCalls(calls, sizeof(calls) / sizeof(calls[0]), 1 << bc);

        for(int i = 0; i < count; ++i)
        {
            char    username[MAXNAMELEN + 1];
            char    vpnumber[MAXNUMBERLEN + 1];

            vpstack()->GetCallRemoteName(calls[i], username);
            vpstack()->GetCallRemoteNumber(calls[i], vpnumber);

            if(name == username || name == vpnumber)
            {
                showMessage(tr("You cannot call same user twice"), MessageOk, m_infoForm);

                return;
            }
        }

        m_callHandler[bc]->call(name);
    }
}

//  MainWindow::onBackClicked
//  -------------------------------------------------------------------------
void    MainWindow::onBackClicked()
{
    QWidget*    current     = visibleWidget();
    QVariant    resident    = current->property("resident");

    if(resident.isValid() && resident.toBool())
    {
        int index   = m_backstack.indexOf(current);
        if(index != -1)
            m_backstack.remove(index);

        m_backstack.push_front(current);

        current = m_backstack.back();
        m_backstack.pop_back();

        m_clientLayout->setCurrentWidget(current);
    }
    else
    {
        if(!m_backstack.isEmpty())
        {
            current = m_backstack.back();
            m_backstack.pop_back();

            m_clientLayout->setCurrentWidget(current);
        }
        else
        {
            m_clientLayout->setCurrentWidget(m_infoForm);
        }
    }

    LogWrite(QString("back      - [%1] ")
                .arg(visibleWidget()->metaObject()->className()));
    foreach(QWidget*widget, m_backstack)
    {
        LogWrite(QString("<%1> ").arg(widget->metaObject()->className()));
    }
    LogWrite("\n");

    updateUI();
}

//  MainWindow::updateUI
//  -------------------------------------------------------------------------
void    MainWindow::updateUI()
{
    QSqlQuery   query;
    QString     incoming;

    if(m_incomingCall != -1)
        incoming    = "AND (log.id != max(log.id))";

    //  SMS
    //  ---------------------------------------------------------------------
    query.exec(QString("SELECT COUNT(*) FROM log JOIN user ON log.owner == user.id WHERE user.username = '%1' AND (log.type & %2) = %3 %4")
                    .arg(getUsername())
                    .arg(logBcMask | logtRead | logtOutgoing)
                    .arg(logtSms)
                    .arg(incoming));
    LOG_SQL_QUERY(query);
    query.first();
    if(query.value(0).toInt() != 0)
        m_missedSms->show();
    else
        m_missedSms->hide();


    //  MMS
    //  ---------------------------------------------------------------------
    query.exec(QString("SELECT COUNT(*) FROM log JOIN user ON log.owner == user.id WHERE user.username = '%1' AND (log.type & %2) = %3 %4")
                    .arg(getUsername())
                    .arg(logBcMask | logtRead | logtOutgoing)
                    .arg(logtVideoMsg)
                    .arg(incoming));
    LOG_SQL_QUERY(query);
    query.first();
    if(query.value(0).toInt() != 0)
        m_missedMms->show();
    else
        m_missedMms->hide();


    //  VOICE
    //  ---------------------------------------------------------------------
    query.exec(QString("SELECT COUNT(*) FROM log JOIN user ON log.owner == user.id WHERE user.username = '%1' AND (log.type & %2) = %3 %4")
                    .arg(getUsername())
                    .arg(logBcMask | logtRead | logtOutgoing)
                    .arg(logtVoice)
                    .arg(incoming));
    LOG_SQL_QUERY(query);
    query.first();
    if(query.value(0).toInt() != 0)
        m_missedVoice->show();
    else
        m_missedVoice->hide();


    //  VIDEO
    //  ---------------------------------------------------------------------
    query.exec(QString("SELECT COUNT(*) FROM log JOIN user ON log.owner == user.id WHERE user.username = '%1' AND (log.type & %2) = %3 %4")
                    .arg(getUsername())
                    .arg(logBcMask | logtRead | logtOutgoing)
                    .arg(logtAudioVideo)
                    .arg(incoming));
    LOG_SQL_QUERY(query);
    query.first();
    if(query.value(0).toInt() != 0)
        m_missedVideo->show();
    else
        m_missedVideo->hide();

    //  set indicator
    //  ---------------------------------------------------------------------
    int activeCalls = vpstack()->EnumCalls(NULL, 0, (1 << (BC_LAST + 1)) - 1);
    if(activeCalls == 0 && m_intensityThread->isRunning())
    {
        m_intensityThread->quit();
    }

    if(activeCalls != 0 && !m_intensityThread->isRunning())
    {
        m_intensityThread->start();
    }

    for(int i = 0; i < 6; i++)
    {
        m_intensity[i]->setShown(activeCalls != 0);
    }

    m_talking->raise();
    m_talking->setShown(activeCalls != 0);

    //  set buttons
    //  ---------------------------------------------------------------------
    QWidget*    top     = visibleWidget();
    bool        disable =    top == m_messageForm
                          || top == m_connectingForm;
    bool        login   = top == m_login;

    if(login)
    {
        m_back->hide();
        m_log->hide();
        m_add->hide();
        m_abook->hide();
    }
    else
    {
        m_back->show();
        m_log->show();
        m_add->show();
        m_abook->show();

        m_back->setDisabled(    disable);
        m_log->setDisabled(     disable || top == m_callHistoryForm);
        m_add->setDisabled(     disable || top == m_addContactForm);
        m_abook->setDisabled(   disable || top == m_contactListForm);
    }

}

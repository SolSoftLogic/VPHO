#include "LoginWindow.h"
#include "Utils.h"
#include "Commands.h"

#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QLineEdit>
#include <QtCore/QVariant>
#include <QtCore/QSettings>
#include "Keyboard.h"


//  LoginWindow::LoginWindow
//  -------------------------------------------------------------------------
LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent)
{
#ifdef ANDROID_VERSION
    QSettings settings("/sdcard/"PROJECTNAME"/"PROJECTNAME".ini",QSettings::IniFormat);
#else
    QSettings   settings;
#endif

    QWidget*    widget  = loadForm("qss/forms/login.ui");

    QVBoxLayout*layout  = new QVBoxLayout;

    layout->addWidget(widget);
    layout->addWidget(new Keyboard);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    this->setLayout(layout);

    assign(this,    m_login,    "login_login");
    assign(this,    m_register, "login_register");
    assign(this,    m_password, "login_password");
    assign(this,    m_pin,      "login_pin");
    assign(this,    m_username, "login_username");
    assign(this,    m_remember, "login_remember");

    connect(m_username, SIGNAL(textChanged(QString)), this, SLOT(textChanged()));
    connect(m_password, SIGNAL(textChanged(QString)), this, SLOT(textChanged()));
    connect(m_pin,      SIGNAL(textChanged(QString)), this, SLOT(textChanged()));
    connect(m_remember, SIGNAL(clicked()),            this, SLOT(textChanged()));
    connectButton(m_register, commandHome);

    connect(m_login, SIGNAL(clicked()), this, SLOT(loginClicked()));

    m_remember->setChecked(settings.value("login/remember").toBool());
}

//  LoginWindow::textChanged
//  -------------------------------------------------------------------------
void        LoginWindow::textChanged()
{
#ifdef ANDROID_VERSION
    QSettings settings("/sdcard/"PROJECTNAME"/"PROJECTNAME".ini",QSettings::IniFormat);
#else
    QSettings   settings;
#endif

    settings.setValue("login/remember", m_remember->isChecked());

    m_login->setEnabled(   !m_username->text().isEmpty()
                        && !m_password->text().isEmpty());
}

//  LoginWindow::loginClicked
//  -------------------------------------------------------------------------
void        LoginWindow::loginClicked()
{
#ifdef ANDROID_VERSION
    QSettings settings("/sdcard/"PROJECTNAME"/"PROJECTNAME".ini",QSettings::IniFormat);
#else
    QSettings   settings;
#endif

    settings.setValue("login/remember", m_remember->isChecked());

    //  login vp server
    vpstack()->Logon(
                qPrintable(m_username->text()), 
                qPrintable(m_password->text()));

    //  set SIM card PIN
    if(!m_pin->text().isEmpty())
    {
        settings.setValue("login/PIN",      m_pin->text());
        vpstack()->Logon(NULL, qPrintable(m_pin->text()));
    }
}

//  LoginWindow::hideEvent
//  -------------------------------------------------------------------------
void        LoginWindow::hideEvent ( QHideEvent * event )
{
    m_username->setText("");
    m_password->setText("");
}

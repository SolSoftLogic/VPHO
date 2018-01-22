#include "../webbrowser.h"
#include "../../Utils.h"
#include <windows.h>

#include <QtGui/QFrame>
#include <QtGui/QVBoxLayout>

//  WebBrowser::WebBrowser
//  -------------------------------------------------------------------------
Browser::Browser(QWidget* parent)
    : QWidget(parent)
    , m_browser("{8856F961-340A-11D0-A96B-00C04FD705A2}")
{
    QWidget*    widget  = loadForm("qss/forms/browser.ui");

    QVBoxLayout*layout  = new QVBoxLayout;

    layout->addWidget(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    this->setLayout(layout);

    assign(this,    m_back,     "browser_back");
    assign(this,    m_forward,  "browser_forward");
    assign(this,    m_refresh,  "browser_refresh");
    assign(this,    m_go,       "browser_go");
    assign(this,    m_address,  "browser_address");
    placeholder(this, &m_browser, "browser_client");

    connect(m_back,    SIGNAL(clicked()),   &m_browser, SLOT(GoBack()));
    connect(m_forward, SIGNAL(clicked()),   &m_browser, SLOT(GoForward()));
    connect(m_refresh, SIGNAL(clicked()),   &m_browser, SLOT(Refresh()));
    connect(m_go,      SIGNAL(clicked()),   this,       SLOT(onGo()));

    connect(&m_browser,SIGNAL(signal(const QString &, int, void*)),
                                            this,       SLOT(onSignal(const QString &, int, void*)));
}

//  WebBrowser::resizeEvent
//  -------------------------------------------------------------------------
void    Browser::resizeEvent(QResizeEvent * event)
{
   // m_browser.setGeometry(0, 0, event->size().width(), event->size().height());
}

//  WebBrowser::Navigate
//  -------------------------------------------------------------------------
void    Browser::Navigate(   QString         url)
{
    m_browser.dynamicCall("Navigate(const QString&)", url);
    m_address->setText(url);
}

//  WebBrowser::NavigateHome
//  -------------------------------------------------------------------------
void    Browser::NavigateHome()
{
    m_browser.dynamicCall("GoHome()");
}

//  WebBrowser::onGo
//  -------------------------------------------------------------------------
void    Browser::onGo()
{
    Navigate(m_address->text());
}

//  Browser::setCaption
//  -------------------------------------------------------------------------
void    Browser::onSignal(const QString& name, int argc, void* argv)
{
    VARIANTARG* params = (VARIANTARG*)argv;

    if (name.startsWith("NavigateComplete2("))
    {
        IDispatch*  pDisp           = params[argc - 1].pdispVal;
        VARIANT*    URL             = params[argc - 2].pvarVal;
        BSTR        bstr            = URL->bstrVal;
        QString     url((QChar*)bstr, wcslen(bstr));

        m_address->setText(url);
    }
}

/*
//  WebBrowser::onBack
//  -------------------------------------------------------------------------
void    WebBrowser::onBack()
{

}

//  WebBrowser::onForward
//  -------------------------------------------------------------------------
void    WebBrowser::onForward()
{
}

//  WebBrowser::onRefresh
//  -------------------------------------------------------------------------
void    WebBrowser::onRefresh()
{
}
*/

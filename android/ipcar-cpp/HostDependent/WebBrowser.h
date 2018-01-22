#ifndef __WEB_BROWSER_H_
#define __WEB_BROWSER_H_

#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QResizeEvent>

#ifdef  WIN32
#include <ActiveQt/QAxWidget>
#endif

class Browser
    : public QWidget
{
    Q_OBJECT
    public:
        Browser(QWidget* parent = 0);


    public slots:
        void            Navigate(   QString         url);
        void            NavigateHome();


    protected:
        void    resizeEvent(QResizeEvent*   event);

    public slots:
        void    onGo();
        void    onSignal(const QString& name, int argc, void* argv);
/*
    public slots:
        void on_WebBrowser_TitleChange(const QString &title);
        void on_WebBrowser_ProgressChange(int a, int b);
        void on_WebBrowser_CommandStateChange(int cmd, bool on);
        void on_WebBrowser_BeforeNavigate();
        void on_WebBrowser_NavigateComplete(QString);

        void on_actionGo_triggered();
        void on_actionNewWindow_triggered();
        void on_actionAbout_triggered();
        void on_actionAboutQt_triggered();
        void on_actionFileClose_triggered();
*/

    private:
        QPushButton*    m_back;
        QPushButton*    m_forward;
        QLineEdit*      m_address;
        QPushButton*    m_go;
        QPushButton*    m_refresh;

#ifdef  WIN32
        QAxWidget       m_browser;
#endif
};

#endif

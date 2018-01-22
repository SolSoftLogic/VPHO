#include <QtGui/QApplication>
#include <QtCore/QFile>
#include <QtCore/QDir>

#include <QtCore/QSignalMapper>
#include "mainwindow.h"
#include "Commands.h"
#include "Utils.h"
#include "forms/LoginWindow.h"

#ifdef _DEBUG
#include "MemLeakDetect.h"
#endif


MainWindow*     g_mainWindow;
QSignalMapper*  signalMapper;
extern int generatelog, debugmode;

//  getExecutor
//  -------------------------------------------------------------------------
void    connectButton(QPushButton*  button, command_t command)
{
    QObject::connect(button, SIGNAL(clicked()), signalMapper, SLOT(map()));
    signalMapper->setMapping(button, command);
}

//  sendCommand
//  -------------------------------------------------------------------------
void    sendCommand(    command_t       command,
                        const QString&  param)
{
    g_mainWindow->execute(command, param);
}

//  showMe
//  -------------------------------------------------------------------------
void    showMe(         QWidget*        widget)
{
    g_mainWindow->showMe(widget);
}

//  hideMe
//  -------------------------------------------------------------------------
void    hideMe(         QWidget*        widget)
{
    g_mainWindow->hideForm(widget);
}

//  showMessage
//  -------------------------------------------------------------------------
void    showMessage(    QWidget*        parent,
                        const QString&  message)
{
    g_mainWindow->showMessage(message, MessageOk, parent);
}

//  call
//  -------------------------------------------------------------------------
int     call(           const QString&  address,
                        int             bearer)
{
    return  g_mainWindow->call(address, bearer);
}


//  main
//  -------------------------------------------------------------------------
int     main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    QCoreApplication::setOrganizationName(PROJECTNAME);
    QCoreApplication::setOrganizationDomain(PROJECTNAME ".com");
    QCoreApplication::setApplicationName(PROJECTNAME);

    InitDatabase();

//#if defined _WIN32 && !defined _WIN32_WCE
//    generatelog = 3;
//#endif

    QString         stylesheet;
    QDomDocument    layout;
    loadXml("qss/stylesheet.xml", stylesheet, layout);

    qApp->setStyleSheet(stylesheet);
    qApp->addLibraryPath(qApp->applicationDirPath()); 

    signalMapper    = new QSignalMapper;
    g_mainWindow    = new MainWindow;

#if     defined(PROJECT_VOIP2CAR)
    g_mainWindow->resize(800, 600);
#elif   defined(PROJECT_IP_PAD)
    g_mainWindow->resize(800, 480);
#else
#error  "project name is not set"
#endif


    g_mainWindow->connect(signalMapper, SIGNAL(mapped(int)),   g_mainWindow, SLOT(execute(int)));
    g_mainWindow->show();

#ifdef HOME_PATH_SDCARD
    QDir    home    = QDir("/sdcard");
#else
    QDir    home    = QDir::home();

#endif
    home.mkdir(PROJECTNAME);
    home.mkdir(PROJECTNAME"/Data");
    home.mkdir(PROJECTNAME"/Received Files");
    home.mkdir(PROJECTNAME"/Received Video Messages");
    home.mkdir(PROJECTNAME"/Sent Video Messages");
    home.mkdir(PROJECTNAME"/Voice Messages");
    home.mkdir(PROJECTNAME"/Video Capture");

    return app.exec();
}

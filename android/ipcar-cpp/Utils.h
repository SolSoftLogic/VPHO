#ifndef UTILS_H
#define UTILS_H

#if     defined(PROJECT_VOIP2CAR)
#define PROJECTNAME "IPCar"
#elif   defined(PROJECT_IP_PAD)
#define PROJECTNAME "IP-Pad"
#else
#error  "project name is not set"
#endif

#include "../vpstack/vpstack.h"
#define MAX_PATH    260
#define MAX_NUM_OF_PARTIES  2

#include <QtCore/QString>
#include <QtXml/QDomDocument>
#include <QtCore/QMap>
#include <QtGui/QWidget>
#include <QtSql/QSqlQuery>
#include <QtGui/QMessageBox>
#include <QtGui/QApplication>
#ifndef   ANDROID_VERSION
#include <QtGui/QSound>
#else

#include <android/log.h>
#define  LOG_TAG    "ipcar"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#include "QSound.h"
#endif
#include <QtGui/QPushButton>

IVPSTACK*           vpstack();
VPVIDEODATAFACTORY* vpvideoFactory(
                        QWidget*        widget);

QString     getReason(  unsigned        reason);
void        txAbook();


bool        InitDatabase();
bool        OpenDatabase(
                        const QString&  username,
                        const QString&  vpnumber,
                        const QString&  password);

void        setUserData(const QString&  username,
                        const QString&  key,
                        const QString&  value);
QString     getUserData(const QString&  username,
                        const QString&  key);

void        setUsername(const QString&  username);
QString     getUsername();
void        setVpNumber(const QString&  vpnumber);
QString     getVpNumber();

void        QueryOnline(const QString&  vpnumber);

class   QWidget;
void        loadXml(    const char*     xml,
                        QString&        stylesheet,
                        QDomDocument&   layout);
QWidget*    loadForm(   const char*     formui);
QString     getFilename(const QString&  filename,
                        bool            assert = true);
bool        doesExist(  const QString&  filename);
uint        getFileSize(const QString&  filename);
QString     getSpeed(   unsigned long   size,
                        unsigned long   elapsed);
void        LogWrite(   const QString&  text);
void        LogSqlQuery(const QSqlQuery&query);

QString     getQPushButtonStylesheet(
                        const QString&  objname);

#define LOG_SQL_QUERY(query)    LogSqlQuery(query)

template<typename T>
void        assign(QWidget* parent, T*& widget, const char* name)
{
    widget  = parent->findChild<T*>(name);

    if(widget == NULL)
    {
        QMessageBox::critical(
                        0,
                        QObject::tr(PROJECTNAME),
                        QString(QObject::tr("cannot find object '%1', please reinstall the program"))
                            .arg(name),
                        QMessageBox::Ok);
        qApp->quit();
    }
}

void        assign(QWidget* parent, QPushButton*& widget, const char* name);


void        placeholder(QWidget*    parent, QWidget* widget, const char* name);

//  vp message obsever/subject
//  -------------------------------------------------------------------------
class   VpMessageObserver;
class   VpMessageSubject
{
    public:
        virtual
        void        addObserver(
                            VpMessageObserver*  observer)   = 0;
        virtual
        void        delObserver(
                            VpMessageObserver*  observer)   = 0;
        virtual
        void        message(
                            unsigned            msg,
                            unsigned            pcall,
                            unsigned            param)      = 0;
};

class   VpMessageObserver
{
    public:
        virtual
        void        message(VpMessageSubject*   subject,
                            unsigned            msg,
                            unsigned            pcall,
                            unsigned            param)      = 0;
};

VpMessageSubject*   vpMessageSubject();


class   VpMessageObserverFilterByBearer
    : public VpMessageObserver
{
    public:
        VpMessageObserverFilterByBearer(
                            VpMessageObserver*  observer,
                            int                 bearerMask,
                            bool                selfDelete = true)
            : m_observer(observer)
            , m_bearerMask(bearerMask)
            , m_selfDelete(selfDelete)
        {

        }

        void        message(VpMessageSubject*   subject,
                            unsigned            msg,
                            unsigned            pcall,
                            unsigned            param)
        {
            if(     msg < VPMSG_INVALIDADDR
               ||   msg > VPMSG_REQUESTREFUSED)
            {
                //  this is not a call message
                return;
            }

            int bearer  = vpstack()->GetCallBearer((VPCALL)pcall);

            if((1 << bearer) & m_bearerMask)
            {
                m_observer->message(subject, msg, pcall, param);
            }
        }

    private:
        VpMessageObserver*  m_observer;
        int                 m_bearerMask;
        bool                m_selfDelete;
};

class   VpMessageObserverFilterByCall
    : public VpMessageObserver
{
    public:
        VpMessageObserverFilterByCall(
                            VpMessageObserver*  observer,
                            VPCALL              call,
                            bool                selfDelete = true)
            : m_observer(observer)
            , m_call(call)
            , m_selfDelete(selfDelete)
        {

        }

        void        message(VpMessageSubject*   subject,
                            unsigned            msg,
                            unsigned            pcall,
                            unsigned            param)
        {
            if(     msg < VPMSG_INVALIDADDR
               ||   msg > VPMSG_REQUESTREFUSED)
            {
                //  this is not a call message
                return;
            }

            if(pcall = (unsigned)m_call)
            {
                m_observer->message(subject, msg, pcall, param);

                if(msg == VPMSG_CALLENDED)
                {
                    subject->delObserver(this);
                    if(m_selfDelete)
                        delete this;
                }
            }
        }

    private:
        VpMessageObserver*  m_observer;
        VPCALL              m_call;
        bool                m_selfDelete;
};


#endif // UTILS_H

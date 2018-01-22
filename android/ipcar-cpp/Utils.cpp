#include <QtCore/QFile>
#include <QtCore/QMap>
#include <QtCore/QSet>
#include <QtCore/QDir>
#include <QtCore/QSettings>
#include <QtCore/QString>
#include <QtCore/QTemporaryFile>

#include <QtGui/QApplication>
#include <QtGui/QWidget>
#include <QtGui/QMessageBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QAbstractItemView>

#include <QtXml/QXmlDefaultHandler>
#include <QtXml/QDomDocument>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlResult>
#include <QtSql/QSqlError>
#include <QtSql/QSqlField>
#include <QtSql/QSqlRecord>

#include <QtUiTools/QUiLoader>

#include "Utils.h"
#include "VpStackLogDecorator.h"
#include "VpStackAolDecorator.h"
#include "VpStackDbgLogDecorator.h"
#include "VpStackSoundDecorator.h"
#include "VpStackAnsweringMachineDecorator.h"
#include "VpStackContactDecorator.h"
#include "VpStackChooserDecorator.h"

#pragma warning(disable: 4100)
#pragma warning(push, 3)
#include "../vpstack/vpstack.h"
#pragma warning(pop)

//#include "core/vpstack/mixingaudio.h"

extern int generatelog;

class Xml2Stylesheet
    : public QXmlDefaultHandler
{

    public:
        explicit Xml2Stylesheet()
        {

        }

        void    loadXml(            const char*                 xmlfile,
                                    QString&                    stylesheet,
                                    QDomDocument&               layout);

    protected:
        bool    startElement(       const QString&              namespaceURI,
                                    const QString&              localName,
                                    const QString&              qName,
                                    const QXmlAttributes&       attributes);
        bool    endElement(         const QString&              namespaceURI,
                                    const QString&              localName,
                                    const QString&              qName);
        bool    fatalError(         const QXmlParseException&   exception)
        {
            return false;
        }

        QString errorString() const
        {
            return m_errorStr;
        }

        QString             m_errorStr;
        bool                m_instylesheet;
        QDomDocument*       m_layout;
        QDomNode            m_curr;

    signals:

    public slots:

    private:
        QString*    m_stylesheet;
};

//  Xml2Stylesheet::loadXml
//  -------------------------------------------------------------------------
void        Xml2Stylesheet::loadXml(        const char*     xmlfile,
                                            QString&        stylesheet,
                                            QDomDocument&   layout)
{
    QFile               file(getFilename(xmlfile));
    QXmlInputSource     stream(&file);
    QXmlSimpleReader    reader;

    m_stylesheet    = &stylesheet;
    m_instylesheet  = false;
    m_layout        = &layout;

    reader.setContentHandler(this);
    reader.setErrorHandler(this);
    reader.parse(stream);
}

//  Xml2Stylesheet::startElement
//  -------------------------------------------------------------------------
bool    Xml2Stylesheet::startElement(
                            const QString&              namespaceURI,
                            const QString&              localName,
                            const QString&              qName,
                            const QXmlAttributes&       attributes)
{
    if(m_layout->hasChildNodes() == false)
    {
        if(qName.compare(PROJECTNAME, Qt::CaseInsensitive) == 0)
        {
            QDomElement elem= m_layout->createElement(PROJECTNAME);
            m_curr  = m_layout->appendChild(elem);
        }
        else
        {
            m_errorStr = QObject::tr(qPrintable(QString("<%1> should be the first node.").arg(PROJECTNAME)));
            return   false;
        }
    }

    if(qName.compare("stylesheet", Qt::CaseInsensitive) == 0)
    {
        if(m_instylesheet)
        {
            m_errorStr = QObject::tr("stylesheets are not recursive.");
            return   false;
        }

        m_instylesheet  = true;

        QString         selector    = attributes.value("selector");
        (*m_stylesheet)   += selector.replace("'", "\"") + "{";

        return  true;
    }

    if(m_instylesheet)
    {
         QString    value   = attributes.value("value");

         if(value.left(3).compare("url", Qt::CaseInsensitive) == 0)
         {
            // 4 is the length of 'url('
            // 5 is the length of 'url(' + ')'
            QString url = value.mid(4, value.length() - 5);

            url = getFilename(url);

            value   = "url(" + url + ")";
         }

        (*m_stylesheet) += qName + ":" + value + ";";
    }
    else
    {
        QDomElement elem    = m_layout->createElement(qName);

        for(int i = 0; i < attributes.count(); ++i)
        {
            elem.setAttribute(attributes.qName(i), attributes.value(i));
        }

        m_curr  = m_curr.appendChild(elem);
    }

    return  true;
}

//  Xml2Stylesheet::endElement
//  -------------------------------------------------------------------------
bool    Xml2Stylesheet::endElement(
                            const QString&              namespaceURI,
                            const QString&              localName,
                            const QString&              qName)
{
    if(!m_instylesheet)
    {
        m_curr  = m_curr.parentNode();
    }

    if(qName.compare("stylesheet", Qt::CaseInsensitive) == 0)
    {
        m_instylesheet  = false;

        (*m_stylesheet) += "} ";
    }


    return  true;
}

//  loadStylesheet
//  -------------------------------------------------------------------------
void        loadXml(    const char*     xml,
                        QString&        stylesheet,
                        QDomDocument&   layout)
{
    Xml2Stylesheet  xml2Stylesheet;

    xml2Stylesheet.loadXml(xml, stylesheet, layout);
}

void        setGeometry(QWidget*        widget,
                        QDomNode        layout)
{
    QString x   = layout.toElement().attribute("x");
    QString y   = layout.toElement().attribute("y");
    QString w   = layout.toElement().attribute("w");
    QString h   = layout.toElement().attribute("h");

    Q_ASSERT(!x.isNull());
    Q_ASSERT(!y.isNull());
    Q_ASSERT(!w.isNull());
    Q_ASSERT(!h.isNull());

    QPoint  pos(x.toInt(), y.toInt());
    QSize   size(w.toInt(), h.toInt());

    if(widget->parentWidget())
    {
//        pos = widget->parentWidget()->mapFromGlobal(pos);
    }

    widget->move(pos);
    widget->setMinimumSize(size);
    widget->setMaximumSize(size);
    widget->resize(size);
//    widget->raise();
//    widget->show();
}

bool        doesExist(  const QString&  filename)
{
    QFile   file(filename);
    return  file.exists();
}

uint        getFileSize(  const QString&  filename)
{
    QFile   file(filename);

    return  file.size();
}

QString     getSpeed(   unsigned long   size,
                        unsigned long   elapsed)
{
    if(elapsed != 0)
    {
        double  speed   = (double)size / ((double)elapsed / 1000);
        QString str;

        if(speed > 1000000)
        {
            str.setNum(speed / 1000000, 'f', 2);
            return QString(QObject::tr("%1 MB/s")).arg(str);
        }
        else if (speed > 1000)
        {
            str.setNum(speed / 1000, 'f', 2);
            return QString(QObject::tr("%1 KB/s")).arg(str);
        }
        else
        {
            str.setNum(speed / 1000, 'f', 2);
            return QString(QObject::tr("%1 B/s")).arg(str);
        }
    }
    else
    {
        return  QObject::tr("n/a");
    }
}

extern char *StartDirPtr();

//  getFilename
//  -------------------------------------------------------------------------
#ifndef ANDROID_VERSION
QString     getFilename(const QString&  filename,
                        bool            assert)
{
	QString name = filename; 
#ifndef ANDROID_VERSION
    name    = (QString)StartDirPtr() + "/" + filename;
    QFile   file1(name);
#else
    name    = (QString)StartDirPtr() + "/sdcard/"PROJECTNAME +"/"+ filename;
    QFile   file1(name);
#endif
    if(file1.exists())
        return  QFileInfo(file1).canonicalFilePath();
    name    = ":/" + filename;
    QFile   file2(name);
    if(file2.exists())
        return  name;

    if(assert)
    {
        QMessageBox::critical(
                        0,
                        QObject::tr(PROJECTNAME),
                        QString(QObject::tr("cannot find file '%1', please reinstall the program"))
                            .arg(filename),
                        QMessageBox::Ok);
        qApp->quit();
		return "";
    }
    else
    {
        return  "";
    }

}
#else
QString     getFilename(const QString&  filename,
                        bool            assert)
{
    QString name    = filename;

    QFile   file1(name);
    if(file1.exists())
        return  filename;

    name    = "/sdcard/"PROJECTNAME"/" + name;
    QFile   file2(name);
    
    if(file2.exists()){
        return  name;
    }
    
    if(assert)
    {
        QMessageBox::critical(
                        0,
                        QObject::tr("voip2car"),
                        QString(QObject::tr("cannot find file '%1', please reinstall the program"))
                            .arg(filename),
                        QMessageBox::Ok);
        qApp->quit();
    }
    else
    {
        return  "";
    }

}

#endif
//  getQPushButtonStylesheetHelper
//  -------------------------------------------------------------------------
QString     getQPushButtonStylesheetHelper(
                        const QString&  a,
                        const QString&  b)
{
    if(doesExist(a))
        return  a;

    if(doesExist(b))
        return  b;
    return  "";
}

//  getQPushButtonStylesheet
//  -------------------------------------------------------------------------
QString     getQPushButtonStylesheet(
                        const QString&  objname)
{
    QString     stylesheet;
    QString     stem;
    QString     name;

    if(objname.contains("_") == false)
    {
        Q_ASSERT(0);
        return  "";
    }

    QStringList list    = objname.split("_");

    stem    = list[0] + "Form";
    name    = list[1];

    QString     filename0   = getFilename(QString("qss/%1/%2.png").arg(stem).arg(name), false);
    QString     filename4   = getFilename(QString("qss/%1/%2-hover.png").arg(stem).arg(name), false);
    QString     filename8   = getFilename(QString("qss/%1/%2-disabled.png").arg(stem).arg(name), false);

    while(!name.isEmpty() && QString("0123456789").contains(name[name.length() - 1]))
    {
        name.resize(name.length() - 1);
    }

    QString     filename2   = getFilename(QString("qss/%1/%2.png").arg(stem).arg(name), false);
    QString     filename6   = getFilename(QString("qss/%1/%2-hover.png").arg(stem).arg(name), false);
    QString     filenameA   = getFilename(QString("qss/%1/%2-disabled.png").arg(stem).arg(name), false);

    QString     filename            = getQPushButtonStylesheetHelper(filename0, filename2);
    QString     filename_hover      = getQPushButtonStylesheetHelper(filename4, filename6);
    QString     filename_disabled   = getQPushButtonStylesheetHelper(filename8, filenameA);

    if(filename != "")
    {
        stylesheet.append(
                    QString("#%1 {background-image : url(%2);}\n").arg(objname).arg(filename));
    }

    if(filename_hover != "")
    {
        stylesheet.append(
                    QString("#%1:hover:!pressed {background-image : url(%2);}\n").arg(objname).arg(filename_hover));
    }

    if(filename_disabled != "")
    {
        stylesheet.append(
                    QString("#%1:disabled {background-image : url(%2);}\n").arg(objname).arg(filename_disabled));
    }

    return  stylesheet;
}

//  loadForm|
//  -------------------------------------------------------------------------
QWidget*    loadForm(   const char*     formui)
{
    QUiLoader   loader;
    QFile       file(getFilename(formui));

    file.open(QFile::ReadOnly);
    QWidget*    widget  = loader.load(&file);
    file.close();

    if(!widget)
    {
        QMessageBox::critical(
                        0,
                        QObject::tr(PROJECTNAME),
                        QString(QObject::tr("cannot find file '%1', please reinstall the program"))
                            .arg(formui),
                        QMessageBox::Ok);
        qApp->quit();
    }

    QString     formName    = widget->objectName();
    QString     stem;

    if(formName.right(4) == "Form")
    {
        stem    = formName.left(formName.length() - 4);
    
        QString     filename   = getFilename(QString("qss/%1/background.png").arg(formName), false);
		
        if(filename != "")
        {
            widget->setStyleSheet(
                    QString("#%1 {background-repeat : repeat-xy;"
                                 "background-image  : url(%2);}").arg(formName).arg(filename));
        }

        foreach(QObject* child, widget->children())
        {
            QPushButton*    push        = qobject_cast<QPushButton*>(child);
            QLabel*         label       = qobject_cast<QLabel*>(child);
            QString         childName   = child->objectName();

            if(childName.left(stem.length() + 1) == stem + "_")
            {
                QString     name        = childName.mid(stem.length() + 1);

                if(push)
                {
                    QString stylesheet  = getQPushButtonStylesheet(childName);

                    if(stylesheet != "")
                    {
                        push->setStyleSheet(stylesheet);
                    }
                }

                if(label)
                {
                    QString     imagefile   = getFilename(QString("qss/%1/%2.png").arg(formName).arg(name), false);
                    if(imagefile != "")
                    {
                        QImage  image(imagefile);
                        label->setPixmap(QPixmap::fromImage(image));
                        label->setFrameStyle(QFrame::NoFrame);
                    }
                }
            }

        }
    }

    return  widget;
}

void        placeholder(QWidget*    parent, QWidget* widget, const char* name)
{
    QWidget*    temp;

    assign<QWidget>(parent, temp, name);

    temp->setLayout(new QVBoxLayout);
    temp->layout()->addWidget(widget);
    temp->layout()->setContentsMargins(0, 0, 0, 0);
    temp->layout()->setSpacing(0);
    widget->show();
}

void        assign(QWidget* parent, QPushButton*& widget, const char* name)
{
    static
    QSound      click(getFilename("sounds/click.wav"));

    widget  = parent->findChild<QPushButton*>(name);

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

    QObject::connect(widget, SIGNAL(clicked()), &click, SLOT(play()));
}

//  -------------------------------------------------------------------------
//#ifndef ANDROID_VIDEO
class VpVideoFactory
    : public VPVIDEOFACTORY
{
    public:
                        VpVideoFactory(
                            QWidget*        hParent);
        VPVIDEODATA*    New(IVPSTACK*       vps, 
                            VPCALL          vpcall, 
                            unsigned        fourccrx, 
                            unsigned        xresrx, 
                            unsigned        yresrx, 
                            unsigned        fourcc, 
                            unsigned        xres, 
                            unsigned        yres, 
                            unsigned        framerate, 
                            unsigned        quality);
    private:
        QWidget*    m_widget;
        int         m_winid;
        RECTANGLE   m_rect;
};

VpVideoFactory::VpVideoFactory(
                            QWidget*        widget)
   : m_widget(widget)
#ifndef ANDROID_VERSION
    , m_winid((int)widget->winId())
#endif
{
}
VPVIDEODATA*    VpVideoFactory::New(
                            IVPSTACK*       vps, 
                            VPCALL          vpcall, 
                            unsigned        fourccrx, 
                            unsigned        xresrx, 
                            unsigned        yresrx, 
                            unsigned        fourcc, 
                            unsigned        xres, 
                            unsigned        yres, 
                            unsigned        framerate, 
                            unsigned        quality)
{
    RECTANGLE   rect;
    QRect       qrect;
#ifndef ANDROID_VERSION

    qrect   = m_widget->rect();

    rect.left   = qrect.left();
    rect.right  = qrect.right();
    rect.bottom = qrect.bottom();
    rect.top    = qrect.top();
#endif
    VPVIDEODATA*    vd  = CreateVPVIDEO(vps, vpcall, fourccrx, xresrx, yresrx, fourcc, xres, yres, framerate, quality);
#ifndef ANDROID_VERSION
    vd->SetVideoWindowData((HWINDOW)m_winid, 'abcd', &rect, false);
#else
    vd->SetVideoWindowData(m_widget, 'abcd', &rect, false);
#endif
    vd->Start();
    return vd;
}

//#endif

class   VpAudioFactory
    : public VPAUDIODATAFACTORY
{
    public:
        VPAUDIODATA*    New(IVPSTACK*       vps,
                            VPCALL          vpcall,
                            int             codec)
        {
#if SIMPLEAUDIO
            return CreateVPSIMPLEAUDIO(vps, vpcall, codec);
#else
            return CreateVPMIXINGAUDIO(vps, vpcall, codec);
#endif
        }
};

//#ifndef ANDROID_VIDEO
VPVIDEODATAFACTORY* vpvideoFactory(QWidget* widget)
{
    VPVIDEODATAFACTORY* _vpvideoFactory = NULL;

    _vpvideoFactory = new VpVideoFactory(widget);

    return  _vpvideoFactory;
}
//#endif
static
QScopedPointer<IVPSTACK>    g_vpstack;

IVPSTACK*           vpstack()
{
    static
    IVPSTACK*   _vpstack    = NULL;
    static
    IVPSTACK*   _netStack   = NULL;
#if defined _WIN32 && !defined _WIN32_WCE
    static
    IVPSTACK*   _gsmStack   = NULL;
#endif

    if(_vpstack == NULL)
    {
#ifdef ANDROID_VERSION
    QSettings settings("/sdcard/"PROJECTNAME"/"PROJECTNAME".ini",QSettings::IniFormat);
#else
    QSettings   settings;
#endif
        QString     video   = settings.value("video/device").toString();
        QString     oAudio  = settings.value("audio/oDevice").toString();
        QString     iAudio  = settings.value("audio/iDevice").toString();
        unsigned    oLevel  = settings.value("audio/oLevel").toUInt();
        unsigned    iLevel  = settings.value("audio/ilevel").toUInt();

        VPVIDEO_SetCaptureDevice(qPrintable(video));
#if SIMPLEAUDIO
#else

        VPMIXINGAUDIO_SetAudioDevice(0, qPrintable(oAudio));
        VPMIXINGAUDIO_SetAudioDevice(1, qPrintable(iAudio));
        VPMIXINGAUDIO_SetLevel(0, oLevel);
        VPMIXINGAUDIO_SetLevel(1, iLevel);
#endif
//        strcpy(videocapturedevice, qPrintable(video));

        VPInit();
#if SIMPLEAUDIO
	VPSIMPLEAUDIO_Init();
#else
        VPMIXINGAUDIO_Init();
#endif
        VPVIDEO_Init();

        _netStack   = CreateVPSTACK( new VpAudioFactory, "net");

/*

	    176x144 for android 
	    android camera supports only 176x144 320x240 640x480....
	    
	    resolution 160x120 unsupported!!!
*/
#ifdef ANDROID_VERSION

	LOGI("InitDefaultVideoParams 176x144\n");

//	_netStack->SetDefaultVideoParameters(mmioFOURCC('H','2','6','4'), 160, 120, 10, 0x35);
//emulator & devices with android
	_netStack->SetDefaultVideoParameters(mmioFOURCC('H','2','6','4'), 176, 144, 10, 0x35);
//For 2.3 version test
//	_netStack->SetDefaultVideoParameters(mmioFOURCC('H','2','6','4'), 720, 480, 10, 0x35);
//For 2.2HTC version test
//	_netStack->SetDefaultVideoParameters(mmioFOURCC('H','2','6','4'), 640, 480, 10, 0x35);

#else
	_netStack->SetDefaultVideoParameters(mmioFOURCC('H','2','6','4'), 196, 144, 10, 0x35);
#endif
#if defined _WIN32 && !defined _WIN32_WCE
        _gsmStack   = CreateGSMSTACK(new VpAudioFactory, "gsm");
#endif

        _netStack->SetBindPort(10675);
        _netStack->Init();
#if defined _WIN32 && !defined _WIN32_WCE
        _gsmStack->Init();
#endif

//#ifdef _DEBUG
		if(generatelog >= 2)
			_netStack   = new VpStackDbgLogDecorator(   _netStack);
#if defined _WIN32 && !defined _WIN32_WCE
        _gsmStack   = new VpStackDbgLogDecorator(   _gsmStack);
#endif

//#endif

#if defined _WIN32 && !defined _WIN32_WCE
        _vpstack    = new VpStackChooserDecorator(_netStack, _gsmStack);
#else
		_vpstack  =_netStack;
#endif
		if(generatelog >= 2)
			_vpstack    = new VpStackDbgLogDecorator(   _vpstack);

        _vpstack    = new VpStackLogDecorator(      _vpstack);
        _vpstack    = new VpStackAolDecorator(      _vpstack);
        _vpstack    = new VpStackContactDecorator(  _vpstack);
        _vpstack    = new VpStackAnsweringMachineDecorator(
                                                    _vpstack);
        _vpstack    = new VpStackSoundDecorator(    _vpstack);

        g_vpstack.reset(_vpstack);
    }

    return  _vpstack;
}

bool    InitDatabase()
{
QString DBname;

    QSqlDatabase    db  = QSqlDatabase::addDatabase("QSQLITE");
#ifdef HOME_PATH_SDCARD
    db.setDatabaseName("/sdcard/"PROJECTNAME"/"PROJECTNAME".db");
#else
    db.setDatabaseName(PROJECTNAME".db");
#endif
    
    if(!db.open())
    {
        QSqlError   error   = QSqlDatabase::database().lastError();
        QMessageBox::critical(0, qApp->tr("Cannot open database"), error.databaseText(), QMessageBox::Ok);
        return false;
    }
    QStringList tables  = db.tables();

    //if(tables.isEmpty())
    {
        QFile   file(getFilename(PROJECTNAME".sql"));
        file.open(QFile::ReadOnly);
        QString sql     = file.readAll();

        QStringList commands    = sql.split(";");
        db.transaction();
        foreach(QString command, commands)
        {
            QSqlQuery query   = db.exec(command);
        }
        db.commit();
    }

    return  true;
}

//  OpenDatabase
//  -------------------------------------------------------------------------
bool    OpenDatabase(   const QString&  username,
                        const QString&  vpnumber,
                        const QString&  password)
{
    QSqlQuery   query(QString("SELECT id FROM user WHERE username == '%1'").arg(username));

    if(!query.next())
    {
        QSqlQuery   query(QString("INSERT INTO user VALUES(NULL, '%1', '%2')").arg(username).arg(vpnumber));
        
        query.exec();
    }

    return  true;
}

static
QString     g_username;
static
QString     g_vpnumber;

//  setUsername
//  -------------------------------------------------------------------------
void        setUsername(const QString&  username)
{
    if(username == "")
    {
#ifdef ANDROID_VERSION
    QSettings settings("/sdcard/"PROJECTNAME"/"PROJECTNAME".ini",QSettings::IniFormat);
#else
    QSettings   settings;
#endif
        g_username  = settings.value("login/username").toString();
    }
    else
    {
        g_username  = username;
    }
}

//  getUsername
//  -------------------------------------------------------------------------
QString     getUsername()
{
    return  g_username;
}

//  setVpNumber
//  -------------------------------------------------------------------------
void        setVpNumber(const QString&  vpnumber)
{
    g_vpnumber  = vpnumber;
}

//  getVpNumber
//  -------------------------------------------------------------------------
QString     getVpNumber()
{
    return  g_vpnumber;
}


//  setUserData
//  -------------------------------------------------------------------------
void        setUserData(const QString&  username,
                        const QString&  key,
                        const QString&  value)
{
    QSqlQuery       query;
    query   = QString("INSERT OR REPLACE INTO settings SELECT id, '%1', '%2' FROM user WHERE username = '%3'")
                    .arg(key)
                    .arg(value)
                    .arg(username);
    query.exec();
}

//  getUserData
//  -------------------------------------------------------------------------
QString     getUserData(const QString&  username,
                        const QString&  key)
{
    QSqlQuery       query;
    query   = QString("SELECT value FROM settings JOIN user ON settings.owner = user.id WHERE user.username = '%1' AND settings.key = '%2'")
                    .arg(username)
                    .arg(key);
    if(query.next())
    {
        return  query.value(0).toString();
    }
    return  "";
}


//  QueryOnline
void    QueryOnline(const QString&  vpnumber)
{
    AOL*    aol = new AOL;
    AOLNUM* num = new AOLNUM;

    aol->notificationreqop  = NOTIFICATIONREQ_ENABLE;
    aol->N                  = 0;

    aol->nums   = num;

    num->srvid  = vpnumber.left(3).toInt();
    num->num    = vpnumber.right(vpnumber.length() - 3).toInt();
    num->online = 0;

    vpstack()->AskOnline(aol);
}

//  txAbook
//  -------------------------------------------------------------------------
void        txAbook()
{
    QStringList update;
    //  update id
    QSqlQuery   query;

    //update.append(QString("DELETE FROM contact WHERE owner = '%1';\n").arg("%1"));

    query.exec(QString("SELECT contact.* FROM contact JOIN user ON contact.owner = user.id WHERE user.username = '%1'")
               .arg(getUsername()));
    LOG_SQL_QUERY(query);

    while(query.next())
    {
        QSqlRecord      record  = query.record();
        QStringList     varval;

        update.append(QString("INSERT OR IGNORE INTO contact (id, username, owner) VALUES(NULL, '%1', '%2');\n")
                            .arg(record.value("username").toString())
                            .arg("%1"));

        for(int i = 0; i < record.count(); ++i)
        {
            QString     var = record.field(i).name();
            QString     val = record.field(i).value().toString();

            if(var != "username" && var != "owner" && var != "id")
            {
                varval.append(QString("'%1' = '%2'")
                                .arg(var)
                                .arg(val));
            }
        }

        QString         command;

        command = QString("UPDATE contact SET ")
                + varval.join(", ")
                + QString("WHERE username = '%1' AND owner = '%2';\n")
                    .arg(record.value("username").toString())
                    .arg("%1");
        update.append(command);
    }

    QTemporaryFile  file;

    file.open();

    QTextStream     stream(&file);

    foreach(QString line, update)
    {
        stream  << line;
    }

    file.setAutoRemove(false);

    file.close();

    vpstack()->ServerTransfer(TCPFT_TXAB, qPrintable(file.fileName()), true);
}

QString     getReason(  unsigned        reason)
{
    static
    const char* reasons[] =
    {
        "NORMAL",               "BEARER_NOT_SUPPORTED", "CODEC_NOT_SUPPORTED",  "CALLPROCESSING",
        "MISSINGCLID",          "CALLDEFLECTION",       "BUSY",                 "FILESAME",
        "TIMEOUT",              "DISCONNECTED",         "AUTHERROR",            "NOTFOUND",
        "SYNTAXERROR",          "INVALIDNUMBER",        "STORAGEEXCEEDED",      "ALREADYCONNECTED",
        "PROTOCOLNOTSUPPORTED", "SERVERREDIRECTION",    "ADMINERROR",           "ACCOUNTBLOCKED",
        "CALLTRANSFERCOMPLETE", "NOQUOTA",              "USEROFFLINE",          "SERVERNOTRESPONDING",
        "SERVERNOTRUNNING",     "VPHONENOTRESPONDING",  "VPHONENOTRUNNING",     "CALLPROCEEDING",
        "CONNECTIONLOST",       "WRONGVERSION",         "EARLYAUDIO",           "ANOTHERPORT",
        "ANOTHERADDR",          "CANNOTNOTIFY",         "NOTKNOWN",             "LOOPCALL",
        "VOICE2VIDEO"};

    if(reason >= 0 && reason <= sizeof(reasons) / sizeof(reasons[0]))
        return reasons[reason];

    if(reason == REASON_LOGGEDON)
        return QObject::tr("Logged on");

    if(reason == REASON_VERSIONOLD)
        return QObject::tr("Version old");

    if(reason == REASON_SERVERRESET)
        return QObject::tr("No server running on the specified host");
    return QObject::tr("Unknown");
}

void    LogWrite(   const QString&      text)
{
    static
    QFile*      file    = NULL;
    static
    QTextStream*stream;

	if(generatelog < 2)
		return;
    if(file == NULL)
    {
        QString     datetime    = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
#ifdef HOME_PATH_SDCARD
        QString     filename    = QDir("/sdcard").path() + "/"PROJECTNAME"/Data/log-%1.log";
#else
        QString     filename    = QDir::homePath() + "/"PROJECTNAME"/Data/log-%1.log";
#endif
        file    = new QFile(filename.arg(datetime));
        file->open(QIODevice::WriteOnly);

        stream  = new QTextStream;
        stream->setDevice(file);
    }

    (*stream)   << text;

    stream->flush();
}

void    LogSqlQuery(const QSqlQuery&    query)
{
    static
    QFile*      file    = NULL;
    static
    QTextStream*stream;

	if(generatelog < 2)
		return;
    if(file == NULL)
    {
        QString     datetime    = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
#ifdef HOME_PATH_SDCARD
        QString     filename    = QDir("/sdcard").path()  + "/"PROJECTNAME"/Data/sqlquery-%1.log";
#else
        QString     filename    = QDir::homePath() + "/"PROJECTNAME"/Data/sqlquery-%1.log";
#endif
        file    = new QFile(filename.arg(datetime));
        file->open(QIODevice::WriteOnly);

        stream  = new QTextStream;
        stream->setDevice(file);
    }

    (*stream)   << "query: " << query.lastQuery()       << endl;
    (*stream)   << "error: " << query.lastError().text()<< endl << endl;
}

//  -------------------------------------------------------------------------
class   ConcreteVpMessageSubject
    : public VpMessageSubject
{
    public:
        ConcreteVpMessageSubject();
        void        addObserver(
                            VpMessageObserver*  observer);
        void        delObserver(
                            VpMessageObserver*  observer);
        void        message(
                            unsigned            msg,
                            unsigned            pcall,
                            unsigned            param);
    private:
        QList<VpMessageObserver*>   m_observer;
};

ConcreteVpMessageSubject::ConcreteVpMessageSubject()
{

}

void        ConcreteVpMessageSubject::addObserver(
                    VpMessageObserver*  observer)
{
    m_observer.append(observer);
}

void        ConcreteVpMessageSubject::delObserver(
                    VpMessageObserver*  observer)
{
    for(QList<VpMessageObserver*>::iterator it = m_observer.begin(); it != m_observer.end(); ++it)
    {
        if(*it == observer)
        {
            m_observer.erase(it);
            break;
        }
    }
}

void        ConcreteVpMessageSubject::message(
                    unsigned            msg,
                    unsigned            pcall,
                    unsigned            param)
{
    foreach(VpMessageObserver* observer, m_observer)
    {
        observer->message(this, msg, pcall, param);
    }
}

VpMessageSubject*   vpMessageSubject()
{
    static
    VpMessageSubject*   _subject    = 0;

    if(_subject == 0)
    {
        _subject    = new ConcreteVpMessageSubject;
    }

    return  _subject;
}

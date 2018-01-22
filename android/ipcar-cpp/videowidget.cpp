#include "videowidget.h"
#include <QApplication>
#include <QWidget>
videowidget::videowidget(QWidget *parent) :
    QWidget(parent)
{
    connect(&thread, SIGNAL(renderedImage(QImage)),
                this, SLOT(setPicture(QImage)));
    connect(&thread, SIGNAL(finished()),
                this, SLOT(stopVideoThread()));
            
    setGeometry ( 0, 0, 400, 300 );
//    setMaximumSize(parent->maximumSize());
//    setMaximumSize(maximumSize());
//    setSizePolicy ( QSizePolicy::Policy horizontal | QSizePolicy::Policy vertical );
//    thread.start();
    setAutoFillBackground(true);
}

void videowidget::paintEvent(QPaintEvent *) 
{
    QPainter painter(this);
    painter.drawPixmap(((QWidget*)parent())->rect(),pixmap);
}

void videowidget::setPicture(QImage i)
{
    pixmap=QPixmap::fromImage(i);
    update();    
    qApp->processEvents();
}

void videowidget::stopVideoThread()
{
LOGI("Thread end. Load Picture Default\n");
#if 1
    QImage *qq=new QImage();
     //!!!!!!!should be replaced with getFilename();
     if(qq->load("/sdcard/IPCar/qss/videoCallForm/waiting.png"))
     {
//    ((videowidget*)video)->SetImgloaded();
//     emit renderedImage(*qq);
//    ((videowidget*)video)->setPicture(*qq);
        pixmap=QPixmap::fromImage(*qq);
     }
     delete qq;
     update();
     qApp->processEvents();
#endif
}


void videowidget::startthread(){
    thread.start();
}

videowidget::~videowidget(){
    LOGI("videowidget::~videowidget()\n");
}

bool videowidget::threadisRunning()
{
    return thread.isRunning(); 
}
int videowidget::Setsizex(int x)
{
    thread.imgx = x;
}
int videowidget::Setsizey(int y)
{
    thread.imgy = y;
}
unsigned char *videowidget::Getbuffer()
{
    return thread.buffer;
}
int videowidget::setCallEnded(int status)
{
    thread.callended = status;
}

volatile int videowidget::getCallEnded()
{
    return thread.callended;
}

volatile int videowidget::getImgLoaded()
{
    return thread.imgloaded;
}

volatile int videowidget::setImgLoaded(int status)
{
    thread.imgloaded = status;
}






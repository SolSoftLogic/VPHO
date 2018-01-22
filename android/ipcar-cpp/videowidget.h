#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QQueue>
#include <QMutex>
#include <QPixmap>

#include "widgethread.h"

class videowidget : public QWidget
{
Q_OBJECT
public:
    videowidget(QWidget *parent = 0);
    ~videowidget();

    QPixmap pixmap;
    CaptureThread thread;
    
    bool  threadisRunning();
    void startthread();
    int Setsizex(int x);
    int Setsizey(int y);
    int setCallEnded(int status);
    volatile    int getCallEnded();
    volatile int getImgLoaded();
    volatile int setImgLoaded(int status);


   unsigned char * Getbuffer();
    
protected:
    void paintEvent(QPaintEvent *event);

signals:

public slots:
    void setPicture(QImage);
    void stopVideoThread();
private:


};

#endif // VIDEOWIDGET_H

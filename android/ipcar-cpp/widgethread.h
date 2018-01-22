#ifndef CAPTURETHREAD_H
#define CAPTURETHREAD_H

#include <QThread>
#include <QImage>
#include <QMutex>
#include <QWaitCondition>

#include <android/log.h>
#define  LOG_TAG    "CaptureThread"

#if 0
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#else
#define  LOGI(...)  while(0){;}
#define  LOGE(...)  while(0){;}
#endif

class CaptureThread : public QThread
{
    Q_OBJECT

public:
    CaptureThread(QObject *parent = 0);
    ~CaptureThread();
    volatile    int imgloaded;
    volatile  int callended;
    unsigned char * buffer;
    int imgx,imgy;
    friend void VideoPutFrame(void *video,unsigned char *data ,int sizex,int sizey);
    friend void VideoTestFrame(void *video,unsigned char *data ,int sizex,int sizey);
    friend void VideoConnectFrame(void  *video,unsigned char *data ,int sizex,int sizey);

    void stopUlan();
    void startUlan();

protected:
    void run();
signals:
    void renderedImage(const QImage &image);

private:
    QMutex mutex;
    QWaitCondition condition;
};

#endif // CAPTURETHREAD_H

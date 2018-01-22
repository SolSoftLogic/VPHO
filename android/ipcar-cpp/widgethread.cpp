#include <QImage>
#include "widgethread.h"
#include "videowidget.h"
#define CLEAR(x) memset(&(x), 0, sizeof(x))

CaptureThread::CaptureThread(QObject *parent) : QThread(parent)
{
    imgloaded = 0;
    callended = 0; //set to 1 for thread termination
    LOGI("CaptureThread::CaptureThread(QObject *parent) : QThread(parent)\n");
    //allocate predefined buffer
    buffer =  (unsigned char *)malloc(1024*768*3); //maX size
    LOGI("CaptureThread::CaptureThread buffer size %i ,buffer %p\n",1024*768*3,buffer);

    imgx=0;
    imgy=0;
}

void CaptureThread::run(){
int x =0;
int y =0;
LOGI("CaptureThread: Started\n");

    while(!callended)
    {

	if  (imgloaded)
	{
    
	    LOGI("CaptureThread: IMG loaded Processing.....\n");

		x = imgx;
		y = imgy;
	
		if ( (x >0)  && (y>0))
		{ //size is set
	    
    		    QImage *qq=new QImage(buffer,x,y,QImage::Format_RGB888);
#ifndef YUV2RGBSWAP
    		    *qq = qq->rgbSwapped();
#endif	        		    
#ifndef YUV2RGBMIRROR
    		    *qq = qq->mirrored(false,true);
#endif	    
    	    
	    	    emit renderedImage(*qq);
		    delete qq;
    		}
    	    imgloaded=0;
//        msleep(100);
	}
    msleep(100);
    }
callended = 0;
LOGI("CaptureThread: Ended\n");
}
CaptureThread::~CaptureThread()
{
}
void CaptureThread::stopUlan()
{
}

void CaptureThread::startUlan()
{
    this->start();
}

void VideoPutFrame(void  *video,unsigned char *data ,int sizex,int sizey)
{

	volatile    int imgloaded;
	unsigned char * buf;
	int x,y;
	//thread
	
	if ( ! ((videowidget*)video)->threadisRunning() )
	{
	    ((videowidget*)video)->startthread();
	    LOGI("CaptureThread: startthread: \n");

	}
	if ( !((videowidget*)video)->getImgLoaded() )
	{
	     ((videowidget*)video)->Setsizex(sizex);
	     ((videowidget*)video)->Setsizey(sizey);
	     buf = ((videowidget*)video)->Getbuffer();
             memcpy(buf,data,sizex*sizey*3);
    	
	     LOGI("PUT FRAME FOR IMG THEAD \n");
    	    ((videowidget*)video)->setImgLoaded(1);
	}
}
void VideoTestFrame(void  *video,unsigned char *data ,int sizex,int sizey)
{
//        QImage *qq=new QImage(sizex,sizey,QImage::Format_RGB888);
//        if(qq->load("/sdcard/image.png")){
//	    ((videowidget*)video)->SetImgloaded();
            //emit renderedImage(*qq);
//            ((videowidget*)video)->setPicture(qq);
//	}

}

void VideoConnectFrame(void  *video,unsigned char *data ,int sizex,int sizey)
{
	((videowidget*)video)->setCallEnded(1);
#if 0
	LOGI("CaptureThread: Wait For END < >!!!\n");
	while (((videowidget*)video)->getCallEnded() )
	{
		usleep(100);
	}
	LOGI("CaptureThread:  WAit ENDED\n");
#endif
#if 0
//         QImage *qq=new QImage();//sizex,sizey,QImage::Format_RGB888);
//         if(qq->load("/sdcard/IPCOM-Group/qss/videoCallForm/waiting.png")){
//	    ((videowidget*)video)->SetImgloaded();
//            emit renderedImage(*qq);
//            ((videowidget*)video)->setPicture(*qq);
//	 }
//	 delete qq;
#endif
}

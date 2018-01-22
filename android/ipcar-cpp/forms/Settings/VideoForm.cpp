#include "VideoForm.h"
#include "WidgetFactory.h"

#include <QtGui/QLabel>
#include <QtCore/QSettings>
//  VideoForm::VideoForm
//  --------------------------------------------------------------------------
VideoForm::VideoForm(QWidget *parent)
    : QWidget(parent)
{
    QWidget*    widget  = loadForm("qss/forms/settings/video.ui");

    QVBoxLayout*layout  = new QVBoxLayout;

    layout->addWidget(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    this->setLayout(layout);

    assign(this, m_device,      "video_device");
    assign(this, m_test,        "video_test");
//    assign(this, m_image,        "video_test");

    char    cdev[10][100];
    int     n   = sizeof(cdev) / sizeof(cdev[0]);
	int cur = -1;
#ifdef ANDROID_VIDEO    
    VPVIDEO_EnumerateCaptureDevices(cdev, &n);
#endif
#ifdef ANDROID_VERSION
    QSettings settings("/sdcard/"PROJECTNAME"/"PROJECTNAME".ini",QSettings::IniFormat);
#else
    QSettings   settings;
#endif
    QString     device  = settings.value("video/device").toString();
    bool        found   = false;

    for(int i = 0; i < n; ++i)
    {
        m_device->addItem(cdev[i]);

        if(device == cdev[i])
		{
            found   = true;
			cur = i;
		}
    }

	m_device->setCurrentIndex(cur);
    if(!found && n != 0)
    {
        settings.setValue("video/device", cdev[0]);
    }

    connect(m_device, SIGNAL(currentIndexChanged(QString)), this, SLOT(onDeviceChanged(QString)));

    m_video = NULL;
    m_vfactory = NULL;

    img_video = new videowidget(m_test);
 
    LOGI("img_video widget = %p\n",img_video);
}

//  VideoForm::onDeviceChanged
//  -------------------------------------------------------------------------
void    VideoForm::onDeviceChanged(const QString&  device)
{
#ifdef ANDROID_VERSION
    QSettings settings("/sdcard/"PROJECTNAME"/"PROJECTNAME".ini",QSettings::IniFormat);
#else
    QSettings   settings;
#endif
    settings.setValue("video/device", device);
#ifdef ANDROID_VIDEO    
	VPVIDEO_SetCaptureDevice(qPrintable(device));
#endif
}

//  VideoForm::showEvent
//  -------------------------------------------------------------------------
void    VideoForm::showEvent(QShowEvent *)
{
    unsigned    fourcc;
    unsigned    w;
    unsigned    h;
    unsigned    fr;
    unsigned    q;
    RECTANGLE   rect;
    QRect       qrect   = m_test->rect();

    vpstack()->GetDefaultVideoParameters(&fourcc, &w, &h, &fr, &q);
#ifdef ANDROID_VIDEO
//    m_vfactory  = vpvideoFactory(m_test);
    m_vfactory  = vpvideoFactory(img_video);
#endif    
    m_video     = m_vfactory->New(0, 0, fourcc, w, h, fourcc, w, h, fr, q);
#ifndef ANDROID_VERSION
    rect.left   = qrect.left();
    rect.right  = qrect.right();
    rect.top    = qrect.top();
    rect.bottom = qrect.bottom();
#endif
#ifdef ANDROID_VERSION
//    m_video->SetVideoWindowData((void *)m_test->winId(), 'abcd', &rect, false);
    m_video->SetVideoWindowData((void *)img_video, 'abcd', &rect, false);
#else
    m_video->SetVideoWindowData(m_test->winId(), 'abcd', &rect, false);
#endif
}

//  VideoForm::hideEvent
//  -------------------------------------------------------------------------
void    VideoForm::hideEvent(QHideEvent *)
{
    if(m_video != NULL)
    {

        delete m_video;
        m_video = 0;

    }
	if(m_vfactory != NULL)
	{

        delete m_vfactory;
        m_vfactory = 0;

	}
}

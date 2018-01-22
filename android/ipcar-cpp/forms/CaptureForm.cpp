#include "CaptureForm.h"
#include "Utils.h"
#include "Commands.h"

#include <QtGui/QVBoxLayout>

#define CLOSE_TIMEOUT_msec  5000

CaptureForm::CaptureForm(QWidget *parent) :
    QWidget(parent)
{
    QWidget*    widget  = loadForm("qss/forms/capture.ui");

    QVBoxLayout*layout  = new QVBoxLayout;

    layout->addWidget(widget);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    this->setLayout(layout);

    assign(this, m_image,   "capture_image");
    assign(this, m_close,   "capture_close");

    connect(m_close, SIGNAL(clicked()), this,   SLOT(onCloseClicked()));
    connect(&m_timer,SIGNAL(timeout()), this,   SLOT(onCloseTimeout()));

    m_pixmap    = NULL;

    m_image->setMargin(0);
}

//  CaptureForm::onCloseClicked
//  -------------------------------------------------------------------------
void    CaptureForm::onCloseClicked()
{
    hideMe(this);
}

//  CaptureForm::onCloseTimeout
//  -------------------------------------------------------------------------
void    CaptureForm::onCloseTimeout()
{
    hideMe(this);
}

//  CaptureForm::showEvent
//  -------------------------------------------------------------------------
void    CaptureForm::showEvent(QShowEvent *)
{
    m_timer.start(CLOSE_TIMEOUT_msec);
}

//  CaptureForm::hideEvent
//  -------------------------------------------------------------------------
void    CaptureForm::hideEvent(QHideEvent *)
{
    m_timer.stop();
}

//  CaptureForm::setFile
//  -------------------------------------------------------------------------
void    CaptureForm::setFile(const QString& filename)
{
    QPixmap*    pixmap  = new QPixmap(filename);
    m_image->setPixmap(*pixmap);

    if(m_pixmap)
        delete  m_pixmap;

    m_pixmap    = pixmap;
//    m_image->setStyleSheet(QString("QWidget {background-image: url(%1)}").arg(filename));
}

#ifndef VIDEOPANE_H
#define VIDEOPANE_H

#include "Utils.h"

#include <QtGui/QFrame>
#include <QtGui/QComboBox>
#include <QtGui/QGridLayout>
#include <QtGui/QVBoxLayout>
#include "videowidget.h"

class VideoForm
    : public QWidget
{
    Q_OBJECT
    public:
        explicit VideoForm(QWidget *parent = 0);

    signals:

    private slots:
        void    onDeviceChanged(const QString&  device);

    private:
        void    showEvent(QShowEvent *);
        void    hideEvent(QHideEvent *);

    private:
         QComboBox*             m_device;
         QWidget*               m_test;
         QImage			m_image;
//         videowidget*               m_test;
         VPVIDEODATAFACTORY*    m_vfactory;
         VPVIDEODATA*           m_video;
         videowidget*		img_video;
};

#endif // VIDEOPANE_H

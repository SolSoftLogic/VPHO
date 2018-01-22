#ifndef CAPTUREFORM_H
#define CAPTUREFORM_H

#include <QtGui/QWidget>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtCore/QTimer>

class CaptureForm 
    : public QWidget
{
    Q_OBJECT

    public:
        explicit    CaptureForm(QWidget *parent = 0);
        void        setFile(const QString&  filename);

    signals:

    public slots:
        void    onCloseClicked();
        void    onCloseTimeout();

    protected:
        void    showEvent(QShowEvent *);
        void    hideEvent(QHideEvent *);

    private:
        QPushButton*    m_close;
        QLabel*         m_image;
        QTimer          m_timer;
        QPixmap*        m_pixmap;

};

#endif // CAPTUREFORM_H

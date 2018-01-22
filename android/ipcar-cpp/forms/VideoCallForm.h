#ifndef VIDEOFORM_H
#define VIDEOFORM_H

#include "CallHandlerWidget.h"


#include <QtGui/QWidget>
#include <QtGui/QPushButton>
#include <QtGui/QScrollBar>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QDateTimeEdit>
#include <QtCore/QTimer>
#include <QtCore/QSet>
#include "videowidget.h"
class VideoCallForm
    : public CallHandlerWidget
    , public VpMessageObserver
{
    Q_OBJECT
    public:
        explicit    VideoCallForm(QWidget *parent = 0);

        void    onVpStackMessage(
                                unsigned        msg,
                                VPCALL          call,
                                unsigned        param);

        QWidget*
                getVideoWindow()
        {
            return  m_video;
        }

    private slots:
        void    onTimerTick();
        void    onEndClicked();
        void    onHoldClicked();
        void    onResumeClicked();
        void    onCaptureClicked();
        void    onAddClicked();
        void    onPipClicked();
        void    onSmsClicked();
        void    onChatClicked();
        void    onFileClicked();

    public:
        void    call(    const QString&  username);

    private:
        void        message(VpMessageSubject*   subject,
                            unsigned            msg,
                            unsigned            pcall,
                            unsigned            param);
        void        updateUI();

    private:
        QPushButton*    m_end;
        QPushButton*    m_sms;
        QPushButton*    m_chat;
        QPushButton*    m_file;
        QPushButton*    m_resume;
        QPushButton*    m_hold;
        QPushButton*    m_add;
        QPushButton*    m_pip;
        QPushButton*    m_capture;
        QWidget*        m_video;
        QWidget*        m_me;

        QLabel*         m_talking;
        QLabel*         m_datetime;

        QTimer          m_timer;
        int             m_counter;

        QLabel*         m_party;

        QSet<int>       m_calls;

        bool            m_fullscreen;

        VPVIDEODATAFACTORY*    m_vfactory;
        VPVIDEODATA*           m_vpvideo;
        videowidget	       *img_video;
};

#endif // VIDEOFORM_H

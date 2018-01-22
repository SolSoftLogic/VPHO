#ifndef MMSPANE_H
#define MMSPANE_H

#include "CallHandlerWidget.h"

#include <QtGui/QFrame>
#include <QtGui/QTextEdit>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QProgressBar>
#include <QtCore/QTime>

#include "Utils.h"


class MmsForm
    : public CallHandlerWidget
    , public VpMessageObserver
{
    Q_OBJECT
    public:
        explicit MmsForm(QWidget *parent = 0);

    signals:
        void    mmsSent(        bool            success,
                                const QString&  username);

    public:
        void    call(      const QString&  username);
    public slots:
        void    onShowMms(      int             id);
        void    onVpStackMessage(
                                unsigned        msg,
                                VPCALL          call,
                                unsigned        param);

    protected slots:
        void    onRecStartClicked();
        void    onRecStopClicked();
        void    onPlayStartClicked();
        void    onPlayStopClicked();
        void    onSendClicked();
        void    onCloseClicked();
        void    onPlayTimerTick();

    private:
        void        setupPlayer();
        void        message(VpMessageSubject*   subject,
                            unsigned            msg,
                            unsigned            pcall,
                            unsigned            param);
    private:
        QLabel*         m_username;
        QLabel*         m_vpnumber;
        QWidget*        m_video;
        QWidget*        m_videoin;

        QPushButton*    m_recStart;
        QPushButton*    m_recStop;
        QPushButton*    m_playStart;
        QPushButton*    m_playStop;
        QPushButton*    m_send;
        QPushButton*    m_close;
        QLabel*         m_speed;
        QLabel*         m_remain;
        QTime           m_start;
        uint            m_filesize;

        QProgressBar*   m_progress;

        VPCALL          m_call;
        bool            m_inCall;
//        QString         m_username;
        QString         m_avifile;
        QString         m_party;

        VPVIDEODATAFACTORY* m_vfactory;
        VPVIDEODATA*        m_videodata;
        IAVIPLAYER*         m_aviplayer;
        QTimer*             m_playTimer;
};

#endif // SMSPANE_H

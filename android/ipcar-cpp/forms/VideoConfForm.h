#ifndef VIDEOCONFFORM_H
#define VIDEOCONFFORM_H

#include "CallHandlerWidget.h"

#include <QtGui/QWidget>
#include <QtGui/QPushButton>
#include <QtGui/QLineEdit>
#include <QtCore/QSet>

#include "Utils.h"
#include "videowidget.h"
struct VideoConfPartner
{
    QLineEdit*      title;

    QPushButton*    end;
    QPushButton*    hold;
    QPushButton*    resume;
    QPushButton*    sms;
    QPushButton*    chat;
    QPushButton*    file;

    QWidget*        video;

    QString         name;
    VPCALL          call;
    bool            incall;
};

class VideoConfForm
    : public QWidget
    , public VpMessageObserver
{
        Q_OBJECT
    public:
        explicit    VideoConfForm(QWidget *parent = 0);

    signals:
        void        calling(int                 call,
                            const QString&      username,
                            int                 bearer);
        void        makeCall(
                            const QString&      username,
                            int                 bearer);
    private slots:
        void        onButttonClicked();

    private:
        void        showEvent(
                            QShowEvent*         event);
        void        updateUI();
        void        message(VpMessageSubject*   subject,
                            unsigned            msg,
                            unsigned            pcall,
                            unsigned            param);

    private:
        VideoConfPartner    m_partner[MAX_NUM_OF_PARTIES];
        QSet<int>           m_calls;
};

#endif // VIDEOCONFFORM_H

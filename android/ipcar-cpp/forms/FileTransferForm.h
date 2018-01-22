#ifndef FILETRANSFERPANE_H
#define FILETRANSFERPANE_H

#include "Utils.h"
#include "CallHandlerWidget.h"


#include <QtGui/QFrame>
#include <QtGui/QTableWidget>
#include <QtGui/QPushButton>
#include <QtGui/QLineEdit>
#include <QtGui/QProgressBar>
#include <QtGui/QLabel>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QTime>
#include <QtGui/QCheckBox>

class FileTransferForm
    : public CallHandlerWidget
{
    Q_OBJECT
    public:
        explicit FileTransferForm(QWidget *parent = 0);

    signals:

    private:
        void        call(      const QString&  username);
        void        updateUI();
        void        showEvent(  QShowEvent*     event);


    public slots:

        void        onNewCall(
                                VPCALL          call);
        void        onCallAccepted(
                                VPCALL          call);
        void        onCallRefused(
                                VPCALL          call,
                                int             reason);
        void        onVpStackMessage(
                                unsigned        msg,
                                VPCALL          pcall,
                                unsigned        param);
        void        onFileSelected(
                                const QString&  file);


    private slots:
        void            onAddClicked();
        void            onDelClicked();
        void            onSendClicked();
        void            onDiscClicked();
        void            onHoldClicked();
        void            onResumeClicked();

    private:
        QTableWidget*   m_table;
        QPushButton*    m_add;
        QPushButton*    m_del;
        QPushButton*    m_open;
        QPushButton*    m_send;
        QPushButton*    m_disc;
        QPushButton*    m_hold;
        QPushButton*    m_resume;
        QProgressBar*   m_current;
        QProgressBar*   m_total;
        QLabel*         m_speed;
        QLabel*         m_username;
        QCheckBox*      m_coc;

        QTime           m_time;

        int             m_receivedBytes;

        VPCALL          m_call;
        int             m_cfile;
        QList<QString>  m_files;

        CallType        m_calltype;
};

#endif // FILETRANSFERPANE_H

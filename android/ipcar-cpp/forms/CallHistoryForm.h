#ifndef CallHistoryForm_H
#define CallHistoryForm_H

#include <QtGui/QFrame>
#include <QtGui/QScrollBar>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QTableWidget>
#include <QtGui/QRadioButton>
#include <QtGui/QCheckBox>
#include <QtGui/QPushButton>
#include <QtGui/QCalendarWidget>
#include <QtGui/QTableWidget>
#include <QtCore/QSet>
#include <QtSql/QSqlQueryModel>

#ifndef   ANDROID_VERSION
#include <QtGui/QSound>
#else
#include "QSound.h"
#endif

class CallHistoryForm
    : public QWidget
{
    Q_OBJECT
    public:
        explicit CallHistoryForm(QWidget *parent = 0);
        void            doClear();

    signals:
        void            showSms(int id);
        void            showMms(int id);
        void            clearing();

    public slots:
        void            chooseDate();
        void            updateView();

        void            onClearClicked();
        void            onShowSms();

    private slots:
        void            onTableClicked(
                                const QModelIndex&    index);
        void            onDateSelectionChanged();
        void            onCallClicked();
        //void            onRowChanged(
        //                        const QModelIndex&      current,
        //                        const QModelIndex&      previous);


    private:
        void            showEvent(QShowEvent *);
        QString         filter();

    private:
        QTableView*     m_client;

        QRadioButton*   m_all;
        QRadioButton*   m_voice;
        QRadioButton*   m_video;
        QRadioButton*   m_videom;
        QRadioButton*   m_sms;
        QRadioButton*   m_chat;
        QRadioButton*   m_file;

        QCheckBox*      m_incoming;
        QCheckBox*      m_outgoint;
        QCheckBox*      m_answered;

        QPushButton*    m_chooseDate;
        QPushButton*    m_clear;
        QPushButton*    m_call;

        QCalendarWidget*m_calendar;

        QSqlQueryModel  model;

        QString         m_date;

        QSound          m_click;

};

#endif // CallHistoryForm_H

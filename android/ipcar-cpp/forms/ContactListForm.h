#ifndef ContactListForm_H
#define ContactListForm_H

#include <QtGui/QFrame>
#include <QtGui/QScrollArea>
#include <QtGui/QVBoxLayout>
#include <QtCore/QSignalMapper>
#include <QtGui/QTableView>
#include <QtSql/QSqlQueryModel>
#include <QtCore/QTimer>
#include <QtCore/QSemaphore>
#ifndef   ANDROID_VERSION
#include <QtGui/QSound>
#else
#include "QSound.h"
#endif
#include "Utils.h"

class ContactListForm
    : public QWidget
{
    Q_OBJECT
    public:
        explicit ContactListForm(
                            QWidget*            parent = 0);
        void    aolAck(     bool                nlast,
                            AOL*                aol);

    signals:
        void    editContact(int                 id);

        void    makeCall(   const QString&      username,
                            int                 bc);
        void    contactSelected(
                            const QString&      username);

    public:
        void    selectContact();

    public slots:
        void    onAbUpdate( const QString&      username,
                            const QString&      vpnumber);
        void    onLogon();
        void    onSelectContact();

    private slots:
        void    onTableClicked(
                            const QModelIndex&  index);
        void    onTimeout();

    private:
        void    showEvent(  QShowEvent*         event);
        void    hideEvent(  QHideEvent*         event);
        void    updateView();

    private:
        QTableView*         m_table;
        QTimer              m_timer;
        QSqlQueryModel      m_model;
        int                 m_lastClickedRow;
        QSound              m_click;
        bool                m_full;
};

#endif // ContactListForm_H

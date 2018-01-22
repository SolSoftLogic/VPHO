#ifndef ADDCONTACTPANE_H
#define ADDCONTACTPANE_H

//#include "AddressBook.h"
#include "LogEntry.h"

#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>

struct  Contact
{
    QString     name;
    QString     number;
    QString     sdial;

    QString     firstName;
    QString     lastName;
    QString     country;
    QString     state;

    QString     homepage;
    QString     email;
    QString     mobile;
    QString     office;

    void    reset()
    {
        name        = "";
        number      = "";
        sdial       = "";

        firstName   = "";
        lastName    = "";
        country     = "";
        state;

        homepage    = "";
        email       = "";
        mobile      = "";
        office      = "";
    }

    bool    operator==(Contact const& o)
    {
        return
                name        == o.name     &&
                number      == o.number   &&
                sdial       == o.sdial    &&

                firstName   == o.firstName&&
                lastName    == o.lastName &&
                country     == o.country  &&
                state       == o.state    &&

                homepage    == o.homepage &&
                email       == o.email    &&
                mobile      == o.mobile   &&
                office      == o.office;
    }

    bool    operator!=(Contact const& o)
    {
        return
                name        != o.name     ||
                number      != o.number   ||
                sdial       != o.sdial    ||

                firstName   != o.firstName||
                lastName    != o.lastName ||
                country     != o.country  ||
                state       != o.state    ||

                homepage    != o.homepage ||
                email       != o.email    ||
                mobile      != o.mobile   ||
                office      != o.office;
    }
};


class AddContactForm
    : public QWidget
{
    Q_OBJECT
    public:
        explicit    AddContactForm(
                                QWidget*        parent = 0);

    signals:
        void        deleting();

    public slots:
        void        doDelete();
        void        onEditContact(
                                int             id);
        void        onAddContact();
        void        onDelContact();

    private slots:
        void        onVisitHomepage();
        void        onSendEmail();
        void        onCallOffice();
        void        onCallMobile();
        void        updateUI();

    private:
        void        saveContact();
        void        hideEvent(  QHideEvent*     event);

    private:
        QLineEdit*  m_username;
        QLineEdit*  m_vpnumber;
        QPushButton*m_del;

        QLineEdit*  m_firstName;
        QLineEdit*  m_lastName;
        QLineEdit*  m_country;

        QLineEdit*  m_email;
        QLineEdit*  m_mobile;
        QLineEdit*  m_office;

        QPushButton*m_sendEmail;
        QPushButton*m_callMobile;
        QPushButton*m_callOffice;

        int         m_id;

        Contact     m_curr;
};

#endif // ADDCONTACTPANE_H

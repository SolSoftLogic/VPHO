#include "AddContactForm.h"
#include "WidgetFactory.h"
#include "Utils.h"
#include "Keyboard.h"
#include "Commands.h"

#include <QtGui/QLabel>
#include <QtCore/QFile>

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlError>
#include <QtSql/QSqlRelationalTableModel>

AddContactForm::AddContactForm(QWidget *parent)
    : QWidget(parent)
{
    QWidget*    widget  = loadForm("qss/forms/add.ui");

    QVBoxLayout*layout  = new QVBoxLayout;

    layout->addWidget(widget);
    layout->addWidget(new Keyboard);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    this->setLayout(layout);

    assign(this, m_username,    "add_name");
//    connect(m_username, SIGNAL(editingFinished()), SLOT(onNameChange()));

    assign(this, m_vpnumber,    "add_number");
    assign(this, m_del,         "add_del");

    assign(this, m_firstName,   "add_firstname");
    assign(this, m_lastName,    "add_lastname");
    assign(this, m_country,     "add_country");

    assign(this, m_email,       "add_email");
    connect(m_email,        SIGNAL(editingFinished()),  this,   SLOT(updateUI()));

    assign(this, m_mobile,      "add_mobile");
    connect(m_mobile,       SIGNAL(editingFinished()),  this,   SLOT(updateUI()));

    assign(this, m_office,      "add_office");
    connect(m_office,       SIGNAL(editingFinished()),  this,   SLOT(updateUI()));

    assign(this, m_del,         "add_del");
    connect(m_del, SIGNAL(clicked()), this, SLOT(onDelContact()));

    assign(this, m_sendEmail,       "add_sendEmail");
    connect(m_sendEmail,        SIGNAL(clicked()),  this,   SLOT(onSendEmail()));

    assign(this, m_callMobile,      "add_callMobile");
    connect(m_callMobile,       SIGNAL(clicked()),  this,   SLOT(onCallMobile()));

    assign(this, m_callOffice,      "add_callOffice");
    connect(m_callOffice,       SIGNAL(clicked()),  this,   SLOT(onCallOffice()));


    m_id    = -1;
    m_username->setText("");
}

//  AddContactForm::onEditContact
//  -------------------------------------------------------------------------
void    AddContactForm::onEditContact(
                    int                 id)
{
    QSqlQuery   query;

    query.exec(QString("SELECT * FROM contact JOIN user ON contact.owner = user.id WHERE user.username = '%1' AND contact.id = '%2'")
                    .arg(getUsername())
                    .arg(id));
    LOG_SQL_QUERY(query);

    if(!query.first())
        hideMe(this);

    m_id    = id;

    QSqlRecord      record  = query.record();

    m_username->setEnabled(false);

    m_curr.name     = record.value("username").toString();
    m_curr.number   = record.value("vpnumber").toString();
    m_curr.sdial    = record.value("sdial").toString();

    m_curr.firstName= record.value("firstName").toString();
    m_curr.lastName = record.value("lastName").toString();
    m_curr.country  = record.value("country").toString();
    m_curr.state    = record.value("state").toString();

    m_curr.homepage = record.value("homepage").toString();
    m_curr.email    = record.value("email").toString();
    m_curr.mobile   = record.value("mobile").toString();
    m_curr.office   = record.value("office").toString();

    m_username->setText(    m_curr.name);
    m_vpnumber->setText(    m_curr.number);

    m_firstName->setText(   m_curr.firstName);
    m_lastName->setText(    m_curr.lastName);
    m_country->setText(     m_curr.country);

    m_email->setText(       m_curr.email);
    m_mobile->setText(      m_curr.mobile);
    m_office->setText(      m_curr.office);

    m_vpnumber->setReadOnly(true);
    m_username->setReadOnly(true);

    updateUI();

    showMe(this);
}

//  AddContactForm::updateUI
//  -------------------------------------------------------------------------
void    AddContactForm::updateUI()
{
    m_sendEmail->setDisabled(       m_email->text().isEmpty());
    m_callMobile->setDisabled(      m_mobile->text().isEmpty());
    m_callOffice->setDisabled(      m_office->text().isEmpty());
}

//  AddContactForm::onAddContact
//  -------------------------------------------------------------------------
void    AddContactForm::onAddContact()
{
    if(!m_username->text().isEmpty())
    {
        saveContact();
    }

    m_id    = -1;

    m_curr.reset();

    m_username->setEnabled(true);

    m_username->setText("");
    m_vpnumber->setText("");

    m_firstName->setText("");
    m_lastName->setText("");
    m_country->setText("");

    m_email->setText("");
    m_mobile->setText("");
    m_office->setText("");

    m_vpnumber->setReadOnly(false);
    m_username->setReadOnly(false);
}

//  AddContactForm::hideEvent
//  -------------------------------------------------------------------------
void    AddContactForm::hideEvent(QHideEvent*         event)
{
    saveContact();
}

void    AddContactForm::saveContact()
{
    QSqlQuery   query;
    Contact     contact;

    if(m_id == -1)
    {
        if(m_username->text().isEmpty() && m_vpnumber->text().isEmpty())
        {
            return;
        }

        if(!m_username->text().isEmpty())
        {
            query.exec(QString("INSERT INTO contact(owner, username) SELECT id, '%1' FROM user WHERE username = '%2'")
                            .arg(m_username->text())
                            .arg(getUsername()));
            LOG_SQL_QUERY(query);
        }
        else
        {
            query.exec(QString("INSERT INTO contact(owner, vpnumber, username) SELECT id, '%1', '%1' FROM user WHERE username = '%2'")
                       .arg(m_vpnumber->text())
                       .arg(getUsername()));
            LOG_SQL_QUERY(query);
        }

        query.exec(QString("SELECT max(id) FROM contact"));
        LOG_SQL_QUERY(query);

        if(!query.next())
        {
            return;
        }
        m_id    = query.value(0).toInt();
    }

    if(m_username->text().isEmpty())
        m_username->setText(m_vpnumber->text());

    contact.name        = m_username->text();
    contact.number      = m_vpnumber->text();

    contact.firstName   = m_firstName->text();
    contact.lastName    = m_lastName->text();
    contact.country     = m_country->text();

    contact.email       = m_email->text();
    contact.mobile      = m_mobile->text();
    contact.office      = m_office->text();

    if(m_curr != contact)
    {
        QSqlDatabase::database().transaction();
        query.exec(QString("UPDATE contact SET username  = '%1' WHERE id = '%2'").arg(contact.name).arg(m_id));
        LOG_SQL_QUERY(query);

        query.exec(QString("UPDATE contact SET vpnumber  = '%1' WHERE id = '%2'").arg(contact.number).arg(m_id));
        LOG_SQL_QUERY(query);

        query.exec(QString("UPDATE contact SET sdial     = '%1' WHERE id = '%2'").arg(contact.sdial).arg(m_id));
        LOG_SQL_QUERY(query);

        query.exec(QString("UPDATE contact SET firstName = '%1' WHERE id = '%2'").arg(contact.firstName).arg(m_id));
        LOG_SQL_QUERY(query);

        query.exec(QString("UPDATE contact SET lastName  = '%1' WHERE id = '%2'").arg(contact.lastName).arg(m_id));
        LOG_SQL_QUERY(query);

        query.exec(QString("UPDATE contact SET country   = '%1' WHERE id = '%2'").arg(contact.country).arg(m_id));
        LOG_SQL_QUERY(query);

        query.exec(QString("UPDATE contact SET state     = '%1' WHERE id = '%2'").arg(contact.state).arg(m_id));
        LOG_SQL_QUERY(query);

        query.exec(QString("UPDATE contact SET sdial     = '%1' WHERE id = '%2'").arg(contact.sdial).arg(m_id));
        LOG_SQL_QUERY(query);

        query.exec(QString("UPDATE contact SET homepage  = '%1' WHERE id = '%2'").arg(contact.homepage).arg(m_id));
        LOG_SQL_QUERY(query);

        query.exec(QString("UPDATE contact SET email     = '%1' WHERE id = '%2'").arg(contact.email).arg(m_id));
        LOG_SQL_QUERY(query);

        query.exec(QString("UPDATE contact SET mobile    = '%1' WHERE id = '%2'").arg(contact.mobile).arg(m_id));
        LOG_SQL_QUERY(query);

        query.exec(QString("UPDATE contact SET office    = '%1' WHERE id = '%2'").arg(contact.office).arg(m_id));
        LOG_SQL_QUERY(query);

        QSqlDatabase::database().commit();

        txAbook();
    }
}

//  AddContactForm::onDelContact
//  -------------------------------------------------------------------------
void    AddContactForm::onDelContact()
{
    emit deleting();
}

//  AddContactForm::onVisitHomepage
//  -------------------------------------------------------------------------
void    AddContactForm::onVisitHomepage()
{
}

//  AddContactForm::onSendEmail
//  -------------------------------------------------------------------------
void    AddContactForm::onSendEmail()
{
    sendCommand(commandNavigate, QString(m_email->text()));
}

//  AddContactForm::onCallOffice
//  -------------------------------------------------------------------------
void    AddContactForm::onCallOffice()
{
    emit    call(m_office->text(), BC_VOICE);
}

//  AddContactForm::onCallMobile
//  -------------------------------------------------------------------------
void    AddContactForm::onCallMobile()
{
    emit    call(m_mobile->text(), BC_VOICE);
}

//  AddContactForm::doDelete
//  -------------------------------------------------------------------------
void    AddContactForm::doDelete()
{
    QSqlQuery   query;

    query.exec(QString("DELETE FROM contact WHERE id = '%1'")
                    .arg(m_id));
    LOG_SQL_QUERY(query);

    m_id    = -1;
    m_username->setText("");
    m_vpnumber->setText("");
}

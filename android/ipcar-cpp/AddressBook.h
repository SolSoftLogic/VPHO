#ifndef ADDRESSBOOK_H
#define ADDRESSBOOK_H

#include <QtCore/QString>

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
};

#endif // ADDRESSBOOK_H

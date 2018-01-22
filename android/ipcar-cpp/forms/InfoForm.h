#ifndef INFOPANE_H
#define INFOPANE_H

#include <QtGui/QFrame>
#include <QtGui/QWidget>
#include <QtGui/QGridLayout>
#include <QtGui/QPushButton>
#include "Commands.h"

class InfoForm
    : public QWidget
{
    Q_OBJECT
    public:
        explicit InfoForm(QWidget *parent = 0);

    signals:

    public slots:

    private:
        QPushButton*    m_settings;
        QPushButton*    m_communication;
        QPushButton*    m_internet;
        QPushButton*    m_entertainment;
        QPushButton*    m_info;
};

#endif // InfoForm_H

#ifndef PORTFORWARDFORM_H
#define PORTFORWARDFORM_H

#include <QtGui/QWidget>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QLineEdit>
#include "Utils.h"

class PortForwardForm
    : public QWidget
    , public VpMessageObserver
{
    Q_OBJECT

    public:
        explicit PortForwardForm(QWidget *parent = 0);

    signals:

    public slots:
    private slots:
        void            onCheckClicked();

    private:
        void            message(
                            VpMessageSubject*   subject,
                            unsigned            msg,
                            unsigned            pcall,
                            unsigned            param);
    private:
        QLineEdit*      m_port;
        QLabel*         m_message;
        QPushButton*    m_check;
};

#endif // PORTFORWARDFORM_H

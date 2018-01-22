#ifndef NETWORKPANE_H
#define NETWORKPANE_H

#include <QtGui/QFrame>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QComboBox>

class   NetworkForm
    : public QWidget
{
    Q_OBJECT
    public:
        explicit NetworkForm(QWidget *parent = 0);

    signals:

    private slots:
        void        onPortChanged();

    public slots:
    private:
        QLabel*     m_signal;
        QLabel*     m_operator;
        QPushButton*m_connect;
        QPushButton*m_disconnect;
        QComboBox*  m_aport;
        QComboBox*  m_cport;

};

#endif // NETWORKPANE_H

#ifndef DIALPAD_H
#define DIALPAD_H

#include "Utils.h"

#include <QtGui/QFrame>
#include <QtGui/QPushButton>
#include <QtGui/QGridLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>

class DialPad
    : public QWidget
    , public VpMessageObserver
{
    Q_OBJECT

    public:
        explicit        DialPad(QWidget*    parent = 0);

    signals:
        void            calling(int             call,
                                const QString&  address,
                                int             bearer);

    private slots:
        void            focusChanged(QWidget* old, QWidget * now);
        void            onButtonClick();
        void            message(VpMessageSubject*   subject,
                            unsigned            msg,
                            unsigned            pcall,
                            unsigned            param);
        void            onTextChanged(
                            const QString&      text);
    private:
        void            updateUI();
        void            hideEvent(QHideEvent *);

    private:
        QPushButton*    m_del;

        QPushButton*    m_key[10];
        QPushButton*    m_star;
        QPushButton*    m_grid;

        QPushButton*    m_send;

        QLineEdit*      m_number;

        QWidget*        m_focus;

        VPCALL          m_call;
};

#endif // DIALPAD_H

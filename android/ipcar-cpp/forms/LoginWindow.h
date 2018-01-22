#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QtGui/QFrame>
#include <QtGui/QCheckBox>

class   QLineEdit;
class   QPushButton;

class LoginWindow : public QWidget
{
    Q_OBJECT
    public:
        explicit LoginWindow(QWidget *parent = 0);

    signals:
        void        login(const QString& username,
                          const QString& password);

    public slots:
        void        textChanged();
        void        loginClicked();

    private:
        void        hideEvent ( QHideEvent * event ) ;

    private:
        QLineEdit*  m_username;
        QLineEdit*  m_password;
        QLineEdit*  m_pin;
        QCheckBox*  m_remember;

        QPushButton*m_login;
        QPushButton*m_register;
};

#endif // LOGINWINDOW_H

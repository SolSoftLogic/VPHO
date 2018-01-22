#ifndef CALLMANAGEMENTPANE_H
#define CALLMANAGEMENTPANE_H

#include <QtGui/QFrame>
#include <QtGui/QCheckBox>
#include <QtGui/QLineEdit>
#include <QtGui/QRadioButton>
#include <QtGui/QLabel>
#include <QtGui/QDoubleSpinBox>

class CallManagementForm
    : public QWidget
{
    Q_OBJECT
    public:
        explicit CallManagementForm(QWidget *parent = 0);

        void            showEvent(QShowEvent *);

    signals:

    private slots:
        void            onRadiobuttonToggle(bool    toggle);
        void            onCheckboxToggle(   bool    toggle);
        void            onRefuseTextChanged();
        void            onDeflectNumberChanged();
        void            onForwardToTextChanged();
        void            onWaitChanged(double);

    private:
        QCheckBox*      m_always;
        QCheckBox*      m_offline;
        QLineEdit*      m_forwardTo;

        QRadioButton*   m_ring;
        QRadioButton*   m_refuse;
        QLineEdit*      m_refuseText;
        QRadioButton*   m_deflect;
        QLineEdit*      m_deflectText;
        QRadioButton*   m_accept;
        QRadioButton*   m_answer;
        QLineEdit*      m_seconds;

        QDoubleSpinBox* m_wait;

        int             m_forwardOp;
};

#endif // CALLMANAGEMENTPANE_H

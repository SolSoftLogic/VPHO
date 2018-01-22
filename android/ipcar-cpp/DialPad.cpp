#include "DialPad.h"
#include "Utils.h"
#include <QtGui/QApplication>

#include "Utils.h"

DialPad::DialPad(QWidget*    parent)
    : QWidget(parent)
    , m_call((VPCALL)-1)
{
    QWidget*    widget  = loadForm("qss/forms/dial.ui");

    QVBoxLayout*layout  = new QVBoxLayout;

    layout->addWidget(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    this->setLayout(layout);

    connect(qApp, SIGNAL(focusChanged(QWidget*,QWidget*)), this, SLOT(focusChanged(QWidget*,QWidget*)));

    assign(this, m_number,  "dial_number");
    assign(this, m_del,     "dial_del");    connect(m_del,      SIGNAL(clicked()),  SLOT(onButtonClick()));

    assign(this, m_key[0],  "dial_key0");   connect(m_key[0],   SIGNAL(clicked()),  SLOT(onButtonClick()));
    assign(this, m_key[1],  "dial_key1");   connect(m_key[1],   SIGNAL(clicked()),  SLOT(onButtonClick()));
    assign(this, m_key[2],  "dial_key2");   connect(m_key[2],   SIGNAL(clicked()),  SLOT(onButtonClick()));
    assign(this, m_key[3],  "dial_key3");   connect(m_key[3],   SIGNAL(clicked()),  SLOT(onButtonClick()));
    assign(this, m_key[4],  "dial_key4");   connect(m_key[4],   SIGNAL(clicked()),  SLOT(onButtonClick()));
    assign(this, m_key[5],  "dial_key5");   connect(m_key[5],   SIGNAL(clicked()),  SLOT(onButtonClick()));
    assign(this, m_key[6],  "dial_key6");   connect(m_key[6],   SIGNAL(clicked()),  SLOT(onButtonClick()));
    assign(this, m_key[7],  "dial_key7");   connect(m_key[7],   SIGNAL(clicked()),  SLOT(onButtonClick()));
    assign(this, m_key[8],  "dial_key8");   connect(m_key[8],   SIGNAL(clicked()),  SLOT(onButtonClick()));
    assign(this, m_key[9],  "dial_key9");   connect(m_key[9],   SIGNAL(clicked()),  SLOT(onButtonClick()));

    assign(this, m_star,    "dial_star");   connect(m_star,     SIGNAL(clicked()),  SLOT(onButtonClick()));
    assign(this, m_grid,    "dial_grid");   connect(m_grid,     SIGNAL(clicked()),  SLOT(onButtonClick()));
    assign(this, m_send,    "dial_send");   connect(m_send,     SIGNAL(clicked()),  SLOT(onButtonClick()));

    connect(m_number, SIGNAL(textChanged(QString)), this, SLOT(onTextChanged(QString)));

    resize(widget->size());

    vpMessageSubject()->addObserver(this);

    updateUI();
}

//  DialPad::focusChanged
//  -------------------------------------------------------------------------
void    DialPad::focusChanged( QWidget * old, QWidget * now )
{
    m_focus = now;
}

//  DialPad::onButtonClick
//  -------------------------------------------------------------------------
void    DialPad::onButtonClick()
{
    QObject*    object  = sender();
    QString     number  = m_number->text();

    if(object == m_del)
    {
        if(number.length() == 1 && number[0] == '+')
        {
            number  = "0";
        }
        else if(number.length())
        {
            number.resize(number.length() - 1);
        }
    }

    else if(object == m_key[0])
    {
        if(number.length() == 1 && number[0] == '0')
        {
            number  = "+";
        }
        else
        {
            number.append("0");
        }
    }
    else if(object == m_key[1])     number.append("1");
    else if(object == m_key[2])     number.append("2");
    else if(object == m_key[3])     number.append("3");
    else if(object == m_key[4])     number.append("4");
    else if(object == m_key[5])     number.append("5");
    else if(object == m_key[6])     number.append("6");
    else if(object == m_key[7])     number.append("7");
    else if(object == m_key[8])     number.append("8");
    else if(object == m_key[9])     number.append("9");
    else if(object == m_star)       number.append("*");
    else if(object == m_grid)       number.append("#");

    m_number->setText(number);

    if(object == m_send)
    {
        if(m_call == (VPCALL)-1)
        {
            m_call  = vpstack()->CreateCall();

            emit calling((int)m_call, qPrintable(number), BC_VOICE);
        }

        hide();
    }

    updateUI();
}

//  DialPad::onTextChanged
//  -------------------------------------------------------------------------
void    DialPad::onTextChanged(
                    const QString&      text)
{
    updateUI();
}

//  DialPad::message
//  -------------------------------------------------------------------------
void    DialPad::message(
                    VpMessageSubject*   subject,
                    unsigned            msg,
                    unsigned            pcall,
                    unsigned            param)
{
    if(msg == VPMSG_CALLENDED && pcall == (int)m_call)
    {
        m_call  = (VPCALL)-1;

        updateUI();
    }
}

//  DialPad::hideEvent
//  -------------------------------------------------------------------------
void    DialPad::hideEvent(QHideEvent *)
{
    m_number->setText("");
}

//  DialPad::updateUI
//  -------------------------------------------------------------------------
void    DialPad::updateUI()
{
    m_send->setEnabled(!m_number->text().isEmpty() && (int)m_call == -1);
}


#include "Keyboard.h"
#include <QtXml/QXmlSimpleReader>
#include <QtXml/QXmlInputSource>
#include <QtGui/QPainter>
#include <QtGui/QMoveEvent>
#include <QtGui/QApplication>
#include <QtGui/QKeyEvent>
#include "Utils.h"



//  Keyboard::Keyboard
//  -------------------------------------------------------------------------
Keyboard::Keyboard(QWidget *parent)
    : QWidget(parent)
    , m_inKeyboard(false)
    , m_pressed(-1)
    , m_over(-1)
    , m_norm(getFilename("qss/keyboard/norm.png"))
    , m_normHover(getFilename("qss/keyboard/norm-hover.png"))
    , m_caps(getFilename("qss/keyboard/caps.png"))
    , m_capsHover(getFilename("qss/keyboard/caps-hover.png"))
    , m_symb(getFilename("qss/keyboard/symb.png"))
    , m_symbHover(getFilename("qss/keyboard/symb-hover.png"))
    , m_shift(false)
    , m_abc(true)
    , m_click(getFilename("sounds/click.wav"))

{
    connect(qApp, SIGNAL(focusChanged(QWidget*,QWidget*)), this, SLOT(focusChanged(QWidget*,QWidget*)));

//    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    this->setFocusPolicy(Qt::NoFocus);

    QFile               file(getFilename("qss/keyboard/layout.xml"));
    QXmlInputSource     stream(&file);
    QXmlSimpleReader    reader;

    reader.setContentHandler(this);
    reader.setErrorHandler(this);
    reader.parse(stream);

    QRect   rect;
    rect.setWidth(m_norm.width());
    rect.setHeight(m_norm.height());
    setGeometry(rect);
    setMinimumSize(rect.width(), rect.height());

    setMouseTracking(true);
}

//  Keyboard::findButton
//  -------------------------------------------------------------------------
int     Keyboard::findButton( const QPoint&       point)
{
    for(int i = 0; i < m_buttons.size(); ++i)
    {
        VButton button  = m_buttons[i];

        if(button.rect.contains(point))
        {
            return  i;
        }
    }

    return  -1;
}

//  Keyboard::mousePressEvent
//  -------------------------------------------------------------------------
void    Keyboard::mousePressEvent(    QMouseEvent*    event)
{
    m_pressed   = findButton(event->pos());

    if(m_pressed != -1)
    {
        m_click.play();
        repaint(m_buttons[m_pressed].rect);
    }
}

//  Keyboard::moveEvent
//  -------------------------------------------------------------------------
void    Keyboard::mouseMoveEvent(   QMouseEvent*     event)
{
    int current = findButton(event->pos());

    if(current != m_over)
    {
        if(m_over != -1)
        {
            int temp    = m_over;

            m_over  = -1;
            repaint(m_buttons[temp].rect);
        }

        if(current != -1)
        {
            m_over  = current;
            repaint(m_buttons[m_over].rect);
        }
    }
}

//  Keyboard::mouseReleaseEvent
//  -------------------------------------------------------------------------
void    Keyboard::mouseReleaseEvent(  QMouseEvent*    event)
{
    int temp    = findButton(event->pos());

    //  this is the same button
    if(temp == m_pressed && temp != -1)
    {
        VButton button  = m_buttons[temp];

        if(button.handler)
        {
            (this->*(button.handler))(&button, true);
        }
    }

    //  repaint
    temp        = m_pressed;
    m_pressed   = -1;

    if(temp != -1)
    {
        repaint(m_buttons[temp].rect);
    }
}

//  Keyboard::startElement
//  -------------------------------------------------------------------------
bool    Keyboard::startElement(
                            const QString&              namespaceURI,
                            const QString&              localName,
                            const QString&              qName,
                            const QXmlAttributes&       attributes)
{
    if (!m_inKeyboard && qName != "keyboard")
    {
        m_errorStr = QObject::tr("The file is not an keyboard.xml file.");
        return false;
    }

    if (qName == "keyboard")
    {
        m_inKeyboard = true;
    }
    else
    {
        VButton button;

        button.rect.setX(attributes.value("x").toInt());
        button.rect.setY(attributes.value("y").toInt());
        button.rect.setWidth(attributes.value("cx").toInt());
        button.rect.setHeight(attributes.value("cy").toInt());

        if (qName == "key")
        {
            button.vkey[0]      = qPrintable(attributes.value("norm"))[0];
            button.vkey[1]      = qPrintable(attributes.value("caps"))[0];
            button.vkey[2]      = qPrintable(attributes.value("symb"))[0];

            button.handler      = &Keyboard::OnPressKey;

            m_buttons.push_back(button);

        }
        else if (qName == "backspace")
        {
            button.handler      = &Keyboard::OnPressBackspace;

            m_buttons.push_back(button);
        }
        else if (qName == "shift")
        {
            button.handler      = &Keyboard::OnPressShift;
            m_buttons.push_back(button);
        }
        else if (qName == "abc123")
        {
            button.handler      = &Keyboard::OnPressAbc;

            m_buttons.push_back(button);
        }
        else if (qName == "send")
        {
            button.handler      = &Keyboard::OnPressSend;

            m_buttons.push_back(button);
        }
        else if (qName == "enter")
        {
            button.handler      = &Keyboard::OnPressEnter;

            m_buttons.push_back(button);
        }
    }

    return true;
}

//  Keyboard::OnPressBackspace
//  -------------------------------------------------------------------------
void        Keyboard::OnPressBackspace(
                        VButton*            button,
                        bool                pressed)
{
    QKeyEvent               press(   QEvent::KeyPress,    Qt::Key_Backspace, Qt::NoModifier, "08");
    QKeyEvent               release( QEvent::KeyRelease,  Qt::Key_Backspace, Qt::NoModifier, "08");

    qApp->sendEvent(m_focus, &press);
    qApp->sendEvent(m_focus, &release);
}

//  Keyboard::OnPressShift
//  -------------------------------------------------------------------------
void        Keyboard::OnPressShift(
                        VButton*            button,
                        bool                pressed)
{
    m_shift = !m_shift;

    repaint();
}

//  Keyboard::OnPressAbc
//  -------------------------------------------------------------------------
void        Keyboard::OnPressAbc(
                        VButton*            button,
                        bool                pressed)
{
    m_abc   = !m_abc;

    repaint();
}

//  Keyboard::OnPressKey
//  -------------------------------------------------------------------------
void        Keyboard::OnPressKey(
                        VButton*            button,
                        bool                pressed)
{
    Qt::KeyboardModifiers   modifier    = (m_abc || ! m_shift) ? Qt::NoModifier : Qt::ShiftModifier;
    char                    key         = m_abc ? (m_shift ? button->vkey[1]
                                                           : button->vkey[0])
                                                : button->vkey[2];
    char                    text[2]     = {key, 0};

    QKeyEvent               press(   QEvent::KeyPress,   (Qt::Key)key, modifier, text);
    QKeyEvent               release( QEvent::KeyRelease, (Qt::Key)key, modifier, text);

    qApp->sendEvent(m_focus, &press);
    qApp->sendEvent(m_focus, &release);
}

//  Keyboard::OnPressSend
//  -------------------------------------------------------------------------
void        Keyboard::OnPressSend(
                        VButton*            button,
                        bool                pressed)
{
    emit sendClicked();
}

//  Keyboard::OnPressEnter
//  -------------------------------------------------------------------------
void        Keyboard::OnPressEnter(
                        VButton*            button,
                        bool                pressed)
{
    Qt::KeyboardModifiers   modifier    = (m_abc || ! m_shift) ? Qt::NoModifier : Qt::ShiftModifier;
    QKeyEvent               press(   QEvent::KeyPress,   Qt::Key_Enter, modifier, "\n");
    QKeyEvent               release( QEvent::KeyRelease, Qt::Key_Enter, modifier, "\n");

    qApp->sendEvent(m_focus, &press);
    qApp->sendEvent(m_focus, &release);
}

//  Keyboard::paintEvent
//  -------------------------------------------------------------------------
void        Keyboard::paintEvent(
                        QPaintEvent*        event)
{
    QPainter    painter(this);
    QImage*     image   = !m_abc ? &m_symb
                                 : (m_shift ? &m_caps
                                            : &m_norm);

    painter.drawImage(0, 0, *image);

    if(m_over != -1 && m_pressed != m_over)
    {
        VButton     button  = m_buttons[m_over];
        QImage*     image   = !m_abc ? &m_symbHover
                                     : (m_shift ? &m_capsHover
                                                : &m_normHover);

        painter.drawImage(button.rect, *image, button.rect);
    }

    if(m_pressed != -1 && m_pressed != m_over)
    {
        VButton     button  = m_buttons[m_pressed];
        QImage*     image   = !m_abc ? &m_symbHover
                                     : (m_shift ? &m_capsHover
                                                : &m_normHover);

        painter.drawImage(button.rect, *image, button.rect);
    }
}

//  DialPad::focusChanged
//  -------------------------------------------------------------------------
void    Keyboard::focusChanged( QWidget * old, QWidget * now )
{
    m_focus = now;
}

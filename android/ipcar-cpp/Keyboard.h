#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <QtGui/QFrame>
#include <QtGui/QImage>
#include <QtXml/QXmlDefaultHandler>
#include <QtCore/QVector>
#ifndef   ANDROID_VERSION
#include <QtGui/QSound>
#else
#include "QSound.h"
#endif

class Keyboard
    : public QWidget
    , public QXmlDefaultHandler
{
    Q_OBJECT

    public:
        explicit Keyboard(QWidget *parent = 0);

    protected:
        void    mousePressEvent(    QMouseEvent*    event);
        void    mouseMoveEvent(     QMouseEvent*    event);
        void    mouseReleaseEvent(  QMouseEvent*    event);
        void    paintEvent(         QPaintEvent*    event);

    protected:
        bool    startElement(       const QString&              namespaceURI,
                                    const QString&              localName,
                                    const QString&              qName,
                                    const QXmlAttributes&       attributes);
        bool    fatalError(         const QXmlParseException&   exception)
        {
            return false;
        }

        QString errorString() const
        {
            return m_errorStr;
        }

        QString     m_errorStr;
        bool        m_inKeyboard;

    signals:
        void        sendClicked();

    public slots:
        void        focusChanged( QWidget * old, QWidget * now );

    private:
        struct  VButton
        {
            QRect       rect;
            char        vkey[3];

            void        (Keyboard::*handler)(
                                            VButton*        button,
                                            bool            pressed);   //  pressed or released
        };

        QVector<VButton>    m_buttons;
        int                 m_pressed;
        int                 m_over;

        int         findButton( const QPoint&       point);

        void        OnPressBackspace(
                                VButton*            button,
                                bool                pressed);
        void        OnPressShift(
                                VButton*            button,
                                bool                pressed);
        void        OnPressAbc( VButton*            button,
                                bool                pressed);
        void        OnPressKey( VButton*            button,
                                bool                pressed);
        void        OnPressSend(VButton*            button,
                                bool                pressed);
        void        OnPressEnter(
                                VButton*            button,
                                bool                pressed);

        QImage      m_norm;
        QImage      m_normHover;

        QImage      m_caps;
        QImage      m_capsHover;

        QImage      m_symb;
        QImage      m_symbHover;

        bool        m_shift;
        bool        m_abc;

        QWidget*    m_focus;

        QSound      m_click;
};

#endif // KEYBOARD_H

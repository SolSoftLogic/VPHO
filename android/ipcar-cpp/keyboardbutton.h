#ifndef KEYBOARDBUTTON_H
#define KEYBOARDBUTTON_H

#include <QtCore/QObject>
#include <QtCore/QRect>

class KeyboardButton : public QObject
{
Q_OBJECT
public:
    explicit KeyboardButton(QObject*    parent,
                            QRect       rect);

signals:

public slots:

private:
    QRect       m_rect;

};

#endif // KEYBOARDBUTTON_H

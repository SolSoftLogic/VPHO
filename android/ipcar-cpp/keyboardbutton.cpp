#include "keyboardbutton.h"

//  KeyboardButton::KeyboardButton
//  -------------------------------------------------------------------------
KeyboardButton::KeyboardButton(QObject *parent, QRect       rect)
    : QObject(parent)
{
    m_rect  = rect;
}

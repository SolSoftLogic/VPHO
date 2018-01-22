#include "BlinkingPushButton.h"

BlinkingPushButton::BlinkingPushButton(QWidget *parent)
    : QPushButton(parent)
{
    m_interval  = 200;
}

//  BlinkingPushButton::showEvent
//  -------------------------------------------------------------------------
void    BlinkingPushButton::showEvent( QShowEvent * event)
{
    m_timer.stop();
}

//  BlinkingPushButton::hideEvent
//  -------------------------------------------------------------------------
void    BlinkingPushButton::hideEvent( QHideEvent * event)
{
    m_timer.start(m_interval);
}

//  BlinkingPushButton::onTimerTick
//  -------------------------------------------------------------------------
void    BlinkingPushButton::onTimerTick()
{
//    this->backgroundRole()
}

#include <QtGui/QPushButton>

#ifndef COMMANDS_H
#define COMMANDS_H

typedef unsigned long   command_t;
/*
static const command_t  commandBack             =  0;
static const command_t  commandCallHistory      =  2;
static const command_t  commandGmail            =  5;
static const command_t  commandFacebook         =  6;
static const command_t  commandTwitter          =  7;
static const command_t  commandGpsMenu          =  8;
static const command_t  commandKeyboard         = 11;
static const command_t  commandDialPad          = 12;
static const command_t  commandCommunication    = 13;
*/
static const command_t  commandContactList      =  1;
static const command_t  commandHideDialpad      =  2;
static const command_t  commandSlides           =  3;
static const command_t  commandSettings         =  9;
static const command_t  commandHome             =  4;
static const command_t  commandVc               = 10;
static const command_t  commandInternet         = 14;
static const command_t  commandEntertainment    = 15;
static const command_t  commandAudio            = 16;
static const command_t  commandCallManagement   = 17;
static const command_t  commandNetwork          = 18;
static const command_t  commandVideo            = 19;
static const command_t  commandVoiceCommand     = 20;
static const command_t  commandVideoCaptured    = 21;
static const command_t  commandSelectFile       = 22;
static const command_t  commandNavigate         = 23;
static const command_t  commandAccept           = 24;

void    connectButton(  QPushButton*    button,
                        command_t       command);
void    sendCommand(    command_t       command,
                        const QString&  param = "");
void    showMe(         QWidget*        widget);
void    hideMe(         QWidget*        widget);
void    showMessage(    QWidget*        parent,
                        const QString&  message);
int     call(           const QString&  address,
                        int             bearer);

template<int command>
class   CommandButton
{
    public:
        CommandButton()
        {
            connectButton(&m_button, command);
        }

        CommandButton(QWidget* parent)
            : m_button(parent)
        {
            connectButton(&m_button, command);
        }

        operator QPushButton&()
        {
            return  m_button;
        }

        QPushButton*    operator&()
        {
            return  &m_button;
        }

        QPushButton*    operator->()
        {
            return  &m_button;
        }



    private:
        QPushButton m_button;
};

#endif // COMMANDS_H

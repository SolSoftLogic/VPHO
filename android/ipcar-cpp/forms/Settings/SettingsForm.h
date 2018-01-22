#ifndef SETTINGSPANE_H
#define SETTINGSPANE_H

#include <QtGui/QFrame>
#include <QtGui/QPushButton>
#include <QtGui/QGridLayout>
#include "Commands.h"

class SettingsForm
    : public QWidget
{
    Q_OBJECT
    public:
        explicit SettingsForm(QWidget *parent = 0);

    signals:

    public slots:
    private:
        QPushButton*    m_audio;
        QPushButton*    m_callManage;
        QPushButton*    m_network;
        QPushButton*    m_video;
        QPushButton*    m_voiceCommand;
};

#endif // SETTINGSPANE_H

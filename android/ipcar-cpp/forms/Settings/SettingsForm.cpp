#include "SettingsForm.h"
#include "Commands.h"
#include "Utils.h"

SettingsForm::SettingsForm(QWidget *parent)
    : QWidget(parent)
{
    QWidget*    widget  = loadForm("qss/forms/settings.ui");

    QVBoxLayout*layout  = new QVBoxLayout;

    layout->addWidget(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    this->setLayout(layout);

    assign(this, m_audio,       "settings_audio");   connectButton(m_audio,  commandAudio);
    assign(this, m_callManage,  "settings_call");    connectButton(m_callManage, commandCallManagement);
    assign(this, m_network,     "settings_network"); connectButton(m_network, commandNetwork);
    assign(this, m_video,       "settings_video");   connectButton(m_video, commandVideo);
    assign(this, m_voiceCommand,"settings_vc");      connectButton(m_voiceCommand, commandVc);
}

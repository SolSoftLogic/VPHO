#include "InfoForm.h"
#include "Commands.h"
#include "Utils.h"

#include <QtCore/QFile>

InfoForm::InfoForm(QWidget *parent)
    : QWidget(parent)
{
    QWidget*    widget  = loadForm("qss/forms/info.ui");

    QVBoxLayout*layout  = new QVBoxLayout;

    layout->addWidget(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    this->setLayout(layout);

    assign(this, m_settings,        "info_settings");       connectButton(m_settings,       commandSettings);
    assign(this, m_communication,   "info_communication");  connectButton(m_communication,  commandContactList);
    assign(this, m_internet,        "info_internet");       connectButton(m_internet,       commandHome);
    assign(this, m_entertainment,   "info_entertainment");  connectButton(m_entertainment,  commandEntertainment);
    assign(this, m_info,            "info_info");           connectButton(m_info,           commandSlides);
}

#include "NetworkForm.h"
#include "Utils.h"
#include <QtGui/QVBoxLayout>
#include <QtCore/QList>
#include <QtCore/QSettings>

#ifndef ANDROID_VERSION
#include "qextserialport.h"
#include "qextserialenumerator.h"
#endif

NetworkForm::NetworkForm(QWidget *parent)
    : QWidget(parent)
{
    QWidget*    widget  = loadForm("qss/forms/settings/network.ui");

    QVBoxLayout*layout  = new QVBoxLayout;

    layout->addWidget(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    this->setLayout(layout);

    assign(this, m_signal,      "network_signal");
    assign(this, m_operator,    "network_operator");
    assign(this, m_connect,     "network_connect");
    assign(this, m_disconnect,  "network_disconnect");
    assign(this, m_cport,       "network_cport");
    assign(this, m_aport,       "network_aport");

#ifndef ANDROID_VERSION

    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();

#ifdef ANDROID_VERSION
    QSettings settings("/sdcard/"PROJECTNAME"/"PROJECTNAME".ini",QSettings::IniFormat);
#else
    QSettings   settings;
#endif
    QString         aport   = settings.value("network/aport").toString();
    QString         cport   = settings.value("network/cport").toString();

    foreach(QextPortInfo port, ports)
    {
        QString     item;
            
        item    = QString("%1 - %2")
                    .arg(qPrintable(port.portName), 6, QChar(' '))
                    .arg(qPrintable(port.friendName));
        m_cport->addItem(item);
        m_aport->addItem(item);

        if(aport == qPrintable(port.portName))
        {
            m_aport->setCurrentIndex(m_aport->count() - 1);
        }

        if(cport == qPrintable(port.portName))
        {
            m_cport->setCurrentIndex(m_aport->count() - 1);
        }

    }
#endif
    connect(m_cport,    SIGNAL(currentIndexChanged(int)), this, SLOT(onPortChanged()));
    connect(m_aport,    SIGNAL(currentIndexChanged(int)), this, SLOT(onPortChanged()));
}

//  NetworkForm::onPortChanged
//  -------------------------------------------------------------------------
void        NetworkForm::onPortChanged()
{
#ifndef ANDROID_VERSION
    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();

#ifdef ANDROID_VERSION
    QSettings settings("/sdcard/"PROJECTNAME"/"PROJECTNAME".ini",QSettings::IniFormat);
#else
    QSettings   settings;
#endif
    settings.setValue("network/aport", qPrintable(ports.at(m_aport->currentIndex()).portName));
    settings.setValue("network/cport", qPrintable(ports.at(m_cport->currentIndex()).portName));
#endif
}

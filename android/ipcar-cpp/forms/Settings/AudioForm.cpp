#include "AudioForm.h"
#include "WidgetFactory.h"
#include "Utils.h"
#ifndef ANDROID_VERSIO
#include <QtMultimedia/QAudioDeviceInfo>
#endif
#include <QtCore/QSettings>

//  -------------------------------------------------------------------------
AudioForm::AudioForm(QWidget *parent)
    : QWidget(parent)
{
    QWidget*    widget  = loadForm("qss/forms/settings/audio.ui");

    QVBoxLayout*layout  = new QVBoxLayout;

    layout->addWidget(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    this->setLayout(layout);

    assign(this, m_oDeviceName,     "audio_oDevice");
    assign(this, m_oDeviceVolume,   "audio_oLevel");
    m_oDeviceVolume->setRange(0, 1000);
    connect(m_oDeviceVolume,     SIGNAL(valueChanged(int)),  this,   SLOT(onLevelChanged(int)));

    assign(this, m_iDeviceName,     "audio_iDevice");
    assign(this, m_iDeviceLine,     "audio_iLine");
    assign(this, m_iDeviceVolume,   "audio_iLevel");

    assign(this, m_minIVolume,      "audio_minIVolume");
    assign(this, m_maxIVolume,      "audio_maxIVolume");
    assign(this, m_minOVolume,      "audio_minOVolume");
    assign(this, m_maxOVolume,      "audio_maxOVolume");

    m_iDeviceVolume->setRange(0, 1000);
    connect(m_iDeviceVolume,   SIGNAL(valueChanged(int)),  this,   SLOT(onLevelChanged(int)));

    connect(m_minIVolume,   SIGNAL(clicked()),  this,   SLOT(onMinMaxVolumeClicked()));
    connect(m_maxIVolume,   SIGNAL(clicked()),  this,   SLOT(onMinMaxVolumeClicked()));
    connect(m_minOVolume,   SIGNAL(clicked()),  this,   SLOT(onMinMaxVolumeClicked()));
    connect(m_maxOVolume,   SIGNAL(clicked()),  this,   SLOT(onMinMaxVolumeClicked()));

#ifdef ANDROID_VERSION
    QSettings settings("/sdcard/"PROJECTNAME"/"PROJECTNAME".ini",QSettings::IniFormat);
#else
    QSettings   settings;
#endif
    QString     oDevice = settings.value("audio/oDevice").toString();
    QString     iDevice = settings.value("audio/iDevice").toString();
    QString     iLine   = settings.value("audio/iLine").toString();
    int         index   = false;

    if(!settings.value("audio/oLevel").isValid())
    {
        settings.setValue("audio/oLevel", QVariant(500));
    }

    if(!settings.value("audio/iLevel").isValid())
    {
        settings.setValue("audio/iLevel", QVariant(500));
    }
#ifndef ANDROID_VERSION
    //  oDevice
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioOutput))
    {
        m_oDeviceName->addItem(deviceInfo.deviceName(), qVariantFromValue(deviceInfo));
    }
#endif
    connect(m_oDeviceName, SIGNAL(currentIndexChanged(int)), SLOT(onDeviceChanged(int)));
    index   = m_oDeviceName->findText(oDevice);

    if(index != -1)
        m_oDeviceName->setCurrentIndex(index);

#ifndef ANDROID_VERSION
    //  iDevice
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
    {
        m_iDeviceName->addItem(deviceInfo.deviceName(), qVariantFromValue(deviceInfo));
    }
#endif
    connect(m_iDeviceName, SIGNAL(currentIndexChanged(int)), SLOT(onDeviceChanged(int)));
    index   = m_iDeviceName->findText(iDevice);

    if(index != -1)
        m_iDeviceName->setCurrentIndex(index);

    //  iLine
    connect(m_iDeviceLine,  SIGNAL(currentIndexChanged(int)),   this,   SLOT(onILineChanged(int)));

    updateUI();

    index   = m_iDeviceLine->findText(iLine);

    if(index != -1)
        m_iDeviceLine->setCurrentIndex(index);
}

//  AudioForm::updateUI
//  -------------------------------------------------------------------------
void    AudioForm::updateUI()
{
#ifdef ANDROID_VERSION
    QSettings settings("/sdcard/"PROJECTNAME"/"PROJECTNAME".ini",QSettings::IniFormat);
#else
    QSettings   settings;
#endif

    unsigned    oLevel  = settings.value("audio/oLevel").toUInt();
    unsigned    iLevel  = settings.value("audio/iLevel").toUInt();

    m_oDeviceVolume->setValue(oLevel);
#if SIMPLEAUDIO
#else
    VPMIXINGAUDIO_SetLevel(0, oLevel);
#endif
    m_iDeviceVolume->setValue(iLevel);
#if SIMPLEAUDIO
#else
    VPMIXINGAUDIO_SetLevel(1, iLevel);
#endif
    m_iDeviceLine->clear();
    char    cdev[10][100];
    int     n   = sizeof(cdev) / sizeof(cdev[0]);
#if SIMPLEAUDIO
    VPSIMPLEAUDIO_EnumerateDevices(cdev, &n, 2);
#else
    VPMIXINGAUDIO_EnumerateDevices(cdev, &n, 2);
#endif
    for(int i = 0; i < n; ++i)
    {
        m_iDeviceLine->addItem(cdev[i]);
    }
}

//  AudioForm::onLevelChanged
//  -------------------------------------------------------------------------
void    AudioForm::onLevelChanged(int)
{
#ifdef ANDROID_VERSION
    QSettings settings("/sdcard/"PROJECTNAME"/"PROJECTNAME".ini",QSettings::IniFormat);
#else
    QSettings   settings;
#endif

    settings.setValue("audio/oLevel", QVariant(m_oDeviceVolume->value()));
    settings.setValue("audio/iLevel", QVariant(m_iDeviceVolume->value()));
}

//  AudioForm::onDeviceChanged
//  -------------------------------------------------------------------------
void    AudioForm::onDeviceChanged(int)
{
#ifdef ANDROID_VERSION
    QSettings settings("/sdcard/"PROJECTNAME"/"PROJECTNAME".ini",QSettings::IniFormat);
#else
    QSettings   settings;
#endif

    QString     oDevice = m_oDeviceName->currentText();
    QString     iDevice = m_iDeviceName->currentText();

    settings.setValue("audio/oDevice",  QVariant(oDevice));
    settings.setValue("audio/iDevice",  QVariant(iDevice));
#if SIMPLEAUDIO
#else
    VPMIXINGAUDIO_SetAudioDevice(0, qPrintable(oDevice));
    VPMIXINGAUDIO_SetAudioDevice(1, qPrintable(iDevice));
#endif
    updateUI();
}

//  AudioForm::onILineChanged
//  -------------------------------------------------------------------------
void    AudioForm::onILineChanged(int)
{
#ifdef ANDROID_VERSION
    QSettings settings("/sdcard/"PROJECTNAME"/"PROJECTNAME".ini",QSettings::IniFormat);
#else
    QSettings   settings;
#endif

    settings.setValue("audio/iLine",    QVariant(m_iDeviceLine->currentText()));
}

//  AudioForm::onMinMaxVolumeClicked
//  -------------------------------------------------------------------------
void    AudioForm::onMinMaxVolumeClicked()
{
    if(sender() == m_minIVolume)
    {
        m_iDeviceVolume->setValue(m_iDeviceVolume->value() - 0.1 * (m_iDeviceVolume->maximum() - m_iDeviceVolume->minimum()));
    }

    if(sender() == m_maxIVolume)
    {
        m_iDeviceVolume->setValue(m_iDeviceVolume->value() + 0.1 * (m_iDeviceVolume->maximum() - m_iDeviceVolume->minimum()));
    }

    if(sender() == m_minOVolume)
    {
        m_oDeviceVolume->setValue(m_oDeviceVolume->value() - 0.1 * (m_oDeviceVolume->maximum() - m_oDeviceVolume->minimum()));
    }

    if(sender() == m_maxOVolume)
    {
        m_oDeviceVolume->setValue(m_oDeviceVolume->value() + 0.1 * (m_oDeviceVolume->maximum() - m_oDeviceVolume->minimum()));
    }
}

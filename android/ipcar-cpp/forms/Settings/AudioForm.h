#ifndef AUDIOPANE_H
#define AUDIOPANE_H

#include <QtGui/QFrame>
#include <QtGui/QLabel>
#include <QtGui/QGridLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QComboBox>
#include <QtGui/QSlider>
#include <QtGui/QPushButton>

class AudioForm
    : public QWidget
{
    Q_OBJECT
    public:
        explicit AudioForm(QWidget *parent = 0);

    signals:

    public slots:

    private slots:
        void        onLevelChanged(int);
        void        onDeviceChanged(int);
        void        onILineChanged(int);

        void        onMinMaxVolumeClicked();

    private:
        void        updateUI();

    private:
        QComboBox*  m_oDeviceName;
        QSlider*    m_oDeviceVolume;

        QComboBox*  m_iDeviceName;
        QComboBox*  m_iDeviceLine;
        QSlider*    m_iDeviceVolume;

        QPushButton*m_minIVolume;
        QPushButton*m_maxIVolume;

        QPushButton*m_minOVolume;
        QPushButton*m_maxOVolume;
};

#endif // AUDIOPANE_H

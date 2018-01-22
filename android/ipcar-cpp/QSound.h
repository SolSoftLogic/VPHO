
#ifndef QSOUND_H
#define QSOUND_H

//#include <QtCore/qobject.h>
#include <QObject>

class QSoundPrivate;

class  QSound : public QObject
{
    Q_OBJECT

public:
    static bool isAvailable();
    static void play(const QString& filename);

    explicit QSound(const QString& filename, QObject* parent = 0);
    ~QSound();

    int loops() const;
    int loopsRemaining() const;
    void setLoops(int);
    QString fileName() const;

    bool isFinished() const;

public Q_SLOTS:
    void play();
    void stop();
private:
    Q_DECLARE_PRIVATE(QSound)

};


//QT_END_NAMESPACE

//QT_END_HEADER

#endif // QSOUND_H

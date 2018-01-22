#include "QSound.h"

class QSoundPrivate : public QObject
{
public:
    QSoundPrivate(const QString& fname)
        : filename(fname), looprem(0), looptotal(1)
    {
    }

    ~QSoundPrivate()
    {
    }

    QString filename;

    int looprem;
    int looptotal;
};


void QSound::play(const QString& filename)
{
}

/*!
    Constructs a QSound object from the file specified by the given \a
    filename and with the given \a parent.

    This may use more memory than the static play() function, but it
    may also play more immediately (depending on the underlying
    platform audio facilities).

    \sa play()
*/

QSound::QSound(const QString& filename, QObject* parent)
{
}

/*!
    Destroys this sound object. If the sound is not finished playing,
    the stop() function is called before the sound object is
    destructed.

    \sa stop(), isFinished()
*/
QSound::~QSound()
{
}

/*!
    Returns true if the sound has finished playing; otherwise returns false.

    \warning On Windows this function always returns true for unlooped sounds.
*/
bool QSound::isFinished() const
{
}

/*!
    \overload

    Starts playing the sound specified by this QSound object.

    The function returns immediately.  Depending on the platform audio
    facilities, other sounds may stop or be mixed with the new
    sound. The sound can be played again at any time, possibly mixing
    or replacing previous plays of the sound.

    \sa fileName()
*/
void QSound::play()
{
//    Q_D(QSound);
//    d->looprem = d->looptotal;
//    server().play(this);
}

/*!
    Returns the number of times the sound will play.

    \sa loopsRemaining(), setLoops()
*/
int QSound::loops() const
{
//    Q_D(const QSound);
//    return d->looptotal;
return 0;
}

/*!
    Returns the remaining number of times the sound will loop (this
    value decreases each time the sound is played).

    \sa loops(), isFinished()
*/
int QSound::loopsRemaining() const
{
//    Q_D(const QSound);
//    return d->looprem;
    return 0;
}

/*!
    \fn void QSound::setLoops(int number)

    Sets the sound to repeat the given \a number of times when it is
    played.

    Note that passing the value -1 will cause the sound to loop
    indefinitely.

    \sa loops()
*/
void QSound::setLoops(int n)
{
//    Q_D(QSound);
//    d->looptotal = n;
}

/*!
    Returns the filename associated with this QSound object.

    \sa QSound()
*/
QString QSound::fileName() const
{
//    Q_D(const QSound);
//    return d->filename;
return NULL;
}

/*!
    Stops the sound playing.

    Note that on Windows the current loop will finish if a sound is
    played in a loop.

    \sa play()
*/
void QSound::stop()
{
//    Q_D(QSound);
//    server().stop(this);
//    d->looprem = 0;
}


/*!
    Returns true if sound facilities exist on the platform; otherwise
    returns false.

    If no sound is available, all QSound operations work silently and
    quickly. An application may choose either to notify the user if
    sound is crucial to the application or to operate silently without
    bothering the user.

    Note: On Windows this always returns true because some sound card
    drivers do not implement a way to find out whether it is available
    or not.
*/
bool QSound::isAvailable()
{
//    return server().okay();
return 0;
}

/*!
    Sets the internal bucket record of sound \a s to \a b, deleting
    any previous setting.
*/
//void QAuServer::setBucket(QSound* s, QAuBucket* b)
//{
//    delete s->d_func()->bucket;
//    s->d_func()->bucket = b;
//}


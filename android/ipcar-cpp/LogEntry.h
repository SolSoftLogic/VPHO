#ifndef LOGENTRY_H
#define LOGENTRY_H

#include <QtCore/QString>
#include <QtCore/QDateTime>

enum    LogEntryType
{
    logtIncomming       = 0x00000,
    logtOutgoing        = 0x00001,
    logtAnswered        = 0x00002,
    logtEctSrc          = 0x00004,
    logtEctDst          = 0x00008,

    logtCauseMask       = 0x000f0,
    logtRefused         = 0x00010,
    logtAutoRefused     = 0x00020,
    logtDeflected       = 0x00030,
    logtAutoDeflected   = 0x00040,
    logtBusy            = 0x00050,
    logtAnswer          = 0x00060,
    logtPicked          = 0x00070,
    logtInvalidBC       = 0x00080,
    logtNotOwnNumber    = 0x00090,
    logtAutoAnswered    = 0x000a0,

    logBcMask           = 0x00f00,
    logtVoice           = 0x00000,
    logtAudioVideo      = 0x00100,
    logtChat            = 0x00200,
    logtSms             = 0x00300,
    logtFile            = 0x00400,
    logtVideo           = 0x00500,
    logtVideoMsg        = 0x00600,

    logtFlag            = 0x10000,
    logtRead            = 0x20000,


/*
// Incoming
#define LOGT_BCSHIFT 8
#define LOGT_REASONSHIFT 24
#define LOGT_BCMASK 0x3f00
#define LOGT_CLIR 0x4000
// Incoming not answered

#define LOGT_FILES_UNIQUE 0
#define LOGT_FILES_PERYEAR 1
#define LOGT_FILES_PERMONTH 2
#define LOGT_FILES_PERDAY 3
*/
};

struct  LogEntry
{
    QString         name;
    QString         number;
    QString         message;
    QDateTime       beg;
    unsigned        len;    // seconds
    LogEntryType    type;
};
#endif // LOGENTRY_H

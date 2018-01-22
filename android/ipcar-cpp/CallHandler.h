#ifndef CALLHANDLER_H
#define CALLHANDLER_H
#include "Utils.h"

typedef enum CallType
{
    NoCall,
    IncomingCall,
    OutgoingCall
} CallType;

struct  CallHandler
{
//    virtual         ~CallHandler(){}

    virtual void    onVpStackMessage(   unsigned        msg,
                                        VPCALL          pcall,
                                        unsigned        param)      = 0;
};

#endif // CALLHANDLER_H

#ifndef _OSCL_H_INCLUDED_
#define _OSCL_H_INCLUDED_

#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef NULL
#define NULL 0
#endif

#define OSCL_DLL_ENTRY_POINT_DEFAULT()
#define OSCL_EXPORT_REF
#define OSCL_IMPORT_REF
#define OsclErrNone 0
#define OsclErrNoMemory -1
#define OSCL_TRY(a,b)
#define OSCL_LEAVE(a)
#define OSCL_FIRST_CATCH_ANY(_leave_status, _statements) \
   if (_leave_status!=OsclErrNone) { _statements; }

#define OSCL_ARRAY_DELETE(ptr) delete [] ptr
#define OSCL_ASSERT(expr)
#define OSCL_NEW( T, params) new T params
#define OSCL_DELETE(a) delete a
#define OSCL_UNUSED_ARG(vbl) (void)(vbl)
#define OSCL_STATIC_CAST(type,exp) ((type)(exp))
#define oscl_memset(a,b,c) memset(a,b,c)
#define oscl_memcpy(a,b,c) memcpy(a,b,c)
#define oscl_sqrt(a) sqrt(a)
#define oscl_pow(a,b) pow(a,b)
#define oscl_malloc(a) malloc(a)
#define oscl_free(a) free(a)

typedef unsigned uint;
typedef unsigned uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;

typedef int int32;
typedef short int16;
typedef char int8;

typedef float OsclFloat;

#endif

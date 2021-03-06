#ifndef __IOS_PORTABILITY
#define __IOS_PORTABILITY

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdarg.h>
#include <netdb.h>
#include <ctype.h>
#include <errno.h>

/*

#ifdef DEBUGLOG
#define DLog(fmt, ...) { \
							printf("Func: %s , Line: %d, ",  __PRETTY_FUNCTION__, __LINE__); \
							printf(fmt, ##__VA_ARGS__); \
							printf("\n"); \
						}
#else
#   define DLog(...)
#endif


typedef char			TCHAR;
typedef unsigned char	BYTE;
typedef short			SHORT;
typedef unsigned short	USHORT;
typedef unsigned short	WORD;
typedef unsigned long	DWORD;
typedef long			LONG;
typedef int				SOCKET;
typedef pthread_mutex_t CRITICAL_SECTION;
typedef pthread_cond_t	HANDLE;
typedef time_t			filetime;

#define FALSE			0
#define TRUE			1
#define REG_DWORD		0
#define REG_SZ			1
#define SOCKET_ERROR	(-1)
#define INVALID_SOCKET	(-1)
#define MAX_PATH		260
#define O_BINARY		0
#define WSAECONNRESET	ECONNREFUSED

// recursive mutexes allow a thread to lock it more than once
void InitializeCriticalSection(pthread_mutex_t *a);
#define EnterCriticalSection(a)		pthread_mutex_lock(a)
#define LeaveCriticalSection(a)		pthread_mutex_unlock(a)
#define DeleteCriticalSection(a)	pthread_mutex_destroy(a)

void InitializeEvent(pthread_cond_t &a);
#define DeleteEvent(a)				pthread_cond_destroy(&a)
#define SetEvent(a)					pthread_cond_signal(&a)
#define WaitEvent(a, b, c)			pthread_cond_wait(&a, b)	// To implement as a function that considers c (timeout)

#define closesocket(sk)				close(sk)
#define ZeroMemory(a,b)				memset(a,0,b)
#define TEXT(a) (a)
#define stricmp(a,b)				strcasecmp(a,b)
#define strnicmp(a,b,c)				strncasecmp(a,b,c)
#define _tcscpy(a,b)				strcpy(a,b)
#define _tcschr(a,b)				strchr(a,b)
#define _tcsrchr(a,b)				strrchr(a,b)
//#define GetCurrentThread(a)			();
#define GetCurrentThreadId()		0
#define SetThreadPriority(a,b)
#define WSAGetLastError()			errno
#ifndef MAX
#define MAX(a,b)					((a)>(b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b)					((a)<(b) ? (a) : (b))
#endif
#define CreateDirectory(a,b)		mkdir(a, 0777);
#define mmioFOURCC(a,b,c,d)			ntohl((d | (c<<8) | (b<<16) | (a<<24)))

typedef struct _SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME;

 */

/*
typedef struct tWAVEFORMATEX
{
    WORD        wFormatTag;         // format type 
    WORD        nChannels;          // number of channels (i.e. mono, stereo...) 
    DWORD       nSamplesPerSec;     // sample rate 
    DWORD       nAvgBytesPerSec;    // for buffer estimation 
    WORD        nBlockAlign;        // block size of data 
    WORD        wBitsPerSample;     // number of bits per sample of mono data 
    WORD        cbSize;             // the count in bytes of the size of 
									// extra information (after cbSize) 
} WAVEFORMATEX;


unsigned long _beginthread(void (*start_address)(void *), unsigned stack_size, void *arglist);
int memicmp(const void *a, const void *b, unsigned int len);
unsigned GetTickCount();
void GetLocalTime(SYSTEMTIME *t);
void GetSystemTime(SYSTEMTIME *t);
LONG InterlockedIncrement(LONG *a);
LONG InterlockedDecrement(LONG *a);
LONG InterlockedCompareExchangeLong(LONG *destination, LONG exchange, LONG comperand);
void InterlockedInit();

#define WSAStartup(a,b)
#define WSACleanup()
#define MAKEWORD(a,b) ((((a) & 0xff) << 8) | ((b) & 0xff))
#define WSADATA int

class dummythreadclass {};
typedef void (dummythreadclass::*ClassThreadStart)(void *param);
#define BEGINCLASSTHREAD(a,b,c) beginclassthread((ClassThreadStart)a,b,c)
void beginclassthread(ClassThreadStart start, void *object, void *userparam);

typedef void *HWND;

typedef struct
{
	LONG left, top, right, bottom;
} RECT;

typedef struct {
        DWORD      biSize;
        LONG       biWidth;
        LONG       biHeight;
        WORD       biPlanes;
        WORD       biBitCount;
        DWORD      biCompression;
        DWORD      biSizeImage;
        LONG       biXPelsPerMeter;
        LONG       biYPelsPerMeter;
        DWORD      biClrUsed;
        DWORD      biClrImportant;
} BITMAPINFOHEADER;

#define CopyDWord(a,b) *(DWORD *)(a) = (b)
#define CopyWord(a,b) *(WORD *)(a) = (b)
#define CopyDWordP(a,b) *(DWORD *)(a) = *(DWORD *)(b)
#define CopyWordP(a,b) *(WORD *)(a) = *(WORD *)(b)
#define BuildDWord(a) *(DWORD *)(a)
#define Sleep(a) usleep(1000*(a))
#define MulDiv(a,b,c) (int)((long long)(a) * (b) / (c))
#define topenfile(a,b) openfile(a,b)
*/
 
#endif//__IOS_PORTABILITY
#include <windows.h>
#include <winsock.h>
#ifndef _WIN32_WCE
#include <process.h>
#include <fcntl.h>
#endif
#include <tchar.h>

#define InitializeEvent(a) a = CreateEvent(0, FALSE, FALSE, 0)
#define DeleteEvent(a) CloseHandle(a)
#define WaitEvent(a, b, c) (LeaveCriticalSection(b), WaitForSingleObject(a, c), EnterCriticalSection(b))

// SHAREABLEMUTEXes are mutexes that can be unlocked by a thread that was not the
// thread that locked them
// Under Windows XP, CRITICAL_SECTIONs are good (despite the documentation), but
// under Windows CE they are not good. According to documentation, mutexes are not good,
// but semaphores are good, so they will be used here

#define SHAREABLEMUTEX HANDLE
#define InitShareableMutex(a) *a = CreateSemaphore(0,1,1,0)
#define LockShareableMutex(a) WaitForSingleObject(a, INFINITE)
#define UnlockShareableMutex(a) ReleaseSemaphore(a, 1, 0)
#define CloseShareableMutex(a) CloseHandle(a)

typedef int socklen_t;
typedef FILETIME filetime;
#ifndef MIN
#define MIN(a,b) __min(a,b)
#define MAX(a,b) __max(a,b)
#endif

#ifdef __cplusplus
class dummythreadclass {};
typedef void (dummythreadclass::*ClassThreadStart)(void *param);
#define BEGINCLASSTHREAD(a,b,c) beginclassthread((ClassThreadStart)a,b,c)
void beginclassthread(ClassThreadStart start, void *object, void *userparam);
#endif

#ifdef _WIN32_WCE
#define InterlockedCompareExchangeLong(a,b,c) InterlockedCompareExchangePointer((PVOID *)a,(PVOID)b,(PVOID)c)
#ifdef __cplusplus
extern "C" {
#endif
time_t time(time_t *tm);
struct tm *localtime(const time_t *t);
int _beginthread(void (*start)(void *), int stacksize, void *param);
int memicmp(const void *a, const void *b, int len);
int stricmp(const char *a, const char *b);
int strnicmp(const char *a, const char *b, int len);
void CopyDWord(void *a, DWORD b);
void CopyWord(void *a, WORD b);
void CopyDWordP(void *a, const void *b);
void CopyWordP(void *a, const void *b);
WORD BuildWord(const void *a);
DWORD BuildDWord(const void *a);
#ifdef __cplusplus
}
#endif

#define TPM_RIGHTBUTTON 0
#define O_WRONLY 1
#define O_RDONLY 2
#define O_BINARY 4
#define O_CREAT 8
#define O_TRUNC 16
#define MulDiv(a,b,c) (int)((LONGLONG)(a) * (b) / (c))
#define CoInitialize(x) CoInitializeEx(x, COINIT_MULTITHREADED)

#define WS_OVERLAPPEDWINDOW (WS_OVERLAPPED     | \
                             WS_CAPTION        | \
                             WS_SYSMENU        | \
                             WS_THICKFRAME     | \
                             WS_MINIMIZEBOX    | \
                             WS_MAXIMIZEBOX)


#else

#define CopyDWord(a,b) *(unsigned *)(a) = (b)
#define CopyWord(a,b) *(unsigned short *)(a) = (b)
#define CopyDWordP(a,b) *(unsigned *)(a) = *(unsigned *)(b)
#define CopyWordP(a,b) *(unsigned short *)(a) = *(unsigned short *)(b)
#define BuildDWord(a) *(unsigned *)(a)
void PerformExceptionHandling();
#if _MSC_VER <= 1200
#define InterlockedCompareExchangeLong(a,b,c) InterlockedCompareExchange((PVOID *)a,(PVOID)b,(PVOID)c)
#else
#define InterlockedCompareExchangeLong(a,b,c) InterlockedCompareExchange(a,b,c)
#endif

#endif

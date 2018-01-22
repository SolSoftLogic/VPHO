#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "util.h"

// destsize is the TOTAL available space in the dest buffer
int generatelog, debugmode, Lprintf_opt_nocrlf;
// Logging levels:
// 1 basic
// 2 dump all protocol messages unless media
// 3 dump also media, but dump at most 20 bytes
// 4 dump everything

void strncpy2(char *dest, const char *src, int destsize)
{
	if(src)
	{
		destsize--;
		while(destsize && *src)
		{
			*dest++ = *src++;
			destsize--;
		}
	}
	*dest = 0;
}

void LogAsIs(const char *text, int len)
{
	char path[MAX_PATH];
	int f;
	SYSTEMTIME t;

	if(!generatelog)
		return;
	GetSystemTime(&t);
#ifdef VPLOGDIR
	sprintf(path, "%s\\%s\\%04d%02d%02d.log", StartDirPtr(), VPLOGDIR, t.wYear, t.wMonth, t.wDay);
#else
	sprintf(path, "%s\\VPSTACK.LOG", StartDirPtr());
#endif
	if(debugmode)
		fwrite(text, len, 1, stdout);
	f = openfile(path, O_WRONLY | O_CREAT | O_BINARY);
	if(f != -1)
	{
		seekfile(f, 0, SEEK_END);
		writefile(f, text, len);
		writefile(f, "\r\n", 2);
		closefile(f);
	}
}

#ifdef LAZYLOG
static char *ldb_buf;
static int ldb_size, ldb_alloc;
static CRITICAL_SECTION ldb_cs;

void WriteLazyLog()
{
	int f;
	char path[MAX_PATH];

	if(!ldb_size)
		return;
	sprintf(path, "%s\\VPSTACK.LOG", StartDirPtr());
	f = openfile(path, O_WRONLY | O_BINARY | O_CREAT);
	if(f != -1)
	{
		seekfile(f, 0, SEEK_END);
		writefile(f, ldb_buf, ldb_size);
		closefile(f);
	}
	ldb_size = 0;
}

extern void DumpJobs();

void lazylogthread(void *dummy)
{
	for(;;)
	{
		Sleep(5000);
		EnterCriticalSection(&ldb_cs);
		if(generatelog >= 2)
		{
			Lprintf("Periodical write");
			DumpJobs();
		}
		WriteLazyLog();
		LeaveCriticalSection(&ldb_cs);
	}
}

void InitLazyLog()
{
	InitializeCriticalSection(&ldb_cs);
	ldb_alloc = 100000;
	ldb_buf = (char *)malloc(ldb_alloc);
	_beginthread(lazylogthread, 0, 0);
}

#endif

void Lprintf(const char *fmt, ...)
{
	char s[5500], *p, path[MAX_PATH];
	va_list ap;
	SYSTEMTIME t;
	int f;

	if(!generatelog)
		return;
	GetSystemTime(&t);
#ifdef VPLOGDIR
	sprintf(s, "%d:%02d:%02d.%03d   ", t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);
#else
	sprintf(s, "%d/%d/%d %d:%02d:%02d.%03d   ", t.wDay, t.wMonth, t.wYear, t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);
#endif
	p = s + strlen(s);
	va_start(ap, fmt);
	vsprintf(p, fmt, ap);
	va_end(ap);
	p = s + strlen(s);
	if(p-2 >= s && !(p[-2] == '\r' && p[-1] == '\n'))
	{
		if(p[-1] == '\n')
		{
			p[-1] = '\r';
			p[0] = '\n';
			p[1] = 0;
		} else {
			p[0] = '\r';
			p[1] = '\n';
			p[2] = 0;
		}
	}
#ifdef VPLOGDIR
	sprintf(path, "%s\\%s\\%04d%02d%02d.log", StartDirPtr(), VPLOGDIR, t.wYear, t.wMonth, t.wDay);
#else
	sprintf(path, "%s\\VPSTACK.LOG", StartDirPtr());
#endif
	if(debugmode)
		fputs(s, stdout);
#ifdef LAZYLOG
	EnterCriticalSection(&ldb_cs);
	f = strlen(s);
	strcpy(ldb_buf + ldb_size, s);
	ldb_size += f;
	if(ldb_size > ldb_alloc)
		WriteLazyLog();
	LeaveCriticalSection(&ldb_cs);
#else
	f = openfile(path, O_WRONLY | O_BINARY | O_CREAT);
	if(f != -1)
	{
		seekfile(f, 0, SEEK_END);
		writefile(f, s, strlen(s));
		closefile(f);
	}
#endif
}

void VPLprintf(const char *fmt, ...)
{
	char s[5500];
	
	va_list ap;
	va_start(ap, fmt);
	vsprintf(s, fmt, ap);
	va_end(ap);
	Lprintf("%s", s);
}

int Resolve(const char *address1, struct sockaddr_in *addr, int defaultport)
{
	char *p, address[300];
	int i;

	if(p = (char *)strchr(address1, '<'))
	{
		strcpy(address, p + 1);
		if(p = strchr(address, '>'))
			*p = 0;
	} else strcpy(address, address1);
	memset(addr, 0, sizeof(struct sockaddr_in));
	addr->sin_family = AF_INET;
	addr->sin_port = htons((unsigned short)defaultport);
	if(p = strchr(address, ':'))
	{
		addr->sin_port = htons((unsigned short)atoi(p + 1));
		*p = 0;
	}
	if(!*address)
		return 0;
	for(i = 0; address[i]; i++)
		if(!isdigit((BYTE)address[i]) && address[i] != '.')
			break;
	if(address[i])
	{
		struct hostent *h;
		
		h = gethostbyname(address);
		if(!h)
			return 0;	// Error host not found
		addr->sin_addr.s_addr = *((unsigned long *)h->h_addr);
		return 1;
	} else {
		addr->sin_addr.s_addr = inet_addr(address);
		if(addr->sin_addr.s_addr)
			return 1;
		else return 0;
	}
}

#define TOHEX(a) ((a) >= 10 ? 'a' + (a) - 10 : '0' + (a))
#define TOBIN(a) ((a) >= 'a' ? (a) - 'a' + 10 : (a) >= 'A' ? (a) - 'A' + 10 : (a) - '0')

void BinaryToHex(unsigned char *binary, int len, char *hex)
{
	int i;

	for(i = 0; i < len; i++)
		sprintf(hex + 2 * i, "%c%c", TOHEX(binary[i] / 16), TOHEX(binary[i] % 16));
	hex[2 * i] = 0;
}

void HexToBinary(char *hex, unsigned char *binary)
{
	int len = strlen(hex) / 2, i;

	for(i = 0; i < len; i++)
		binary[i] = TOBIN(hex[2 * i]) * 16 + TOBIN(hex[2 * i + 1]);
}

typedef struct {
	ClassThreadStart start;
	dummythreadclass *object;
	void *param;
} classthreadparam;

void classthreadstart(classthreadparam *param)
{
	(param->object->*param->start)(param->param);
	delete param;
}

void beginclassthread(ClassThreadStart start, void *object, void *userparam)
{
	classthreadparam *param = new classthreadparam;

	param->object = (dummythreadclass *)object;
	param->start = start;
	param->param = userparam;
	_beginthread((void (*)(void *))classthreadstart, 0, param);
}

// Maxfields is the len of the ptrs array, returns the number of valid fields (the final zero is excluded)
int Fieldize(char *string, char **ptrs, int maxfields)
{
	int i = 0;
	char *p;

	if(p = strchr(string, '\n'))
		*p = 0;
	if(!*string)
	{
		ptrs[0] = 0;
		return 0;
	}
	ptrs[0] = string;
	ptrs[maxfields - 1] = 0;
	while(i < maxfields - 2)
	{
		ptrs[i + 1] = strchr(ptrs[i], '\t');
		if(ptrs[i + 1])
			*ptrs[i + 1]++ = 0;
		else break;
		i++;
	}
	return i + 1;
}

int Fieldize(char *string, char **ptrs, int maxfields, const char *separators)
{
	int i = 0, j;
	char *p;

	if(p = strchr(string, '\n'))
		*p = 0;
	if(!*string)
	{
		ptrs[0] = 0;
		return 0;
	}
	ptrs[0] = string;
	ptrs[maxfields - 1] = 0;
	while(i < maxfields - 2)
	{
		for(j = 0; separators[j]; j++)
			if(ptrs[i + 1] = strchr(ptrs[i], separators[j]))
				break;
		if(ptrs[i + 1])
			*ptrs[i + 1]++ = 0;
		else break;
		i++;
	}
	return i + 1;
}

void utimetotext(time_t tm, char *text)
{
#ifdef WIN32
	SYSTEMTIME t;
	FILETIME ft, lft;
	LONGLONG lt;

	memset(&t, 0, sizeof(t));
	t.wDay = 1;
	t.wMonth = 1;
	t.wYear = 1970;
	SystemTimeToFileTime(&t, &ft);
	lt = (LONGLONG)ft.dwHighDateTime << 32 | ft.dwLowDateTime;
	lt += tm * (LONGLONG)10000000;
	ft.dwLowDateTime = (ULONG)lt;
	ft.dwHighDateTime = (ULONG)(lt >> 32);

	FileTimeToLocalFileTime(&ft, &lft);
	FileTimeToSystemTime(&lft, &t);

	sprintf(text, "%d/%d/%d %d:%02d:%02d", t.wDay, t.wMonth, t.wYear, t.wHour, t.wMinute, t.wSecond);
#else
	struct tm *t = localtime((time_t *)&tm);
	
	sprintf(text, "%d/%d/%d %d:%02d:%02d", t->tm_mday, t->tm_mon+1, t->tm_year+1900, t->tm_hour, t->tm_min, t->tm_sec);
#endif
}

int CreatePath(const TCHAR *path, bool pathisfile)
{
	TCHAR path2[MAX_PATH], *p;

	_tcscpy(path2, path);
	if(pathisfile)
	{
		if(p = _tcsrchr(path2, '\\'))
			*p = 0;
		else return -1;
	}
	p = path2;
	p = _tcschr(p, '\\');
	if(!p)
		return -1;
	while(p = _tcschr(p, '\\'))
	{
		*p = 0;
		CreateDirectory(path2, 0);
		*p++ = '\\';
	}
	CreateDirectory(path2, 0);
	return 0;
}

int DebugAppendFile(const char *path, const void *buf, int datalen)
{
	int f = openfile(path, O_WRONLY | O_BINARY | O_CREAT);
	if(f == -1)
		return -1;
	seekfile(f, 0, SEEK_END);
	writefile(f, buf, datalen);
	closefile(f);
	return 0;
}

/* J. Courtois, F. Heymans, D. L. Parnas, MBLE Research Laboratory, Brussels, Belgium
Concurrent Control with "Readers" and "Writers"
Communications of the ACM, October 1971, Volume 14, Number 10*/

RWLOCK::RWLOCK()
{
	InitializeCriticalSection(&m1);
	InitializeCriticalSection(&m2);
	InitializeCriticalSection(&m3);
	InitShareableMutex(&w);
	InitShareableMutex(&r);
	readcount = writecount = 0;
}

RWLOCK::~RWLOCK()
{
	DeleteCriticalSection(&m1);
	DeleteCriticalSection(&m2);
	DeleteCriticalSection(&m3);
	CloseShareableMutex(w);
	CloseShareableMutex(r);
}

void RWLOCK::RLock()
{
	EnterCriticalSection(&m3);
	LockShareableMutex(r);
	EnterCriticalSection(&m1);
	readcount++;
	if(readcount == 1)
		LockShareableMutex(w);
	LeaveCriticalSection(&m1);
	UnlockShareableMutex(r);
	LeaveCriticalSection(&m3);

}

void RWLOCK::RUnlock()
{
	EnterCriticalSection(&m1);
	readcount--;
	if(readcount == 0)
		UnlockShareableMutex(w);
	LeaveCriticalSection(&m1);
}

void RWLOCK::WLock()
{
	EnterCriticalSection(&m2);
	writecount++;
	if(writecount == 1)
		LockShareableMutex(r);
	LeaveCriticalSection(&m2);
	LockShareableMutex(w);
}

void RWLOCK::WUnlock()
{
	UnlockShareableMutex(w);
	EnterCriticalSection(&m2);
	writecount--;
	if(writecount == 0)
		UnlockShareableMutex(r);
	LeaveCriticalSection(&m2);
}

int VPFreeBuffer(void *buffer)
{
	if(buffer)
		free(buffer);
	return 0;
}
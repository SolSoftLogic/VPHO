#include <stdio.h>
#include <pthread.h>
#include <ctype.h>
#include <sys/times.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include "../portability.h"
#include "../util.h"

void InitializeCriticalSection(pthread_mutex_t *a)
{
	static const pthread_mutex_t tmp = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
	
	*a = tmp;
	pthread_mutex_init(a, 0);
}

void InitializeEvent(pthread_cond_t &a)
{
	static const pthread_cond_t tmp = PTHREAD_COND_INITIALIZER;
	
	a = tmp;
	pthread_cond_init(&a, 0);
}

int memicmp (const void * first, const void * last, unsigned int count)
{
	int f = 0;
	int l = 0;

	while ( count-- )
	{
		if ( (*(unsigned char *)first == *(unsigned char *)last) ||
			((f = tolower( *(unsigned char *)first )) ==
			(l = tolower( *(unsigned char *)last ))) )
		{
			first = (char *)first + 1;
			last = (char *)last + 1;
		} else break;
	}
	return ( f - l );
}

unsigned GetTickCount()
{
	struct tms tm;
	return times(&tm) * 10;
}

CRITICAL_SECTION iopscs;

LONG InterlockedIncrement(LONG *a)
{
	LONG b;
	
	EnterCriticalSection(&iopscs);
	b = ++(*a);
	LeaveCriticalSection(&iopscs);
	return b;
}

LONG InterlockedDecrement(LONG *a)
{
	LONG b;
	
	EnterCriticalSection(&iopscs);
	b = --(*a);
	LeaveCriticalSection(&iopscs);
	return b;
}

LONG InterlockedCompareExchangeLong(LONG *destination, LONG exchange, LONG comperand)
{
	LONG prevdest;
	
	EnterCriticalSection(&iopscs);
	prevdest = *destination;
	if(prevdest == comperand)
		*destination = exchange;
	LeaveCriticalSection(&iopscs);
	return prevdest;
}

void InterlockedInit()
{
	InitializeCriticalSection(&iopscs);
}

unsigned long _beginthread(void (*start_address)(void *), unsigned stack_size, void *arglist)
{
	pthread_t tid;

	pthread_create(&tid, 0, (void *(*)(void *))start_address, arglist);
	pthread_detach(tid);
	return 0;
}

void GetLocalTime(SYSTEMTIME *t)
{
	struct timeval tv;
	struct tm *tm;
	
	gettimeofday(&tv, 0);
	t->wMilliseconds = tv.tv_usec / 1000;
	tm = localtime(&tv.tv_sec);
	t->wYear = tm->tm_year + 1900;
	t->wMonth = tm->tm_mon + 1;
	t->wDay = tm->tm_mday;
	t->wDayOfWeek = tm->tm_wday;
	t->wHour = tm->tm_hour;
	t->wMinute = tm->tm_min;
	t->wSecond = tm->tm_sec;
}

void GetSystemTime(SYSTEMTIME *t)
{
	struct timeval tv;
	struct tm *tm;

	gettimeofday(&tv, 0);
	t->wMilliseconds = tv.tv_usec / 1000;
	tm = gmtime(&tv.tv_sec);
	t->wYear = tm->tm_year + 1900;
	t->wMonth = tm->tm_mon + 1;
	t->wDay = tm->tm_mday;
	t->wDayOfWeek = tm->tm_wday;
	t->wHour = tm->tm_hour;
	t->wMinute = tm->tm_min;
	t->wSecond = tm->tm_sec;
}

char *StartDirPtr()
{
	static char startdir[MAX_PATH];
	char *p;
	
	if(!*startdir)
	{
		if(readlink("/proc/self/exe", startdir, sizeof(startdir)) == -1)
			strcpy(startdir, "~");
		p = strrchr(startdir, '/');
		if(p)
			*p = 0;
	}
	return startdir;
}

void RandomData(BYTE *data, int len)
{
	int f;
	
	f = open("/dev/urandom", O_RDONLY);
	if(f != -1)
	{
		if(read(f, data, len) != len)
			memset(data, 0, len);
		close(f);
	}
}

void RandomString128(char *s)
{
	BYTE buf[16];

	RandomData(buf, 16);
	BinaryToHex(buf, 16, s);
}

void strcpypath(char *dest, const char *src)
{
	int i;

	for(i = 0; src[i]; i++)
	{
		*dest = src[i];
		if(*dest == '\\')
			*dest = '/';
		*dest++;
		if(i > 0 && src[i] == '\\' && src[i + 1] == '\\' || src[i] == '/' && src[i + 1] == '/')
			i++;
	}
	*dest = 0;
}

unsigned filelengthpath(const char *path)
{
	char path2[MAX_PATH];
	struct stat st;

	strcpypath(path2, path);
	if(stat(path2, &st))
		return 0;
	return st.st_size;
}

filetime getfilewritetime(const char *path)
{
	char path2[MAX_PATH];
	struct stat st;

	strcpypath(path2, path);
	if(stat(path2, &st))
		return 0;
	return st.st_mtime;
}

int deletefile(const char *path)
{
	return unlink(path);
}

int renamefile(const char *from, const char *to)
{
	return rename(from, to);
}

int createdirectory(const char *path)
{
	return mkdir(path, 0755);
}

int openfile(const char *path, int mode)
{
	char path2[MAX_PATH];

	strcpypath(path2, path);
	if(mode & O_CREAT)
		return open(path2, mode, 0644);
	else return open(path2, mode);
}

int seekfile(int f, long offset, int origin)
{
	return lseek(f, offset, origin);
}

int closefile(int f)
{
	return close(f);
}

int getfilesize(int f)
{
	struct stat st;

	if(fstat(f, &st))
		return 0;
	return st.st_size;
}

int readfile(int f, void *buf, int len)
{
	return read(f, buf, len);
}

int writefile(int f, const void *buf, int len)
{
	return write(f, buf, len);
}

int VPInit()
{
	static int init;

	if(init)
		return 0;
	init = 1;
#ifdef LAZYLOG
	InitLazyLog();
#endif
	InterlockedInit();
	return 0;
}

#define LADDRI (*(unsigned long *)&(((struct sockaddr_in *)&ifr[i].ifr_addr)->sin_addr))
#define BADDRI ((unsigned char *)&(((struct sockaddr_in *)&ifr[i].ifr_addr)->sin_addr))

unsigned long FindLocalHostAddr(char *localhostsaddr, int autolocalhostaddr)
{
	SOCKET sk;
	struct ifconf ifc;
	struct ifreq ifr[10];
	int i;
	unsigned localhostaddr = 0;
	
	sk = socket(AF_INET, SOCK_STREAM, 0);
	ifc.ifc_req = ifr;
	ifc.ifc_len = sizeof ifr;
	if(ioctl(sk, SIOCGIFCONF, &ifc))
	{
	printf("Local host address is error\n");
		closesocket(sk);
		return 0;
	}
	closesocket(sk);
	if(!autolocalhostaddr && localhostsaddr)
	{
		localhostaddr = inet_addr(localhostsaddr);
		for(i = 0; i * sizeof ifr[0] < (unsigned)ifc.ifc_len; i++)
		{
			if(localhostaddr == LADDRI)
				break;
		}
		if(!LADDRI)
			localhostaddr = 0;
	} else localhostaddr = 0;
	if(!localhostaddr)
	for(i = 0; i * sizeof ifr[0] < (unsigned)ifc.ifc_len; i++)
	{
		if(!localhostaddr || BADDRI[0] != 169)
		{
			localhostaddr = LADDRI;
			if(BADDRI[0] == 192 && BADDRI[1] == 168 ||
				BADDRI[0] == 10 ||
				BADDRI[0] == 172 && (BADDRI[1] & 0xf0) == 0x10)
				break;
		}
	}
	if(localhostsaddr)
		strcpy(localhostsaddr, inet_ntoa(*(struct in_addr *)&localhostaddr));
	Lprintf("Local host address is %s", inet_ntoa(*(struct in_addr *)&localhostaddr));
	return 0;
}

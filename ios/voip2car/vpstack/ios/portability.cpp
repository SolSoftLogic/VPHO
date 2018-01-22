#include <stdio.h>
#include <pthread.h>
#include <ctype.h>
#include <sys/times.h>
#include <sys/time.h>
#include <sys/stat.h>
#include "../portability.h"
#include "../util.h"

// Provide the Linux initializers for MacOS X 
#define PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP  { 0x4d555458, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x20 } }

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
/*	struct tms tm;
	return times(&tm) * 10;
*/
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return (unsigned)(tv.tv_sec*1000 + (tv.tv_usec / 1000));
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
   strcpy(dest, src);
}

unsigned filelengthpath(const char *path)
{
		//char path2[MAX_PATH];
	struct stat st;

	//strcpypath(path2, path);
	if(stat(path, &st))
		return 0;
	return st.st_size;
}

filetime getfilewritetime(const char *path)
{
		//char path2[MAX_PATH];
	struct stat st;

	//strcpypath(path2, path);
	if(stat(path, &st))
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
		//char path2[MAX_PATH];

	//strcpypath(path2, path);
	if(mode & O_CREAT)
		return open(path, mode, 0644);
	else return open(path, mode);
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
	InterlockedInit();
	return 0;
}


unsigned long FindLocalHostAddr(char *localhostsaddr, int autolocalhostaddr)
{
	char buf[300];
	struct hostent *h;
	int i;
	unsigned localhostaddr;
	
	gethostname(buf, 300);
	h = gethostbyname(buf);
	localhostaddr = 0;
	if(h)
	{
		if(!autolocalhostaddr && localhostsaddr)
		{
			localhostaddr = inet_addr(localhostsaddr);
			for(i = 0; h->h_addr_list[i]; i++)
			{
				if(localhostaddr == *((unsigned long *)h->h_addr_list[i]))
					break;
			}
			if(!h->h_addr_list[i])
				localhostaddr = 0;
		} else localhostaddr = 0;
		if(!localhostaddr)
			for(i = 0; h->h_addr_list[i]; i++)
			{
				if(!localhostaddr ||
				   *((unsigned long *)h->h_addr_list[i]) != 0xeeeea8c0 &&
				   (*((unsigned long *)h->h_addr_list[i]) & 0xff) != 0xa9)
				{
					localhostaddr = *((unsigned long *)h->h_addr_list[i]);
					if((localhostaddr & 0xffff) == 0xa8c0
					   && localhostaddr != 0x6537a8c0	// This is something on the 7" device
					   ||
					   (localhostaddr & 0xff) == 0x0a ||
					   (localhostaddr & 0xf0ff) == 0x10ac)
						break;
				}
			}
		if(localhostsaddr)
			strcpy(localhostsaddr, inet_ntoa(*(struct in_addr *)&localhostaddr));
	}
	Lprintf("Local host address is %s", inet_ntoa(*(struct in_addr *)&localhostaddr));
	return localhostaddr;
}

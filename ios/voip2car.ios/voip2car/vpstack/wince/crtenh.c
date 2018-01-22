#include <windows.h>
#include <time.h>
#include "../windows/wportability.h"

struct threadparam
{
	void (*start)(void *);
	void *param;
};

static DWORD WINAPI ThreadProc(void *param)
{
	((struct threadparam *)param)->start(((struct threadparam *)param)->param);
	free(param);
	return 0;
}

int _beginthread(void (*start)(void *), int stacksize, void *param)
{
	DWORD id;
	struct threadparam *tp;

	tp = (struct threadparam *)malloc(sizeof(struct threadparam));
	tp->start = start;
	tp->param = param;
	CreateThread(0, stacksize, ThreadProc, tp, 0, &id);
	return 0;
}

int memicmp(const void *a, const void *b, int len)
{

	while(len && toupper(*(unsigned char *)a) == toupper(*(unsigned char *)b))
	{
		((const char *)a)++;
		((const char *)b)++;
		len--;
	}
	if(!len)
		return 0;
	return (int)*(char *)a - (int)*(char *)b;
}

int stricmp(const char *a, const char *b)
{
	while(*a && *b && toupper((unsigned char)*a) == toupper((unsigned char)*b))
	{
		a++;
		b++;
	}
	return (int)*a - (int)*b;
}

int strnicmp(const char *a, const char *b, int n)
{
	while(n > 0 && *a && *b && toupper((unsigned char)*a) == toupper((unsigned char)*b))
	{
		a++;
		b++;
		n--;
	}
	return (int)*a - (int)*b;
}

int unlink(const char *path)
{
	WCHAR wpath[MAX_PATH];

	MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, MAX_PATH);
	return DeleteFile(wpath);
}

int rename(const char *frompath, const char *topath)
{
	WCHAR wfrompath[MAX_PATH], wtopath[MAX_PATH];

	MultiByteToWideChar(CP_UTF8, 0, frompath, -1, wfrompath, MAX_PATH);
	MultiByteToWideChar(CP_UTF8, 0, topath, -1, wtopath, MAX_PATH);
	return MoveFile(wfrompath, wtopath);
}

static LONGLONG ltbase;

time_t time(time_t *tm)
{
	SYSTEMTIME t;
	FILETIME ft;
	LONGLONG lt;
	unsigned tm1;

	if(!ltbase)
	{
		memset(&t, 0, sizeof(t));
		t.wDay = 1;
		t.wMonth = 1;
		t.wYear = 1970;
		SystemTimeToFileTime(&t, &ft);
		ltbase = (LONGLONG)ft.dwHighDateTime << 32 | ft.dwLowDateTime;
	}
	GetSystemTime(&t);
	SystemTimeToFileTime(&t, &ft);
	lt = (LONGLONG)ft.dwHighDateTime << 32 | ft.dwLowDateTime;
	tm1 = (unsigned)((lt - ltbase) / 10000000);
	if(tm)
		*tm = tm1;
	return tm1;
}

struct tm *localtime(const time_t *t)
{
	SYSTEMTIME st;
	FILETIME ft;
	LONGLONG lt;
	static struct tm tm;

	if(!ltbase)
	{
		memset(&st, 0, sizeof(st));
		st.wDay = 1;
		st.wMonth = 1;
		st.wYear = 1970;
		SystemTimeToFileTime(&st, &ft);
		ltbase = (LONGLONG)ft.dwHighDateTime << 32 | ft.dwLowDateTime;
	}
	lt = *t * (LONGLONG)10000000 + ltbase;
	FileTimeToLocalFileTime((FILETIME *)&lt, &ft);
	FileTimeToSystemTime(&ft, &st);
	tm.tm_hour = st.wHour;
	tm.tm_min = st.wMinute;
	tm.tm_sec = st.wSecond;
	tm.tm_mday = st.wDay;
	tm.tm_mon = st.wMonth - 1;
	tm.tm_year = st.wYear - 1900;
	tm.tm_wday = st.wDayOfWeek;
	return &tm;
}

void CopyDWord(void *a, DWORD b)
{
	BYTE *a1 = (BYTE *)a, *b1 = (BYTE *)&b;
	a1[0] = b1[0];
	a1[1] = b1[1];
	a1[2] = b1[2];
	a1[3] = b1[3];
}

void CopyWord(void *a, WORD b)
{
	BYTE *a1 = (BYTE *)a, *b1 = (BYTE *)&b;
	a1[0] = b1[0];
	a1[1] = b1[1];
}

void CopyDWordP(void *a, const void *b)
{
	BYTE *a1 = (BYTE *)a, *b1 = (BYTE *)b;
	a1[0] = b1[0];
	a1[1] = b1[1];
	a1[2] = b1[2];
	a1[3] = b1[3];
}

void CopyWordP(void *a, const void *b)
{
	BYTE *a1 = (BYTE *)a, *b1 = (BYTE *)b;
	a1[0] = b1[0];
	a1[1] = b1[1];
}

WORD BuildWord(const void *a)
{
	const BYTE *a1 = (const BYTE *)a;

	return ((WORD)a1[0]) | ((WORD)a1[1] << 8);
}

DWORD BuildDWord(const void *a)
{
	const BYTE *a1 = (const BYTE *)a;

	return ((DWORD)a1[0]) | ((DWORD)a1[1] << 8) | ((DWORD)a1[2] << 16) | ((DWORD)a1[3] << 24);
}

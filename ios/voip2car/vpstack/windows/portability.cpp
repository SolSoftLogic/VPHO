#ifndef _WIN32_WCE
#define _WIN32_WINNT 0x400
#endif

#include <windows.h>
#include <wincrypt.h>
#ifndef _WIN32_WCE
#include <imagehlp.h>
#include <io.h>
#include <fcntl.h>
#endif
#include <stdio.h>
#include <tchar.h>
#include "../util.h"
#include "../portability.h"

#ifndef SETTINGS_INI
#ifndef SETTINGS_HKCU
#ifndef SETTINGS_HKLM

#ifdef _WIN32_WCE
#define SETTINGS_HKCU
#else
#define SETTINGS_INI
#endif

#endif
#endif
#endif

static HCRYPTPROV g_hcrypt;
static bool init;

void InterlockedInit()
{
}

void RandomString128(char *s)
{
	BYTE buf[16];

	if(!init)
	{
		CryptAcquireContext(&g_hcrypt, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
		init = true;
	}

	HRESULT hr = CryptGenRandom(g_hcrypt, 16, buf);
	BinaryToHex(buf, 16, s);
}

void RandomData(BYTE *data, int len)
{
	if(!init)
	{
		init = true;
		CryptAcquireContext(&g_hcrypt, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
	}

	CryptGenRandom(g_hcrypt, len, data);
}

int UTF8ToWindowsString(const char *s, TCHAR *ts)
{
#ifdef UNICODE
	return MultiByteToWideChar(CP_UTF8, 0, s, -1, ts, 300);
#else
	strcpy(ts, s);
	return 0;
#endif
}

int WindowsStringToUTF8(const TCHAR *ts, char *s)
{
#ifdef UNICODE
	return WideCharToMultiByte(CP_UTF8, 0, ts, -1, s, 300, 0, 0);
#else
	strcpy(s, ts);
	return 0;
#endif
}

char *StartDirPtr()
{
	char *p;
	static char startdir[MAX_PATH];

	if(*startdir)
		return startdir;
#ifdef UNICODE
	TCHAR startdirw[MAX_PATH];

	GetModuleFileName(0, startdirw, MAX_PATH);
	WideCharToMultiByte(CP_ACP, 0, startdirw, -1, startdir, MAX_PATH, 0, 0);
#else
	GetModuleFileNameA(0, startdir, MAX_PATH);
#endif
	p = strrchr(startdir, '\\');
	if(p)
		*p = 0;
	else strcpy(startdir, ".");
	return startdir;
}

void strcpypath(TCHAR *dest, const char *src)
{
	int i;
	char tmp[MAX_PATH], *p = tmp;

#ifndef UNICODE
	p = dest;
#endif
	for(i = 0; src[i]; i++)
	{
		*p = src[i];
		if(*p == '/')
			*p = '\\';
		p++;
		if(i > 0 && src[i] == '\\' && src[i + 1] == '\\' || src[i] == '/' && src[i + 1] == '/')
			i++;
	}
	*p = 0;
#ifdef UNICODE
	MultiByteToWideChar(CP_UTF8, 0, tmp, -1, dest, MAX_PATH);
#endif
}

unsigned filelengthpath(const char *path)
{
	HANDLE h;
	WIN32_FIND_DATA fd;
	TCHAR path2[MAX_PATH];
	
	strcpypath(path2, path);
	h = FindFirstFile(path2, &fd);
	if(h == INVALID_HANDLE_VALUE)
		return 0;
	FindClose(h);
	return fd.nFileSizeLow;
}

filetime getfilewritetime(const char *path)
{
	WIN32_FIND_DATA fd;
	HANDLE h;
	TCHAR path2[MAX_PATH];
	static FILETIME nullft;

	strcpypath(path2, path);
	h = FindFirstFile(path2, &fd);
	if(h == INVALID_HANDLE_VALUE)
		return (filetime)nullft;
	FindClose(h);
	return fd.ftLastWriteTime;
}

int deletefile(const char *path)
{
	TCHAR path2[MAX_PATH];

	strcpypath(path2, path);
	return -!DeleteFile(path2);
}

int renamefile(const char *from, const char *to)
{
	TCHAR tfrom[MAX_PATH], tto[MAX_PATH];
	
	strcpypath(tfrom, from);
	strcpypath(tto, to);
	return -!MoveFile(tfrom, tto);
}

int createdirectory(const char *path)
{
	TCHAR path2[MAX_PATH];

	strcpypath(path2, path);
	return -!CreateDirectory(path2, 0);
}

int topenfile(const TCHAR *path, int mode)
{
	HANDLE h;

	h = CreateFile(path, mode & O_WRONLY ? GENERIC_WRITE : GENERIC_READ, FILE_SHARE_READ, 0, mode & O_TRUNC ? CREATE_ALWAYS : mode & O_CREAT ? OPEN_ALWAYS : OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if(h == INVALID_HANDLE_VALUE)
		return -1;
	return (int)h;
}

int openfile(const char *path, int mode)
{
	TCHAR path2[MAX_PATH];
	HANDLE h;
	
	strcpypath(path2, path);
	h = CreateFile(path2, mode & O_WRONLY ? GENERIC_WRITE : GENERIC_READ, FILE_SHARE_READ, 0, mode & O_TRUNC ? CREATE_ALWAYS : mode & O_CREAT ? OPEN_ALWAYS : OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if(h == INVALID_HANDLE_VALUE)
		return -1;
	return (int)h;
}

int seekfile(int f, long offset, int origin)
{
	return SetFilePointer((HANDLE)f, offset, 0, origin == SEEK_SET ? FILE_BEGIN : origin == SEEK_CUR ? FILE_CURRENT : FILE_END);
}

int closefile(int f)
{
	return CloseHandle((HANDLE)f);
}

int getfilesize(int f)
{
	return (int)GetFileSize((HANDLE)f, 0);
}

int readfile(int f, void *buf, int len)
{
	DWORD br;

	if(!ReadFile((HANDLE)f, buf, len, &br, 0))
		return -1;
	return (int)br;
}

int writefile(int f, const void *buf, int len)
{
	DWORD bw;

	if(!WriteFile((HANDLE)f, buf, len, &bw, 0))
		return -1;
	return (int)bw;
}

int LoadRegSetting(HKEY hkey, const char *key, const char *name, void *buf, int *type)
{
	DWORD type1, len = 300;
	TCHAR tname[300], ts[300];

	UTF8ToWindowsString(key, ts);
	UTF8ToWindowsString(name, tname);

	if(RegOpenKeyEx(hkey, ts, 0, KEY_QUERY_VALUE, &hkey) != ERROR_SUCCESS)
		return -1;
	if(RegQueryValueEx(hkey, tname, 0, &type1, (BYTE *)ts, &len) != ERROR_SUCCESS)
	{
		RegCloseKey(hkey);
		return -1;
	}
	if(type1 == REG_SZ)
		WindowsStringToUTF8(ts, (char *)buf);
	else memcpy(buf, ts, len);
	if(type)
		*type = (int)type1;
	RegCloseKey(hkey);
	return 0;
}

int SaveRegSetting(HKEY hkey, const char *key, const char *name, void *buf, int type)
{
	DWORD len = 300;
	DWORD disposition;
	TCHAR tname[300], ts[300];
	int rc;

	UTF8ToWindowsString(key, ts);
	UTF8ToWindowsString(name, tname);

	if(RegCreateKeyEx(HKEY_CURRENT_USER, ts, 0, TEXT(""), 0, KEY_SET_VALUE, 0, &hkey, &disposition) != ERROR_SUCCESS)
		return -1;
	if(type == REG_SZ)
	{
		UTF8ToWindowsString((char *)buf, ts);
		rc = RegSetValueEx(hkey, tname, 0, (BYTE)type, (BYTE *)ts, (strlen((char *)buf) + 1) * sizeof(TCHAR));
	} else if(type == REG_BINARY)
		rc = RegSetValueEx(hkey, tname, 0, (BYTE)type, (BYTE *)buf, 16);
	else rc = RegSetValueEx(hkey, tname, 0, (BYTE)type, (BYTE *)buf, 4);
	if(rc != ERROR_SUCCESS)
	{
		RegCloseKey(hkey);
		return -1;
	}
	RegCloseKey(hkey);
	return 0;
}

int LoadSettings(const char *key, const char *name, void *buf, int type)
{
#ifdef SETTINGS_HKLM
	char lkey[300];
	int type1, rc;

	sprintf(lkey, "SOFTWARE\\%s", key);
	rc = LoadRegSetting(HKEY_LOCAL_MACHINE, lkey, name, buf, &type1);
	if(type != type1)
		return -1;
	return rc;
#endif
#ifdef SETTINGS_HKCU
	char lkey[300];
	int type1, rc;

	sprintf(lkey, "SOFTWARE\\%s", key);
	rc = LoadRegSetting(HKEY_CURRENT_USER, lkey, name, buf, &type1);
	if(type != type1)
		return -1;
	return rc;
#endif
#ifdef SETTINGS_INI
	static char path[MAX_PATH];
	char *p;

	if(!*path)
	{
		GetModuleFileName(0, path, MAX_PATH);
		p = strrchr(path, '.');
		if(p)
			strcpy(p + 1, "ini");
	}
	if(!*path)
		return -1;
	if(type == REG_SZ)
		GetPrivateProfileString(key, name, "", (char *)buf, 300, path);
	else *(int *)buf = GetPrivateProfileInt(key, name, *(int *)buf, path);
	return 0;
#endif
}

int SaveSettings(const char *key, const char *name, void *buf, int type)
{
#ifdef SETTINGS_HKLM
	char lkey[300];

	sprintf(lkey, "SOFTWARE\\%s", key);
	return SaveRegSetting(HKEY_LOCAL_MACHINE, lkey, name, buf, type);
#endif
#ifdef SETTINGS_HKCU
	char lkey[300];

	sprintf(lkey, "SOFTWARE\\%s", key);
	return SaveRegSetting(HKEY_CURRENT_USER, lkey, name, buf, type);
#endif
#ifdef SETTINGS_INI
	static char path[MAX_PATH];
	char *p;

	if(!*path)
	{
		GetModuleFileName(0, path, MAX_PATH);
		p = strrchr(path, '.');
		if(p)
			strcpy(p + 1, "ini");
	}
	if(!*path)
		return -1;
	if(type == REG_SZ)
		WritePrivateProfileString(key, name, (char *)buf, path);
	else {
		char s[20];

		itoa(*(int *)buf, s, 10);
		WritePrivateProfileString(key, name, s, path);
	}
	return 0;
#endif
}

int MoveRegistryTree(HKEY hroot, char *keyfrom, char *keyto)
{
	HKEY hkeyfrom, hkeyto;
	DWORD disposition;
	int i = 0;
	DWORD type, namelen, datalen;
	TCHAR name[100];
	BYTE data[500];
	TCHAR tkeyfrom[100], tkeyto[100];

	UTF8ToWindowsString(keyfrom, tkeyfrom);
	UTF8ToWindowsString(keyto, tkeyto);
	if(RegOpenKeyEx(hroot, tkeyfrom, 0, KEY_QUERY_VALUE, &hkeyfrom) != ERROR_SUCCESS)
		return -1;
	if(RegCreateKeyEx(hroot, tkeyto, 0, TEXT(""), 0, KEY_SET_VALUE, 0, &hkeyto, &disposition) != ERROR_SUCCESS)
	{
		RegCloseKey(hkeyfrom);
		return -1;
	}
	namelen = 100;
	datalen = 500;
	while(RegEnumValue(hkeyfrom, i, name, &namelen, 0, &type, data, &datalen) == ERROR_SUCCESS)
	{
		RegSetValueEx(hkeyto, name, 0, type, data, datalen);
		namelen = 100;
		datalen = 500;
		i++;
	}
	RegCloseKey(hkeyfrom);
	RegCloseKey(hkeyto);
	RegDeleteKey(hroot, tkeyfrom);
	return 0;
}

int DeleteSettingsKey(const char *key)
{
#ifdef SETTINGS_HKLM
	char key1[300];
	TCHAR tkey[300];

	sprintf(key1, "SOFTWARE\\%s", key);
	UTF8ToWindowsString(key1, tkey);
	RegDeleteKey(HKEY_LOCAL_MACHINE, tkey);
	return 0;
#endif
#ifdef SETTINGS_HKCU
	char key1[300];
	TCHAR tkey[300];

	sprintf(key1, "SOFTWARE\\%s", key);
	UTF8ToWindowsString(key1, tkey);
	RegDeleteKey(HKEY_CURRENT_USER, tkey);
	return 0;
#endif
#ifdef SETTINGS_INI
	char key1[300];
	static char path[MAX_PATH];
	char tmppath[MAX_PATH];
	FILE *fpsrc, *fpdst;
	char *p, s[500];
	bool copy = true;

	sprintf(key1, "[%s]\n", key);
	if(!*path)
	{
		GetModuleFileName(0, path, MAX_PATH);
		p = strrchr(path, '.');
		if(p)
			strcpy(p + 1, "ini");
	}
	if(!*path)
		return -1;
	strcpy(tmppath, path);
	strcat(tmppath, ".$$$");
	fpsrc = fopen(path, "r");
	if(!fpsrc)
		return -1;
	fpdst = fopen(tmppath, "w");
	if(!fpdst)
	{
		fclose(fpsrc);
		return -1;
	}
	while(fgets(s, 500, fpsrc))
	{
		if(!stricmp(s, key1))
			copy = false;
		else if(*s == '[')
			copy = true;
		if(copy)
			fputs(s, fpdst);
	}
	fclose(fpsrc);
	fclose(fpdst);
	MoveFileEx(tmppath, path, MOVEFILE_REPLACE_EXISTING);
	return 0;
#endif
}

int RenameSettingsKey(const char *src, const char *dest)
{
#ifdef SETTINGS_HKLM
	char keysrc[300], keydest[300];

	sprintf(keysrc, "SOFTWARE\\%s", src);
	sprintf(keydest, "SOFTWARE\\%s", dest);
	return MoveRegistryTree(HKEY_LOCAL_MACHINE, keysrc, keydest);
#endif
#ifdef SETTINGS_HKCU
	char keysrc[300], keydest[300];

	sprintf(keysrc, "SOFTWARE\\%s", src);
	sprintf(keydest, "SOFTWARE\\%s", dest);
	return MoveRegistryTree(HKEY_CURRENT_USER, keysrc, keydest);
#endif
#ifdef SETTINGS_INI
	char keysrc[300], keydest[300];
	static char path[MAX_PATH];
	char tmppath[MAX_PATH];
	FILE *fpsrc, *fpdst;
	char *p, s[500];

	DeleteSettingsKey(dest);
	sprintf(keysrc, "[%s]\n", src);
	sprintf(keydest, "[%s]\n", dest);
	if(!*path)
	{
		GetModuleFileName(0, path, MAX_PATH);
		p = strrchr(path, '.');
		if(p)
			strcpy(p + 1, "ini");
	}
	if(!*path)
		return -1;
	strcpy(tmppath, path);
	strcat(tmppath, ".$$$");
	fpsrc = fopen(path, "r");
	if(!fpsrc)
		return -1;
	fpdst = fopen(tmppath, "w");
	if(!fpdst)
	{
		fclose(fpsrc);
		return -1;
	}
	while(fgets(s, 500, fpsrc))
	{
		if(!stricmp(s, keysrc))
			fputs(keydest, fpdst);
		else fputs(s, fpdst);
	}
	fclose(fpsrc);
	fclose(fpdst);
	MoveFileEx(tmppath, path, MOVEFILE_REPLACE_EXISTING);
	return 0;
#endif
}

void excprintf(char *fmt, ...)
{
	va_list ap;
	char s[2000];
	FILE *fp;

	va_start(ap, fmt);
	vsprintf(s, fmt, ap);
	va_end(ap);
	fp = fopen("C:\\Crash.log", "a+");
	if(fp)
	{
		fprintf(fp, "%s\n", s);
		fclose(fp);
	}
}

#ifndef _WIN32_WCE
static long exceptionfilterroutine(struct _EXCEPTION_POINTERS *exceptioninfo)
{
	char s[300];
	int i, j;
	DWORD buffer[8];
	static int alreadyrun;
	HINSTANCE hih;
	SYSTEMTIME t;

	if(alreadyrun)
		return EXCEPTION_CONTINUE_SEARCH;
	GetSystemTime(&t);
	alreadyrun = 1;
	GetModuleFileName(GetModuleHandle(0), s, 300);
	excprintf("%s crashed", s);
	excprintf("Time %d/%d/%d %d:%02d:%02d.%03d", t.wDay, t.wMonth, t.wYear, t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);
	excprintf("Exception 0x%x", exceptioninfo->ExceptionRecord->ExceptionCode);
	excprintf("Flags %x", exceptioninfo->ExceptionRecord->ExceptionFlags);
	excprintf("Address 0x%x", exceptioninfo->ExceptionRecord->ExceptionAddress);
	if((DWORD)exceptioninfo->ExceptionRecord->ExceptionAddress >= 0x10000000)
	{
		hih = LoadLibrary("imagehlp.dll");
		if(hih)
		{
			BOOL (WINAPI *SymInitialize1)(HANDLE, LPSTR, BOOL);
			BOOL (WINAPI *SymGetModuleInfo1)(HANDLE, DWORD, PIMAGEHLP_MODULE);
			BOOL (WINAPI *SymGetSymFromAddr1)(HANDLE, DWORD, PDWORD, PIMAGEHLP_SYMBOL);
			IMAGEHLP_MODULE mi;
			union {
				IMAGEHLP_SYMBOL is;
				char fill[sizeof(IMAGE_SYMBOL) + 256];
			} is;
			HANDLE hProcess;

			SymInitialize1 = (BOOL (WINAPI *)(HANDLE, LPSTR, BOOL))GetProcAddress(hih, "SymInitialize");
			SymGetModuleInfo1 = (BOOL (WINAPI *)(HANDLE, DWORD, PIMAGEHLP_MODULE))GetProcAddress(hih, "SymGetModuleInfo");
			SymGetSymFromAddr1 = (BOOL (WINAPI *)(HANDLE, DWORD, PDWORD, PIMAGEHLP_SYMBOL))GetProcAddress(hih, "SymGetSymFromAddr");
			if(SymInitialize1 && SymGetModuleInfo1 && SymGetSymFromAddr1)
			{	
				SymInitialize1(hProcess = GetCurrentProcess(), 0, TRUE);
				mi.SizeOfStruct = sizeof(mi);
				*mi.ImageName = 0;
				SymGetModuleInfo1(hProcess, (DWORD)exceptioninfo->ExceptionRecord->ExceptionAddress, &mi);
				excprintf("Module %s", mi.ImageName);
				is.is.SizeOfStruct = sizeof(IMAGE_SYMBOL);
				is.is.MaxNameLength = 256;
				*is.is.Name = 0;
				SymGetSymFromAddr1(hProcess, (DWORD)exceptioninfo->ExceptionRecord->ExceptionAddress, 0, &is.is);
				excprintf("Function %s", is.is.Name);
			}
		}
	}
	if(exceptioninfo->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION &&
		exceptioninfo->ExceptionRecord->NumberParameters)
		if(exceptioninfo->ExceptionRecord->ExceptionInformation)
			excprintf("Access violation - write");
		else excprintf("Access violation - read");
	if(exceptioninfo->ContextRecord->ContextFlags & CONTEXT_INTEGER)
		excprintf("EAX = 0x%08x EBX = 0x%08x ECX = 0x%08x EDX = 0x%08x ESI = 0x%08x EDI = 0x%08x",
			exceptioninfo->ContextRecord->Eax,
			exceptioninfo->ContextRecord->Ebx,
			exceptioninfo->ContextRecord->Ecx,
			exceptioninfo->ContextRecord->Edx,
			exceptioninfo->ContextRecord->Esi,
			exceptioninfo->ContextRecord->Edi);
	if(exceptioninfo->ContextRecord->ContextFlags & CONTEXT_CONTROL)
	{
		excprintf("EBP = 0x%08x ESP = 0x%08x EIP = 0x%08x EFLAGS = 0x%08x",
			exceptioninfo->ContextRecord->Ebp,
			exceptioninfo->ContextRecord->Esp,
			exceptioninfo->ContextRecord->Eip,
			exceptioninfo->ContextRecord->EFlags);
		for(j = 0; j < 128; j++)
		if(ReadProcessMemory(GetCurrentProcess(), (LPCVOID)(exceptioninfo->ContextRecord->Esp + j * 32),
			buffer, 32, 0))
		{
			for(i = 0; i < 8; i++)
				sprintf(s + 9 * i, "%08x ", buffer[i]);
			excprintf("Stack dump %03x %s", j * 32, s);
		} else break;
	}
/*	GetThreadName(s);
	excprintf("Thread: %s", s);
	sprintf(text + strlen(text), "\nThread: %s", s);*/
	return EXCEPTION_EXECUTE_HANDLER;
}

void PerformExceptionHandling()
{
	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)&exceptionfilterroutine);
}
#endif

int VPInit()
{
	static int init;

	if(init)
		return 0;
	init = 1;
#ifdef WIN32
	WSADATA wsa;

	CoInitialize(0);
	WSAStartup(MAKEWORD(1,1), &wsa);
#endif
#ifdef LAZYLOG
	InitLazyLog();
#endif
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

#ifndef PORTABILITY_H_INCLUDED
#define PORTABILITY_H_INCLUDED

#ifdef _WIN32
#include "windows/wportability.h"
#else
#include "unix/uportability.h"
#endif

char *StartDirPtr();
int LoadSettings(const char *key, const char *name, void *buf, int type);
int SaveSettings(const char *key, const char *name, void *buf, int type);
int RenameSettingsKey(const char *src, const char *dest);
int DeleteSettingsKey(const char *key);
void RandomString128(char *s);
void RandomData(BYTE *data, int len);
void InterlockedInit();
void strcpypath(TCHAR *dest, const char *src);
unsigned filelengthpath(const char *path);
unsigned long FindLocalHostAddr(char *localhostsaddr, int autolocalhostaddr);

int topenfile(const TCHAR *path, int mode);
int openfile(const char *path, int mode);
int closefile(int f);
int readfile(int f, void *buf, int len);
int writefile(int f, const void *buf, int len);
int seekfile(int f, long offset, int origin);
int getfilesize(int f);
filetime getfilewritetime(const char *path);
int deletefile(const char *path);
int renamefile(const char *from, const char *to);
int createdirectory(const char *path);

inline int SaveSettings(const char *key, const char *name, const char *buf)
{ return SaveSettings(key, name, (void *)buf, REG_SZ); }
inline int SaveSettings(const char *key, const char *name, int value)
{ return SaveSettings(key, name, &value, REG_DWORD); }
inline int SaveSettings(const char *key, const char *name, unsigned value)
{ return SaveSettings(key, name, &value, REG_DWORD); }
inline int SaveSettings(const char *key, const char *name, DWORD value)
{ return SaveSettings(key, name, &value, REG_DWORD); }
inline int LoadSettings(const char *key, const char *name, char *buf)
{ return LoadSettings(key, name, buf, REG_SZ); }
inline int LoadSettings(const char *key, const char *name, int *buf)
{ return LoadSettings(key, name, buf, REG_DWORD); }
inline int LoadSettings(const char *key, const char *name, unsigned *buf)
{ return LoadSettings(key, name, buf, REG_DWORD); }
inline int LoadSettings(const char *key, const char *name, DWORD *buf)
{ return LoadSettings(key, name, buf, REG_DWORD); }

int UTF8ToWindowsString(const char *s, TCHAR *ts);
int WindowsStringToUTF8(const TCHAR *ts, char *s);

#endif

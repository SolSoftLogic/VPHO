#ifndef __UTIL_INCLUDED
#define __UTIL_INCLUDED

#include <time.h>
#include "portability.h"

#define Zero(a) memset(&(a), 0, sizeof(a))

void strncpy2(char *dest, const char *src, int destsize);
void Lprintf(const char *fmt, ...);
void VPLprintf(const char *fmt, ...);
void LogAsIs(const char *text, int len);
int Resolve(const char *address1, struct sockaddr_in *addr, int defaultport);
void HexToBinary(char *hex, unsigned char *binary);
void BinaryToHex(unsigned char *binary, int len, char *hex);
int Fieldize(char *string, char **ptrs, int maxfields);
int Fieldize(char *string, char **ptrs, int maxfields, const char *separators);
void utimetotext(time_t tm, char *text);
int CreatePath(const char *path, bool pathisfile);
int DebugAppendFile(const char *path, const void *buf, int datalen);
void InitLazyLog();

extern int generatelog, debugmode;

enum {LOG_NONE, LOG_NORMAL, LOG_PACKETS, LOG_MEDIAPACKETS, LOG_FULLMEDIAPACKETS};

class RWLOCK
{
public:
	RWLOCK();
	~RWLOCK();
	void RLock();
	void RUnlock();
	void WLock();
	void WUnlock();
protected:
	CRITICAL_SECTION m1, m2, m3;
	SHAREABLEMUTEX w, r;
	int readcount, writecount;
};

#endif

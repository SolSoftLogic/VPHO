//	Rate counter
#ifndef _NETUTIL_H_INCLUDED
#define _NETUTIL_H_INCLUDED

#include "vpstack.h"

#define XC_ALPHA 0.8f
#define XC_MULT (1.0f - XC_ALPHA) * 1000.0f / XC_INTERVAL
#define XC_INTERVAL 100
#define UDPIPHLEN 28
#define MAXBYTESOUT 5000
#define MAXLBTXQUEUE 400	// Maximum number of packets in tx queue

void RateCount(ratecounter *xc, int len);
int sendto2(SOCKET sk, const char *buf, int len, int flags, const sockaddr *to, int tolen);
int nbsendto(SOCKET sock, char *buf, int buflen, unsigned flags, struct sockaddr *addr, int addrlen);
int bindretry(SOCKET sk, const struct sockaddr *name, int namelen);
int InetAddrCmp(const struct sockaddr_in *addr1, const struct sockaddr_in *addr2);
int InetAddrIsPrivate(unsigned addr);

struct RESOLVER
{
	RESOLVER() { memset(this, 0, sizeof(RESOLVER)); }
	char address[100];
	int usename, stat, bc, offline, codec;
	unsigned supportedcodecs;
	struct sockaddr_in addr, privateaddr, publicaddr;
	unsigned token, waittimeout;
	char resolvedusername[MAXNAMELEN+1], resolvedvpnumber[MAXNUMBERLEN+1];
	bool active;
};

class LBTransmitter {
public:
	LBTransmitter();
	~LBTransmitter();
	int Send(const struct sockaddr_in *addr, const void *buf, int len);
	void Run();
	void Stop();
	void SendingThread(void *dummy);

	SOCKET sk;
	int maxkbps;
	unsigned backworkms;
	ratecounter xc;

protected:
	bool running, stop, transmitting;
	unsigned endtxtm;
	CRITICAL_SECTION cs;
	HANDLE hSendEvent;
	struct {
		void *buf;
		int len, prio;
		struct sockaddr_in addr;
	} queue[MAXLBTXQUEUE];
	int queuelen;
};

#endif

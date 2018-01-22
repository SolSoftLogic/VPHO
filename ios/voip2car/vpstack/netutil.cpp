#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "util.h"
#include "netutil.h"
#include "vpstack.h"

/**********************************************************************/
/* Transceiver functions                                              */
/**********************************************************************/

void RateCount(ratecounter *xc, int len)
{
	int t;

	t = GetTickCount();
	xc->bytes += len;
	if(!xc->lasttm)
		xc->lasttm = t;
	while(t > xc->lasttm)
	{
		xc->rate = (int)(xc->rate * XC_ALPHA) + xc->bytes;
		xc->lasttm += XC_INTERVAL;
		xc->bytes = 0;
	}
}

int sendto2(SOCKET sk, const char *buf, int len, int flags, const sockaddr *to, int tolen)
{
	if(generatelog >= 2 && (generatelog > 2 || (BYTE)*buf >= 0x80))
	{
		char s[5000], *p;
		int i, len2 = len;

		sprintf(s, "%u %s:%d Send %d bytes (", GetTickCount(), to ? inet_ntoa(((struct sockaddr_in *)to)->sin_addr) : "", to ? ntohs(((struct sockaddr_in *)to)->sin_port) : 0, len);
		p = s + strlen(s);
		if(generatelog == 3 && (BYTE)*buf < 0x80 && len > 20)
			len2 = 20;
		for(i = 0; i < len2; i++)
		{
			sprintf(p, i == len2 - 1 ? "%02x)" : "%02x,", ((BYTE *)buf)[i]);
			p += strlen(p);
		}
		Lprintf("%s", s);
	}
	return sendto(sk, buf, len, flags, to, tolen);
}

int nbsendto(SOCKET sock, char *buf, int buflen, unsigned flags, struct sockaddr *addr, int addrlen)
{
	fd_set fds;

	FD_ZERO(&fds);
	FD_SET(sock,&fds);
	if(select(sock+1, 0, &fds, 0, 0))
		return sendto2(sock, buf, buflen, 0, addr, addrlen);
	return SOCKET_ERROR;
}

LBTransmitter::LBTransmitter()
{ 
	running = stop = false;
	queuelen = 0;
	maxkbps = 0;
	Zero(xc);
	transmitting = false;
	InitializeCriticalSection(&cs);
	InitializeEvent(hSendEvent);
}

LBTransmitter::~LBTransmitter()
{
	Stop();
	DeleteCriticalSection(&cs);
	DeleteEvent(hSendEvent);
}

void LBTransmitter::Run()
{
	running = true;
	beginclassthread((ClassThreadStart)&LBTransmitter::SendingThread, this, 0);
}

void LBTransmitter::Stop()
{
	stop = true;
	SetEvent(hSendEvent);
	while(running)
		Sleep(10);
}

int LBTransmitter::Send(const struct sockaddr_in *addr, const void *buf, int len)
{
	int type = *(BYTE *)buf, prio, rc, i, totbytes = 0;

	if(*(BYTE *)buf == PKT_FT || stop || !maxkbps)
	{
		rc = nbsendto(sk, (char *)buf, len, 0, (struct sockaddr *)addr, sizeof(sockaddr_in));
		RateCount(&xc, len + 28);	// 28 is UDP+IP headers
		return rc;
	}
	if(type == PKT_VIDEO)
		prio = 2;
	else if(type < PKT_VIDEO)
		prio = 1;
	else prio = 0;
	EnterCriticalSection(&cs);
	if(queuelen < MAXLBTXQUEUE)
	{
		for(i = 0; i < queuelen && queue[i].prio <= prio; i++);
		memmove(&queue[i+1], &queue[i], sizeof(queue[0]) * (queuelen - i));
		queuelen++;
		queue[i].len = len;
		queue[i].addr = *addr;
		queue[i].prio = prio;
		queue[i].buf = malloc(len);
		memcpy(queue[i].buf, buf, len);
		rc = 0;
//		printf("%u %d (%d) queued to pos %d, prio=%d, queuelen=%d\n", GetTickCount(), *(BYTE *)buf, len, i, prio, queuelen);
	} else rc = SOCKET_ERROR;
	for(i = 0; i < queuelen && queue[i].prio <= 2; i++)
		totbytes += queue[i].len + 28;
	if(transmitting)
		backworkms = endtxtm + totbytes * 8 / maxkbps - GetTickCount();
	else backworkms = totbytes * 8 / maxkbps;
	if(backworkms > 0x80000000)
		backworkms = 0;
//	printf("%u backworkms=%u\n", GetTickCount(), backworkms);
	LeaveCriticalSection(&cs);
	SetEvent(hSendEvent);
	return rc;
}

void LBTransmitter::SendingThread(void *dummy)
{
	unsigned towait, tm, bytesout;

	EnterCriticalSection(&cs);
	while(!stop)
	{
		tm = GetTickCount();
		if(transmitting && tm - endtxtm < 0x80000000)
		{
			transmitting = false;
//			printf("%u Transmitting = false, endtxtm=%u\n", tm, endtxtm);
		}
		if(!transmitting)
			endtxtm = tm;
//		printf("%u Start while\n", GetTickCount());
		while(queuelen)
		{
			bytesout = (endtxtm - tm) * maxkbps / 8;
//			printf("%u Bytesout = %d\n", GetTickCount(), bytesout);
			if(bytesout + queue[0].len + 28 < MAXBYTESOUT)
			{
				nbsendto(sk, (char *)queue[0].buf, queue[0].len, 0, (struct sockaddr *)&queue[0].addr, sizeof(queue[0].addr));
				RateCount(&xc, queue[0].len + 28);	// 28 is UDP+IP headers
				endtxtm += (queue[0].len + 28) * 8 / maxkbps;
				queuelen--;
				transmitting = true;
//				printf("%u %d (%d) sent, endtxtm=%u, queuelen=%d\n", GetTickCount(), *(BYTE *)queue[0].buf, queue[0].len, endtxtm, queuelen);
				free(queue[0].buf);
				memmove(&queue[0], &queue[1], sizeof(queue[0]) * queuelen);
			} else break;
		}
		if(queuelen)
		{
			towait = endtxtm - tm - 10;
			if(towait >= 0x80000000 || towait < 10)
				towait = 10;
		} else towait = 1000;
//		printf("towait=%d\n", towait);
		WaitEvent(hSendEvent, &cs, towait);
	}
	LeaveCriticalSection(&cs);
	running = false;
}

int bindretry(SOCKET sk, const struct sockaddr *name, int namelen)
{
	int rc, i;
	struct sockaddr_in *addr = (struct sockaddr_in *)name;

	for(i = 1; i < 10; i++)
	{
		rc = bind(sk, name, namelen);
		if(!rc)
			return 0;
		Lprintf("bind on %s:%d failed, retrying after %d seconds", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port), i);
		Sleep(1000 * i);
	}
	return rc;
}

int InetAddrCmp(const struct sockaddr_in *addr1, const struct sockaddr_in *addr2)
{
	return addr1->sin_addr.s_addr == addr2->sin_addr.s_addr && addr1->sin_port == addr2->sin_port;
}

int InetAddrIsPrivate(unsigned addr)
{
	BYTE *a = (BYTE *)&addr;
	if(a[0] == 10 || a[0] == 192 && a[1] == 168 || a[0] == 172 && (a[1] & 0xf0) == 0x10)
		return 1;
	return 0;
}

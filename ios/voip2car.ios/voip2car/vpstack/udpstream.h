#ifndef _UDPSTREAM_H_INCLUDED
#define _UDPSTREAM_H_INCLUDED

#include "portability.h"

// Implements reliable and congestion controlled transmission of streams over UDP (like TCP over UDP)
class UDPStream {
public:
	UDPStream();
	~UDPStream();
	int Init(int (*send)(void *user, void *buf, int len), void *user1, int (*recv)(void *user, void *buf, int len), void *user2, int mtu1);
	int Send(void *buf, int len);
	void Abort();
	int udprecv(void *buf, int len);
	bool IsError() { return fatalerror; }
	void *senduser, *recvuser;

protected:
	void SendingThread();
	int SingleSend();
	void ReSend();
	static void SendingThread1(void *udpstream);
	int (*sendrtn)(void *user, void *buf, int len);
	int (*recvrtn)(void *user, void *buf, int len);
	int mtu, cwnd, rwnd, ssthresh, duplicated;
	char *txbuffer, *rxbuffer;
	bool canrun, running, fatalerror, busy;
	unsigned tx_bufferstart, tx_txpos, rx_expecting, buflen, tx_lastseq, tx_noacktm, rx_buflen, maxrtt;
	CRITICAL_SECTION cs;
	HANDLE hSEvent;
};

#endif

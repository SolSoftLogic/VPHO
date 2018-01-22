#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "portability.h"
#include "udpstream.h"
#include "util.h"

#ifdef _DEBUG
#define Dprintf printf
#else
#define Dprintf
#endif

#define UDPSTREAMBUF 20000
#define MAXWINDOW 8000
#define SENDTIMEOUT 10000

void UDPStream::SendingThread1(void *udpstream)
{
	((UDPStream *)udpstream)->SendingThread();
}

UDPStream::UDPStream()
{
	InitializeCriticalSection(&cs);
	InitializeEvent(hSEvent);
	rxbuffer = txbuffer = 0;
	buflen = 0;
	canrun = running = busy = false;
}

UDPStream::~UDPStream()
{
	canrun = false;
	while(running || busy)
		Sleep(10);
	if(txbuffer)
		free(txbuffer);
	if(rxbuffer)
		free(rxbuffer);
	DeleteCriticalSection(&cs);
	DeleteEvent(hSEvent);
}

int UDPStream::Init(int (*send)(void *user, void *buf, int len), void *user1, int (*recv)(void *user, void *buf, int len), void *user2, int mtu1)
{
	sendrtn = send;
	recvrtn = recv;
	senduser = user1;
	recvuser = user2;
	mtu = mtu1 - 8;
	buflen = rx_buflen = 0;
	tx_bufferstart = tx_txpos = rx_expecting = 0;
	tx_lastseq = 0xffffffff;
	tx_noacktm = 0;
	duplicated = 0;
	fatalerror = false;
	cwnd = 2 * mtu;
	rwnd = 2 * mtu;
	maxrtt = 250;
	ssthresh = UDPSTREAMBUF;
	return 0;
}

int UDPStream::Send(void *buf, int len)
{
	unsigned tmout = GetTickCount() + SENDTIMEOUT;

	Dprintf("Send %d", len);
	if(!canrun)
	{
		txbuffer = (char *)malloc(UDPSTREAMBUF);
		canrun = running = true;
		_beginthread(SendingThread1, 0, this);
	}
	EnterCriticalSection(&cs);
	for(;;)
	{
		if(fatalerror)
		{
			LeaveCriticalSection(&cs);
			return -2;
		}
		if(buf && buflen + len <= UDPSTREAMBUF)
		{
			memcpy(txbuffer + buflen, buf, len);
			buflen += len;
			Dprintf("ADDED buflen=%d", buflen);
			if(buflen == (unsigned)len)
			{
				cwnd = 2 * mtu;
				duplicated = 0;
				SingleSend();
				SingleSend();
			}
			LeaveCriticalSection(&cs);
			return 0;
		} else if(!buf)
		{
			if(!buflen)
			{
				LeaveCriticalSection(&cs);
				return 0;
			}
		} else if(GetTickCount() - tmout < 0x80000000)
		{
			LeaveCriticalSection(&cs);
			return -1;
		}
		WaitEvent(hSEvent, &cs, 1000);
	}
	LeaveCriticalSection(&cs);
}

int UDPStream::SingleSend()
{
	BYTE pkt[1500];
	int icwnd;

	icwnd = cwnd / mtu * mtu;
	if(tx_txpos < tx_bufferstart + buflen && !fatalerror
		&& tx_txpos < tx_bufferstart + icwnd
		&& tx_txpos < tx_bufferstart + rwnd
		&& tx_txpos < tx_bufferstart + MAXWINDOW/mtu*mtu)
	{
		if(tx_bufferstart + buflen - tx_txpos < (unsigned)mtu)
		{
			*(unsigned *)pkt = htonl(GetTickCount() & 0x7fffffff);
			((unsigned *)pkt)[1] = htonl(tx_txpos);
			memcpy(pkt + 8, txbuffer + tx_txpos - tx_bufferstart, tx_bufferstart + buflen - tx_txpos);
			sendrtn(senduser, pkt, tx_bufferstart + buflen - tx_txpos + 8);
			Dprintf("Tx %u, bufstart=%u, txpos=%u, buflen=%u, ts=%u", tx_bufferstart + buflen - tx_txpos,
				tx_bufferstart, tx_bufferstart + buflen, buflen, *(unsigned *)pkt);
			tx_txpos = tx_bufferstart + buflen;
		} else {
			*(unsigned *)pkt = htonl(GetTickCount() & 0x7fffffff);
			((unsigned *)pkt)[1] = htonl(tx_txpos);
			memcpy(pkt + 8, txbuffer + tx_txpos - tx_bufferstart, mtu);
			sendrtn(senduser, pkt, mtu + 8);
			tx_txpos += mtu;
			Dprintf("Tx %u, bufstart=%u, txpos=%u, buflen=%u, ts=%u", mtu, tx_bufferstart, tx_txpos, buflen, *(unsigned *)pkt);
		}
		tx_noacktm = GetTickCount() + maxrtt * 2;
		return 1;
	}
	return 0;
}

void UDPStream::ReSend()
{
	BYTE pkt[1500];
	int icwnd;

	icwnd = cwnd / mtu * mtu;
	if(buflen > 0 && !fatalerror)
	{
		if(buflen < (unsigned)mtu)
		{
			*(unsigned *)pkt = htonl(GetTickCount() & 0x7fffffff);
			((unsigned *)pkt)[1] = htonl(tx_bufferstart);
			memcpy(pkt + 8, txbuffer, buflen);
			sendrtn(senduser, pkt, tx_bufferstart + buflen - tx_txpos + 8);
			Dprintf("TxResend %u, bufstart=%u, txpos=%u, buflen=%u, ts=%u", tx_bufferstart + buflen - tx_txpos,
				tx_bufferstart, tx_bufferstart + buflen, buflen, *(unsigned *)pkt);
		} else {
/*			*(unsigned *)pkt = (tx_bufferstart + 4 * mtu) & 0x7fffffff;	// Another advance packet send for debugging
			memcpy(pkt + 4, txbuffer + 4 * mtu, mtu);
			sendrtn(senduser, pkt, mtu + 4);*/
			*(unsigned *)pkt = htonl(GetTickCount() & 0x7fffffff);
			((unsigned *)pkt)[1] = htonl(tx_bufferstart);
			memcpy(pkt + 8, txbuffer, mtu);
			sendrtn(senduser, pkt, mtu + 8);
			Dprintf("TxResend %u, bufstart=%u, txpos=%u, buflen=%u, ts=%u", mtu, tx_bufferstart, tx_txpos, buflen, *(unsigned *)pkt);
		}
		tx_noacktm = GetTickCount() + maxrtt * 2;
	}
}

void UDPStream::SendingThread()
{
	while(canrun)
	{
		Sleep(500);
		if(!canrun)
			break;
		EnterCriticalSection(&cs);
		if(buflen && tx_noacktm && (GetTickCount() - tx_noacktm) < 0x80000000)	// No ack timeout, restart
		{
			ssthresh = MAX(2 * mtu, MIN(cwnd, rwnd) / 2);
			cwnd = mtu;
			tx_txpos = tx_bufferstart;
			Dprintf("NOACK TIMEOUT, RESTART, txpos = %u", tx_txpos);
			SingleSend();
		}
		LeaveCriticalSection(&cs);
	}
	running = false;
}

int UDPStream::udprecv(void *buf, int len)
{
	unsigned seq, ts, rtt;

	busy = true;
	if(len < 8)
	{
		fatalerror = true;
		recvrtn(recvuser, 0, 0);
		busy = false;
		return -1;
	}
	ts = ntohl(*(unsigned *)buf & 0x7fffffff);
	seq = ntohl(((unsigned *)buf)[1]);
	if(ntohl(*(unsigned *)buf) & 0x80000000)
	{	// Acknowledge packet
		if(len < 12)
		{
			fatalerror = true;
			recvrtn(recvuser, 0, 0);
			busy = false;
			return -1;
		}
		rwnd = ntohl(((unsigned *)buf)[2]);
		rtt = GetTickCount() - ts;
		Dprintf("Rx ACK seq=%u, ts=%u, rwnd=%u, len=%d, rtt=%d", seq, ts, rwnd, len, rtt);
		EnterCriticalSection(&cs);
		if(rtt > maxrtt)
			maxrtt = rtt;
		if(tx_lastseq == seq)
		{
			if(!duplicated)
			{
				ssthresh = MAX(2 * mtu, MIN(cwnd, rwnd) / 2);
				cwnd = ssthresh;
				Dprintf("Doubleack first detected, tx_txpos = %d, cwnd = %d, ssthresh = %d", tx_txpos, cwnd, ssthresh);
				ReSend();
			} else Dprintf("Doubleack detected, tx_txpos = %d, cwnd = %d, ssthresh = %d", tx_txpos, cwnd, ssthresh);
			duplicated++;
			cwnd += mtu;
			while(SingleSend());
			LeaveCriticalSection(&cs);
		} else {
			tx_lastseq = seq;
			if(duplicated)
			{
				cwnd = ssthresh;
				duplicated = 0;
			} else {
				if(cwnd <= ssthresh)
					cwnd += mtu;
				else cwnd += mtu * mtu / cwnd;
			}
			if(cwnd > UDPSTREAMBUF)
				cwnd = UDPSTREAMBUF / mtu * mtu;
			Dprintf("New ack, bufferstart = %u, buflen = %u, cwnd = %u", tx_bufferstart, buflen, cwnd);
			if(seq > tx_bufferstart)
			{
				if(seq - tx_bufferstart <= buflen)
				{
					buflen -= seq - tx_bufferstart;
					memmove(txbuffer, txbuffer + seq - tx_bufferstart, buflen);
					tx_bufferstart = seq;
					Dprintf("Buffer shifted, bufferstart = %u, buflen = %u", tx_bufferstart, buflen);
				} else {
					fatalerror = true;
					recvrtn(recvuser, 0, 0);
				}
				SetEvent(hSEvent);
			}
			while(SingleSend());
			LeaveCriticalSection(&cs);
		}
	} else {
		// Data packet
		unsigned ackmsg[11], i, j;

		buf = (unsigned *)buf + 1;
		len -= 4;
		Dprintf("Rx DATA seq=%u, ts=%u, datalen=%d", seq, ts, len - 4);
/*		{
			static int err;
			if(seq > 28000 && err < 5)
			{
				err++;
				Dprintf("Simulated lost");
				busy = false;
				return 0;
			}
		}*/
		if(rx_expecting == seq)
		{
			// Process this packet
			Dprintf("Receive seq=%d, len=%d", rx_expecting, len - 4);
			recvrtn(recvuser, (char *)buf + 4, len - 4);
			rx_expecting += len - 4;

			// Check if there is other data in the buffer
			if(rxbuffer && rx_buflen && *(unsigned *)(rxbuffer + 4) == rx_expecting)
			{
				len = *(unsigned *)rxbuffer;
				Dprintf("Receive from buffer seq=%d, len=%d", rx_expecting, len - 4);
				recvrtn(recvuser, rxbuffer + 8, len - 4);
				rx_expecting += len - 4;
				rx_buflen -= len + 4;
				memmove(rxbuffer, rxbuffer + len + 4, rx_buflen);
			}
		} else if(rx_expecting < seq)
		{
			if(rx_buflen + 4 + len < UDPSTREAMBUF + 1000)
			{
				if(!rxbuffer)
					rxbuffer = (char *)malloc(UDPSTREAMBUF + 1000);
				for(i = 0; i < rx_buflen; i += 4 + *(unsigned *)(rxbuffer + i))
				{
					if(*(unsigned *)(rxbuffer + i + 4) > seq + len - 4)
					{
						Dprintf("Created new segment (%u, %u)", seq, len - 4);
						memmove(rxbuffer + i + len + 4, rxbuffer + i, rx_buflen - i);
						*(unsigned *)(rxbuffer + i) = len;
						memcpy(rxbuffer + i + 4, buf, len);
						rx_buflen += len + 4;
						break;
					} else if(*(unsigned *)(rxbuffer + i + 4) == seq + len - 4)
					{
						Dprintf("Put at the beginning of the segment (%u,%u)", *(unsigned *)(rxbuffer + i + 4), *(unsigned *)(rxbuffer + i) - 4);
						memmove(rxbuffer + i + 8 + len - 4, rxbuffer + i + 8, rx_buflen - (i + 8));
						memcpy(rxbuffer + i + 8, (char *)buf + 4, len - 4);
						*(unsigned *)(rxbuffer + i) += len - 4;
						*(unsigned *)(rxbuffer + i + 4) -= len - 4;
						rx_buflen += len - 4;
						break;
					} else if(*(unsigned *)(rxbuffer + i + 4) < seq + len - 4 &&
						*(unsigned *)(rxbuffer + i + 4) + *(unsigned *)(rxbuffer + i) - 4 > seq)
					{
						Dprintf("Packet already present in segment (%u,%u)", *(unsigned *)(rxbuffer + i + 4), *(unsigned *)(rxbuffer + i) - 4);
						break;
					} else if(*(unsigned *)(rxbuffer + i + 4) + *(unsigned *)(rxbuffer + i) - 4 == seq)
					{
						Dprintf("Put at the end of the segment (%u,%u)", *(unsigned *)(rxbuffer + i + 4), *(unsigned *)(rxbuffer + i) - 4);
						memmove(rxbuffer + i + 4 + *(unsigned *)(rxbuffer + i) + len - 4,
							rxbuffer + i + 4 + *(unsigned *)(rxbuffer + i),
							rx_buflen - (i + 4 + *(unsigned *)(rxbuffer + i)));
						memcpy(rxbuffer + i + *(unsigned *)(rxbuffer + i) + 4, (char *)buf + 4, len - 4);
						*(unsigned *)(rxbuffer + i) += len - 4;
						rx_buflen += len - 4;
						break;
					}
				}
				if(i == rx_buflen)
				{
					Dprintf("Created new segment (%u,%u)", seq, len - 4);
					*(unsigned *)(rxbuffer + rx_buflen) = len;
					memcpy(rxbuffer + rx_buflen + 4, buf, len);
					rx_buflen += len + 4;
				}
			} else Dprintf("Packet, no memory in receive buffer");
		}
		ackmsg[0] = htonl(ts | 0x80000000);
		ackmsg[1] = htonl(rx_expecting);
		ackmsg[2] = htonl(UDPSTREAMBUF);
		j = 3;
		for(i = 0; i < rx_buflen && j < 11; i += *(unsigned *)(rxbuffer + i) + 4)
		{
//			ackmsg[j++] = htonl(*(unsigned *)(rxbuffer + i + 4));
			ackmsg[j++] = *(unsigned *)(rxbuffer + i + 4);	// This is not the same as Vphonet, but correct according to the protocol specification; it's not used, so it's compatibility is not affected
			ackmsg[j++] = htonl(*(unsigned *)(rxbuffer + i) - 4);
		}
#ifdef _DEBUG
		{
			char s[500];

			*s = 0;
			for(i = 0; i < rx_buflen; i += *(unsigned *)(rxbuffer + i) + 4)
				sprintf(s + strlen(s), "(%u,%u)", *(unsigned *)(rxbuffer + i + 4), *(unsigned *)(rxbuffer + i) - 4);
			Dprintf("Tx ack rx_expecting = %u, segments in buffer=%s", rx_expecting, s);
		}
			
#endif
		sendrtn(senduser, ackmsg, 4 * j);
	}
	busy = false;
	return 0;
}

void UDPStream::Abort()
{
	fatalerror = true;
	recvrtn(recvuser, 0, 0);
}

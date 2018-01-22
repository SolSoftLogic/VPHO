#include <windows.h>
#include <stdio.h>
#include "../serialport.h"
#include "../util.h"

typedef struct {
	HANDLE h;
	char line[1000];
	int offset;
} PORTDATA;

int sp_debug;

void *sp_open(const char *port)
{
	HANDLE h;
	COMMTIMEOUTS ct;
	PORTDATA *d;

	h = CreateFile(port, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
	if(h == INVALID_HANDLE_VALUE)
		return 0;
	ct.ReadIntervalTimeout = 100;
	ct.ReadTotalTimeoutMultiplier = 0;
	ct.ReadTotalTimeoutConstant = 100;
	ct.WriteTotalTimeoutMultiplier = 0;
	ct.WriteTotalTimeoutConstant = 100;
	SetCommTimeouts(h, &ct);
	d = (PORTDATA *)malloc(sizeof *d);
	memset(d, 0, sizeof(*d));
	d->h = h;
	sp_setparam(d, 115200, 8, 1, PARITYNONE, FLOW_NONE);
	return d;
}

int sp_setparam(void *h, int bps, int bits, int stopbits, int parity, int flow)
{
	DCB dcb;

	memset(&dcb, 0, sizeof(dcb));
	dcb.DCBlength = sizeof(dcb);
	dcb.BaudRate = bps;
	dcb.fBinary = TRUE;
	dcb.fParity = parity ? TRUE : FALSE;
	dcb.fOutxCtsFlow = flow == FLOW_RTSCTS ? TRUE : FALSE;
	dcb.fOutxDsrFlow = FALSE;
	dcb.fDtrControl = DTR_CONTROL_ENABLE;
	dcb.fDsrSensitivity = FALSE;
	dcb.fTXContinueOnXoff = TRUE;
	dcb.fOutX = flow == FLOW_XONXOFF ? TRUE : FALSE;
	dcb.fInX = flow == FLOW_XONXOFF ? TRUE : FALSE;
	dcb.fErrorChar = FALSE;
	dcb.fNull = FALSE;
	dcb.fRtsControl = flow == FLOW_RTSCTS ? RTS_CONTROL_HANDSHAKE : RTS_CONTROL_ENABLE;
	dcb.fAbortOnError = FALSE;
	dcb.ByteSize = bits;
	if(flow == FLOW_XONXOFF)
	{
		dcb.XoffLim = 128;
		dcb.XonLim = 200;
		dcb.XonChar = 17;
		dcb.XoffChar = 19;
	}
	dcb.Parity = parity == PARITYODD ? ODDPARITY : parity == PARITYEVEN ? EVENPARITY : NOPARITY;
	dcb.StopBits = bits == 2 ? TWOSTOPBITS : ONESTOPBIT;
	if(SetCommState(((PORTDATA *)h)->h, &dcb))
		return 0;
	return -1;
}

int sp_close(void *h)
{
	PORTDATA *d = (PORTDATA *)h;
	CloseHandle(d->h);
	free(d);
	return 0;
}

int sp_writestring(void *h, const char *line)
{
	int n = strlen(line);
	DWORD bw;

	if(!WriteFile(((PORTDATA *)h)->h, line, n, &bw, 0) || bw != (unsigned)n)
		return -1;
	if(sp_debug)
		printf("W[%*.*s]\n", strlen(line)-1, strlen(line)-1, line);
	Lprintf("COM write [%*.*s]", strlen(line)-1, strlen(line)-1, line);
	return 0;
}

int sp_write(void *h, const void *buf, unsigned bufsize)
{
	DWORD bw;

	if(!WriteFile(((PORTDATA *)h)->h, buf, bufsize, &bw, 0) || bw != bufsize)
		return -1;
	if(sp_debug > 1)
		printf("W%d]", bufsize);
	if(generatelog >= 2)
		Lprintf("COM write %d bytes raw", bufsize);
	return 0;
}

int sp_read(void *h, void *buf, unsigned bufsize)
{
	DWORD br;

	if(!ReadFile(((PORTDATA *)h)->h, buf, bufsize, &br, 0))
		return -1;
	if(sp_debug > 1)
		printf("R%d]", br);
	if(generatelog >= 2)
		Lprintf("COM read %d bytes raw", br);
	return (int)br;
}

int sp_printf(void *h, const char *fmt, ...)
{
	va_list ap;
	char s[1000];

	va_start(ap, fmt);
	if(_vsnprintf(s, sizeof(s), fmt, ap) == -1)
	{
		va_end(ap);
		return -1;
	}
	return sp_writestring(h, s);
}

int sp_readline(void *h, char *line, int linesize)
{
	PORTDATA *d = (PORTDATA *)h;
	DWORD br;
	char *pcr, *plf;
	int n;

	for(;;)
	{
		if(!strcmp(d->line, "> ") && linesize >= 2)
		{
			*d->line = 0;
			d->offset = 0;
			strcpy(line, "> ");
			if(sp_debug)
				printf("R[> ]\n");
			Lprintf("COM read [> ]");
			return 2;
		}
		plf = strchr(d->line, '\n');
		pcr = strchr(d->line, '\r');
		if(plf || pcr)
		{
			// Find first end of line
			if(plf)
			{
				if(pcr && pcr < plf)
					plf = pcr;
			} else plf = pcr;
			if(plf > d->line)
			{
				// If line not empty, return it
				n = plf - d->line;
				if(n >= linesize)
					n = linesize - 1;
				memcpy(line, d->line, n);
				line[n] = 0;
				memmove(d->line, d->line + n, d->offset + 1 - n);
				d->offset -= n;
				// Now delete all the end of line characters
				plf = d->line;
				while(*plf == '\n' || *plf == '\r')
					plf++;
				memmove(d->line, plf, d->line + d->offset + 1 - plf);
				d->offset -= plf - d->line;
				if(sp_debug)
					printf("R[%s]\n", line);
				Lprintf("COM read [%s]", line);
				return n;
			} else {
				// Otherwise just delete all end of line characters
				while(*plf == '\n' || *plf == '\r')
					plf++;
				memmove(d->line, plf, d->line + d->offset + 1 - plf);
				d->offset -= plf - d->line;
			}
		}
		if(!ReadFile(d->h, d->line + d->offset, sizeof(d->line) - 1 - d->offset, &br, 0))
		{
			Sleep(10);	// In case of error, yield CPU for some time
			n = GetLastError();
			if(n == ERROR_OPERATION_ABORTED)
			{
				if(ClearCommError(d->h, &br, 0))
				{
					if(ReadFile(d->h, d->line + d->offset, sizeof(d->line) - 1 - d->offset, &br, 0))
						return -1;
				} else return -1;
			} else return -1;
		}
		if(br == 0)
			return 0;
		d->offset += br;
		d->line[d->offset] = 0;
	}
}

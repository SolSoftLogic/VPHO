#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <string.h>
#include <stdarg.h>
#include "../util.h"

typedef struct {
	int h;
	char line[1000];
	int offset;
} PORTDATA;

int sp_debug;

int sp_setparam(void *f, int bps, int bits, int stopbits, int parity, int flow)
{
	struct termios tio;
	
	memset(&tio, 0, sizeof tio);
	tio.c_cflag = CLOCAL | CREAD;
	switch(bits)
	{
	case 5: tio.c_cflag |= CS5; break;
	case 6: tio.c_cflag |= CS6; break;
	case 7: tio.c_cflag |= CS7; break;
	case 8: tio.c_cflag |= CS8; break;
	default: return -1;
	}
	switch(stopbits)
	{
	case 1: break;
	case 2: tio.c_cflag |= CSTOPB; break;
	default: return -1;
	}
	switch(parity)
	{
	case 0: break;
	case 1: tio.c_cflag |= PARENB | PARODD; tio.c_iflag |= INPCK; break;
	case 2: tio.c_cflag |= PARENB; tio.c_iflag |= INPCK; break;
	default: return -1;
	}
	switch(flow)
	{
	case 0: break;
	case 1: tio.c_cflag |= CRTSCTS; break;
	case 2: tio.c_iflag |= IXON | IXOFF | IXANY; tio.c_cc[VSTART] = 17; tio.c_cc[VSTOP] = 19; break;
	default: return -1;
	}
	switch(bps)
	{
	case 50: tio.c_cflag |= B50; break;
	case 75: tio.c_cflag |= B75; break;
	case 110: tio.c_cflag |= B110; break;
	case 134: tio.c_cflag |= B134; break;
	case 150: tio.c_cflag |= B150; break;
	case 200: tio.c_cflag |= B200; break;
	case 300: tio.c_cflag |= B300; break;
	case 600: tio.c_cflag |= B600; break;
	case 1200: tio.c_cflag |= B1200; break;
	case 1800: tio.c_cflag |= B1800; break;
	case 2400: tio.c_cflag |= B2400; break;
	case 4800: tio.c_cflag |= B4800; break;
	case 9600: tio.c_cflag |= B9600; break;
	case 19200: tio.c_cflag |= B19200; break;
	case 38400: tio.c_cflag |= B38400; break;
	case 57600: tio.c_cflag |= B57600; break;
	case 115200: tio.c_cflag |= B115200; break;
	case 230400: tio.c_cflag |= B230400; break;
	case 460800: tio.c_cflag |= B460800; break;
	case 576000: tio.c_cflag |= B576000; break;
	case 921600: tio.c_cflag |= B921600; break;
	case 1152000: tio.c_cflag |= B1152000; break;
	default: return -1;
	}
	tio.c_cc[0] = 0;
	tio.c_line = 0;
	if(tcsetattr(d->h, TCSANOW, &tio) < 0)
		return -1;
	return 0;
}

void *sp_open(const char *port)
{
	int f;
	PORTDATA *d;
	
	f = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
	if(f == -1)
		return 0;
	d = (PORTDATA *)malloc(sizeof *d);
	memset(d, 0, sizeof(*d));
	d->h = f;
	sp_setparam(d, 115200, 8, 1, 0, 0);
	return d;
}

static int tmread(int f, char *buffer, unsigned size)
{
	struct timeval tv;
	fd_set fs;
	int rc;

	FD_ZERO(&fs);
	FD_SET(f, &fs);
	tv.tv_usec = 100000;
	tv.tv_sec = 0;
	if(select(f+1, &fs, 0, 0, &tv) != 1)
	{
		close(f);
		if(generatelog >= 2)
			Lprintf("COM port select timeout");
		return 0;
	}
	rc = read(f, buffer, size);
	if(generatelog >= 2)
		Lprintf("COM port read, rc=%d", rc);
	return rc;
}

int sp_fileno(void *h)
{
	if(!h)
		return -1;
	PORTDATA *d = (PORTDATA *)h;
	return d->h;
}

int sp_close(void *h)
{
	if(!h)
		return -1;
	PORTDATA *d = (PORTDATA *)h;
	close(d->h);
	free(d);
	return 0;
}

int sp_write(void *h, const void *buf, unsigned bufsize)
{
	if(!h)
		return -1;
	if(sp_debug > 1)
		printf("W%d]", bufsize);
	if(generatelog >= 2)
		Lprintf("COM write %d bytes raw", bufsize);
	return write(((PORTDATA *)h)->h, buf, bufsize);
}

int sp_writestring(void *h, const char *line)
{
	if(!h)
		return -1;
	if(sp_debug)
		printf("W[%s]\n", line);
	Lprintf("COM write [%s]", line);
	return sp_write(h, line, strlen(line));
}

int sp_read(void *h, void *buf, unsigned bufsize)
{
	int rc;
	
	if(!h)
		return -1;
	rc = tmread(((PORTDATA *)h)->h, buf, bufsize);
	if(sp_debug > 1)
		printf("R%d]", rc);
	if(generatelog >= 2)
		Lprintf("COM read %d bytes raw", rc);
	return rc;
}

int sp_printf(void *h, const char *fmt, ...)
{
	va_list ap;
	char s[1000];

	va_start(ap, fmt);
	if(vsnprintf(s, sizeof(s), fmt, ap) == -1)
	{
		va_end(ap);
		return -1;
	}
	va_end(ap);
	return sp_writestring(h, s);
}

int sp_readline(void *h, char *line, int linesize)
{
	PORTDATA *d = (PORTDATA *)h;
	char *pcr, *plf;
	int n, rc;

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
		rc = tmread(((PORTDATA *)h)->h, d->line + d->offset, sizeof(d->line) - 1 - d->offset);
		if(rc <= 0)
			return rc;
		d->offset += rc;
		d->line[d->offset] = 0;
	}
}

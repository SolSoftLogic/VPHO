#ifndef _SERIALPORT_H_INCLUDED_
#define _SERIALPORT_H_INCLUDED_

extern int sp_debug;

void *sp_open(const char *port);
int sp_close(void *h);
int sp_fileno(void *h);
int sp_writestring(void *h, const char *line);
int sp_printf(void *h, const char *fmt, ...);
int sp_readline(void *h, char *line, int linesize);	// Should return -1 on fatal error, 0 on timeout, otherwise the number of characters in line
int sp_write(void *h, const void *buf, unsigned bufsize);
int sp_read(void *h, void *buf, unsigned bufsize);
int sp_setparam(void *f, int bps, int bits, int stopbits, int parity, int flow);

#define PARITYNONE 0
#define PARITYODD 1
#define PARITYEVEN 2

#define FLOW_NONE 0
#define FLOW_RTSCTS 1
#define FLOW_XONXOFF 2

#endif

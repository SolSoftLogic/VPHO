// cl /DHASMAIN decodepdu.c ../common.c wsock32.lib user32.lib advapi32.lib shell32.lib user32.lib gdi32.lib comdlg32.lib

#ifdef _WIN32
#include <windows.h>
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pdu.h"

char *statusmsg1[] = {"Message received by recipient", "Message sent to recipient, but delivery is unconfirmed", "Message replaced by switch"};
char *statusmsg2[] = {"Congestion, still trying", "Recipient busy, still trying", "No response from recipient, still trying", "Service rejected, still trying", "Quality of service not available, still trying", "Error in recipient, still trying"};
char *statusmsg3[] = {"Remote procedure error", "Incompatible destination", "Connection rejected by recipient", "Not obtainable",
	"Quality of service not available", "No interworking available", "Message validity period expired", "Message deleted by originating terminal",
	"Message deleted by administration", "Message does not exist"};
char *statusmsg4[] = {"Congestion", "Recipient busy", "No response from recipient", "Service rejected", "Quality of service not available", "Error in recipient"};

char *statusreport(int error)
{
	if(0 <= error && error <= 2)
		return statusmsg1[error];
	if(32 <= error && error <= 37)
		return statusmsg2[error - 32];
	if(64 <= error && error <= 73)
		return statusmsg3[error - 64];
	if(96 <= error && error <= 101)
		return statusmsg4[error - 96];
	return "Unknown or reserved error";
}

#ifdef HASMAIN
void main()
{
	char s[500];
	tsms sms;
	int rc;

//	decodepdu("040C9193434766621000001010508104030433D324881A74529FAEA334A84D1641455033C9641641C1EAB12A4D828A20617098040541D66712440C829CCFA40B", s);
//	decodepdu("000B913074031492F200001234567890123412C374F8CD02ADC3EB37485D06ADC3EA1F", s);
//	decodepdu("040B918346714633F40000103031019340001BCBA012342D8288CF635019044D41D4A0F0097206A9C16910", s);
//	encodepdu("+393473041292", "Ciao", 1, s);
//	printf("+CMGS=%d\n%s\032", strlen(s) / 2 - 1, s);
	gets(s);
	rc = decodepdu(s, &sms);
	if(!rc)
		printf("%s: %s\n", sms.snum, sms.text);
	else printf("rc=%d\n", rc);
}
#endif

static char gsmtoascii[129] =
"@£$¥éèùìòÇ\nØø\rÅå _          Ææ É"
" !\"# %&\'()*+,-./0123456789:;<=>?"
"¡ABCDEFGHIJKLMNOPQRSTUVWXYZÄÖÑÜ "
"¿abcdefghijklmnopqrstuvwxyzäöñüà";
static char asciitogsm[256];

/*
Encoding for deliver messages
1 byte: 2 message type, 1 more messages to send, 1 reply path exists, 1 UD header present, 1 status report request
1 byte: originating address length
1-11 bytes: originating address
1 byte: protocol identifier
1 byte: data coding scheme
7 bytes: service centre timestamp
1 byte: user data length
n bytes: user data

Encoding for submit messages
1 byte: 2 message type, 1 reject duplicates, 2 validity period present, 1 request reply path, 1 UD header present, 1 status report request
1 byte: message reference
1 byte: destination address length
1-11 bytes: destination address
1 byte: protocol identifier
1 byte: data coding scheme
1 or 7 bytes: validity
1 byte: user data length
n bytes: user data
*/

int Unicode2UTF8(const unsigned short *src, char *dst, unsigned dstsize)
{
#ifdef _WIN32
	WideCharToMultiByte(CP_UTF8, 0, src, wcslen(src), dst, dstsize, 0, 0);
	return 0;
#else
	unsigned n = 0;

	dstsize--;
	while(*src && n < dstsize)
	{
		if(*src <= 127)
			dst[n++] = (char)*src;
		else dst[n++] = '?';
		src++;
	}
	dst[n] = 0;
	return 0;
#endif
}

int decodepdu(const char *pdu, SMS *sms)
{
	unsigned char bpdu[300], *p;
	char *ap, isunicode;
	int i, c, bpdulen, tl;
	struct tm tm;

	memset(sms, 0, sizeof *sms);
	// Convert hex to raw
	for(i = 0; *pdu; i++)
	{
		if(*pdu >= '0' && *pdu <= '9')
			c = *pdu - '0';
		else if(*pdu >= 'A' && *pdu <= 'F')
			c = *pdu - 'A' + 10;
		else if(*pdu >= 'a' && *pdu <= 'f')
			c = *pdu - 'a' + 10;
		else break;
		c <<= 4;
		pdu++;
		if(!*pdu)
			return -1;
		if(*pdu >= '0' && *pdu <= '9')
			c |= *pdu - '0';
		else if(*pdu >= 'A' && *pdu <= 'F')
			c |= *pdu - 'A' + 10;
		else if(*pdu >= 'a' && *pdu <= 'f')
			c |= *pdu - 'a' + 10;
		else return -2;
		bpdu[i] = (unsigned char)c;
		pdu++;
	}

	bpdulen = i;
	p = bpdu;
	if((bpdu[0] & 3) == 2)
	{
		// Message type is STATUS REPORT

		if(bpdu[0] & 0x20)
			return -3;	// Status report is for command and not for submit
		p++;
		sms->report = 2;
	} else if((bpdu[0] & 3) == 1)
	{
		sms->submit = 1;
		p++;
	} else if(bpdu[0] & 3)
		return -4;	// Unknown type

	// Format output as e.g. (deliver)
	// "+38641325505",,"99/09/05,18:38:28+00"
	// Text

	// (status report)
	// "+38641325505",,"99/09/05,18:38:28+00","99/09/05,18:38:40+00",0

	sms->snum[0] = 0;
	if((p[2] & 0xd0) == 0xd0)
	{
		tl = p[1] * 4 / 7;
		if(tl > 19)
			tl = 19;
		for(i = 0; i < tl; i++)
			sms->snum[i] = gsmtoascii[((p[3 + i * 7 / 8] >> (8 - i % 8)) | (p[3 + (i * 7 + 7) / 8] << (i % 8))) & 0x7f];
		sms->snum[i] = 0;
	} else {
		if((p[2] & 0xf0) == 0x90)	// International
			strcat(sms->snum, "+");
		else if((p[2] & 0xf0) == 0xa0) // National
			strcat(sms->snum, "0");
		for(i = 0; i < (p[1] + 1) / 2; i++)
		{
			char twodigits[3];

			if((p[3 + i] & 0xf) <= 9)
				twodigits[0] = (p[3 + i] & 0xf) + '0';
			else twodigits[0] = 0;
			if((p[3 + i] & 0xf0) <= 0x90)
				twodigits[1] = (p[3 + i] >> 4) + '0';
			else twodigits[1] = 0;
			twodigits[2] = 0;
			strcat(sms->snum, twodigits);
		}
	}
	if(sms->report)
		p = bpdu + 2 + (p[1] + 1) / 2 + 2;	// Go to Service Centre Time Stamp (status report)
	else {
		p = bpdu + 3 + (p[1] + 1) / 2 + 2;	// Go to Service Centre Time Stamp (deliver or submit)
		if(sms->submit)
			p++;
		isunicode = p[-1] & 8;	// The byte before timestamp is data coding scheme
	}
	if(p >= bpdu + bpdulen)
		return -5;
	if(!sms->submit || (bpdu[0] & 0x18) == 0x18)
	{
		tm.tm_mday = (p[2] & 0xf) * 10 + (p[2] >> 4);
		tm.tm_mon = (p[1] & 0xf) * 10 + (p[1] >> 4) - 1;
		tm.tm_year = (p[0] & 0xf) * 10 + (p[0] >> 4) + 100;
		tm.tm_hour = (p[3] & 0xf) * 10 + (p[3] >> 4);
		tm.tm_min = (p[4] & 0xf) * 10 + (p[4] >> 4);
		tm.tm_sec = (p[5] & 0xf) * 10 + (p[5] >> 4);
		tm.tm_wday = 0;
		tm.tm_yday = 0;
		tm.tm_isdst = 0;
		sms->time = mktime(&tm);
		p += 7;	// Go to User Data Length
	} else if(sms->submit)
	{
		sms->time = 0;
		if((bpdu[0] & 0x18) == 0x10)
			p++;
		else if((bpdu[0] & 0x18) == 0x08)
			p += 7;
	}
	if(p >= bpdu + bpdulen)
		return -6;
	if(sms->report)
	{
		if(p + 5 >= bpdu + bpdulen)
			return -7;
		tm.tm_mday = (p[2] & 0xf) * 10 + (p[2] >> 4);
		tm.tm_mon = (p[1] & 0xf) * 10 + (p[1] >> 4) - 1;
		tm.tm_year = (p[0] & 0xf) * 10 + (p[0] >> 4) + 100;
		tm.tm_hour = (p[3] & 0xf) * 10 + (p[3] >> 4);
		tm.tm_min = (p[4] & 0xf) * 10 + (p[4] >> 4);
		tm.tm_sec = (p[5] & 0xf) * 10 + (p[5] >> 4);
		tm.tm_wday = 0;
		tm.tm_yday = 0;
		tm.tm_isdst = 0;
		sms->deliverytime = mktime(&tm);
		return 0;
	} else {
		int skip, start;

		ap = sms->text;
		if(bpdu[0] & 0x40)
		{
			if(p[1] == 5 && p[2] == 0 && p[3] == 3)
			{
				ap[0] = '(';
				ap[1] = '0' + p[6];
				ap[2] = '/';
				ap[3] = '0' + p[5];
				ap[4] = ')';
				ap[5] = ' ';
				skip = 6;
			} else skip = 0;
			if(isunicode)
			{
				tl = p[0] - p[1] - 1;
				p += 1 + p[1];
				if(skip)
					p++;
			} else {
				tl = p[0];
				start = (((1 + p[1]) * 8 + 6) / 7 * 7 + 7) / 8;
				skip -= ((1 + p[1]) * 8 + 6) / 7;
				p++;
			}
		} else {
			tl = p[0];
			skip = start = 0;
			p++;
		}

		// Decode septets sequence to octets sequence
		if(isunicode)
		{
			unsigned short text[71];

			if(tl > 70)
				tl = 70;
			for(i = 0; i < tl; i++)
				text[i] = (p[i*2]<<8) + p[i*2+1];
			text[tl] = 0;
			Unicode2UTF8(text, ap+skip, 161-skip);
		} else {
			for(i = start; i < tl; i++)
				ap[i+skip] = gsmtoascii[((p[i * 7 / 8] >> (8 - i % 8)) | (p[(i * 7 + 7) / 8] << (i % 8))) & 0x7f];
		}
		if(p > bpdu + bpdulen)
			return -8;
		ap[i] = 0;
		return 0;
	}
}

static void createasciitogsm()
{
	int i;

	memset(asciitogsm, 32, 256);
	for(i = 0; i < 128; i++)
		asciitogsm[(unsigned char)gsmtoascii[i]] = i;
	asciitogsm[32] = 32;
}

int encodepdu(const char *number, const char *text, int report, char *pdu)
{
	int i, len;

	if(!asciitogsm[13])
		createasciitogsm();
	if(*number == '+')
	{
		number++;
		sprintf(pdu, "00%c100%0.2X91", report ? 'B' : '9', strlen(number));
	} else if(*number == '0' && number[1] == '0')
	{
		number += 2;
		sprintf(pdu, "00%c100%0.2X91", report ? 'B' : '9', strlen(number));
	} else {
		sprintf(pdu, "00%c100%0.2X81", report ? 'B' : '9', strlen(number));
	}
	pdu += strlen(pdu);
	while(*number)
	{
		if(number[1])
			*pdu++ = number[1];
		else *pdu++ = 'F';
		*pdu++ = *number;
		*pdu = 0;
		if(number[1])
			number += 2;
		else break;
	}
	sprintf(pdu, "0000FF%0.2X", len = strlen(text));
	pdu += 8;
	len = (len * 7 + 7) / 8;
	for(i = 0; i < len; i++)
	{
		sprintf(pdu, "%0.2X", (asciitogsm[text[i * 8 / 7]] >> (i % 7)) | ((asciitogsm[text[i * 8 / 7 + 1]] << (7 - i % 7))) & 0xff);
		pdu += 2;
	}
	*pdu = 0;
	return 0;
}

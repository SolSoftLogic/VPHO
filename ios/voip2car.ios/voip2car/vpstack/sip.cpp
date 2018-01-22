#include <stdio.h>
#include <time.h>
#include "portability.h"
#include "vpstack.h"
#include "ielements.h"
#include "sip.h"
#include "md5.h"

//#define LOCKDEBUGGING
void DebugCurrentOp(const char *label);
#define debugstring(a) DebugCurrentOp(a)

#define Lock() LockL(__LINE__)
#define Unlock() UnlockL(__LINE__)
#define ReadLock() ReadLockL(__LINE__)
#define ReadUnlock() ReadUnlockL(__LINE__)
#define WriteLock() WriteLockL(__LINE__)
#define WriteUnlock() WriteUnlockL(__LINE__)
#define LockCall(a) LockCallL(a, __LINE__)

#define ISSPACE(a) ((a) == ' ' || (a) == '\t')

#define SIPPORT 5060

#define IEBUFSIZE 1500
#define SIPCMD_INVITE PKT_CONNECTREQ
#define SIPCMD_BYE PKT_DISCONNECTREQ
#define SIPCMD_REGISTER	PKT_NOTIFY
#define SIPCMD_INVITEACK PKT_CONNECTACK
#define SIPCMD_ACK_INVITE PKT_CONNECTACK_RECEIVED
#define SIPCMD_ACK_REGISTER 0xc2
#define SIPCMD_BYEACK PKT_DISCONNECTACK
#define SIPCMD_REGISTERACK PKT_NOTIFYACK
#define SIPCMD_CANCEL	0xc0
#define SIPCMD_CANCELACK 0xc1
#define SIPCMD_GENACK (-1)
#define SIPCMD_INFO PKT_INFO
#define SIPCMD_INFOACK PKT_GENERALACK
#define SIPCMD_OPTIONS 0xc3
#define SIPCMD_OPTIONSACK 0xc4
#define SIPCMD_UNKNOWN 0xc5
#define SIPCMD_UNKNOWNMETHOD 0xc6
#define SIPCMD_REFER 0xc7
#define SIPCMD_REFERACK 0xc8

#define TSTART	InterlockedIncrement(&nthreads);
#define TRETURN	{InterlockedDecrement(&nthreads); return; }

void MD5(const char *s, char *digest)
{
	MD5_CTX ctx;
	BYTE bdigest[16];

	MD5Init(&ctx);
	MD5Update(&ctx, (unsigned char *)s, strlen(s));
	MD5Final(bdigest, &ctx);
	BinaryToHex(bdigest, 16, digest);
}

int Format(char pdu[1500], SIP_ELEMENTS *ie)
{
	char sdp[1500], *p;
	int i;

	if(ie->command == SIPCMD_INVITE || ie->command == SIPCMD_INVITEACK && ie->answer == ANSWER_ACCEPT)
	{
		BYTE *addr;

		// Format SDP
		strcpy(sdp, "v=0\r\n");
		p = sdp + strlen(sdp);
		if(ie->command == SIPCMD_INVITE)
			sprintf(p, "o=%s %u %u IN IP4 %d.%d.%d.%d\r\n", ie->fromname && *ie->fromname && !strchr(ie->fromname, ' ') ? ie->fromname : "-", ie->sessionid, ie->sessionversion, ie->fulllocaladdr[0], ie->fulllocaladdr[1], ie->fulllocaladdr[2], ie->fulllocaladdr[3]);
		else sprintf(p, "o=%s %u %u IN IP4 %d.%d.%d.%d\r\n", ie->toname && *ie->toname && !strchr(ie->toname, ' ')  ? ie->toname : "-", ie->sessionid, ie->sessionversion, ie->fulllocaladdr[0], ie->fulllocaladdr[1], ie->fulllocaladdr[2], ie->fulllocaladdr[3]);
		p += strlen(p);
		strcpy(p, "s=VPStack\r\n");
		p += strlen(p);
		addr = (BYTE *)&ie->rtpaddr.sin_addr;
		if(ie->hold)
			sprintf(p, "c=IN IP4 0.0.0.0\r\n");
		else sprintf(p, "c=IN IP4 %d.%d.%d.%d\r\n", addr[0], addr[1], addr[2], addr[3]);
		p += strlen(p);
		strcpy(p, "t=0 0\r\n");
		p += strlen(p);
		//strcpy(p, "a=direction:active\r\n");
		//p += strlen(p);
		if(ie->command == SIPCMD_INVITE)
		{
			sprintf(p, "m=audio %d RTP/AVP", ntohs(ie->rtpaddr.sin_port));
			if(ie->bc_codecs & (1<<CODEC_ILBC30))
				strcat(p, " 97");
			if(ie->bc_codecs & (1<<CODEC_ILBC20))
				strcat(p, " 98");
			if(ie->bc_codecs & (1<<CODEC_G729))
				strcat(p, " 18");
			if(ie->bc_codecs & (1<<CODEC_GSMFR))
				strcat(p, " 3");
			if(ie->bc_codecs & (1<<CODEC_G711A))
				strcat(p, " 8");
			if(ie->bc_codecs & (1<<CODEC_G711U))
				strcat(p, " 0");
			sprintf(p + strlen(p), " %d\r\n", ie->keypadmap);
			p += strlen(p);
			if(ie->bc_codecs & (1<<CODEC_G729))
				strcat(p, "a=rtpmap:18 g729/8000\r\n");
			if(ie->bc_codecs & (1<<CODEC_GSMFR))
				strcat(p, "a=rtpmap:3 gsm/8000\r\n");
			if(ie->bc_codecs & (1<<CODEC_G711A))
				strcat(p, "a=rtpmap:8 pcma/8000\r\n");
			if(ie->bc_codecs & (1<<CODEC_G711U))
				strcat(p, "a=rtpmap:0 pcmu/8000\r\n");
			if(ie->bc_codecs & (1<<CODEC_ILBC30))
				strcat(p, "a=rtpmap:97 iLBC/8000\r\na=fmtp:97 mode=30\r\n");
			if(ie->bc_codecs & (1<<CODEC_ILBC20))
				strcat(p, "a=rtpmap:98 iLBC/8000\r\na=fmtp:98 mode=20\r\n");
			p = p + strlen(p);
		} else {
			sprintf(p, "m=audio %d RTP/AVP", ntohs(ie->rtpaddr.sin_port));
			if(ie->codec == CODEC_GSMFR)
				sprintf(p + strlen(p), " 3 %d\r\na=rtpmap:3 gsm/8000\r\n", ie->keypadmap);
			else if(ie->codec == CODEC_G711A)
				sprintf(p + strlen(p), " 8 %d\r\na=rtpmap:8 pcma/8000\r\n", ie->keypadmap);
			else if(ie->codec == CODEC_G711U)
				sprintf(p + strlen(p), " 0 %d\r\na=rtpmap:0 pcmu/8000\r\n", ie->keypadmap);
			else if(ie->codec == CODEC_G729)
				sprintf(p + strlen(p), " 18 %d\r\na=rtpmap:18 g729/8000\r\n", ie->keypadmap);
			else if(ie->codec == CODEC_ILBC20)
				sprintf(p + strlen(p), " %d %d\r\na=rtpmap:%d iLBC/8000\r\na=fmtp:%d mode=20\r\n",
					ie->keypadmap, ie->iLBC20map, ie->iLBC20map, ie->iLBC20map);
			else if(ie->codec == CODEC_ILBC30)
				sprintf(p + strlen(p), " %d %d\r\na=rtpmap:%d iLBC/8000\r\na=fmtp:%d mode=30\r\n",
					ie->keypadmap, ie->iLBC30map, ie->iLBC30map, ie->iLBC30map);
			else sprintf(p + strlen(p), " %d\r\n", ie->keypadmap);
			p = p + strlen(p);
		}
		sprintf(p, "a=rtpmap:%d telephone-event/8000\r\n"
					"a=fmtp:%d 0-16\r\n", ie->keypadmap, ie->keypadmap);
		p += strlen(p);
		if(ie->hold == HOLD_SENDONLY)
			sprintf(p, "a=sendonly\r\n");
		else if(ie->hold == HOLD_RECVONLY)
			sprintf(p, "a=recvonly\r\n");
		else if(ie->hold == HOLD_INACTIVE)
			sprintf(p, "a=inactive\r\n");
	} else *sdp = 0;

	switch(ie->command)
	{
	case SIPCMD_INVITE:
		sprintf(pdu, "INVITE sip:%s SIP/2.0\r\n",  ie->destaddr);
		break;
	case SIPCMD_REFER:
		sprintf(pdu, "REFER sip:%s SIP/2.0\r\n",  ie->destaddr);
		break;
	case SIPCMD_OPTIONSACK:
		strcpy(pdu, "SIP/2.0 200 OK\r\n");
		break;
	case SIPCMD_INVITEACK:
		if(ie->answer == ANSWER_ACCEPT)
		{
			strcpy(pdu, "SIP/2.0 200 OK\r\n");
		} else if(ie->answer == ANSWER_USERALERTED)
			strcpy(pdu, ie->reason == REASON_CALLPROCEEDING ? "SIP/2.0 100 Trying\r\n" : "SIP/2.0 180 Ringing\r\n");
		else if(ie->reason == REASON_AUTHERROR)
			strcpy(pdu, "SIP/2.0 407 Proxy Authentication Required\r\n");
		else if(ie->reason == REASON_NORMAL)
			strcpy(pdu, "SIP/2.0 486 Busy here\r\n");
		else strcpy(pdu, "SIP/2.0 403 Forbidden\r\n");
		break;
	case SIPCMD_REFERACK:
		if(ie->answer == ANSWER_ACCEPT)
			strcpy(pdu, "SIP/2.0 202 Accepted\r\n");
		else strcpy(pdu, "SIP/2.0 403 Forbidden\r\n");
		break;
	case SIPCMD_ACK_INVITE:
	case SIPCMD_ACK_REGISTER:
		sprintf(pdu, "ACK sip:%s SIP/2.0\r\n", ie->destaddr);
		break;
	case SIPCMD_REGISTER:
		sprintf(pdu, "REGISTER sip:%s SIP/2.0\r\n", ie->destaddr);
		break;
	case SIPCMD_REGISTERACK:
		if(ie->answer == ANSWER_ACCEPT)
			strcpy(pdu, "SIP/2.0 200 OK\r\n");
		else if(ie->reason == REASON_AUTHERROR)
			strcpy(pdu, "SIP/2.0 407 Proxy Authentication Required\r\n");
		else strcpy(pdu, "SIP/2.0 403 Forbidden\r\n");
		break;
	case SIPCMD_BYE:
	case SIPCMD_CANCEL:
		if(ie->command == SIPCMD_BYE)
			sprintf(pdu, "BYE sip:%s SIP/2.0\r\n", ie->destaddr);
		else sprintf(pdu, "CANCEL sip:%s SIP/2.0\r\n", ie->destaddr);
		break;
	case SIPCMD_BYEACK:
	case SIPCMD_CANCELACK:
	case SIPCMD_INFOACK:
		strcpy(pdu, "SIP/2.0 200 OK\r\n");
		break;
	case SIPCMD_UNKNOWNMETHOD:
		strcpy(pdu, "SIP/2.0 501 Not Implemented\r\n");
		break;
	}
	p = pdu + strlen(pdu);
	sprintf(p, "Content-Length: %d\r\n", strlen(sdp));
	p += strlen(p);
	if(ie->expire)
		sprintf(p, "Contact: <sip:%s>;expires=%d\r\n", ie->contact, ie->expire);
	else sprintf(p, "Contact: <sip:%s>\r\n", ie->contact);
	p += strlen(p);
	sprintf(p, "Call-ID: %s\r\n", ie->callid);
	p += strlen(p);
	sprintf(p, "Allow: INVITE,ACK,CANCEL,BYE,INFO\r\n");
	p += strlen(p);
	if(*sdp)
	{
		strcpy(p, "Content-Type: application/sdp\r\n");
		p += strlen(p);
	}
	if(ie->command == SIPCMD_REGISTER || ie->command == SIPCMD_REGISTERACK || ie->command == SIPCMD_ACK_REGISTER)
		sprintf(p, "CSeq: %u REGISTER\r\n", ie->seqnumber);
	else if(ie->command == SIPCMD_OPTIONSACK)
		sprintf(p, "CSeq: %u OPTIONS\r\n", ie->seqnumber);
	else if(ie->command == SIPCMD_INVITE || ie->command == SIPCMD_INVITEACK)
		sprintf(p, "CSeq: %u INVITE\r\n", ie->seqnumber);
	else if(ie->command == SIPCMD_BYEACK || ie->command == SIPCMD_BYE)
		sprintf(p, "CSeq: %u BYE\r\n", ie->seqnumber);
	else if(ie->command == SIPCMD_REFER || ie->command == SIPCMD_REFERACK)
		sprintf(p, "CSeq: %u REFER\r\n", ie->seqnumber);
	else if(ie->command == SIPCMD_CANCELACK || ie->command == SIPCMD_CANCEL)
		sprintf(p, "CSeq: %u CANCEL\r\n", ie->seqnumber);
	else if(ie->command == SIPCMD_ACK_INVITE)
		sprintf(p, "CSeq: %u ACK\r\n", ie->seqnumber);
	else if(ie->command == SIPCMD_INFOACK)
		sprintf(p, "CSeq: %u INFO\r\n", ie->seqnumber);
	p += strlen(p);
	if(ie->command != SIPCMD_INVITEACK && ie->command != SIPCMD_REGISTERACK && ie->command != SIPCMD_BYEACK && ie->command != SIPCMD_CANCELACK)
	{
		strcpy(p, "Max-Forwards: 70\r\n");
		p += strlen(p);
	}
	for(i = 0; i < MAXVIA; i++)
		if(ie->route[i])
		{
			if(ie->command == SIPCMD_INVITEACK)
				sprintf(p, "Record-Route: %s\r\n", ie->route[i]);
			else sprintf(p, "Route: %s\r\n", ie->route[i]);
			p += strlen(p);
		}
	if(ie->fromname && *ie->fromname)
		sprintf(p, "From: \"%s\"<sip:%s>;tag=%s\r\n", ie->fromname, ie->fromaddr, ie->fromtag);
	else sprintf(p, "From: <sip:%s>;tag=%s\r\n", ie->fromaddr, ie->fromtag);
	p += strlen(p);
	if(ie->totag && *ie->totag)
		sprintf(p, "To: \"%s\"<sip:%s>;tag=%s\r\n", ie->toname && *ie->toname ? ie->toname : "Anonymous", ie->toaddr, ie->totag);
	else sprintf(p, "To: \"%s\"<sip:%s>\r\n", ie->toname && *ie->toname ? ie->toname : "Anonymous", ie->toaddr);
	p += strlen(p);
	for(i = 0; i < MAXVIA; i++)
		if(ie->viaaddr[i])
		{
			if(ie->viarport[i] > 0)
			{
				if(ie->viabranch[i])
					sprintf(p, "Via: SIP/2.0/UDP %s;rport=%d;branch=%s\r\n", ie->viaaddr[i], ie->viarport[i], ie->viabranch[i]);
				else sprintf(p, "Via: SIP/2.0/UDP %s;rport=%d\r\n", ie->viaaddr[i], ie->viarport[i]);
			} else if(ie->viarport[i] == -1)
			{
				if(ie->viabranch[i])
					sprintf(p, "Via: SIP/2.0/UDP %s;branch=%s\r\n", ie->viaaddr[i], ie->viabranch[i]);
				else sprintf(p, "Via: SIP/2.0/UDP %s\r\n", ie->viaaddr[i]);
			} else {
				if(ie->viabranch[i])
					sprintf(p, "Via: SIP/2.0/UDP %s;rport;branch=%s\r\n", ie->viaaddr[i], ie->viabranch[i]);
				else sprintf(p, "Via: SIP/2.0/UDP %s;rport\r\n", ie->viaaddr[i]);
			}
			p += strlen(p);
		}
	if(ie->nonce && ie->command != SIPCMD_ACK_INVITE && ie->command != SIPCMD_ACK_REGISTER)
	{
		if(ie->command == SIPCMD_INVITEACK || ie->command == SIPCMD_REGISTERACK)
			sprintf(p, "Proxy-Authenticate: Digest nonce=\"%s\"", ie->nonce);
		else if(ie->authtype == 2)
			sprintf(p, "Proxy-Authorization: Digest nonce=\"%s\"", ie->nonce);
		else sprintf(p, "Authorization: Digest nonce=\"%s\"", ie->nonce);
		p += strlen(p);
		sprintf(p, ", uri=\"sip:%s\"", ie->destaddr);
		p += strlen(p);
		if(ie->realm)
		{
			sprintf(p, ", realm=\"%s\"", ie->realm);
			p += strlen(p);
		}
		if(ie->username)
		{
			sprintf(p, ", username=\"%s\"", ie->username);
			p += strlen(p);
		}
		if(ie->response)
		{
			sprintf(p, ", response=\"%s\"", ie->response);
			p += strlen(p);
		}
		if(ie->opaque)
		{
			sprintf(p, ", opaque=\"%s\"", ie->opaque);
			p += strlen(p);
		}
		if(ie->cnonce)
		{
			sprintf(p, ", cnonce=\"%s\"", ie->cnonce);
			p += strlen(p);
		}
		if(ie->qop == QOP_AUTH)
		{
			sprintf(p, ", qop=auth, nc=%08x", ie->nc);
			p += strlen(p);
		} else if(ie->qop == QOP_AUTHINT)
		{
			sprintf(p, ", qop=\"authint\", nc=%08x", ie->nc);
			p += strlen(p);
		}
		strcpy(p, ", algorithm=md5\r\n");
		p += strlen(p);
	}
	if(ie->command == SIPCMD_REFER)
	{
		if(ie->referto && *ie->referto)
		{
			sprintf(p, "Refer-To: <sip:%s>\r\n", ie->referto);
			p += strlen(p);
		}
		if(ie->referredby && *ie->referredby)
		{
			sprintf(p, "Referred-By: <sip:%s>\r\n", ie->referredby);
			p += strlen(p);
		}
		sprintf(p, "Refer-Sub: false\r\n");
		p += strlen(p);
	}
	if(ie->command == SIPCMD_INVITEACK || ie->command == SIPCMD_REGISTERACK)
		strcpy(p, "Server: VPStack/1.0\r\n");
	else strcpy(p, "User-Agent: VPStack/1.0\r\n");
	p += strlen(p);
	strcpy(p, "\r\n");
	p += strlen(p);
	strcpy(p, sdp);
	return strlen(pdu);
}

int sgetline(char **p, char *line, int linebuflen)
{
	char *q;
	int n;

	q = strchr(*p, '\r');
	if(!q)
		q = strchr(*p, '\n');
	if(!q)
		return -1;
	n = q - *p >= linebuflen ? linebuflen - 1 : q - *p;
	memcpy(line, *p, n);
	line[n] = 0;
	if(*q == '\r')
	{
		q++;
		if(*q == '\n')
			q++;
	} else q++;
	*p = q;
	return n;
}

int geteqsides(char *left, char **right)
{
	while(*left && *left != '=' && !ISSPACE(*left))
		left++;
	if(*left)
	{
		if(ISSPACE(*left))
		{
			*left++ = 0;
			while(ISSPACE(*left));
			if(*left == '=')
				left++;
		} else *left++ = 0;
		while(ISSPACE(*left));
	}
	*right = left;
	return 0;
}

int addiestring(const char *base, char **p, char **dst, const char *src)
{
	int len = strlen(src);

	if(*p + len + 1 > base + IEBUFSIZE)
		return -1;
	strcpy(*p, src);
	*dst = *p;
	*p += len + 1;
	return 0;
}

int addieunquotedstring(const char *base, char **p, char **dst, char **src)
{
	char *q = *p;

	while(q - base < IEBUFSIZE)
	{
		if(**src == ' ' || **src == '<')
		{
			*q++ = 0;
			while(ISSPACE(**src))
				(*src)++;
			*dst = *p;
			*p = q;
			return 0;
		}
		if(**src == '\\')
			(*src)++;
		if(**src)
			*q++ = *(*src)++;
		else return -1;
	}
	for(;;)
	{
		if(**src == ' ' || **src == '<')
		{
			while(ISSPACE(**src))
				(*src)++;
			return -1;
		}
		if(**src == '\\')
			(*src)++;
		if(**src)
			*(*src)++;
		else return -1;
	}
}

int addiequotedstring(const char *base, char **p, char **dst, char **src)
{
	char *q = *p;

	while(q - base < IEBUFSIZE)
	{
		if(**src == '\"')
		{
			*q++ = 0;
			(*src)++;
			*dst = *p;
			*p = q;
			return 0;
		}
		if(**src == '\\')
			(*src)++;
		if(**src)
			*q++ = *(*src)++;
		else return -1;
	}
	for(;;)
	{
		if(**src == '\"')
		{
			(*src)++;
			return -1;
		}
		if(**src == '\\')
			(*src)++;
		if(**src)
			*(*src)++;
		else return -1;
	}
}

int ParseToIE(char *pdu, char iebuf[IEBUFSIZE], SIP_ELEMENTS *ie)
{
	char line[300], line2[300], *pdup = pdu, *token, *p, *iep;
	int nvia = 0, nroute = 0, iLBCmap = -1, codecslisti = 0, i;
	unsigned char codecslist[50];

	iep = iebuf;
	if(sgetline(&pdup, line, 300) < 0)
		return -1;
	strcpy(line2, line);
	token = strtok(line, " \t");
	if(!token)
		return -1;
	if(!stricmp(token, "INVITE"))
		ie->command = SIPCMD_INVITE;
	else if(!stricmp(token, "REGISTER"))
		ie->command = SIPCMD_REGISTER;
	else if(!stricmp(token, "BYE"))
		ie->command = SIPCMD_BYE;
	else if(!stricmp(token, "CANCEL"))
		ie->command = SIPCMD_CANCEL;
	else if(!stricmp(token, "ACK"))
		ie->command = SIPCMD_ACK_INVITE;
	else if(!stricmp(token, "INFO"))
		ie->command = SIPCMD_INFO;
	else if(!stricmp(token, "OPTIONS"))
		ie->command = SIPCMD_OPTIONS;
	else if(!stricmp(token, "REFER"))
		ie->command = SIPCMD_REFER;
	else if(!stricmp(token, "SIP/2.0"))
	{
		ie->command = SIPCMD_GENACK;
		ie->answer = ANSWER_REFUSE;
		token = strtok(0, " \t");
		ie->sipreason = atoi(token);
		addiestring(iebuf, &iep, &ie->sipreasontext, line2);
		if(!stricmp(token, "200") || !stricmp(token, "202"))
			ie->answer = ANSWER_ACCEPT;
		else if(!stricmp(token, "180"))
			ie->answer = ANSWER_USERALERTED;
		else if(*token == '1')
		{
			ie->answer = ANSWER_USERALERTED;
			ie->reason = REASON_CALLPROCEEDING;
		} else if(!stricmp(token, "407") || !stricmp(token, "401"))
			ie->reason = REASON_AUTHERROR;
		else if(!stricmp(token, "487"))
			ie->reason = REASON_DISCONNECTED;	// Request terminated
		else if(!stricmp(token, "404") || !stricmp(token, "484") || !stricmp(token, "604"))
			ie->reason = REASON_INVALIDNUMBER;
		else if(!stricmp(token, "486") || !stricmp(token, "600") || !stricmp(token, "603"))
			ie->reason = REASON_BUSY;
		else if(*token == '5')
			ie->reason = REASON_ADMINERROR;
		else ie->reason = REASON_NOTKNOWN;
	} else ie->command = SIPCMD_UNKNOWN;
	token = strtok(0, " \t");
	if(!memicmp(token, "sip:", 4))
		addiestring(iebuf, &iep, &ie->destaddr, token+4);
	while(sgetline(&pdup, line, 300) > 0)
	{
		strcpy(line2, line);
		token = strtok(line, " \t:");
		if(!token)
			continue;
		if(!stricmp(token, "Call-ID"))
		{
			token = strtok(0, " \t:");
			if(token)
				addiestring(iebuf, &iep, &ie->callid, token);
		} else if(!stricmp(token, "Reason"))
		{
			addiestring(iebuf, &iep, &ie->reasonline, line2);
		} else if(!stricmp(token, "CSeq"))
		{
			token = strtok(0, " \t:");
			if(token)
			{
				ie->seqnumber = strtoul(token, 0, 10);
				token = strtok(0, " \t");
				if(token)
				{
					if(!stricmp(token, "BYE"))
					{
						if(ie->command == SIPCMD_GENACK)
							ie->command = SIPCMD_BYEACK;
					} else if(!stricmp(token, "INVITE"))
					{
						if(ie->command == SIPCMD_GENACK)
							ie->command = SIPCMD_INVITEACK;
					} else if(!stricmp(token, "REGISTER"))
					{
						if(ie->command == SIPCMD_GENACK)
							ie->command = SIPCMD_REGISTERACK;
						else if(ie->command == SIPCMD_ACK_INVITE)
							ie->command = SIPCMD_ACK_REGISTER;;
					} else if(!stricmp(token, "CANCEL"))
					{
						if(ie->command == SIPCMD_GENACK)
							ie->command = SIPCMD_CANCELACK;
					} else if(!stricmp(token, "REFER"))
					{
						if(ie->command == SIPCMD_GENACK)
							ie->command = SIPCMD_REFERACK;
					}
				}
			}
		} else if(!stricmp(token, "From"))
		{
			token = token + strlen(token) + 1;	// This can generate problems, if nothing follows "From" in the original line
			while(ISSPACE(*token))
				token++;
			if(*token == '\"')
			{
				token++;
				addiequotedstring(iebuf, &iep, &ie->fromname, &token);
				while(ISSPACE(*token))
					token++;
			} else if(*token != '<')
				addieunquotedstring(iebuf, &iep, &ie->fromname, &token);
			if(*token == '<')
			{
				token++;
				if(!memicmp(token, "sip:", 4))
					token += 4;
				while(ISSPACE(*token))
					token++;
				for(p = token; *p && *p != '>'; p++);
				if(*p == '>')
				{
					*p++ = 0;
					addiestring(iebuf, &iep, &ie->fromaddr, token);
				}
				token = p;
			}
			while(ISSPACE(*token))
				token++;
			token = strtok(token, " \t;");
			if(token)
			{
				geteqsides(token, &p);
				if(!stricmp(token, "tag"))
					addiestring(iebuf, &iep, &ie->fromtag, p);
			}
		} else if(!stricmp(token, "To"))
		{
			token = token + strlen(token) + 1;
			while(ISSPACE(*token))
				token++;
			if(*token == '\"')
			{
				token++;
				addiequotedstring(iebuf, &iep, &ie->toname, &token);
				while(ISSPACE(*token))
					token++;
			} else if(!strnicmp(token, "sip:", 4))
			{
				token += 4;
				for(p = token; *p && *p != ';'; p++);
				addiestring(iebuf, &iep, &ie->toaddr, token);
			} else if(*token != '<')
				addieunquotedstring(iebuf, &iep, &ie->toname, &token);
			if(*token == '<')
			{
				token++;
				if(!memicmp(token, "sip:", 4))
					token += 4;
				while(ISSPACE(*token))
					token++;
				for(p = token; *p && *p != '>'; p++);
				if(*p == '>')
				{
					*p++ = 0;
					addiestring(iebuf, &iep, &ie->toaddr, token);
				}
				token = p;
			}
			while(ISSPACE(*token))
				token++;
			token = strtok(token, " \t;");
			if(token)
			{
				geteqsides(token, &p);
				if(!stricmp(token, "tag"))
					addiestring(iebuf, &iep, &ie->totag, p);
			}
		} else if(!stricmp(token, "Via"))
		{
			token = strtok(0, " \t/");
			if(token)	// Now token should contain SIP
			{
				token = strtok(0, " \t/");
				if(token)	// Now token should contain 2.0
				{
					token = strtok(0, " \t/");
					if(token)		// Now token should contain UDP
					{
						token = strtok(0, " \t;");
						if(token)	// Now token should contain via host
						{
							addiestring(iebuf, &iep, &ie->viaaddr[nvia], token);
							ie->viarport[nvia] = -1;
							token = strtok(0, " \t;");
							while(token)
							{
								geteqsides(token, &p);
								if(!stricmp(token, "rport"))
									ie->viarport[nvia] = atoi(p);
								else if(!stricmp(token, "branch"))
									addiestring(iebuf, &iep, &ie->viabranch[nvia], p);
								token = strtok(0, " \t;");
							}
							if(nvia < MAXVIA - 1)
								nvia++;
						}
					}

				}
			}
		} else if(!stricmp(token, "Contact"))
		{
			token = token + strlen(token) + 1;
			while(ISSPACE(*token))
				token++;
			if(*token == '\"')
			{
				token++;
				while(*token && *token != '\"')
					token++;
				if(*token == '\"')
					token++;
				while(ISSPACE(*token))
					token++;
			}
			if(*token == '<')
			{
				token++;
				if(!memicmp(token, "sip:", 4))
					token += 4;
				while(ISSPACE(*token))
					token++;
				for(p = token; *p && *p != '>'; p++);
				if(*p == '>')
				{
					*p++ = 0;
					addiestring(iebuf, &iep, &ie->contact, token);
				}
				token = p;
			}
			token = strtok(token, " \t;");
			while(token)
			{
				geteqsides(token, &p);
				if(!stricmp(token, "expires"))
					ie->expire = atoi(p);
				token = strtok(0, " \t;");
			}
		} else if(!stricmp(token, "Expire"))
		{
			token = token + strlen(token) + 1;
			while(ISSPACE(*token))
				token++;
			ie->expire = atoi(token);
		} else if(!stricmp(token, "Route") || !stricmp(token, "Record-Route"))
		{
			token = token + strlen(token) + 1;
			while(ISSPACE(*token))
				token++;
			addiestring(iebuf, &iep, &ie->route[nroute++], token);
		} else if(!stricmp(token, "Proxy-Authenticate") || !stricmp(token, "Proxy-Authorization") ||
			!stricmp(token, "WWW-Authenticate") || !stricmp(token, "Authorization"))
		{
			if(!stricmp(token, "WWW-Authenticate"))
				ie->authtype = 1;
			else if(!stricmp(token, "Proxy-Authorization"))
				ie->authtype = 2;
			else if(!stricmp(token, "Authorization"))
				ie->authtype = 3;
			else ie->authtype = 0;
			token = strtok(0, " \t,");
			while(token)
			{
				geteqsides(token, &p);
				if(!stricmp(token, "username"))
				{
					if(*p == '\"')
					{
						p++;
						addiequotedstring(iebuf, &iep, &ie->username, &p);
					} else addiestring(iebuf, &iep, &ie->username, p);
				} else if(!stricmp(token, "realm"))
				{
					if(*p == '\"')
					{
						p++;
						addiequotedstring(iebuf, &iep, &ie->realm, &p);
					} else addiestring(iebuf, &iep, &ie->realm, p);
				} else if(!stricmp(token, "nonce"))
				{
					if(*p == '\"')
					{
						p++;
						addiequotedstring(iebuf, &iep, &ie->nonce, &p);
					} else addiestring(iebuf, &iep, &ie->nonce, p);
				} else if(!stricmp(token, "cnonce"))
				{
					if(*p == '\"')
					{
						p++;
						addiequotedstring(iebuf, &iep, &ie->cnonce, &p);
					} else addiestring(iebuf, &iep, &ie->cnonce, p);
				} else if(!stricmp(token, "opaque"))
				{
					if(*p == '\"')
					{
						p++;
						addiequotedstring(iebuf, &iep, &ie->opaque, &p);
					} else addiestring(iebuf, &iep, &ie->opaque, p);
				} else if(!stricmp(token, "response"))
				{
					if(*p == '\"')
					{
						p++;
						addiequotedstring(iebuf, &iep, &ie->response, &p);
					} else addiestring(iebuf, &iep, &ie->response, p);
				} else if(!stricmp(token, "nc"))
				{
					if(*p == '\"')
					{
						ie->nc = strtoul(p + 1, &p, 16);
						p++;
					} else ie->nc = strtoul(p, &p, 16);
				} else if(!stricmp(token, "qop"))
				{
					if(*p == '\"')
					{
						char *q;

						p++;
						q = p;
						while(*q && *q != '\"')
							q++;
						if(!memicmp(p, "auth", q - p) || !memicmp(p, "auth,auth-int", q - p))
							ie->qop = QOP_AUTH;
						else if(!memicmp(p, "auth-int", q - p))
							ie->qop = QOP_AUTHINT;						
					}
				}
				token = strtok(0, " \t,");
			}
		}
	}
	while(sgetline(&pdup, line, 300) > 0)
	{
		if(!memicmp(line, "c=IN IP4 ", 9))
		{
			ie->rtpaddr.sin_family = AF_INET;
			ie->rtpaddr.sin_addr.s_addr = inet_addr(line + 9);
		} else if(!memicmp(line, "m=audio ", 8))
		{
			if(ie->answer == ANSWER_USERALERTED)
			{
				ie->answer = ANSWER_ACCEPT;
				ie->reason = REASON_EARLYAUDIO;
			}
			ie->rtpaddr.sin_port = htons((unsigned short)strtoul(line + 8, &p, 10));
			if(!memicmp(p, " RTP/AVP ", 9))
			{
				p = strtok(p + 9, " ");
				while(p)
				{
					int i = atoi(p);
					
					if(i >= 0 && i <= 255)
					{
						if(codecslisti < 50)
							codecslist[codecslisti++] = i;
						// Default rtpmaps
						switch(i)
						{
						case 3:
							ie->bc_codecs |= (1<<CODEC_GSMFR);
							break;
						case 8:
							ie->bc_codecs |= (1<<CODEC_G711A);
							break;
						case 0:
							ie->bc_codecs |= (1<<CODEC_G711U);
							break;
						case 18:
							ie->bc_codecs |= (1<<CODEC_G729);
							break;
						}
					}
					p = strtok(0, " ");
				}
			}
		} else if(!memicmp(line, "a=rtpmap:", 9))
		{
			p = strtok(line + 9, " ");
			if(p)
			{
				int i = atoi(p);

				if(i >= 0 && i <= 255)
				{
					p = strtok(0, " ");
					if(p)
					{
						if(!stricmp(p, "telephone-event/8000"))
							ie->keypadmap = i;
						if(!stricmp(p, "iLBC/8000"))
							iLBCmap = i;
					}
				}
			}
		} else if(!memicmp(line, "a=fmtp:", 7))
		{
			p = strtok(line+7, " ");
			if(p)
			{
				int i = atoi(p), j;
				if(i == iLBCmap)
				{
					p = strtok(0, " ");
					if(!memicmp(p, "mode=20", 7))
					{
						ie->iLBC20map = iLBCmap;
						for(j = 0; j < codecslisti; j++)
							if(codecslist[j] == i)
							{
								ie->bc_codecs |= (1<<CODEC_ILBC20);
								break;
							}
					} else if(!memicmp(p, "mode=30", 7))
					{
						ie->iLBC30map = iLBCmap;
						for(j = 0; j < codecslisti; j++)
							if(codecslist[j] == i)
							{
								ie->bc_codecs |= (1<<CODEC_ILBC30);
								break;
							}
					}
				}
			}
		} else if(!memicmp(line, "signal=", 7))
		{
			ie->keypadmap=line[7];
		} else if(!stricmp(line, "a=recvonly"))
			ie->hold = HOLD_RECVONLY;
		else if(!stricmp(line, "a=sendonly"))
			ie->hold = HOLD_SENDONLY;
		else if(!stricmp(line, "a=inactive"))
			ie->hold = HOLD_INACTIVE;
	}
	for(i = 0; i < codecslisti; i++)
	{
		if(codecslist[i] == 3)
			ie->codec = CODEC_GSMFR;
		else if(codecslist[i] == 8)
			ie->codec = CODEC_G711A;
		else if(codecslist[i] == 0)
			ie->codec = CODEC_G711U;
		else if(codecslist[i] == 18)
			ie->codec = CODEC_G729;
		else if(codecslist[i] == ie->iLBC20map)
			ie->codec = CODEC_ILBC20;
		else if(codecslist[i] == ie->iLBC30map)
			ie->codec = CODEC_ILBC30;
		else continue;
		break;
	}
	return 0;
}

void Test()
{
	SIP_ELEMENTS ie;
	char iebuf[1500];
	char pdu[1500];

	strcpy(pdu,
"INVITE sip :192.168.0.10 SIP/2.0\r\n"
"Content-Length: 335\r\n"
"Contact: <sip:192.168.0.4:5060>\r\n"
"Call-ID: CB90A7D5-1A52-420A-9C09-4A1C8A487C86@192.168.0.4\r\n"
"Content-Type: application/sdp\r\n"
"From: \"Administrator\"<sip:192.168.0.4>;tag=2323382348259\r\n"
"CSeq: 1 INVITE\r\n"
"Max-Forwards: 70\r\n"
"To: <sip:192.168.0.10>\r\n"
"Via: SIP/2.0/UDP 192.168.0.4;rport;branch=z9hG4bKc0a800040131c9b14261288100004b1500000001\r\n"
"User-Agent: SJphone/1.50.271d (SJ Labs)\r\n"
"Proxy-Authenticate: Digest realm=\"sip.gafachi.com\", nonce=\"ba021be71466698f\"\r\n"
"\r\n"
"v=0\r\n"
"o=- 3322652376 3322652376 IN IP4 192.168.0.10\r\n"
"s=SJphone\r\n"
"c=IN IP4 192.168.0.10\r\n"
"t=0 0\r\n"
"a=direction:active\r\n"
"m=audio 49152 RTP/AVP 3 101\r\n"
"a=rtpmap:3 GSM/8000\r\n"
"a=rtpmap:101 telephone-event/8000\r\n"
"a=fmtp:101 0-16\r\n");
	ParseToIE(pdu, iebuf, &ie);
}

/**********************************************************************/
/* Locking                                                            */
/**********************************************************************/

void SIPCALLDATA::LockL(int line)
{
#ifdef LOCKDEBUGGING
	char s[100];

	sprintf(s, "%u SIPCALL %d (%x) lock in line %d", GetTickCount(), vpcall, this, line);
	debugstring(s);
	EnterCriticalSection(&cs);
	sprintf(s, "%u SIPCALL %d (%x) lock in line %d passed", GetTickCount(), vpcall, this, line);
	debugstring(s);
#else
	EnterCriticalSection(&cs);
#endif
}

void SIPCALLDATA::UnlockL(int line)
{
#ifdef LOCKDEBUGGING
	char s[100];

	sprintf(s, "%u SIPCALL %d (%x) unlock in line %d", GetTickCount(), vpcall, this, line);
	debugstring(s);
#endif
	LeaveCriticalSection(&cs);
}

SIPSTACK::SIPSTACK(VPAUDIODATAFACTORY *vpaf, const char *stackname)
{
	vpaudiodatafactory = vpaf;
	nw = nr = 0;
	lastvpcallvalue = 0;
	nthreads = 0;
	gseqnumber = 1;
	Zero(settings);
	Zero(calls);
	Zero(rxxc);
	NotifyRoutine = 0;
	SyncNotifyRoutine = 0;
	natresolved = false;
	settings.supported_codecs = (1<<CODEC_G711A) | (1<<CODEC_G711U) | (1<<CODEC_GSMFR);
#ifdef INCLUDEILBC
	settings.supported_codecs |= (1<<CODEC_ILBC20) | (1<<CODEC_ILBC30);
#endif
#ifdef LEO
	settings.supported_codecs = (1<<CODEC_G711A);
#endif
	settings.notifyinterval = 60000;
	if(stackname)
		strncpy2(this->stackname, stackname, sizeof(this->stackname));
//	settings.remotenat = 1;
	Lprintf("SIP stack created");
}

SIPSTACK::~SIPSTACK()
{
	int i;
	VPCALL vpcall;
	unsigned tm;

	Lprintf("Destroying SIP stack");
	tx.Stop();
	Logoff();
	ReadLock();
	for(i = 0; i < MAXCALLS; i++)
		if(calls[i])
		{
			vpcall = calls[i]->vpcall;
			ReadUnlock();
			Disconnect(vpcall, 0);
			ReadLock();
		}
	ReadUnlock();
	SOCKET sk = sock;
	sock = 0;
	closesocket(sk);
	tm = GetTickCount() + 1000;
	while((int)(tm - GetTickCount()) > 0)
	{
		for(i = 0; i < MAXCALLS; i++)
			if(calls[i])
				break;
		if(i == MAXCALLS)
			break;
	}
	ReadLock();
	for(i = 0; i < MAXCALLS; i++)
		if(calls[i])
		{
			vpcall = calls[i]->vpcall;
			ReadUnlock();
			FreeCall(vpcall);
			ReadLock();
		}
	ReadUnlock();
	while(nthreads)
		Sleep(10);
	Lprintf("SIP stack destroyed");
}

void SIPSTACK::ReadLockL(int line)
{
#ifdef LOCKDEBUGGING
	char s[100];

	sprintf(s, "%u SIPSTACK read lock in line %d, nr=%d, nw=%d", GetTickCount(), line, nr, nw);
	debugstring(s);
	lock.RLock();
	nr++;
	sprintf(s, "%u SIPSTACK read lock in line %d passed, nr=%d, nw=%d", GetTickCount(), line, nr, nw);
	debugstring(s);
#else
	lock.RLock();
#endif
}

void SIPSTACK::ReadUnlockL(int line)
{
#ifdef LOCKDEBUGGING
	char s[100];

	nr--;
	sprintf(s, "%u SIPSTACK read unlock in line %d, nr=%d, nw=%d", GetTickCount(), line, nr, nw);
	debugstring(s);
#endif
	lock.RUnlock();
}

void SIPSTACK::WriteLockL(int line)
{
#ifdef LOCKDEBUGGING
	char s[100];

	sprintf(s, "%u SIPSTACK write lock in line %d, nr=%d, nw=%d", GetTickCount(), line, nr, nw);
	debugstring(s);
	lock.WLock();
	nw++;
	sprintf(s, "%u SIPSTACK write lock in line %d, nr=%d, nw=%d", GetTickCount(), line, nr, nw);
	debugstring(s);
#else
	lock.WLock();
#endif
}

void SIPSTACK::WriteUnlockL(int line)
{
#ifdef LOCKDEBUGGING
	char s[100];

	nw--;
	sprintf(s, "%u SIPSTACK write unlock in line %d, nr=%d, nw=%d", GetTickCount(), line, nr, nw);
	debugstring(s);
#endif
	lock.WUnlock();
}

void SIPSTACK::BuildLocalCallData(SIPCALLDATA *vpc)
{
	if(*vpc->ourorigname)
		sprintf(vpc->ouraddr, "%s@%d.%d.%d.%d:%d", vpc->ourorigname, fulllocaladdr[0], fulllocaladdr[1], fulllocaladdr[2], fulllocaladdr[3], fulllocaladdr[4] * 256 + fulllocaladdr[5]);
	else sprintf(vpc->ouraddr, "%d.%d.%d.%d:%d", fulllocaladdr[0], fulllocaladdr[1], fulllocaladdr[2], fulllocaladdr[3], fulllocaladdr[4] * 256 + fulllocaladdr[5]);
	if(*vpc->ourorigname)
		sprintf(vpc->ourcontact, "%s@%d.%d.%d.%d:%d", vpc->ourorigname, fulllocaladdr[0], fulllocaladdr[1], fulllocaladdr[2], fulllocaladdr[3], fulllocaladdr[4] * 256 + fulllocaladdr[5]);
	else sprintf(vpc->ourcontact, "%d.%d.%d.%d:%d", fulllocaladdr[0], fulllocaladdr[1], fulllocaladdr[2], fulllocaladdr[3], fulllocaladdr[4] * 256 + fulllocaladdr[5]);
	sprintf(vpc->viaaddr[0], "%d.%d.%d.%d:%d", fulllocaladdr[0], fulllocaladdr[1], fulllocaladdr[2], fulllocaladdr[3], fulllocaladdr[4] * 256 + fulllocaladdr[5]);
	sprintf(vpc->viabranch[0], "z9hG4bK%02x%02x%02x%02x%02x%02x", fulllocaladdr[0], fulllocaladdr[1], fulllocaladdr[2], fulllocaladdr[3],
		fulllocaladdr[4], fulllocaladdr[5]);
	RandomString128(vpc->viabranch[0] + strlen(vpc->viabranch[0]));
}

void SIPSTACK::BuildVia(SIPCALLDATA *vpc)
{
	memset(vpc->viaaddr, 0, sizeof(vpc->viaaddr));
	memset(vpc->viabranch, 0, sizeof(vpc->viabranch));
	sprintf(vpc->viaaddr[0], "%d.%d.%d.%d:%d", fulllocaladdr[0], fulllocaladdr[1], fulllocaladdr[2], fulllocaladdr[3], fulllocaladdr[4] * 256 + fulllocaladdr[5]);
	sprintf(vpc->viabranch[0], "z9hG4bK%02x%02x%02x%02x%02x%02x", fulllocaladdr[0], fulllocaladdr[1], fulllocaladdr[2], fulllocaladdr[3],
		fulllocaladdr[4], fulllocaladdr[5]);
	RandomString128(vpc->viabranch[0] + strlen(vpc->viabranch[0]));
}

VPCALL SIPSTACK::CreateCall()
{
	int i;
	VPCALL vpcall;
	SIPCALLDATA *vpc;
	struct sockaddr_in addr;
	int rc;
	socklen_t addrlen;

	WriteLock();
	for(i = 0; i < MAXCALLS; i++)
		if(!calls[i])
		{
			vpc = new SIPCALLDATA;
			calls[i] = vpc;
			vpc->keypadmap = 101;
			vpc->vpcall = vpcall = (VPCALL)++lastvpcallvalue;
			vpc->framesperpacket = 2;	// Without bandwidth management
			vpc->rtpsk = socket(AF_INET, SOCK_DGRAM, 0);
			vpc->aa.f = -1;
			vpc->lastkeypadseqnumber = (unsigned)-1;
			if(!vpc->rtpsk)
			{
				delete vpc;
				WriteUnlock();
				return 0;
			}
			Zero(addr);
			addr.sin_family = AF_INET;
			if(settings.bindaddr)
				addr.sin_addr.s_addr = settings.bindaddr;
			rc = bind(vpc->rtpsk, (struct sockaddr *)&addr, sizeof(addr));
			if(rc)
			{
				closesocket(vpc->rtpsk);
				delete vpc;
				WriteUnlock();
				return 0;
			}
			addrlen = sizeof(addr);
			getsockname(vpc->rtpsk, (struct sockaddr *)&vpc->lrtpaddr, &addrlen);
/*			while(ntohs(vpc->lrtpaddr.sin_port) & 1)	// This is not useful, as NAT router will change the port in any case
			{
				closesocket(vpc->rtpsk);
				vpc->rtpsk = socket(AF_INET, SOCK_DGRAM, 0);
				rc = bind(vpc->rtpsk, (struct sockaddr *)&addr, sizeof(addr));
				if(rc)
				{
					closesocket(vpc->rtpsk);
					delete vpc;
					WriteUnlock();
					return 0;
				}
				getsockname(vpc->rtpsk, (struct sockaddr *)&vpc->lrtpaddr, &addrlen);
			}*/
			Lprintf("RTP socket for call %d bound on port %d", vpc->vpcall, ntohs(vpc->lrtpaddr.sin_port));
			vpc->sessionversion = vpc->sessionid = (unsigned)time(0) + 2208988800U;
			addrlen = sizeof(addr);
			if(!*(BYTE *)&vpc->lrtpaddr.sin_addr)
				memcpy(&vpc->lrtpaddr.sin_addr, fulllocaladdr, 4);
			if(!settings.undernat)
				vpc->rtpnatok = true;
			vpc->nc = 1;
			strcpy(vpc->ourname, settings.username);
			strcpy(vpc->ourorigname, settings.username);
			strcpy(vpc->password, settings.password);
			BuildLocalCallData(vpc);
			WriteUnlock();
			TSTART
			beginclassthread((ClassThreadStart)&SIPSTACK::RTPReceiver, this, vpcall);
			return vpcall;
		}
	WriteUnlock();
	return 0;
}

void SIPSTACK::FreeCall(VPCALL vpcall)
{
	int i;

	WriteLock();
	for(i = 0; i < MAXCALLS; i++)
	{
		if(calls[i] && calls[i]->vpcall == vpcall)
		{
			calls[i]->Lock();
			closesocket(calls[i]->rtpsk);
			delete calls[i];
			calls[i] = 0;
			break;
		}
	}
	WriteUnlock();
}

SIPCALLDATA *SIPSTACK::LockCallL(int index, int line)
{
	if((unsigned)index < MAXCALLS)
	{
#ifdef LOCKDEBUGGING
		char s[100];

		sprintf(s, "%u LockCall idx=%d lock in line %d", GetTickCount(), index, line);
		debugstring(s);
#endif
		ReadLock();
		SIPCALLDATA *vpc = calls[index];
		if(vpc)
			vpc->Lock();
		ReadUnlock();
		return vpc;
	}
	return 0;
}

SIPCALLDATA *SIPSTACK::LockCallL(VPCALL vpcall, int line)
{
	int i;

#ifdef LOCKDEBUGGING
	char s[100];

	sprintf(s, "%u LockCall %d lock in line %d", GetTickCount(), vpcall, line);
	debugstring(s);
#endif
	ReadLock();
	for(i = 0; i < MAXCALLS; i++)
	{
		if(calls[i] && calls[i]->vpcall == vpcall)
		{
			SIPCALLDATA *vpc = calls[i];
			if(vpc)
				vpc->Lock();
			ReadUnlock();
			return vpc;
		}
	}
	ReadUnlock();
	return 0;
}

unsigned SIPSTACK::PickSeqNumber()
{
	return gseqnumber++;
}

/**********************************************************************/
/* Public functions: general                                          */
/**********************************************************************/

int SIPSTACK::Init()
{
	struct sockaddr_in addr;
	int rcvlen, tos = 0x98;
	unsigned short bindport;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock == INVALID_SOCKET)
	{
		sock = 0;
		return VPERR_TCPIPERROR;
	}
	rcvlen = 128000;
	setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&rcvlen, sizeof(rcvlen));
	setsockopt(sock, IPPROTO_IP, IP_TOS, (char *)&tos, sizeof(tos));
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	for(bindport = SIPPORT; bindport < SIPPORT + 1000; bindport++)
	{
		addr.sin_port = htons(bindport);
		if(settings.bindaddr)
			addr.sin_addr.s_addr = settings.bindaddr;
		if(!bind(sock, (sockaddr *)&addr, sizeof(addr)))
			break;
	}
	if(bindport == SIPPORT + 1000)
	{
		closesocket(sock);
		sock = 0;
		return VPERR_TCPIPERROR;
	}
	CopyDWord(fulllocaladdr, FindLocalHostAddr(0, 1));
	fulllocaladdr[4] = (BYTE)(bindport >> 8);
	fulllocaladdr[5] = (BYTE)bindport;
	BuildLocalAddr();
	tx.sk = sock;
	tx.Run();
	TSTART
	beginclassthread((ClassThreadStart)&SIPSTACK::TransmitThread, this, 0);
	TSTART
	beginclassthread((ClassThreadStart)&SIPSTACK::ReceiveThread, this, 0);
	Lprintf("SIP stack listening on port %d", bindport);
	return 0;	
}

void SIPSTACK::SetBindAddr(long bindaddr)
{
	settings.bindaddr = bindaddr;
	memcpy(fulllocaladdr, &bindaddr, 4);
	BuildLocalAddr();
}


int SIPSTACK::Connect(VPCALL *vpcall, const char *address, int bc)
{
	VPCALL tmpvpcall;
	int rc;

	if(bc != BC_VOICE)
		return VPERR_UNSUPPORTEDBEARER;
	tmpvpcall = CreateCall();
	if(!tmpvpcall)
		return VPERR_NOMORECALLSAVAILABLE;

	rc = Connect(tmpvpcall, address, bc);
	if(rc)
	{
		FreeCall(tmpvpcall);
		return rc;
	}
	*vpcall = tmpvpcall;
	return 0;
}

int SIPSTACK::Connect(VPCALL vpcall, const char *address, int bc)
{
	SIPCALLDATA *vpc;
	int rc;

	if(bc != BC_VOICE)
		return VPERR_UNSUPPORTEDBEARER;
	rc = SetCallAddress(vpcall, address);
	if(rc)
		return rc;

	vpc = LockCall(vpcall);
	if(vpc)
	{
		vpc->originatedcall = true;
		vpc->status = VPCS_CONNECTING;
		vpc->Unlock();
	} else return VPERR_INVALIDCALL;
	TSTART
	beginclassthread((ClassThreadStart)&SIPSTACK::ConnectThread, this, (void *)vpcall);
	return 0;
}

static ADDRESSTYPE AddressTypeSIP(const char *address)
{
	const char *p;

	if(!memicmp(address, "ip:", 3))
		return ADDRT_IPADDR;
	p = strchr(address, '@');
	if(p)
		address = p + 1;
	if(*address == '+')
	{
		while(*++address)
			if(!(*address >= '0' && *address <= '9' || *address == '*' || *address == '#' || *address == '-'))
				return ADDRT_INVALID;
		return ADDRT_PSTN;
	}
	if(address[0] >= 'a' && address[0] <= 'z' || address[0] >= 'A' && address[0] <= 'Z')
		return ADDRT_VNAME;
	return ADDRT_INVALID;
}

static ADDRESSTYPE CanonicFormAddressSIP(char *address)
{
	ADDRESSTYPE type = AddressTypeSIP(address);
	char *p;

	p = strchr(address, '@');
	if(p)
		address = p + 1;
	if(type == ADDRT_PSTN)
	{
		while(*address)
		{
			if(*address == '-')
				memmove(address, address + 1, strlen(address));
			else address++;
		}
	}
	return type;
}

int SIPSTACK::SetCallAddress(VPCALL vpcall, const char *address)
{
	SIPCALLDATA *vpc;
	char *user, *pwd, *srv, s[300], *p;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	strncpy2(vpc->address, address, MAXADDRLEN+1);
	vpc->addrtype = CanonicFormAddressSIP(vpc->address);
	if(vpc->addrtype == ADDRT_VNAME && !stricmp(vpc->address, vpc->ourname))
	{
		vpc->Unlock();
		return VPERR_LOOPCALL;
	}
	if(vpc->addrtype == ADDRT_IPADDR)
	{
		memmove(vpc->address, vpc->address + 3, strlen(vpc->address + 2));
		p = strchr(vpc->address, '@');
		Resolve(p ? p+1 : vpc->address, &vpc->addr, SIPPORT);
		strcpy(vpc->connectedaddr, vpc->address);
	} else {
		strcpy(s, address);
/*		if(*vpc->connectedname == '+')
		{
			memmove(vpc->connectedname + 1, vpc->connectedname, strlen(vpc->connectedname) + 1);
			vpc->connectedname[0] = vpc->connectedname[1] = '0';
		}*/
		user = strchr(s, '@');
		if(user)
		{
			pwd = strchr(user+1, ':');
			if(pwd)
				srv = strchr(pwd, '@');
			if(pwd && srv)
			{	
				*user++ = 0;
				*pwd++ = 0;
				*srv++ = 0;
				Resolve(srv, &vpc->addr, SIPPORT);
				strcpy(vpc->password, pwd);
				strcpy(vpc->ourname, user);
				strcpy(vpc->ourorigname, user);
				strcpy(vpc->connectedname, s);
				strcpy(vpc->address, s);
				strcat(vpc->address, "@");
				strcat(vpc->address, srv);
				sprintf(vpc->ouraddr, "%s@%s", user, srv);
			} else {
				strcpy(vpc->address, s);
				*user++ = 0;
				strcpy(vpc->connectedname, s);
				Resolve(user, &vpc->addr, SIPPORT);
			}
		} else {
			strcpy(vpc->connectedname, address);
			Resolve(settings.server, &vpc->addr, SIPPORT);
			if(*vpc->connectedname)
				sprintf(vpc->address, "%s@%s", vpc->connectedname, settings.domain);
			else strcpy(vpc->address, settings.domain);
		}
		strcpy(vpc->connectedaddr, vpc->address);
	}
	sprintf(vpc->viabranch[0], "z9hG4bK%02x%02x%02x%02x%02x%02x", fulllocaladdr[0], fulllocaladdr[1], fulllocaladdr[2], fulllocaladdr[3],
		fulllocaladdr[4], fulllocaladdr[5]);
	RandomString128(vpc->viabranch[0] + strlen(vpc->viabranch[0]));
	RandomString128(vpc->callid);
	sprintf(vpc->callid + strlen(vpc->callid), "@%d.%d.%d.%d", fulllocaladdr[0], fulllocaladdr[1], fulllocaladdr[2], fulllocaladdr[3]);
	sprintf(vpc->ourtag, "F%u", GetTickCount());
	if(*settings.domain)
		sprintf(vpc->ouraddr, "%s@%s", vpc->ourorigname, settings.domain);
	vpc->Unlock();
	return 0;
}

int SIPSTACK::SetCallLocalName(VPCALL vpcall, const char *username)
{
	SIPCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	strncpy2(vpc->ourname, username, MAXNAMELEN+1);
	strncpy2(vpc->ourorigname, username, MAXNAMELEN+1);
	vpc->Unlock();
	return 0;
}

int SIPSTACK::SetCallLocalNumber(VPCALL vpcall, const char *number)
{
	SIPCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	strncpy2(vpc->ourname, number, MAXNUMBERLEN+1);
	strncpy2(vpc->ourorigname, number, MAXNUMBERLEN+1);
	vpc->Unlock();
	return 0;
}

int SIPSTACK::GetCallRemoteName(VPCALL vpcall, char *username)
{
	SIPCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	if(*vpc->connectedname)
		strncpy2(username, vpc->connectedname, MAXNAMELEN+1);
	else strncpy2(username, vpc->address, MAXADDRLEN+1);
	vpc->Unlock();
	return 0;
}

int SIPSTACK::GetCallRemoteNumber(VPCALL vpcall, char *number)
{
	SIPCALLDATA *vpc;
	char s[300], *p;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	strcpy(s, vpc->connectedaddr);
	p = strchr(s, '@');
	if(p)
		*p = 0;
	if(*s)
		strncpy2(number, s, MAXNUMBERLEN+1);
	else if(*vpc->connectedname)
		strncpy2(number, vpc->connectedname, MAXNUMBERLEN+1);
	else strncpy2(number, vpc->address, MAXADDRLEN+1);
	vpc->Unlock();
	return 0;
}

int SIPSTACK::GetCallRemoteAddress(VPCALL vpcall, char *address)
{
	SIPCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	if(vpc->address)
		strcpy(address, vpc->address);
	else sprintf(address, "%s:%d", inet_ntoa(vpc->addr.sin_addr), ntohs(vpc->addr.sin_port));
	vpc->Unlock();
	return 0;
}

int SIPSTACK::GetCallRemoteAddress(VPCALL vpcall, struct sockaddr_in *addr)
{
	SIPCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	*addr = vpc->addr;
	vpc->Unlock();
	return 0;
}

int SIPSTACK::GetCallLocalName(VPCALL vpcall, char *username)
{
	SIPCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	strncpy2(username, vpc->ourname, MAXNAMELEN+1);
	vpc->Unlock();
	return 0;
}

int SIPSTACK::GetCallLocalNumber(VPCALL vpcall, char *number)
{
	SIPCALLDATA *vpc;
	char s[300], *p;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	strcpy(s, vpc->ouraddr);
	p = strchr(s, '@');
	if(p)
		*p = 0;
	if(*s)
		strncpy2(number, s, MAXNUMBERLEN+1);
	else strncpy2(number, vpc->ourname, MAXNUMBERLEN+1);
	vpc->Unlock();
	return 0;
}

int SIPSTACK::GetCallCodec(VPCALL vpcall)
{
	SIPCALLDATA *vpc;
	int rc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	if(vpc->status == VPCS_CONNECTED)
		rc = vpc->codec;
	else rc = VPERR_INVALIDSTATUS;
	vpc->Unlock();
	return rc;
}

int SIPSTACK::GetCallStatus(VPCALL vpcall)
{
	int status;

	SIPCALLDATA *vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	status = vpc->status;
	vpc->Unlock();
	return status;
}

int SIPSTACK::GetSIPReason(VPCALL vpcall)
{
	int sipreason;

	SIPCALLDATA *vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	sipreason = vpc->sipreason;
	vpc->Unlock();
	return sipreason;
}

int SIPSTACK::GetSIPReasonText(VPCALL vpcall, char *reasontext)
{
	SIPCALLDATA *vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	strcpy(reasontext, vpc->sipreasontext);
	vpc->Unlock();
	return 0;
}

int SIPSTACK::GetReasonLine(VPCALL vpcall, char *reasonline)
{
	SIPCALLDATA *vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	strcpy(reasonline, vpc->reasonline);
	vpc->Unlock();
	return 0;
}

void SIPSTACK::CopyCallFields(SIP_ELEMENTS *se, SIPCALLDATA *vpc, bool answer)
{
	int i;

	se->callid = vpc->callid;
	if(answer)
	{
		se->fromaddr = vpc->connectedaddr;
		se->fromname = vpc->connectedname;
		se->fromtag = vpc->connectedtag;
		se->toaddr = vpc->ouraddr;
		se->toname = vpc->ourname;
		se->totag = vpc->ourtag;
	} else {
		se->fromaddr = vpc->ouraddr;
		se->fromname = vpc->ourname;
		se->fromtag = vpc->ourtag;
		se->toaddr = vpc->connectedaddr;
		se->toname = vpc->connectedname;
		se->totag = vpc->connectedtag;
	}
	se->referto = vpc->referto;
	se->referredby = vpc->ouraddr;
	se->keypadmap = vpc->keypadmap;
	se->iLBC20map = vpc->iLBC20map;
	se->iLBC30map = vpc->iLBC30map;
	se->contact = vpc->ourcontact;
	for(i = 0; i < MAXVIA; i++)
		if(*vpc->viaaddr[i])
		{
			se->viaaddr[i] = vpc->viaaddr[i];
			if(vpc->viarport[i])
				se->viarport[i] = vpc->viarport[i];
			if(*vpc->viabranch[i])
				se->viabranch[i] = vpc->viabranch[i];
		}
	memcpy(se->fulllocaladdr, fulllocaladdr, 6);
}

int SIPSTACK::Disconnect(VPCALL vpcall, int reason)
{
	int i;
	SIPCALLDATA *vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;

	if(vpc->connecttype == CT_REGISTER || vpc->connecttype == CT_REFER)
	{
		VPCALL vpcall = vpc->vpcall;
		vpc->Unlock();
		FreeCall(vpcall);
		return 0;
	}

	if(vpc->status == VPCS_INALERTED)
	{
		SIP_ELEMENTS se;
		struct sockaddr_in addr = vpc->addr;
		char pdu[1500];
		int len;

		BuildVia(vpc);
		CopyCallFields(&se, vpc, true);
		se.command = SIPCMD_INVITEACK;
		se.reason = reason;
		se.seqnumber = vpc->seqnumber;
		for(i = 0; i < MAXVIA; i++)
			if(*vpc->route[i])
				se.route[i] = vpc->route[i];
		len = Format(pdu, &se);

		vpc->Unlock();
		Send(&addr, pdu, len);
		NotifyOnCall(VPMSG_CALLENDED, vpcall, reason);
	} else if(vpc->status >= VPCS_CONNECTING && vpc->status <= VPCS_CONNECTED && vpc->addr.sin_addr.s_addr)
	{
		vpc->reason = reason;
		vpc->status = VPCS_DISCONNECTING;
		vpc->Unlock();
		TSTART
		beginclassthread((ClassThreadStart)&SIPSTACK::DisconnectThread, this, vpcall);
	} else if(vpc->status == VPCS_CONNECTING)
	{
		vpc->Unlock();
		NotifyOnCall(VPMSG_CALLENDED, vpcall, reason);
	} else vpc->Unlock();
	return 0;
}

int SIPSTACK::AnswerCall(VPCALL vpcall)
{
	SIPCALLDATA *vpc = LockCall(vpcall);

	if(!vpc)
		return VPERR_INVALIDCALL;
	if(vpc->status != VPCS_INALERTED)
	{
		vpc->Unlock();
		return VPERR_INVALIDSTATUS;
	}
	vpc->status = VPCS_INCONNECTING;
	vpc->Unlock();
	TSTART
	beginclassthread((ClassThreadStart)&SIPSTACK::IncomingCallThread, this, vpcall);
	return 0;
}

int SIPSTACK::IncomingCall(const struct sockaddr_in *addr, SIP_ELEMENTS *ie)
{
	return 2;
}

int SIPSTACK::Send(const struct sockaddr_in *addr,  const void *buf, int len)
{
	int i, rc;
	SIPCALLDATA *vpc;

	if(generatelog)
	{
		Lprintf("%u %s:%d Send %d bytes:", GetTickCount(), inet_ntoa(addr->sin_addr), ntohs(addr->sin_port), len);
		LogAsIs((const char *)buf, len);
	}
	if(addr && addr->sin_port)
	{
		rc = tx.Send(addr, buf, len);
		if(rc == SOCKET_ERROR)
			return 0;
	} else for(i = 0; i < MAXCALLS; i++)
	{
		vpc = LockCall(i);
		if(vpc)
		{
			if(vpc->status == VPCS_CONNECTED && vpc->cantx)
				rc = tx.Send(&vpc->addr, buf, len);
			else rc = 0;
			vpc->Unlock();
			if(rc == SOCKET_ERROR)
				return 0;
		}
	}
	return 1;
}

int SIPSTACK::SetNotifyRoutine(void (*notify)(void *param, unsigned, unsigned, unsigned), void *param)
{
	NotifyRoutine = notify;
	NotifyParam = param;
	return 0;
}

int SIPSTACK::SetSyncNotifyRoutine(int (*notify)(void *param, unsigned, unsigned, unsigned), void *param)
{
	SyncNotifyRoutine = notify;
	SyncNotifyParam = param;
	return 0;
}

void SIPSTACK::Notify(int message, unsigned param1, unsigned param2)
{
	bool notify = true;

	Lprintf("Notify %d,%d,%d", message, param1, param2);
	if(message == VPMSG_CALLESTABLISHED || message == VPMSG_CALLACCEPTED)
	{
		SIPCALLDATA *vpc = LockCall((VPCALL)param1);
		if(vpc)
		{
			CallEstablished(vpc);
			vpc->Unlock();
		}
	} else if(message == VPMSG_CALLENDED)
	{
		SIPCALLDATA *vpc = LockCall((VPCALL)param1);
		if(vpc)
		{
			if(vpc->status == VPCS_TERMINATED)
				notify = false;
			else if(vpc->status && vpc->status < VPCS_TERMINATED)
			{
				vpc->status = VPCS_TERMINATED;
				CallEnded(vpc);
			}
			vpc->Unlock();
		}
	}
	if(notify && NotifyRoutine)
		NotifyRoutine(NotifyParam, message, param1, param2);
}

int SIPSTACK::NotifyRc(int message, unsigned param1, unsigned param2)
{
	if(SyncNotifyRoutine)
		return SyncNotifyRoutine(SyncNotifyParam, message, param1, param2);
	return 0;
}

void SIPSTACK::TransmitThread(void *dummy)
{
	while(sock)
	{
		Sleep(10);
		PeriodicalUpdateServerAndAwake();
//		AutoAttendantProcess();
	}
	TRETURN
}

VPCALL SIPSTACK::FindRegisterCall()
{
	unsigned i;
	VPCALL vpcall;

	ReadLock();
	for(i = 0; i < MAXCALLS; i++)
	{
		if(calls[i] && calls[i]->connecttype == CT_REGISTER)
		{
			vpcall = calls[i]->vpcall;
			ReadUnlock();
			return vpcall;

		}
	}
	ReadUnlock();
	return 0;
}

void SIPSTACK::PeriodicalUpdateServerAndAwake()
{
	SIP_ELEMENTS se;
	VPCALL vpcall;
	SIPCALLDATA *vpc;

	if(!*settings.server || !*settings.username || !*settings.password)
		return;
	if(GetTickCount() - nextregistertm >= 0x80000000)
		return;
	if(settings.undernat && !natresolved || !sock)
		return;
	nextregistertm = GetTickCount() + settings.notifyinterval;
	vpcall = FindRegisterCall();
	if(!vpcall)
		vpcall = CreateCall();
	if(vpcall)
	{
		vpc = LockCall(vpcall);
		if(vpc)
		{
			vpc->nc++;
			vpc->connecttype =  CT_REGISTER;
			vpc->rtpnatok = true;
			vpc->Unlock();
			Connect(vpcall, "", 0);
		}
	}
}

void SIPSTACK::CallEstablished(SIPCALLDATA *vpc)
{
	ncalls++;
	vpc->start_ticks = GetTickCount();
	time(&vpc->start_time);
	if(vpaudiodatafactory)
		vpc->audio = vpaudiodatafactory->New(this, vpc->vpcall, vpc->codec);
}

void SIPSTACK::CallEnded(SIPCALLDATA *vpc)
{
	if(vpc->audio)
	{
		if(vpc->audiolocked)
			vpc->destroyaudio = true;
		else {
			delete vpc->audio;
			vpc->audio = 0;
		}
	}
	ncalls--;
}

void GetLocalAddrFromSTUN(const BYTE *buf, BYTE *localaddr, BYTE *localport)
{
	int msglen = buf[2] << 8 | buf[3], i;

	if(msglen > 1000)
		msglen = 1000;
	i = 20;
	while(i + 12 <= msglen)
	{
		if(buf[i] == 0 && buf[i+1] == 1)	// Mapped address attribute
		{
			memcpy(localaddr, buf + i + 8, 4);
			localport[0] = buf[i+6];
			localport[1] = buf[i+7];
			break;
		}
		i += 4 + (buf[2] << 8 | buf[3]);
	}
}

void SIPSTACK::ReceiveThread(void *dummy)
{
	BYTE buf[1501];
	struct sockaddr_in addr;
	int len, i, rc;
	socklen_t addrlen;
	timeval tv;
	fd_set fs;
	unsigned tm;
	SIPCALLDATA *vpc;
	VPCALL vpcall;

	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
	while(sock)
	{
		tm = GetTickCount();
		// This is not neceassary, as it is already done in RTPReceiver
/*		for(i = 0; i < MAXCALLS; i++)
		{
			vpc = LockCall(i);
			if(vpc)
			{
				if(!vpc->isregister && vpc->awaketm && tm - vpc->awaketm > 10000)
				{
					vpcall = vpc->vpcall;
					vpc->Unlock();
					Disconnect(vpcall, REASON_CONNECTIONLOST);
				} else vpc->Unlock();
			}
		}*/
		addrlen = sizeof(addr);
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		FD_ZERO(&fs);
		FD_SET(sock, &fs);
		select((int)sock + 1, &fs, 0, 0, &tv);
		if(!sock)
			break;
		if(!FD_ISSET(sock, &fs))
		{
			RateCount(&rxxc, 0);
			RateCount(&tx.xc, 0);
			continue;
		}
		len = recvfrom(sock, (char *)buf, 1500, 0, (sockaddr *)&addr, &addrlen);
		if(len < 0)
			rc = WSAGetLastError();
		else rc = 0;
		if(settings.undernat && addr.sin_addr.s_addr == settings.natserveraddr.sin_addr.s_addr &&
			addr.sin_port == settings.natserveraddr.sin_port && (BYTE)*buf == PKT_NATDISCOVERACK &&
			len >= 12 && buf[1] == 7 && (BYTE)buf[2] == IE_INADDR_HOST && buf[9] == 2 && (BYTE)buf[10] == IE_REASON && buf[11] == REASON_NORMAL)
		{
			memcpy(fulllocaladdr, buf + 3, 4);
			memcpy(fulllocaladdr + 4, buf + 7, 2);
			BuildLocalAddr();
			natresolved = true;
		}
		if(settings.undernat && addr.sin_addr.s_addr == settings.stunserveraddr.sin_addr.s_addr &&
			addr.sin_port == settings.stunserveraddr.sin_port && buf[0] == 1 && buf[1] == 1)
		{
			GetLocalAddrFromSTUN(buf, fulllocaladdr, fulllocaladdr + 4);
			BuildLocalAddr();
			natresolved = true;
		}
		if(len > 0)
			buf[len] = 0;
		if(generatelog)
		{
			if(len > 0)
			{
				Lprintf("%u %s:%d Received %d bytes:", GetTickCount(), inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), len);
				if(*buf & 0x80)
				{
					if(generatelog >= 2)
					{
						char s[3000], *p;

						strcpy(s, "RAW ");
						p = s + 4;
						for(i = 0; i < len; i++)
						{
							sprintf(p, i == len - 1 ? "%02x" : "%02x,", ((BYTE *)buf)[i]);
							p += strlen(p);
						}
						Lprintf("%s", s);
					}
				} else LogAsIs((const char *)buf, len);
			} else {
				Lprintf("%u %s:%d Error", GetTickCount(), inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), len);
				buf[0] = 0;
			}
		}
		if(len == -1 && rc == WSAECONNRESET)
		{
			for(i = 0; i < MAXCALLS; i++)
			{
				vpc = LockCall(i);
				if(vpc)
				{
					if((vpc->status == VPCS_CONNECTED || vpc->status == VPCS_HELD) && addr.sin_addr.s_addr == vpc->addr.sin_addr.s_addr && addr.sin_port == vpc->addr.sin_port)
					{
						vpcall = vpc->vpcall;
						vpc->Unlock();
						Lprintf("Call SIP[%d] disconnected, because a connection reset was received", vpcall);
						NotifyOnCall(VPMSG_CALLENDED, vpcall, REASON_CONNECTIONLOST);
					} else vpc->Unlock();
				}
			}
		}
		if(len < 1)
			continue;
		RateCount(&rxxc, len+UDPIPHLEN);
		ProcessReceiverMessage(&addr, (char *)buf, len);
	}
	TRETURN
}

void SIPSTACK::BuildLocalAddr()
{
	if(*settings.username)
		sprintf(sfulllocaladdr, "%s@%d.%d.%d.%d:%d", settings.username, fulllocaladdr[0], fulllocaladdr[1], fulllocaladdr[2], fulllocaladdr[3], fulllocaladdr[4] * 256 + fulllocaladdr[5]);
	else sprintf(sfulllocaladdr, "%d.%d.%d.%d:%d", fulllocaladdr[0], fulllocaladdr[1], fulllocaladdr[2], fulllocaladdr[3], fulllocaladdr[4] * 256 + fulllocaladdr[5]);
}

void SIPSTACK::BuildLocalSEPart(SIP_ELEMENTS *se)
{
	BuildLocalAddr();
	memcpy(se->fulllocaladdr, fulllocaladdr, 6);
	se->contact = sfulllocaladdr;
}

void SIPSTACK::RefuseCall(const struct sockaddr_in *addr, SIP_ELEMENTS *se, int reason)
{
	char pdu[1500];
	int len;

	BuildLocalSEPart(se);
	se->command = SIPCMD_INVITEACK;
	se->reason = reason;
	len = Format(pdu, se);
	Send(addr, pdu, len);
}

void SIPSTACK::AcceptCall(VPCALL vpcall, int answer)
{
	SIPCALLDATA *vpc = LockCall(vpcall);
	struct sockaddr_in addr;
	SIP_ELEMENTS se;
	char pdu[1500];
	int len, i;

	if(!vpc)
		return;

	CopyCallFields(&se, vpc, true);
	se.command = SIPCMD_INVITEACK;
	se.answer = answer;
	se.seqnumber = vpc->seqnumber;
	se.codec = vpc->codec;
	se.rtpaddr = vpc->lrtpaddr;
	se.sessionid = vpc->sessionid;
	se.sessionversion = vpc->sessionversion;
	if(vpc->held && vpc->hold)
		se.hold = HOLD_INACTIVE;
	else if(vpc->held)
		se.hold = HOLD_RECVONLY;
	else if(vpc->hold)
		se.hold = HOLD_SENDONLY;
	else se.hold = HOLD_NONE;
	for(i = 0; i < MAXVIA; i++)
		if(*vpc->route[i])
			se.route[i] = vpc->route[i];
	len = Format(pdu, &se);
	addr = vpc->addr;
	vpc->Unlock();

	Send(&addr, pdu, len);
}

int strcmpnoaddr(const char *s1, const char *s2)
{
	int i = 0;

	while(s1[i] && s1[i] != '@' && s1[i] == s2[i])
		i++;
	return s1[i] - s2[i];
}

SIPCALLDATA *SIPSTACK::FindCall(const char *callid)
{
	int i;

	if(!callid || !*callid)
		return 0;
	ReadLock();
	for(i = 0; i < MAXCALLS; i++)
	{
		// Only compares the part before '@'
		// Some NAT routers change the address after '@'
		if(calls[i] && !strcmpnoaddr(callid, calls[i]->callid))
		{
			SIPCALLDATA *vpc = calls[i];
			vpc->Lock();
			ReadUnlock();
			return vpc;
		}
	}
	ReadUnlock();
	return 0;
}

int SIPSTACK::EnumCalls(VPCALL *vpcalls, unsigned maxvpcalls, unsigned mask)
{
	unsigned n = 0, i;

	if(!(mask & (1 << BC_VOICE)))
		return 0;
	ReadLock();
	for(i = 0; i < MAXCALLS; i++)
	{
		if(calls[i] && (calls[i]->connecttype == CT_REGULAR || calls[i]->connecttype == CT_HOLDRESUME))
		{
			if(vpcalls && n < maxvpcalls)
				vpcalls[n] = calls[i]->vpcall;
			if(!vpcalls || n < maxvpcalls)
				n++;
		}
	}
	ReadUnlock();
	return n;
}

void SIPSTACK::DisconnectAck(const struct sockaddr_in *addr, SIP_ELEMENTS *se, int cmd)
{
	char pdu[1500];
	int len;

	se->command = cmd;
	BuildLocalSEPart(se);
	len = Format(pdu, se);

	Send(addr, pdu, len);
}

void SIPSTACK::UnknownMethod(const struct sockaddr_in *addr, SIP_ELEMENTS *se)
{
	char pdu[1500];
	int len;

	se->command = SIPCMD_UNKNOWNMETHOD;
	BuildLocalSEPart(se);
	len = Format(pdu, se);

	Send(addr, pdu, len);
}

void SIPSTACK::RegisterAck(const struct sockaddr_in *addr, SIP_ELEMENTS *se, int reason)
{
	char pdu[1500];
	int len;

	se->command = SIPCMD_REGISTERACK;
	if(!reason)
		se->answer = ANSWER_ACCEPT;
	else {
		se->answer = ANSWER_REFUSE;
		se->reason = reason;
	}
	BuildLocalSEPart(se);
	len = Format(pdu, se);

	Send(addr, pdu, len);
}

void SIPSTACK::InfoAck(const struct sockaddr_in *addr, SIP_ELEMENTS *se)
{
	char pdu[1500];
	int len;

	se->command = SIPCMD_INFOACK;
	BuildLocalSEPart(se);
	len = Format(pdu, se);

	Send(addr, pdu, len);
}

void SIPSTACK::SendInviteAck(const struct sockaddr_in *addr, SIP_ELEMENTS *se)
{
	char pdu[1500];
	int len;

	if(se->command == SIPCMD_REGISTERACK)
	{
		se->command = SIPCMD_ACK_REGISTER;
		se->destaddr = se->toaddr;
	} else {
		se->command = SIPCMD_ACK_INVITE;
		if(se->contact)
			se->destaddr = se->contact;
		else se->destaddr = se->toaddr;
	}
	BuildLocalSEPart(se);
	len = Format(pdu, se);
	Send(addr, pdu, len);
}

void SIPSTACK::SendInviteNAck(const struct sockaddr_in *addr, SIP_ELEMENTS *se)
{
	char pdu[1500];
	int len;

	se->command = SIPCMD_BYE;
	se->seqnumber++;
	if(se->contact)
		se->destaddr = se->contact;
	else se->destaddr = se->toaddr;
	BuildLocalSEPart(se);
	len = Format(pdu, se);
	Send(addr, pdu, len);
}

void SIPSTACK::ProcessReceiverMessage(const struct sockaddr_in *addr, char *buf, int buflen)
{
	char iebuf[IEBUFSIZE], *p;
	SIP_ELEMENTS se1, *se = &se1;
	VPCALL vpcall;
	SIPCALLDATA *vpc;
	int i, rc, codec = 0, notifymsg;
	bool codecgood;

	if(ParseToIE(buf, iebuf, se))
		return;
	switch(se->command)
	{
	case SIPCMD_INVITE:
		codecgood = true;
		if(se->bc_codecs & (1<<CODEC_ILBC30) && settings.supported_codecs & (1<<CODEC_ILBC30))
			codec = CODEC_ILBC30;
		else if(se->bc_codecs & (1<<CODEC_ILBC20) && settings.supported_codecs & (1<<CODEC_ILBC20))
			codec = CODEC_ILBC20;
		else if(se->bc_codecs & (1<<CODEC_GSMFR) && settings.supported_codecs & (1<<CODEC_GSMFR))
			codec = CODEC_GSMFR;
		else if(se->bc_codecs & (1<<CODEC_G711A) && settings.supported_codecs & (1<<CODEC_G711A))
			codec = CODEC_G711A;
		else if(se->bc_codecs & (1<<CODEC_G711U) && settings.supported_codecs & (1<<CODEC_G711U))
			codec = CODEC_G711U;
		else if(se->bc_codecs & (1<<CODEC_G729) && settings.supported_codecs & (1<<CODEC_G729))
			codec = CODEC_G729;
		else codecgood = false;
		vpc = FindCall(se->callid);
		if(vpc)
		{
			i = vpc->status;
			vpcall = vpc->vpcall;
			vpc->Unlock();
			switch(i)
			{
			case VPCS_INCONNECTING:
				AcceptCall(vpcall, ANSWER_ACCEPT);
				break;
			case VPCS_INALERTED:
				AcceptCall(vpcall, ANSWER_USERALERTED);
				break;
			case VPCS_CONNECTED:
				vpc = FindCall(se->callid);
				if(vpc)
				{
					VPAUDIODATA *audio = 0;

					for(i = 0; i < MAXVIA; i++)
					{
						strncpy2(vpc->viabranch[i], se->viabranch[i], MAXBRANCHLEN+1);
						strncpy2(vpc->viaaddr[i], se->viaaddr[i], MAXSIPADDRLEN+1);
						vpc->viarport[i] = se->viarport[i];
					}
					if(se->hold == HOLD_SENDONLY || se->hold == HOLD_INACTIVE)
						vpc->held = true;
					else vpc->held = false;
					vpc->sessionversion++;
					vpc->seqnumber = se->seqnumber;
					if(codecgood && vpc->codec != codec)
					{
						vpc->codec = codec;
						if(vpc->audio)
						{
							audio = vpc->audio;
							vpc->audiolocked = true;
						}
						vpc->keypadmap = se->keypadmap;
						vpc->iLBC20map = se->iLBC20map;
						vpc->iLBC30map = se->iLBC30map;
					}
					if(se->rtpaddr.sin_addr.s_addr && se->rtpaddr.sin_port)
						vpc->rrtpaddr = se->rtpaddr;
					if(settings.remotenat == 2)
						vpc->rrtpaddr.sin_addr = addr->sin_addr;
					Lprintf("RTP socket for call %d is transmitting to port %d", vpc->vpcall, ntohs(vpc->rrtpaddr.sin_port));
					vpc->addr = *addr;
					vpc->Unlock();
					if(audio)
					{
						audio->ChangeCodec(codec);
						vpc = LockCall(vpcall);
						if(vpc)
						{
							vpc->audiolocked = false;
							vpc->Unlock();
						}
					}
				}
				AcceptCall(vpcall, ANSWER_ACCEPT);
				break;
			default:
				RefuseCall(addr, se, REASON_BUSY);
				break;
			}
			break;
		}
		if(!codecgood)
		{
			RefuseCall(addr, se, REASON_NORMAL);
			break;
		}
		rc = IncomingCall(addr, se);
		if(rc <= 0)
		{
			RefuseCall(addr, se, -rc);
			break;
		}
		vpcall = CreateCall();
		if(!vpcall)
		{
			RefuseCall(addr, se, REASON_BUSY);
			break;
		}
		vpc = LockCall(vpcall);
		if(!vpc)
			break;
		vpc->codec = codec;
		vpc->keypadmap = se->keypadmap;
		vpc->iLBC20map = se->iLBC20map;
		vpc->iLBC30map = se->iLBC30map;
		vpc->rrtpaddr = se->rtpaddr;
		if(settings.remotenat == 2)
			vpc->rrtpaddr.sin_addr = addr->sin_addr;
		Lprintf("RTP socket for call %d is transmitting to port %d", vpc->vpcall, ntohs(vpc->rrtpaddr.sin_port));
		vpc->addr = *addr;
		strncpy2(vpc->callid, se->callid, MAXCALLIDLEN+1);
		strncpy2(vpc->connectedaddr, se->fromaddr, MAXSIPADDRLEN+1);
		if(se->fromname)
			strncpy2(vpc->connectedname, se->fromname, MAXNAMELEN+1);
		else *vpc->connectedname = 0;
		strncpy2(vpc->connectedtag, se->fromtag, MAXTAGLEN+1);
		if(se->toname)
			strncpy2(vpc->ourname, se->toname, MAXNAMELEN+1);
		else {
			strncpy2(vpc->ourname, se->toaddr, MAXNAMELEN+1);
			p = strchr(vpc->ourname, '@');
			if(p)
				*p = 0;
		}
		strncpy2(vpc->ouraddr, se->toaddr, MAXSIPADDRLEN+1);

//		strncpy2(vpc->contact, se->contact, MAXCONTACTLEN+1);
		for(i = 0; i < MAXVIA; i++)
			strncpy2(vpc->route[i], se->route[i], MAXROUTELEN+1);
		for(i = 0; i < MAXVIA; i++)
		{
			strncpy2(vpc->viabranch[i], se->viabranch[i], MAXBRANCHLEN+1);
			strncpy2(vpc->viaaddr[i], se->viaaddr[i], MAXSIPADDRLEN+1);
			vpc->viarport[i] = se->viarport[i];
		}
		sprintf(vpc->ourtag, "T%u", GetTickCount());
		vpc->seqnumber = se->seqnumber;
		strncpy2(vpc->address, se->contact, MAXADDRLEN+1);

		vpc->status = rc == 1 ? VPCS_INCONNECTING : VPCS_INALERTED;
		vpc->Unlock();
		if(rc == 1)
			AcceptCall(vpcall, ANSWER_ACCEPT);
		else AcceptCall(vpcall, ANSWER_USERALERTED);
		NotifyOnCall(VPMSG_NEWCALL, vpcall, 0);
		break;
	case SIPCMD_OPTIONS:
		{
			char pdu[1500];
			int len;

			BuildLocalSEPart(se);
			se->command = SIPCMD_OPTIONSACK;
			len = Format(pdu, se);
			Send(addr, pdu, len);
		}
		break;
	case SIPCMD_INVITEACK:
	case SIPCMD_REGISTERACK:
	case SIPCMD_REFERACK:
		vpc = FindCall(se->callid);
		if(!vpc)
		{
			if(se->answer == ANSWER_REFUSE)
				SendInviteAck(addr, se);
			else SendInviteNAck(addr, se);
			break;
		}
		vpcall = vpc->vpcall;
		notifymsg = 0;
		if(vpc->status == VPCS_CONNECTING || vpc->status == VPCS_CALLPROCEEDING || vpc->status == VPCS_ALERTED)
		{
			vpc->sipreason = se->sipreason;
			strncpy2(vpc->sipreasontext, se->sipreasontext, sizeof(vpc->sipreasontext));
			strncpy2(vpc->reasonline, se->reasonline, sizeof(vpc->reasonline));
			vpc->reason = se->reason;
			vpc->authtype = se->authtype;
			if(se->toaddr)
				strcpy(vpc->connectedaddr, se->toaddr);
			if(se->toname)
				strcpy(vpc->connectedname, se->toname);
			if(se->totag)
				strcpy(vpc->connectedtag, se->totag);
			if(se->answer == ANSWER_USERALERTED && se->reason == REASON_CALLPROCEEDING && vpc->status == VPCS_CONNECTING)
				vpc->status = VPCS_CALLPROCEEDING;
			else if(se->answer == ANSWER_ACCEPT ||
				se->answer == ANSWER_USERALERTED && !se->reason && se->rtpaddr.sin_port && (vpc->status == VPCS_CONNECTING || vpc->status == VPCS_CALLPROCEEDING))
			{
				if(se->answer == ANSWER_ACCEPT && se->reason == REASON_EARLYAUDIO)
					vpc->earlyaudio = true;
				if(se->hold == HOLD_RECVONLY || se->hold == HOLD_INACTIVE)
					vpc->hold = true;
				else vpc->hold = false;
				vpc->codec = se->codec;
				vpc->keypadmap = se->keypadmap;
				vpc->iLBC20map = se->iLBC20map;
				vpc->iLBC30map = se->iLBC30map;
				vpc->rrtpaddr = se->rtpaddr;
				if(settings.remotenat == 2)
					vpc->rrtpaddr.sin_addr = addr->sin_addr;
				Lprintf("RTP socket for call %d is transmitting to port %d", vpc->vpcall, ntohs(vpc->rrtpaddr.sin_port));
				if(se->contact)
					strcpy(vpc->contact, se->contact);
				for(i = 0; i < MAXVIA; i++)
					if(se->route[i])
						strcpy(vpc->route[i], se->route[i]);
				vpc->status = VPCS_CONNECTED;
				vpc->sipreason = 200;
				vpc->callwasestablished = true;
				if(vpc->connecttype == CT_HOLDRESUME)
				{
					notifymsg = vpc->hold ? VPMSG_HELD : VPMSG_RESUMED;
					vpc->status = vpc->hold ? VPCS_HELD : VPCS_CONNECTED;
				} else if(vpc->connecttype == CT_REGISTER)
					notifymsg = VPMSG_SERVERSTATUS;
				else notifymsg = se->answer == ANSWER_ACCEPT && !se->reason ? VPMSG_CALLACCEPTEDBYUSER : VPMSG_CALLACCEPTED;
				vpc->awaketm = GetTickCount();
				vpc->cantx = true;
				rc = 1;
			} else if(se->answer == ANSWER_USERALERTED && !se->reason && (vpc->status == VPCS_CONNECTING || vpc->status == VPCS_CALLPROCEEDING))
			{
				vpc->status = VPCS_ALERTED;
				if(vpc->connecttype == CT_REGULAR)
					notifymsg = VPMSG_USERALERTED;
			} else if(se->answer == ANSWER_REFUSE)
			{
				if(se->reason == REASON_AUTHERROR && se->nonce)
				{
					vpc->seqnumber = PickSeqNumber();
					if(strcmp(vpc->nonce, se->nonce))
					{
						strncpy2(vpc->nonce, se->nonce, MAXNONCELEN+1);
						vpc->nc = 1;
					}
					if(se->realm)
						strncpy2(vpc->realm, se->realm, MAXREALMLEN+1);
					if(se->opaque)
						strncpy2(vpc->opaque, se->opaque, MAXOPAQUELEN+1);
					vpc->qop = se->qop;
					vpc->restartconnect = true;
					vpc->status = VPCS_CONNECTING;
				} else {
					if(vpc->connecttype == CT_REGISTER)
						notifymsg = VPMSG_SERVERSTATUS;
					else if(vpc->connecttype == CT_REGULAR)
						notifymsg = VPMSG_CALLREFUSED;
					else notifymsg = VPMSG_REQUESTREFUSED;
					vpc->reason = se->reason;
				}
			}
		} else if(vpc->status == VPCS_CONNECTED && se->answer == ANSWER_ACCEPT && !se->reason && vpc->earlyaudio)
		{
			vpc->earlyaudio = false;
			notifymsg = VPMSG_CALLACCEPTEDBYUSER;
			rc = 0;
		} else if(vpc->status == VPCS_CONNECTED && se->answer == ANSWER_REFUSE)
		{
			vpc->sipreason = se->sipreason;
			strncpy2(vpc->sipreasontext, se->sipreasontext, sizeof(vpc->sipreasontext));
			strncpy2(vpc->reasonline, se->reasonline, sizeof(vpc->reasonline));
			vpc->reason = se->reason;
			notifymsg = VPMSG_CALLREFUSED;
		} else if(vpc->status == VPCS_DISCONNECTING && se->answer == ANSWER_REFUSE && se->reason == REASON_DISCONNECTED)
		{
			vpcall = vpc->vpcall;
			se->reason = vpc->reason;
			notifymsg = VPMSG_CALLENDED;
		} else if(vpc->status == VPCS_DISCONNECTING)
			SendInviteNAck(addr, se);
		vpc->Unlock();
		if(notifymsg == VPMSG_SERVERSTATUS)
		{
			if(se->answer == ANSWER_ACCEPT)
			{
				loggedon = true;
				Notify(VPMSG_SERVERSTATUS, REASON_LOGGEDON, 0);
			} else Notify(VPMSG_SERVERSTATUS, se->reason, 0);
		} else if(notifymsg)
		{
			if(notifymsg == VPMSG_CALLACCEPTEDBYUSER && rc)
				NotifyOnCall(VPMSG_CALLACCEPTED, vpcall, se->reason);
			NotifyOnCall(notifymsg, vpcall, se->reason);
			if(notifymsg == VPMSG_CALLREFUSED)
				NotifyOnCall(VPMSG_CALLENDED, vpcall, se->reason);
		}
		if(se->answer == ANSWER_ACCEPT && se->reason != REASON_EARLYAUDIO || se->answer == ANSWER_REFUSE)
			SendInviteAck(addr, se);
		break;
	case SIPCMD_ACK_INVITE:
		vpc = FindCall(se->callid);
		if(!vpc)
			break;
		if(vpc->status != VPCS_INCONNECTING)
		{
			vpc->Unlock();
			break;
		}
		vpc->status = VPCS_CONNECTED;
		vpc->sipreason = 0;
		vpc->callwasestablished = true;
		vpc->awaketm = GetTickCount();
		vpc->cantx = true;
		vpcall = vpc->vpcall;
		vpc->Unlock();
		NotifyOnCall(VPMSG_CALLESTABLISHED, vpcall, 0);
		break;
	case SIPCMD_BYE:
	case SIPCMD_CANCEL:
		DisconnectAck(addr, se, se->command == SIPCMD_BYE ? SIPCMD_BYEACK : SIPCMD_CANCELACK);
		vpc = FindCall(se->callid);
		if(!vpc)
			break;
		vpcall = vpc->vpcall;
		vpc->Unlock();
		NotifyOnCall(VPMSG_CALLDISCONNECTED, vpcall, se->reason);
		NotifyOnCall(VPMSG_CALLENDED, vpcall, se->reason);
		break;
	case SIPCMD_BYEACK:
	case SIPCMD_CANCELACK:
		vpc = FindCall(se->callid);	// This answer seems not enough, the server sent it, but it didn't complete the CANCEL
		if(!vpc)
			break;
		vpcall = vpc->vpcall;
		rc = vpc->reason;
		vpc->Unlock();
		NotifyOnCall(VPMSG_CALLENDED, vpcall, rc);
		break;
	case SIPCMD_INFO:
		vpc = FindCall(se->callid);
		if(!vpc)
			break;
		vpcall = vpc->vpcall;
		if(vpc->lastkeypadseqnumber != se->seqnumber)
		{
			vpc->lastkeypadseqnumber = se->seqnumber;
			rc = se->keypadmap;
		} else rc = 0;
		vpc->Unlock();
		InfoAck(addr, se);
		if(rc)
			NotifyOnCall(VPMSG_KEYPAD, vpcall, se->keypadmap);
		break;
	case SIPCMD_REGISTER:
		RegisterAck(addr, se, 0);
		break;
	case SIPCMD_UNKNOWN:
		UnknownMethod(addr, se);
		break;
	}
}

void SIPSTACK::IncomingCallThread(void *vpcall1)
{
	VPCALL vpcall = (VPCALL)vpcall1;
	SIPCALLDATA *vpc;
	unsigned ticks = GetTickCount(), nextsendticks = ticks;
	int status, i;

	// Wait to know our exact port
	for(i = 0; i < 50; i++)
	{
		vpc = LockCall(vpcall);
		if(!vpc)
			TRETURN
		if(vpc->rtpnatok)
		{
			vpc->Unlock();
			break;
		}
		vpc->Unlock();
		Sleep(100);
	}
	while(GetTickCount() - ticks < 5000)
	{
		vpc = LockCall(vpcall);
		if(!vpc)
			TRETURN
		if(vpc->status != VPCS_INCONNECTING)
		{
			status = vpc->status;
			vpc->Unlock();
			break;
		}
		status = vpc->status;
		vpc->Unlock();
		if(GetTickCount() >= nextsendticks)
		{
			AcceptCall(vpcall, ANSWER_ACCEPT);
			nextsendticks = GetTickCount() + 1000;
		}
		Sleep(10);
	}
	if(status != VPCS_CONNECTED)
	{
		vpc = LockCall(vpcall);
		if(!vpc)
			TRETURN
		vpc->status = VPCS_CONNECTED;
		vpc->sipreason = 0;
		vpc->callwasestablished = true;
		vpc->awaketm = GetTickCount();
		vpc->cantx = true;
		vpcall = vpc->vpcall;
		vpc->Unlock();
		NotifyOnCall(VPMSG_CALLESTABLISHED, vpcall, 0);
//		NotifyOnCall(VPMSG_CALLSETUPFAILED, vpcall, 0);
//		NotifyOnCall(VPMSG_CALLENDED, vpcall, 0);
	}
	TRETURN
}

void SIPSTACK::DisconnectThread(void *vpcall1)
{
	VPCALL vpcall = (VPCALL)vpcall1;
	int i, reason = REASON_NORMAL;
	SIPCALLDATA *vpc;
	SIP_ELEMENTS se;
	struct sockaddr_in addr;
	char pdu[1500];
	int len;

	for(i = 0; i < 5; i++)
	{
		vpc = LockCall(vpcall);
		if(!vpc)
			TRETURN
		addr = vpc->addr;
		CopyCallFields(&se, vpc, false);
		se.destaddr = vpc->address;
		if(vpc->callwasestablished && !vpc->earlyaudio)
		{
			if(i == 0)
				vpc->seqnumber++;
			se.command = SIPCMD_BYE;
			if(*vpc->contact)
				se.destaddr = vpc->contact;
		} else {
			se.totag = 0;
			se.command = SIPCMD_CANCEL;
		}
		for(i = 0; i < MAXVIA; i++)
			if(*vpc->route[i])
				se.route[i] = vpc->route[i];
		se.seqnumber = vpc->seqnumber;
		len = Format(pdu, &se);
		vpc->Unlock();
		Send(&addr, pdu, len);
		Sleep(1000);
	}
	NotifyOnCall(VPMSG_CALLENDED, vpcall, reason);
	TRETURN
}

void SIPSTACK::ConnectThread(void *vpcall1)
{
	VPCALL vpcall = (VPCALL)vpcall1;
	SIPCALLDATA *vpc;
	int i, j, status = VPCS_CONNECTING, reason = REASON_NORMAL;
	CONNECTTYPE connecttype;

	// Wait to know our exact port
	for(i = 0; i < 50; i++)
	{
		vpc = LockCall(vpcall);
		if(!vpc)
			TRETURN
		if(vpc->rtpnatok)
		{
			vpc->Unlock();
			break;
		}
		vpc->Unlock();
		Sleep(100);
	}
	for(i = 0; i < 5; i++)
	{
		SIP_ELEMENTS se;
		struct sockaddr_in addr;
		char pdu[1500];
		int len;

		vpc = LockCall(vpcall);
		if(!vpc)
			TRETURN

		connecttype = vpc->connecttype;
		if(vpc->status != VPCS_CONNECTING)
		{
			status = vpc->status;
			reason = vpc->reason;
			vpc->Unlock();
			break;
		}
		if(!i)
			vpc->seqnumber = PickSeqNumber();
		addr = vpc->addr;
		CopyCallFields(&se, vpc, false);
		if(*settings.authusername)
			se.username = settings.authusername;
		if(vpc->connecttype == CT_REGISTER)
		{
			se.command = SIPCMD_REGISTER;
			se.expire = settings.notifyinterval * 19 / 6;
			sprintf(vpc->connectedaddr, "%s@%s", settings.username, settings.domain);
			se.fromaddr = se.toaddr;
			se.toname = settings.username;
		} else if(vpc->connecttype == CT_REFER)
			se.command = SIPCMD_REFER;
		else se.command = SIPCMD_INVITE;
		se.destaddr = vpc->address;
		se.seqnumber = vpc->seqnumber;
		se.answer = ANSWER_ACCEPT;
		se.bc_codecs = settings.supported_codecs;
		se.codec = vpc->codec;
		se.rtpaddr = vpc->lrtpaddr;
		se.sessionid = vpc->sessionid;
		se.sessionversion = vpc->sessionversion;
		if(vpc->hold)
			se.hold = HOLD_SENDONLY;
		for(j = 0; j < MAXVIA; j++)
			if(*vpc->route[j])
				se.route[j] = vpc->route[j];
		if(*vpc->nonce)
		{
			char s[300], digest1[33], digest2[33];

			if(*settings.authusername)
				strcpy(s, settings.authusername);
			else strcpy(s, vpc->ourorigname);
			strcat(s, ":");
			strcat(s, vpc->realm);
			strcat(s, ":");
			strcat(s, vpc->password);
			MD5(s, digest1);
			if(vpc->connecttype == CT_REGISTER)
				strcpy(s, "REGISTER:sip:");
			else if(vpc->connecttype == CT_REFER)
				strcpy(s, "REFER:sip:");
			else strcpy(s, "INVITE:sip:");
			strcat(s, se.destaddr);
			MD5(s, digest2);
			strcpy(s, digest1);
			strcat(s, ":");
			strcat(s, vpc->nonce);
			strcat(s, ":");
			if(vpc->qop == QOP_AUTH)
			{
				RandomString128(vpc->cnonce);
				sprintf(s + strlen(s), "%08x:%s:auth:", vpc->nc, vpc->cnonce);
			}
			strcat(s, digest2);
			MD5(s, digest1);
			if(vpc->connecttype == CT_REGULAR || vpc->connecttype == CT_REGISTER)
				se.totag = 0;
			se.nonce = vpc->nonce;
			if(*vpc->cnonce)
				se.cnonce = vpc->cnonce;
			se.qop = vpc->qop;
			se.nc = vpc->nc;
			if(*settings.authusername)
				se.username = settings.authusername;
			else se.username = vpc->ourorigname;
			se.realm = vpc->realm;
			if(*vpc->opaque)
				se.opaque = vpc->opaque;
			se.response = digest1;
			se.authtype = vpc->authtype == 1 ? 3 : 2;
		}
		len = Format(pdu, &se);

		vpc->Unlock();
		Send(&addr, pdu, len);
		for(j = 0; j < 10; j++)
		{
			Sleep(100);
			vpc = LockCall(vpcall);
			if(!vpc)
				TRETURN
			if(vpc->restartconnect)
			{
				vpc->restartconnect = false;
				vpc->Unlock();
				break;
			} else {
				reason = vpc->reason;
				vpc->Unlock();
			}
		}
	}
	if(connecttype == CT_REFER)
	{
		vpc = LockCall(vpcall);
		if(!vpc)
			TRETURN
		vpc->connecttype = CT_REGULAR;
		vpc->Unlock();
	}
	if(connecttype == CT_REGISTER)
	{
		if(status == VPCS_CONNECTING)
			NotifyOnCall(VPMSG_SERVERSTATUS, reason, 0);
	} else {
		if(status == VPCS_CONNECTING)
			NotifyOnCall(VPMSG_CONNECTTIMEOUT, vpcall, 0);
		NotifyOnCall(VPMSG_CONNECTFINISHED, vpcall, 0);
		if(connecttype == CT_REGULAR && status != VPCS_HELD && status != VPCS_CONNECTED && status != VPCS_CALLPROCEEDING && status != VPCS_ALERTED && status != VPCS_DISCONNECTING)
			NotifyOnCall(VPMSG_CALLENDED, vpcall, 0);
	}
	TRETURN
}

int SIPSTACK::SetNATServer(const char *addr)
{
	natresolved = false;
	if(!addr || !*addr)
	{
		settings.undernat = 0;
		Zero(settings.natserveraddr);
		return 0;
	}
	if(Resolve(addr, &settings.natserveraddr, 11675))
	{
		BYTE buf = PKT_NATDISCOVER;
		
		settings.undernat = true;
		Send(&settings.natserveraddr, (char *)&buf, 1);
		return 0;
	}
	return -1;
}

int SIPSTACK::SetSTUNServer(const char *addr)
{
	natresolved = false;
	if(!addr || !*addr)
	{
		settings.undernat = 0;
		Zero(settings.stunserveraddr);
		return 0;
	}
	if(Resolve(addr, &settings.stunserveraddr, 3478))
	{
		BYTE buf[20];

		settings.undernat = true;
		buf[0] = 0;
		buf[1] = 1;
		buf[2] = 0;
		buf[3] = 0;
		memset(buf + 4, 0, 16);
		Send(&settings.stunserveraddr, (char *)buf, 20);
		return 0;
	}
	return -1;
}

int SIPSTACK::SetServers(const char *list)
{
	const char *p;
	int len;

	p = strchr(list, ';');
	if(!p)
		p = strchr(list, ',');
	if(p)
	{
		len = p-list;
		if(len > MAXADDRLEN)
			len = MAXADDRLEN;
		memcpy(settings.server, list, len);
		settings.server[len] = 0;
		strncpy2(settings.domain, p+1, MAXADDRLEN+1);
	} else {
		strncpy2(settings.domain, list, MAXADDRLEN+1);
		strcpy(settings.server, settings.domain);
	}
	return 0;
}

int SIPSTACK::Logon(const char *username, const char *password)
{
	const char *p;
	int len;
	VPCALL vpcall;

	p = strchr(username, ';');
	if(!p)
		p = strchr(username, ',');
	if(p)
	{
		len = p - username;
		if(len > MAXNAMELEN)
			len = MAXNAMELEN;
		memcpy(settings.username, username, len);
		settings.username[len] = 0;
		strncpy2(settings.authusername, p+1, MAXNAMELEN+1);
	} else {
		strncpy2(settings.username, username, MAXNAMELEN+1);
		*settings.authusername = 0;
	}
	strncpy2(settings.password, password, MAXNAMELEN+1);
	vpcall = FindRegisterCall();
	if(vpcall)
		FreeCall(vpcall);
	loggedon = false;
	nextregistertm = GetTickCount();
	BuildLocalAddr();
	return 0;
}

int SIPSTACK::Logoff()
{
	if(!loggedon)
		return VPERR_NOTLOGGEDON;
	return 0;
}

const char *SIPSTACK::LogonName()
{
	return settings.username;
}

const char *SIPSTACK::LogonNumber()
{
	return settings.username;
}

int SIPSTACK::IsLoggedOn()
{
	if(loggedon)
		return 1;
	return 0;
}

void SIPSTACK::RTPReceiver(VPCALL vpcall)
{
	SIPCALLDATA *vpc;
	SOCKET sk;
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	int	len;
	char buf[1500];
	timeval tv;
	fd_set fs;

	vpc = LockCall(vpcall);
	if(!vpc)
		TRETURN
	sk = vpc->rtpsk;
	vpc->Unlock();
	if(settings.undernat)
	{
		if(settings.natserveraddr.sin_port)
		{
			buf[0] = (char)PKT_NATDISCOVER;
			if(nbsendto(sk, buf, 1, 0, (struct sockaddr *)&settings.natserveraddr, sizeof(struct sockaddr_in)) > 0)
				RateCount(&tx.xc, 1+UDPIPHLEN);
		}
		if(settings.stunserveraddr.sin_port)
		{
			buf[0] = 0;
			buf[1] = 1;
			buf[2] = 0;
			buf[3] = 0;
			memset(buf + 4, 0, 16);
			if(nbsendto(sk, buf, 20, 0, (struct sockaddr *)&settings.stunserveraddr, sizeof(struct sockaddr_in)) > 0)
				RateCount(&tx.xc, 20+UDPIPHLEN);
		}
	}
	for(;;)
	{
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		FD_ZERO(&fs);
		FD_SET(sk, &fs);
		select((int)sk + 1, &fs, 0, 0, &tv);
		if(!sk)
			break;
		if(!FD_ISSET(sk, &fs))
		{
			vpc = LockCall(vpcall);
			if(!vpc)
				TRETURN
			if(vpc->connecttype != CT_REGISTER && vpc->awaketm && GetTickCount() - vpc->awaketm > 600000)
			{
				vpcall = vpc->vpcall;
				vpc->Unlock();
				Lprintf("Call SIP[%d] disconnected, because nothing was received from the party for 10 minutes", vpcall);
				Disconnect(vpcall, REASON_CONNECTIONLOST);
				TRETURN
			} else vpc->Unlock();
			continue;
		}
		len = recvfrom(sk, buf, 1500, 0, (struct sockaddr *)&addr, &addrlen);
		if(len <= 0)
		{
			vpc = LockCall(vpcall);
			if(!vpc)
				TRETURN
			vpc->Unlock();
			Sleep(10);
			continue;
		}
		if(generatelog >= 2)
		{
			if(len > 0)
			{
				Lprintf("%u %s:%d Received %d bytes:", GetTickCount(), inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), len);
			} else {
				Lprintf("%u %s:%d Error", GetTickCount(), inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), len);
				buf[0] = 0;
			}
			if(*buf & 0x80)
			{
				char s[3000], *p;
				int i;

				strcpy(s, "RAW ");
				p = s + 4;
				for(i = 0; i < len; i++)
				{
					sprintf(p, i == len - 1 ? "%02x" : "%02x,", ((BYTE *)buf)[i]);
					p += strlen(p);
				}
				Lprintf("%s", s);
			} else LogAsIs((const char *)buf, len);
		}
		RateCount(&rxxc, len+UDPIPHLEN);
		vpc = LockCall(vpcall);
		if(!vpc)
			TRETURN
		if(settings.undernat && addr.sin_addr.s_addr == settings.natserveraddr.sin_addr.s_addr &&
			addr.sin_port == settings.natserveraddr.sin_port && (BYTE)*buf == PKT_NATDISCOVERACK &&
			len >= 12 && buf[1] == 7 && (BYTE)buf[2] == IE_INADDR_HOST && buf[9] == 2 && (BYTE)buf[10] == IE_REASON && buf[11] == REASON_NORMAL)
		{
			memcpy(&vpc->lrtpaddr.sin_addr, buf + 3, 4);
			memcpy(&vpc->lrtpaddr.sin_port, buf + 7, 2);
			vpc->rtpnatok = true;
			Lprintf("RTP socket for call %d after NAT is on port %d", vpc->vpcall, ntohs(vpc->lrtpaddr.sin_port));
		}

		if(settings.undernat && addr.sin_addr.s_addr == settings.stunserveraddr.sin_addr.s_addr &&
			addr.sin_port == settings.stunserveraddr.sin_port && buf[0] == 1 && buf[1] == 1)
		{
			GetLocalAddrFromSTUN((BYTE *)buf, (BYTE *)&vpc->lrtpaddr.sin_addr, (BYTE *)&vpc->lrtpaddr.sin_port);
			vpc->rtpnatok = true;
			Lprintf("RTP socket for call %d after NAT is on port %d", vpc->vpcall, ntohs(vpc->lrtpaddr.sin_port));
		}

		if(!vpc->raddrwrong &&
			(vpc->rrtpaddr.sin_addr.s_addr != addr.sin_addr.s_addr ||
				vpc->rrtpaddr.sin_port != addr.sin_port))
		{
			char s1[20], s2[20];

			strcpy(s1, inet_ntoa(vpc->rrtpaddr.sin_addr));
			strcpy(s2, inet_ntoa(addr.sin_addr));
			Lprintf("RTP socket for call %d is transmitting to %s:%d, but data comes from %s:%d",
				vpc->vpcall, s1, ntohs(vpc->rrtpaddr.sin_port), s2, ntohs(addr.sin_port));
			vpc->raddrwrong = true;
		}
		if(settings.remotenat)
			vpc->rrtpaddr = addr;
		if(addr.sin_addr.s_addr == vpc->rrtpaddr.sin_addr.s_addr &&
			addr.sin_port == vpc->rrtpaddr.sin_port && len > 12)
		{
			vpc->awaketm = GetTickCount();
			if((buf[1] & 0x7f) == vpc->keypadmap)
				RTPKeypad(vpc, (BYTE *)buf, len);	// RTPKeypad and RTPPacket *have* to unlock
			else RTPPacket(vpc, (BYTE *)buf, len);
		} else vpc->Unlock();
	}
	TRETURN
}

void SIPSTACK::RTPKeypad(SIPCALLDATA *vpc, const BYTE *buf, int len)
{
	int key;
	unsigned seq;
	VPCALL vpcall = vpc->vpcall;

	if(len < 14)
	{
		vpc->Unlock();
		return;
	}
	seq = (buf[4] << 24) | (buf[5] << 16) | (buf[6] << 8) | buf[7];
	if(generatelog >= 2)
		Lprintf("RTPkeypad %d, M=%d, E=%d, seq=%u, lastseq=%u", buf[12], buf[1]>>7, buf[13]>>7, seq, vpc->lastkeypadseqnumber);
	if(vpc->lastkeypadseqnumber == seq)
	{
		vpc->Unlock();
		return;
	}
	vpc->lastkeypadseqnumber = seq;
	if(buf[12] <= 16)
	{
		vpc->Unlock();
		if(buf[12] <= 9)
			key = '0' + buf[12];
		else if(buf[12] == 10)
			key = '*';
		else if(buf[12] == 11)
			key = '#';
		else if(buf[12] < 16)
			key = 'A' + buf[12] - 12;
		else key = 'F';
		NotifyOnCall(VPMSG_KEYPAD, vpcall, key);
		return;
	}
	vpc->Unlock();
}

int SIPSTACK::SendKeypad(VPCALL vpcall, int key)
{
	SendKeypad(vpcall, key, 0, 0);
	SendKeypad(vpcall, key, 0, 0);
	SendKeypad(vpcall, key, 1280, 1);
	return SendKeypad(vpcall, key, 1280, 1);
}

int SIPSTACK::SendKeypad(VPCALL vpcall, int key, int len, bool end)
{
	SIPCALLDATA *vpc;
	BYTE pkt[16];
	struct sockaddr_in addr;
	SOCKET sk;
	int code;

	if(key >= '0' && key <= '9')
		code = key - '0';
	else if(key == '*')
		code = 10;
	else if(key == '#')
		code = 11;
	else if(key >= 'A' && key <= 'D')
		code = key - 'A' + 12;
	else if(key == 'F')
		code = 16;
	else return -1;
	vpc = LockCall(vpcall);
	if(!vpc)
		return -1;
	pkt[0] = (BYTE)0x80;
	pkt[1] = vpc->keypadmap | (len ? 0 : 0x80);	// New event
	pkt[2] = (BYTE)(vpc->seqnumber >> 8);
	pkt[3] = (BYTE)vpc->seqnumber;
	vpc->seqnumber++;
	pkt[4] = (BYTE)(vpc->timestamp >> 24);
	pkt[5] = (BYTE)(vpc->timestamp >> 16);
	pkt[6] = (BYTE)(vpc->timestamp >> 8);
	pkt[7] = (BYTE)vpc->timestamp;
	CopyDWord(pkt + 8, vpc->sessionid);
	pkt[12] = code;
	pkt[13] = end ? 0x80 : 0;
	pkt[14] = (BYTE)(len >> 8);
	pkt[15] = (BYTE)len;
	sk = vpc->rtpsk;
	addr = vpc->rrtpaddr;
	vpc->Unlock();
	if(nbsendto(sk, (char *)pkt, 16, 0, (struct sockaddr *)&addr, sizeof(addr)) > 0)
		RateCount(&tx.xc, 16+UDPIPHLEN);
	return 0;
}

void SIPSTACK::RTPPacket(SIPCALLDATA *vpc, const BYTE *buf, int len)
{
	int headerlen, codec;
	unsigned timestamp;
	VPAUDIODATA *audio;
	VPCALL vpcall;

	if(len <= 12 || (*buf & 0xf0) != 0x80)
	{
		vpc->Unlock();
		return;
	}
	if(!vpc->audio)
	{
		vpc->Unlock();
		return;
	}
	headerlen = 12 + (buf[0] & 0xf) * 4;
	if(headerlen >= len)
	{
		vpc->Unlock();
		return;
	}
	switch(buf[1] & 0x7f)
	{
	case 0:
		codec = CODEC_G711U;
		break;
	case 3:
		codec = CODEC_GSMFR;
		break;
	case 8:
		codec = CODEC_G711A;
		break;
	case 18:
		codec = CODEC_G729;
		break;
	default:
		if(vpc->iLBC20map && (buf[1] & 0x7f) == vpc->iLBC20map)
			codec = CODEC_ILBC20;
		else if(vpc->iLBC30map && (buf[1] & 0x7f) == vpc->iLBC30map)
			codec = CODEC_ILBC30;
		else {
			vpc->Unlock();
			return;
		}
	}
	timestamp = buf[4] << 24 | buf[5] << 16 | buf[6] << 8 | buf[7];
	audio = vpc->audio;
	vpc->audiolocked = true;
	vpcall = vpc->vpcall;
	vpc->Unlock();
	audio->AudioFrame(codec, timestamp / 8, buf + headerlen, len - headerlen);
	vpc = LockCall(vpcall);
	if(vpc)
	{
		if(vpc->destroyaudio && vpc->audio)
		{
			delete vpc->audio;
			vpc->audio = 0;
			vpc->destroyaudio = false;
		}
		vpc->audiolocked = false;
		vpc->Unlock();
	}
}

int SIPSTACK::SendAudio(VPCALL vpcall, int codec, unsigned timestamp, const void *buf, int len, bool wait)
{
	SIPCALLDATA *vpc;
	char pkt[1500];
	int tsinc;
	unsigned lastsyncticks;
	struct sockaddr_in addr;
	SOCKET sk;

	vpc = LockCall(vpcall);
	if(!vpc)
		return -1;
	if(!vpc->cantx || vpc->held || vpc->hold)
	{
		vpc->Unlock();
		return -1;
	}
	pkt[0] = (BYTE)0x80;
	switch(codec)
	{
	case CODEC_GSMFR:
		pkt[1] = 3;
		tsinc = len / 33 * 160;
		break;
	case CODEC_G711A:
		pkt[1] = 8;
		tsinc = len;
		break;
	case CODEC_G711U:
		pkt[1] = 0;
		tsinc = len;
		break;
	case CODEC_G729:
		pkt[1] = 18;
		tsinc = len * 8;
		break;
	case CODEC_ILBC20:
		if(vpc->iLBC20map)
		{
			pkt[1] = vpc->iLBC20map;
			tsinc = len / 38 * 160;
		} else {
			vpc->Unlock();
			return -1;
		}
		break;
	case CODEC_ILBC30:
		if(vpc->iLBC30map)
		{
			pkt[1] = vpc->iLBC30map;
			tsinc = len / 50 * 240;
		} else {
			vpc->Unlock();
			return -1;
		}
		break;
	default:
		vpc->Unlock();
		return -1;
	}
	pkt[2] = (BYTE)(vpc->audioseqno >> 8);
	pkt[3] = (BYTE)vpc->audioseqno;
	vpc->audioseqno++;
	if(!timestamp)
		timestamp = vpc->timestamp;
	else timestamp *= 8;
	pkt[4] = (BYTE)(timestamp >> 24);
	pkt[5] = (BYTE)(timestamp >> 16);
	pkt[6] = (BYTE)(timestamp >> 8);
	pkt[7] = (BYTE)timestamp;
	vpc->timestamp += tsinc;
	CopyDWord(pkt + 8, vpc->sessionid);
	memcpy(pkt + 12, buf, len);
	sk = vpc->rtpsk;
	addr = vpc->rrtpaddr;
	if(!wait || !vpc->lastsyncticks || (int)(GetTickCount() - (vpc->lastsyncticks+timestamp/8)) > 100)
		vpc->lastsyncticks = GetTickCount() - timestamp/8 - 40;
	lastsyncticks = vpc->lastsyncticks;
	vpc->Unlock();
	if(wait)
		while((int)(GetTickCount() - (lastsyncticks+timestamp/8)) < 0)
			Sleep(10);
	if(nbsendto(sk, pkt, len + 12, 0, (struct sockaddr *)&addr, sizeof(addr)) > 0)
		RateCount(&tx.xc, len+12+UDPIPHLEN);
	return 0;
}

int SIPSTACK::GetTransceiverBandwidths(int *rx, int *tx)
{
	*rx = (int)(rxxc.rate * XC_MULT);
	*tx = (int)(this->tx.xc.rate * XC_MULT);
	return 0;
}

int SIPSTACK::Hold(VPCALL vpcall)
{
	SIPCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(vpc)
	{
		if(vpc->status != VPCS_CONNECTED)
		{
			vpc->Unlock();
			return VPERR_INVALIDSTATUS;
		}
	} else return VPERR_INVALIDCALL;
	BuildVia(vpc);
	if(*vpc->contact)
		strcpy(vpc->address, vpc->contact);
	*vpc->nonce = 0;
	vpc->sessionversion++;
	vpc->status = VPCS_CONNECTING;
	vpc->hold = true;
	vpc->connecttype = CT_HOLDRESUME;
	vpc->Unlock();
	TSTART
	beginclassthread((ClassThreadStart)&SIPSTACK::ConnectThread, this, (void *)vpcall);
	return 0;
}

int SIPSTACK::Resume(VPCALL vpcall)
{
	SIPCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(vpc)
	{
		if(vpc->status != VPCS_HELD)
		{
			vpc->Unlock();
			return VPERR_INVALIDSTATUS;
		}
	} else return VPERR_INVALIDCALL;
	BuildVia(vpc);
	if(*vpc->contact)
		strcpy(vpc->address, vpc->contact);
	*vpc->nonce = 0;
	vpc->sessionversion++;
	vpc->status = VPCS_CONNECTING;
	vpc->hold = false;
	vpc->connecttype = CT_HOLDRESUME;
	vpc->Unlock();
	TSTART
	beginclassthread((ClassThreadStart)&SIPSTACK::ConnectThread, this, (void *)vpcall);
	return 0;
}

int SIPSTACK::ConferenceCalls(VPCALL *vpcalls, int ncalls, bool detach)
{
	if(!detach || ncalls != 2)
		return VPERR_UNSUPPORTEDCALL;
	return Transfer(vpcalls[0], vpcalls[1]);
}

int SIPSTACK::Transfer(VPCALL vpcall1, VPCALL vpcall2)
{
	SIPCALLDATA *vpc;
	VPCALL maincall, secondcall;
	char address[MAXSIPADDRLEN+1];
	int status;

	vpc = LockCall(vpcall1);
	if(vpc)
	{
		if(vpc->status == VPCS_HELD || vpc->status == VPCS_CONNECTED)
		{
			maincall = vpcall1;
			secondcall = vpcall2;
		} else {
			maincall = vpcall2;
			secondcall = vpcall1;
		}
		vpc->Unlock();
	} else return VPERR_INVALIDCALL;

	vpc = LockCall(secondcall);
	if(vpc)
	{
		status = vpc->status;
		strcpy(address, vpc->address);
		if(vpc->status == VPCS_HELD || vpc->status == VPCS_CONNECTED)
			sprintf(address + strlen(address), "?Replaces=%s;to-tag=%s;from-tag=%s", vpc->callid, vpc->connectedtag, vpc->ourtag);
		vpc->Unlock();
		if(vpc->status == VPCS_IDLE)
			NotifyOnCall(VPMSG_CALLENDED, secondcall, 0);
	} else return VPERR_INVALIDCALL;
	if(!*address)
		return VPERR_INVALIDADDR;
	vpc = LockCall(maincall);
	if(vpc)
	{
		if(vpc->status != VPCS_HELD && vpc->status != VPCS_CONNECTED)
		{
			vpc->Unlock();
			return VPERR_INVALIDSTATUS;
		}
	} else return VPERR_INVALIDCALL;
	BuildVia(vpc);
	if(*vpc->contact)
		strcpy(vpc->address, vpc->contact);
	*vpc->nonce = 0;
	vpc->status = VPCS_CONNECTING;
	vpc->connecttype = CT_REFER;
	strcpy(vpc->referto, address);
	vpc->Unlock();
	TSTART
	beginclassthread((ClassThreadStart)&SIPSTACK::ConnectThread, this, (void *)maincall);
	return 0;
}

void SIPSTACK::SetAudioDataFactory(VPAUDIODATAFACTORY *af)
{
	vpaudiodatafactory = af;
}

int SIPSTACK::GetCallTimes(VPCALL vpcall, time_t *start_time, unsigned int *length_ms)
{
	SIPCALLDATA *vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	*start_time = vpc->start_time;
	*length_ms = GetTickCount() - vpc->start_ticks;
	vpc->Unlock();
	return 0;
}

IVPSTACK *CreateSIPSTACK(VPAUDIODATAFACTORY *vpaf, const char *stackname)
{
	return new SIPSTACK(vpaf, stackname);
}
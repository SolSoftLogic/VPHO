#include "portability.h"
#include "vpstack.h"
#include "vprotocol.h"
#include "ielements.h"

#ifdef IE_ELEMENTS_DEBUG
#define dprintf printf
#else
#ifdef _WIN32
#define dprintf
#else
#define dprintf(...)
#endif
#endif

OUTMSG::OUTMSG(unsigned mtu, BYTE type)
{
	this->mtu = mtu;
	msg[0] = type;
	ptr = msg + 1;
	avail = mtu - 1;
}

void OUTMSG::Reset(BYTE type)
{
	msg[0] = type;
	ptr = msg + 1;
	avail = mtu - 1;
}

int OUTMSG::AddIE_String(BYTE type, const char *s)
{
	unsigned len = strlen(s);
	
	if(len <= 247 && len + 2 <= avail)
	{
		ptr[0] = (BYTE)len + 1;
		ptr[1] = type;
		memcpy(ptr + 2, s, len);
		ptr += len + 2;
		avail -= len + 2;
	} else if(len + 3 <= avail)
	{
		ptr[0] = (BYTE)(0xf8 | ((len + 1) >> 8));
		ptr[1] = (BYTE)((len + 1) & 0xff);
		ptr[2] = type;
		memcpy(ptr + 3, s, len);
		ptr += len + 3;
		avail -= len + 3;
	} else return -1;
	return 0;
}

int OUTMSG::AddIE_Word(BYTE type, WORD w)
{
	if(avail < 4)
		return -1;
	ptr[0] = 3;
	ptr[1] = type;
	ptr[2] = (BYTE)(w >> 8);
	ptr[3] = (BYTE)w;
	ptr += 4;
	avail -= 4;
	return 0;
}

int OUTMSG::AddIE_DWord(BYTE type, DWORD dw)
{
	if(avail < 6)
		return -1;
	ptr[0] = 5;
	ptr[1] = type;
	ptr[2] = (BYTE)(dw >> 24);
	ptr[3] = (BYTE)(dw >> 16);
	ptr[4] = (BYTE)(dw >> 8);
	ptr[5] = (BYTE)dw;
	ptr += 6;
	avail -= 6;
	return 0;
}

int OUTMSG::AddIE_Binary(BYTE type, const BYTE *data, unsigned datalen)
{
	if(avail < 2 + datalen)
		return -1;
	ptr[0] = 1 + datalen;
	ptr[1] = type;
	memcpy(ptr + 2, data, datalen);
	ptr += 2 + datalen;
	avail -= datalen;
	return 0;
}

int OUTMSG::AddIE_RecordEnd()
{
	if(avail < 1)
		return -1;
	ptr[0] = 0;
	ptr += 1;
	avail -= 1;
	return 0;
}

int OUTMSG::AddIE_Byte(BYTE type, BYTE b)
{
	if(avail < 3)
		return -1;
	ptr[0] = 2;
	ptr[1] = type;
	ptr[2] = b;
	ptr += 3;
	avail -= 3;
	return 0;
}

int OUTMSG::AddIE_2Bytes(BYTE type, BYTE b1, BYTE b2)
{
	if(avail < 4)
		return -1;
	ptr[0] = 3;
	ptr[1] = type;
	ptr[2] = b1;
	ptr[3] = b2;
	ptr += 4;
	avail -= 4;
	return 0;
}

int OUTMSG::AddIE_3Bytes(BYTE type, BYTE b1, BYTE b2, BYTE b3)
{
	if(avail < 5)
		return -1;
	ptr[0] = 4;
	ptr[1] = type;
	ptr[2] = b1;
	ptr[3] = b2;
	ptr[4] = b3;
	ptr += 5;
	avail -= 5;
	return 0;
}

int OUTMSG::AddIE_BC(BYTE bc, BYTE codec, BYTE flags, DWORD codecs)
{
	unsigned ielen = 4;

	if(codecs & 0xffff)
		ielen++;
	if(codecs & 0xff00)
		ielen++;
	if(avail < ielen)
		return -1;
	ptr[0] = ielen;
	ptr[1] = IE_BEARER;
	ptr[2] = bc;
	ptr[3] = codec;
	ptr[4] = flags;
	if(codecs & 0xff)
		ptr[5] = (BYTE)codecs;
	if(codecs & 0xff00)
		ptr[6] = (BYTE)(codecs >> 8);
	ptr += ielen + 1;
	avail -= ielen + 1;
	return 0;
}

int OUTMSG::AddIE_Video(DWORD fourcc, WORD xres, WORD yres)
{
	if(avail < 10)
		return -1;
	ptr[0] = 9;
	ptr[1] = IE_VIDEOFMT;
	ptr[2] = (BYTE)(fourcc >> 24);
	ptr[3] = (BYTE)(fourcc >> 16);
	ptr[4] = (BYTE)(fourcc >> 8);
	ptr[5] = (BYTE)fourcc;
	ptr[6] = (BYTE)(xres >> 8);
	ptr[7] = (BYTE)xres;
	ptr[8] = (BYTE)(yres >> 8);
	ptr[9] = (BYTE)yres;
	ptr += 10;
	avail -= 10;
	return 0;
}

int OUTMSG::AddIE_InAddr(BYTE type, DWORD addr, WORD port)
{
	if(avail < 8)
		return -1;
	ptr[0] = 7;
	ptr[1] = type;
	CopyDWord(ptr + 2, addr);
	CopyWord(ptr + 6, port);
	ptr += 8;
	avail -= 8;
	return 0;
}

int OUTMSG::AddIE_MultiInAddr(BYTE type, struct sockaddr_in *addr, int count)
{
	int i;

	if(avail < 2 + 6 * (unsigned)count)
		return -1;
	ptr[0] = 1 + 6 * count;
	ptr[1] = type;
	ptr += 2;
	avail -= 2;
	for(i = 0; i < count; i++)
	{
		CopyDWord(ptr, addr[i].sin_addr.s_addr);
		CopyWord(ptr + 4, addr[i].sin_port);
		ptr += 6;
		avail -= 6;
	}
	return 0;
}

int OUTMSG::AddIE_MissedCall(DWORD tm, BYTE count, BYTE bc)
{
	if(avail < 8)
		return -1;
	ptr[0] = 7;
	ptr[1] = IE_MISSEDCALL;
	ptr[2] = (BYTE)(tm >> 24);
	ptr[3] = (BYTE)(tm >> 16);
	ptr[4] = (BYTE)(tm >> 8);
	ptr[5] = (BYTE)tm;
	ptr[6] = count;
	ptr[7] = bc;
	ptr += 8;
	avail -= 8;
	return 0;
}

int OUTMSG::AddIE_Text(BYTE encoding, char *data, int datalen)
{
	if(datalen + 2 <= 247 && datalen + 3 <= (int)avail)
	{
		ptr[0] = (BYTE)(datalen + 2);
		ptr[1] = IE_TEXT;
		ptr[2] = encoding;
		memcpy(ptr + 3, data, datalen);
		ptr += datalen + 3;
		avail -= datalen + 3;
	} else if(datalen + 4 <= (int)avail)
	{
		ptr[0] = (BYTE)(0xf8 | ((datalen + 2) >> 8));
		ptr[1] = (BYTE)((datalen + 2) & 0xff);
		ptr[2] = IE_TEXT;
		ptr[3] = encoding;
		memcpy(ptr + 4, data, datalen);
		ptr += datalen + 4;
		avail -= datalen + 4;
	} else return -1;
	return 0;
}

int OUTMSG::AddIE_EncodedNumbers(const DWORD *numbers, int num)
{
	int i;

	if(avail < 7)
		return -1;
	if(num * 4 + 1 <= 247)
	{
		if(4*num + 2 > (int)avail)
			num = (avail - 2) / 4;
		ptr[0] = (BYTE)(num * 4 + 1);
		ptr[1] = IE_ENCODEDNUMBERS;
		for(i = 0; i < num; i++)
			CopyDWord(ptr + 2 + 4 * i, htonl(numbers[i]));
		ptr += 4*num + 2;
		avail -= 4*num + 2;
	} else {
		if(4*num + 3 > (int)avail)
			num = (avail - 3) / 4;
		ptr[0] = (BYTE)(0xf8 | ((4*num + 1) >> 8));
		ptr[1] = (BYTE)((4*num + 1) & 0xff);
		ptr[2] = IE_ENCODEDNUMBERS;
		for(i = 0; i < num; i++)
			CopyDWord(ptr + 3 + 4 * i, htonl(numbers[i]));
		ptr += 4 * num + 3;
		avail -= 4 * num + 3;
	}
	return 0;
}

int GetIE_Elements(BYTE *msg, unsigned len, IE_ELEMENTS *ie)
{
	BYTE fb;	// The first byte of IE (length) can be set to null (for strings), fb is this first byte
	unsigned ielen, i;
	BYTE *ieptr, type;

	ZeroMemory(ie, sizeof(*ie));
	// When you add here, don't forget to add to DupIE
	ie->srcnum = ie->srcname = ie->destname = ie->srcsubaddr = ie->destnum = ie->destsubaddr = ie->message = ie->filename =
		ie->deflnum = ie->serveraccesscode = ie->fileslist = ie->sessionid = ie->sessionsecret = ie->serverslist =
		ie->firstname = ie->lastname = ie->email = ie->phonenumber = ie->country = &ie->nullchar;
	if(!len)
		return 0;
	ie->packetbuf = (char *)msg;
	ie->packetlen = len;
	ie->bc = 0xff;
	msg++;
	len--;
	if(len == 1)
		return -1;
	fb = *msg;
	while(len)
	{
		if(len == 1)
			return -1;
		if((fb & 0xf8) == 0xf8)
		{
			if(len == 2)
				return -1;
			ielen = ((fb & 7) << 8) | msg[1];
			if(ielen < 1 || ielen + 2 > len)
				return -1;
			type = msg[2];
			ieptr = msg + 3;
			msg += 2 + ielen;
			len -= 2 + ielen;
			ielen--;
		} else {
			ielen = fb;
			if(ielen < 1 || ielen + 1 > len)
				return -1;
			type = msg[1];
			ieptr = msg + 2;
			msg += 1 + ielen;
			len -= 1 + ielen;
			ielen--;
		}
		if(len > 0)
			fb = *msg;
		switch(type)
		{
		case IE_CONNECTIONID:
			if(ielen >= 2)
			{
				ie->connectionid = ieptr[0] << 8 | ieptr[1];
				dprintf("Connection id: %d\n", ie->connectionid);
			}
			break;
		case IE_BEARER:
			if(ielen >= 1)
			{
				ie->bc = ieptr[0];
				dprintf("Bearer: %d\n", ie->bc);
			}
			if(ie->bc == BC_VOICE || ie->bc == BC_AUDIOVIDEO)
			{
				if(ielen >= 2)
				{
					ie->codec = ieptr[1];
					dprintf("Codec: %d\n", ie->codec);
				}
				if(ielen >= 3)
				{
					ie->bc_flags = ieptr[2];
					dprintf("Flags: %d\n", ie->bc_flags);
				}
				if(ielen >= 4)
				{
					ie->bc_codecs = ieptr[3];
					if(ielen >= 5)
						ie->bc_codecs |= ieptr[4] << 8;
					dprintf("Supported codecs: %x\n", ie->bc_codecs);
				}
			} else if(ie->bc == BC_CHAT && ielen >= 2)
			{
				ie->rtfencoding = ieptr[1];
				dprintf("RTFEncoding: %d\n", ie->rtfencoding);
			}
			break;
		case IE_VIDEOFMT:
			if(ielen >= 8)
			{
				ie->fourcc = ieptr[0] << 24 | ieptr[1] << 16 | ieptr[2] << 8 | ieptr[3];
				ie->xres = ieptr[4] << 8 | ieptr[5];
				ie->yres = ieptr[6] << 8 | ieptr[7];
				dprintf("Video format: %c%c%c%c, %dx%d\n", ieptr[3], ieptr[2], ieptr[1], ieptr[0], ie->xres, ie->yres);
			}
			break;
		case IE_ANSWER:
			if(ielen >= 1)
			{
				ie->answer = ieptr[0];
				dprintf("Answer: %d\n", ie->answer);
			}
			if(ielen >= 2)
			{
				ie->reason = ieptr[1];
				dprintf("Reason: %d\n", ie->reason);
			}
			break;
		case IE_REASON:
			if(ielen >= 1)
			{
				ie->reason = ieptr[0];
				dprintf("Reason: %d\n", ie->reason);
			}
			break;
		case IE_OPERATION:
			if(ielen >= 1)
			{
				ie->operation = ieptr[0];
				dprintf("Operation: %d\n", ie->operation);
			}
			break;
		case IE_SOURCENUMBER:
			ie->srcnum = (char *)ieptr;
			ieptr[ielen] = 0;
			if(ielen > MAXNUMBERLEN)
				ieptr[MAXNUMBERLEN] = 0;
			dprintf("Source number: %s\n", ie->srcnum);
			break;
		case IE_SOURCENAME:
			ie->srcname = (char *)ieptr;
			ieptr[ielen] = 0;
			if(ielen > MAXNAMELEN)
				ieptr[MAXNAMELEN] = 0;
			dprintf("Source name: %s\n", ie->srcname);
			break;
		case IE_SOURCESUBADDRESS:
			ie->srcsubaddr = (char *)ieptr;
			ieptr[ielen] = 0;
			if(ielen > MAXSUBADDRLEN)
				ieptr[MAXSUBADDRLEN] = 0;
			dprintf("Source subaddress: %s\n", ie->srcsubaddr);
			break;
		case IE_DESTNUMBER:
			ie->destnum = (char *)ieptr;
			ieptr[ielen] = 0;
			if(ielen > MAXNUMBERLEN)
				ieptr[MAXNUMBERLEN] = 0;
			dprintf("Destination number: %s\n", ie->destnum);
			break;
		case IE_DESTNAME:
			ie->destname = (char *)ieptr;
			ieptr[ielen] = 0;
			if(ielen > MAXNAMELEN)
				ieptr[MAXNAMELEN] = 0;
			dprintf("Destination name: %s\n", ie->destname);
			break;
		case IE_DESTSUBADDRESS:
			ie->destsubaddr = (char *)ieptr;
			ieptr[ielen] = 0;
			if(ielen > MAXSUBADDRLEN)
				ieptr[MAXSUBADDRLEN] = 0;
			dprintf("Destination subaddress: %s\n", ie->destsubaddr);
			break;
		case IE_MESSAGE:
			ie->message = (char *)ieptr;
			ieptr[ielen] = 0;
			if(ielen > MAXMESSAGELEN)
				ieptr[MAXMESSAGELEN] = 0;
			dprintf("Message: %s\n", ie->message);
			break;
		case IE_FILENAME:
			ie->filename = (char *)ieptr;
			ieptr[ielen] = 0;
			if(ielen > MAXFILENAMELEN)
				ieptr[MAXFILENAMELEN] = 0;
			dprintf("Filename: %s\n", ie->filename);
			break;
		case IE_DEFLECTTONUMBER:
			ie->deflnum = (char *)ieptr;
			ieptr[ielen] = 0;
			if(ielen > MAXNUMBERLEN)
				ieptr[MAXNUMBERLEN] = 0;
			dprintf("Deflect to number: %s\n", ie->deflnum);
			break;
		case IE_SERVERACCESSCODE:
			ie->serveraccesscode = (char *)ieptr;
			ieptr[ielen] = 0;
			if(ielen > MAXSERVERACCESSCODELEN)
				ieptr[MAXSERVERACCESSCODELEN] = 0;
			dprintf("Server access code: %s\n", ie->serveraccesscode);
			break;
		case IE_FILESLIST:
			ie->fileslist = (char *)ieptr;
			ieptr[ielen] = 0;
			if(ielen > MAXMESSAGELEN)
				ieptr[MAXMESSAGELEN] = 0;
			dprintf("Files list\n");
			break;
		case IE_TEXT:
			if(ielen >= 1)
				ie->rtfencoding = ieptr[0];
			ie->rtfdata = (char *)ieptr + 1;
			ie->rtfdatalen = ielen - 1;
			if(ie->rtfdatalen > MAXTEXTLEN)
				ie->rtfdatalen = MAXTEXTLEN;
			dprintf("RTF text, encoding = %d\n", ie->rtfencoding);
			break;
		case IE_CALLREQUEST:
			if(ielen >= 1)
			{
				ie->callrequest = ieptr[0];
				dprintf("Call request: %d\n", ie->callrequest);
			}
			break;
		case IE_VERSION:
			if(ielen >= 4)
			{
				ie->version = ieptr[0] << 24 | ieptr[1] << 16 | ieptr[2] << 8 | ieptr[3];
				dprintf("Version: %u.%u, build %u\n", ieptr[0], ieptr[1], ieptr[2] << 8 | ieptr[3]);
			}
			break;
		case IE_INADDR_HOST:
			if(ielen >= 6)
			{
				memset(&ie->hostaddr, 0, sizeof(ie->hostaddr));
				ie->hostaddr.sin_family = AF_INET;
				CopyDWordP(&ie->hostaddr.sin_addr, ieptr);
				CopyWordP(&ie->hostaddr.sin_port, ieptr + 4);
				dprintf("Host address: %s:%d\n", inet_ntoa(ie->hostaddr.sin_addr), ntohs(ie->hostaddr.sin_port));
			}
			break;
		case IE_INADDR_PRIVATE:
			if(ielen >= 6)
			{
				memset(&ie->privateaddr, 0, sizeof(ie->privateaddr));
				ie->privateaddr.sin_family = AF_INET;
				CopyDWordP(&ie->privateaddr.sin_addr, ieptr);
				CopyWordP(&ie->privateaddr.sin_port, ieptr + 4);
				dprintf("Private address: %s:%d\n", inet_ntoa(ie->privateaddr.sin_addr), ntohs(ie->privateaddr.sin_port));
			}
			break;
		case IE_INADDR_PUBLIC:
			if(ielen >= 6)
			{
				memset(&ie->publicaddr, 0, sizeof(ie->publicaddr));
				ie->publicaddr.sin_family = AF_INET;
				CopyDWordP(&ie->publicaddr.sin_addr, ieptr);
				CopyWordP(&ie->publicaddr.sin_port, ieptr + 4);
				dprintf("Public address: %s:%d\n", inet_ntoa(ie->publicaddr.sin_addr), ntohs(ie->publicaddr.sin_port));
			}
			break;
		case IE_INADDR_REQUESTER:
			if(ielen >= 6)
			{
				memset(&ie->reqaddr, 0, sizeof(ie->reqaddr));
				ie->reqaddr.sin_family = AF_INET;
				CopyDWordP(&ie->reqaddr.sin_addr, ieptr);
				CopyWordP(&ie->reqaddr.sin_port, ieptr + 4);
				dprintf("Requester address: %s:%d\n", inet_ntoa(ie->reqaddr.sin_addr), ntohs(ie->reqaddr.sin_port));
			}
			break;
		case IE_INADDR_REQUESTER2:
			if(ielen >= 6)
			{
				memset(&ie->reqaddr2, 0, sizeof(ie->reqaddr2));
				ie->reqaddr2.sin_family = AF_INET;
				CopyDWordP(&ie->reqaddr2.sin_addr, ieptr);
				CopyWordP(&ie->reqaddr2.sin_port, ieptr + 4);
				dprintf("Requester address 2: %s:%d\n", inet_ntoa(ie->reqaddr2.sin_addr), ntohs(ie->reqaddr2.sin_port));
			}
			break;
		case IE_SEQNUMBER:
			if(ielen >= 4)
			{
				ie->seqnumber = ieptr[0] << 24 | ieptr[1] << 16 | ieptr[2] << 8 | ieptr[3];
				ie->seqnumberpresent = true;
				dprintf("Sequential number: %u\n", ie->seqnumber);
			}
			break;
		case IE_MTU:
			if(ielen >= 2)
			{
				ie->mtu = ieptr[0] << 8 | ieptr[1];
				dprintf("MTU: %d\n", ie->mtu);
			}
			break;
		case IE_FILELENGTH:
			if(ielen >= 4)
			{
				ie->filelength = ieptr[0] << 24 | ieptr[1] << 16 | ieptr[2] << 8 | ieptr[3];
				dprintf("File length: %u\n", ie->filelength);
			}
			break;
		case IE_FILECRC:
			if(ielen >= 4)
			{
				ie->filecrc = ieptr[0] << 24 | ieptr[1] << 16 | ieptr[2] << 8 | ieptr[3];
				dprintf("File CRC: %x\n", ie->filecrc);
			}
			break;
		case IE_STARTPOS:
			if(ielen >= 4)
			{
				ie->startpos = ieptr[0] << 24 | ieptr[1] << 16 | ieptr[2] << 8 | ieptr[3];
				dprintf("Start position: %u\n", ie->startpos);
			}
			break;
		case IE_TOKEN:
			if(ielen >= 4)
			{
				ie->token = ieptr[0] << 24 | ieptr[1] << 16 | ieptr[2] << 8 | ieptr[3];
				ie->tokenpresent = true;
				dprintf("Token: %x\n", ie->token);
			}
			break;
		case IE_SMSTYPE:
			if(ielen >= 4)
			{
				ie->smstype = ieptr[0] << 24 | ieptr[1] << 16 | ieptr[2] << 8 | ieptr[3];
				dprintf("SMS Type: %x\n", ie->smstype);
			}
			break;
		case IE_SMSID:
			if(ielen >= 4)
			{
				ie->smsid = ieptr[0] << 24 | ieptr[1] << 16 | ieptr[2] << 8 | ieptr[3];
				dprintf("SMS ID: %x\n", ie->smsid);
			}
			break;
		case IE_MULTIINADDR_HOST:
			memset(&ie->hostaddr_array, 0, sizeof(ie->hostaddr_array));
			for(i = 0; ielen >= 6*(i + 1) && i < MAXVPARTIES; i++)
			{
				ie->hostaddr_array[i].sin_family = AF_INET;
				CopyDWordP(&ie->hostaddr_array[i].sin_addr, ieptr + 6 * i);
				CopyWordP(&ie->hostaddr_array[i].sin_port, ieptr + 6 * i + 4);
				dprintf("Multiple address: %s:%d\n", inet_ntoa(ie->hostaddr_array[i].sin_addr), ntohs(ie->hostaddr_array[i].sin_port));
			}
			break;
		case IE_CONFSETUPSTEP:
			if(ielen >= 1)
			{
				ie->confsetupstep = *ieptr;
				dprintf("Conference setup step: %d\n", ie->confsetupstep);
			}
			break;
		case IE_NFILES:
			if(ielen >= 4)
			{
				ie->nfiles = ieptr[0] << 24 | ieptr[1] << 16 | ieptr[2] << 8 | ieptr[3];
				dprintf("Number of files: %u\n", ie->nfiles);
			}
			break;
		case IE_NBYTES:
			if(ielen >= 4)
			{
				ie->nbytes = ieptr[0] << 24 | ieptr[1] << 16 | ieptr[2] << 8 | ieptr[3];
				dprintf("Number of bytes: %u\n", ie->nbytes);
			}
			break;
		case IE_SERVERID:
			if(ielen >= 4)
			{
				ie->serverid = ieptr[0] << 24 | ieptr[1] << 16 | ieptr[2] << 8 | ieptr[3];
				dprintf("Server id: %u\n", ie->serverid);
			}
			break;
		case IE_TIMESTAMP:
			if(ielen >= 4)
			{
				ie->timestamp = ieptr[0] << 24 | ieptr[1] << 16 | ieptr[2] << 8 | ieptr[3];
				dprintf("Time stamp: %u\n", ie->timestamp);
			}
			break;
		case IE_CALLTRANSFER:
			if(ielen >= 1)
			{
				ie->calltransfer = ieptr[0];
				dprintf("Call transfer: %d\n", ie->calltransfer);
			}
			break;
		case IE_CALLSTATUS:
			if(ielen >= 1)
			{
				ie->callstatus = ieptr[0];
				dprintf("Call status: %d\n", ie->callstatus);
			}
			break;
		case IE_MISSEDCALL:
			if(ielen >= 6)
			{
				ie->missedcalltime = (ieptr[0] << 24) | (ieptr[1] << 16) | (ieptr[2] << 8) | ieptr[3];
				ie->missedcallscount = ieptr[4];
				ie->missedcallbc = ieptr[5];
				dprintf("Missed call: count=%d, timestamp=%u, bearer=%d\n", ie->missedcallscount, ie->missedcalltime, ie->missedcallbc);
			}
			break;
		case IE_ENCODEDNUMBERS:
			if(ielen >= 4)
			{
				ie->nencodednumbers = ielen / 4;
				ie->encodednumbersptr = (DWORD *)ieptr;
				for(i = 0; i < ie->nencodednumbers; i++)
				{
					ie->encodednumbersptr[i] = ntohl(((DWORD *)ieptr)[i]);
					dprintf("Encoded number: %u (%d)\n", ie->encodednumbersptr[i] & 0xffffff, ie->encodednumbersptr[i] >> 24);
				}
			}
			break;
		case IE_CODEPAGE:
			if(ielen >= 2)
			{
				ie->codepage = (ieptr[0] << 8) | ieptr[1];
				dprintf("Codepage: %u\n", ie->codepage);
			}
			break;
		case IE_AUTHDATA:
			if(ielen >= 16)
			{
				ie->authdata = ieptr;
				dprintf("Authorization data\n");
			}
			break;
		case IE_OFFLINE:
			if(ielen >= 1)
			{
				ie->offline = ieptr[0];
				dprintf("Offline: %u\n", ie->offline);
			}
			break;
		case IE_USERSTATUS:
			if(ielen >= 4)
			{
				ie->userstatus = ieptr[0] << 24 | ieptr[1] << 16 | ieptr[2] << 8 | ieptr[3];
				dprintf("User status: %u\n", ie->userstatus);
			}
			break;
		case IE_SESSIONID:
			ie->sessionid = (char *)ieptr;
			ieptr[ielen] = 0;
			if(ielen > MAXSESSIONIDLEN)
				ieptr[MAXSESSIONIDLEN] = 0;
			dprintf("Session id: %s\n", ie->sessionid);
			break;
		case IE_SESSIONSECRET:
			ie->sessionsecret = (char *)ieptr;
			ieptr[ielen] = 0;
			if(ielen > MAXSESSIONSECRETLEN)
				ieptr[MAXSESSIONSECRETLEN] = 0;
			dprintf("Session secret: %s\n", ie->sessionsecret);
			break;
		case IE_SERVERSLIST:
			ie->serverslist = (char *)ieptr;
			ieptr[ielen] = 0;
			if(ielen > MAXMESSAGELEN)
				ieptr[MAXMESSAGELEN] = 0;
			dprintf("Servers list: %s\n", ie->message);
			break;
		case IE_KEYPAD:
			if(ielen >= 1)
			{
				ie->keypad = ieptr[0];
				dprintf("Keypad: \'%c\' (%u)\n", ie->keypad);
			}
			break;
		case IE_SERIAL:
			if(ielen >= 4)
			{
				ie->serial = ieptr[0] << 24 | ieptr[1] << 16 | ieptr[2] << 8 | ieptr[3];
				dprintf("Serial: %u\n", ie->serial);
			}
			break;
		case IE_TRANSPARENTDATA:
			ie->transparentdata = (char *)ieptr;
			ie->transparentdatalen = ielen;
			dprintf("Transparent data\n");
			break;
		case IE_PROTVERSION:
			if(ielen >= 1)
				ie->protversion = ieptr[0];
			break;
		case IE_CALLID:
			if(ielen >= 16)
			{
				memcpy(ie->callid, ieptr, 16);
				ie->callidpresent = true;
			}
			break;
		case IE_QUOTA:
			if(ielen >= 4)
			{
				ie->quota = (int)(ieptr[0] << 24 | ieptr[1] << 16 | ieptr[2] << 8 | ieptr[3]);
				dprintf("Quota: %d\n", ie->quota);
			}
			break;
		case IE_SMSQUOTA:
			if(ielen >= 4)
			{
				ie->smsquotapresent = true;
				ie->smsquota = (int)(ieptr[0] << 24 | ieptr[1] << 16 | ieptr[2] << 8 | ieptr[3]);
				dprintf("SMS Quota: %d\n", ie->smsquota);
			}
			break;
		case IE_FIRSTNAME:
			ie->firstname = (char *)ieptr;
			ieptr[ielen] = 0;
			if(ielen > MAXNAMELEN)
				ieptr[MAXNAMELEN] = 0;
			dprintf("First name: %s\n", ie->firstname);
			break;
		case IE_LASTNAME:
			ie->lastname = (char *)ieptr;
			ieptr[ielen] = 0;
			if(ielen > MAXNAMELEN)
				ieptr[MAXNAMELEN] = 0;
			dprintf("Last name: %s\n", ie->lastname);
			break;
		case IE_EMAIL:
			ie->email = (char *)ieptr;
			ieptr[ielen] = 0;
			if(ielen > MAXNAMELEN)
				ieptr[MAXNAMELEN] = 0;
			dprintf("E-mail: %s\n", ie->email);
			break;
		case IE_PHONENUMBER:
			ie->phonenumber = (char *)ieptr;
			ieptr[ielen] = 0;
			if(ielen > MAXNAMELEN)
				ieptr[MAXNAMELEN] = 0;
			dprintf("Phone number: %s\n", ie->phonenumber);
			break;
		case IE_COUNTRYID:
			ie->country = (char *)ieptr;
			ieptr[ielen] = 0;
			if(ielen > 2)
				ieptr[2] = 0;
			dprintf("Country: %s\n", ie->country);
			break;
		case IE_EXPIRATION:
			if(ielen >= 4)
			{
				ie->expiration = ieptr[0] << 24 | ieptr[1] << 16 | ieptr[2] << 8 | ieptr[3];
				dprintf("Expiration: %u\n", ie->expiration);
			}
			break;
		}
	}
	return 0;
}

IE_ELEMENTS *DupIE(IE_ELEMENTS *ie)
{
	IE_ELEMENTS *nie = (IE_ELEMENTS *)malloc(sizeof(IE_ELEMENTS));

	memcpy(nie, ie, sizeof(IE_ELEMENTS));
	nie->packetbuf = (char *)malloc(ie->packetlen + 1);
	nie->packetbuf[ie->packetlen] = 0;
	memcpy(nie->packetbuf, ie->packetbuf, ie->packetlen);
	nie->srcnum = *ie->srcnum ? nie->packetbuf + (ie->srcnum - ie->packetbuf) : &nie->nullchar;
	nie->srcname =	*ie->srcname ? nie->packetbuf + (ie->srcname - ie->packetbuf) : &nie->nullchar;
	nie->destname =	*ie->destname ? nie->packetbuf + (ie->destname - ie->packetbuf) : &nie->nullchar;
	nie->srcsubaddr = *ie->srcsubaddr ? nie->packetbuf + (ie->srcsubaddr - ie->packetbuf) : &nie->nullchar;
	nie->destnum = *ie->destnum ? nie->packetbuf + (ie->destnum - ie->packetbuf) : &nie->nullchar;
	nie->destsubaddr = *ie->destsubaddr ? nie->packetbuf + (ie->destsubaddr - ie->packetbuf) : &nie->nullchar;
	nie->message = *ie->message ? nie->packetbuf + (ie->message - ie->packetbuf) : &nie->nullchar;
	nie->filename = *ie->filename ? nie->packetbuf + (ie->filename - ie->packetbuf) : &nie->nullchar;
	nie->deflnum = *ie->deflnum ? nie->packetbuf + (ie->deflnum - ie->packetbuf) : &nie->nullchar;
	nie->serveraccesscode = *ie->serveraccesscode ? nie->packetbuf + (ie->serveraccesscode - ie->packetbuf) : &nie->nullchar;
	nie->fileslist = *ie->fileslist ? nie->packetbuf + (ie->fileslist - ie->packetbuf) : &nie->nullchar;
	nie->sessionid = *ie->sessionid ? nie->packetbuf + (ie->sessionid - ie->packetbuf) : &nie->nullchar;
	nie->sessionsecret = *ie->sessionsecret ? nie->packetbuf + (ie->sessionsecret - ie->packetbuf) : &nie->nullchar;
	nie->serverslist = *ie->serverslist ? nie->packetbuf + (ie->serverslist - ie->packetbuf) : &nie->nullchar;
	nie->transparentdata = ie->transparentdata ? nie->packetbuf + (ie->transparentdata - ie->packetbuf) : 0;
	nie->firstname = *ie->firstname ? nie->packetbuf + (ie->firstname - ie->packetbuf) : &nie->nullchar;
	nie->lastname = *ie->lastname ? nie->packetbuf + (ie->lastname - ie->packetbuf) : &nie->nullchar;
	nie->email = *ie->email ? nie->packetbuf + (ie->email - ie->packetbuf) : &nie->nullchar;
	nie->phonenumber = *ie->email ? nie->packetbuf + (ie->email - ie->packetbuf) : &nie->nullchar;
	nie->country = *ie->country ? nie->packetbuf + (ie->country - ie->packetbuf) : &nie->nullchar;

	if(ie->rtfdata)	nie->rtfdata = nie->packetbuf + (ie->rtfdata - ie->packetbuf);
	if(ie->encodednumbersptr)	nie->encodednumbersptr = (DWORD *)(nie->packetbuf + ((char *)ie->encodednumbersptr - ie->packetbuf));
	if(ie->authdata)	nie->authdata = (BYTE *)(nie->packetbuf + ((char *)ie->authdata - ie->packetbuf));
	return nie;
}

void FreeIE(IE_ELEMENTS *ie)
{
	if(ie)
	{
		free(ie->packetbuf);
		free(ie);
	}
}

void PlainNumber(const char *vpnumber, char *plainnumber)
{
	int i;

	for(i = 0; i < vpnumber[i]; i++)
	{
		if(!(vpnumber[i] >= '0' && vpnumber[i] <= '9' || vpnumber[i] == '+' || vpnumber[i] == '-' ||
			i == 0 && (vpnumber[i] == 'v' || vpnumber[i] == 'V') || i == 1 && (vpnumber[i] == 'p' || vpnumber[i] == 'P')))
		{
			strcpy(plainnumber, vpnumber);
			return;
		}
	}
	for(i = 0; vpnumber[i]; i++)
		if(vpnumber[i] >= '0' && vpnumber[i] <= '9')
			*plainnumber++ = vpnumber[i];
	*plainnumber = 0;
}

ADDRESSTYPE AddressType(const char *address)
{
	const char *p;
	int digits, i;

	p = strchr(address, '@');
	if(p)
		address = p + 1;
	if(!memicmp(address, "ip:", 3))
		return ADDRT_IPADDR;
	if(*address == '+')
	{
		while(*++address)
			if(!(*address >= '0' && *address <= '9' || *address == '*' || *address == '#' || *address == '-'))
				return ADDRT_INVALID;
		return ADDRT_PSTN;
	} else if(!stricmp(address, "vPdata-voicebox-tcp") || !stricmp(address, "vPdata-contacts-tcp") ||
		!stricmp(address, "vsearch") || !stricmp(address, "vproxy"))
		return ADDRT_SPECIAL;
	
	// Check if vPNumber
	if(!memicmp(address, "vp", 2))
		p = address + 2;
	else p = address;
	digits = 0;
	for(i = 0; p[i]; i++)
	{
		if(p[i] >= '0' && p[i] <= '9')
			digits++;
		else if(p[i] != '-')
			break;
	}
	if(!p[i] && (digits == 7 || digits == 10))
		return ADDRT_VNUMBER;
	if(address[0] >= 'a' && address[0] <= 'z' || address[0] >= 'A' && address[0] <= 'Z')
		return ADDRT_VNAME;
	return ADDRT_INVALID;
}

ADDRESSTYPE CanonicFormAddress(char *address)
{
	ADDRESSTYPE type = AddressType(address);
	char *p;

	p = strchr(address, '@');
	if(p)
		address = p + 1;
	if(type == ADDRT_PSTN || type == ADDRT_VNUMBER)
	{
		if(!memicmp(address, "vp", 2))
			memmove(address, address + 2, strlen(address + 1));
		while(*address)
		{
			if(*address == '-')
				memmove(address, address + 1, strlen(address));
			else address++;
		}
	}
	return type;
}

int IsAddressVName(const char *address)
{
	int digits = 0, i;
	const char *p;

	p = strchr(address, '@');
	if(p)
		address = p + 1;
	if(*address == '+' || !stricmp(address, "vPdata-voicebox-tcp") || !stricmp(address, "vPdata-contacts-tcp") ||
		!stricmp(address, "vsearch") || !stricmp(address, "vproxy"))
		return 0;
	if(!memicmp(address, "vp", 2))
	{
		for(i = 2; address[i]; i++)
		{
			if(address[i] >= 'a' && address[i] <= 'z' || address[i] >= 'A' && address[i] <= 'Z')
				return 1;
			if(address[i] >= '0' && address[i] <= '9')
				digits++;
		}
		if(digits == 7 || digits == 10)
			return 0;
		return 1;
	}
	if(address[0] >= 'a' && address[0] <= 'z' || address[0] >= 'A' && address[0] <= 'Z')
		return 1;
	return 0;
}

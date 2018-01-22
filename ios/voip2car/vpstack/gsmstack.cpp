#include <windows.h>
#include <stdio.h>
#include <time.h>
#include "vpstack.h"
#include "gsmstack.h"
#include "serialport.h"
#include "pdu.h"

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
#define TSTART	InterlockedIncrement(&nthreads);
#define TRETURN	{InterlockedDecrement(&nthreads); return; }
enum {ATPROT_DEFAULT, ATPROT_HUAWEI};

IVPSTACK *CreateGSMSTACK(VPAUDIODATAFACTORY *vpaf, const char *stackname)
{
	return new GSMSTACK(vpaf, stackname);
}

GSMSTACK::GSMSTACK(VPAUDIODATAFACTORY *vpaf, const char *stackname)
{
	vpaudiodatafactory = vpaf;
	nthreads = 0;
	hCmdPort = hAudioPort = 0;
	nw = nr = 0;
	lastvpcallvalue = 0;
	signal = 0;
	*oper = 0;
	network3g = false;
	vpcallvoice = vpcallsms = 0;
	canrun = false;
	atprotocol = ATPROT_DEFAULT;
	Zero(calls);
	Zero(settings);
	settings.codec = CODEC_G711U;
	if(stackname)
		strncpy2(this->stackname, stackname, sizeof(this->stackname));
	else *this->stackname = 0;
	NotifyRoutine = 0;
	Lprintf("[GSMSTACK] Created");
}

GSMSTACK::~GSMSTACK()
{
	int i;
	VPCALL vpcall;

	Lprintf("[GSMSTACK] Destroyed");
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
	do Sleep(300);
	while(vpcallvoice || vpcallsms);
	canrun = false;
	while(nthreads)
		Sleep(10);
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
	if(hCmdPort)
		sp_close(hCmdPort);
	Lprintf("[GSMSTACK] Destroy complete");
}

/**********************************************************************/
/* Locking                                                            */
/**********************************************************************/

void GSMCALLDATA::LockL(int line)
{
#ifdef LOCKDEBUGGING
	char s[100];

	sprintf(s, "%u GSMCALL %x lock in line %d", GetTickCount(), this, line);
	debugstring(s);
	EnterCriticalSection(&cs);
	sprintf(s, "%u GSMCALL %x lock in line %d passed", GetTickCount(), this, line);
	debugstring(s);
#else
	EnterCriticalSection(&cs);
#endif
}

void GSMCALLDATA::UnlockL(int line)
{
#ifdef LOCKDEBUGGING
	char s[100];

	sprintf(s, "%u GSMCALL %x unlock in line %d", GetTickCount(), this, line);
	debugstring(s);
#endif
	LeaveCriticalSection(&cs);
}

void GSMSTACK::ReadLockL(int line)
{
#ifdef LOCKDEBUGGING
	char s[100];

	sprintf(s, "%u GSMSTACK read lock in line %d, nr=%d, nw=%d", GetTickCount(), line, nr, nw);
	debugstring(s);
	lock.RLock();
	nr++;
	sprintf(s, "%u GSMSTACK read lock in line %d passed, nr=%d, nw=%d", GetTickCount(), line, nr, nw);
	debugstring(s);
#else
	lock.RLock();
#endif
}

void GSMSTACK::ReadUnlockL(int line)
{
#ifdef LOCKDEBUGGING
	char s[100];

	nr--;
	sprintf(s, "%u GSMSTACK read unlock in line %d, nr=%d, nw=%d", GetTickCount(), line, nr, nw);
	debugstring(s);
#endif
	lock.RUnlock();
}

void GSMSTACK::WriteLockL(int line)
{
#ifdef LOCKDEBUGGING
	char s[100];

	sprintf(s, "%u GSMSTACK write lock in line %d, nr=%d, nw=%d", GetTickCount(), line, nr, nw);
	debugstring(s);
	lock.WLock();
	nw++;
	sprintf(s, "%u GSMSTACK write lock in line %d passed, nr=%d, nw=%d", GetTickCount(), line, nr, nw);
	debugstring(s);
#else
	lock.WLock();
#endif
}

void GSMSTACK::WriteUnlockL(int line)
{
#ifdef LOCKDEBUGGING
	char s[100];

	nw--;
	sprintf(s, "%u GSMSTACK write unlock in line %d, nr=%d, nw=%d", GetTickCount(), line, nr, nw);
	debugstring(s);
#endif
	lock.WUnlock();
}

VPCALL GSMSTACK::CreateCall()
{
	int i;
	VPCALL vpcall;
	GSMCALLDATA *vpc;

	WriteLock();
	for(i = 0; i < MAXCALLS; i++)
		if(!calls[i])
		{
			vpc = new GSMCALLDATA;
			calls[i] = vpc;
			calls[i]->start_ticks = GetTickCount();
			time(&calls[i]->start_time);
			vpc->vpcall = vpcall = (VPCALL)++lastvpcallvalue;
			vpc->aa.f = -1;
			WriteUnlock();
			return vpcall;
		}
	WriteUnlock();
	return 0;
}

void GSMSTACK::FreeCall(VPCALL vpcall)
{
	int i;

retry:
	WriteLock();
	for(i = 0; i < MAXCALLS; i++)
	{
		if(calls[i] && calls[i]->vpcall == vpcall)
		{
			calls[i]->Lock();
			if(calls[i]->inuse)
			{
				calls[i]->deleting = true;
				calls[i]->Unlock();
				WriteUnlock();
				Sleep(10);
				goto retry;
			}
			if(calls[i]->audio)
				delete calls[i]->audio;
			delete calls[i];
			calls[i] = 0;
			if(vpcall == vpcallvoice)
				vpcallvoice = 0;
			if(vpcall == vpcallsms)
				vpcallsms = 0;
			break;
		}
	}
	WriteUnlock();
}

GSMCALLDATA *GSMSTACK::LockCallL(int index, int line)
{
	if((unsigned)index < MAXCALLS)
	{
#ifdef LOCKDEBUGGING
		char s[100];

		sprintf(s, "%u LockCall lock in line %d, index = %d", GetTickCount(), line, index);
		debugstring(s);
#endif
		ReadLock();
		GSMCALLDATA *vpc = calls[index];
		if(vpc)
			vpc->Lock();
		ReadUnlock();
		return vpc;
	}
	return 0;
}

GSMCALLDATA *GSMSTACK::LockInUseCall(VPCALL vpcall)
{
	int i;

	ReadLock();
	for(i = 0; i < MAXCALLS; i++)
	{
		if(calls[i] && calls[i]->vpcall == vpcall)
		{
			GSMCALLDATA *vpc = calls[i];
			if(vpc)
			{
				if(vpc->inuse)
					vpc->inuse--;
				if(vpc->deleting)
					vpc = 0;
				else vpc->Lock();
			}
			ReadUnlock();
			return vpc;
		}
	}
	ReadUnlock();
	return 0;
}

GSMCALLDATA *GSMSTACK::LockCallL(VPCALL vpcall, int line)
{
	int i;

#ifdef LOCKDEBUGGING
	char s[100];

	sprintf(s, "%u LockCall lock in line %d, vpcall = %d", GetTickCount(), line, vpcall);
	debugstring(s);
#endif
	ReadLock();
	for(i = 0; i < MAXCALLS; i++)
	{
		if(calls[i] && calls[i]->vpcall == vpcall)
		{
			GSMCALLDATA *vpc = calls[i];
			if(vpc)
				vpc->Lock();
			ReadUnlock();
			return vpc;
		}
	}
	ReadUnlock();
	return 0;
}

int GSMSTACK::EnumCalls(VPCALL *vpcalls, unsigned maxvpcalls, unsigned mask)
{
	unsigned n = 0, i;

	if(!(mask & (1 << BC_VOICE)))
		return 0;
	ReadLock();
	for(i = 0; i < MAXCALLS; i++)
	{
		if(calls[i])
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

/**********************************************************************/
/* Public functions: general                                          */
/**********************************************************************/

int GSMSTACK::Init()
{
	canrun = true;
	TSTART
	BEGINCLASSTHREAD(&GSMSTACK::cmdportworkerthread, this, 0);
	return 0;
}

int GSMSTACK::SetCallLocalName(VPCALL vpcall, const char *username)
{
	GSMCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	strncpy2(vpc->ourname, username, MAXNAMELEN+1);
	vpc->Unlock();
	return 0;
}

int GSMSTACK::SetCallLocalNumber(VPCALL vpcall, const char *number)
{
	GSMCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	strncpy2(vpc->ournumber, number, MAXNUMBERLEN+1);
	vpc->Unlock();
	return 0;
}

int GSMSTACK::GetCallRemoteName(VPCALL vpcall, char *username)
{
	GSMCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	if(*vpc->connectedname)
		strncpy2(username, vpc->connectedname, MAXNAMELEN+1);
	else strncpy2(username, vpc->connectednumber, MAXNUMBERLEN+1);
	vpc->Unlock();
	return 0;
}

int GSMSTACK::GetCallRemoteNumber(VPCALL vpcall, char *number)
{
	GSMCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	strncpy2(number, vpc->connectednumber, MAXNUMBERLEN+1);
	vpc->Unlock();
	return 0;
}

int GSMSTACK::GetCallLocalName(VPCALL vpcall, char *username)
{
	GSMCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	if(*vpc->connectedname)
		strncpy2(username, vpc->ourname, MAXNAMELEN+1);
	else if(*vpc->ournumber)
		strncpy2(username, vpc->ournumber, MAXNUMBERLEN+1);
	else *username = 0;
	vpc->Unlock();
	return 0;
}

int GSMSTACK::GetCallLocalNumber(VPCALL vpcall, char *number)
{
	GSMCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	strncpy2(number, vpc->ournumber, MAXNUMBERLEN+1);
	vpc->Unlock();
	return 0;
}

int GSMSTACK::GetCallBearer(VPCALL vpcall)
{
	GSMCALLDATA *vpc;
	int rc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	rc = vpc->bc;
	vpc->Unlock();
	return rc;
}

int GSMSTACK::GetCallCodec(VPCALL vpcall)
{
	return settings.codec;
}

int GSMSTACK::GetCallStatus(VPCALL vpcall)
{
	int status;

	GSMCALLDATA *vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	status = vpc->status;
	vpc->Unlock();
	return status;
}

int GSMSTACK::GetCallText(VPCALL vpcall, char *text)
{
	GSMCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	strcpy(text, vpc->message);
	vpc->Unlock();
	return 0;
}

int GSMSTACK::SetCallText(VPCALL vpcall, const char *text)
{
	GSMCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	strncpy2(vpc->message, text, sizeof vpc->message);
	vpc->Unlock();
	return 0;
}

int GSMSTACK::Connect(VPCALL *vpcall, const char *address, int bc)
{
	VPCALL tmpvpcall;
	int rc;

	Lprintf("[GSMSTACK] Connect1 [%s],%d", address, bc);
	if(bc != BC_VOICE && bc != BC_SMS)
		return VPERR_UNSUPPORTEDBEARER;
	if(!*oper)
		return VPERR_NOTLOGGEDON;
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

int GSMSTACK::Connect(VPCALL vpcall, const char *address, int bc)
{
	GSMCALLDATA *vpc;
	int rc;

	Lprintf("[GSMSTACK] Connect2 %d,[%s],%d", vpcall, address, bc);
	if(bc != BC_VOICE && bc != BC_SMS)
		return VPERR_UNSUPPORTEDBEARER;
	if(!*oper)
		return VPERR_NOTLOGGEDON;
	if(bc == BC_VOICE && vpcallvoice || bc == BC_SMS && vpcallsms)
		return VPERR_INVALIDSTATUS;
	rc = SetCallAddress(vpcall, address);
	if(rc)
		return rc;

	vpc = LockCall(vpcall);
	if(vpc)
	{
		vpc->bc = bc;
		vpc->status = VPCS_CONNECTING;
		if(bc == BC_SMS)
		{
			encodepdu(vpc->destnumber, vpc->message, vpc->report, vpc->pdu);
			vpcallsms = vpcall;
		} else {
			vpcallvoice = vpcall;
			TSTART
			BEGINCLASSTHREAD(&GSMSTACK::audioworkerthread, this, 0);
		}
		vpc->Unlock();
	} else return VPERR_INVALIDCALL;
	return 0;
}

int GSMSTACK::SetCallAddress(VPCALL vpcall, const char *address)
{
	GSMCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	strcpy(vpc->destnumber, address);
	strcpy(vpc->connectednumber, vpc->destnumber);
	vpc->Unlock();
	return 0;
}

int GSMSTACK::Disconnect(VPCALL vpcall, int reason)
{
	Lprintf("[GSMSTACK] Disconnect %d,%d", vpcall, reason);
	GSMCALLDATA *vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;

	if(vpc->status == VPCS_INCALLPROCEEDING || vpc->status == VPCS_INALERTED)
	{
		vpc->status = VPCS_DISCONNECTING;
		vpc->Unlock();
	} else if(vpc->status >= VPCS_CONNECTING && vpc->status <= VPCS_CONNECTED)
	{
		vpc->status = VPCS_DISCONNECTING;
		vpc->Unlock();
	} else vpc->Unlock();
	return 0;
}

int GSMSTACK::AnswerCall(VPCALL vpcall)
{
	Lprintf("[GSMSTACK] AnswerCall %d", vpcall);
	GSMCALLDATA *vpc = LockCall(vpcall);

	if(!vpc)
		return VPERR_INVALIDCALL;
	if(vpc->status != VPCS_INALERTED)
	{
		vpc->Unlock();
		return VPERR_INVALIDSTATUS;
	}
	vpc->status = VPCS_INCONNECTING;
	vpc->Unlock();
	return 0;
}

void GSMSTACK::Notify(int message, unsigned param1, unsigned param2)
{
	bool notify = true;

	Lprintf("[GSMSTACK] Notify %d,%d,%d", message, param1, param2);
	if(message == VPMSG_CALLESTABLISHED || message == VPMSG_CALLACCEPTED)
	{
		GSMCALLDATA *vpc = LockCall((VPCALL)param1);
		if(vpc)
		{
			CallEstablished(vpc);
			vpc->Unlock();
		}
	} else if(message == VPMSG_CALLENDED)
	{
		GSMCALLDATA *vpc = LockCall((VPCALL)param1);
		if(vpc)
		{
			if(vpc->status == VPCS_TERMINATED)
				notify = false;
			else if(vpc->status && vpc->status < VPCS_TERMINATED)
				vpc->status = VPCS_TERMINATED;
			vpc->Unlock();
		}
	}
	if(notify && NotifyRoutine)
		NotifyRoutine(NotifyParam, message, param1, param2);
}

int GSMSTACK::NotifyRc(int message, unsigned param1, unsigned param2)
{
	return 0;
}

int GSMSTACK::SendKeypad(VPCALL vpcall, int key)
{
	return 0;
}

/**********************************************************************/
/* Other protocol functions                                           */
/**********************************************************************/

int GSMSTACK::SetSyncNotifyRoutine(int (*notify)(void *param, unsigned, unsigned, unsigned), void *param)
{
	return 0;
}

int GSMSTACK::SetNotifyRoutine(void (*notify)(void *param, unsigned, unsigned, unsigned), void *param)
{
	NotifyRoutine = notify;
	NotifyParam = param;
	return 0;
}

// data has been received from GSM: send it to the destination
// after that fill data with data to send to GSM
int GSMSTACK::GSMAudio(BYTE *data, int datalen)
{
	GSMCALLDATA *vpc;
	VPAUDIODATA *audio;
	unsigned ts;

	if(!vpcallvoice)
		return 0;
	vpc = LockCall(vpcallvoice);
	if(vpc)
	{
		if(!vpc->audio)
		{
			vpc->Unlock();
			return 0;
		}
		audio = vpc->audio;
		ts = vpc->rxtimestamp;
		vpc->InUse();
		vpc->Unlock();
		vpc->audio->AudioFrame(settings.codec, ts, data, datalen);
		vpc = LockInUseCall(vpcallvoice);
		if(vpc)
		{
			if(vpc->audiobuf.GetData(data, datalen))
				memset(data, settings.codec == CODEC_G711A ? 0xab : settings.codec == CODEC_G711U ? 0x7f : 0, datalen);
			vpc->Unlock();
		}
		return 1;
	}
	return 0;
}

int	GSMSTACK::SendAudio(VPCALL vpcall, int codec, unsigned timestamp, const void *buf, int len, bool wait)
{
	GSMCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return -1;
	vpc->audiobuf.PutData((unsigned char *)buf, len);
	vpc->Unlock();
	return 0;
}

void GSMSTACK::CallEstablished(GSMCALLDATA *vpc)
{
	vpc->start_ticks = GetTickCount();
	time(&vpc->start_time);
	if(vpaudiodatafactory)
		vpc->audio = vpaudiodatafactory->New(this, vpc->vpcall, settings.codec);
}

int FIFOBUFFER::PutData(const unsigned char *data, unsigned len)
{
	if(head == tail || (head + AUDIOBUFSIZE - tail) % AUDIOBUFSIZE > len)
	{
		if(tail + len <= AUDIOBUFSIZE)
			memcpy(buf + tail, data, len);
		else {
			memcpy(buf + tail, data, AUDIOBUFSIZE - tail);
			memcpy(buf, data + AUDIOBUFSIZE - tail, len - (AUDIOBUFSIZE - tail));
		}
		tail = (tail + len) % AUDIOBUFSIZE;
		return 0;
	}
	return -1;
}

int FIFOBUFFER::GetData(unsigned char *data, unsigned len)
{
	if((tail + AUDIOBUFSIZE - head) % AUDIOBUFSIZE > len)
	{
		if(head + len <= AUDIOBUFSIZE)
			memcpy(data, buf + head, len);
		else {
			memcpy(data, buf + head, AUDIOBUFSIZE - head);
			memcpy(data + AUDIOBUFSIZE - head, buf, len - (AUDIOBUFSIZE - head));
		}
		head = (head + len) % AUDIOBUFSIZE;
		return 0;
	}
	return -1;
}

/**********************************************************************/
/* Supplementary services                                             */
/**********************************************************************/

int GSMSTACK::IsHeld(VPCALL vpcall)
{
	GSMCALLDATA *vpc;
	int rc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	rc = 0;
	vpc->Unlock();
	return rc;
}

int GSMSTACK::Hold(VPCALL vpcall)
{
	GSMCALLDATA *vpc;
	int rc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	rc = 0;
	vpc->Unlock();
	if(rc)
		return VPERR_INVALIDSTATUS;
	return 0;
}

int GSMSTACK::Resume(VPCALL vpcall)
{
	GSMCALLDATA *vpc;
	int rc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	rc = 0;
	vpc->Unlock();
	if(rc)
		return VPERR_INVALIDSTATUS;
	return 0;
}

int GSMSTACK::GetCallTimes(VPCALL vpcall, time_t *start_time, unsigned int *length_ms)
{
	GSMCALLDATA *vpc = LockCall(vpcall);

	if(!vpc)
		return VPERR_INVALIDCALL;
	*start_time = vpc->start_time;
	*length_ms = GetTickCount() - vpc->start_ticks;
	vpc->Unlock();
	return 0;
}

int GSMSTACK::SetServers(const char *list)
{
	strncpy2(settings.portslist, list, sizeof(settings.portslist));
	return 0;
}

int GSMSTACK::ReopenPort(const char *portslist, int index)
{
	char port[100], *port2;

	if(hCmdPort && index == 0)
	{
		sp_close(hCmdPort);
		hCmdPort = 0;
	}
	if(hAudioPort && index == 1)
	{
		sp_close(hAudioPort);
		hAudioPort = 0;
	}
	strncpy2(port, portslist, sizeof(port));
	port2 = strchr(port, ';');
	if(port2)
		*port2++ = 0;
	if(index == 0)
	{
		hCmdPort = sp_open(port);
		if(!hCmdPort)
			return VPERR_FILENOTFOUND;
	} else if(index == 1 && port2)
	{
		hAudioPort = sp_open(port2);
		if(!hAudioPort)
		{
			sp_close(hCmdPort);
			hCmdPort = 0;
			return VPERR_FILENOTFOUND;
		}
	}
	return 0;
}

static int getfield(char **ptr, char *field, int fieldsize)
{
	char *p = *ptr, *ep;
	bool quote = false;

	while(*p == ' ')
		p++;
	if(*p == '\"')
	{
		quote = true;
		p++;
	}
	ep = p;
	while(*ep && (quote && *ep != '\"' || !quote && *ep != ','))
		ep++;
	if(ep - p < fieldsize)
		fieldsize = ep - p;
	memcpy(field, p, fieldsize);
	field[fieldsize] = 0;
	if(*ep && quote)
		ep++;
	while(*ep == ' ')
		ep++;
	if(*ep == ',')
		ep++;
	*ptr = ep;
	return fieldsize;
}

void GSMSTACK::audioworkerthread(void *dummy)
{
	unsigned char audiobuf[320];
	int rc;

	if(ReopenPort(settings.portslist, 1))
	{
		Notify(VPMSG_SERVERSTATUS, REASON_SERVERNOTRESPONDING, 1);
		TRETURN
	}
	if(!hAudioPort)
		TRETURN
	while(vpcallvoice)
	{
		rc = sp_read(hAudioPort, audiobuf, settings.codec == CODEC_LINEARBE || settings.codec == CODEC_LINEARLE ? 320 : 160);
		if(rc > 0)
		{
			GSMAudio(audiobuf, rc);
			sp_write(hAudioPort, audiobuf, rc);
		} else Sleep(10);
	}
	if(hAudioPort)
	{
		sp_close(hAudioPort);
		hAudioPort = 0;
	}
	TRETURN
}

int GSMSTACK::Logon(const char *username, const char *password)
{
	strncpy2(settings.pin, password, sizeof settings.pin);
	return 0;
}

static int DecodeATSMS(const char *pdu, SMS *sms)
{
	const char *r;
	int len, rc;

	if(!memicmp(pdu, "+CMT: ", 6))
	{
		len = atoi(pdu + 7);
		pdu = strchr(pdu, '\n');
		if(!pdu)
			return -1;
		while(isspace(*pdu))
			pdu++;
		rc = 0;
	} else if(!memicmp(pdu, "+CDS: ", 6))
	{
		len = atoi(pdu + 6);
		pdu = strchr(pdu, '\n');
		if(!pdu)
			return -1;
		while(isspace(*pdu))
			pdu++;
		rc = 0;
	} else if(!memicmp(pdu, "+CMGL: ", 7))
	{
		pdu += 7;
		rc = atoi(pdu);
		while(*pdu && isdigit(*pdu) && !isspace(*pdu))
			pdu++;
		if(!*pdu || isspace(*pdu))
			return -1;
		pdu++;
		if(!isdigit(*pdu))
			return -1;
		if(atoi(pdu) > 1)	// type != DELIVER
			return -1;
		while(*pdu && isdigit(*pdu) && !isspace(*pdu))
			pdu++;
		if(!*pdu || isspace(*pdu))
			return -1;
		pdu++;
		if(*pdu == ',')
			pdu++;
		if(!isdigit(*pdu))
			return -1;
		len = atoi(pdu);
		pdu = strchr(pdu, '\n');
		if(!pdu)
			return -1;
		while(isspace(*pdu))
			pdu++;
	}
	for(r = pdu; *r >= '0' && *r <= '9' || *r >= 'A' && *r <= 'F' || *r >= 'a' && *r <= 'f'; r++);
	if(r - 2 * len < pdu)
		return -1;
	if(decodepdu(r - 2 * len, sms) || sms->submit)
		return -1;
	return rc;
}

void GSMSTACK::cmdportworkerthread(void *dummy)
{
	char portslist[100];
	char line[1000];
	char field[100], *p;
	int rc;
	unsigned lastreq = GetTickCount() - 5000;
	VPCALL lockoncall = 0;
	VPCALL notifyoncall = 0;
	bool online = false;
	bool pingiven = false;

	*portslist = 0;
	while(canrun)
	{
		if(strcmp(portslist, settings.portslist))
		{
			strcpy(portslist, settings.portslist);
			if(ReopenPort(portslist, 0))
				Notify(VPMSG_SERVERSTATUS, REASON_NOTFOUND, 0);
			else if(hCmdPort)
				sp_writestring(hCmdPort, "AT+CLIP=1;+CGMI;+CPIN?\r");
		}
		if(!hCmdPort)
		{
			Sleep(10);
			continue;
		}
		if(!lockoncall && (int)(GetTickCount() - lastreq) > 1000)
		{
			sp_writestring(hCmdPort, "AT+CSQ;+COPS?\r");
			sp_writestring(hCmdPort, "AT+CMGL=4\r");
			lastreq = GetTickCount();
		}
		while((rc = sp_readline(hCmdPort, line, sizeof(line))) > 0)
		{
			if(!stricmp(line, "huawei"))
			{
				atprotocol = ATPROT_HUAWEI;
				settings.codec = CODEC_LINEARLE;
			} else if(!strnicmp(line, "+CPIN:", 6))
			{
				if(!pingiven && strstr(line, "SIM PIN") && *settings.pin)
				{
					pingiven = true;
					sp_printf(hCmdPort, "AT+CPIN=%s\r", settings.pin);
				}
			} else if(!strnicmp(line, "+COPS:", 6))
			{
				p = line+6;
				getfield(&p, field, sizeof field);
				getfield(&p, field, sizeof field);
				getfield(&p, oper, sizeof oper);
				getfield(&p, field, sizeof field);
				if(atoi(field) == 2)
					network3g = true;
				else network3g = false;
				if(*oper && !online)
				{
					online = true;
					Notify(VPMSG_SERVERSTATUS, REASON_LOGGEDON, 0);
				} else if(!*oper && online)
				{
					online = false;
					Notify(VPMSG_SERVERSTATUS, REASON_SERVERNOTRESPONDING, 0);
				}
			} else if(!strnicmp(line, "+CSQ:", 5))
				if(atoi(line+5) == 99)
					signal = 0;
				else signal = -113 + atoi(line+5) * 2;
			else if(!stricmp(line, "CONNECT") && vpcallvoice)
			{
				// This is only used by ZTE
				GSMCALLDATA *vpc = LockCall(vpcallvoice);
				if(!vpc)
					vpcallvoice = 0;
				else {
					if(vpc->status == VPCS_CALLPROCEEDING || vpc->status == VPCS_INCALLPROCEEDING)
						vpc->status = VPCS_CONNECTED;
					vpc->Unlock();
					if(lockoncall && lockoncall == vpcallvoice)
						lockoncall = 0;
					NotifyOnCall(VPMSG_CALLACCEPTED, vpcallvoice, 0);
				}
			} else if(!stricmp(line, "RING") || !strnicmp(line, "+CRING", 6))
			{
				if(!vpcallvoice)
				{
					VPCALL vpcall = CreateCall();
					if(vpcall)
					{
						GSMCALLDATA *vpc = LockCall(vpcall);
						if(vpc)
						{
							vpc->status = VPCS_INALERTED;
							vpc->bc = BC_VOICE;
							vpc->Unlock();
							vpcallvoice = vpcall;
							TSTART
							BEGINCLASSTHREAD(&GSMSTACK::audioworkerthread, this, 0);
							notifyoncall = vpcall;
						}
					}
				}
			} else if(!stricmp(line, "OK") && vpcallvoice && lockoncall == vpcallvoice)
			{
				// This is only used by ZTE
				// ZTE answers OK, when the call cannot be made, otherwise it answers CONNECT
				GSMCALLDATA *vpc = LockCall(vpcallvoice);
				if(vpc)
				{
					vpc->Unlock();
					NotifyOnCall(VPMSG_CALLREFUSED, vpcallvoice, 0);
					NotifyOnCall(VPMSG_CALLENDED, vpcallvoice, 0);
					vpcallvoice = 0;
					lockoncall = 0;
				}
			} else if(!strnicmp(line, "+CLIP:", 6))
			{
				if(vpcallvoice)
				{
					GSMCALLDATA *vpc = LockCall(vpcallvoice);
					if(vpc)
					{
						if(vpc->status == VPCS_INALERTED)
						{
							p = line+6;
							getfield(&p, field, sizeof field);
							strncpy2(vpc->connectednumber, field, MAXNUMBERLEN+1);
						}
						vpc->Unlock();
					}
				}
			} else if(!stricmp(line, "STOPRING") || !strnicmp(line, "HANGUP", 6) || !strnicmp(line, "^CEND", 5))
			{
				if(vpcallvoice)
				{
					GSMCALLDATA *vpc = LockCall(vpcallvoice);
					if(vpc)
					{
						vpc->Unlock();
						NotifyOnCall(VPMSG_CALLDISCONNECTED, vpcallvoice, REASON_NORMAL);
						NotifyOnCall(VPMSG_CALLENDED, vpcallvoice, REASON_NORMAL);
						vpcallvoice = 0;
					}
				}
			} else if(!stricmp(line, "> ") && vpcallsms && lockoncall == vpcallsms)
			{
				GSMCALLDATA *vpc = LockCall(vpcallsms);
				if(!vpc)
				{
					vpcallsms = 0;
					lockoncall = 0;
				} else {
					if(vpc->status == VPCS_CALLPROCEEDING)
					{
						vpc->status = VPCS_CALLPROCEEDING;
						sprintf(line, "%s\032", vpc->pdu);
						vpc->status = VPCS_CONNECTED;
					} else *line = 0;
					vpc->Unlock();
					if(*line)
						sp_writestring(hCmdPort, line);
				}
			} else if(!stricmp(line, "ERROR") && vpcallsms && lockoncall == vpcallsms)
			{
				GSMCALLDATA *vpc = LockCall(vpcallsms);
				if(vpc)
				{
					vpc->Unlock();
					NotifyOnCall(VPMSG_CALLREFUSED, vpcallsms, 0);
					NotifyOnCall(VPMSG_CALLENDED, vpcallsms, 0);
					vpcallsms = 0;
					lockoncall = 0;
				}
			} else if(!strnicmp(line, "+CMGS:", 6) && vpcallsms && lockoncall == vpcallsms)
			{
				GSMCALLDATA *vpc = LockCall(vpcallsms);
				if(vpc)
				{
					vpc->smsref = atoi(line+6);
					vpc->Unlock();
					NotifyOnCall(VPMSG_CALLACCEPTED, vpcallsms, 0);
					NotifyOnCall(VPMSG_CALLENDED, vpcallsms, 0);
					vpcallsms = 0;
					lockoncall = 0;
				}
			} else if(!strnicmp(line, "+CMGL:", 6))
			{
				SMS sms;
				int rc;

				strcat(line, "\n");
				rc = sp_readline(hCmdPort, line + strlen(line), sizeof(line) - strlen(line));
				if(rc > 0)
				{
					rc = DecodeATSMS(line, &sms);
					if(rc != -1)
					{
						VPCALL vpcall = CreateCall();
						if(vpcall)
						{
							GSMCALLDATA *vpc = LockCall(vpcall);
							if(vpc)
							{
								strcpy(vpc->message, sms.text);
								strcpy(vpc->connectednumber, sms.snum);
								vpc->status = VPCS_CONNECTED;
								vpc->bc = BC_SMS;
								vpc->Unlock();
								NotifyOnCall(VPMSG_NEWCALL, vpcall, 0);
								NotifyOnCall(VPMSG_CALLENDED, vpcall, 0);
								sp_printf(hCmdPort, "AT+CMGD=%d\r", rc);	// Delete message from memory
							}
						}
					}
				}
			}
		}
		if(notifyoncall)
		{
			NotifyOnCall(VPMSG_NEWCALL, notifyoncall, 0);
			notifyoncall = 0;
		}
		if(vpcallvoice && (!lockoncall || lockoncall == vpcallvoice))
		{
			GSMCALLDATA *vpc = LockCall(vpcallvoice);
			if(!vpc)
			{
				vpcallvoice = 0;
				if(lockoncall == vpcallvoice)
					lockoncall = 0;
			} else {
				int notify = 0;
				if(vpc->status == VPCS_INCONNECTING)
				{
					vpc->status = VPCS_INCALLPROCEEDING;
					strcpy(line, "ATA\r");
					if(atprotocol == ATPROT_HUAWEI)
					{
						sp_writestring(hCmdPort, "AT^DDSETEX=2\r");
						vpc->status = VPCS_CONNECTED;
						notify = VPMSG_CALLESTABLISHED;
					}
				} else if(vpc->status == VPCS_CONNECTING)
				{
					vpc->status = VPCS_CALLPROCEEDING;
					sprintf(line, "ATD%s;\r", vpc->destnumber);
					if(atprotocol == ATPROT_HUAWEI)
					{
						// Huawei has no reporting of connect success
						// Start audio channel
						lastreq = GetTickCount() + 5000;
						sp_writestring(hCmdPort, "AT^DDSETEX=2\r");
						vpc->status = VPCS_CONNECTED;
						notify = VPMSG_CALLACCEPTED;
					} else {
						// ZTE blocks until there is an answer
						lockoncall = vpcallvoice;
					}
				} else if(vpc->status == VPCS_DISCONNECTING)
				{
					strcpy(line, "AT+CHUP\r");
					if(lockoncall == vpcallvoice)
						lockoncall = 0;
					notify = VPMSG_CALLENDED;
				} else *line = 0;
				vpc->Unlock();
				if(notify)
					NotifyOnCall(notify, vpcallvoice, 0);
				if(*line)
					sp_writestring(hCmdPort, line);
			}
		}
		if(!lockoncall && vpcallsms)
		{
			GSMCALLDATA *vpc = LockCall(vpcallsms);
			if(!vpc)
			{
				vpcallsms = 0;
				if(lockoncall == vpcallsms)
					lockoncall = 0;
			} else {
				if(vpc->status == VPCS_CONNECTING)
				{
					vpc->status = VPCS_CALLPROCEEDING;
					sprintf(line, "AT+CMGS=%d\r", strlen(vpc->pdu) / 2 - 1);
					lockoncall = vpcallsms;
				} else *line = 0;
				vpc->Unlock();
				if(*line)
					sp_writestring(hCmdPort, line);
			}
		}
		if(rc == -1)
			*portslist = 0;
		Sleep(10);
	}
	vpcallvoice = vpcallsms = 0;
	TRETURN
}

int GSMSTACK::NetworkQuality()
{
	if(signal)
	{
		if(signal < -110)
			return 0;
		if(signal >= -70)
			return 5;
		return (signal + 120) / 10;
	}
	return VPERR_NOTLOGGEDON;
}

int GSMSTACK::SignalLevel(int *dBm)
{
	if(signal)
	{
		*dBm = signal;
		return 0;
	}
	return VPERR_NOTLOGGEDON;
}

int GSMSTACK::GetOperatorName(char *name, int namesize, unsigned *flags)
{
	if(*oper)
	{
		strncpy2(name, oper, namesize);
		*flags = network3g ? 1 : 0;
		return 0;
	}
	return VPERR_NOTLOGGEDON;
}

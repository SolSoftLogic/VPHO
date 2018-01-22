#include <windows.h>
#include <stdio.h>
#include <time.h>
#include "vpstack.h"
#include "isdn.h"

#define Lock() LockL(__LINE__)
#define Unlock() UnlockL(__LINE__)
#define ReadLock() ReadLockL(__LINE__)
#define ReadUnlock() ReadUnlockL(__LINE__)
#define WriteLock() WriteLockL(__LINE__)
#define WriteUnlock() WriteUnlockL(__LINE__)
#define LockCall(a) LockCallL(a, __LINE__)
#define TSTART	InterlockedIncrement(&nthreads);
#define TRETURN	{InterlockedDecrement(&nthreads); return; }
#define ISDNBUFFERLEN 160

static ISDNSTACK *vps;

ISDNSTACK::ISDNSTACK(VPAUDIODATAFACTORY *vpaf)
{
	vpaudiodatafactory = vpaf;
	vps = this;
	nthreads = 0;
	nw = nr = 0;
	lastvpcallvalue = 0;
	Zero(calls);
	Zero(settings);
	settings.dialoutcontroller = 1;
	settings.callflags = CAPIFLAG_EARLYB3;
	settings.codec = CODEC_G711A;
}

ISDNSTACK::~ISDNSTACK()
{
	CAPI_Exit();
}

/**********************************************************************/
/* Locking                                                            */
/**********************************************************************/

void ISDNCALLDATA::LockL(int line)
{
#ifdef LOCKDEBUGGING
	char s[100];

	sprintf(s, "%u ISDNCALL lock in line %d\n", GetTickCount(), line);
	debugstring(s);
	EnterCriticalSection(&cs);
	sprintf(s, "%u ISDNCALL lock in line %d passed\n", GetTickCount(), line);
	debugstring(s);
#else
	EnterCriticalSection(&cs);
#endif
}

void ISDNCALLDATA::UnlockL(int line)
{
#ifdef LOCKDEBUGGING
	char s[100];

	sprintf(s, "%u ISDNCALL unlock in line %d\n", GetTickCount(), line);
	debugstring(s);
#endif
	LeaveCriticalSection(&cs);
}

void ISDNSTACK::ReadLockL(int line)
{
#ifdef LOCKDEBUGGING
	char s[100];

	sprintf(s, "%u ISDNSTACK read lock in line %d, nr=%d, nw=%d", GetTickCount(), line, nr, nw);
	debugstring(s);
	lock.RLock();
	nr++;
	sprintf(s, "%u ISDNSTACK read lock in line %d passed, nr=%d, nw=%d", GetTickCount(), line, nr, nw);
	debugstring(s);
#else
	lock.RLock();
#endif
}

void ISDNSTACK::ReadUnlockL(int line)
{
#ifdef LOCKDEBUGGING
	char s[100];

	nr--;
	sprintf(s, "%u ISDNSTACK read unlock in line %d, nr=%d, nw=%d", GetTickCount(), line, nr, nw);
	debugstring(s);
#endif
	lock.RUnlock();
}

void ISDNSTACK::WriteLockL(int line)
{
#ifdef LOCKDEBUGGING
	char s[100];

	sprintf(s, "%u ISDNSTACK write lock in line %d, nr=%d, nw=%d", GetTickCount(), line, nr, nw);
	debugstring(s);
	lock.RLock();
	nw++;
	sprintf(s, "%u ISDNSTACK write lock in line %d passed, nr=%d, nw=%d", GetTickCount(), line, nr, nw);
	debugstring(s);
#else
	lock.RLock();
#endif
}

void ISDNSTACK::WriteUnlockL(int line)
{
#ifdef LOCKDEBUGGING
	char s[100];

	nw--;
	sprintf(s, "%u ISDNSTACK write unlock in line %d, nr=%d, nw=%d", GetTickCount(), line, nr, nw);
	debugstring(s);
#endif
	lock.RUnlock();
}

VPCALL ISDNSTACK::CreateCall()
{
	int i;
	VPCALL vpcall;
	ISDNCALLDATA *vpc;

	WriteLock();
	for(i = 0; i < MAXCALLS; i++)
		if(!calls[i])
		{
			vpc = new ISDNCALLDATA;
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

void ISDNSTACK::FreeCall(VPCALL vpcall)
{
	int i;

	WriteLock();
	for(i = 0; i < MAXCALLS; i++)
	{
		if(calls[i] && calls[i]->vpcall == vpcall)
		{
			calls[i]->Lock();
			delete calls[i];
			calls[i] = 0;
			break;
		}
	}
	WriteUnlock();
}

ISDNCALLDATA *ISDNSTACK::LockCallL(int index, int line)
{
	if((unsigned)index < MAXCALLS)
	{
#ifdef LOCKDEBUGGING
		char s[100];

		sprintf(s, "%u LockCall lock in line %d\n", GetTickCount(), line);
		debugstring(s);
#endif
		ReadLock();
		ISDNCALLDATA *vpc = calls[index];
		if(vpc)
			vpc->Lock();
		ReadUnlock();
		return vpc;
	}
	return 0;
}

ISDNCALLDATA *ISDNSTACK::LockCallL(VPCALL vpcall, int line)
{
	int i;

#ifdef LOCKDEBUGGING
	char s[100];

	sprintf(s, "%u LockCall lock in line %d\n", GetTickCount(), line);
	debugstring(s);
#endif
	ReadLock();
	for(i = 0; i < MAXCALLS; i++)
	{
		if(calls[i] && calls[i]->vpcall == vpcall)
		{
			ISDNCALLDATA *vpc = calls[i];
			if(vpc)
				vpc->Lock();
			ReadUnlock();
			return vpc;
		}
	}
	ReadUnlock();
	return 0;
}

ISDNCALLDATA *ISDNSTACK::FindCall(int line)
{
	int i;

	ReadLock();
	for(i = 0; i < MAXCALLS; i++)
	{
		if(calls[i] && calls[i]->line == line &&
			calls[i]->status >= VPCS_CONNECTING && calls[i]->status <= VPCS_CONNECTED)
		{
			ISDNCALLDATA *vpc = calls[i];
			vpc->Lock();
			ReadUnlock();
			return vpc;
		}
	}
	ReadUnlock();
	return 0;
}

int ISDNSTACK::EnumCalls(VPCALL *vpcalls, unsigned maxvpcalls, unsigned mask)
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

int ISDNSTACK::Init()
{
	int rc = CAPI_Init(ISDNBUFFERLEN, 1);
	
	if(rc)
		return -1;
	return 0;
}

int ISDNSTACK::SetCallLocalName(VPCALL vpcall, const char *username)
{
	ISDNCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	strncpy2(vpc->ourname, username, MAXNAMELEN+1);
	vpc->Unlock();
	return 0;
}

int ISDNSTACK::SetCallLocalNumber(VPCALL vpcall, const char *number)
{
	ISDNCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	strncpy2(vpc->ournumber, number, MAXNUMBERLEN+1);
	vpc->Unlock();
	return 0;
}

int ISDNSTACK::GetCallRemoteName(VPCALL vpcall, char *username)
{
	ISDNCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	if(*vpc->connectedname)
		strncpy2(username, vpc->connectedname, MAXNAMELEN+1);
	else strncpy2(username, vpc->connectednumber, MAXNUMBERLEN+1);
	vpc->Unlock();
	return 0;
}

int ISDNSTACK::GetCallRemoteNumber(VPCALL vpcall, char *number)
{
	ISDNCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	strncpy2(number, vpc->connectednumber, MAXNUMBERLEN+1);
	vpc->Unlock();
	return 0;
}

int ISDNSTACK::GetCallLocalName(VPCALL vpcall, char *username)
{
	ISDNCALLDATA *vpc;

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

int ISDNSTACK::GetCallLocalNumber(VPCALL vpcall, char *number)
{
	ISDNCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	strncpy2(number, vpc->ournumber, MAXNUMBERLEN+1);
	vpc->Unlock();
	return 0;
}

int ISDNSTACK::GetCallCodec(VPCALL vpcall)
{
	return settings.codec;
}

int ISDNSTACK::GetCallStatus(VPCALL vpcall)
{
	int status;

	ISDNCALLDATA *vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	status = vpc->status;
	vpc->Unlock();
	return status;
}

int ISDNSTACK::Connect(VPCALL *vpcall, const char *address, int bc)
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

int ISDNSTACK::Connect(VPCALL vpcall, const char *address, int bc)
{
	ISDNCALLDATA *vpc;
	int rc;

	if(bc != BC_VOICE)
		return VPERR_UNSUPPORTEDBEARER;
	rc = SetCallAddress(vpcall, address);
	if(rc)
		return rc;

	vpc = LockCall(vpcall);
	if(vpc)
	{
		vpc->line = CAPI_FindFreeLine();
		if(vpc->line == -1)
		{
			vpc->Unlock();
			return VPERR_NOMORECALLSAVAILABLE;
		}
		vpc->status = VPCS_CONNECTING;
		CAPI_Dial(vpc->line, settings.dialoutcontroller, CIP_ISDNAUDIO, BPROTOCOL_TRANSPARENT,
			vpc->destnumber, vpc->ournumber, 0, settings.callflags, vpc->ourname);
		vpc->Unlock();
	} else return VPERR_INVALIDCALL;
	return 0;
}

int ISDNSTACK::SetCallAddress(VPCALL vpcall, const char *address)
{
	ISDNCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	strcpy(vpc->destnumber, address);
	strcpy(vpc->connectednumber, vpc->destnumber);
	vpc->Unlock();
	return 0;
}

int ISDNSTACK::Disconnect(VPCALL vpcall, int reason)
{
	ISDNCALLDATA *vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;

	if(vpc->status == VPCS_INCALLPROCEEDING || vpc->status == VPCS_INALERTED)
	{
		CAPI_Accept(vpc->line, 3, 0, vpc->ournumber, settings.callflags & CAPIFLAG_SCREEN);
		vpc->Unlock();
//		NotifyOnCall(VPMSG_CALLENDED, vpcall, reason);
	} else if(vpc->status >= VPCS_CONNECTING && vpc->status <= VPCS_CONNECTED)
	{
		CAPI_Disconnect(vpc->line);
		vpc->Unlock();
//		NotifyOnCall(VPMSG_CALLENDED, vpcall, reason);
	} else vpc->Unlock();
	return 0;
}

int ISDNSTACK::AnswerCall(VPCALL vpcall)
{
	ISDNCALLDATA *vpc = LockCall(vpcall);

	if(!vpc)
		return VPERR_INVALIDCALL;
	if(vpc->status != VPCS_INALERTED)
	{
		vpc->Unlock();
		return VPERR_INVALIDSTATUS;
	}
	CAPI_Accept(vpc->line, 0, BPROTOCOL_TRANSPARENT, vpc->ournumber, settings.callflags & CAPIFLAG_SCREEN);
	vpc->status = VPCS_INCONNECTING;
	vpc->Unlock();
	return 0;
}

void ISDNSTACK::Notify(int message, unsigned param1, unsigned param2)
{
	bool notify = true;

	if(message == VPMSG_CALLESTABLISHED || message == VPMSG_CALLACCEPTED)
	{
		ISDNCALLDATA *vpc = LockCall((VPCALL)param1);
		if(vpc)
		{
			CallEstablished(vpc);
			vpc->Unlock();
		}
	} else if(message == VPMSG_CALLENDED)
	{
		ISDNCALLDATA *vpc = LockCall((VPCALL)param1);
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

int ISDNSTACK::NotifyRc(int message, unsigned param1, unsigned param2)
{
	return 0;
}

int ISDNSTACK::SendKeypad(VPCALL vpcall, int key)
{
	return 0;
}

/**********************************************************************/
/* Other protocol functions                                           */
/**********************************************************************/

int ISDNSTACK::SetNotifyRoutine(void (*notify)(void *param, unsigned, unsigned, unsigned), void *param)
{
	NotifyRoutine = notify;
	NotifyParam = param;
	return 0;
}

extern "C" int CAPI_CallBack(unsigned line, unsigned msg, DWORD param1, DWORD param2)
{
	if(vps)
		return vps->CAPI_CallBack(line, msg, param1, param2);
	else return 0;
}

int ISDNSTACK::CAPI_CallBack(unsigned line, unsigned msg, DWORD param1, DWORD param2)
{
	INCOMINGCALLDATA *icd;
	ISDNCALLDATA *vpc;
	VPCALL vpcall;
	int rc, notifymsg;

	switch(msg)
	{
	case CB_CONNECTERROR:
		vpc = FindCall(line);
		if(vpc)
		{
			vpcall = vpc->vpcall;
			vpc->Unlock();
			NotifyOnCall(VPMSG_CALLSETUPFAILED, vpcall, 0);
			NotifyOnCall(VPMSG_ISDNINFO, vpcall, param1);
			NotifyOnCall(VPMSG_CALLENDED, vpcall, 0);
		}
		break;
	case CB_INCOMINGCALL:
		icd = (INCOMINGCALLDATA *)param1;
		rc = IncomingCall(icd);
		if(rc <= 0)
			return 0;
		vpcall = CreateCall();
		if(!vpcall)
			return 0;	// No more calls available
		icd->colr = settings.callflags & CAPIFLAG_SCREEN;
		icd->reject = 0;
		if(rc == 2)
			icd->alert = 1;
		vpc = LockCall(vpcall);
		vpc->line = line;
		vpc->status = VPCS_INCALLPROCEEDING;
		strcpy(vpc->connectednumber, icd->callingnumber);
		memcpy(vpc->connectedname, icd->userinfo, icd->userinfolen);
		vpc->connectedname[icd->userinfolen] = 0;
		strcpy(vpc->ournumber, icd->callednumber);
		vpc->CIP = icd->CIP;
		vpc->clir = icd->clir;
		vpc->Unlock();
		NotifyOnCall(VPMSG_NEWCALL, vpcall, 0);
		return 1;
	case CB_ALERTCONF:
		vpc = FindCall(line);
		if(vpc)
		{
			vpc->status = VPCS_INALERTED;
			vpc->Unlock();
		}
		break;
	case CB_CONNECTEDEB3:
	case CB_CONNECTED:
		vpc = FindCall(line);
		if(vpc)
		{
			vpcall = vpc->vpcall;
			if(vpc->status >= VPCS_CONNECTING && vpc->status <= VPCS_ALERTED)
			{
				notifymsg = VPMSG_CALLACCEPTED;
				if(msg == CB_CONNECTED)
					notifymsg = VPMSG_CALLACCEPTEDBYUSER;
				vpc->status = VPCS_CONNECTED;
				rc = 1;
			} else if(vpc->status >= VPCS_INCALLPROCEEDING && vpc->status <= VPCS_INCONNECTING)
			{
				notifymsg = VPMSG_CALLESTABLISHED;
				vpc->status = VPCS_CONNECTED;
			} else if(vpc->status == VPCS_CONNECTED && msg == CB_CONNECTED)
			{
				notifymsg = VPMSG_CALLACCEPTEDBYUSER;
				rc = 0;
			}
			vpc->Unlock();
			if(notifymsg == VPMSG_CALLACCEPTEDBYUSER && rc)
				NotifyOnCall(VPMSG_CALLACCEPTED, vpcall, 0);
			if(notifymsg)
				NotifyOnCall(notifymsg, vpcall, 0);
		}
		break;
	case CB_HELD:
		vpc = FindCall(line);
		if(vpc)
		{
			vpc->isdnheld = true;
			vpc->Unlock();
		}
		break;
	case CB_RECONNECTED:
		vpc = FindCall(line);
		if(vpc)
		{
			vpc->isdnheld = false;
			vpc->Unlock();
		}
		break;
	case CB_DISCONNECTED:
		vpc = FindCall(line);
		if(vpc)
		{
			vpcall = vpc->vpcall;
			vpc->Unlock();
			NotifyOnCall(VPMSG_CALLDISCONNECTED, vpcall, 0);
			NotifyOnCall(VPMSG_ISDNINFO, vpcall, param1);
			NotifyOnCall(VPMSG_CALLENDED, vpcall, 0);
		}
		break;
	case CB_CALLPROGRESS:
		break;
	case CB_USERDISCONNECTED:
		CAPI_Disconnect(line);
		break;
	case CB_SSERROR:
		vpc = FindCall(line);
		if(vpc)
		{
			vpcall = vpc->vpcall;
			vpc->Unlock();
			NotifyOnCall(VPMSG_SSERROR, vpcall, MAKELPARAM(param1, param2));
		}
		break;
	case CB_USERUSERINFO:
		vpc = FindCall(line);
		if(vpc)
		{
			memcpy(vpc->displayinfo, (void *)(param1 + 1), param2 - 1);
			vpc->displayinfo[param2] = 0;
			vpcall = vpc->vpcall;
			vpc->Unlock();
			NotifyOnCall(VPMSG_DISPLAYINFO, vpcall, 0);
		}
		break;
	case CB_DISPLAYINFO:
		vpc = FindCall(line);
		if(vpc)
		{
			memcpy(vpc->displayinfo, (void *)param1, param2);
			vpc->displayinfo[param2] = 0;
			vpcall = vpc->vpcall;
			vpc->Unlock();
			NotifyOnCall(VPMSG_DISPLAYINFO, vpcall, 0);
		}
		break;
	case CB_3PTYBEGIN:
		vpc = FindCall(line);
		if(vpc)
		{
			vpc->isdnconferenced = true;
			vpc->Unlock();
		}
		vpc = FindCall(param1);
		if(vpc)
		{
			vpc->isdnconferenced = true;
			vpc->Unlock();
		}
		break;
	case CB_3PTYEND:
		vpc = FindCall(line);
		if(vpc)
		{
			vpc->isdnconferenced = false;
			vpc->Unlock();
		}
		vpc = FindCall(param1);
		if(vpc)
		{
			vpc->isdnconferenced = false;
			vpc->Unlock();
		}
		break;
	case CB_DATA:
		if(ISDNAudio(line, (BYTE *)param1, param2))
			CAPI_SendData(line, (BYTE *)param1, param2);
		break;
	}
	return 0;
}

/**********************************************************************/
/* Audio                                                              */
/**********************************************************************/

static unsigned char bitrev[256] = {
0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff};

static void memcpyinvbits(unsigned char *dest, const unsigned char *src, int len)
{
	int i;

	for(i = 0; i < len; i++)
		dest[i] = bitrev[src[i]];
}

int ISDNSTACK::ISDNAudio(int line, BYTE *data, int datalen)
{
	ISDNCALLDATA *vpc;
	VPAUDIODATA *audio;
	VPCALL vpcall;
	BYTE outbuf[ISDNBUFFERLEN];
	int i;
	unsigned ts;

	vpc = FindCall(line);
	if(vpc)
	{
		if(!vpc->audio)
		{
			vpc->Unlock();
			return 0;
		}
		for(i = 0; i < datalen; i++)
			outbuf[i] = bitrev[data[i]];
		audio = vpc->audio;
		ts = vpc->rxtimestamp;
		vpc->rxtimestamp += datalen / 8;
		if(vpc->audiobuf.GetData(data, datalen))
			memset(data, settings.codec == CODEC_G711A ? 0xab : 0x7f, datalen);
		vpcall = vpc->vpcall;
		vpc->audiolocked = true;
		vpc->Unlock();
		audio->AudioFrame(settings.codec, ts, outbuf, datalen);
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
		return 1;
	}
	return 0;
}

int	ISDNSTACK::SendAudio(VPCALL vpcall, int codec, unsigned timestamp, const void *buf, int len, bool wait)
{
	ISDNCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return -1;
	vpc->audiobuf.PutData((unsigned char *)buf, len);
	vpc->Unlock();
	return 0;
}

void ISDNSTACK::CallEstablished(ISDNCALLDATA *vpc)
{
	ncalls++;
	vpc->start_ticks = GetTickCount();
	time(&vpc->start_time);
	if(vpaudiodatafactory)
		vpc->audio = vpaudiodatafactory->New(this, vpc->vpcall, settings.codec);
}

void ISDNSTACK::CallEnded(ISDNCALLDATA *vpc)
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
			memcpyinvbits(data, buf + head, len);
		else {
			memcpyinvbits(data, buf + head, AUDIOBUFSIZE - head);
			memcpyinvbits(data + AUDIOBUFSIZE - head, buf, len - (AUDIOBUFSIZE - head));
		}
		head = (head + len) % AUDIOBUFSIZE;
		return 0;
	}
	return -1;
}

/**********************************************************************/
/* Supplementary services                                             */
/**********************************************************************/

int ISDNSTACK::IsHeld(VPCALL vpcall)
{
	ISDNCALLDATA *vpc;
	int rc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	rc = vpc->isdnheld;
	vpc->Unlock();
	return rc;
}

int ISDNSTACK::Hold(VPCALL vpcall)
{
	ISDNCALLDATA *vpc;
	int rc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	if(vpc->isdnconferenced)
		CAPI_3PtyEnd(vpc->line);
	else if(!vpc->held)
	{
		vpc->held = true;
		rc = CAPI_Hold(vpc->line);
	}
	vpc->Unlock();
	if(rc)
		return VPERR_INVALIDSTATUS;
	return 0;
}

int ISDNSTACK::Resume(VPCALL vpcall)
{
	ISDNCALLDATA *vpc;
	int rc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	if(vpc->held)
	{
		vpc->held = false;
		rc = CAPI_Retrieve(vpc->line);
	}
	vpc->Unlock();
	if(rc)
		return VPERR_INVALIDSTATUS;
	return 0;
}

int ISDNSTACK::ConferenceCalls(VPCALL *vpcalls, int ncalls, bool detach)
{
	ISDNCALLDATA *vpc;
	int lines[2], rc;
	bool isdnheld[2];

	if(ncalls != 2)
		return VPERR_UNSUPPORTEDCALL;
	vpc = LockCall(vpcalls[0]);
	if(!vpc)
		return VPERR_INVALIDCALL;
	lines[0] = vpc->line;
	isdnheld[0] = vpc->isdnheld;
	vpc->Unlock();
	vpc = LockCall(vpcalls[1]);
	if(!vpc)
		return VPERR_INVALIDCALL;
	lines[1] = vpc->line;
	isdnheld[1] = vpc->isdnheld;
	vpc->Unlock();
	if(isdnheld[0] && isdnheld[1] || !isdnheld[0] && !isdnheld[1])
		return VPERR_INVALIDSTATUS;
	if(isdnheld[1])
	{
		if(detach)
			rc = CAPI_ECT(lines[0], lines[1]);
		else rc = CAPI_3PtyBegin(lines[0], lines[1]);
	} else {
		if(detach)
			rc = CAPI_ECT(lines[1], lines[0]);
		else rc = CAPI_3PtyBegin(lines[1], lines[0]);
	}
	if(rc)
		return VPERR_INVALIDSTATUS;
	return 0;
}

int ISDNSTACK::GetCallTimes(VPCALL vpcall, time_t *start_time, unsigned int *length_ms)
{
	ISDNCALLDATA *vpc = LockCall(vpcall);

	if(!vpc)
		return VPERR_INVALIDCALL;
	*start_time = vpc->start_time;
	*length_ms = GetTickCount() - vpc->start_ticks;
	vpc->Unlock();
	return 0;
}

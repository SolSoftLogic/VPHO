#include <windows.h>
#include <stdio.h>
#include <time.h>
#include "vpstack.h"
#include "gsmstack.h"

#define Lock() LockL(__LINE__)
#define Unlock() UnlockL(__LINE__)
#define ReadLock() ReadLockL(__LINE__)
#define ReadUnlock() ReadUnlockL(__LINE__)
#define WriteLock() WriteLockL(__LINE__)
#define WriteUnlock() WriteUnlockL(__LINE__)
#define LockCall(a) LockCallL(a, __LINE__)
#define TSTART	InterlockedIncrement(&nthreads);
#define TRETURN	{InterlockedDecrement(&nthreads); return; }

IVPSTACK *CreateGSMSTACK(VPAUDIODATAFACTORY *vpaf, const char *stackname)
{
	return new GSMSTACK(vpaf, stackname);
}

GSMSTACK::GSMSTACK(VPAUDIODATAFACTORY *vpaf, const char *stackname)
{
	vpaudiodatafactory = vpaf;
	nthreads = 0;
	nw = nr = 0;
	lastvpcallvalue = 0;
	Zero(calls);
	Zero(settings);
	settings.codec = CODEC_G711A;
}

GSMSTACK::~GSMSTACK()
{
}

/**********************************************************************/
/* Locking                                                            */
/**********************************************************************/

void GSMCALLDATA::LockL(int line)
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

void GSMCALLDATA::UnlockL(int line)
{
#ifdef LOCKDEBUGGING
	char s[100];

	sprintf(s, "%u GSMCALL unlock in line %d\n", GetTickCount(), line);
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
	lock.RLock();
	nw++;
	sprintf(s, "%u GSMSTACK write lock in line %d passed, nr=%d, nw=%d", GetTickCount(), line, nr, nw);
	debugstring(s);
#else
	lock.RLock();
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
	lock.RUnlock();
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

GSMCALLDATA *GSMSTACK::LockCallL(int index, int line)
{
	if((unsigned)index < MAXCALLS)
	{
#ifdef LOCKDEBUGGING
		char s[100];

		sprintf(s, "%u LockCall lock in line %d\n", GetTickCount(), line);
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

GSMCALLDATA *GSMSTACK::LockCallL(VPCALL vpcall, int line)
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

int GSMSTACK::Connect(VPCALL *vpcall, const char *address, int bc)
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

int GSMSTACK::Connect(VPCALL vpcall, const char *address, int bc)
{
	GSMCALLDATA *vpc;
	int rc;

	if(bc != BC_VOICE)
		return VPERR_UNSUPPORTEDBEARER;
	rc = SetCallAddress(vpcall, address);
	if(rc)
		return rc;

	vpc = LockCall(vpcall);
	if(vpc)
	{
		vpc->status = VPCS_CONNECTING;
		//
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
	GSMCALLDATA *vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;

	if(vpc->status == VPCS_INCALLPROCEEDING || vpc->status == VPCS_INALERTED)
	{
		//
		vpc->Unlock();
	} else if(vpc->status >= VPCS_CONNECTING && vpc->status <= VPCS_CONNECTED)
	{
		//
		vpc->Unlock();
	} else vpc->Unlock();
	return 0;
}

int GSMSTACK::AnswerCall(VPCALL vpcall)
{
	GSMCALLDATA *vpc = LockCall(vpcall);

	if(!vpc)
		return VPERR_INVALIDCALL;
	if(vpc->status != VPCS_INALERTED)
	{
		vpc->Unlock();
		return VPERR_INVALIDSTATUS;
	}
	//
	vpc->status = VPCS_INCONNECTING;
	vpc->Unlock();
	return 0;
}

void GSMSTACK::Notify(int message, unsigned param1, unsigned param2)
{
	bool notify = true;

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

int GSMSTACK::SetNotifyRoutine(void (*notify)(void *param, unsigned, unsigned, unsigned), void *param)
{
	NotifyRoutine = notify;
	NotifyParam = param;
	return 0;
}

int	GSMSTACK::SendAudio(VPCALL vpcall, int codec, unsigned timestamp, const void *buf, int len, bool wait)
{
	GSMCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return -1;
	//
	vpc->Unlock();
	return 0;
}

void GSMSTACK::CallEstablished(GSMCALLDATA *vpc)
{
	ncalls++;
	vpc->start_ticks = GetTickCount();
	time(&vpc->start_time);
	if(vpaudiodatafactory)
		vpc->audio = vpaudiodatafactory->New(this, vpc->vpcall, settings.codec);
}

void GSMSTACK::CallEnded(GSMCALLDATA *vpc)
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

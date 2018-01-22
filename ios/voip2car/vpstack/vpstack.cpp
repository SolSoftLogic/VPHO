#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "vprotocol.h"
#include "aes.h"
#include "util.h"
#include "vpstack.h"
#include "../codecs/videocodecs.h"
#include "vpstackpriv.h"
#include "rtfdictionary.h"
#include "avifile.h"

static bool forceuseproxy = false; //true;

static struct {
	char job[5][100];
	unsigned id;
	unsigned tm;
} threads[100];

void DebugCurrentOp(const char *label)
{
	unsigned threadid = GetCurrentThreadId();
	int i;

	for(i = 0; i < 100; i++)
		if(threads[i].id == threadid)
		{
			memmove(threads[i].job[1], threads[i].job[0], 400);
			strcpy(threads[i].job[0], label);
			threads[i].tm = GetTickCount();
			break;
		}
	if(i == 100)
	{
		for(i = 0; i < 100; i++)
			if(!threads[i].id)
			{
				threads[i].id = threadid;
				memmove(threads[i].job[1], threads[i].job[0], 400);
				strcpy(threads[i].job[0], label);
				threads[i].tm = GetTickCount();
				break;
			}
	}
	Lprintf("%x %s", threadid, label);
	//fprintf(stderr, "tid=%d, ", threadid);
	//fprintf(stderr, "%s\n", label);
	//fflush(stderr);
}

void DumpJobs()
{
	int i, j;

	for(i = 0; i < 100; i++)
		if(threads[i].id)
		{
			for(j = 0; j < 5; j++)
				Lprintf("Thread %x, tm=%u, job[%d]=%s", threads[i].id, threads[i].tm, j, threads[i].job[j]);
		}
}

//#define LOCKDEBUGGING

/**********************************************************************/
/* Locking                                                            */
/**********************************************************************/

void VPCALLDATA::LockL(int line)
{
#ifdef LOCKDEBUGGING
	char s[100];

	sprintf(s, "%u VPCALL %x lock in line %d", GetTickCount(), this, line);
	DebugCurrentOp(s);
	EnterCriticalSection(&cs);
	sprintf(s, "%u VPCALL %x lock in line %d passed", GetTickCount(), this, line);
	DebugCurrentOp(s);
#else
	EnterCriticalSection(&cs);
#endif
}

void VPCALLDATA::UnlockL(int line)
{
#ifdef LOCKDEBUGGING
	char s[100];

	sprintf(s, "%u VPCALL %x unlock in line %d", GetTickCount(), this, line);
	DebugCurrentOp(s);
#endif
	LeaveCriticalSection(&cs);
}

/**********************************************************************/
/* Utility functions                                                  */
/**********************************************************************/

static long holdrand;

int rand2()
{
	return (((holdrand = holdrand * 214013L + 2531011L) >> 16) & 0x7fff);
}

static unsigned GenerateRandomDWord()
{
	static bool initialized;
	unsigned ret;

	if(!initialized)
	{
		holdrand = GetTickCount();
		initialized = true;
	}
	do {
		ret = rand2();
		ret |= (rand2() << 15);
		ret |= rand2() << 30;
		ret ^= GetTickCount();
	} while(!ret);
	return ret;
}

static void AddIE_Reply(OUTMSG *outmsg, IE_ELEMENTS *ie)
{
	outmsg->AddIE_Word(IE_CONNECTIONID, ie->connectionid);
	if(ie->tokenpresent)
		outmsg->AddIE_DWord(IE_TOKEN, ie->token);
	if(ie->seqnumberpresent)
		outmsg->AddIE_DWord(IE_SEQNUMBER, ie->seqnumber);
	if(ie->transparentdata)
		outmsg->AddIE_Binary(IE_TRANSPARENTDATA, (unsigned char *)ie->transparentdata, ie->transparentdatalen);
}

/**********************************************************************/
/* Calls management functions                                         */
/**********************************************************************/

IVPSTACK *CreateVPSTACK(VPAUDIODATAFACTORY *vpaf, const char *stackname, VPVIDEODATAFACTORY *vpvf)
{
	return new VPSTACK(vpaf, stackname, vpvf);
}

VPSTACK::VPSTACK(VPAUDIODATAFACTORY *vpaf, const char *stackname, VPVIDEODATAFACTORY *vpvf)
{
	vpaudiodatafactory = vpaf;
	vpvideodatafactory = vpvf;
	nw = nr = 0;
	InitializeCriticalSection(&ctokencs);
	InitializeCriticalSection(&resolvercs);
	InitializeCriticalSection(&adlcs);
	adllen = 0;
	adltm = GetTickCount();
	Zero(settings);
	Zero(vpcalls);
#ifdef EVCSOFT
	strcpy(settings.serverslist, "main.evcsoft.net;backup.evcsoft.net");
#else
	strcpy(settings.serverslist, "271.epserver.net;383.epserver.net");
#endif
	settings.autolocalhostaddr = true;
	settings.fourcc = F_VP31;
#ifdef INCLUDEXVID
	settings.fourcc = F_XVID;
#endif
#ifdef INCLUDEAVC
	settings.fourcc = F_H264;
#endif
	settings.txvideoparity = true;
	settings.width = 176;
	settings.height = 144;
#ifdef TARGET_OS_IPHONE                                                                                                     
	settings.width = 192;                                                                                                
	settings.notifyinterval = 600000;
#endif 
	settings.framerate = 10;
	settings.quality = 0x35;
	settings.port = GetTickCount() % 200000 / 10 + 10000;
	Zero(rxxc);
	lastserverupdatetm = lastawaketm = 0;
	notifyackawaitedtm = notifyackfailed = 0;
	allowlogon = loggedon = false;
	lastversion = 0;
	lastvpcallvalue = 0;
	smsseqnumber = 1;
	lastseqnumberscleantm = GetTickCount();
	nseqnumbers = 0;
	stackversion = STACKVERSION;
	seqnumber = GenerateRandomDWord();
	NotifyRoutine = 0;
	SyncNotifyRoutine = 0;
	cfinprogress = false;
	
	Zero(serveraddr);
	Zero(localhostpublicaddr);
	Zero(resolvers);
	lastnotifylogonseqnumber = lastnotifylogonaddr = 0;
	nencodednumbers = 0;
	queryonlineack = false;
	lastamactive = -1;
	Zero(aoldata);
	nthreads = 0;
	if(stackname)
		strncpy2(this->stackname, stackname, sizeof(this->stackname));
	else *this->stackname = 0;
	memset(&dvm, 0, sizeof(dvm));
	dvm.sk = INVALID_SOCKET;
	recordvpcall = 0;
	memset(&nat, 0, sizeof(nat));
	strcpy(natserverslist, "271.epserver.net;383.epserver.net");
	measuringbandwidth = false;
	apntoken = 0;
	Lprintf("VP stack created");
}

VPSTACK::~VPSTACK()
{
	int i;
	VPCALL vpcall;

	tx.Stop();
	Logoff();
	ReadLock();
	for(i = 0; i < MAXCALLS; i++)
		if(vpcalls[i])
		{
			vpcall = vpcalls[i]->vpcall;
			ReadUnlock();
			Disconnect(vpcall, 0);
			ReadLock();
		}
	ReadUnlock();
	SOCKET sk = sock;
	sock = 0;
	closesocket(sk);
	if(dvm.sk != INVALID_SOCKET)
	{
		SOCKET sk = dvm.sk;

		dvm.sk = INVALID_SOCKET;
		closesocket(sk);
	}
	aoldata.stop = true;
	while(nthreads)
		Sleep(10);
	ReadLock();
	for(i = 0; i < MAXCALLS; i++)
		if(vpcalls[i])
		{
			vpcall = vpcalls[i]->vpcall;
			ReadUnlock();
			FreeCall(vpcall);
			ReadLock();
		}
	ReadUnlock();
	DeleteCriticalSection(&ctokencs);
	DeleteCriticalSection(&resolvercs);
	DeleteCriticalSection(&adlcs);
	Lprintf("VP stack destroyed");
}

void VPSTACK::ReadLockL(int line)
{
#ifdef LOCKDEBUGGING
	char s[100];

	sprintf(s, "%u VPSTACK read lock in line %d, nr=%d, nw=%d", GetTickCount(), line, nr, nw);
	DebugCurrentOp(s);
	lock.RLock();
	nr++;
	sprintf(s, "%u VPSTACK read lock in line %d passed, nr=%d, nw=%d", GetTickCount(), line, nr, nw);
	DebugCurrentOp(s);
#else
	lock.RLock();
#endif
}

void VPSTACK::ReadUnlockL(int line)
{
#ifdef LOCKDEBUGGING
	char s[100];

	nr--;
	sprintf(s, "%u VPSTACK read unlock in line %d, nr=%d, nw=%d", GetTickCount(), line, nr, nw);
	DebugCurrentOp(s);
#endif
	lock.RUnlock();
}

void VPSTACK::WriteLockL(int line)
{
#ifdef LOCKDEBUGGING
	char s[100];

	sprintf(s, "%u VPSTACK write lock in line %d, nr=%d, nw=%d", GetTickCount(), line, nr, nw);
	DebugCurrentOp(s);
	lock.WLock();
	nw++;
	sprintf(s, "%u VPSTACK write lock in line %d passed, nr=%d, nw=%d", GetTickCount(), line, nr, nw);
	DebugCurrentOp(s);
#else
	lock.WLock();
#endif
}

void VPSTACK::WriteUnlockL(int line)
{
#ifdef LOCKDEBUGGING
	char s[100];

	nw--;
	sprintf(s, "%u VPSTACK write unlock in line %d, nr=%d, nw=%d", GetTickCount(), line, nr, nw);
	DebugCurrentOp(s);
#endif
	lock.WUnlock();
}

VPCALL VPSTACK::CreateCall()
{
	int i;
	VPCALL vpcall;

	WriteLock();
	for(i = 0; i < MAXCALLS; i++)
		if(!vpcalls[i])
		{
			vpcalls[i] = new VPCALLDATA;
			vpcalls[i]->missedcallbc = 0xff;
			vpcalls[i]->vpcall = vpcall = (VPCALL)++lastvpcallvalue;
			vpcalls[i]->token = GenerateRandomDWord() & 0xffff0000 | (unsigned short)(int)vpcall;
			vpcalls[i]->smsid = smsseqnumber++;
			vpcalls[i]->aa.f = -1;
			vpcalls[i]->framesperpacket = 2;	// Without bandwidth management
			vpcalls[i]->mtu = DEFAULT_MTU;
			vpcalls[i]->videofmt.fourcc = settings.fourcc;
			vpcalls[i]->videofmt.width = settings.width;
			vpcalls[i]->videofmt.height = settings.height;
			vpcalls[i]->videofmt.quality = settings.quality;
			vpcalls[i]->videofmt.framerate = settings.framerate;
			vpcalls[i]->start_ticks = GetTickCount();
			time(&vpcalls[i]->start_time);
			strcpy(vpcalls[i]->ourname, settings.username);
			strcpy(vpcalls[i]->ournumber, settings.usernum);
			vpcalls[i]->start_ticks = GetTickCount();
			time(&vpcalls[i]->start_time);
			WriteUnlock();
			return vpcall;
		}
	WriteUnlock();
	return 0;
}

void VPSTACK::FreeCall(VPCALL vpcall)
{
	int i;
	VPCALL discondisccall = 0;

retry:
	WriteLock();
	for(i = 0; i < MAXCALLS; i++)
	{
		if(vpcalls[i] && vpcalls[i]->vpcall == vpcall)
		{
			vpcalls[i]->Lock();
			if(vpcalls[i]->inuse)
			{
				vpcalls[i]->deleting = true;
				vpcalls[i]->Unlock();
				WriteUnlock();
				Sleep(10);
				goto retry;
			}
			if(vpcalls[i]->mconf.conferencedata)
				delete vpcalls[i]->mconf.conferencedata;
			if(vpcalls[i]->mconf.discondisccall)
				discondisccall = vpcalls[i]->mconf.discondisccall;
			if(vpcalls[i]->ft.udpstream)
				delete vpcalls[i]->ft.udpstream;
			if(vpcalls[i]->ft.f)
				closefile(vpcalls[i]->ft.f);
			if(vpcalls[i]->audio)
				delete vpcalls[i]->audio;
			if(vpcalls[i]->video)
				delete vpcalls[i]->video;
			if(vpcalls[i]->videopkt.buf)
				free(vpcalls[i]->videopkt.buf);
			if(vpcalls[i]->videopkt.parity)
				free(vpcalls[i]->videopkt.parity);
			if(vpcalls[i]->havi)
				AVIClose(vpcalls[i]->havi);
			if(vpcalls[i]->hwav)
				WAVClose(vpcalls[i]->hwav);
			delete vpcalls[i];
			vpcalls[i] = 0;
			break;
		}
	}
	WriteUnlock();
	if(discondisccall)
		Disconnect(discondisccall, REASON_NORMAL);
}

VPCALLDATA *VPSTACK::LockCallL(int index, int line)
{
	if((unsigned)index < MAXCALLS && vpcalls[index])
	{
#ifdef LOCKDEBUGGING
		char s[100];

		sprintf(s, "%u LockCall lock in line %d, index = %d", GetTickCount(), line, index);
		DebugCurrentOp(s);
#endif
		ReadLock();
		VPCALLDATA *vpc = vpcalls[index];
		if(vpc)
		{
			if(vpcalls[index]->deleting)
				vpc = 0;
			else vpc->Lock();
		}
		ReadUnlock();
		return vpc;
	}
	return 0;
}

VPCALLDATA *VPSTACK::LockCallL(VPCALL vpcall, int line)
{
	int i;

#ifdef LOCKDEBUGGING
	char s[100];

	sprintf(s, "%u LockCall lock in line %d, vpcall = %d", GetTickCount(), line, vpcall);
	DebugCurrentOp(s);
#endif
	ReadLock();
	for(i = 0; i < MAXCALLS; i++)
	{
		if(vpcalls[i] && vpcalls[i]->vpcall == vpcall)
		{
			VPCALLDATA *vpc = vpcalls[i];
			if(vpc)
			{
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

VPCALLDATA *VPSTACK::LockInUseCall(VPCALL vpcall)
{
	int i;

	ReadLock();
	for(i = 0; i < MAXCALLS; i++)
	{
		if(vpcalls[i] && vpcalls[i]->vpcall == vpcall)
		{
			VPCALLDATA *vpc = vpcalls[i];
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

VPCALLDATA *VPSTACK::FindCall(const struct sockaddr_in *addr, int connectionid, int bc)
{
	int i;

	ReadLock();
	for(i = 0; i < MAXCALLS; i++)
	{
		if(vpcalls[i] && vpcalls[i]->addr.sin_addr.s_addr == addr->sin_addr.s_addr &&
			(vpcalls[i]->addr.sin_port == addr->sin_port || vpcalls[i]->altport == addr->sin_port) &&
			(vpcalls[i]->connectionid == connectionid || !connectionid) && (vpcalls[i]->bc == bc || bc == 0xff) &&
			(vpcalls[i]->status >= VPCS_CONNECTING && vpcalls[i]->status <= VPCS_CONNECTED))
		{
			VPCALLDATA *vpc = vpcalls[i];
			vpc->Lock();
			ReadUnlock();
			return vpc;
		}
	}
	ReadUnlock();
	return 0;
}

VPCALLDATA *VPSTACK::FindCallByToken(const struct sockaddr_in *addr, unsigned token)
{
	int i;

	ReadLock();
	for(i = 0; i < MAXCALLS; i++)
	{
		if(vpcalls[i] && vpcalls[i]->addr.sin_addr.s_addr == addr->sin_addr.s_addr &&
			(vpcalls[i]->addr.sin_port == addr->sin_port || vpcalls[i]->altport == addr->sin_port) &&
			vpcalls[i]->token == token &&
			(vpcalls[i]->status >= VPCS_CONNECTING && vpcalls[i]->status <= VPCS_CONNECTED))
		{
			VPCALLDATA *vpc = vpcalls[i];
			vpc->Lock();
			ReadUnlock();
			return vpc;
		}
	}
	ReadUnlock();
	return 0;
}

VPCALLDATA *VPSTACK::FindCallByConfToken(const struct sockaddr_in *addr1, const struct sockaddr_in *addr2, unsigned token)
{
	int i;

	ReadLock();
	for(i = 0; i < MAXCALLS; i++)
	{
		if(vpcalls[i] && vpcalls[i]->sconf.token == token &&
			(vpcalls[i]->publicaddr.sin_addr.s_addr == addr1->sin_addr.s_addr && vpcalls[i]->publicaddr.sin_port == addr1->sin_port ||
			vpcalls[i]->publicaddr.sin_addr.s_addr == addr2->sin_addr.s_addr && vpcalls[i]->publicaddr.sin_port == addr2->sin_port) &&
			(vpcalls[i]->status >= VPCS_CONNECTING && vpcalls[i]->status <= VPCS_CONNECTED))
		{
			VPCALLDATA *vpc = vpcalls[i];
			vpc->Lock();
			ReadUnlock();
			return vpc;
		}
	}
	ReadUnlock();
	return 0;
}

VPCALLDATA *VPSTACK::FindMasterCall(const struct sockaddr_in *addr, unsigned token)
{
	int i;

	ReadLock();
	for(i = 0; i < MAXCALLS; i++)
	{
		if(vpcalls[i] &&
			// Address is not verified, because the other side can see our master with a different address
			// if we are e.g. in the same network of master
			// (!addr->sin_addr.s_addr || vpcalls[i]->addr.sin_addr.s_addr == addr->sin_addr.s_addr &&
			// (vpcalls[i]->addr.sin_port == addr->sin_port || vpcalls[i]->altport == addr->sin_port)) &&
			vpcalls[i]->sconf.status == CONFSLAVESTATUS_INITIATED &&
			vpcalls[i]->sconf.token == token && vpcalls[i]->status == VPCS_CONNECTED)
		{
			VPCALLDATA *vpc = vpcalls[i];
			vpc->Lock();
			ReadUnlock();
			return vpc;
		}
	}
	ReadUnlock();
	return 0;
}

int VPSTACK::FindConferencedCalls(unsigned token, int bc, VPCALL *calls)
{
	int i, n = 0;

	ReadLock();
	for(i = 0; i < MAXCALLS && n < MAXVPARTIES; i++)
		if(vpcalls[i] && 
			vpcalls[i]->mconf.status != CONFSTATUS_IDLE &&
			vpcalls[i]->mconf.token == token && vpcalls[i]->bc == bc)
			calls[n++] = vpcalls[i]->vpcall;
	ReadUnlock();
	return n;
}

int VPSTACK::FindConferenceCalls(const struct sockaddr_in *addr, unsigned token, int bc, VPCALL *calls)
{
	int i, n = 0;

	ReadLock();
	for(i = 0; i < MAXCALLS && n < MAXVPARTIES; i++)
	{
		if(addr)	// Call conneted to the master
		{
			if(vpcalls[i] && vpcalls[i]->sconf.masteraddr.sin_addr.s_addr == addr->sin_addr.s_addr &&
				vpcalls[i]->sconf.masteraddr.sin_port == addr->sin_port &&
				vpcalls[i]->sconf.status &&
				vpcalls[i]->sconf.token == token && vpcalls[i]->status == VPCS_CONNECTED && vpcalls[i]->bc == bc)
				calls[n++] = vpcalls[i]->vpcall;
		} else {	// Calls connected to the slaves
			if(vpcalls[i] && 
				(vpcalls[i]->sconf.status == CONFSLAVESTATUS_CONNECTING ||
				vpcalls[i]->sconf.status == CONFSLAVESTATUS_CONNECTED) &&
				vpcalls[i]->sconf.token == token && vpcalls[i]->bc == bc)
				calls[n++] = vpcalls[i]->vpcall;
		}
	}
	ReadUnlock();
	return n;
}

int VPSTACK::EnumCalls(VPCALL *vpcalls, unsigned maxvpcalls, unsigned mask)
{
	unsigned n = 0, i;

	ReadLock();
	for(i = 0; i < MAXCALLS; i++)
	{
		if(this->vpcalls[i] && (mask & (1 << this->vpcalls[i]->bc)))
		{
			if(vpcalls && n < maxvpcalls)
				vpcalls[n] = this->vpcalls[i]->vpcall;
			if(!vpcalls || n < maxvpcalls)
				n++;
		}
	}
	ReadUnlock();
	return n;
}

void VPSTACK::CleanConferenceCalls(const struct sockaddr_in *addr, unsigned token)
{
	int i;

	WriteLock();
	for(i = 0; i < MAXCALLS; i++)
	{
		if(vpcalls[i] && (!addr->sin_addr.s_addr || vpcalls[i]->sconf.masteraddr.sin_addr.s_addr == addr->sin_addr.s_addr &&
			vpcalls[i]->sconf.masteraddr.sin_port == addr->sin_port) &&
			vpcalls[i]->sconf.status == CONFSLAVESTATUS_CONNECTING &&
			vpcalls[i]->sconf.token == token && vpcalls[i]->status == VPCS_CONNECTING)
		{
			vpcalls[i]->Lock();
			delete vpcalls[i];
			vpcalls[i] = 0;
		}
	}
	WriteUnlock();
}

void VPSTACK::PrintCalls()
{
	int i;
	static const char *statuses[] = {"IDLE", "CONNECTING", "CALLPROCEEDING", "ALERTED", "INCALLPROCEEDING", "INALERTED",
		"INCONNECTING", "HELD", "CONNECTED", "DISCONNECTING", "TERMINATED"};
	static const char *confstatuses[] = {"IDLE", "ALERTNECESSARY", "USERALERTED", "USERCONFIRMED", "INITIATED",
		"CONNECTING", "CONNECTED"};

	ReadLock();
	for(i = 0; i < MAXCALLS; i++)
		if(vpcalls[i])
		{
			Lprintf("%d: %d %s:%d:%d (%s,%s) [%s] [%s %x]", i, vpcalls[i]->vpcall, inet_ntoa(vpcalls[i]->addr.sin_addr), ntohs(vpcalls[i]->addr.sin_port), vpcalls[i]->connectionid,
				vpcalls[i]->connectedname, vpcalls[i]->connectednumber, statuses[vpcalls[i]->status], confstatuses[vpcalls[i]->sconf.status], vpcalls[i]->sconf.token);
		}
	ReadUnlock();
}

VPCALLDATA *VPSTACK::FindCallOrDisconnect(const struct sockaddr_in *addr, int connectionid, int bc)
{
	VPCALLDATA *vpc = FindCall(addr, connectionid, bc == BC_VIDEO ? BC_AUDIOVIDEO : bc);

	if(vpc)
		return vpc;
	OUTMSG outmsg(DEFAULT_MTU, PKT_DISCONNECTREQ);
	if(bc != 0xff)
		outmsg.AddIE_Byte(IE_BEARER, bc);
	outmsg.AddIE_Word(IE_CONNECTIONID, connectionid);
	Send(addr, &outmsg);
	return 0;;
}

static int ushortcmp(const void *a, const void *b)
{
	return *(unsigned short *)a - *(unsigned short *)b;
}

int VPSTACK::AssignConnectionId(VPCALL vpcall, const struct sockaddr_in *addr)
{
	int i, connectionid = 1, ncids = 0;
	unsigned short *cids;

	cids = (unsigned short *)malloc(2 * MAXCALLS);
	ReadLock();
	// Find unique connection id
	for(i = 0; i < MAXCALLS; i++)
	{
		if(vpcalls[i] && vpcalls[i]->vpcall != vpcall &&
			vpcalls[i]->addr.sin_addr.s_addr == addr->sin_addr.s_addr &&
			vpcalls[i]->addr.sin_port == addr->sin_port)	// TODO: This line should be taken away, when legacy vPhone will be dismissed
			cids[ncids++] = vpcalls[i]->connectionid;
	}
	if(ncids)
	{
		qsort(cids, ncids, 2, ushortcmp);
		for(i = 0; i < ncids - 1 && cids[i] + 1 == cids[i + 1]; i++);
		connectionid = cids[i] + 1;
	}
	free(cids);
	// Assign it
	for(i = 0; i < MAXCALLS; i++)
	{
		if(vpcalls[i] && vpcalls[i]->vpcall == vpcall)
		{
			vpcalls[i]->connectionid = connectionid;
			break;
		}
	}
	if(i == MAXCALLS)
	{
		ReadUnlock();
		return VPERR_INVALIDCALL;
	}
	ReadUnlock();
	return 0;
}

/**********************************************************************/
/* Public functions: general                                          */
/**********************************************************************/

int VPSTACK::Init()
{
	struct sockaddr_in addr;
	int rcvlen, tos = 0x98, tmpport;

	instancetoken = GenerateRandomDWord();
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
	for(tmpport = settings.port; tmpport < settings.port + 50; tmpport++)
	{
		addr.sin_port = htons(tmpport);
		if(!bind(sock, (sockaddr *)&addr, sizeof(addr)))
			break;
	}
	if(tmpport == settings.port + 50)
	{
		closesocket(sock);
		sock = 0;
		return -1;
	}
	settings.port = tmpport;
	tx.sk = sock;
	tx.Run();
	TSTART
	beginclassthread((ClassThreadStart)&VPSTACK::TransmitThread, this, 0);
	TSTART
	beginclassthread((ClassThreadStart)&VPSTACK::ReceiveThread, this, 0);
	settings.supportedbearers = 0;
	if(vpaudiodatafactory)
		settings.supportedbearers |= (1<<BC_VOICE);
	settings.supportedcodecs = 0xd01;	// MSGSM, GSM and G.711
	settings.preferredcodec = CODEC_MSGSM;
#ifdef INCLUDEILBC
	settings.supportedcodecs |= 0x3000;	// Add iLBC codecs
#endif
#ifdef INCLUDESPEEX
#ifndef _WIN32_WCE	// ARM is not powerful enough for speex encoding, but leave it for video messages
	settings.supportedcodecs |= 0xf8;	// Add speex codecs
	settings.preferredcodec = CODEC_SPEEXB;
#endif
#endif
	Lprintf("VP stack initialized");
	return 0;	
}

int VPSTACK::SetServers(const char *list)
{
	strncpy(settings.serverslist, list, MAXMESSAGELEN);
	return 0;
}

void VPSTACK::UsernameLogon()
{
	char *p, *q, s[300];
	struct sockaddr_in addr;

	OUTMSG outmsg(500, PKT_ACTIVATEACCOUNT);
	outmsg.AddIE_String(IE_SOURCENAME, settings.username);
	if(*settings.serveraccesscode)
		outmsg.AddIE_String(IE_SERVERACCESSCODE, settings.serveraccesscode);

	p = settings.serverslist;
	while(*p)
	{
		q = strchr(p, ';');
		if(!q)
			q = p + strlen(p);
		memcpy(s, p, q - p);
		s[q - p] = 0;
		if(Resolve(s, &addr, VPHONEPORT))
		{
			Send(&addr, &outmsg);
			Sleep(30);
		}
		p = *q == ';' ? q + 1 : q;
	}
}

int VPSTACK::SetPublicAddr(const char *addr)
{
	localhostaddr = FindLocalHostAddr(settings.localhostsaddr, settings.autolocalhostaddr);
	Resolve(addr, &localhostpublicaddr, settings.port);
	return 0;
}

int VPSTACK::Logon(const char *username, const char *password)
{
	if(!*username && !*password)
		return Resolve(settings.serverslist, &serveraddr, VPHONEPORT);
	strncpy2(settings.username, username, MAXNAMELEN+1);
	strncpy2(settings.password, password, MAXNAMELEN+1);
	memset(settings.accountsecret, 0, 16);
	*settings.serveraccesscode = 0;
	allowlogon = false;
	loggedon = false;
	lastserverupdatetm = 0;
	if(*settings.password)
		UsernameLogon();
	return 0;
}

int VPSTACK::Logon(const char *serveraccesscode, const BYTE *accountsecret)
{
	strcpy(settings.serveraccesscode, serveraccesscode);
	memcpy(settings.accountsecret, accountsecret, 16);
	lastserverupdatetm = 0;
	allowlogon = true;
	loggedon = false;
	return 0;
}

int VPSTACK::Logoff()
{
	if(!loggedon)
		return VPERR_NOTLOGGEDON;
	OUTMSG outmsg(DEFAULT_MTU, PKT_NOTIFYDISC);
	outmsg.AddIE_InAddr(IE_INADDR_HOST, localhostaddr, htons(settings.port));
	if(settings.forceport)
		outmsg.AddIE_InAddr(IE_INADDR_PUBLIC, 0, htons(settings.port));
	outmsg.AddIE_String(IE_SERVERACCESSCODE, settings.serveraccesscode);
	Send(&serveraddr, &outmsg);
	return 0;
}

int VPSTACK::GetLogonData(char *serveraccesscode, BYTE *accountsecret)
{
	if(!loggedon)
		return VPERR_NOTLOGGEDON;
	strcpy(serveraccesscode, settings.serveraccesscode);
	memcpy(accountsecret, settings.accountsecret, 16);
	return 0;
}

int VPSTACK::IsLoggedOn()
{
	if(loggedon)
		return 1;
	return 0;
}

const char *VPSTACK::LogonName()
{
	return settings.username;
}

const char *VPSTACK::LogonNumber()
{
	return settings.usernum;
}

int VPSTACK::Connect(VPCALL *vpcall, const char *address, int bc)
{
	VPCALL tmpvpcall;
	int rc;

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

int VPSTACK::SetDefaultVideoParameters(unsigned fourcc, unsigned width, unsigned height, unsigned framerate, unsigned quality)
{
	settings.fourcc = fourcc;
	settings.width = width;
	settings.height = height;
	settings.framerate = framerate;
	settings.quality = quality;
	return 0;
}

int VPSTACK::GetDefaultVideoParameters(unsigned *fourcc, unsigned *width, unsigned *height, unsigned *framerate, unsigned *quality)
{
	*fourcc = settings.fourcc;
	*width = settings.width;
	*height = settings.height;
	*framerate = settings.framerate;
	*quality = settings.quality;
	return 0;
}

int VPSTACK::SetCallVideoParameters(VPCALL vpcall, unsigned fourcc, unsigned width, unsigned height, unsigned framerate, unsigned quality)
{
	VPCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	vpc->videofmt.fourcc = fourcc;
	vpc->videofmt.width = width;
	vpc->videofmt.height = height;
	vpc->videofmt.framerate = framerate;
	vpc->videofmt.quality = quality;
	vpc->Unlock();
	return 0;
}

int VPSTACK::GetCallVideoParameters(VPCALL vpcall, unsigned *fourcc, unsigned *width, unsigned *height, unsigned *framerate, unsigned *quality)
{
	VPCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	*fourcc = vpc->videofmt.fourcc;
	*width = vpc->videofmt.width;
	*height = vpc->videofmt.height;
	*framerate = vpc->videofmt.framerate;
	*quality = vpc->videofmt.quality;
	vpc->Unlock();
	return 0;
}

int VPSTACK::Connect(VPCALL vpcall, const char *address, int bc)
{
	VPCALLDATA *vpc;
	int rc;

	vpc = LockCall(vpcall);
	if(vpc)
	{
		vpc->bc = bc;
		vpc->Unlock();
	} else return VPERR_INVALIDCALL;
	rc = SetCallAddress(vpcall, address);
	if(rc)
		return rc;
	vpc = LockCall(vpcall);
	if(vpc)
	{
		vpc->status = VPCS_CONNECTING;
		vpc->Unlock();
	} else return VPERR_INVALIDCALL;
	TSTART
	beginclassthread((ClassThreadStart)&VPSTACK::ConnectThread, this, (void *)vpcall);
	return 0;
}

int VPSTACK::SetCallAddress(VPCALL vpcall, const char *address)
{
	VPCALLDATA *vpc;
	const char *p;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	p = strchr(address, '@');
	if(p)
	{
		int n = p - address;

		if(n > MAXSUBADDRLEN)
			n = MAXSUBADDRLEN;
		memcpy(vpc->subaddress, address, n);
		vpc->subaddress[n] = 0;
		address = p + 1;
	} else *vpc->subaddress = 0;
	strncpy2(vpc->address, address, MAXADDRLEN+1);
	vpc->addrtype = CanonicFormAddress(vpc->address);
	if(vpc->addrtype == ADDRT_INVALID)
	{
		vpc->Unlock();
		return VPERR_INVALIDADDR;
	}
	if(vpc->bc != BC_SMS && (vpc->addrtype == ADDRT_VNAME && !stricmp(vpc->address, vpc->ourname) ||
		vpc->addrtype == ADDRT_VNUMBER && !stricmp(vpc->address, vpc->ournumber)))
	{
		vpc->Unlock();
		return VPERR_LOOPCALL;
	}
	vpc->Unlock();
	return 0;
}

int VPSTACK::SetCallLocalName(VPCALL vpcall, const char *username)
{
	VPCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	strncpy2(vpc->ourname, username, MAXNAMELEN+1);
	vpc->Unlock();
	return 0;
}

int VPSTACK::SetCallLocalNumber(VPCALL vpcall, const char *number)
{
	VPCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	strncpy2(vpc->ournumber, number, MAXNUMBERLEN+1);
	vpc->Unlock();
	return 0;
}

int VPSTACK::GetCallLocalSubAddr(VPCALL vpcall, char *number)
{
	VPCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	strncpy2(number, vpc->oursubaddress, MAXSUBADDRLEN+1);
	vpc->Unlock();
	return 0;
}

int VPSTACK::GetCallRemoteName(VPCALL vpcall, char *username)
{
	VPCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	if(*vpc->connectedname)
		strncpy2(username, vpc->connectedname, MAXNAMELEN+1);
	else if(*vpc->connectednumber)
		strncpy2(username, vpc->connectednumber, MAXNUMBERLEN+1);
	else strncpy2(username, vpc->address, MAXADDRLEN+1);
	vpc->Unlock();
	return 0;
}

int VPSTACK::GetCallRemoteNumber(VPCALL vpcall, char *number)
{
	VPCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	strncpy2(number, vpc->connectednumber, MAXNUMBERLEN+1);
	vpc->Unlock();
	return 0;
}

int VPSTACK::GetCallRemoteSubAddr(VPCALL vpcall, char *number)
{
	VPCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	strncpy2(number, vpc->connectedsubaddress, MAXSUBADDRLEN+1);
	vpc->Unlock();
	return 0;
}

int VPSTACK::GetCallRemoteAddress(VPCALL vpcall, char *address)
{
	VPCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	sprintf(address, "%s:%d", inet_ntoa(vpc->addr.sin_addr), ntohs(vpc->addr.sin_port));
	vpc->Unlock();
	return 0;
}


int VPSTACK::GetCallRemoteServerAccessCode(VPCALL vpcall, char *serveraccesscode)
{
	VPCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	strncpy2(serveraccesscode, vpc->connectedserveraccesscode, MAXSERVERACCESSCODELEN+1);
	vpc->Unlock();
	return 0;
}

int VPSTACK::GetCallLocalName(VPCALL vpcall, char *username)
{
	VPCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	if(*vpc->ourname)
		strncpy2(username, vpc->ourname, MAXNAMELEN+1);
	else if(*vpc->ournumber)
		strncpy2(username, vpc->ournumber, MAXNUMBERLEN+1);
	else *username = 0;
	vpc->Unlock();
	return 0;
}

int VPSTACK::GetCallLocalNumber(VPCALL vpcall, char *number)
{
	VPCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	strncpy2(number, vpc->ournumber, MAXNUMBERLEN+1);
	vpc->Unlock();
	return 0;
}

int VPSTACK::GetCallBearer(VPCALL vpcall)
{
	VPCALLDATA *vpc;
	int rc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	rc = vpc->bc;
	vpc->Unlock();
	return rc;
}

int VPSTACK::GetCallCodec(VPCALL vpcall)
{
	VPCALLDATA *vpc;
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

int VPSTACK::SetCallNFiles(VPCALL vpcall, unsigned nfiles, unsigned nbytes)
{
	VPCALLDATA *vpc;
	int rc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	if(vpc->status == VPCS_IDLE)
	{
		vpc->nfiles = nfiles;
		vpc->nbytes = nbytes;
		rc = 0;
	} else rc = VPERR_INVALIDSTATUS;
	vpc->Unlock();
	return rc;
}

int VPSTACK::GetCallNFiles(VPCALL vpcall, unsigned *nfiles, unsigned *nbytes)
{
	VPCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	*nfiles = vpc->nfiles;
	*nbytes = vpc->nbytes;
	vpc->Unlock();
	return 0;
}

int VPSTACK::SetCallFilePath(VPCALL vpcall, const char *path)
{
	VPCALLDATA *vpc;
	int rc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	if(vpc->status == VPCS_CONNECTED)
	{
		strncpy2(vpc->ft.path, path, sizeof(vpc->ft.path));
		rc = 0;
	} else rc = VPERR_INVALIDSTATUS;
	vpc->Unlock();
	return rc;
}

int VPSTACK::GetCallFilePath(VPCALL vpcall, char *path)
{
	VPCALLDATA *vpc;
	int rc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	if(vpc->status == VPCS_CONNECTED)
	{
		strcpy(path, vpc->ft.path);
		rc = 0;
	} else rc = VPERR_INVALIDSTATUS;
	vpc->Unlock();
	return 0;
}

int VPSTACK::GetFileProgress(VPCALL vpcall, unsigned *current, unsigned *total)
{
	VPCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	*current = vpc->ft.pos;
	*total = vpc->ft.length;
	vpc->Unlock();
	return 0;
}

int VPSTACK::IsCallOffline(VPCALL vpcall)
{
	VPCALLDATA *vpc;
	int rc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	rc = vpc->offline;
	vpc->Unlock();
	return rc;
}

int VPSTACK::IsCallWithProxy(VPCALL vpcall)
{
	VPCALLDATA *vpc;
	int rc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	rc = vpc->useproxy;
	vpc->Unlock();
	return rc;
}

int VPSTACK::GetMissedCallsData(VPCALL vpcall, unsigned *missedcallscount, unsigned *missedcalltime, unsigned *missedcallbc)
{
	VPCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	*missedcallscount = vpc->missedcallscount;
	*missedcalltime = vpc->missedcalltime;
	*missedcallbc = vpc->missedcallbc;
	vpc->Unlock();
	return 0;
}

int VPSTACK::SetMissedCallBC(VPCALL vpcall, int bc)
{
	VPCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	vpc->missedcallbc = bc;
	vpc->smstype = SMSTYPE_MISSEDCALLS;
	vpc->Unlock();
	return 0;
}

int VPSTACK::SetSMSType(VPCALL vpcall, unsigned type)
{
	VPCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	vpc->smstype = type;
	vpc->Unlock();
	return 0;
}

int VPSTACK::GetSMSType(VPCALL vpcall, unsigned *type)
{
	VPCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	*type = vpc->smstype;
	vpc->Unlock();
	return 0;
}

int VPSTACK::SetSMSId(VPCALL vpcall, unsigned id)
{
	VPCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	vpc->smsid = id;
	vpc->Unlock();
	return 0;
}

int VPSTACK::GetSMSId(VPCALL vpcall, unsigned *id)
{
	VPCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	*id = vpc->smsid;
	vpc->Unlock();
	return 0;
}

int VPSTACK::GetCallText(VPCALL vpcall, char *text)
{
	VPCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	strncpy2(text, vpc->message, MAXMESSAGELEN+1);
	vpc->Unlock();
	return 0;
}

int VPSTACK::SetCallText(VPCALL vpcall, const char *text)
{
	VPCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	strncpy2(vpc->message, text, MAXMESSAGELEN+1);
	vpc->Unlock();
	return 0;
}

int VPSTACK::SetCallDisconnectMessage(VPCALL vpcall, const char *text)
{
	VPCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	vpc->hasdisconnectmessage = true;
	strncpy2(vpc->message, text, MAXMESSAGELEN+1);
	vpc->Unlock();
	return 0;
}

void VPSTACK::DelayedClearCall(VPCALL vpcall)
{
	VPCALLDATA *vpc = LockCall(vpcall);
	if(!vpc)
		return;
	vpc->clearattm = GetTickCount() + 5000;
	vpc->Unlock();
}

int VPSTACK::GetCallStatus(VPCALL vpcall)
{
	int status;

	VPCALLDATA *vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	status = vpc->status;
	vpc->Unlock();
	return status;
}

int VPSTACK::Disconnect(VPCALL vpcall, int reason)
{
	VPCALLDATA *vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;

	if(vpc->status == VPCS_INCALLPROCEEDING || vpc->status == VPCS_INALERTED)
	{
		OUTMSG outmsg(DEFAULT_MTU, PKT_CONNECTACK);
		struct sockaddr_in addr = vpc->addr;

		if(vpc->hasdisconnectmessage)
			outmsg.AddIE_String(reason == REASON_CALLDEFLECTION? IE_DEFLECTTONUMBER : IE_MESSAGE, vpc->message);
		outmsg.AddIE_2Bytes(IE_ANSWER, ANSWER_REFUSE, reason);
		outmsg.AddIE_Word(IE_CONNECTIONID, vpc->connectionid);
		if(!settings.clir)
		{
			outmsg.AddIE_String(IE_SOURCENUMBER, vpc->ournumber);
			outmsg.AddIE_String(IE_SOURCENAME, vpc->ourname);
		}
		vpc->Unlock();
		Send(&addr, &outmsg);
		NotifyOnCall(VPMSG_CALLENDED, vpcall, reason);
		return 0;
	}
	if(vpc->status >= VPCS_CONNECTING && vpc->status <= VPCS_CONNECTED && vpc->addr.sin_addr.s_addr)
	{
		OUTMSG outmsg(DEFAULT_MTU, PKT_DISCONNECTREQ);

		if(vpc->hasdisconnectmessage)
			outmsg.AddIE_String(IE_MESSAGE, vpc->message);
		outmsg.AddIE_Word(IE_CONNECTIONID, vpc->connectionid);
		outmsg.AddIE_Byte(IE_REASON, reason);
		outmsg.AddIE_Byte(IE_BEARER, vpc->bc);
		Send(&vpc->addr, &outmsg);
		vpc->Unlock();
		NotifyOnCall(VPMSG_CALLENDED, vpcall, reason);
	} else if(vpc->status == VPCS_CONNECTING)
	{
		vpc->Unlock();
		NotifyOnCall(VPMSG_CALLENDED, vpcall, reason);
	} else vpc->Unlock();
	return 0;
}

int VPSTACK::AnswerCall(VPCALL vpcall)
{
	VPCALLDATA *vpc = LockCall(vpcall);

	if(!vpc)
		return VPERR_INVALIDCALL;
	if(vpc->status != VPCS_INCALLPROCEEDING && vpc->status != VPCS_INALERTED)
	{
		vpc->Unlock();
		return VPERR_INVALIDSTATUS;
	}
	vpc->status = VPCS_INCONNECTING;
	vpc->Unlock();
	TSTART
	beginclassthread((ClassThreadStart)&VPSTACK::IncomingCallThread, this, vpcall);
	return 0;
}

int VPSTACK::IncomingCall(const struct sockaddr_in *addr, IE_ELEMENTS *ie)
{
	if(ie->bc == BC_SMS)
		return 1;
	else return 2;
}

int VPSTACK::DropFromConference(VPCALL vpcall)
{
	int i, n;
	BYTE bc;
	unsigned token;
	struct sockaddr_in addr, publicaddr, privateaddr;
	VPCALL vpcalls[MAXVPARTIES];
	VPCALLDATA *vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;

	if(vpc->mconf.status == CONFSTATUS_IDLE)
	{
		vpc->Unlock();
		return VPERR_INVALIDSTATUS;
	}
	vpc->mconf.status = CONFSTATUS_IDLE;
	OUTMSG outmsg(DEFAULT_MTU, PKT_CONFERENCEEND);
	outmsg.AddIE_Byte(IE_BEARER, vpc->bc == BC_AUDIOVIDEO ? BC_VIDEO : vpc->bc);
	outmsg.AddIE_DWord(IE_TOKEN, vpc->mconf.token);
	outmsg.AddIE_Word(IE_CONNECTIONID, vpc->connectionid);
	addr = vpc->addr;
	publicaddr = vpc->publicaddr;
	privateaddr = vpc->privateaddr;
	token = vpc->mconf.token;
	bc = vpc->bc;
	vpc->Unlock();
	Send(&addr, &outmsg);
	n = FindConferencedCalls(token, bc, vpcalls);
	for(i = 0; i < n; i++)
	{
		struct sockaddr_in inaddr;

		vpc = LockCall(vpcalls[i]);
		if(!vpc)
			continue;
		OUTMSG outmsg(DEFAULT_MTU, PKT_DROPCONFERENCEPEER);
		outmsg.AddIE_Byte(IE_BEARER, vpc->bc == BC_AUDIOVIDEO ? BC_VIDEO : vpc->bc);
		outmsg.AddIE_DWord(IE_TOKEN, vpc->token);
		outmsg.AddIE_Word(IE_CONNECTIONID, vpc->connectionid);
		if(vpc->publicaddr.sin_addr.s_addr == publicaddr.sin_addr.s_addr && privateaddr.sin_addr.s_addr)
			inaddr = privateaddr;
		else inaddr = publicaddr;
		outmsg.AddIE_MultiInAddr(IE_MULTIINADDR_HOST, &inaddr, 1);
		vpc->Unlock();
		Send(&addr, &outmsg);
	}
	return 0;
}


int VPSTACK::StopConference(VPCALL vpcall)
{
	int i;
	VPCALLDATA *vpc;
	unsigned conferencetoken;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	conferencetoken = vpc->mconf.token;
	vpc->Unlock();
	ReadLock();
	for(i = 0; i < MAXCALLS; i++)
	{
		if(vpcalls[i] && vpcalls[i]->mconf.token == conferencetoken && vpcalls[i]->mconf.status > CONFSTATUS_IDLE)
		{
			OUTMSG outmsg(DEFAULT_MTU, PKT_CONFERENCEEND);
			outmsg.AddIE_Byte(IE_BEARER, vpc->bc == BC_AUDIOVIDEO ? BC_VIDEO : vpc->bc);
			outmsg.AddIE_DWord(IE_TOKEN, conferencetoken);
			outmsg.AddIE_Word(IE_CONNECTIONID, vpcalls[i]->connectionid);
			Send(&vpcalls[i]->addr, &outmsg);
			vpcalls[i]->mconf.status = CONFSTATUS_IDLE;
		}
	}
	ReadUnlock();
	return 0;
}

int VPSTACK::ConferenceCalls(VPCALL *vpcalls, int ncalls, bool detach)
{
	VPCALLDATA *vpc;
	int i, n = 0, waitingcalli = -1, connectedcalli = -1;
	int bc = -1;

	for(i = 0; i < ncalls; i++)
	{
		vpc = LockCall(vpcalls[i]);
		if(!vpc)
			return VPERR_INVALIDCALL;
		if(bc == -1)
			bc = vpc->bc;
		else if(bc != vpc->bc)
		{
			vpc->Unlock();
			return VPERR_INCOMPATIBLECALLS;
		}
		if(vpc->mconf.status == CONFSTATUS_IDLE)
		{
			if(vpc->status == VPCS_CONNECTED)
			{
				n++;
				connectedcalli = i;
			} else if(vpc->status >= VPCS_CONNECTING && vpc->status <= VPCS_INCONNECTING)
				waitingcalli = i;
		}
		vpc->Unlock();
	}
	if(n == ncalls)
	{
		CONFERENCEDATA *conferencedata = new CONFERENCEDATA;
		conferencedata->detach = detach;
		conferencedata->ncalls = ncalls;
		conferencedata->vpcalls = (VPCALL *)malloc(ncalls * sizeof(VPCALL));
		memcpy(conferencedata->vpcalls, vpcalls, ncalls * sizeof(VPCALL));
		TSTART
		beginclassthread((ClassThreadStart)&VPSTACK::CreateConferenceThread, this, conferencedata);
		return 0;
	} else if(n == ncalls - 1 && waitingcalli >= 0)
	{
		vpc = LockCall(vpcalls[waitingcalli]);
		if(!vpc)
			return VPERR_INVALIDCALL;
		if(vpc->mconf.conferencedata)
		{
			vpc->Unlock();
			return VPERR_INVALIDSTATUS;
		}
		CONFERENCEDATA *conferencedata = new CONFERENCEDATA;
		conferencedata->detach = detach;
		conferencedata->ncalls = ncalls;
		conferencedata->vpcalls = (VPCALL *)malloc(ncalls * sizeof(VPCALL));
		memcpy(conferencedata->vpcalls, vpcalls, ncalls * sizeof(VPCALL));
		vpc->mconf.conferencedata = conferencedata;
		vpc->Unlock();
		if(n == 1)
		{
			vpc = LockCall(vpcalls[connectedcalli]);
			if(vpc)
			{
				vpc->mconf.discondisccall = vpcalls[waitingcalli];
				vpc->Unlock();
			}
		}
		return 0;
	} else return VPERR_INVALIDSTATUS;
}

void VPSTACK::CreateConferenceThread(void *conferencedata1)
{
	CONFERENCEDATA *conferencedata = (CONFERENCEDATA *)conferencedata1;
	int i, j, n;
	unsigned conferencetoken;
	VPCALLDATA *vpc;
	unsigned tmstart = GetTickCount();
	struct sockaddr_in addr;
	bool persist, failed;
	struct sockaddr_in inaddr[MAXVPARTIES];

	EnterCriticalSection(&ctokencs);
	conferencetoken = ++lastvpcallvalue | GenerateRandomDWord() & 0xffff0000;
	LeaveCriticalSection(&ctokencs);

	// Initialize conference status
	n = 0;
	for(i = 0; i < conferencedata->ncalls; i++)
	{
		vpc = LockCall(conferencedata->vpcalls[i]);
		if(vpc)
		{
			vpc->mconf.token = conferencetoken;
			vpc->mconf.status = CONFSTATUS_WAITING;
			vpc->Unlock();
			n++;
		}
	}
	if(n < 2)
	{
		for(i = 0; i < conferencedata->ncalls; i++)
		{
			vpc = LockCall(conferencedata->vpcalls[i]);
			if(vpc)
			{
				vpc->mconf.status = CONFSTATUS_IDLE;
				vpc->Unlock();
			}
		}
		delete conferencedata;
		TRETURN
	}

	// Transmit conference request
	persist = true;
	while(GetTickCount() - tmstart < 5000 && persist)
	{
		for(i = 0; i < conferencedata->ncalls; i++)
		{
			vpc = LockCall(conferencedata->vpcalls[i]);
			if(vpc)
			{
				OUTMSG outmsg(DEFAULT_MTU, PKT_CONFERENCEREQ);
				outmsg.AddIE_Word(IE_CONNECTIONID, vpc->connectionid);
				outmsg.AddIE_Byte(IE_BEARER, vpc->bc == BC_AUDIOVIDEO ? BC_VIDEO : vpc->bc);
				outmsg.AddIE_DWord(IE_TOKEN, conferencetoken);
				if(conferencedata->detach)
					outmsg.AddIE_Byte(IE_CALLTRANSFER, 1);

				n++;
				addr = vpc->addr;
				vpc->Unlock();
				Send(&addr, &outmsg);
			}
		}
		Sleep(500);
		persist = false;
		for(i = 0; i < conferencedata->ncalls; i++)
		{
			vpc = LockCall(conferencedata->vpcalls[i]);
			if(vpc)
			{
				if(vpc->mconf.status == CONFSTATUS_WAITING)
				{
					vpc->Unlock();
					persist = true;
					break;
				}
				vpc->Unlock();
			}
		}
	}

	// Wait for user to confirm
	persist = true;
	while(GetTickCount() - tmstart < 15000 && persist)
	{
		persist = false;
		for(i = 0; i < conferencedata->ncalls; i++)
		{
			vpc = LockCall(conferencedata->vpcalls[i]);
			if(vpc)
			{
				if(vpc->mconf.status == CONFSTATUS_WAITING || vpc->mconf.status == CONFSTATUS_USERALERTED)
				{
					vpc->Unlock();
					persist = true;
					break;
				}
				vpc->Unlock();
			}
		}
		Sleep(100);
	}

	n = 0;
	for(i = 0; i < conferencedata->ncalls; i++)
	{
		vpc = LockCall(conferencedata->vpcalls[i]);
		if(vpc)
		{
			if(vpc->mconf.status == CONFSTATUS_WAITING || vpc->mconf.status == CONFSTATUS_USERALERTED)
			{
				vpc->Unlock();
				persist = true;
				break;
			}
			vpc->Unlock();
		}
	}
	n = 0;
	for(i = 0; i < conferencedata->ncalls; i++)
	{
		vpc = LockCall(conferencedata->vpcalls[i]);
		if(vpc)
		{
			if(vpc->mconf.status == CONFSTATUS_WAITING || vpc->mconf.status == CONFSTATUS_USERALERTED)
			{
				vpc->mconf.status = CONFSTATUS_IDLE;
				OUTMSG outmsg(DEFAULT_MTU, PKT_CONFERENCEEND);
				outmsg.AddIE_Word(IE_CONNECTIONID, vpc->connectionid);
				outmsg.AddIE_Byte(IE_BEARER, vpc->bc == BC_AUDIOVIDEO ? BC_VIDEO : vpc->bc);
				outmsg.AddIE_DWord(IE_TOKEN, conferencetoken);
				if(conferencedata->detach)
					outmsg.AddIE_Byte(IE_CALLTRANSFER, 1);
				addr = vpc->addr;
				vpc->Unlock();
				Send(&addr, &outmsg);
			} else {
				if(vpc->mconf.status == CONFSTATUS_ACCEPTED)
					n++;
				vpc->Unlock();
			}
		}
	}
	if(n < 2)
	{
		for(i = 0; i < conferencedata->ncalls; i++)
		{
			vpc = LockCall(conferencedata->vpcalls[i]);
			if(vpc)
			{
				if(vpc->mconf.status)
				{
					vpc->mconf.status = CONFSTATUS_IDLE;
					OUTMSG outmsg(DEFAULT_MTU, PKT_CONFERENCEEND);
					outmsg.AddIE_Word(IE_CONNECTIONID, vpc->connectionid);
					outmsg.AddIE_Byte(IE_BEARER, vpc->bc == BC_AUDIOVIDEO ? BC_VIDEO : vpc->bc);
					outmsg.AddIE_DWord(IE_TOKEN, conferencetoken);
					if(conferencedata->detach)
						outmsg.AddIE_Byte(IE_CALLTRANSFER, 1);
					addr = vpc->addr;
					vpc->Unlock();
					Send(&addr, &outmsg);
				} else vpc->Unlock();
			}
		}
		for(i = 0; i < conferencedata->ncalls; i++)
			NotifyOnCall(VPMSG_CONFERENCEFAILED, conferencedata->vpcalls[i], conferencedata->detach);
		delete conferencedata;
		TRETURN
	}
	tmstart = GetTickCount();
	persist = true;
	while(GetTickCount() < tmstart + 5000 && persist)
	{
		for(i = 0; i < conferencedata->ncalls; i++)
		{
			vpc = LockCall(conferencedata->vpcalls[i]);
			if(vpc)
			{
				if(vpc->mconf.status == CONFSTATUS_ACCEPTED)
				{
					struct sockaddr_in publicaddr, privateaddr;

					OUTMSG outmsg(DEFAULT_MTU, PKT_CONFERENCEREQ);
					outmsg.AddIE_Word(IE_CONNECTIONID, vpc->connectionid);
					outmsg.AddIE_Byte(IE_BEARER, vpc->bc == BC_AUDIOVIDEO ? BC_VIDEO : vpc->bc);
					outmsg.AddIE_DWord(IE_TOKEN, conferencetoken);
					if(conferencedata->detach)
						outmsg.AddIE_Byte(IE_CALLTRANSFER, 1);
					n = 0;
					publicaddr = vpc->publicaddr;
					privateaddr = vpc->privateaddr;
					addr = vpc->addr;
					vpc->Unlock();
					for(j = 0; j < conferencedata->ncalls; j++)
					{
						vpc = LockCall(conferencedata->vpcalls[j]);
						if(vpc)
						{
							if((vpc->mconf.status == CONFSTATUS_ACCEPTED || vpc->mconf.status == CONFSTATUS_INITIATED) && j != i && n < MAXVPARTIES)
							{								
								if(!forceuseproxy && vpc->publicaddr.sin_addr.s_addr == publicaddr.sin_addr.s_addr &&
									InetAddrIsPrivate(vpc->privateaddr.sin_addr.s_addr))
									inaddr[n++] = vpc->privateaddr;
								else inaddr[n++] = vpc->publicaddr;
							}
							vpc->Unlock();
						}
					}
					if(n > 0)
					{
						outmsg.AddIE_MultiInAddr(IE_MULTIINADDR_HOST, inaddr, n);
						Send(&addr, &outmsg);
					}
				}
			}
		}
		Sleep(500);
		persist = false;
		for(i = 0; i < conferencedata->ncalls; i++)
		{
			vpc = LockCall(conferencedata->vpcalls[i]);
			if(vpc)
			{
				if(vpc->mconf.status == CONFSTATUS_ACCEPTED)
					persist = true;
				vpc->Unlock();
			}
		}
	}
	failed = false;
	for(i = 0; i < conferencedata->ncalls; i++)
	{
		vpc = LockCall(conferencedata->vpcalls[i]);
		if(vpc)
		{
			if(vpc->mconf.status == CONFSTATUS_INITIATED)
				vpc->mconf.status = CONFSTATUS_ACTIVE;
			else {
				vpc->mconf.status = CONFSTATUS_IDLE;
				failed = true;
			}
			vpc->Unlock();
		}
	}
	for(i = 0; i < conferencedata->ncalls; i++)
	{
		if(failed)
			NotifyOnCall(VPMSG_CONFERENCEFAILED, conferencedata->vpcalls[i], conferencedata->detach);
		else NotifyOnCall(VPMSG_CONFERENCEESTABLISHED, conferencedata->vpcalls[i], conferencedata->detach);
		if(conferencedata->detach && !failed)
		{
			Sleep(5000);
			Disconnect(conferencedata->vpcalls[i], REASON_CALLTRANSFERCOMPLETE);
		}
	}
	delete conferencedata;
	TRETURN
}

int VPSTACK::SendAudio(VPCALL vpcall, int codec, unsigned int timestamp, const void *buf, int len, bool wait)
{
	BYTE pkt[1500];
	VPCALLDATA *vpc;
	struct sockaddr_in addr;

	pkt[0] = codec;
	pkt[5] = timestamp >> 8;
	pkt[6] = timestamp & 0xff;
	addr.sin_addr.s_addr = 0;
	vpc = LockCall(vpcall);
	if(vpc)
	{
		if(vpc->status == VPCS_RECORDING && vpc->havi)
			AVIWriteAudio(vpc->havi, buf, len);
		else if(vpc->status == VPCS_RECORDING && vpc->hwav)
			WAVWrite(vpc->hwav, buf, len);
		else if(vpc->status == VPCS_CONNECTED && vpc->cantx && !vpc->held && vpc->aa.f == -1 &&
			(vpc->bc == BC_VOICE || vpc->bc == BC_AUDIOVIDEO))
			addr = vpc->addr;
		pkt[1] = (BYTE)(vpc->connectionid >> 8);
		pkt[2] = (BYTE)vpc->connectionid;
		pkt[3] = (BYTE)(vpc->audioseqno >> 8);
		pkt[4] = (BYTE)vpc->audioseqno;
		vpc->audioseqno++;
		vpc->Unlock();
	}
	if(addr.sin_addr.s_addr)
	{
		memcpy(pkt + HEADERLEN, buf, len);
		Send(&addr, pkt, HEADERLEN + len);
	}
	return 0;
}

int VPSTACK::SendAudioMessage(VPCALL vpcall, const char *file, unsigned msgid)
{
	int f, ptr, rc;
	char buf[100];
	struct WAVEHDR {
		char riff[4];
		DWORD rifflen;
		char wavefmt[8];
		DWORD wavelen;
		WAVEFORMATEX wf;
	} *header;
	VPCALLDATA *vpc;

	f = openfile(file, O_RDONLY | O_BINARY);
	if(f == -1)
		return VPERR_FILENOTFOUND;
	rc = readfile(f, buf, 100);
	header = (WAVEHDR *)buf;
	if(rc != 100 || memcmp(header->riff, "RIFF", 4) || memcmp(header->wavefmt, "WAVEfmt ", 8))
	{
		closefile(f);
		return VPERR_INVALIDFILEFORMAT;
	}
	if(header->wf.wFormatTag != 0x31 || header->wf.nChannels != 1 || header->wf.nSamplesPerSec != 8000)
	{
		closefile(f);	// We only recognize GSM 8kHz at the moment
		return VPERR_INVALIDFILEFORMAT;
	}
	ptr = sizeof(WAVEHDR);
	while(ptr < 96 && !(buf[ptr] == 'd' && buf[ptr + 1] == 'a' && buf[ptr + 2] == 't' && buf[ptr + 3] == 'a'))
		ptr += BuildDWord(buf + ptr + 4) + 8;
	if(ptr >= 96)
	{
		closefile(f);
		return -1;
	}
	ptr += 8;
	if(seekfile(f, ptr, SEEK_SET) != ptr)
	{
		closefile(f);
		return VPERR_INVALIDFILEFORMAT;
	}
	vpc = LockCall(vpcall);
	if(!vpc)
	{
		closefile(f);
		return VPERR_INVALIDCALL;
	}
	if(vpc->aa.f != -1)
		closefile(vpc->aa.f);
	else vpc->aa.lastsendtm = GetTickCount();
	vpc->aa.msgid = msgid;
	vpc->aa.f = f;
	vpc->aa.readlen = 65 * vpc->framesperpacket;
	vpc->aa.sendinterval = 40 * vpc->framesperpacket;
	vpc->Unlock();
	return 0;
}

/**********************************************************************/
/* Public functions: advanced                                         */
/**********************************************************************/

int VPSTACK::SendKeypad(VPCALL vpcall, int key)
{
	VPCALLDATA *vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;

	if(vpc->status == VPCS_CONNECTED)
	{
		unsigned seqnumber;

		OUTMSG outmsg(DEFAULT_MTU, PKT_KEYPAD);
		outmsg.AddIE_Byte(IE_KEYPAD, key);
		seqnumber = ++vpc->seqnumber;
		outmsg.AddIE_DWord(IE_SEQNUMBER, seqnumber);
		vpc->Unlock();
		SendPacketReliable(vpcall, &outmsg, seqnumber);
		return 0;
	}
	vpc->Unlock();
	return VPERR_INVALIDSTATUS;
}

/**********************************************************************/
/* Public functions: information on calls                             */
/**********************************************************************/

int VPSTACK::GetConferencePeers(VPCALL vpcall, VPCALL *vpcalls, int *ncalls)
{
	VPCALLDATA *vpc = LockCall(vpcall);
	int i, n;

	if(!vpc)
		return VPERR_INVALIDCALL;
	if(vpc->mconf.conferencedata)
	{
		for(i = n = 0; i < vpc->mconf.conferencedata->ncalls && n < *ncalls; i++)
			if(vpc->mconf.conferencedata->vpcalls[i] != vpcall)
				vpcalls[n++] = vpc->mconf.conferencedata->vpcalls[i];
		*ncalls = n;
	} else *ncalls = 0;
	vpc->Unlock();
	return 0;
}

int VPSTACK::GetLocallyConferencedCalls(VPCALL vpcall, VPCALL *vpcalls, int *ncalls)
{
	VPCALLDATA *vpc = LockCall(vpcall);
	int i, n;

	if(!vpc)
		return VPERR_INVALIDCALL;
	for(i = n = 0; vpc->sconf.vpcalls[i] && n < *ncalls; i++)
		if(vpc->sconf.vpcalls[i] != vpcall)
			vpcalls[n++] = vpc->sconf.vpcalls[i];
	*ncalls = n;
	vpc->Unlock();
	return 0;
}

int VPSTACK::GetLinkedCall(VPCALL vpcall, VPCALL *linkedcall)
{
	VPCALLDATA *vpc = LockCall(vpcall);

	if(!vpc)
		return VPERR_INVALIDCALL;
	if(vpc->mconf.discondisccall)
	{
		*linkedcall = vpc->mconf.discondisccall;
		vpc->Unlock();
		return 0;
	}
	vpc->Unlock();
	return VPERR_INVALIDCALL;
}

int VPSTACK::Send(const struct sockaddr_in *addr, OUTMSG *msg)
{
	return Send(addr, msg->msg, msg->ptr - msg->msg);
}

int VPSTACK::Send(const struct sockaddr_in *addr,  const void *buf, int len)
{
	int i, rc;
	VPCALLDATA *vpc;

	if(addr && addr->sin_port)
	{
		rc = tx.Send(addr, buf, len);
		if(rc == SOCKET_ERROR)
			return 0;
		if(tx.backworkms > MAXBACKWORKFORVIDEO)
		{
			NotifyBackworkHigh();
//			printf("DROP FRAME, backworkmx = %d\n", tx.backworkms);
		}
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

void VPSTACK::NotifyBackworkHigh()
{
}

int VPSTACK::SendPacketReliable(VPCALL vpcall, OUTMSG *msg, unsigned seqnumber, int (VPSTACK::*deferredproc)(int, int), int param1, int param2)
{
	VPCALLDATA *vpc = LockCall(vpcall);

	if(!vpc)
		return VPERR_INVALIDCALL;
	ACKDATAGRAM *ad = new ACKDATAGRAM;
	ad->addr = vpc->addr;
	ad->connectionid = vpc->connectionid;
	vpc->Unlock();
	ad->buflen = msg->ptr - msg->msg;
	ad->buf = malloc(ad->buflen);
	memcpy(ad->buf, msg->msg, ad->buflen);
	ad->nextsendtm = GetTickCount();
	ad->expiretm = ad->nextsendtm + 5000;
	ad->seqnumber = seqnumber;
//	ad->deferredproc = deferredproc;
	ad->param1 = param1;
	ad->param2 = param2;
	EnterCriticalSection(&adlcs);
	adl[adllen] = ad;
	adllen++;
	LeaveCriticalSection(&adlcs);
	return 0;
}

void VPSTACK::ReliableSender()
{
	unsigned tm = GetTickCount();
	unsigned i;

	if((int)(tm - adltm) <= 0)
		return;
	EnterCriticalSection(&adlcs);
	for(i = 0; i < adllen; i++)
	{
		if((int)(tm - adl[i]->expiretm) > 0)
		{
			free(adl[i]->buf);
			delete adl[i];
			adllen--;
			memmove(adl + i, adl + i + 1, (adllen - i) * sizeof(adl[0]));
			i--;
			continue;
		}
		if((int)(tm - adl[i]->nextsendtm) > 0)
		{
			Send(&adl[i]->addr, (char *)adl[i]->buf, adl[i]->buflen);
			adl[i]->nextsendtm += 1000;
		}
	}
	LeaveCriticalSection(&adlcs);
}

void VPSTACK::ReliableReceiver(const struct sockaddr_in *addr, IE_ELEMENTS *ie)
{
	unsigned i;

	EnterCriticalSection(&adlcs);
	for(i = 0; i < adllen; i++)
	{
		if(adl[i]->addr.sin_addr.s_addr == addr->sin_addr.s_addr &&
			adl[i]->addr.sin_port == addr->sin_port &&
			adl[i]->connectionid == ie->connectionid &&
			adl[i]->seqnumber == ie->seqnumber)
		{
			free(adl[i]->buf);
			delete adl[i];
			adllen--;
			memmove(adl + i, adl + i + 1, (adllen - i) * sizeof(adl[0]));
			i--;
			continue;
		}
	}
	LeaveCriticalSection(&adlcs);
}

/**********************************************************************/
/* Other protocol functions                                           */
/**********************************************************************/

int VPSTACK::SetNotifyRoutine(void (*notify)(void *param, unsigned, unsigned, unsigned), void *param)
{
	NotifyRoutine = notify;
	NotifyParam = param;
	return 0;
}

int VPSTACK::SetSyncNotifyRoutine(int (*notify)(void *param, unsigned, unsigned, unsigned), void *param)
{
	SyncNotifyRoutine = notify;
	SyncNotifyParam = param;
	return 0;
}

void VPSTACK::Notify(int message, unsigned param1, unsigned param2)
{
	bool notify = true;

	if(message == VPMSG_CALLESTABLISHED || message == VPMSG_CALLACCEPTED || message == VPMSG_CONFERENCECALLESTABLISHED)
	{
		VPCALLDATA *vpc = LockCall((VPCALL)param1);
		if(vpc)
		{
			if(vpc->mconf.conferencedata)
			{
				CONFERENCEDATA *cd = vpc->mconf.conferencedata;
				vpc->mconf.conferencedata = 0;
				vpc->Unlock();
				TSTART
				beginclassthread((ClassThreadStart)&VPSTACK::CreateConferenceThread, this, cd);
			} else {
				vpc->Unlock();
				CallEstablished((VPCALL)param1);
			}
		}
	} else if(message == VPMSG_CALLENDED)
	{
		VPCALLDATA *vpc = LockCall((VPCALL)param1);
		if(vpc)
		{
			if(vpc->status == VPCS_TERMINATED)
				notify = false;
			else if(vpc->status && vpc->status < VPCS_TERMINATED)
			{
				vpc->status = VPCS_TERMINATED;
				vpc->cantx = false;
			}
			vpc->Unlock();
		}
	}
	if(notify && NotifyRoutine)
		NotifyRoutine(NotifyParam, message, param1, param2);
}

int VPSTACK::NotifyRc(int message, unsigned param1, unsigned param2)
{
	if(SyncNotifyRoutine)
		return SyncNotifyRoutine(SyncNotifyParam, message, param1, param2);
	return 0;
}

void VPSTACK::AutoAttendantProcess()
{
	int i, rc;
	VPCALLDATA *vpc;
	bool notify;
	char buf[1500];
	VPCALL vpcall;
	unsigned tm, msgid;

	for(i = 0; i < MAXCALLS; i++)
	{
		vpc = LockCall(i);
		if(vpc)
		{
			notify = false;
			tm = GetTickCount();
			if(vpc->status == VPCS_CONNECTED && vpc->aa.f != -1 && tm - vpc->aa.lastsendtm > vpc->aa.sendinterval)
			{
				buf[0] = vpc->codec;
				buf[1] = (BYTE)(vpc->connectionid >> 8);
				buf[2] = (BYTE)vpc->connectionid;
				buf[3] = (BYTE)(vpc->audioseqno >> 8);
				buf[4] = (BYTE)vpc->audioseqno;
				buf[5] = (BYTE)(tm >> 8);
				buf[6] = (BYTE)tm;
				rc = readfile(vpc->aa.f, buf + HEADERLEN, vpc->aa.readlen);
				if(rc == (int)vpc->aa.readlen)
				{
					Send(&vpc->addr, buf, HEADERLEN + rc);
					vpc->aa.lastsendtm += vpc->aa.sendinterval;
					vpc->audioseqno++;
				} else {
					closefile(vpc->aa.f);
					vpc->aa.f = -1;
					vpcall = vpc->vpcall;
					msgid = vpc->aa.msgid;
					notify = true;
				}
			}
			vpc->Unlock();
			if(notify)
				NotifyOnCall(VPMSG_AUDIOMSGCOMPLETE, vpcall, msgid);
		}
	}
}

void VPSTACK::TransmitThread(void *dummy)
{
	while(sock)
	{
		Sleep(10);
		PeriodicalUpdateServerAndAwake();
		AutoAttendantProcess();
		ReliableSender();
	}
	TRETURN
}

void VPSTACK::ReceiveThread(void *dummy)
{
	BYTE buf[1500];
	struct sockaddr_in addr;
	socklen_t addrlen;
	int len, i, rc;
	timeval tv;
	fd_set fs;
	unsigned tm;
	VPCALLDATA *vpc;
	VPCALL vpcall;

	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
	while(sock)
	{
		tm = GetTickCount();
		for(i = 0; i < MAXCALLS; i++)
		{
			vpc = LockCall(i);
			if(vpc)
			{
				if(vpc->awaketm && tm - vpc->awaketm > 10000 || vpc->clearattm && (int)(tm - vpc->clearattm) > 0)
				{
					vpcall = vpc->vpcall;
					vpc->Unlock();
					Disconnect(vpcall, vpc->clearattm ? REASON_NORMAL : REASON_CONNECTIONLOST);
				} else vpc->Unlock();
			}
		}
		addrlen = sizeof(addr);
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		FD_ZERO(&fs);
		FD_SET(sock, &fs);
		select(sock+1, &fs, 0, 0, &tv);
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
		if(generatelog >= 2 && (generatelog > 2 || (BYTE)*buf >= 0x80))
		{
			char s[5000], *p;
			int i, len2;

			if(len > 0)
				sprintf(s, "%u %s:%d Received %d bytes (", GetTickCount(), inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), len);
			else sprintf(s, "%u %s:%d Error\n", GetTickCount(), inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
			p = s + strlen(s);
			if(generatelog == 3 && (BYTE)*buf < 0x80 && len > 20)
				len2 = 20;
			else len2 = len;
			for(i = 0; i < len2; i++)
			{
				sprintf(p, i == len2 - 1 ? "%02x)" : "%02x,", ((BYTE *)buf)[i]);
				p += strlen(p);
			}
			Lprintf("%s", s);
		}
		if(!serveraddr.sin_addr.s_addr ||
			serveraddr.sin_addr.s_addr && addr.sin_addr.s_addr == serveraddr.sin_addr.s_addr &&
			addr.sin_port == serveraddr.sin_port || len > 0 && *(BYTE *)buf == PKT_QUERYADDRACK ||
			len > 0 && *(BYTE *)buf == PKT_QUERYONLINEACK || len > 0 && *(BYTE *)buf == PKT_NATDISCOVERACK)
		{
			if(len == -1 && rc == WSAECONNRESET || len > 0 && *buf >= 0x90 && *buf <= 0x9f ||
				*buf == PKT_NOTIFYLOGON || *buf == PKT_ACCOUNTINFO || *buf == PKT_NATDISCOVERACK)
			{
				ProcessServerMessage(&addr, buf, len);
				continue;
			}
		}
		if(len == -1 && rc == WSAECONNRESET)
		{
			for(i = 0; i < MAXCALLS; i++)
			{
				vpc = LockCall(i);
				if(vpc)
				{
					if(vpc->status == VPCS_CONNECTED && addr.sin_addr.s_addr == vpc->addr.sin_addr.s_addr && addr.sin_port == vpc->addr.sin_port)
					{
						vpcall = vpc->vpcall;
						vpc->Unlock();
						NotifyOnCall(VPMSG_CALLENDED, vpcall, REASON_CONNECTIONLOST);
					} else vpc->Unlock();
				}
			}
		}
		if(len < 1)
			continue;
		RateCount(&rxxc, len+UDPIPHLEN);
		if(*buf < 0x40)
			ProcessAudioMessage(&addr, buf, len);
		else if(*buf == PKT_VIDEO || *buf == PKT_VIDEOPARITY)
			ProcessVideoMessage(&addr, buf, len);
		else if(*buf >= 0x80 || *buf == PKT_FT)
			ProcessReceiverMessage(&addr, buf, len);
	}
	TRETURN
}

void VPSTACK::RefuseCall(const struct sockaddr_in *addr, IE_ELEMENTS *ie, int reason)
{
	OUTMSG outmsg(DEFAULT_MTU, PKT_CONNECTACK);
	outmsg.AddIE_2Bytes(IE_ANSWER, ANSWER_REFUSE, reason);
	outmsg.AddIE_Word(IE_CONNECTIONID, ie->connectionid);
	if(ie->tokenpresent)
		outmsg.AddIE_DWord(IE_TOKEN, ie->token);
	if(ie->seqnumberpresent)
		outmsg.AddIE_DWord(IE_SEQNUMBER, ie->seqnumber);
	if(!settings.clir)
	{
		outmsg.AddIE_String(IE_SOURCENUMBER, settings.usernum);
		outmsg.AddIE_String(IE_SOURCENAME, settings.username);
	}
	Send(addr, &outmsg);
	NotifyRefusedCall(addr, ie, reason);
}

void VPSTACK::AcceptCall(VPCALL vpcall, int answer, int reason, unsigned seqnumber)
{
	VPCALLDATA *vpc = LockCall(vpcall);
	struct sockaddr_in addr;
	VPCALL vpcalltoclear;

	if(!vpc)
		return;
	OUTMSG outmsg(DEFAULT_MTU, PKT_CONNECTACK);
	outmsg.AddIE_2Bytes(IE_ANSWER, answer, reason);
	outmsg.AddIE_Word(IE_CONNECTIONID, vpc->connectionid);
	if(!settings.clir)
	{
		outmsg.AddIE_String(IE_SOURCENUMBER, vpc->ournumber);
		outmsg.AddIE_String(IE_SOURCENAME, vpc->ourname);
	}
	if(vpc->addauthdata)
		outmsg.AddIE_Binary(IE_AUTHDATA, vpc->authdata, 16);
	outmsg.AddIE_DWord(IE_SEQNUMBER, seqnumber);
	outmsg.AddIE_DWord(IE_TOKEN, vpc->token);
	if(vpc->bc == BC_VOICE || vpc->bc == BC_AUDIOVIDEO)
		outmsg.AddIE_2Bytes(IE_BEARER, vpc->bc, vpc->codec);
	else outmsg.AddIE_Byte(IE_BEARER, vpc->bc);
	if(vpc->bc == BC_AUDIOVIDEO)
		outmsg.AddIE_Video(settings.fourcc, settings.width, settings.height);
	addr = vpc->addr;
	if(answer == ANSWER_REFUSE && vpc->status == VPCS_INCALLPROCEEDING && !vpc->addauthdata)
		vpcalltoclear = vpc->vpcall;
	else vpcalltoclear = 0;
	vpc->Unlock();
	Send(&addr, &outmsg);
	if(vpcalltoclear)
		DelayedClearCall(vpcalltoclear);
}

void VPSTACK::NotifyRefusedCall(const struct sockaddr_in *addr, IE_ELEMENTS *ie, int reason)
{
}

// This routine is not thread-safe, as it should be called only by the receiving thread
int VPSTACK::SeqNumberLatelyFound(const struct sockaddr_in *addr, unsigned seqnumber)
{
	unsigned tm = GetTickCount();
	unsigned i;

	if(tm - lastseqnumberscleantm > 1000)
	{
		for(i = 0; i < nseqnumbers; i++)
			if(tm - seqnumbers[i].tm > 10000)
			{
				nseqnumbers--;
				memmove(seqnumbers + i, seqnumbers + i + 1, (nseqnumbers - i) * sizeof(*seqnumbers));
			}
		lastseqnumberscleantm = tm;
	}
	for(i = 0; i < nseqnumbers; i++)
	{
		if(seqnumbers[i].addr.sin_addr.s_addr == addr->sin_addr.s_addr &&
//			seqnumbers[i].addr.sin_port == addr->sin_port &&	// not checked because of NAT, the same message can come from two different ports
			seqnumbers[i].seqnumber == seqnumber)
			return 1;
	}
	if(nseqnumbers == MAXSEQNUMBERS)
		return 0;
	seqnumbers[nseqnumbers].addr = *addr;
	seqnumbers[nseqnumbers].seqnumber = seqnumber;
	seqnumbers[nseqnumbers].tm = tm;
	nseqnumbers++;
	return 0;
}

int rxsendrtn(void *param, void *buf, int len)
{
	UDPSTREAMDATA *sd = (UDPSTREAMDATA *)param;

	BYTE buf2[1500];

	buf2[0] = PKT_FT;
	buf2[1] = 1;
	buf2[2] = (BYTE)(sd->connectionid >> 8);
	buf2[3] = (BYTE)sd->connectionid;
	memcpy(buf2 + 4, buf, len);
	return !sd->vpstack->Send(&sd->addr, buf2, len + 4);
}

int rxrecvrtn(void *param, void *buf, int len)
{
	UDPSTREAMDATA *sd = (UDPSTREAMDATA *)param;

	return sd->vpstack->RxFile(sd->vpc, buf, len);
}

int txsendrtn(void *param, void *buf, int len)
{
	UDPSTREAMDATA *sd = (UDPSTREAMDATA *)param;
	BYTE buf2[1500];

	buf2[0] = PKT_FT;
	buf2[1] = 0;
	buf2[2] = (BYTE)(sd->connectionid >> 8);
	buf2[3] = (BYTE)sd->connectionid;
	memcpy(buf2 + 4, buf, len);
	return !sd->vpstack->Send(&sd->addr, buf2, len + 4);
}

int txrecvrtn(void *param, void *buf, int len)
{
	return 0;
}

static unsigned filecrc(char *path, unsigned to);

int FourccSupported(unsigned fourcc)
{
#ifdef INCLUDEVP3
	if(fourcc == F_VP31)
		return 1;
#endif
#ifdef INCLUDEXVID
	if(fourcc == F_XVID)
		return 1;
#endif
#ifdef INCLUDEAVC
	if(fourcc == F_H264)
		return 1;
#endif
	return 0;
}

void VPSTACK::ProcessReceiverMessage(const struct sockaddr_in *addr, BYTE *buf, int buflen)
{
	IE_ELEMENTS ie;
	VPCALLDATA *vpc;
	VPCALL vpcall;
	int notifymsg, rc, i, prevrc, prevhl;

	switch(*buf)
	{
	case PKT_CONNECTREQ:
		GetIE_Elements(buf, buflen, &ie);
		if(ie.bc > BC_LAST || !(settings.supportedbearers & (1<<ie.bc)))
		{
			RefuseCall(addr, &ie, REASON_BEARER_NOT_SUPPORTED);
			break;
		}
		if((ie.bc == BC_VOICE || ie.bc == BC_AUDIOVIDEO) &&
			!(settings.supportedcodecs & ie.bc_codecs ||
				ie.bc_flags & 1 & settings.supportedcodecs ||
				ie.codec == 0 && settings.supportedcodecs & 1))
		{
			RefuseCall(addr, &ie, REASON_CODEC_NOT_SUPPORTED);
			break;
		}
		if(ie.bc == BC_VIDEOMSG && ie.codec && !(settings.supportedcodecs & (1<<ie.codec)))
		{
			RefuseCall(addr, &ie, REASON_CODEC_NOT_SUPPORTED);
			break;
		}
		if((ie.bc == BC_VIDEO || ie.bc == BC_AUDIOVIDEO || ie.bc == BC_VIDEOMSG) &&
			(ie.fourcc || ie.bc != BC_VIDEOMSG) && !FourccSupported(ie.fourcc))
		{
			RefuseCall(addr, &ie, REASON_CODEC_NOT_SUPPORTED);
			break;
		}
		if(!*ie.srcnum && settings.refuseclir)
		{
			RefuseCall(addr, &ie, REASON_MISSINGCLID);
			break;
		}
		vpc = FindCall(addr, ie.connectionid, ie.bc);
		if(vpc)
		{
			int reason, answer;

			vpcall = vpc->vpcall;
			i = vpc->status;
			if(vpc->status == VPCS_INCALLPROCEEDING)
			{
				if(vpc->gacd.reqauth)
				{
					if(ie.authdata)
					{
						if(!memcmp(ie.authdata, vpc->gacd.authdata, 16))
						{
							vpc->gacd.authenticated = true;
							vpc->Unlock();
							AcceptCall(vpcall, ANSWER_USERALERTED, REASON_CALLPROCEEDING, 0);
						} else {
							vpc->addauthdata = 0;
							answer = ANSWER_REFUSE;
							reason = REASON_AUTHERROR;
							vpc->Unlock();
							AcceptCall(vpcall, ANSWER_REFUSE, REASON_AUTHERROR, ie.seqnumber);
							DelayedClearCall(vpcall);
						}
					} else {
						vpc->Unlock();
						AcceptCall(vpcall, ANSWER_REFUSE, REASON_AUTHERROR, ie.seqnumber);
					}
				} else {
					vpc->Unlock();
					AcceptCall(vpcall, ANSWER_USERALERTED, REASON_CALLPROCEEDING, ie.seqnumber);
				}
				break;
			}
			vpc->Unlock();
			switch(i)
			{
			case VPCS_INCONNECTING:
				AcceptCall(vpcall, ANSWER_ACCEPT, REASON_NORMAL, ie.seqnumber);
				break;
			case VPCS_INALERTED:
				AcceptCall(vpcall, ANSWER_USERALERTED, REASON_NORMAL, ie.seqnumber);
				break;
			case VPCS_CONNECTED:
				RefuseCall(addr, &ie, REASON_ALREADYCONNECTED);
				break;
			default:
				RefuseCall(addr, &ie, REASON_BUSY);
				break;
			}
			break;
		}
		rc = IncomingCall(addr, &ie);
		if(rc <= 0)
		{
			RefuseCall(addr, &ie, -rc);
			break;
		}
		if(ie.seqnumberpresent && SeqNumberLatelyFound(addr, ie.seqnumber))
			break;
		vpcall = CreateCall();
		if(!vpcall)
		{
			RefuseCall(addr, &ie, REASON_BUSY);
			break;
		}
		vpc = LockCall(vpcall);
		if(!vpc)
			break;
		vpc->addr = *addr;
		strcpy(vpc->connectedname, ie.srcname);
		strcpy(vpc->connectednumber, ie.srcnum);
		strcpy(vpc->connectedsubaddress, ie.srcsubaddr);
		strcpy(vpc->connectedserveraccesscode, ie.serveraccesscode);
		strcpy(vpc->sessionid, ie.sessionid);
		strcpy(vpc->sessionsecret, ie.sessionsecret);
		vpc->nfiles = ie.nfiles;
		vpc->nbytes = ie.nbytes;
		if(*ie.destname)
			strcpy(vpc->ourname, ie.destname);
		if(*ie.destnum)
			strcpy(vpc->ournumber, ie.destnum);
		if(*ie.destsubaddr)
			strcpy(vpc->oursubaddress, ie.destsubaddr);
		strcpy(vpc->message, ie.message);
		vpc->connectionid = ie.connectionid;
		vpc->bc = ie.bc;
		vpc->token = ie.token;
		vpc->smstype = ie.smstype;
		vpc->smsid = ie.smsid;
		if(ie.bc == BC_VOICE || ie.bc == BC_AUDIOVIDEO)
			vpc->codec = DecideCodec(&ie);
		vpc->rtfencoding = ie.rtfencoding;
		vpc->privateaddr = ie.privateaddr;
		vpc->publicaddr = ie.publicaddr;
		vpc->missedcallbc = ie.missedcallbc;
		vpc->missedcallscount = ie.missedcallscount;
		vpc->missedcalltime = ie.missedcalltime;
		vpc->videofmt.fourcc = ie.fourcc;
		vpc->videofmt.width = ie.xres;
		vpc->videofmt.height = ie.yres;
		vpc->status = rc == 1 ? VPCS_INCONNECTING : rc == 2 ? VPCS_INALERTED : VPCS_INCALLPROCEEDING;
		vpc->Unlock();
		if(rc == 1)
			AcceptCall(vpcall, ANSWER_ACCEPT, REASON_NORMAL, ie.seqnumber);
		else if(rc == 2)
			AcceptCall(vpcall, ANSWER_USERALERTED, REASON_NORMAL, ie.seqnumber);
		else AcceptCall(vpcall, ANSWER_USERALERTED, REASON_CALLPROCEEDING, ie.seqnumber);
		NotifyOnCall(VPMSG_NEWCALL, vpcall, 0);
		break;
	case PKT_CONNECTACK:
		GetIE_Elements(buf, buflen, &ie);
		vpc = FindCallOrDisconnect(addr, ie.connectionid, ie.bc);
		if(!vpc)
			break;
		notifymsg = 0;
		vpcall = vpc->vpcall;
		if((vpc->status == VPCS_CONNECTING || vpc->status == VPCS_CALLPROCEEDING || 
			vpc->status == VPCS_ALERTED || vpc->status == VPCS_CONNECTED)
			&& ie.answer == ANSWER_ACCEPT && vpc->bc != BC_SMS)
		{
			OUTMSG outmsg(DEFAULT_MTU, PKT_CONNECTACK_RECEIVED);
			if(vpc->bc == BC_VOICE || vpc->bc == BC_AUDIOVIDEO)
				outmsg.AddIE_3Bytes(IE_BEARER, vpc->bc, vpc->codec, vpc->codec == CODEC_GSM ? 1 : 0);
			AddIE_Reply(&outmsg, &ie);
			AddIE_Addresses(&outmsg);
			Send(addr, &outmsg);
			vpc->cantx = true;
		}
		if(vpc->status == VPCS_CONNECTING || vpc->status == VPCS_CALLPROCEEDING || vpc->status == VPCS_ALERTED)
		{
			strcpy(vpc->connectedname, ie.srcname);
			strcpy(vpc->connectednumber, ie.srcnum);
			strcpy(vpc->connectedsubaddress, ie.srcsubaddr);
			strcpy(vpc->connectedserveraccesscode, ie.serveraccesscode);
			vpc->codec = ie.codec;
			vpc->rtfencoding = ie.rtfencoding;
			if(vpc->bc == BC_VIDEO || vpc->bc == BC_AUDIOVIDEO)
			{
				vpc->videofmt.fourcc = ie.fourcc;
				vpc->videofmt.width = ie.xres;
				vpc->videofmt.height = ie.yres;
			}
			if(ie.answer == ANSWER_USERALERTED || ie.answer == ANSWER_ACCEPT)
			{
				if(ie.answer == ANSWER_USERALERTED && ie.reason == REASON_CALLPROCEEDING && vpc->status == VPCS_CONNECTING)
					vpc->status = VPCS_CALLPROCEEDING;
				else if(ie.answer == ANSWER_USERALERTED && (vpc->status == VPCS_CONNECTING || vpc->status == VPCS_CALLPROCEEDING))
				{
					vpc->status = VPCS_ALERTED;
					notifymsg = VPMSG_USERALERTED;
				} else if(ie.answer == ANSWER_ACCEPT)
				{
					vpc->status = VPCS_CONNECTED;
					notifymsg = VPMSG_CALLACCEPTED;
				}
			} else if(ie.answer == ANSWER_REFUSE)
			{
				if(ie.reason == REASON_CALLDEFLECTION || ie.reason == REASON_SERVERREDIRECTION)
				{
					if(*ie.deflnum)
					{
						SetCallAddress(vpc->vpcall, ie.deflnum);
						vpc->restartconnectthread = true;
					} else if(ie.hostaddr.sin_addr.s_addr)
					{
						vpc->addr = ie.hostaddr;
						vpc->retry = 0;
					}
				} else if(ie.reason == REASON_AUTHERROR && ie.authdata)
				{
					if(!vpc->addauthdata)
					{
						aes_ctx acx;

						aes_dec_key(settings.accountsecret, 16, &acx);
						aes_dec_blk(ie.authdata, vpc->authdata, &acx);
						vpc->addauthdata = 1;
						vpc->starttm = GetTickCount() + 1000;
						vpc->status = VPCS_CONNECTING;
					}
				} else {
					strcpy(vpc->message, ie.message);
					vpc->status = VPCS_DISCONNECTING;
					notifymsg = VPMSG_CALLREFUSED;
				}
			}
		}
		vpc->Unlock();
		if(notifymsg)
			NotifyOnCall(notifymsg, vpcall, ie.reason);
		break;
	case PKT_CONNECTACK_RECEIVED:
		GetIE_Elements(buf, buflen, &ie);
		vpc = FindCallOrDisconnect(addr, ie.connectionid, ie.bc);
		if(!vpc)
			break;
		if(vpc->status != VPCS_INCONNECTING)
		{
			vpc->Unlock();
			break;
		}
		if(ie.bc == BC_VOICE || ie.bc == BC_AUDIOVIDEO)
			if((ie.bc_flags >> 1) & 1)
				vpc->codec = CODEC_GSM;
		vpc->status = VPCS_CONNECTED;
		if(ie.privateaddr.sin_addr.s_addr)
			vpc->privateaddr = ie.privateaddr;
		if(ie.publicaddr.sin_addr.s_addr)
			vpc->publicaddr = ie.publicaddr;
		vpc->cantx = true;
		vpcall = vpc->vpcall;
		vpc->Unlock();
		NotifyOnCall(VPMSG_CALLESTABLISHED, vpcall, 0);
		break;
	case PKT_CALLREQUESTACK:
		GetIE_Elements(buf, buflen, &ie);
		for(i = 0; i < MAXCALLS; i++)
		{
			VPCALLDATA *vpc;

			vpc = LockCall(i);
			if(vpc)
			{
				if(vpc->status == VPCS_CONNECTING && vpc->sendcallrequests && vpc->token == ie.token)
				{
					vpc->sendcallrequests = false;
					vpc->altport = vpc->addr.sin_port;
					vpc->addr = *addr;
					vpc->retry = 0;
				}
				vpc->Unlock();
			}
		}
		break;
	case PKT_DISCONNECTREQ:
		GetIE_Elements(buf, buflen, &ie);
		vpc = FindCall(addr, ie.connectionid, ie.bc);
		if(!vpc)
			break;
		if(*ie.message)
			strcpy(vpc->message, ie.message);
		vpcall = vpc->vpcall;
		vpc->Unlock();
		NotifyOnCall(VPMSG_CALLDISCONNECTED, vpcall, ie.reason);
		NotifyOnCall(VPMSG_CALLENDED, vpcall, ie.reason);
		break;
	case PKT_PROXYBINDACK:
		GetIE_Elements(buf, buflen, &ie);
		vpc = FindCallByToken(addr, ie.token);
		if(!vpc)
			break;
		notifymsg = 0;
		if(ie.reason == REASON_AUTHERROR && ie.authdata)
		{
			if(!vpc->addauthdata)
			{
				aes_ctx acx;

				aes_enc_key(settings.accountsecret, 16, &acx);
				aes_enc_blk(ie.authdata, vpc->authdata, &acx);
				vpc->connectionid = ie.connectionid;
				vpc->addauthdata = 1;
				vpc->starttm = GetTickCount() + 1000;
			}
		} else if(ie.reason == REASON_NORMAL)
		{
			if(!vpc->proxybound)
				vpc->connectionid = ie.connectionid;
			vpc->proxybound = true;
		} else {
			notifymsg = VPMSG_CALLREFUSED;
			vpc->status = VPCS_DISCONNECTING;
		}
		vpcall = vpc->vpcall;
		vpc->Unlock();
		if(notifymsg)
			NotifyOnCall(notifymsg, vpcall, ie.reason);
		break;
	case PKT_CHAT:
	case PKT_CHATACK:
		GetIE_Elements(buf, buflen, &ie);
		vpc = FindCallOrDisconnect(addr, ie.connectionid, BC_CHAT);
		if(!vpc)
			break;
		vpcall = vpc->vpcall;
		memcpy(vpc->rtfdata, ie.rtfdata, ie.rtfdatalen);
		vpc->rtfdatalen = ie.rtfdatalen;
		vpc->rtfencoding = ie.rtfencoding;
		if(*buf == PKT_CHAT)
		{
			vpc->Unlock();	
			OUTMSG outmsg(DEFAULT_MTU, PKT_CHATACK);
			outmsg.AddIE_Byte(IE_BEARER, ie.bc);
			AddIE_Reply(&outmsg, &ie);
			Send(addr, &outmsg);
			NotifyOnCall(VPMSG_CHAT, vpcall, 0);
		} else {
			if(vpc->chatqueuelen && ie.seqnumber == vpc->chatqueue[0].seqnumber)
			{
				vpc->rtfdatalen = vpc->chatqueue[0].rtfdatalen;
				memcpy(vpc->rtfdata, vpc->chatqueue[0].rtfdata, vpc->rtfdatalen);
				vpc->chatqueuelen--;
				vpc->chatretry = 5;
				memmove(vpc->chatqueue, vpc->chatqueue+1, vpc->chatqueuelen*sizeof(vpc->chatqueue[0]));
			}
			vpc->Unlock();
			NotifyOnCall(VPMSG_CHATACK, vpcall, 0);
		}
		break;
	case PKT_AWAKE:
		for(i = 0; i < MAXCALLS; i++)
		{
			VPCALLDATA *vpc;

			vpc = LockCall(i);
			if(vpc)
			{
				if(vpc->status == VPCS_CONNECTED && vpc->addr.sin_addr.s_addr == addr->sin_addr.s_addr && vpc->addr.sin_port == addr->sin_port)
					vpc->awaketm = GetTickCount();
				vpc->Unlock();
			}
		}
		break;
	case PKT_MTUTEST:
		GetIE_Elements(buf, buflen, &ie);
		vpc = FindCallOrDisconnect(addr, ie.connectionid, ie.bc);
		if(!vpc)
			break;
		if(buflen > 548)
			vpc->mtu = buflen;
		else vpc->mtu = DEFAULT_MTU;
		{
			OUTMSG outmsg(DEFAULT_MTU, PKT_INFO);
			AddIE_Reply(&outmsg, &ie);
			outmsg.AddIE_Word(IE_MTU, vpc->mtu);
			vpc->Unlock();
			Send(addr, &outmsg);
		}
		break;
	case PKT_INFO:
		GetIE_Elements(buf, buflen, &ie);
		vpc = FindCallOrDisconnect(addr, ie.connectionid, 0xff);
		if(!vpc)
			break;
		if(ie.mtu > DEFAULT_MTU)
			vpc->mtu = ie.mtu;
		vpc->Unlock();
		break;
		break;
	case PKT_CONFERENCEACK:
		GetIE_Elements(buf, buflen, &ie);
		vpc = FindCallOrDisconnect(addr, ie.connectionid, 0xff);
		if(!vpc)
			break;
		if(vpc->mconf.status >= CONFSTATUS_WAITING && vpc->mconf.status <= CONFSTATUS_INITIATED &&
			vpc->mconf.token == ie.token && ie.bc == (vpc->bc == BC_AUDIOVIDEO ? BC_VIDEO : vpc->bc))
		{
			if(ie.answer == ANSWER_USERALERTED && vpc->mconf.status == CONFSTATUS_WAITING)
				vpc->mconf.status = CONFSTATUS_USERALERTED;
			if(ie.answer == ANSWER_ACCEPT)
				vpc->mconf.status = CONFSTATUS_ACCEPTED;
			if(ie.answer == ANSWER_REFUSE)
				vpc->mconf.status = CONFSTATUS_IDLE;
			if(ie.answer == ANSWER_INITIATED)
				vpc->mconf.status = CONFSTATUS_INITIATED;
			vpc->Unlock();
		} else vpc->Unlock();
		break;
	case PKT_CONFERENCEREQ:
		GetIE_Elements(buf, buflen, &ie);
		vpc = FindCallOrDisconnect(addr, ie.connectionid, ie.bc);
		if(!vpc)
			break;
		if(!(ie.bc == BC_VIDEO && !ie.calltransfer || ie.bc == BC_VOICE && ie.calltransfer))	// Wrong bearer
		{
			vpc->Unlock();
			OUTMSG outmsg(DEFAULT_MTU, PKT_CONFERENCEACK);
			outmsg.AddIE_Byte(IE_BEARER, ie.bc);
			AddIE_Reply(&outmsg, &ie);
			outmsg.AddIE_2Bytes(IE_ANSWER, ANSWER_REFUSE, REASON_BEARER_NOT_SUPPORTED);
			Send(addr, &outmsg);
			break;
		} else if(vpc->sconf.status && (vpc->sconf.token != ie.token || vpc->sconf.masteraddr.sin_addr.s_addr !=
			addr->sin_addr.s_addr || vpc->sconf.masteraddr.sin_port != addr->sin_port))
		{
			vpc->Unlock();
			OUTMSG outmsg(DEFAULT_MTU, PKT_CONFERENCEACK);
			outmsg.AddIE_Byte(IE_BEARER, ie.bc);
			AddIE_Reply(&outmsg, &ie);
			outmsg.AddIE_2Bytes(IE_ANSWER, ANSWER_REFUSE, REASON_CALLPROCESSING);
			Send(addr, &outmsg);
		} else if(!ie.hostaddr_array[0].sin_addr.s_addr)	// Request
		{
			if(ie.calltransfer)
			{
				memset(&vpc->sconf, 0, sizeof(vpc->sconf));
				vpc->sconf.masteraddr = *addr;
				vpc->sconf.token = ie.token;
				vpc->sconf.status = CONFSLAVESTATUS_USERCONFIRMED;
				OUTMSG outmsg(DEFAULT_MTU, PKT_CONFERENCEACK);
				outmsg.AddIE_Byte(IE_BEARER, ie.bc);
				AddIE_Reply(&outmsg, &ie);
				outmsg.AddIE_Byte(IE_ANSWER, ANSWER_ACCEPT);
				Send(addr, &outmsg);
				vpc->Unlock();
			} else {
				OUTMSG outmsg(DEFAULT_MTU, PKT_CONFERENCEACK);
				outmsg.AddIE_Byte(IE_BEARER, ie.bc);
				AddIE_Reply(&outmsg, &ie);
				outmsg.AddIE_Byte(IE_ANSWER, vpc->sconf.status >= CONFSLAVESTATUS_USERCONFIRMED ? ANSWER_ACCEPT : ANSWER_USERALERTED);
				Send(addr, &outmsg);
				if(vpc->sconf.status == CONFSLAVESTATUS_IDLE || vpc->sconf.status == CONFSLAVESTATUS_ALERTNECESSARY)
				{
					memset(&vpc->sconf, 0, sizeof(vpc->sconf));
					vpc->sconf.masteraddr = *addr;
					vpc->sconf.token = ie.token;
					vpc->sconf.status = CONFSLAVESTATUS_USERALERTED;
					vpcall = vpc->vpcall;
					vpc->Unlock();
					NotifyOnCall(VPMSG_CONFERENCEREQ, vpcall, 0);
				} else vpc->Unlock();
			}
		} else {	// Initiate
			if(vpc->sconf.status < CONFSLAVESTATUS_USERCONFIRMED)
			{
				vpc->Unlock();
				OUTMSG outmsg(DEFAULT_MTU, PKT_CONFERENCEACK);
				outmsg.AddIE_Byte(IE_BEARER, ie.bc);
				AddIE_Reply(&outmsg, &ie);
				outmsg.AddIE_Byte(IE_ANSWER, ANSWER_REFUSE);
				Send(addr, &outmsg);
				break;
			}
			if(vpc->sconf.status == CONFSLAVESTATUS_USERCONFIRMED)
			{
				char ourname[MAXNAMELEN+1], ournumber[MAXNUMBERLEN+1];
				VPCALL newvpcall;
				int n, loopcalls;

				vpc->sconf.status = CONFSLAVESTATUS_INITIATED;
				vpc->sconf.detach = !!ie.calltransfer;
				vpc->sconf.bc = ie.bc;
				strcpy(ourname, vpc->ourname);
				strcpy(ournumber, vpc->ournumber);
				vpcall = vpc->vpcall;
				vpc->Unlock();

				// Create calls
				n = 0;
				loopcalls = 0;
				for(i = 0; i < MAXVPARTIES; i++)
					if(ie.hostaddr_array[i].sin_addr.s_addr)
					{
						if(ie.hostaddr_array[i].sin_addr.s_addr != localhostaddr ||
							ie.hostaddr_array[i].sin_port != htons(settings.port))
						{
							newvpcall = CreateCall();
							if(newvpcall)
							{
								// Set call parameters
								vpc = LockCall(newvpcall);
								vpc->status = VPCS_CONNECTING;
								vpc->sconf.status = CONFSLAVESTATUS_CONNECTING;
								vpc->sconf.token = ie.token;
								vpc->sconf.masteraddr = *addr;
								vpc->bc = ie.bc;								
								vpc->publicaddr = vpc->addr = ie.hostaddr_array[i];
								strcpy(vpc->ourname, ourname);
								strcpy(vpc->ournumber, ournumber);
								vpc->Unlock();
								AssignConnectionId(newvpcall, &ie.hostaddr_array[i]);
								
								// Assign call to master call
								vpc = LockCall(vpcall);
								if(vpc)
								{
									vpc->sconf.vpcalls[n++] = newvpcall;
									vpc->Unlock();
								}
							}
						} else loopcalls++;
					}
				if(n)
				{
					TSTART
					beginclassthread((ClassThreadStart)&VPSTACK::DoConferenceCallThread, this, (void *)vpc->vpcall);
				} else {
					vpc = LockCall(vpcall);
					if(vpc)
					{
						if(vpc->sconf.detach)
							vpc->sconf.status = CONFSLAVESTATUS_IDLE;
						vpc->Unlock();
					}
				}
				OUTMSG outmsg(DEFAULT_MTU, PKT_CONFERENCEACK);
				outmsg.AddIE_Byte(IE_BEARER, ie.bc);
				AddIE_Reply(&outmsg, &ie);
				outmsg.AddIE_Byte(IE_ANSWER, ANSWER_INITIATED);
				Send(addr, &outmsg);
				if(loopcalls)
					LocalConference(vpcall);
				if(ie.calltransfer && !n)
					Disconnect(vpcall, REASON_CALLTRANSFERCOMPLETE);
			}
		}
		break;
	case PKT_CONFERENCEEND:
		GetIE_Elements(buf, buflen, &ie);
		{
			VPCALL vpcalls[MAXVPARTIES];
			int n;
			OUTMSG outmsg(DEFAULT_MTU, PKT_GENERALACK);

			AddIE_Reply(&outmsg, &ie);
			Send(addr, &outmsg);
			memset(vpcalls, 0, sizeof(vpcalls));
			// Find the call connected to the master and sign it as "non-conferenced"
			n = FindConferenceCalls(addr, ie.token, ie.bc == BC_VIDEO ? BC_AUDIOVIDEO : ie.bc, vpcalls);
			for(i = 0; i < n; i++)
			{
				vpc = LockCall(vpcalls[i]);
				if(vpc)
				{
					vpc->sconf.status = CONFSLAVESTATUS_IDLE;
					memcpy(vpc->sconf.vpcalls, vpcalls, sizeof(vpcalls));
					vpc->Unlock();
				}
			}
			if(n)
				NotifyOnCall(VPMSG_CONFERENCEEND, vpcalls[0], 0);
			// Find the calls connected to the other slaves and disconnect them
			n = FindConferenceCalls(0, ie.token, ie.bc == BC_AUDIOVIDEO ? BC_VIDEO : ie.bc, vpcalls);
			for(i = 0; i < n; i++)
				Disconnect(vpcalls[i], REASON_NORMAL);
		}
		break;
	case PKT_CONFERENCECALL:
		GetIE_Elements(buf, buflen, &ie);
		if(!(ie.bc == BC_VIDEO && !ie.calltransfer || ie.bc == BC_VOICE && ie.calltransfer))	// Wrong bearer
			break;
		if(ie.confsetupstep < 2)
		{
			char ourname[MAXNAMELEN+1];
			char ournumber[MAXNUMBERLEN+1];
			bool docalltransfer;
			int usedcodec;
			VPCALL mastervpcall;

			vpc = FindMasterCall(&ie.hostaddr, ie.token);
			if(!vpc)
				break;
			docalltransfer = vpc->sconf.detach;
			strcpy(ourname, vpc->ourname);
			strcpy(ournumber, vpc->ournumber);
			mastervpcall = vpc->vpcall;
			vpc->Unlock();
			vpc = FindCallByConfToken(&ie.publicaddr, &ie.privateaddr, ie.token);
			if(!vpc)
				break;
			if(ie.confsetupstep == 1)
				vpc->sconf.step0acked = true;
			if(ie.confsetupstep == 0)
			{
				// Take the private address on the lowest
				int p1 = InetAddrIsPrivate(addr->sin_addr.s_addr);
				int p2 = InetAddrIsPrivate(vpc->addr.sin_addr.s_addr);
				if(ie.connectionid < vpc->connectionid)
					vpc->connectionid = ie.connectionid;
				if(p1 && !p2 || (p1 && p2 || !p1 && !p2) && ntohl(addr->sin_addr.s_addr) < ntohl(vpc->addr.sin_addr.s_addr))
					vpc->addr = *addr;
			}
			strcpy(vpc->connectedname, ie.srcname);
			strcpy(vpc->connectednumber, ie.srcnum);
			vpc->privateaddr = ie.privateaddr;
			vpc->publicaddr = ie.publicaddr;

			OUTMSG outmsg(DEFAULT_MTU, PKT_CONFERENCECALL);
			usedcodec = DecideCodec(&ie);
			if(ie.bc == BC_VOICE)
				outmsg.AddIE_3Bytes(IE_BEARER, BC_VOICE, usedcodec, usedcodec == CODEC_GSM ? 1 : 0);
			else {
				outmsg.AddIE_Byte(IE_BEARER, BC_VIDEO);
				outmsg.AddIE_Video(vpc->videofmt.fourcc, vpc->videofmt.width, vpc->videofmt.height);
			}
			outmsg.AddIE_Word(IE_CONNECTIONID, vpc->connectionid);
			outmsg.AddIE_Byte(IE_CALLTRANSFER, docalltransfer);
			outmsg.AddIE_InAddr(IE_INADDR_HOST, ie.hostaddr.sin_addr.s_addr, ie.hostaddr.sin_port);
			outmsg.AddIE_DWord(IE_TOKEN, ie.token);
			outmsg.AddIE_Byte(IE_CONFSETUPSTEP, ie.confsetupstep + 1);
			AddIE_Addresses(&outmsg);
			if(!settings.clir)
			{
				outmsg.AddIE_String(IE_SOURCENUMBER, vpc->ournumber);
				outmsg.AddIE_String(IE_SOURCENAME, vpc->ourname);
			}
			notifymsg = 0;
			if(vpc->status != VPCS_CONNECTED)
			{
				vpc->sconf.status = CONFSLAVESTATUS_CONNECTED;
				vpc->status = VPCS_CONNECTED;
				vpcall = vpc->vpcall;
				vpc->cantx = true;
				notifymsg = VPMSG_CONFERENCECALLESTABLISHED;
			}
			vpc->Unlock();
			Send(addr, &outmsg);
			if(notifymsg)
			{
				NotifyOnCall(VPMSG_NEWCALL, vpcall, 0);
				NotifyOnCall(VPMSG_CONFERENCECALLESTABLISHED, vpcall, 0);
			}
		} else if(ie.confsetupstep == 2)
		{
			vpc = FindCall(addr, ie.connectionid, ie.bc);
			if(vpc)
			{
				notifymsg = 0;
				if(vpc->status != VPCS_CONNECTED)
				{
					vpc->sconf.status = CONFSLAVESTATUS_CONNECTED;
					vpc->status = VPCS_CONNECTED;
					vpcall = vpc->vpcall;
					vpc->cantx = true;
					notifymsg = VPMSG_CONFERENCECALLESTABLISHED;
				}
				vpc->Unlock();
				if(notifymsg)
				{
					NotifyOnCall(VPMSG_NEWCALL, vpcall, 0);
					NotifyOnCall(VPMSG_CONFERENCECALLESTABLISHED, vpcall, 0);
				}
			}
		}
		break;
	case PKT_DROPCONFERENCEPEER:
		GetIE_Elements(buf, buflen, &ie);
		vpc = FindCallOrDisconnect(addr, ie.connectionid, ie.bc);
		if(!vpc)
			break;
		{
			OUTMSG outmsg(DEFAULT_MTU, PKT_GENERALACK);

			AddIE_Reply(&outmsg, &ie);
			Send(addr, &outmsg);
		}
		if(vpc->sconf.status == CONFSLAVESTATUS_IDLE || ie.token != vpc->sconf.token ||
			addr->sin_addr.s_addr != vpc->sconf.masteraddr.sin_addr.s_addr ||
			addr->sin_port != vpc->sconf.masteraddr.sin_port)
		{
			vpc->Unlock();
			break;
		}
		vpc->Unlock();
		for(i = 0; i < MAXVPARTIES; i++)
			if(ie.hostaddr_array[i].sin_addr.s_addr)
			{
				vpc = FindCall(&ie.hostaddr_array[i], 0, ie.bc == BC_AUDIOVIDEO ? BC_VIDEO : ie.bc);
				if(vpc)
				{
					VPCALL vpcall = vpc->vpcall;
					vpc->Unlock();
					Disconnect(vpcall, REASON_NORMAL);
				}
			}
		break;
	case PKT_KEYPAD:
		GetIE_Elements(buf, buflen, &ie);
		vpc = FindCallOrDisconnect(addr, ie.connectionid, ie.bc);
		if(!vpc)
			break;
		{
			OUTMSG outmsg(DEFAULT_MTU, PKT_GENERALACK);

			AddIE_Reply(&outmsg, &ie);
			Send(addr, &outmsg);
		}
		vpcall = vpc->vpcall;
		if((int)(ie.seqnumber - vpc->lastkeypadseqnumber) > 0)
		{
			vpc->lastkeypadseqnumber = ie.seqnumber;
			vpc->Unlock();
			NotifyOnCall(VPMSG_KEYPAD, vpcall, ie.keypad);
		} else vpc->Unlock();
		break;
	case PKT_CALLSTATUS:
		GetIE_Elements(buf, buflen, &ie);
		vpc = FindCallOrDisconnect(addr, ie.connectionid, ie.bc);
		if(!vpc)
			break;
		prevrc = vpc->remotelyconferenced;
		prevhl = vpc->remotelyheld;
		vpc->remotelyconferenced = ie.callstatus & 2 ? true : false;
		vpc->remotelyheld = ie.callstatus & 1 ? true : false;
		vpcall = vpc->vpcall;
		vpc->Unlock();
		{
			OUTMSG outmsg(DEFAULT_MTU, PKT_GENERALACK);
			AddIE_Reply(&outmsg, &ie);
			Send(addr, &outmsg);
			if(!prevrc && ie.callstatus & 2)
				NotifyOnCall(VPMSG_REMOTELYCONFERENCED, vpcall, 0);
			else if(prevrc && !(ie.callstatus & 2))
				NotifyOnCall(VPMSG_REMOTELYUNCONFERENCED, vpcall, 0);
			if(!prevhl && ie.callstatus & 1)
				NotifyOnCall(VPMSG_REMOTELYHELD, vpcall, 0);
			else if(prevhl && !(ie.callstatus & 1))
				NotifyOnCall(VPMSG_REMOTELYRESUMED, vpcall, 0);
		}
		break;
	case PKT_GENERALACK:
		GetIE_Elements(buf, buflen, &ie);
		ReliableReceiver(addr, &ie);
		break;
	case PKT_GETAUTHCALLDATAACK:
		GetIE_Elements(buf, buflen, &ie);
		GetAuthCallDataAck(addr, &ie);
		break;

	//	Start file transfer messages
	case PKT_SENDFILEREQ:
		GetIE_Elements(buf, buflen, &ie);
		vpc = FindCallOrDisconnect(addr, ie.connectionid, ie.bc);
		if(!vpc)
			break;
		vpcall = 0;
		if(!vpc->ft.receiving)
		{
			vpc->ft.receiving = 1;
			vpc->ft.pos = 0;
			vpc->ft.length = ie.filelength;
			strcpy(vpc->ft.path, ie.filename);
			vpcall = vpc->vpcall;
		}
		{
			OUTMSG outmsg(DEFAULT_MTU, PKT_SENDFILEACK);
			
			outmsg.AddIE_Byte(IE_ANSWER, ANSWER_USERALERTED);
			AddIE_Reply(&outmsg, &ie);
			vpc->Unlock();
			Send(addr, &outmsg);
		}
		if(vpcall)
			NotifyOnCall(VPMSG_SENDFILEREQ, vpcall, 0);
		break;
	case PKT_SENDFILEACK:
		GetIE_Elements(buf, buflen, &ie);
		vpc = FindCallOrDisconnect(addr, ie.connectionid, ie.bc);
		if(!vpc)
			break;
		vpcall = vpc->vpcall;
		if(!vpc->ft.sending)
		{
			vpc->Unlock();
			break;
		}
		if(ie.answer == ANSWER_REFUSE)
		{
			vpc->ft.accepted = 3;
			vpc->Unlock();
			NotifyOnCall(VPMSG_SENDFILEACK_REFUSED, vpcall, ie.reason);
			break;
		}
		if(ie.answer == ANSWER_USERALERTED && !vpc->ft.accepted)
		{
			vpc->ft.accepted = 1;
			vpc->Unlock();
			NotifyOnCall(VPMSG_SENDFILEACK_USERALERTED, vpcall, 0);
		} else if(ie.answer == ANSWER_ACCEPT)
		{
			OUTMSG outmsg(DEFAULT_MTU, PKT_FILEINFO);

			if(vpc->ft.accepted == 2)
				vpcall = 0;
			else {
				vpc->ft.accepted = 2;
				if(ie.mtu >= DEFAULT_MTU)
					vpc->mtu = ie.mtu;
				if(vpc->ft.length >= ie.filelength && filecrc(vpc->ft.path, ie.filelength) == ie.filecrc)
					vpc->ft.startpos = ie.filelength;
				else vpc->ft.startpos = 0;
			}
			AddIE_Reply(&outmsg, &ie);
			outmsg.AddIE_DWord(IE_STARTPOS, vpc->ft.startpos);
			outmsg.AddIE_DWord(IE_FILELENGTH, vpc->ft.length - vpc->ft.startpos);
			Send(addr, &outmsg);
			vpc->Unlock();
			if(vpcall)
				NotifyOnCall(VPMSG_SENDFILEACK_ACCEPTED, vpcall, 0);
		} else vpc->Unlock();
		break;
	case PKT_FILEINFO:
		GetIE_Elements(buf, buflen, &ie);
		vpc = FindCallOrDisconnect(addr, ie.connectionid, ie.bc);
		if(!vpc)
			break;
		if(vpc->ft.receiving != 2)
		{
			vpc->Unlock();
			break;
		}
		vpcall = vpc->vpcall;
		if(vpc->ft.f)	// This should never happen
			closefile(vpc->ft.f);
		if(ie.startpos)
		{
			vpc->ft.f = openfile(vpc->ft.path, O_WRONLY | O_BINARY);
			if(vpc->ft.f != -1)
			{
				if(seekfile(vpc->ft.f, ie.startpos, SEEK_SET) == -1)
				{
					closefile(vpc->ft.f);
					vpc->ft.f = 0;
				}
			} else vpc->ft.f =  0;
		} else {
			vpc->ft.f = openfile(vpc->ft.path, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC);
			if(vpc->ft.f == -1)
				vpc->ft.f = 0;
		}
		vpc->ft.starttm = GetTickCount();
		vpc->ft.startpos = ie.startpos;
		if(vpc->ft.f)
		{
			vpc->ft.receiving = 3;
			UDPSTREAMDATA *sd;
			sd = new UDPSTREAMDATA;
			sd->vpstack = this;
			sd->vpcall = vpc->vpcall;
			sd->addr = vpc->addr;
			sd->connectionid = vpc->connectionid;
			sd->vpc = 0;
			vpc->ft.pos = 0;
			vpc->ft.length = ie.filelength;
			if(vpc->ft.startpos)
				Lprintf("Receiving %s from pos %d", vpc->ft.path, vpc->ft.startpos);
			else Lprintf("Receiving %s", vpc->ft.path);
			vpc->ft.tmout = GetTickCount() + 10000;
			vpc->ft.udpstream = new UDPStream;
			vpc->ft.udpstream->Init(rxsendrtn, sd, rxrecvrtn, sd, 504);
			vpc->Unlock();
			NotifyOnCall(VPMSG_FTPROGRESS, vpcall, 0);
		} else {
			TerminateFTReceive(vpc);
			vpc->Unlock();
			Lprintf("Error opening %s in write mode", vpc->ft.path);
		}
		break;
	case PKT_FT:
		vpc = FindCallOrDisconnect(addr, ntohs(*(WORD *)(buf + 2)), 0xff);
		if(!vpc)
			break;
		if(!buf[1] && vpc->ft.receiving)
		{
			vpc->ft.tmout = GetTickCount() + 10000;
			((UDPSTREAMDATA *)vpc->ft.udpstream->recvuser)->vpc = vpc;
			vpc->ft.udpstream->udprecv(buf + 4, buflen - 4);
			((UDPSTREAMDATA *)vpc->ft.udpstream->recvuser)->vpc = 0;
		} else if(buf[1] && vpc->ft.sending)
		{
			((UDPSTREAMDATA *)vpc->ft.udpstream->recvuser)->vpc = vpc;
			vpc->ft.udpstream->udprecv(buf + 4, buflen - 4);
			((UDPSTREAMDATA *)vpc->ft.udpstream->recvuser)->vpc = 0;
		} else if(buflen != 4)
		{	// Null packet = ERROR
			BYTE msg[4];

			msg[0] = PKT_FT;
			msg[1] = !buf[1];
			msg[2] = buf[2];
			msg[3] = buf[3];
			Send(addr, msg, 4);
		}
		vpc->Unlock();
		break;
	case PKT_SENDFILEEND:
		GetIE_Elements(buf, buflen, &ie);
		vpc = FindCallOrDisconnect(addr, ie.connectionid, ie.bc);
		if(!vpc)
			break;
		vpcall = vpc->vpcall;
		{
			OUTMSG outmsg(DEFAULT_MTU, PKT_SENDFILEENDACK);

			AddIE_Reply(&outmsg, &ie);
			Send(addr, &outmsg);
			if(vpc->ft.receiving)
			{
				TerminateFTReceive(vpc); // Stop receiving if is the case
				vpc->Unlock();
				if(ie.reason == REASON_TIMEOUT)
					NotifyOnCall(VPMSG_FTTIMEOUT, vpcall, 0);
				NotifyOnCall(VPMSG_FTRXCOMPLETE, vpcall, 0);
			} else vpc->Unlock();
		}
		break;
	case PKT_SENDFILEENDACK:
		GetIE_Elements(buf, buflen, &ie);
		vpc = FindCallOrDisconnect(addr, ie.connectionid, ie.bc);
		if(!vpc)
			break;
		vpc->ft.accepted = 2;	// Acknowledged
		vpc->Unlock();
		break;
	// End file transfer messages
	}
}

/**********************************************************************/
/* Names cache                                                        */
/**********************************************************************/

char stackhomedir[MAXPATH];

int ResolveUserName(char *address)
{
	FILE *fp;
	char s[300], path[MAX_PATH], *p;

	if(!*stackhomedir)
		return -1;
	sprintf(path, "%s\\Namescache.txt", stackhomedir);
	fp = fopen(path, "r");
	if(!fp)
		return 0;
	while(fgets(s, 300, fp))
	{
		p = strchr(s, '\n');
		if(p)
			*p = 0;
		p = strchr(s, '\t');
		if(!p)
			continue;
		*p++ = 0;
		if(!stricmp(address, s))
		{
			strcpy(address, p);
			fclose(fp);
			return 1;
		}
	}
	fclose(fp);
	return 0;
}

int CacheUsername(const char *username, const char *vpnumber)
{
	char address[300], path[MAX_PATH], bakpath[MAX_PATH], s[300], *p;
	FILE *fpi, *fpo;
	bool saved = false;

	if(!*stackhomedir)
		return -1;
	strcpy(address, username);
	if(ResolveUserName(address) && !stricmp(vpnumber, address))
		return 0;
	sprintf(path, "%s\\Namescache.txt", stackhomedir);
	sprintf(bakpath, "%s\\Namescache.bak", stackhomedir);
	deletefile(bakpath);
	renamefile(path, bakpath);
	fpo = fopen(path, "w");
	if(!fpo)
		return -1;
	fpi = fopen(bakpath, "r");
	if(!fpi)
	{
		fprintf(fpo, "%s\t%s\n", username, vpnumber);
		fclose(fpo);
		return 0;
	}
	while(fgets(s, 300, fpi))
	{
		p = strchr(s, '\n');
		if(p)
			*p = 0;
		p = strchr(s, '\t');
		if(!p)
			continue;
		*p++ = 0;
		if(!stricmp(s, username))
		{
			saved = true;
			fprintf(fpo, "%s\t%s\n", username, vpnumber);
		} else fprintf(fpo, "%s\t%s\n", s, p);
	}
	if(!saved)
		fprintf(fpo, "%s\t%s\n", username, vpnumber);
	fclose(fpi);
	fclose(fpo);
	return 0;
}

/**********************************************************************/
/* Protocol                                                           */
/**********************************************************************/

void VPSTACK::ProcessServerMessage(const struct sockaddr_in *addr, BYTE *buf, int buflen)
{
	IE_ELEMENTS ie;
	int rc, i;

	if(buflen > 0)
	{
		GetIE_Elements(buf, buflen, &ie);
		switch(*buf)
		{
		case PKT_NOTIFYACK:
			if(*ie.serveraccesscode && (strcmp(ie.serveraccesscode, settings.serveraccesscode) || ie.token != instancetoken))
				break;
			notifyackawaitedtm = 0;
			notifyackfailed = 0;
			if(*ie.serverslist)
			{
				if(strcmp(settings.serverslist, ie.serverslist))
				{
					strcpy(settings.serverslist, ie.serverslist);
					Notify(VPMSG_SERVERSLISTCHANGED, 0, 0);
				}
			}
			if(ie.answer != ANSWER_ACCEPT && ie.reason == REASON_DISCONNECTED)
			{
				char s[300];

				allowlogon = false;
				sprintf(s, "Disconnected from server by %s:%d", inet_ntoa(ie.hostaddr.sin_addr), ntohs(ie.hostaddr.sin_port));
				sprintf(s + strlen(s), " (%s:%d)", inet_ntoa(ie.privateaddr.sin_addr), ntohs(ie.privateaddr.sin_port));
				Lprintf("%s", s);
				Notify(VPMSG_SERVERSTATUS, ie.reason, 0);
			}
			if(ie.answer != ANSWER_ACCEPT)
			{
				if(ie.reason == REASON_AUTHERROR && ie.authdata)
				{
					aes_ctx acx;

					lastserverupdatetm = GetTickCount();
					OUTMSG outmsg(DEFAULT_MTU, PKT_NOTIFY);
					outmsg.AddIE_DWord(IE_VERSION, stackversion);
					outmsg.AddIE_DWord(IE_USERSTATUS, settings.userstatus);
					outmsg.AddIE_InAddr(IE_INADDR_HOST, localhostaddr, htons(settings.port));
					if(settings.forceport)
						outmsg.AddIE_InAddr(IE_INADDR_PUBLIC, 0, htons(settings.port));
					outmsg.AddIE_String(IE_SERVERACCESSCODE, settings.serveraccesscode);
					outmsg.AddIE_DWord(IE_TOKEN, instancetoken);
					if(apntoken)
						outmsg.AddIE_Binary(IE_TRANSPARENTDATA, (const unsigned char *)apntoken, apntokensize);
					aes_dec_key(settings.accountsecret, 16, &acx);
					aes_dec_blk(ie.authdata, ie.authdata, &acx);
					outmsg.AddIE_Binary(IE_AUTHDATA, ie.authdata, 16);
					if(settings.notifyinterval)
						outmsg.AddIE_DWord(IE_EXPIRATION, settings.notifyinterval * 19 / 6);
					notifyackawaitedtm = lastserverupdatetm + 5000;
					Send(addr, &outmsg);
				} else if(ie.reason != REASON_SERVERREDIRECTION)
				{
					loggedon = false;
					allowlogon = false;
					Lprintf("Not logged on");
					Notify(VPMSG_SERVERSTATUS, ie.reason, 0);
				}
			} else {
				serveraddr = *addr;
				if(*ie.destnum)
				{
					PlainNumber(ie.destnum, settings.usernum);
					Lprintf("Usernumber received: \"%s\"", ie.destnum);
				} else if(generatelog >= 2)
					Lprintf("Usernum=\"%s\"", settings.usernum);
				if(*ie.destname)
					strcpy(settings.username, ie.destname);
				lastversion = ie.version;
				if(ie.version > stackversion)
					Notify(VPMSG_SERVERSTATUS, REASON_VERSIONOLD, ie.version);
				localhostpublicaddr = ie.publicaddr;
				if(!loggedon)
				{
					loggedon = true;
					Lprintf("Logged on");
					Notify(VPMSG_SERVERSTATUS, REASON_LOGGEDON, 0);
				}
			}
			break;
		case PKT_QUERYADDRACK:
			EnterCriticalSection(&resolvercs);
			for(i = 0; i < MAXRESOLVERS; i++)
				if(resolvers[i].active && (resolvers[i].token == ie.token || !ie.token))
				{
					RESOLVER *resolver = &resolvers[i];

					if(ie.answer == ANSWER_ACCEPT)
					{
						resolver->addr = ie.hostaddr;
						strcpy(resolver->resolvedusername, ie.destname);
						PlainNumber(ie.srcnum, resolver->resolvedvpnumber);
						resolver->privateaddr = ie.privateaddr;
						resolver->publicaddr = ie.publicaddr;
						if(ie.offline)
							resolver->stat = RES_OFFLINE;
						else resolver->stat = RES_FOUND;
					} else if(ie.answer == ANSWER_USERALERTED)
					{
						// vPservre does not know the address yet (e.g. APN sent to iphone), wait more
						resolver->waittimeout = GetTickCount() + 60000;
					} else if(ie.answer == ANSWER_REFUSE && ie.reason == REASON_SERVERREDIRECTION)
					{
						OUTMSG outmsg(DEFAULT_MTU, PKT_QUERYADDR);
						
						outmsg.AddIE_String(IE_SERVERACCESSCODE, settings.serveraccesscode);
						outmsg.AddIE_String(IE_SOURCENAME, settings.username);
						outmsg.AddIE_String(resolver->usename ? IE_DESTNAME : IE_DESTNUMBER, resolver->address);
						outmsg.AddIE_DWord(IE_TOKEN, resolver->token);
						outmsg.AddIE_Byte(IE_CALLREQUEST, 1);
						if(resolver->bc == BC_VOICE)
							outmsg.AddIE_BC(resolver->bc, resolver->codec, 0, resolver->supportedcodecs);
						else outmsg.AddIE_Byte(IE_BEARER, resolver->bc);
						outmsg.AddIE_Byte(IE_OFFLINE, resolver->offline);
						Send(&ie.hostaddr, &outmsg);
						resolver->waittimeout = GetTickCount() + 2000;
					} else resolver->stat = RES_NOTFOUND;	// Not found indicator
				}
			LeaveCriticalSection(&resolvercs);
			break;
		case PKT_CALLREQUEST:
			{
				OUTMSG outmsg(DEFAULT_MTU, PKT_CALLREQUESTACK);
				outmsg.AddIE_DWord(IE_TOKEN, ie.token);
				Send(&ie.hostaddr, &outmsg);
				break;
			}
		case PKT_CALLFORWARDINGACK:
			cfinprogress = false;
			strcpy(settings.cfnumber, ie.destnum);
			settings.cfop = ie.operation;
			Notify(VPMSG_CALLFORWARDING, ie.reason, 0);
			break;
		case PKT_QUERYONLINEACK:
			if(ie.seqnumber != aoldata.seqnumber || ie.serverid != aoldata.srvid)
				break;
			if(ie.serverid != aoldata.srvid)
				break;
			if(ie.encodednumbersptr && ie.nencodednumbers)
			{
				if(ie.serverid == aoldata.srvid)
				{
					if(ie.nencodednumbers > 130)
						ie.nencodednumbers = 130;
					memcpy(encodednumbers, ie.encodednumbersptr, 4 * ie.nencodednumbers);
					nencodednumbers = ie.nencodednumbers;
					queryonlineack = true;
				}
			} else if(ie.reason == REASON_SERVERREDIRECTION && ie.hostaddr.sin_addr.s_addr)
			{
				OUTMSG outmsg(DEFAULT_MTU, PKT_QUERYONLINE);
				outmsg.AddIE_String(IE_SERVERACCESSCODE, settings.serveraccesscode);
				outmsg.AddIE_DWord(IE_SERVERID, aoldata.srvid);
				outmsg.AddIE_DWord(IE_SEQNUMBER, aoldata.seqnumber);
				outmsg.AddIE_Byte(IE_OPERATION, aoldata.operation);
				outmsg.AddIE_EncodedNumbers(aoldata.numbers, aoldata.N);
				Send(&ie.hostaddr, &outmsg);
			} else queryonlineack = true;
			break;
		case PKT_NOTIFYLOGON:
			{
				OUTMSG outmsg(DEFAULT_MTU, PKT_NOTIFYLOGONACK);
				outmsg.AddIE_DWord(IE_TOKEN, ie.token);
				outmsg.AddIE_DWord(IE_SEQNUMBER, ie.seqnumber);
				if(ie.hostaddr.sin_addr.s_addr)
					addr = &ie.hostaddr;
				rc = NotifyRc(VPMSG_NOTIFYLOGON, (unsigned)ie.srcnum, 0);
				if(rc)
					outmsg.AddIE_Byte(IE_REASON, REASON_NOTFOUND);
				Send(addr, &outmsg);
			}
			break;
		case PKT_NATDISCOVERACK:
			if(ie.token == nat.token && (ie.seqnumber == 1 || ie.seqnumber == 2))
			{
				char s[300];
				struct hostent *h;
				int i, step = ie.seqnumber - 1;

				gethostname(s, 300);
				h = gethostbyname(s);
				if(h)
				{
					for(i = 0; h->h_addr_list[i]; i++)
						if(*((unsigned long *)h->h_addr_list[i]) == ie.hostaddr.sin_addr.s_addr)
							nat.nonat[step] = true;
				}
				nat.received[step] = true;
				nat.addrs[step] = ie.hostaddr;
				if(ie.reason == REASON_ANOTHERADDR)
					nat.fullcone[step] = true;
				else if(ie.reason == REASON_ANOTHERPORT)
					nat.restricted[step] = true;
			}
			break;
		case PKT_ACTIVATEACCOUNTACK:
			if(ie.answer == ANSWER_ACCEPT)
			{
				if(ie.authdata)
				{
					aes_ctx acx;
					int i, n;
					BYTE key[16];

					memset(key, 0, 16);
					for(n = i = 0; i < 16; i++)
					{
						if(!settings.password[n])
							n = 0;
						key[i] = settings.password[n++];
					}
					aes_dec_key(key, 16, &acx);
					aes_dec_blk(ie.authdata, ie.authdata, &acx);
					memcpy(settings.accountsecret, ie.authdata, 16);
				}
				if(*ie.serveraccesscode)
					strcpy(settings.serveraccesscode, ie.serveraccesscode);
				strcpy(settings.usernum, ie.destnum);
				allowlogon = true;
			} else if(ie.reason == REASON_ACCOUNTBLOCKED)
				Notify(VPMSG_SERVERSTATUS, REASON_ACCOUNTBLOCKED, 0);
			else if(ie.reason == REASON_SERVERREDIRECTION)
			{
				if(*settings.username)
				{
					OUTMSG outmsg(500, PKT_ACTIVATEACCOUNT);
					outmsg.AddIE_String(IE_SOURCENAME, settings.username);
					if(*settings.serveraccesscode)
						outmsg.AddIE_String(IE_SERVERACCESSCODE, settings.serveraccesscode);
					Send(&ie.hostaddr, &outmsg);
				}
			} else if(ie.authdata && ie.reason == REASON_AUTHERROR)
			{
				aes_ctx acx;
				int i, n;
				BYTE key[16];

				memset(key, 0, 16);
				for(n = i = 0; i < 16; i++)
				{
					if(!settings.password[n])
						n = 0;
					key[i] = settings.password[n++];
				}
				aes_dec_key(key, 16, &acx);
				aes_dec_blk(ie.authdata, ie.authdata, &acx);
				if(*ie.serveraccesscode)
					strcpy(settings.serveraccesscode, ie.serveraccesscode);
				OUTMSG outmsg(500, PKT_ACTIVATEACCOUNT);
				outmsg.AddIE_String(IE_SOURCENAME, settings.username);
				if(*settings.serveraccesscode)
					outmsg.AddIE_String(IE_SERVERACCESSCODE, settings.serveraccesscode);
				outmsg.AddIE_Binary(IE_AUTHDATA, ie.authdata, 16);
				Send(addr, &outmsg);
			} else if(ie.reason == REASON_AUTHERROR || ie.reason == REASON_NOTFOUND)
				Notify(VPMSG_SERVERSTATUS, ie.reason, 0);
			else Notify(VPMSG_SERVERSTATUS, REASON_ADMINERROR, 0);
			break;
		case PKT_ACCOUNTINFO:
			if(ie.reason == REASON_NORMAL)
				Notify(VPMSG_CREDIT, ie.quota, ie.smsquotapresent ? ie.smsquota : -1);
			break;
		}
	} else {
		ResolveServer();
		Notify(VPMSG_SERVERSTATUS, REASON_SERVERRESET, 0);
	}
}

void VPSTACK::ResolveServer()
{
	if(loggedon || GetTickCount() - lastserverupdatetm > 10000)
		lastserverupdatetm = 0;
	serveraddr.sin_addr.s_addr = 0;
	loggedon = false;
}

void VPSTACK::PeriodicalUpdateServerAndAwake()
{
	unsigned tm;
	int i;
	OUTMSG outmsg(DEFAULT_MTU, PKT_AWAKE);
	VPCALLDATA *vpc;

	// Awake to connected vPhones
	tm = GetTickCount();
	if(AWAKESENDINTERVAL < tm - lastawaketm)
	{
		lastawaketm = tm;
		for(i = 0; i < MAXCALLS; i++)
		{
			vpc = LockCall(i);
			if(vpc)
			{
				if(vpc->status == VPCS_CONNECTED && vpc->cantx)
				{
					outmsg.Reset(PKT_AWAKE);
					outmsg.AddIE_Word(IE_CONNECTIONID, vpc->connectionid);
					Send(&vpc->addr, &outmsg);
				}
				vpc->Unlock();
			}
		}
	}

	// Picoserver periodical notification

	if(notifyackawaitedtm && tm - notifyackawaitedtm < 0x80000000)
	{
		notifyackawaitedtm = 0;
		notifyackfailed++;
		if(notifyackfailed == 2)
		{
			notifyackfailed = 0;
			ResolveServer();
			Notify(VPMSG_SERVERSTATUS, REASON_NORMAL, 0);
		}
	}
	if((!lastserverupdatetm || tm - lastserverupdatetm > (unsigned)(settings.notifyinterval ? settings.notifyinterval : DEFAULTSERVERUPDATEINTERVAL)) && allowlogon)
	{
		localhostaddr = FindLocalHostAddr(settings.localhostsaddr, settings.autolocalhostaddr);
		lastserverupdatetm = tm;
		OUTMSG outmsg(DEFAULT_MTU, PKT_NOTIFY);
		outmsg.AddIE_DWord(IE_VERSION, stackversion);
		outmsg.AddIE_DWord(IE_USERSTATUS, settings.userstatus);
		if(localhostaddr)
			outmsg.AddIE_InAddr(IE_INADDR_HOST, localhostaddr, htons(settings.port));
		if(settings.forceport)
			outmsg.AddIE_InAddr(IE_INADDR_PUBLIC, 0, htons(settings.port));
		outmsg.AddIE_String(IE_SERVERACCESSCODE, settings.serveraccesscode);
		outmsg.AddIE_DWord(IE_TOKEN, instancetoken);
		if(settings.notifyinterval)
			outmsg.AddIE_DWord(IE_EXPIRATION, settings.notifyinterval * 10 / 3);
		if(apntoken)
			outmsg.AddIE_Binary(IE_TRANSPARENTDATA, (const unsigned char *)apntoken, apntokensize);
		notifyackawaitedtm = tm + 5000;
		if(!serveraddr.sin_addr.s_addr)
		{
			char *p, *q, s[300];
			struct sockaddr_in addr;

			p = settings.serverslist;
			while(*p && !serveraddr.sin_addr.s_addr)
			{
				q = strchr(p, ';');
				if(!q)
					q = p + strlen(p);
				memcpy(s, p, q - p);
				s[q - p] = 0;
				if(Resolve(s, &addr, VPHONEPORT))
				{
					Send(&addr, &outmsg);
					Sleep(30);
				}
				p = *q == ';' ? q + 1 : q;
			}
		} else Send(&serveraddr, &outmsg);
	}
}

int VPSTACK::vPResolve(const char *address, char *resolvednumber, sockaddr_in *addr, sockaddr_in *publicaddr, sockaddr_in *privateaddr, unsigned token, int bc, int offline, int codec, int supportedcodecs, VPCALL vpcall)
{
	struct sockaddr_in;
	int rc, n, originalresolveusername;
	RESOLVER *resolver = 0;

	for(;;)
	{
		EnterCriticalSection(&resolvercs);
		for(n = 0; n < MAXRESOLVERS; n++)
			if(!resolvers[n].active)
			{
				resolver = &resolvers[n];
				Zero(*resolver);
				resolver->active = true;
				break;
			}
		LeaveCriticalSection(&resolvercs);
		if(resolver)
			break;
		Sleep(10);
	}
	*resolvednumber = 0;
	originalresolveusername = IsAddressVName(address);
	strcpy(resolver->address, address);
	ResolveUserName(resolver->address);
	resolver->codec = codec;
	resolver->supportedcodecs = supportedcodecs;
	resolver->usename = IsAddressVName(resolver->address);
	for(n = 0; n < 6; n++)
	{
		if(serveraddr.sin_addr.s_addr)
		{
			OUTMSG outmsg(DEFAULT_MTU, PKT_QUERYADDR);
			
			resolver->token = token;
			outmsg.AddIE_String(IE_SERVERACCESSCODE, settings.serveraccesscode);
			outmsg.AddIE_String(IE_SOURCENAME, settings.username);
			outmsg.AddIE_String(resolver->usename ? IE_DESTNAME : IE_DESTNUMBER, resolver->address);
			outmsg.AddIE_DWord(IE_TOKEN, resolver->token);
			if(resolver->token)
				outmsg.AddIE_Byte(IE_CALLREQUEST, 1);
			if(bc == BC_VOICE)
				outmsg.AddIE_BC(bc, codec, 0, supportedcodecs);
			else outmsg.AddIE_Byte(IE_BEARER, bc);
			outmsg.AddIE_Byte(IE_OFFLINE, offline);
			resolver->stat = RES_IDLE;
			resolver->bc = bc;
			resolver->offline = offline;
			resolver->addr.sin_port = 0;
			for(n = 0; n < 5 && resolver->stat == RES_IDLE; n++)
			{
				rc = Send(&serveraddr, &outmsg);
				resolver->waittimeout = GetTickCount() + 1000;
				while(resolver->stat == RES_IDLE && (int)(resolver->waittimeout - GetTickCount()) > 0)
				{
					if(vpcall && (int)(resolver->waittimeout - GetTickCount()) > 2000)
					{
						VPCALLDATA *vpc = LockCall(vpcall);
						if(!vpc)
						{
							resolver->active = false;
							return RES_ABORTED;
						}
						vpc->Unlock();

					}
					Sleep(10);
				}
			}
			if((resolver->stat == RES_FOUND || resolver->stat == RES_NOTFOUND || resolver->stat == RES_OFFLINE) && !resolver->usename && originalresolveusername && stricmp(resolver->resolvedusername, address))
			{
				// If the user searched for a username and the search was peformed using the cached number
				// and they don't match, perform the search using the username
				strcpy(resolver->address, address);
				resolver->usename = 1;
				n = 0;
				continue;
			}
			if((resolver->stat == RES_FOUND || resolver->stat == RES_OFFLINE) && *resolver->resolvedvpnumber && *resolver->resolvedusername)
			{
				strcpy(resolvednumber, resolver->resolvedvpnumber);
				CacheUsername(resolver->resolvedusername, resolver->resolvedvpnumber);
				NotifyRc(VPMSG_ABUPDATE, (unsigned)resolver->resolvedusername, (unsigned)resolver->resolvedvpnumber);
			}
			if(resolver->stat == RES_FOUND)
			{
				if(!resolver->addr.sin_addr.s_addr)
				{
					*addr = serveraddr;
					return RES_OFFLINE;
				}
				if(addr)
					*addr = resolver->addr;
				if(publicaddr)
					*publicaddr = resolver->publicaddr;
				if(privateaddr)
					*privateaddr = resolver->privateaddr;
				resolver->active = false;
				return RES_FOUND;
			}
			rc = resolver->stat;
			if(rc == RES_OFFLINE && addr)
				*addr = resolver->addr;
			if(rc == RES_IDLE)
				ResolveServer();
			resolver->active = false;
			return rc;
		}
		if(!n)
			ResolveServer();
		Sleep(500);
	}
	resolver->active = false;
	return RES_IDLE;
}

void VPSTACK::AddIE_Addresses(OUTMSG *outmsg)
{
	outmsg->AddIE_InAddr(IE_INADDR_PRIVATE, localhostaddr, htons(settings.port));
	outmsg->AddIE_InAddr(IE_INADDR_PUBLIC, localhostpublicaddr.sin_addr.s_addr, localhostpublicaddr.sin_port);
}

int VPSTACK::DecideCodec(IE_ELEMENTS *ie)
{
	int i;

	if(!ie)
	{
		if(settings.supportedcodecs & (1<<settings.preferredcodec))
			return settings.preferredcodec;
		for(i = 0 ; i < 32; i++)
			if(settings.supportedcodecs & (1<<i))
				return i;
		return CODEC_GSM;
	}
	if(1<<ie->codec & settings.supportedcodecs)
		return ie->codec;
	else {
		if(ie->bc_codecs)
		{
			if(ie->bc_codecs & settings.supportedcodecs & (1<<settings.preferredcodec))
				return settings.preferredcodec;
			else if(ie->bc_codecs & settings.supportedcodecs)
			{
				for(i = 0 ; i < 32; i++)
					if(ie->bc_codecs & settings.supportedcodecs & (1<<i))
						return i;
				return -1;
			} else return -1;
		} else if(ie->bc_flags & 1)
			return CODEC_GSM;
		else return -1;
	}
}

void VPSTACK::ConnectThread(void *vpcall1)
{
	int rc;
	VPCALL vpcall = (VPCALL)vpcall1;
	VPCALLDATA *vpc;
	struct {
		char address[100];
		char destnumber[MAXNUMBERLEN+1];
		unsigned token;
		int bc;
		struct sockaddr_in addr, publicaddr, privateaddr;
	} tmpvpcdata;

	vpc = LockCall(vpcall);
	if(!vpc)
		TRETURN
_restartconnectthread:
	OUTMSG outmsg(DEFAULT_MTU, PKT_CONNECTREQ);
	strcpy(tmpvpcdata.address, vpc->address);
	tmpvpcdata.token = vpc->token;
	tmpvpcdata.bc = vpc->bc;
	if(vpc->addrtype == ADDRT_IPADDR)
	{
		vpc->Unlock();
		rc = Resolve(tmpvpcdata.address + 3, &tmpvpcdata.addr, VPHONEPORT);
		vpc = LockCall(vpcall);
		if(!rc)
		{
			if(vpc)
			{
				vpc->Unlock();
				NotifyOnCall(VPMSG_INVALIDADDR, vpcall, 0);
				NotifyOnCall(VPMSG_CONNECTFINISHED, vpcall, 0);
				NotifyOnCall(VPMSG_CALLENDED, vpcall, REASON_INVALIDNUMBER);
			}
			TRETURN
		}
		vpc->addr = tmpvpcdata.addr;
		Zero(vpc->privateaddr);
		Zero(vpc->publicaddr);
		vpc->sendcallrequests = false;
	} else {
		vpc->Unlock();
		if(vpc->useproxy)
			rc = vPResolve("vProxy", tmpvpcdata.destnumber, &tmpvpcdata.addr, &tmpvpcdata.publicaddr, &tmpvpcdata.privateaddr, tmpvpcdata.token, tmpvpcdata.bc, 0, settings.preferredcodec, settings.supportedcodecs);
		else rc = vPResolve(tmpvpcdata.address, tmpvpcdata.destnumber, &tmpvpcdata.addr, &tmpvpcdata.publicaddr, &tmpvpcdata.privateaddr, tmpvpcdata.token, tmpvpcdata.bc, 0, settings.preferredcodec, settings.supportedcodecs, vpcall);
		vpc = LockCall(vpcall);
		if(!vpc)
			TRETURN
		if(vpc->useproxy)
		{
			if(rc != RES_FOUND)
			{
				vpc->Unlock();
				NotifyOnCall(VPMSG_CONNECTTIMEOUT, vpcall, 0);
				NotifyOnCall(VPMSG_CONNECTFINISHED, vpcall, 0);
				NotifyOnCall(VPMSG_CALLENDED, vpcall, 0);
				TRETURN;
			}
			vpc->addr = tmpvpcdata.addr;

			OUTMSG outmsg(DEFAULT_MTU, PKT_PROXYBIND);
			outmsg.AddIE_String(IE_SERVERACCESSCODE, settings.serveraccesscode);
			outmsg.AddIE_InAddr(IE_INADDR_HOST, vpc->publicaddr.sin_addr.s_addr, vpc->publicaddr.sin_port);
			outmsg.AddIE_DWord(IE_TOKEN, vpc->token);
			outmsg.AddIE_Byte(IE_BEARER, vpc->bc);
			for(vpc->retry = 0; vpc->retry < 5; vpc->retry++)
			{
				vpc->starttm = GetTickCount();
				Send(&vpc->addr, &outmsg);
				vpc->Unlock();
				for(;;)
				{
					vpc = LockCall(vpcall);
					if(!vpc)
						TRETURN
					if(!(vpc->status == VPCS_CONNECTING && !vpc->proxybound && GetTickCount() - vpc->starttm < 1000))
						break;
					vpc->Unlock();
					Sleep(10);
				}
				if(vpc->addauthdata == 1)
				{
					outmsg.AddIE_Binary(IE_AUTHDATA, vpc->authdata, 16);
					outmsg.AddIE_Word(IE_CONNECTIONID, vpc->connectionid);
					vpc->addauthdata = 2;
					vpc->retry = -1;
				}
				if(vpc->status != VPCS_CONNECTING || vpc->proxybound)
					break;
			}
			if(vpc->status == VPCS_DISCONNECTING)
			{
				vpc->Unlock();
				NotifyOnCall(VPMSG_CONNECTFINISHED, vpcall, 0);
				NotifyOnCall(VPMSG_CALLENDED, vpcall, 0);
				TRETURN;
			} else if(!vpc->proxybound)
			{
				vpc->Unlock();
				NotifyOnCall(VPMSG_CONNECTTIMEOUT, vpcall, 0);
				NotifyOnCall(VPMSG_CONNECTFINISHED, vpcall, 0);
				NotifyOnCall(VPMSG_CALLENDED, vpcall, 0);
				TRETURN;
			}
		} else {
			if(rc <= RES_IDLE)
			{
				vpc->Unlock();
				NotifyOnCall(VPMSG_SERVERNOTRESPONDING, vpcall, 0);
				NotifyOnCall(VPMSG_CONNECTFINISHED, vpcall, 0);
				NotifyOnCall(VPMSG_CALLENDED, vpcall, REASON_SERVERNOTRESPONDING);
				TRETURN
			}
			if(rc == RES_NOTFOUND)
			{
				vpc->Unlock();
				NotifyOnCall(VPMSG_INVALIDADDR, vpcall, 0);
				NotifyOnCall(VPMSG_CONNECTFINISHED, vpcall, 0);
				NotifyOnCall(VPMSG_CALLENDED, vpcall, REASON_INVALIDNUMBER);
				TRETURN
			}
			if(rc == RES_OFFLINE)
				vpc->offline = true;
			vpc->addr = tmpvpcdata.addr;
			vpc->publicaddr = tmpvpcdata.publicaddr;
			vpc->privateaddr = tmpvpcdata.privateaddr;
			vpc->sendcallrequests = true;
			if(vpc->addrtype == ADDRT_VNAME || vpc->addrtype == ADDRT_VNUMBER)
				strcpy(vpc->destnumber, tmpvpcdata.destnumber);
			else if(vpc->addrtype == ADDRT_PSTN)
				strcpy(vpc->destnumber, tmpvpcdata.address);
		}
	}
	if(vpc->bc == BC_NOTHING)
	{
		vpc->Unlock();
		NotifyOnCall(VPMSG_CONNECTFINISHED, vpcall, 0);
		NotifyOnCall(VPMSG_CALLENDED, vpcall, REASON_NORMAL);
		TRETURN;
	}
	if(vpc->addr.sin_addr.s_addr == localhostaddr && vpc->addr.sin_port == htons(settings.port))
	{
		vpc->Unlock();
		NotifyOnCall(VPMSG_CONNECTFINISHED, vpcall, 0);
		NotifyOnCall(VPMSG_CALLENDED, vpcall, REASON_LOOPCALL);
		TRETURN;
	}
	// We have to unlock the call, because AssignConnectionId locks and we shouldn't have more than one lock
	vpc->Unlock();
	tmpvpcdata.addr = vpc->addr;
	if(!vpc->useproxy)
	{
		AssignConnectionId(vpcall, &tmpvpcdata.addr);
		NotifyOnCall(VPMSG_RESOLVED, vpcall, 0);
	}
	vpc = LockCall(vpcall);
	if(!vpc)
		TRETURN
	if(forceuseproxy)
	{
		if(!(vpc->useproxy || vpc->bc == BC_SMS || vpc->offline))
		{
			vpc->useproxy = true;
			goto _restartconnectthread;
		}
	}
	if(*settings.serveraccesscode)
		outmsg.AddIE_String(IE_SERVERACCESSCODE, settings.serveraccesscode);
	if(vpc->addrtype == ADDRT_VNAME)
		outmsg.AddIE_String(IE_DESTNAME, vpc->address);
	switch(vpc->bc)
	{
	case BC_VOICE:
		vpc->codec = DecideCodec(0);
		outmsg.AddIE_BC(vpc->bc, vpc->codec, 1, settings.supportedcodecs);
		break;
	case BC_AUDIOVIDEO:
		vpc->codec = DecideCodec(0);
		outmsg.AddIE_BC(vpc->bc, vpc->codec, 1, settings.supportedcodecs);
		outmsg.AddIE_Video(vpc->videofmt.fourcc, vpc->videofmt.width, vpc->videofmt.height);
		break;
	case BC_CHAT:
		outmsg.AddIE_2Bytes(IE_BEARER, BC_CHAT, TEXTENC_PLAIN);
		if(*vpc->message)
			outmsg.AddIE_String(IE_MESSAGE, vpc->message);
		break;
	case BC_SMS:
		outmsg.AddIE_DWord(IE_SMSTYPE, vpc->smstype);
		outmsg.AddIE_DWord(IE_SMSID, vpc->smsid);
		outmsg.AddIE_DWord(IE_SEQNUMBER, seqnumber++);
		outmsg.AddIE_Byte(IE_BEARER, BC_SMS);
		if(*vpc->message)
			outmsg.AddIE_String(IE_MESSAGE, vpc->message);
		else if(vpc->missedcallbc != 0xff)
			outmsg.AddIE_MissedCall(time(0), 1, vpc->missedcallbc);
		break;
	case BC_FILE:
		outmsg.AddIE_Byte(IE_BEARER, BC_FILE);
		outmsg.AddIE_String(IE_FILESLIST, vpc->message);
		outmsg.AddIE_DWord(IE_NFILES, vpc->nfiles);
		outmsg.AddIE_DWord(IE_NBYTES, vpc->nbytes);
		break;
	case BC_VIDEOMSG:
		outmsg.AddIE_3Bytes(IE_BEARER, BC_VIDEOMSG, vpc->codec, 1);
		outmsg.AddIE_DWord(IE_NFILES, vpc->nfiles);
		outmsg.AddIE_DWord(IE_NBYTES, vpc->nbytes);
		outmsg.AddIE_Video(vpc->videofmt.fourcc, vpc->videofmt.width, vpc->videofmt.height);
		break;
	}
	outmsg.AddIE_Word(IE_CONNECTIONID, vpc->connectionid);
	outmsg.AddIE_DWord(IE_TOKEN, vpc->token);
	if(*vpc->destnumber)
		outmsg.AddIE_String(IE_DESTNUMBER, vpc->destnumber);
	if(*vpc->subaddress)
		outmsg.AddIE_String(IE_DESTSUBADDRESS, vpc->subaddress);
	if(!settings.clir || vpc->bc == BC_SMS)
	{
		outmsg.AddIE_String(IE_SOURCENUMBER, vpc->ournumber);
		outmsg.AddIE_String(IE_SOURCENAME, vpc->ourname);
	}
	vpc->restartconnectthread = false;
	for(vpc->retry = 0; vpc->retry < 3; vpc->retry++)
	{
		if(vpc->retry && vpc->sendcallrequests && serveraddr.sin_addr.s_addr)
		{
			OUTMSG outmsg(DEFAULT_MTU, PKT_QUERYADDR);

			outmsg.AddIE_String(IE_SERVERACCESSCODE, settings.serveraccesscode);
			outmsg.AddIE_String(IE_SOURCENAME, settings.username);
			outmsg.AddIE_String(IE_DESTNUMBER, vpc->destnumber);
			outmsg.AddIE_DWord(IE_TOKEN, vpc->token);
			outmsg.AddIE_Byte(IE_CALLREQUEST, 1);
			outmsg.AddIE_Byte(IE_BEARER, vpc->bc);
			outmsg.AddIE_Byte(IE_OFFLINE, vpc->offline);
			Send(&serveraddr, &outmsg);
		}
		vpc->starttm = GetTickCount();
		Send(&vpc->addr, &outmsg);
		vpc->Unlock();
		for(;;)
		{
			vpc = LockCall(vpcall);
			if(!vpc)
				TRETURN
			if(!(vpc->status == VPCS_CONNECTING && GetTickCount() - vpc->starttm < 1000 ||
				(vpc->status == VPCS_CALLPROCEEDING || vpc->status == VPCS_ALERTED) && GetTickCount() - vpc->starttm < 60000))
				break;
			if(vpc->restartconnectthread)
				goto _restartconnectthread;
			vpc->Unlock();
			Sleep(10);
		}
		if(vpc->addauthdata == 1)
		{
			outmsg.AddIE_Binary(IE_AUTHDATA, vpc->authdata, 16);
			vpc->addauthdata = 2;
			vpc->retry = -1;
		}
		if(vpc->status != VPCS_CONNECTING && vpc->status != VPCS_CALLPROCEEDING && vpc->status != VPCS_ALERTED)
			break;
	}
	rc = 0;
	if(vpc->status == VPCS_CONNECTING)
	{
		if(vpc->useproxy || vpc->bc == BC_SMS || vpc->offline)
			NotifyOnCall(VPMSG_CONNECTTIMEOUT, vpcall, 0);
		else {
			vpc->useproxy = true;
			goto _restartconnectthread;
		}
	}
	if(vpc->status != VPCS_CONNECTED || vpc->bc == BC_SMS)
		rc = -1;
	vpc->Unlock();
	NotifyOnCall(VPMSG_CONNECTFINISHED, vpcall, 0);
	if(rc)
		NotifyOnCall(VPMSG_CALLENDED, vpcall, 0);
	TRETURN
}

void VPSTACK::IncomingCallThread(void *vpcall1)
{
	VPCALL vpcall = (VPCALL)vpcall1;
	VPCALLDATA *vpc;
	unsigned ticks = GetTickCount(), nextsendticks = ticks;
	int status;

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
			AcceptCall(vpcall, ANSWER_ACCEPT, REASON_NORMAL, 0);
			nextsendticks = GetTickCount() + 1000;
		}
		Sleep(10);
	}
	if(status != VPCS_CONNECTED)
	{
		NotifyOnCall(VPMSG_CALLSETUPFAILED, vpcall, 0);
		NotifyOnCall(VPMSG_CALLENDED, vpcall, 0);
	}
	TRETURN
}

void VPSTACK::NetworkQualityAudio(VPCALLDATA *vpc, WORD seqnumber, WORD ts, int pktlen)
{
	ts = (WORD)GetTickCount();
	if(!vpc->aqos.pkts)
	{
		memset(&vpc->aqos, 0, sizeof(vpc->aqos));
		vpc->aqos.seqnumber = seqnumber;
		vpc->aqos.ts = ts;
		vpc->aqos.mintsdiff = 0xffffffff;
		vpc->aqos.mintsdiff2 = 0xffffffff;
	}
	if(seqnumber == vpc->aqos.seqnumber)
	{
		if(vpc->aqos.pkts)
			vpc->aqos.duplicated++;
	} else if((WORD)(seqnumber - vpc->aqos.seqnumber) >= 0x8000)
	{
		vpc->aqos.outoforderpackets++;
		RateCount(&vpc->aqos.bad, 10000);
	} else {
		vpc->aqos.lostpackets += (WORD)(seqnumber - 1 - vpc->aqos.seqnumber);
		RateCount(&vpc->aqos.bad, 10000 * (WORD)(seqnumber - 1 - vpc->aqos.seqnumber));
		if((WORD)(ts - vpc->aqos.ts) > vpc->aqos.maxtsdiff)
			vpc->aqos.maxtsdiff = (WORD)(ts - vpc->aqos.ts);
		if((WORD)(ts - vpc->aqos.ts) * 1000U > vpc->aqos.maxtsdiff2)
			vpc->aqos.maxtsdiff2 = (WORD)(ts - vpc->aqos.ts) * 1000;
		if(vpc->aqos.pkts)
		{
			unsigned mean, diff;

			diff = (WORD)(ts - vpc->aqos.ts);
			if(diff < vpc->aqos.mintsdiff)
				vpc->aqos.mintsdiff = (WORD)(ts - vpc->aqos.ts);
			vpc->aqos.maxjitter = vpc->aqos.maxtsdiff - vpc->aqos.mintsdiff;

			if(diff * 1000U < vpc->aqos.mintsdiff2)
				vpc->aqos.mintsdiff2 = diff * 1000U;
			mean = (vpc->aqos.mintsdiff2 + vpc->aqos.maxtsdiff2) / 2;
			if(diff > 500U)
				diff = 500U;
			vpc->aqos.mintsdiff2 += (mean - vpc->aqos.mintsdiff2) * diff / 1000;
			vpc->aqos.maxtsdiff2 -= (vpc->aqos.maxtsdiff2 - mean) * diff / 1000;
			vpc->aqos.curjitter = (vpc->aqos.maxtsdiff2 - vpc->aqos.mintsdiff2) / 1000;
		}
	}
	RateCount(&vpc->aqos.xc, pktlen+UDPIPHLEN);
	vpc->aqos.kbps = (int)(vpc->aqos.xc.rate * XC_MULT);
	vpc->aqos.pkts++;
	vpc->aqos.seqnumber = seqnumber;
	vpc->aqos.ts = ts;

	RateCount(&vpc->aqos.good, 10000);
	RateCount(&vpc->aqos.bad, 0);
	if(vpc->aqos.good.rate < 5 * vpc->aqos.bad.rate)
		vpc->aqos.quality = 1;
	else if(vpc->aqos.good.rate < 20 * vpc->aqos.bad.rate)
		vpc->aqos.quality = 2;
	else if(vpc->aqos.good.rate < 100 * vpc->aqos.bad.rate)
		vpc->aqos.quality = 3;
	else if(!vpc->aqos.bad.rate)
	{
		if(vpc->aqos.curjitter < 60)
			vpc->aqos.quality = 5;
		else vpc->aqos.quality = 4;
	}	
}

int VPSTACK::GetAudioParameters(VPCALL vpcall, int *codec, int *framesperpacket)
{
	int rc = -1;

	VPCALLDATA *vpc = LockCall(vpcall);
	if(vpc)
	{
		*codec = vpc->codec;
		*framesperpacket = vpc->framesperpacket;
		rc = 0;
		vpc->Unlock();
	}
	return rc;
}

int VPSTACK::GetQosData(VPCALL vpcall, QOSDATA *qos)
{
	int rc = -1;

	VPCALLDATA *vpc = LockCall(vpcall);
	if(vpc)
	{
		if(vpc->bc == BC_VOICE || vpc->bc == BC_AUDIOVIDEO)
		{
			memcpy(qos, &vpc->aqos, sizeof(*qos));
			rc = 0;
		}
		vpc->Unlock();
	}
	return rc;
}

int VPSTACK::GetVQosData(VPCALL vpcall, QOSDATA *qos)
{
	int rc = -1;

	VPCALLDATA *vpc = LockCall(vpcall);
	if(vpc)
	{
		if(vpc->bc == BC_VIDEO || vpc->bc == BC_AUDIOVIDEO)
		{
			memcpy(qos, &vpc->vqos, sizeof(*qos));
			rc = 0;
		}
		vpc->Unlock();
	}
	return rc;
}

int VPSTACK::NetworkQuality()
{
	VPCALL vpcalls[MAXCALLS];
	int n = EnumCalls(vpcalls, MAXCALLS, (1<<BC_VOICE) | (1<<BC_AUDIOVIDEO));
	int i, qmin = -1;
	VPCALLDATA *vpc;

	for(i = 0; i < n; i++)
	{
		vpc = LockCall(vpcalls[i]);
		if(vpc)
		{
			if(qmin == -1 || (vpc->bc & (1<<BC_VOICE | 1<<BC_AUDIOVIDEO)) && (int)vpc->aqos.quality < qmin)
				qmin = vpc->aqos.quality;
			if(qmin == -1 || (vpc->bc & (1<<BC_VIDEO | 1<<BC_AUDIOVIDEO)) && (int)vpc->vqos.quality < qmin)
				qmin = vpc->vqos.quality;
			vpc->Unlock();
		}
	}
	return qmin;
}

void VPSTACK::ProcessAudioMessage(const struct sockaddr_in *addr, BYTE *buf, int buflen)
{
	VPCALLDATA *vpc;
	unsigned short connectionid;
	unsigned timestamp;
	VPCALL vpcall;

	if(buflen < 8 || *buf >= 0x20 || !(settings.supportedcodecs & (1 << *buf)))
		return;
	connectionid = (unsigned short)buf[1] << 8 | buf[2];
	vpc = FindCallOrDisconnect(addr, connectionid, 0xff);
	if(!vpc)
		return;
	timestamp = buf[5] << 8 | buf[6];
	if(timestamp < (vpc->rxtimestamp & 0xffff))
		vpc->rxtimestamp += 0x10000;
	vpc->rxtimestamp = (vpc->rxtimestamp & 0xffff0000) | timestamp;
	NetworkQualityAudio(vpc, (BYTE)buf[3] << 8 | (BYTE)buf[4], timestamp, buflen);
	if(vpc->held || !vpc->audio)
	{
		vpc->Unlock();
		return;
	}
	vpc->InUse();
	vpcall = vpc->vpcall;
	vpc->Unlock();
	vpc->audio->AudioFrame(*buf, vpc->rxtimestamp, buf + HEADERLEN, buflen - HEADERLEN);
	vpc = LockInUseCall(vpcall);
	if(vpc)
		vpc->Unlock();
}

void VPSTACK::CallEstablished(VPCALL vpcall)
{
	int bc, codec;
	unsigned fourcc, width, height, framerate, videoquality;
	VPAUDIODATA *audio;
	VPVIDEODATA *video;
	struct sockaddr_in addr;
	WORD connectionid;

	VPCALLDATA *vpc = LockCall(vpcall);
	if(!vpc)
		return;
	bc = vpc->bc;
	codec = vpc->codec;
	fourcc = vpc->videofmt.fourcc;
	width = vpc->videofmt.width;
	height = vpc->videofmt.height;
	framerate = vpc->videofmt.framerate;
	videoquality = vpc->videofmt.quality;
	connectionid = vpc->connectionid;
	addr = vpc->addr;
	vpc->start_ticks = GetTickCount();
	time(&vpc->start_time);
	vpc->Unlock();
	if(vpaudiodatafactory && (bc == BC_VOICE || bc == BC_AUDIOVIDEO))
	{
		audio = vpaudiodatafactory->New(this, vpcall, codec);
		if(audio)
		{
			vpc = LockCall(vpcall);
			if(!vpc)
			{
				delete audio;
				return;
			}
			vpc->audio = audio;
			vpc->Unlock();
		}
	}
	if(vpvideodatafactory && (bc == BC_VIDEO || bc == BC_AUDIOVIDEO))
	{
		SendMTUTest(connectionid, &addr);
		video = vpvideodatafactory->New(this, vpcall, fourcc, width, height, settings.fourcc, settings.width, settings.height, settings.framerate, settings.quality);
		if(video)
		{
			vpc = LockCall(vpcall);
			if(!vpc)
			{
				delete video;
				return;
			}
			if(vpc->vparam.set)
			{
				video->SetVideoWindowData(vpc->vparam.hParent, vpc->vparam.childid, &vpc->vparam.rect, vpc->vparam.hidden);
				vpc->vparam.set = false;
			}
			vpc->video = video;
			vpc->videopkt.buf = (BYTE *)malloc(MAXVIDEOFRAMELEN);
			vpc->videopkt.parity = (BYTE *)malloc(1500);
			vpc->Unlock();
		}
	}
}

/**********************************************************************/
/* Supplementary services                                             */
/**********************************************************************/

int VPSTACK::IsHeld(VPCALL vpcall)
{
	VPCALLDATA *vpc;
	int rc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	rc = (vpc->held ? 1 : 0) | (vpc->remotelyheld ? 2 : 0);
	vpc->Unlock();
	return rc;
}

int VPSTACK::IsConferenced(VPCALL vpcall)
{
	VPCALLDATA *vpc;
	int rc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	rc = (vpc->conferenced ? 1 : 0) | (vpc->remotelyconferenced ? 2 : 0);
	vpc->Unlock();
	return rc;
}

int VPSTACK::Hold(VPCALL vpcall)
{
	VPCALLDATA *vpc;
	int status;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	if(vpc->held)
	{
		vpc->Unlock();
		return 0;
	}
	vpc->held = true;
	OUTMSG outmsg(DEFAULT_MTU, PKT_CALLSTATUS);
	seqnumber = ++vpc->seqnumber;
	outmsg.AddIE_DWord(IE_SEQNUMBER, seqnumber);
	status = (vpc->held ? 1 : 0) | (vpc->conferenced ? 2 : 0);
	outmsg.AddIE_Byte(IE_CALLSTATUS, status);
	outmsg.AddIE_Word(IE_CONNECTIONID, vpc->connectionid);
	vpc->Unlock();
	SendPacketReliable(vpcall, &outmsg, seqnumber);
	return 0;
}

int VPSTACK::Resume(VPCALL vpcall)
{
	VPCALLDATA *vpc;
	int status;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	if(!vpc->held)
	{
		vpc->Unlock();
		return 0;
	}
	vpc->held = false;
	OUTMSG outmsg(DEFAULT_MTU, PKT_CALLSTATUS);
	seqnumber = ++vpc->seqnumber;
	outmsg.AddIE_DWord(IE_SEQNUMBER, seqnumber);
	status = (vpc->held ? 1 : 0) | (vpc->conferenced ? 2 : 0);
	outmsg.AddIE_Byte(IE_CALLSTATUS, status);
	outmsg.AddIE_Word(IE_CONNECTIONID, vpc->connectionid);
	vpc->Unlock();
	SendPacketReliable(vpcall, &outmsg, seqnumber);
	return 0;
}

int VPSTACK::ConferenceAll()
{
	VPCALL vpcalls[MAXVPARTIES];
	VPCALLDATA *vpc;
	int i, n, status;

	n = EnumCalls(vpcalls, MAXVPARTIES, (1<<BC_AUDIOVIDEO) | (1<<BC_VOICE));
	if(!n)
		return VPERR_INVALIDSTATUS;
	for(i = 0; i < n; i++)
	{
		vpc = LockCall(vpcalls[i]);
		if(!vpc)
			continue;
		if(vpc->conferenced)
		{
			vpc->Unlock();
			continue;
		}
		vpc->conferenced = true;
		OUTMSG outmsg(DEFAULT_MTU, PKT_CALLSTATUS);
		seqnumber = ++vpc->seqnumber;
		outmsg.AddIE_DWord(IE_SEQNUMBER, seqnumber);
		status = (vpc->held ? 1 : 0) | (vpc->conferenced ? 2 : 0);
		outmsg.AddIE_Byte(IE_CALLSTATUS, status);
		outmsg.AddIE_Word(IE_CONNECTIONID, vpc->connectionid);
		vpc->Unlock();
		SendPacketReliable(vpcalls[i], &outmsg, seqnumber);
	}
	ConferenceVideoCalls();
	return 0;
}

int VPSTACK::ConferenceVideoCalls()
{
	VPCALL vpcalls[MAXVPARTIES];
	VPCALL ccalls[MAXVPARTIES];
	int i, n, k = 0;

	n = EnumCalls(vpcalls, MAXVPARTIES, 1<<BC_AUDIOVIDEO);
	for(i = 0; i < n; i++)
		if(IsConferenced(vpcalls[i]) == 1)
			ccalls[k++] = vpcalls[i];
	if(k > 1)
		ConferenceCalls(ccalls, k, false);
	return 0;
}

int VPSTACK::StopConference()
{
	VPCALL vpcalls[MAXVPARTIES];
	VPCALLDATA *vpc;
	int i, n, status;
	struct sockaddr_in addr;

	n = EnumCalls(vpcalls, MAXVPARTIES, (1<<BC_AUDIOVIDEO) | (1<<BC_VOICE));
	if(!n)
		return VPERR_INVALIDSTATUS;
	for(i = 0; i < n; i++)
	{
		vpc = LockCall(vpcalls[i]);
		if(!vpc)
			continue;
		if(!vpc->conferenced)
		{
			vpc->Unlock();
			continue;
		}
		vpc->conferenced = false;
		OUTMSG outmsg2(DEFAULT_MTU, PKT_CONFERENCEEND);
		if(vpc->mconf.status != CONFSTATUS_IDLE)
		{
			outmsg2.AddIE_Byte(IE_BEARER, vpc->bc == BC_AUDIOVIDEO ? BC_VIDEO : vpc->bc);
			outmsg2.AddIE_DWord(IE_TOKEN, vpc->mconf.token);
			outmsg2.AddIE_Word(IE_CONNECTIONID, vpc->connectionid);
			addr = vpc->addr;
			vpc->mconf.status = CONFSTATUS_IDLE;
		} else addr.sin_addr.s_addr = 0;
		OUTMSG outmsg(DEFAULT_MTU, PKT_CALLSTATUS);
		seqnumber = ++vpc->seqnumber;
		outmsg.AddIE_DWord(IE_SEQNUMBER, seqnumber);
		status = (vpc->held ? 1 : 0) | (vpc->conferenced ? 2 : 0);
		outmsg.AddIE_Byte(IE_CALLSTATUS, status);
		outmsg.AddIE_Word(IE_CONNECTIONID, vpc->connectionid);
		vpc->Unlock();
		if(addr.sin_addr.s_addr)
			Send(&addr, &outmsg2);
		SendPacketReliable(vpcalls[i], &outmsg, seqnumber);
	}
	return 0;
}

int VPSTACK::AddToConference(VPCALL vpcall)
{
	VPCALLDATA *vpc;
	int status;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	if(vpc->conferenced)
	{
		vpc->Unlock();
		return 0;
	}
	vpc->conferenced = true;
	OUTMSG outmsg(DEFAULT_MTU, PKT_CALLSTATUS);
	seqnumber = ++vpc->seqnumber;
	outmsg.AddIE_DWord(IE_SEQNUMBER, seqnumber);
	status = (vpc->held ? 1 : 0) | (vpc->conferenced ? 2 : 0);
	outmsg.AddIE_Byte(IE_CALLSTATUS, status);
	outmsg.AddIE_Word(IE_CONNECTIONID, vpc->connectionid);
	vpc->Unlock();
	SendPacketReliable(vpcall, &outmsg, seqnumber);
	ConferenceVideoCalls();
	return 0;
}

int VPSTACK::RemoveFromConference(VPCALL vpcall)
{
	VPCALLDATA *vpc;
	int status, bc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	if(!vpc->conferenced)
	{
		vpc->Unlock();
		return 0;
	}
	bc = vpc->bc;
	vpc->conferenced = false;
	OUTMSG outmsg(DEFAULT_MTU, PKT_CALLSTATUS);
	seqnumber = ++vpc->seqnumber;
	outmsg.AddIE_DWord(IE_SEQNUMBER, seqnumber);
	status = (vpc->held ? 1 : 0) | (vpc->conferenced ? 2 : 0);
	outmsg.AddIE_Byte(IE_CALLSTATUS, status);
	outmsg.AddIE_Word(IE_CONNECTIONID, vpc->connectionid);
	vpc->Unlock();
	SendPacketReliable(vpcall, &outmsg, seqnumber);
	if(bc == BC_AUDIOVIDEO)
		DropFromConference(vpcall);
	return 0;
}

int VPSTACK::AcceptConference(VPCALL vpcall, bool accept)
{
	VPCALLDATA *vpc;
	struct sockaddr_in addr;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;

	if(vpc->sconf.status == CONFSLAVESTATUS_USERALERTED)
	{
		if(accept)
			vpc->sconf.status = CONFSLAVESTATUS_USERCONFIRMED;
		else vpc->sconf.status = CONFSLAVESTATUS_IDLE;
		OUTMSG outmsg(DEFAULT_MTU, PKT_CONFERENCEACK);
		outmsg.AddIE_Word(IE_CONNECTIONID, vpc->connectionid);
		outmsg.AddIE_Byte(IE_BEARER, vpc->bc == BC_AUDIOVIDEO ? BC_VIDEO : vpc->bc);
		outmsg.AddIE_DWord(IE_TOKEN, vpc->sconf.token);
		outmsg.AddIE_Byte(IE_ANSWER, accept ? ANSWER_ACCEPT : ANSWER_REFUSE);
		addr = vpc->sconf.masteraddr; 
		vpc->Unlock();
		Send(&addr, &outmsg);
		return 0;
	}
	return -1;
}

void VPSTACK::DoConferenceCallThread(void *vpcall1)
{
	VPCALL vpcall = (VPCALL)vpcall1;
	VPCALLDATA *vpc;
	int i, codec, cycle = 0;
	unsigned starttm;
	struct sockaddr_in addr, masteraddr;
	unsigned token;
	VPCALL vpcalls[MAXVPARTIES];

	Lprintf("DoConferenceThread");
	PrintCalls();
	vpc = LockCall(vpcall);
	if(!vpc)
		TRETURN;
	masteraddr = vpc->sconf.masteraddr;
	token = vpc->sconf.token;
	codec = DecideCodec(0);
	memcpy(vpcalls, vpc->sconf.vpcalls, sizeof(vpcalls));
restartconference:
	if(forceuseproxy && cycle == 0)
	{
		i = 0;
		goto proxyconference;
	}
	starttm = GetTickCount();
	while(GetTickCount() < starttm + 3000)
	{
		vpc->Unlock();
		for(i = 0; i < MAXVPARTIES; i++)
		{
			if(vpcalls[i])
			{
				vpc = LockCall(vpcalls[i]);
				if(vpc)
				{
					OUTMSG outmsg(DEFAULT_MTU, PKT_CONFERENCECALL);
					if(vpc->bc == BC_VOICE)
					{
						outmsg.AddIE_BC(BC_VOICE, codec, 1, settings.supportedcodecs);
						outmsg.AddIE_Byte(IE_CALLTRANSFER, 1);
					} else {
						outmsg.AddIE_Byte(IE_BEARER, BC_VIDEO);
						outmsg.AddIE_Video(vpc->videofmt.fourcc, vpc->videofmt.width, vpc->videofmt.height);
					}
					outmsg.AddIE_Word(IE_CONNECTIONID, vpc->connectionid);
					outmsg.AddIE_InAddr(IE_INADDR_HOST, vpc->sconf.masteraddr.sin_addr.s_addr, vpc->sconf.masteraddr.sin_port);
					outmsg.AddIE_DWord(IE_TOKEN, vpc->sconf.token);
					outmsg.AddIE_Byte(IE_CONFSETUPSTEP, 0);
					AddIE_Addresses(&outmsg);
					if(!settings.clir)
					{
						outmsg.AddIE_String(IE_SOURCENUMBER, vpc->ournumber);
						outmsg.AddIE_String(IE_SOURCENAME, vpc->ourname);
					}
					addr = vpc->addr;
					vpc->Unlock();
					Send(&addr, &outmsg);
				}
			}
		}
		Lprintf("DoConferenceThread proceeding");
		PrintCalls();
		Sleep(500);
		// Check if all have been acknowledged
		for(i = 0; i < MAXVPARTIES; i++)
			if(vpcalls[i])
			{
				vpc = LockCall(vpcalls[i]);
				if(vpc)
				{
					if(!vpc->sconf.step0acked)
					{
						vpc->Unlock();
						break;
					} else vpc->Unlock();
				}
			}
		vpc = LockCall(vpcall);
		if(!vpc)
		{
			CleanConferenceCalls(&masteraddr, token);
			Lprintf("DoConferenceThread terminated1");
			PrintCalls();
			TRETURN;
		}
		// If all have been acknowledged, exit loop
		if(i == MAXVPARTIES)
			break;
	}
proxyconference:
	if(i != MAXVPARTIES && cycle == 0)
	{
		struct sockaddr_in dummyaddr, proxyaddr;
		char dummydestnumber[MAXNUMBERLEN+1];
		int tmpbc =  BC_VIDEO, rc;
		unsigned tmptoken = GenerateRandomDWord() & 0xffff0000 | (unsigned short)(int)vpcall;

		cycle = 1;
		vpc->Unlock();
		rc = vPResolve("vProxy", dummydestnumber, &proxyaddr, &dummyaddr, &dummyaddr, tmptoken, tmpbc, 0, settings.preferredcodec, settings.supportedcodecs);
		vpc = LockCall(vpcall);
		if(!vpc)
		{
			CleanConferenceCalls(&masteraddr, token);
			Lprintf("DoConferenceThread terminated2");
			PrintCalls();
			TRETURN;
		}
		if(rc == RES_FOUND)
		{
			starttm = GetTickCount();
			while((int)(GetTickCount() - starttm) + 3000)
			{
				vpc->Unlock();
				for(i = 0; i < MAXVPARTIES; i++)
				{
					if(vpcalls[i])
					{
						vpc = LockCall(vpcalls[i]);
						if(vpc)
						{
							if(vpc->status != VPCS_CONNECTING || vpc->proxybound)
							{
								vpc->Unlock();
								continue;
							}
							vpc->addr = proxyaddr;
							vpc->useproxy = true;
							OUTMSG outmsg(DEFAULT_MTU, PKT_PROXYBIND);
							outmsg.AddIE_String(IE_SERVERACCESSCODE, settings.serveraccesscode);
							outmsg.AddIE_InAddr(IE_INADDR_HOST, vpc->publicaddr.sin_addr.s_addr, vpc->publicaddr.sin_port);
							outmsg.AddIE_DWord(IE_TOKEN, vpc->token);
							outmsg.AddIE_Byte(IE_BEARER, vpc->bc);
							if(vpc->addauthdata)
							{
								outmsg.AddIE_Binary(IE_AUTHDATA, vpc->authdata, 16);
								outmsg.AddIE_Word(IE_CONNECTIONID, vpc->connectionid);
							}
							vpc->Unlock();
							Send(&proxyaddr, &outmsg);
						}
					}
				}
				Sleep(500);
				// Check if all have been acknowledged
				for(i = 0; i < MAXVPARTIES; i++)
					if(vpcalls[i])
					{
						vpc = LockCall(vpcalls[i]);
						if(vpc)
						{
							if(vpc->status != VPCS_CONNECTED && !vpc->proxybound)
							{
								vpc->Unlock();
								break;
							} else vpc->Unlock();
						}
					}
				vpc = LockCall(vpcall);
				if(!vpc)
				{
					CleanConferenceCalls(&masteraddr, token);
					Lprintf("DoConferenceThread terminated3");
					PrintCalls();
					TRETURN;
				}
				// If all have been acknowledged, exit loop
				if(i == MAXVPARTIES)
					break;
			}
			// Check if we shoud retry connection
			vpc->Unlock();
			for(i = 0; i < MAXVPARTIES; i++)
				if(vpcalls[i])
				{
					vpc = LockCall(vpcalls[i]);
					if(vpc)
					{
						if(vpc->status != VPCS_CONNECTED && vpc->proxybound)
						{
							vpc->Unlock();
							break;
						} else vpc->Unlock();
					}
				}
			vpc = LockCall(vpcall);
			if(!vpc)
			{
				CleanConferenceCalls(&masteraddr, token);
				Lprintf("DoConferenceThread terminated4");
				PrintCalls();
				TRETURN;
			}
			if(i < MAXVPARTIES)
				goto restartconference;
		}
	}
	if(vpc->sconf.detach)
	{
		vpc->sconf.status = CONFSLAVESTATUS_IDLE;
		vpc->Unlock();
		if(vpcalls[0])
		{
			vpc = LockCall(vpcalls[0]);
			if(vpc)
			{
				bool notify;

				if(vpc->status == VPCS_CONNECTED)
					notify = true;
				else notify = false;
				vpc->Unlock();
				if(notify)
					NotifyOnCall(VPMSG_CALLTRANSFERRED, vpcall, (unsigned)vpcalls[0]);
			}
		}
		Disconnect(vpcall, REASON_CALLTRANSFERCOMPLETE);
	} else vpc->Unlock();
	CleanConferenceCalls(&masteraddr, token);
	Lprintf("DoConferenceThread terminated");
	PrintCalls();
	TRETURN;
}

void VPSTACK::ChatThread(void *vpcall1)
{
	VPCALL vpcall = (VPCALL)vpcall1;
	VPCALLDATA *vpc;

	while(sock)
	{
		vpc = LockCall(vpcall);
		if(!vpc)
			TRETURN;
		if(vpc->chatqueuelen)
		{
			OUTMSG outmsg(DEFAULT_MTU, PKT_CHAT);
			outmsg.AddIE_Word(IE_CONNECTIONID, vpc->connectionid);
			outmsg.AddIE_Byte(IE_BEARER, vpc->bc);
			outmsg.AddIE_DWord(IE_SEQNUMBER, vpc->chatqueue[0].seqnumber);
			outmsg.AddIE_Text(vpc->chatqueue[0].rtfencoding, vpc->chatqueue[0].rtfdata, vpc->chatqueue[0].rtfdatalen);
			Send(&vpc->addr, &outmsg);
			if(--vpc->chatretry == 0)
			{
				vpc->chatqueuelen--;
				memmove(vpc->chatqueue, vpc->chatqueue+1, vpc->chatqueuelen*sizeof(vpc->chatqueue[0]));
			}
		}
		if(!vpc->chatqueuelen)
		{
			vpc->Unlock();
			TRETURN;
		}
		vpc->Unlock();
		Sleep(500);
	}
	TRETURN
}

static int RTFCompress(const char *text, char *bin)
{
	int len = 0, i, maxlen, maxi;

	while(*text)
	{
		maxi = -1;
		maxlen = 0;
		for(i = 0; dictionary[i]; i++)
			if(!memcmp(text, dictionary[i], strlen(dictionary[i])))
			{
				if(strlen(dictionary[i]) > (unsigned)maxlen)
				{
					maxlen = strlen(dictionary[i]);
					maxi = i;
				}
			}
		if(maxi >= 0)
		{
			if(len + 2 > MAXTEXTLEN)
				return -1;
			*bin++ = 0;
			*bin++ = maxi;
			text += maxlen;
			len += 2;
		} else {
			if(len + 1 > MAXTEXTLEN)
				return -1;
			*bin++ = *text++;
			len++;
		}
	}
	return len;
}

int VPSTACK::SendChat(VPCALL vpcall, const char *text)
{
	VPCALLDATA *vpc;
	char bin[MAXTEXTLEN+1];
	bool spawnthread;
	int len;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	if(vpc->chatqueuelen == MAXCHATQUEUE)
	{
		vpc->Unlock();
		return VPERR_INVALIDSTATUS;
	}
	if(vpc->rtfencoding)
	{
		len = RTFCompress(text, bin);
		if(len == -1)
		{
			vpc->Unlock();
			return VPERR_INVALIDPARAMETER;
		}
		vpc->chatqueue[vpc->chatqueuelen].rtfdata = (char *)malloc(len);
		memcpy(vpc->chatqueue[vpc->chatqueuelen].rtfdata, bin, len);
	} else {
		len = strlen(text);
		vpc->chatqueue[vpc->chatqueuelen].rtfdata = (char *)malloc(len);
		memcpy(vpc->chatqueue[vpc->chatqueuelen].rtfdata, text, len);
	}
	vpc->chatqueue[vpc->chatqueuelen].rtfdatalen = len;
	vpc->chatqueue[vpc->chatqueuelen].rtfencoding = vpc->rtfencoding;
	vpc->chatqueue[vpc->chatqueuelen].seqnumber = ++vpc->seqnumber;
	vpc->chatqueuelen++;
	if(vpc->chatqueuelen == 1)
	{
		spawnthread = true;
		vpc->chatretry = 5;
	} else spawnthread = false;
	vpc->Unlock();	
	if(spawnthread)
	{
		TSTART
		beginclassthread((ClassThreadStart)&VPSTACK::ChatThread, this, vpcall);
	}
	return 0;
}

static char *RTFDecompress(char *bin, int len)
{
	char *text = (char *)malloc(8 * len + 1000);
	char *p = text;
	int i;

	for(i = 0; i < len; i++)
	{
		if(!bin[i])
		{
			if(dictionary[((unsigned char *)bin)[i + 1]])
			{
				strcpy(p, dictionary[((unsigned char *)bin)[i + 1]]);
				p += strlen(dictionary[((unsigned char *)bin)[i + 1]]);
			}
			i++;
		} else *p++ = bin[i];
	}
	*p = 0;
	return text;
}

int VPSTACK::GetChatText(VPCALL vpcall, char *text, int textsize)
{
	VPCALLDATA *vpc = LockCall(vpcall);
	int len;
	char *buf;

	textsize--;
	if(!vpc)
		return VPERR_INVALIDCALL;
	if(vpc->rtfencoding == TEXTENC_PLAIN)
	{
		len = vpc->rtfdatalen > textsize ? textsize : vpc->rtfdatalen;
		memcpy(text, vpc->rtfdata, len);
		text[len] = 0;
	} else {
		buf = RTFDecompress(vpc->rtfdata, vpc->rtfdatalen);
		len = strlen(buf);
		if(len > textsize)
			len = textsize;
		memcpy(text, buf, len);
		text[len] = 0;
		free(buf);
	}
	vpc->Unlock();
	return 0;
}

// File transfer routines

static unsigned crcTable[256];

static void crcgen()
{
	unsigned crc, poly;
	int	i, j;
	static char crctablegenerated;

	if(crctablegenerated)
		return;
	crctablegenerated = 1;
	poly = 0xEDB88320L;
	for (i=0; i<256; i++) {
		crc = i;
		for (j=8; j>0; j--) {
			if (crc&1) {
				crc = (crc >> 1) ^ poly;
			} else {
				crc >>= 1;
			}
		}
		crcTable[i] = crc;
	}
}

static unsigned filecrc(char *path, unsigned to)
{
	register unsigned crc;
	unsigned i, rc, term = 0, filelen = 0;
	unsigned char *buf = (unsigned char *)malloc(32768);
	int f;

	crcgen();
	f = openfile(path, O_RDONLY | O_BINARY);
	if(f == -1)
		return 0;
	crc = 0xFFFFFFFF;
	while(!term && (rc = readfile(f, buf, 32768)) > 0)
	{
		filelen += rc;
		if(filelen > to)
		{
			rc -= filelen - to;
			term = 1;
		}
		for(i = 0; i < rc; i++)
			crc = ((crc>>8) & 0x00FFFFFF) ^ crcTable[(crc^buf[i]) & 0xFF];
	}
	closefile(f);
	return crc^0xFFFFFFFF;
}

void VPSTACK::SendMTUTest(int connectionid, struct sockaddr_in *addr)
{
	BYTE msg[LONG_MTU];

	ZeroMemory(msg, sizeof(msg));
	msg[0] = PKT_MTUTEST;
	msg[1] = 3;
	msg[2] = IE_CONNECTIONID;
	msg[3] = (BYTE)(connectionid >> 8);
	msg[4] = (BYTE)connectionid;
	msg[5] = 0xf8 & ((LONG_MTU - 7) >> 8);
	msg[6] = (BYTE)(LONG_MTU - 7);
	msg[7] = IE_FILLER;
	Send(addr, msg, LONG_MTU);
}

int VPSTACK::SendFile(VPCALL vpcall, const char *path)
{
	VPCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	if(vpc->bc != BC_FILE && vpc->bc != BC_VIDEOMSG)
	{
		vpc->Unlock();
		return VPERR_UNSUPPORTEDBEARER;
	}
	if(vpc->ft.sending || vpc->ft.receiving)
	{
		vpc->Unlock();
		return VPERR_INVALIDSTATUS;
	}
	vpc->ft.pos = vpc->ft.length = 0;
	vpc->ft.accepted = 0;
	vpc->ft.sending = 1;
	strncpy2(vpc->ft.path, path, sizeof(vpc->ft.path));
	vpc->ft.length = filelengthpath(path);
	if(!vpc->ft.length)
	{
		vpc->Unlock();
		return VPERR_FILENOTFOUND;
	}
	vpc->Unlock();
	TSTART
	beginclassthread((ClassThreadStart)&VPSTACK::SendFileThread, this, vpcall);
	return 0;
}

void VPSTACK::SendFileThread(void *vpcall1)
{
	OUTMSG outmsg(DEFAULT_MTU, PKT_SENDFILEREQ);
	VPCALLDATA *vpc;
	int i, max;
	char *p;
	VPCALL vpcall = (VPCALL)vpcall1;
	struct sockaddr_in addr;
	bool timeout;
	unsigned short connectionid;

	vpc = LockCall(vpcall);
	if(!vpc)
		TRETURN
	addr = vpc->addr;
	connectionid = vpc->connectionid;
	vpc->Unlock();
	SendMTUTest(connectionid, &addr);
	vpc = LockCall(vpcall);
	if(!vpc)
		TRETURN;
	outmsg.AddIE_Word(IE_CONNECTIONID, vpc->connectionid);
	outmsg.AddIE_DWord(IE_FILELENGTH, vpc->ft.length);
	p = strstr(vpc->ft.path, "\\\\");
	if(!p)
		p = strstr(vpc->ft.path, "//");
	if(p)
		p += 2;
	else {
		p = strrchr(vpc->ft.path, '\\');
		if(!p)
			p = strrchr(vpc->ft.path, '/');
		if(p)
			p++;
		else p = vpc->ft.path;
	}
	outmsg.AddIE_String(IE_FILENAME, p);
	vpc->ft.accepted = 0;
	max = 500;	//FIX: 5 sec is very short if file replace
	// No, it's not short, because VPSTACK answers immediately 
	// with ANSWER_USERALERTED, ft.accepted changes to 1
	// and max becomes 0x7fffffff
	for(i = 0; i < max && vpc->ft.accepted < 2; i++)
	{
		if(vpc->ft.accepted == 0 && (i % 100) == 0)
		{
			vpc->Unlock();
			Send(&addr, &outmsg);
		} else vpc->Unlock();
		Sleep(10);
		vpc = LockCall(vpcall);
		if(!vpc)
			TRETURN
		if(vpc->ft.accepted == 1)
			max = 0x7fffffff;	// alerted received, wait indefinitely
	}
	if(vpc->ft.accepted == 2)
	{
		SendFile2(vpc);	// this routine returns with vpcall unlocked
		vpc = LockCall(vpcall);
		if(!vpc)
			TRETURN
	}
	if(vpc->ft.accepted != 3)
	{
		if(!vpc->ft.accepted)
			timeout = true;
		else timeout = false;
		vpc->Unlock();
		if(timeout)
		{
			Lprintf("Timeout for sending %s", vpc->ft.path);
			NotifyOnCall(VPMSG_FTTIMEOUT, vpcall, 0);
		}
		OUTMSG outmsg(DEFAULT_MTU, PKT_SENDFILEEND);
		outmsg.AddIE_Word(IE_CONNECTIONID, vpc->connectionid);
		outmsg.AddIE_Byte(IE_REASON, timeout ? REASON_TIMEOUT : REASON_NORMAL);
		vpc = LockCall(vpcall);
		if(!vpc)
			TRETURN
		vpc->ft.accepted = 0;
		for(i = 0; i < 500 && !vpc->ft.accepted; i++)
		{
			if(i % 100 == 0)
			{
				vpc->Unlock();
				Send(&addr, &outmsg);
			} else vpc->Unlock();
			Sleep(10);
			vpc = LockCall(vpcall);
			if(!vpc)
				TRETURN
		}
	}
	vpc->ft.sending = 0;
	vpc->Unlock();
	NotifyOnCall(VPMSG_FTTXCOMPLETE, vpcall, 0);
	TRETURN;
}

int VPSTACK::RxFile(VPCALLDATA *vpc, void *buf, int len)
{
	unsigned tm;

	if(!buf)
	{
		TerminateFTReceive(vpc);
		return 0;
	}
	if(vpc->ft.f)
	{
		writefile(vpc->ft.f, buf, len);
		vpc->ft.pos += len;
		if((tm = GetTickCount()) > vpc->ft.lastnotify + 100 || vpc->ft.length == vpc->ft.pos)
		{
			NotifyOnCall(VPMSG_FTPROGRESS, vpc->vpcall, 0);
			vpc->ft.lastnotify = tm;
		}
	}
	return 0;
}

int VPSTACK::AbortFileTransfer(VPCALL vpcall)
{
	VPCALLDATA *vpc;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	*vpc->ft.path = 0;
	vpc->ft.accepted = 3;
	if(vpc->ft.udpstream)
		vpc->ft.udpstream->Abort();
	vpc->Unlock();
	return 0;
}

int VPSTACK::TerminateFTReceive(VPCALLDATA *vpc)
{
	if(vpc->ft.udpstream)
	{
		delete (UDPSTREAMDATA *)vpc->ft.udpstream->senduser;
		delete vpc->ft.udpstream;
		vpc->ft.udpstream = 0;
	}
	if(vpc->ft.f)
	{
		closefile(vpc->ft.f);
		vpc->ft.f = 0;
	}
	vpc->ft.receiving = 0;
	if(vpc->ft.pos == vpc->ft.length)
	{
		NotifyOnCall(VPMSG_SENDFILESUCCESS, vpc->vpcall, 0);
		Lprintf("File received ok");
	} else {
		NotifyOnCall(VPMSG_SENDFILEFAILED, vpc->vpcall, 0);
		Lprintf("Reception failed");
	}
	return 0;
}

void VPSTACK::SendFile2(VPCALLDATA *vpc)
{
	int f, rc, rc2;
	char buf[3000];
	bool aborted = false;
	unsigned tm;
	VPCALL vpcall = vpc->vpcall;
	UDPStream *stream;
	UDPSTREAMDATA *sd;

	stream = vpc->ft.udpstream = new UDPStream;
	sd = new UDPSTREAMDATA;
	sd->vpstack = this;
	sd->vpcall = vpcall;
	sd->addr = vpc->addr;
	sd->connectionid = vpc->connectionid;
	vpc->ft.udpstream->Init(txsendrtn, sd, txrecvrtn, sd, vpc->mtu-4);

	f = openfile(vpc->ft.path, O_BINARY | O_RDONLY);
	if(f != -1)
	{
		vpc->ft.pos = vpc->ft.startpos;
		vpc->ft.starttm = GetTickCount();
		vpc->Unlock();
		NotifyOnCall(VPMSG_FTPROGRESS, vpcall, 0);
		vpc = LockCall(vpcall);
		if(!vpc)
			return;
		if(vpc->ft.pos)
		{
			seekfile(f, vpc->ft.pos, SEEK_SET);
			Lprintf("Sending file %s starting from position %d", vpc->ft.path, vpc->ft.pos);
		} else Lprintf("Sending file %s", vpc->ft.path);
		while((rc = readfile(f, buf, 3000)) > 0)
		{
			vpc->InUse();
			vpc->Unlock();
			rc2 = stream->Send(buf, rc);
			vpc = LockInUseCall(vpcall);
			if(!vpc)
				return;
			if(rc2)
			{
				aborted = true;
				Lprintf("Transmission failed");
				break;
			} else {
				vpc->ft.pos += rc;
				if((tm = GetTickCount()) > vpc->ft.lastnotify + 100)
				{
					vpc->Unlock();
					NotifyOnCall(VPMSG_FTPROGRESS, vpcall, 0);
					vpc = LockCall(vpcall);
					if(!vpc)
						return;
					vpc->ft.lastnotify = tm;
				}
			}
		}
		closefile(f);
		if(!aborted)
		{
			vpc->InUse();
			vpc->Unlock();
			if(stream->Send(0, 0))
				aborted = true;
			vpc = LockInUseCall(vpcall);
			if(!vpc)
				return;
		}
	} else {
		aborted = true;
		Lprintf("Error opening file");
	}
	delete (UDPSTREAMDATA *)vpc->ft.udpstream->senduser;
	delete vpc->ft.udpstream;
	vpc->ft.udpstream = 0;
	vpc->Unlock();
	NotifyOnCall(VPMSG_FTPROGRESS, vpcall, 0);
	if(aborted)
	{
		aborted = true;
		NotifyOnCall(VPMSG_SENDFILEFAILED, vpcall, 0);
		Lprintf("Transmission failed");
	} else {
		NotifyOnCall(VPMSG_SENDFILESUCCESS, vpcall, 0);
		Lprintf("File transmitted ok");
	}
}

int VPSTACK::AcceptFile(VPCALL vpcall, int accept)
{
	VPCALLDATA *vpc;
	OUTMSG outmsg(DEFAULT_MTU, PKT_SENDFILEACK);
	int rc;
	struct sockaddr_in addr;
	unsigned t;

	vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	if(!accept)
	{
		outmsg.AddIE_2Bytes(IE_ANSWER, ANSWER_REFUSE, REASON_NORMAL);
		outmsg.AddIE_Word(IE_CONNECTIONID, vpc->connectionid);
		Send(&vpc->addr, &outmsg);
		if(vpc->ft.udpstream)
		{
			delete (UDPSTREAMDATA *)vpc->ft.udpstream->senduser;
			delete vpc->ft.udpstream;
			vpc->ft.udpstream = 0;
		}
		vpc->ft.receiving = 0;
		vpc->Unlock();
		return 0;
	}
	vpc->ft.receiving = 2;
	vpc->ft.tmout = GetTickCount();
	outmsg.AddIE_Byte(IE_ANSWER, ANSWER_ACCEPT);
	outmsg.AddIE_Word(IE_CONNECTIONID, vpc->connectionid);
	outmsg.AddIE_Word(IE_MTU, vpc->mtu);
	outmsg.AddIE_DWord(IE_FILELENGTH, rc = filelengthpath(vpc->ft.path));
	if(rc)
		outmsg.AddIE_DWord(IE_FILECRC, filecrc(vpc->ft.path, 0xffffffff));
	addr = vpc->addr;
	vpc->Unlock();
	t = GetTickCount() - 1000;
	for(;;)
	{
		if(GetTickCount() - t >= 1000)
		{
			Send(&addr, &outmsg);
			t = GetTickCount();
		}
		vpc = LockCall(vpcall);
		if(!vpc)
			return VPERR_INVALIDCALL;
		if(vpc->ft.receiving == 3)
		{
			vpc->Unlock();
			break;
		}
		if(GetTickCount() - vpc->ft.tmout > 10000)
		{
			vpc->Unlock();
			return VPERR_TIMEOUT;
		}
		vpc->Unlock();
		Sleep(10);
	}
	return 0;
}

// Video routines

void VPSTACK::SetAudioDataFactory(VPAUDIODATAFACTORY *af)
{
	vpaudiodatafactory = af;
	if(af)
		settings.supportedbearers |= 1<<BC_VOICE;
	else settings.supportedbearers &= ~(1<<BC_VOICE);
}

void VPSTACK::SetVideoDataFactory(VPVIDEODATAFACTORY *vf)
{
	vpvideodatafactory = vf;
	if(vf)
		settings.supportedbearers |= 1<<BC_AUDIOVIDEO | 1<<BC_VIDEO;
	else settings.supportedbearers &= ~(1<<BC_AUDIOVIDEO | 1<<BC_VIDEO);
}

void VPSTACK::NetworkQualityVideo(VPCALLDATA *vpc, BYTE seqnumber, bool keyframe, BYTE fragment, WORD ts, int pktlen)
{
	ts = (WORD)GetTickCount();
	if(!vpc->vqos.pkts)
	{
		memset(&vpc->vqos, 0, sizeof(vpc->vqos));
		vpc->vqos.seqnumber = seqnumber;
		vpc->vqos.ts = ts;
		vpc->vqos.mintsdiff = 0xffffffff;
		vpc->vqos.mintsdiff2 = 0xffffffff;
	}
	if(seqnumber == vpc->vqos.seqnumber && fragment == vpc->vqos.fragment)
	{
		if(vpc->vqos.pkts)
			vpc->vqos.duplicated++;
	} else if((BYTE)(seqnumber - vpc->vqos.seqnumber) >= 0xc0 ||
		seqnumber == vpc->vqos.seqnumber && fragment < vpc->vqos.fragment)
	{
		vpc->vqos.outoforderpackets++;
		RateCount(&vpc->vqos.bad, 10000);
	}
	else {
		if(seqnumber == vpc->vqos.seqnumber && (fragment & 0x7f) != ((vpc->vqos.fragment + 1) & 0x7f))
		{
			if(vpc->vqos.pkts)
			{
				vpc->vqos.lostpackets += (fragment & 0x7f) - ((vpc->vqos.fragment + 1) & 0x7f);
				RateCount(&vpc->vqos.bad, 10000 * (fragment & 0x7f) - ((vpc->vqos.fragment + 1) & 0x7f));
				if(vpc->vqos.lostframe != seqnumber)
				{
					vpc->vqos.lostframes++;
					vpc->vqos.lostframe = seqnumber;
					if(keyframe)
						vpc->vqos.lostkeyframes++;
				}
			}
		} else if(seqnumber != vpc->vqos.seqnumber && (!(vpc->vqos.fragment & 0x80) || (fragment & 0x7f)))
		{
			vpc->vqos.lostframes += (BYTE)(seqnumber - vpc->vqos.seqnumber);
			vpc->vqos.lostframe = seqnumber;
			if(vpc->vqos.keyframe)
				vpc->vqos.lostkeyframes++;
			vpc->vqos.lostpackets += (fragment & 0x7f);
			RateCount(&vpc->vqos.bad, 10000 * (fragment & 0x7f));
		}
		if(fragment & 0x80)
		{
			if((WORD)(ts - vpc->vqos.ts) > vpc->vqos.maxtsdiff)
				vpc->vqos.maxtsdiff = (WORD)(ts - vpc->vqos.ts);
			if((WORD)(ts - vpc->vqos.ts) * 1000U > vpc->vqos.maxtsdiff2)
				vpc->vqos.maxtsdiff2 = (WORD)(ts - vpc->vqos.ts) * 1000;
			if(vpc->vqos.pkts)
			{
				unsigned mean, diff;

				diff = (WORD)(ts - vpc->vqos.ts);
				if(diff < vpc->vqos.mintsdiff)
					vpc->vqos.mintsdiff = diff;
				vpc->vqos.maxjitter = vpc->vqos.maxtsdiff - vpc->vqos.mintsdiff;

				if(diff * 1000U < vpc->vqos.mintsdiff2)
					vpc->vqos.mintsdiff2 = diff * 1000U;
				mean = (vpc->vqos.mintsdiff2 + vpc->vqos.maxtsdiff2) / 2;
				if(diff > 500U)
					diff = 500U;
				vpc->vqos.mintsdiff2 += (mean - vpc->vqos.mintsdiff2) * diff / 1000;
				vpc->vqos.maxtsdiff2 -= (vpc->vqos.maxtsdiff2 - mean) * diff / 1000;
				vpc->vqos.curjitter = (vpc->vqos.maxtsdiff2 - vpc->vqos.mintsdiff2) / 1000;
			}
		}
	}
	RateCount(&vpc->vqos.xc, pktlen+UDPIPHLEN);
	vpc->vqos.kbps = (int)(vpc->vqos.xc.rate * XC_MULT);
	vpc->vqos.pkts++;
	vpc->vqos.seqnumber = seqnumber;
	vpc->vqos.ts = ts;
	vpc->vqos.keyframe = keyframe;
	vpc->vqos.fragment = fragment;

	RateCount(&vpc->vqos.good, 10000);
	RateCount(&vpc->vqos.bad, 0);
	if(vpc->vqos.good.rate < 5 * vpc->vqos.bad.rate)
		vpc->vqos.quality = 1;
	else if(vpc->vqos.good.rate < 20 * vpc->vqos.bad.rate)
		vpc->vqos.quality = 2;
	else if(vpc->vqos.good.rate < 100 * vpc->vqos.bad.rate)
		vpc->vqos.quality = 3;
	else vpc->vqos.quality = 5;
}

void VPSTACK::ProcessVideoMessage(const struct sockaddr_in *addr, BYTE *buf, int buflen)
{
	WORD connectionid = buf[1] << 8 | buf[2];
	BYTE seqno = buf[3];
	bool keyframe = buf[4] & 1;
	BYTE fragment = buf[5];
	BYTE lastfragment;
	WORD timestamp = buf[6] << 8 | buf[7];
	VPCALLDATA *vpc;

	if(buflen < 9)
		return;
	vpc = FindCallOrDisconnect(addr, connectionid, 0xff);
	if(!vpc)
		return;
	if(!vpc->video)
	{
		vpc->Unlock();
		return;
	}
	if(*buf == PKT_VIDEOPARITY)
	{
		memcpy(vpc->videopkt.parity, buf, buflen);
		vpc->videopkt.paritylen = buflen;
		if(CheckDeliverVideoFrame(vpc))
			return;
		vpc->Unlock();
		return;
	}
	buflen -= 8;
	buf += 8;
	NetworkQualityVideo(vpc, seqno, keyframe, fragment, timestamp, buflen);
	lastfragment = fragment & 0x80;
	fragment &= 0x7f;
	if(!lastfragment)
		vpc->videopkt.fraglen = buflen;
	if(!fragment || vpc->videopkt.curseq != seqno)	// New frame
	{
		vpc->videopkt.curseq = seqno;
		vpc->videopkt.size = 0;
		vpc->videopkt.keyframe = keyframe;
		vpc->videopkt.timestamp = timestamp;
		vpc->videopkt.fragments = 0;
		memset(vpc->videopkt.map, 0, sizeof(vpc->videopkt.map));
	}
	if(buflen > vpc->videopkt.fraglen)
		vpc->videopkt.fraglen = 0;	// Invalidate fraglen, as it cannot be correct
	// If we know where to put it and it's inside boundaries, save fragment
	if((vpc->videopkt.fraglen || !fragment) && vpc->videopkt.fraglen * fragment + buflen <= MAXVIDEOFRAMELEN)
	{
		memcpy(vpc->videopkt.buf + vpc->videopkt.fraglen * fragment, buf, buflen);
		vpc->videopkt.map[fragment>>5] |= 1<<(fragment&0x1f);
		vpc->videopkt.fragments++;
	}
	// Try to determine frame size
	if(lastfragment)
	{
		if(!fragment)
			vpc->videopkt.size = buflen;
		else if(vpc->videopkt.fraglen)
			vpc->videopkt.size = vpc->videopkt.fraglen * fragment + buflen;
	}
	if(CheckDeliverVideoFrame(vpc))
		return;
	vpc->Unlock();
}

static void Xor(BYTE *dst, const BYTE *src, int len)
{
	int i;

	for(i = 0; i < len; i++)
		dst[i] ^= src[i];
}

int VPSTACK::CheckDeliverVideoFrame(VPCALLDATA *vpc)
{
	int n, i, len, cpylen;
	VPCALL vpcall;
	BYTE *ptr;

	if(!vpc->videopkt.fraglen && !vpc->videopkt.size)
		return 0;
	if(vpc->videopkt.fraglen)
	{
		// Check number of missing fragments
		if(!vpc->videopkt.size)
		{
			// We don't know size, check parity, if present
			if(!vpc->videopkt.paritylen || vpc->videopkt.parity[3] != vpc->videopkt.curseq)
				return 0;	// Nothing to do
			vpc->videopkt.size = vpc->videopkt.parity[4]<<8 | vpc->videopkt.parity[5];
		}
		n = (vpc->videopkt.size - 1) / vpc->videopkt.fraglen + 1;
		if(n == vpc->videopkt.fragments + 1 && vpc->videopkt.paritylen && vpc->videopkt.parity[3] == vpc->videopkt.curseq)
		{
			// One missing fragment, calculate through parity
			len = vpc->videopkt.fraglen;
			ptr = 0;
			cpylen = 0;
			for(i = 0; i < n; i++)
			{
				if(i == n-1)
					len = vpc->videopkt.size - vpc->videopkt.fraglen*(n-1);
				if(vpc->videopkt.map[i>>5] & (1 << (i & 0x1f)))
					Xor(vpc->videopkt.parity+8, vpc->videopkt.buf + vpc->videopkt.fraglen * i, len);
				else {
					ptr = vpc->videopkt.buf + vpc->videopkt.fraglen * i;
					cpylen = len;
				}
			}
			if(ptr)
			{
				vpc->videopkt.fragments++;
				memcpy(ptr, vpc->videopkt.parity+8, cpylen);
			}
		}
		if(n != vpc->videopkt.fragments)
			return 0;
	}
	vpc->vqos.goodframes++;
	if(vpc->videopkt.keyframe)
		vpc->vqos.keyframes++;
	vpcall = vpc->vpcall;
	vpc->InUse();
	vpc->Unlock();
	if(vpc->videopkt.keyframe)
		vpc->videopkt.firstkeyframe = true;
	if(vpc->videopkt.firstkeyframe)
		vpc->video->VideoFrame(vpc->videopkt.timestamp, vpc->videopkt.buf, vpc->videopkt.size, vpc->videopkt.keyframe ? true : false);
	vpc = LockInUseCall(vpcall);
	if(!vpc)
		return -1;
	vpc->videopkt.size = 0;
	vpc->videopkt.fragments = 0;
	memset(vpc->videopkt.map, 0, sizeof(vpc->videopkt.map));
	return 0;
}

int VPSTACK::SendVideo(VPCALL vpcall, WORD timestamp, void *data, int size, bool keyframe)
{
	VPCALLDATA *vpc;
	int frag, fraglen, tsize;
	BYTE msg[1500], parity[1500];
	BYTE *pdata;
	struct sockaddr_in addr;
	bool txparity = settings.txvideoparity;

	vpc = LockCall(vpcall);
	if(!vpc)
		return -1;
	if(vpc->status == VPCS_RECORDING && vpc->havi)
	{
		AVIWriteVideo(vpc->havi, data, size, keyframe);
		vpc->video->VideoFrame(timestamp, (const BYTE *)data, size, keyframe);
		vpc->Unlock();
		return 0;
	}
	if(!(vpc->status == VPCS_CONNECTED && vpc->cantx && (vpc->bc == BC_VIDEO || vpc->bc == BC_AUDIOVIDEO) && !vpc->held))
	{
		vpc->Unlock();
		return -1;
	}
	addr = vpc->addr;
	pdata = (BYTE *)data;
	tsize = size;
	fraglen = vpc->mtu - 8;
	if(tsize <= 3*fraglen)	// Only transmit parity if split to 4 or more packets
		txparity = false;
	msg[0] = PKT_VIDEO;
	msg[1] = (BYTE)(vpc->connectionid >> 8);
	msg[2] = (BYTE)vpc->connectionid;
	msg[3] = vpc->videoseq;
	msg[4] = keyframe ? 1 : 0;
	msg[6] = (BYTE)(timestamp >> 8);
	msg[7] = (BYTE)timestamp;
	vpc->videoseq++;
	vpc->Unlock();
	if(txparity)
		memset(parity, 0, sizeof(parity));
	for(frag = 0;; frag++)
	{
		msg[5] = (BYTE)(frag & 0x7f);
		if(tsize > fraglen)
		{
			if(txparity)
				Xor(parity+8, pdata, fraglen);
			memcpy(msg + 8, pdata, fraglen);
			tsize -= fraglen;
			pdata += fraglen;
			if(!Send(&addr, msg, 8 + fraglen))
				break;
		} else {
			memcpy(msg + 8, pdata, tsize);
			msg[5] |= 0x80;
			Send(&addr, msg, 8 + tsize);
			if(txparity)
			{
				Xor(parity+8, pdata, tsize);
				memcpy(parity, msg, 8);
				parity[0] = PKT_VIDEOPARITY;
				parity[4] = size>>8;
				parity[5] = size;
				Send(&addr, parity, 8 + fraglen);
			}
			break;
		}
	}
	return 0;
}


int VPSTACK::GetTransceiverBandwidths(int *rx, int *tx)
{
	*rx = (int)(rxxc.rate * XC_MULT);
	*tx = (int)(this->tx.xc.rate * XC_MULT);
	return 0;
}

int VPSTACK::AskOnline(AOL *aol)
{
	if(aoldata.working)
		aoldata.stop = true;
	while(aoldata.working)
		Sleep(10);
	aoldata.working = true;
	aoldata.stop = false;
	if(aol->notificationreqop == NOTIFICATIONREQ_DISABLE)
		AOLThread(aol);
	else {
		TSTART
		beginclassthread((ClassThreadStart)&VPSTACK::AOLThread, this, aol);
	}
	return 0;
}

void VPSTACK::AOLThread(void *aol)
{
	AOL *a = (AOL *)aol;
	int base = 0, i = 0, n;
	aoldata.srvid = a->nums[0].srvid;

	aoldata.operation = a->notificationreqop;
	aoldata.seqnumber = 1;
	while(i < a->N && !aoldata.stop)
	{
		while(i < a->N && a->nums[i].srvid == aoldata.srvid && i - base < 130 && !aoldata.stop)
		{
			aoldata.numbers[i - base] = a->nums[i].num;
			i++;
		}
		if(aoldata.srvid && serveraddr.sin_addr.s_addr)
		{
			aoldata.N = i - base;
			nencodednumbers = 0;
			queryonlineack = false;
			for(n = 0; n < 100 && !aoldata.stop; n++)
			{
				if(!(n % 10))
				{
					OUTMSG outmsg(DEFAULT_MTU, PKT_QUERYONLINE);
					outmsg.AddIE_String(IE_SERVERACCESSCODE, settings.serveraccesscode);
					outmsg.AddIE_DWord(IE_SERVERID, aoldata.srvid);
					outmsg.AddIE_DWord(IE_SEQNUMBER, aoldata.seqnumber);
					outmsg.AddIE_Byte(IE_OPERATION, aoldata.operation);
					outmsg.AddIE_EncodedNumbers(aoldata.numbers, aoldata.N);
					Send(&serveraddr, &outmsg);
				}
				if(a->notificationreqop == 1)
					break;
				Sleep(50);
				if(queryonlineack)
					break;
			}
			if(nencodednumbers)
			{
				for(n = 0; n < aoldata.N; n++)
				{
					if((encodednumbers[n] & 0xffffff) == a->nums[base + n].num)
						a->nums[base + n].online = (encodednumbers[n] >> 24) + 1;
				}
			}
			if(a->notificationreqop != 1)
				Notify(VPMSG_QUERYONLINEACK, 1, (unsigned int)aol);
		}
		base = i;
		if(i < a->N)
			aoldata.srvid = a->nums[i].srvid;
		aoldata.seqnumber++;
	}
	if(!aoldata.stop)
		Notify(VPMSG_QUERYONLINEACK, 0, (unsigned int)aol);
	aoldata.working = false;
	TRETURN;
}

int VPSTACK::QueryAccountInfo()
{
	OUTMSG outmsg(DEFAULT_MTU, PKT_SENDACCOUNTINFO);
	outmsg.AddIE_String(IE_SERVERACCESSCODE, settings.serveraccesscode);
	Send(&serveraddr, &outmsg);
	return 0;
}

static int SendTCPPacket(SOCKET sk, OUTMSG *msg)
{
	char buf[1500];
	int len = msg->ptr - msg->msg;

	buf[0] = (char)(len >> 8);
	buf[1] = (char)(len & 0xff);
	memcpy(buf + 2, msg->msg, len);
	if(send(sk, buf, 2 + len, 0) != 2 + len)
		return -1;
//	printf("TCPTX %d bytes OK\n", 2 + len);
	return 0;
}

static int recvtmout(SOCKET s, void *buf, int len)
{
	fd_set fs;
	struct timeval tv;
	int rc;

	while(len)
	{
		FD_ZERO(&fs);
		FD_SET(s, &fs);
		tv.tv_sec = 30;
		tv.tv_usec = 0;
		rc = select(s + 1, &fs, 0, 0, &tv);
		if(rc < 0)
			return -1;
		if(!rc)
			return -1;
		if((rc = recv(s, (char *)buf, len, 0)) <= 0)
			return -1;
		buf = (char *)buf + rc;
		len -= rc;
	}
	return 0;
}

static int RecvTCPPacket(SOCKET sk, BYTE *buf, IE_ELEMENTS *ie)
{
	int len;

	if(recvtmout(sk, buf, 2))
		return -1;
	len = buf[0] << 8 | buf[1];
	if(len > 1498)
		return -1;
	if(recvtmout(sk, buf + 1, len))
		return -1;
	return GetIE_Elements(buf, len + 1, ie);
}

struct TCPFT_DATA {
	int op;
	char path[MAX_PATH], vname[MAXNAMELEN+1];
	bool amactive;
	unsigned param, flags;
} *data;

int VPSTACK::UserSearch(const char *name, const char *country, unsigned flags, unsigned param)
{
	TCPFT_DATA *data;

	data = new TCPFT_DATA;
	data->op = 10000;
	strncpy2(data->path, name, sizeof(data->path));
	if(country)
		strncpy2(data->vname, country, 3);
	else *data->vname = 0;
	data->param = param;
	data->flags = flags;
	TSTART
	beginclassthread((ClassThreadStart)&VPSTACK::ServerTCPTransfer, this, data);
	return 0;
}

int VPSTACK::ServerTransfer(int op, const char *path, bool amactive, const char *vname)
{
	filetime ft;
	static filetime nullft;
	TCPFT_DATA *data;

	if(op < 0 || op > TCPFT_TXFILE)
		return -1;
	if(op == TCPFT_TXGREETING)
	{
		if(lastamactive == (int)amactive)
		{
			ft = getfilewritetime(path);
			if(memcmp(&ft, &nullft, sizeof(ft)) && !memcmp(&ft, &lastgreetingsent, sizeof(ft)))
				return -2;
		} else lastamactive = amactive;
	}
	data = new TCPFT_DATA;
	data->op = op;
	strncpy2(data->path, path, sizeof(data->path));
	if(vname)
		strncpy2(data->vname, vname, sizeof(data->vname));
	else *data->vname = 0;
	data->amactive = amactive;
	TSTART
	beginclassthread((ClassThreadStart)&VPSTACK::ServerTCPTransfer, this, data);
	return 0;
}

void VPSTACK::ServerTCPTransfer(void *data1)
{
	struct sockaddr_in addr;
	char path[MAX_PATH];
	char resolvednumber[MAXNUMBERLEN+1], *searchresult = 0;
	BYTE buf[1500];
	void *recvbuf;
	IE_ELEMENTS ie;
	int n, rc, f, op, error = -1;
	TCPFT_DATA data;
	OUTMSG outmsg(DEFAULT_MTU, PKT_CONNECTREQ);

	data = *(TCPFT_DATA *)data1;
	delete (TCPFT_DATA *)data1;
	op = data.op;
	Lprintf("DownloadVoiceMessages %d", op);
	while(dvm.working)
		if(!sock) 
			TRETURN
		else Sleep(100);
	Lprintf("DownloadVoiceMessages %d running", op);
	InterlockedIncrement(&dvm.working);
	while(!serveraddr.sin_addr.s_addr || dvm.sk != INVALID_SOCKET)
		if(!sock)
		{
			InterlockedDecrement(&dvm.working);
			if(op == 10000)	// UserSearch
				NotifyRc(VPMSG_USERSEARCH, data.param, (unsigned)"");
			else Notify(VPMSG_SERVERTRANSFERFINISHED, op, -2);
			TRETURN
		} else Sleep(1000);
	if(op >= TCPFT_RXCONTACTS && op <= TCPFT_TXPICT)
		rc = vPResolve("vPdata-contacts-tcp", resolvednumber, &addr, 0, 0, GenerateRandomDWord(), BC_VOICE, 0, settings.preferredcodec, settings.supportedcodecs);
	else rc = vPResolve("vPdata-voicebox-tcp", resolvednumber, &addr, 0, 0, GenerateRandomDWord(), BC_VOICE, 0, settings.preferredcodec, settings.supportedcodecs);
	if(rc != RES_OFFLINE && rc != RES_FOUND)
	{
		Lprintf("DownloadVoiceMessages %d finished, server not resolved", op);
		InterlockedDecrement(&dvm.working);
		if(op == 10000)
			NotifyRc(VPMSG_USERSEARCH, data.param, (unsigned)"");
		else Notify(VPMSG_SERVERTRANSFERFINISHED, op, -3);
		TRETURN
	}
	dvm.sk = socket(AF_INET, SOCK_STREAM, 0);
	if(dvm.sk == INVALID_SOCKET)
	{
		InterlockedDecrement(&dvm.working);
		if(op == 10000)
			NotifyRc(VPMSG_USERSEARCH, data.param, (unsigned)"");
		else Notify(VPMSG_SERVERTRANSFERFINISHED, op, -4);
		TRETURN
	}
	if(connect(dvm.sk, (struct sockaddr *)&addr, sizeof(addr)))
		goto dvm_end;
	outmsg.AddIE_String(IE_SERVERACCESSCODE, settings.serveraccesscode);
	if(SendTCPPacket(dvm.sk, &outmsg))
		goto dvm_end;
	if(RecvTCPPacket(dvm.sk, buf, &ie))
		goto dvm_end;
	if(ie.answer == ANSWER_REFUSE && ie.reason == REASON_AUTHERROR && ie.authdata)
	{
		aes_ctx acx;

		aes_dec_key(settings.accountsecret, 16, &acx);
		aes_dec_blk(ie.authdata, ie.authdata, &acx);
		outmsg.AddIE_Binary(IE_AUTHDATA, ie.authdata, 16);
		if(SendTCPPacket(dvm.sk, &outmsg))
			goto dvm_end;
		if(RecvTCPPacket(dvm.sk, buf, &ie))
			goto dvm_end;
	}
	if(ie.answer != ANSWER_ACCEPT)
		goto dvm_end;
	if(op == TCPFT_TXGREETING || op == TCPFT_TXAB || op == TCPFT_TXMYDETAILS || op == TCPFT_TXCONTACTS || op == TCPFT_TXPICT || op == TCPFT_TXFILE)
	{
		f = openfile(data.path, O_RDONLY | O_BINARY);
		if(f != -1)
		{
			n = getfilesize(f);
			outmsg.Reset(PKT_SENDFILEREQ);
			outmsg.AddIE_DWord(IE_FILELENGTH, n);
			outmsg.AddIE_Byte(IE_OPERATION, data.amactive ? 2 : 1);
			switch(op)
			{
			case TCPFT_TXGREETING:
				outmsg.AddIE_String(IE_FILENAME, "Announce.wav");
				break;
			case TCPFT_TXAB:
				outmsg.AddIE_String(IE_FILENAME, "Pab.db");
				break;
			case TCPFT_TXMYDETAILS:
				outmsg.AddIE_String(IE_FILENAME, "MyDetails.db");
				break;
			case TCPFT_TXCONTACTS:
				outmsg.AddIE_String(IE_FILENAME, "Contacts.db");
				break;
			case TCPFT_TXPICT:
				outmsg.AddIE_String(IE_FILENAME, "VPicture");
				if(*data.vname)
					outmsg.AddIE_String(IE_SOURCENAME, data.vname);
				break;
			case TCPFT_TXFILE:
				{
					char fname[MAX_PATH+1];

					*fname = '$';
					char *p = strrchr(data.path, '\\');
					if(p)
						strncpy2(fname+1, p+1, MAX_PATH);
					else strncpy2(fname+1, data.path, MAX_PATH);
					outmsg.AddIE_String(IE_FILENAME, fname);
				}
				if(*data.vname)
					outmsg.AddIE_String(IE_SOURCENAME, data.vname);
				break;
			}
			if(SendTCPPacket(dvm.sk, &outmsg))
			{
				closefile(f);
				Lprintf("Error sedning %s to vPData", data.path);
				goto dvm_end;
			}
			if(!RecvTCPPacket(dvm.sk, buf, &ie) && ie.answer == ANSWER_ACCEPT)
			{
				recvbuf = malloc(32000);
				error = 0;
				while((n = readfile(f, recvbuf, 32000)) > 0)
					if(send(dvm.sk, (char *)recvbuf, n, 0) != (int)n)
					{
						error = -1;
						break;
					}
				closefile(f);
				free(recvbuf);
				if(op == TCPFT_TXGREETING)
					lastgreetingsent = getfilewritetime(data.path);
			} else closefile(f);
		}
	} else if(op == TCPFT_RXAB || op == TCPFT_RXMYDETAILS || op == TCPFT_RXCONTACTS ||
		op == TCPFT_RXPICT || op == TCPFT_RXVPICT || op == TCPFT_RXFILE || op == 10000)
	{
		outmsg.Reset(PKT_RECVFILEREQ);
		switch(op)
		{
		case TCPFT_RXAB:
			outmsg.AddIE_String(IE_FILENAME, "Pab.db");
			break;
		case TCPFT_RXMYDETAILS:
			outmsg.AddIE_String(IE_FILENAME, "MyDetails.db");
			break;
		case TCPFT_RXCONTACTS:
			outmsg.AddIE_String(IE_FILENAME, "Contacts.db");
			break;
		case TCPFT_RXPICT:
			outmsg.AddIE_String(IE_FILENAME, "Picture");
			if(*data.vname)
				outmsg.AddIE_String(IE_SOURCENAME, data.vname);
			break;
		case TCPFT_RXVPICT:
			outmsg.AddIE_String(IE_FILENAME, "VPicture");
			if(*data.vname)
				outmsg.AddIE_String(IE_SOURCENAME, data.vname);
			break;
		case TCPFT_RXFILE:
			{
				char fname[MAX_PATH+1];

				*fname = '$';
				strncpy2(fname+1, data.vname, MAX_PATH);
				outmsg.AddIE_String(IE_FILENAME, fname);
			}
			break;
		case 10000:
			outmsg.AddIE_String(IE_FILENAME, "VSearch");
			if(data.flags & USERSEARCH_VNAME)
				outmsg.AddIE_String(IE_SOURCENAME, data.path);
			if(data.flags & USERSEARCH_VNUMBER)
				outmsg.AddIE_String(IE_SOURCENUMBER, data.path);
			if(data.flags & USERSEARCH_FIRSTNAME)
				outmsg.AddIE_String(IE_FIRSTNAME, data.path);
			if(data.flags & USERSEARCH_LASTNAME)
				outmsg.AddIE_String(IE_LASTNAME, data.path);
			if(data.flags & USERSEARCH_EMAIL)
				outmsg.AddIE_String(IE_EMAIL, data.path);
			if(data.flags & USERSEARCH_PHONENUMBER)
				outmsg.AddIE_String(IE_PHONENUMBER, data.path);
			if(*data.vname)
				outmsg.AddIE_String(IE_COUNTRYID, data.vname);
			break;
		}
		if(SendTCPPacket(dvm.sk, &outmsg))
			goto dvm_end;
		while(!RecvTCPPacket(dvm.sk, buf, &ie))
		{
			if(ie.answer == ANSWER_ACCEPT)// && (!stricmp(ie.filename, "Pab.db") || !stricmp(ie.filename, "MyDetails.db")))
			{
				if(op == 10000)
				{
					if(ie.filelength < 1000000)
					{
						searchresult = (char *)malloc(ie.filelength + 1);
						if(searchresult)
						{
							if(recvtmout(dvm.sk, searchresult, ie.filelength))
							{
								*searchresult = 0;
								goto dvm_end;
							} else {
								error = 0;
								searchresult[ie.filelength] = 0;
							}
						}
					}
				} else {
					f = openfile(data.path, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC);
					if(f == -1)
						break;
					recvbuf = malloc(32000);
					while(ie.filelength)
					{
						n = ie.filelength > 32000 ? 32000 : ie.filelength;
						if(recvtmout(dvm.sk, recvbuf, n))
						{
							free(recvbuf);
							closefile(f);
							deletefile(path);
							goto dvm_end;
						}
						if(writefile(f, recvbuf, n) != n)
						{
							free(recvbuf);
							closefile(f);
							deletefile(path);
							goto dvm_end;
						}
						ie.filelength -= n;
					}
					free(recvbuf);
					closefile(f);
					error = 0;
				}
			} else break;
		}
	} else { // op == TCPFT_RXMSG
		outmsg.Reset(PKT_RECVFILEREQ);
		if(SendTCPPacket(dvm.sk, &outmsg))
			goto dvm_end;
		while(!RecvTCPPacket(dvm.sk, buf, &ie))
		{
			if(ie.answer == ANSWER_ACCEPT && ie.timestamp)
			{
				struct tm *tm = localtime((time_t *)&ie.timestamp);
				createdirectory(data.path);
				sprintf(path, "%s\\%04d-%02d-%02d _ %02d-%02d-%02d _ %s.wav",
					data.path, tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, ie.srcnum);
				f = openfile(path, O_WRONLY | O_BINARY | O_TRUNC | O_CREAT);
				if(f == -1)
					break;
				recvbuf = malloc(32000);
				while(ie.filelength)
				{
					n = ie.filelength > 32000 ? 32000 : ie.filelength;
					if(recvtmout(dvm.sk, recvbuf, n))
					{
						free(recvbuf);
						closefile(f);
						goto dvm_end;
					}
					if(writefile(f, recvbuf, n) != n)
					{
						free(recvbuf);
						closefile(f);
						goto dvm_end;
					}
					ie.filelength -= n;
				}
				free(recvbuf);
				closefile(f);
				NotifyRc(VPMSG_NEWVOICEMSG, (int)ie.srcnum, (int)ie.srcname);
				error = 0;
			} else break;
		}
	}
dvm_end:
	if(dvm.sk != INVALID_SOCKET)
		closesocket(dvm.sk);
	dvm.sk = INVALID_SOCKET;
	InterlockedDecrement(&dvm.working);
	Lprintf("DownloadVoiceMessages %d finished, error=%d", op, error);
	if(op == 10000)
		NotifyRc(VPMSG_USERSEARCH, data.param, (unsigned)(searchresult ? searchresult : ""));
	else Notify(VPMSG_SERVERTRANSFERFINISHED, (int)op, error);
	if(searchresult)
		free(searchresult);
	TRETURN
}

struct CFPARAM
{
	CFPARAM(int op, const char *number)
	{
		this->op = op;
		if(number)
			strncpy2(this->number, number, MAXNUMBERLEN+1);
		else *this->number = 0;
	}
	int op;
	char number[MAXNUMBERLEN+1];
};

int VPSTACK::CallForwardingRequest(int op, const char *number)
{
	if(!loggedon)
		return VPERR_NOTLOGGEDON;
	if(cfinprogress)
		return VPERR_INVALIDSTATUS;
	if(op < OPERATION_ASK || op > OPERATION_ACTIVATE_ONOFFLINE)
		return VPERR_INVALIDPARAMETER;
	cfinprogress = true;
	TSTART
	beginclassthread((ClassThreadStart)&VPSTACK::CallForwardingThread, this, new CFPARAM(op, number));
	return 0;
}

int VPSTACK::GetCallForwardingStatus(int *op, char *number)
{
	if(settings.cfop == OPERATION_ASK)
		return VPERR_INVALIDSTATUS;
	*op = settings.cfop;
	strcpy(number, settings.cfnumber);
	return 0;
}

void VPSTACK::CallForwardingThread(void *param)
{
	CFPARAM *cfp = (CFPARAM *)param;
	int i;

	for(i = 0; i < 10 && cfinprogress; i++)
	{
		OUTMSG outmsg(DEFAULT_MTU, PKT_CALLFORWARDING);
		outmsg.AddIE_Byte(IE_OPERATION, cfp->op);
		outmsg.AddIE_String(IE_SERVERACCESSCODE, settings.serveraccesscode);
		if(*cfp->number)
			outmsg.AddIE_String(IE_DESTNUMBER, cfp->number);
		Send(&serveraddr, &outmsg);
		Sleep(500);
	}
	cfinprogress = false;
	delete cfp;
	TRETURN
}

void VPSTACK::RecordAVIStartThread(void *dummy)
{
	VPCALLDATA *vpc;
	VPAUDIODATA *audio;
	VPVIDEODATA *video;

	audio = vpaudiodatafactory->New(this, recordvpcall, settings.preferredcodec);
	video = vpvideodatafactory->New(this, recordvpcall, settings.fourcc, settings.width, settings.height, settings.fourcc, settings.width, settings.height, settings.framerate, settings.quality);
	vpc = LockCall(recordvpcall);
	if(!vpc)
	{
		if(vpc->video)
			delete vpc->video;
		if(vpc->audio)
			delete vpc->audio;
		return;
	}
	if(vpc->vparam.set)
	{
		video->SetVideoWindowData(vpc->vparam.hParent, vpc->vparam.childid, &vpc->vparam.rect, vpc->vparam.hidden);
		vpc->vparam.set = false;
	}
	vpc->status = VPCS_RECORDING;
	vpc->video = video;
	vpc->audio = audio;
	vpc->Unlock();
}

int VPSTACK::RecordAVIFile(const char *path)
{
	VPCALLDATA *vpc;
	void *havi;
	AVIDATA ad;

	if(recordvpcall)
		StopAVIRecord();
	recordvpcall = CreateCall();
	if(!recordvpcall)
		return VPERR_NOMORECALLSAVAILABLE;
	ad.fourcc = settings.fourcc;
	ad.width = settings.width;
	ad.height = settings.height;
	ad.framelen = 1000000 / settings.framerate;
	Codec2AD(settings.preferredcodec, &ad);
	havi = AVICreate(path, &ad);
	vpc = LockCall(recordvpcall);
	if(!vpc)
		return VPERR_INVALIDSTATUS;
	vpc->havi = havi;
	vpc->bc = BC_AUDIOVIDEO;
	vpc->Unlock();
	beginclassthread((ClassThreadStart)&VPSTACK::RecordAVIStartThread, this, 0);
	return 0;
}

int VPSTACK::StopAVIRecord()
{
	if(!recordvpcall)
		return VPERR_INVALIDSTATUS;
	FreeCall(recordvpcall);
	recordvpcall = 0;
	return 0;
}

int VPSTACK::RecordWAVFile(const char *path)
{
	VPCALLDATA *vpc;
	VPAUDIODATA *audio;
	void *hwav;
	BYTE wf[1000];

	if(recordvpcall)
		StopWAVRecord();
	recordvpcall = CreateCall();
	if(!recordvpcall)
		return VPERR_NOMORECALLSAVAILABLE;
	audio = vpaudiodatafactory->New(this, recordvpcall, settings.preferredcodec);
	Codec2WF(CODEC_GSM, (WAVEFMT *)wf);
	hwav = WAVCreate(path, (WAVEFMT *)wf);
	if(audio)
	{
		vpc = LockCall(recordvpcall);
		if(!vpc)
		{
			if(audio)
				delete audio;
			return VPERR_INVALIDSTATUS;
		}
		vpc->status = VPCS_RECORDING;
		vpc->hwav = hwav;
		vpc->audio = audio;
		vpc->bc = BC_VOICE;
		vpc->Unlock();
	}
	return 0;
}

int VPSTACK::StopWAVRecord()
{
	return StopAVIRecord();
}

int VPSTACK::GetRecorderCall(VPCALL *vpcall)
{
	if(recordvpcall)
	{
		*vpcall = recordvpcall;
		return 0;
	}
	return VPERR_INVALIDSTATUS;
}

int VPSTACK::SetVideoWindowFullScreen(VPCALL vpcall, int fs)
{
	int rc;

	VPCALLDATA *vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	if(!vpc->video)
	{
		vpc->Unlock();
		return VPERR_INVALIDSTATUS;
	}
	vpc->InUse();
	vpc->Unlock();
	rc = vpc->video->SetFullScreen(fs);
	vpc = LockInUseCall(vpcall);
	if(vpc)
		vpc->Unlock();
	if(rc)
		return VPERR_INVALIDSTATUS;
	return 0;
}

int VPSTACK::SetVideoWindowData(VPCALL vpcall, HWINDOW hParent, int childid, const RECTANGLE *rect, bool hidden)
{
	VPCALLDATA *vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	if(!vpc->video)
	{
		vpc->vparam.hParent = hParent;
		vpc->vparam.childid = childid;
		vpc->vparam.rect = *rect;
		vpc->vparam.hidden = hidden;
		vpc->vparam.set = true;
		vpc->Unlock();
		return 0;
	}
	vpc->InUse();
	vpc->Unlock();
	vpc->video->SetVideoWindowData(hParent, childid, rect, hidden);
	vpc = LockInUseCall(vpcall);
	if(vpc)
		vpc->Unlock();
	return 0;
}

int VPSTACK::MeasureBandwidth()
{
	if(measuringbandwidth)
		return -1;
	measuringbandwidth = true;
	TSTART
	beginclassthread((ClassThreadStart)&VPSTACK::MeasureBandwidthThread, this, 0);
	return 0;
}

void VPSTACK::MeasureBandwidthThread(void *dummy)
{
	static char buf[10000];
	char resolvednumber[MAXNUMBERLEN+1];
	SOCKET sk;
	struct sockaddr_in addr;
	int i, rc;
	unsigned t, bits = 0;

	rc = vPResolve("dumbserver", resolvednumber, &addr, 0, 0, GenerateRandomDWord(), BC_VOICE, 0, settings.preferredcodec, settings.supportedcodecs);
	if(rc != RES_OFFLINE && rc != RES_FOUND)
	{
		Lprintf("MeasureBandwidth finished, server not resolved");
		Notify(VPMSG_MEASUREBANDWIDTH, 0, 0);
		measuringbandwidth = false;
		TRETURN
	}
	Lprintf("MeasureBandwidth, connecting to %s:%d", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
	sk = socket(AF_INET, SOCK_STREAM, 0);
	if(connect(sk, (struct sockaddr *)&addr, sizeof(addr)))
	{
		Lprintf("MeasureBandwidth, cannot connect to %s:%d", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
		closesocket(sk);
		Notify(VPMSG_MEASUREBANDWIDTH, 0, 0);
		measuringbandwidth = false;
		TRETURN
	}
	for(i = 0; i < 10000; i++)
		buf[i] = (char)rand();
	t = GetTickCount();
	while(send(sk, buf, 10000, 0) == 10000 && GetTickCount() - t <= 5000)
		bits += 80000;
	closesocket(sk);
	t = GetTickCount() - t;
	Sleep(500);
	TSTART
	NATDiscoverThread(0);
	if(t)
	{
		Lprintf("MeasureBandwidth finished, bits=%d,t=%d, %d kbps", bits, t, bits/t);
		Notify(VPMSG_MEASUREBANDWIDTH, 0, bits/t);
	} else {
		Lprintf("MeasureBandwidth finished, time elapsed 0");
		Notify(VPMSG_MEASUREBANDWIDTH, 0, 0);
	}
	measuringbandwidth = false;
	TRETURN
}

void VPSTACK::NATDiscoverThread(void *dummy)
{
	struct sockaddr_in addr;
	char natserverslist2[MAXMESSAGELEN+1], *server1, *server2;

	if(nat.discoverrunning)
		TRETURN
	memset(&nat, 0, sizeof(nat));
	strcpy(natserverslist2, natserverslist);
	server1 = strtok(natserverslist2, ";");
	if(!server1)
		TRETURN
	server2 = strtok(0, ";");
	if(!server2)
		TRETURN
	nat.discoverrunning = true;
	OUTMSG outmsg(500, PKT_NATDISCOVER);
	nat.fullcone[0] = nat.restricted[0] = nat.received[0] = nat.fullcone[1] = nat.restricted[1] = nat.received[1] = false;
	outmsg.AddIE_DWord(IE_TOKEN, nat.token = GetTickCount());
	outmsg.AddIE_DWord(IE_SEQNUMBER, 1);
	if(!Resolve(server1, &addr, 11675))
	{
		Lprintf("NAT discover, cannot resolve %s", server1);
		nat.discoverrunning = false;
		TRETURN
	}
	Send(&addr, &outmsg);
	Sleep(200);
	if(!sock)
	{
		nat.discoverrunning = false;
		TRETURN
	}
	Send(&addr, &outmsg);
	Sleep(2000);
	if(!sock)
	{
		nat.discoverrunning = false;
		TRETURN
	}

	outmsg.Reset(PKT_NATDISCOVER);
	outmsg.AddIE_DWord(IE_TOKEN, nat.token = GetTickCount());
	outmsg.AddIE_DWord(IE_SEQNUMBER, 2);
	if(!Resolve(server2, &addr, 11675))
	{
		Lprintf("NAT discover, cannot resolve %s", server2);
		nat.discoverrunning = false;
		TRETURN
	}
	Send(&addr, &outmsg);
	Sleep(200);
	if(!sock)
	{
		nat.discoverrunning = false;
		TRETURN
	}
	Send(&addr, &outmsg);
	Sleep(2000);
	if(!sock)
	{
		nat.discoverrunning = false;
		TRETURN
	}
	if(!nat.received[0] && !nat.received[1])
	{
		nat.type = NAT_FIREWALL;
	} else if(nat.nonat[0] && nat.nonat[1])
	{
		if(nat.fullcone[0] && nat.fullcone[1] || nat.restricted[0] && nat.restricted[1])
			nat.type = NAT_NONAT;
		else nat.type = NAT_NONAT_PACKETLOSS;
	} else if(nat.received[0] && nat.received[1] && !nat.nonat[0] && !nat.nonat[1])
	{
		if(nat.fullcone[0] && nat.fullcone[1])
			nat.type = NAT_FULLCONE;
		else if(nat.restricted[0] && nat.restricted[1])
			nat.type = NAT_RESTRICTEDCONE;
		else if(!nat.fullcone[0] && !nat.fullcone[1] && !nat.restricted[0] && !nat.restricted[1])
		{
			if(nat.addrs[0].sin_addr.s_addr == nat.addrs[1].sin_addr.s_addr && nat.addrs[0].sin_port == nat.addrs[1].sin_port)
				nat.type = NAT_PORTRESTRICTEDCONE;
			else nat.type = NAT_SYMMETRIC;
		} else nat.type = NAT_INCONSISTENT;
	} else nat.type = NAT_INCONSISTENT;
	Lprintf("NAT discover finished, type=%d", nat.type);
	Notify(VPMSG_NATTYPE, 0, nat.type);
	nat.discoverrunning = false;
	TRETURN
}

int VPSTACK::SetNATServer(const char *addr)
{
	strncpy2(natserverslist, addr, sizeof(natserverslist));
	return 0;
}

int VPSTACK::GetCallTimes(VPCALL vpcall, time_t *start_time, unsigned int *length_ms)
{
	VPCALLDATA *vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	*start_time = vpc->start_time;
	*length_ms = GetTickCount() - vpc->start_ticks;
	vpc->Unlock();
	return 0;
}

int VPSTACK::CaptureVideoImage(VPCALL vpcall, void **image, int *w, int *h)
{
	int rc;

	VPCALLDATA *vpc = LockCall(vpcall);
	if(!vpc)
		return VPERR_INVALIDCALL;
	if(vpc->video)
		rc = vpc->video->CaptureImage(image, w, h);
	else rc = VPERR_INVALIDSTATUS;
	vpc->Unlock();
	return rc;
}

int VPSTACK::SetAPNsToken(const void *token, int tokensize)
{
	if(token && (tokensize <= 0 || tokensize >= 256))
		return VPERR_INVALIDPARAMETER;
	if(apntoken)
	{
		free(apntoken);
		apntoken = 0;
	}
	if(!token)
		return 0;
	apntoken = malloc(tokensize);
	apntokensize = tokensize;
	memcpy(apntoken, token, tokensize);
	return 0;
}

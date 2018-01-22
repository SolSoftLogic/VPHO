#include "util.h"

typedef void *VPCALL;

class GSMCALLDATA
{
public:
	GSMCALLDATA() { Zero(*this); InitializeCriticalSection(&cs); }
	~GSMCALLDATA() { DeleteCriticalSection(&cs); }
	void LockL(int line);
	void UnlockL(int line);
	void InUse() { inuse++; }

	// Main statuses
	VPCALL vpcall;
	BYTE status;
	time_t start_time;
	unsigned start_ticks;

	// Audio
	VPAUDIODATA *audio;

	// Party data
	char destnumber[MAXNUMBERLEN+1];
	char ourname[MAXNAMELEN+1], ournumber[MAXNUMBERLEN+1];
	char connectedname[MAXNAMELEN+1], connectednumber[MAXNUMBERLEN+1];

	// Auto attendant
	struct {
		int f;
		unsigned sendinterval, lastsendtm, readlen, msgid;
	} aa;

private:
	CRITICAL_SECTION cs;
};

struct GSMSTACKSETTINGS {
	unsigned supported_codecs;
	int inuse;
	bool deleting;
	friend class GSMSTACK;
};

class GSMSTACK : public IVPSTACK {
public:
	GSMSTACK(VPAUDIODATAFACTORY *vpaf = 0, const char *stackname = 0);
	~GSMSTACK();
	int Init();
	int SetNotifyRoutine(void (*notify)(void *param, unsigned, unsigned, unsigned), void *param);
	int SetSyncNotifyRoutine(int (*notify)(void *param, unsigned, unsigned, unsigned), void *param);
	// Create a call
	VPCALL CreateCall();
	// Free a call
	void FreeCall(VPCALL vpcall);
	// Attempt a connection with address, on success returns a VPCALL
	int Connect(VPCALL *vpcall, const char *address, int bc);
	// Attempt a connection with an already allocated call
	int Connect(VPCALL vpcall, const char *address, int bc);
	// Set the destination address of a call
	int SetCallAddress(VPCALL vpcall, const char *address);
	// Set the source of the call
	int SetCallLocalName(VPCALL vpcall, const char *username);
	int SetCallLocalNumber(VPCALL vpcall, const char *number);
	// Disconnect the call
	int Disconnect(VPCALL vpcall, int reason);
	// Answer a call in alerted state
	int AnswerCall(VPCALL vpcall);
	// Supplementary services
	int Hold(VPCALL vpcall);
	int Resume(VPCALL vpcall);
	int ConferenceCalls(VPCALL *vpcalls, int ncalls, bool detach);
	// Setup
	void SetSupportedBearersMask(unsigned bc_mask) {}

	// Misc
	int SendKeypad(VPCALL vpcall, int key);
	int GetStackType() { return STACKTYPE_GSM; }
	int EnumCalls(VPCALL *vpcalls, unsigned maxvpcalls, unsigned mask);

	int GetCallRemoteName(VPCALL vpcall, char *username);
	int GetCallRemoteNumber(VPCALL vpcall, char *number);
	int GetCallRemoteAddress(VPCALL vpcall, char *address) { *address = 0; return VPERR_UNSUPPORTEDCALL; }
	int GetCallRemoteSubAddr(VPCALL vpcall, char *subaddr) { *subaddr = 0; return VPERR_UNSUPPORTEDCALL; }
	int GetCallLocalName(VPCALL vpcall, char *username);
	int GetCallLocalNumber(VPCALL vpcall, char *number);
	int GetCallLocalSubAddr(VPCALL vpcall, char *subaddr) { *subaddr = 0; return VPERR_UNSUPPORTEDCALL; }
	int GetCallBearer(VPCALL vpcall) { return BC_VOICE; }
	int GetCallCodec(VPCALL vpcall);
	int GetCallTimes(VPCALL vpcall, time_t *start_time, unsigned *length_ms);
	int IsHeld(VPCALL vpcall);
	int GetCallStatus(VPCALL vpcall);

	int	SendAudio(VPCALL vpcall, int codec, unsigned timestamp, const void *buf, int len, bool wait = false);

protected:
	virtual void Notify(int message, unsigned param1, unsigned param2);
	virtual int NotifyRc(int message, unsigned param1, unsigned param2);

	// Calls
	GSMCALLDATA *LockCallL(VPCALL vpcall, int line);
	GSMCALLDATA *LockCallL(int index, int line);

	// Audio
	virtual void CallEstablished(GSMCALLDATA *vpc);
	virtual void CallEnded(GSMCALLDATA *vpc);

	// Misc
	long nthreads;
	int ncalls;
	VPAUDIODATAFACTORY *vpaudiodatafactory;
	
	struct {
		int codec;
	} settings;

private:
	RWLOCK lock;
	void ReadLockL(int line);
	void ReadUnlockL(int line);
	void WriteLockL(int line);
	void WriteUnlockL(int line);
	int nw, nr;
	GSMCALLDATA *calls[MAXCALLS];
	unsigned lastvpcallvalue;

	void (*NotifyRoutine)(void *param, unsigned message, unsigned param1, unsigned param2);
	void *NotifyParam;
};

#include "util.h"

#define MAXCALLS 4
typedef void *VPCALL;
#define AUDIOBUFSIZE (2*1280)

class FIFOBUFFER {
public:
	FIFOBUFFER() { head = tail = 0; }
	int PutData(const unsigned char *data, unsigned len);
	int GetData(unsigned char *data, unsigned len);
protected:
	unsigned head, tail;
	unsigned char buf[AUDIOBUFSIZE];
};

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
	int bc;

	// Audio
	VPAUDIODATA *audio;
	FIFOBUFFER audiobuf;
	unsigned rxtimestamp;

	// Party data
	char destnumber[MAXNUMBERLEN+1];
	char ourname[MAXNAMELEN+1], ournumber[MAXNUMBERLEN+1];
	char connectedname[MAXNAMELEN+1], connectednumber[MAXNUMBERLEN+1];
	char pdu[500];
	char message[161];
	int smsref, report;

	// Auto attendant
	struct {
		int f;
		unsigned sendinterval, lastsendtm, readlen, msgid;
	} aa;

private:
	CRITICAL_SECTION cs;
	int inuse;
	bool deleting;
	friend class GSMSTACK;
};

struct GSMSTACKSETTINGS {
	unsigned supported_codecs;
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

	// Initialization
	int SetServers(const char *list);	// list must contain one or two com ports (the second for audio)

	// Setup
	void SetSupportedBearersMask(unsigned bc_mask) {}
	int Logon(const char *username, const char *password);

	// Misc
	int SendKeypad(VPCALL vpcall, int key);
	int GetStackType() { return STACKTYPE_GSM; }
	int EnumCalls(VPCALL *vpcalls, unsigned maxvpcalls, unsigned mask);
	int NetworkQuality();
	int SignalLevel(int *dBm);
	int GetOperatorName(char *name, int namesize, unsigned *flags);

	int GetCallText(VPCALL vpcall, char *text);
	int SetCallText(VPCALL vpcall, const char *text);
	int GetCallRemoteName(VPCALL vpcall, char *username);
	int GetCallRemoteNumber(VPCALL vpcall, char *number);
	int GetCallRemoteAddress(VPCALL vpcall, char *address) { *address = 0; return VPERR_UNSUPPORTEDCALL; }
	int GetCallRemoteSubAddr(VPCALL vpcall, char *subaddr) { *subaddr = 0; return VPERR_UNSUPPORTEDCALL; }
	int GetCallLocalName(VPCALL vpcall, char *username);
	int GetCallLocalNumber(VPCALL vpcall, char *number);
	int GetCallLocalSubAddr(VPCALL vpcall, char *subaddr) { *subaddr = 0; return VPERR_UNSUPPORTEDCALL; }
	int GetCallBearer(VPCALL vpcall);
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
	GSMCALLDATA *LockInUseCall(VPCALL vpcall);
	GSMCALLDATA *LockCallL(int index, int line);
	VPCALL vpcallvoice, vpcallsms;

	// Audio
	virtual void CallEstablished(GSMCALLDATA *vpc);
	int GSMAudio(BYTE *data, int datalen);

	// Misc
	long nthreads;
	VPAUDIODATAFACTORY *vpaudiodatafactory;
	char oper[100];
	int signal;
	bool network3g;
	bool canrun;
	int atprotocol;
	
	struct {
		int codec;
		char pin[MAXNUMBERLEN+1];
		char portslist[100];
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
	void *hCmdPort, *hAudioPort;

	void (*NotifyRoutine)(void *param, unsigned message, unsigned param1, unsigned param2);
	void *NotifyParam;
	int ReopenPort(const char *portslist, int index);
	void cmdportworkerthread(void *dummy);
	void audioworkerthread(void *dummy);
};

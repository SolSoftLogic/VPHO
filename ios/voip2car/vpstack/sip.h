#include "util.h"
#include "netutil.h"
#include "portability.h"

#define MAXCALLS 100

#define MAXSIPADDRLEN 500
#define MAXTAGLEN 50
#define MAXBRANCHLEN 100
#define MAXCALLIDLEN 100
#define MAXOPAQUELEN 100
#define MAXNONCELEN 64
#define MAXREALMLEN 50
#define MAXCONTACTLEN 200
#define MAXROUTELEN 100
#define MAXREASONLEN 80
#define MAXVIA 10

#define DEFAULT_MTU 548

//	Rate counter
#define XC_ALPHA 0.8f
#define XC_MULT (1.0f - XC_ALPHA) * 1000.0f / XC_INTERVAL
#define XC_INTERVAL 100

typedef void *VPCALL;
void MD5(const char *s, char *digest);

enum {QOP_NONE, QOP_AUTH, QOP_AUTHINT};
enum HOLD {HOLD_NONE, HOLD_SENDONLY, HOLD_RECVONLY, HOLD_INACTIVE};

struct SIP_ELEMENTS
{
	SIP_ELEMENTS() { memset(this, 0, sizeof(SIP_ELEMENTS)); }
	int command;
	char *destaddr;
	char *fromname, *fromaddr, *fromtag;
	char *toname, *toaddr, *totag;
	char *contact, *route[MAXVIA];
	char *referto, *referredby;
	char *viaaddr[MAXVIA], *viabranch[MAXVIA];
	int viarport[MAXVIA];
	char *callid;
	char *username, *realm, *nonce, *response, *opaque, *cnonce, *reasonline, authtype;
	char answer, reason, qop;
	char *sipreasontext;
	int sipreason;
	enum HOLD hold;
	unsigned expire;
	unsigned transactionid;
	unsigned nc;
	unsigned seqnumber;
	unsigned char keypadmap, iLBC20map, iLBC30map;
	unsigned char codec;
	unsigned bc_codecs;
	unsigned sessionid;
	unsigned sessionversion;
	BYTE fulllocaladdr[6];
	struct sockaddr_in rtpaddr;
};

enum CONNECTTYPE {CT_REGULAR, CT_REGISTER, CT_HOLDRESUME, CT_REFER};

class SIPCALLDATA
{
public:
	SIPCALLDATA() { Zero(*this); InitializeCriticalSection(&cs); }
	~SIPCALLDATA() { DeleteCriticalSection(&cs); }
	void LockL(int line);
	void UnlockL(int line);

	// Main statuses
	VPCALL vpcall;
	BYTE status;
	bool cantx, earlyaudio;

	// Addresses
	struct sockaddr_in addr;

	// Audio
	int framesperpacket, codec;
	int iLBC20map, iLBC30map, keypadmap;
	int peak;
	unsigned short audioseqno;
	unsigned timestamp;
	unsigned lastsyncticks;
	unsigned ts;
	unsigned ssrc;
	VPAUDIODATA *audio;
	bool audiolocked, destroyaudio, raddrwrong;

	// Misc
	CONNECTTYPE connecttype;
	unsigned awaketm;
	int addrtype, reason;
	bool restartconnect;
	bool hold;			// Hold call
	bool held;			// Held call from other side
	bool rtpnatok;		// Until this is ok, SDP cannot be transmitted
	bool callwasestablished;	// We passed through VPCS_CONNECTED
	bool originatedcall;	// We have originated this call
	unsigned lastkeypadseqnumber;
	time_t start_time;
	unsigned start_ticks;

	// Party data
	char address[MAXADDRLEN+1], destnumber[MAXNUMBERLEN+1];
	char ourname[MAXNAMELEN+1];	// This name is overwritten when receiving a call
	char ourorigname[MAXNAMELEN+1];	// Name is the part before <sip:
	char connectedname[MAXNAMELEN+1];

	// SIP data
	char callid[MAXCALLIDLEN+1];
	char contact[MAXCONTACTLEN+1], ourcontact[MAXCONTACTLEN+1], route[MAXVIA][MAXROUTELEN+1];
	char connectedaddr[MAXSIPADDRLEN+1], connectedtag[MAXTAGLEN+1];
	char ouraddr[MAXSIPADDRLEN+1], ourtag[MAXTAGLEN+1];
	char viabranch[MAXVIA][MAXBRANCHLEN+1], viaaddr[MAXVIA][MAXSIPADDRLEN+1];
	char referto[MAXSIPADDRLEN+1];
	int viarport[MAXVIA];
	char sipreasontext[MAXREASONLEN+1], reasonline[MAXREASONLEN+1];
	unsigned seqnumber;
	struct sockaddr_in rrtpaddr, lrtpaddr;
	unsigned sessionid, sessionversion;
	int sipreason;
	SOCKET rtpsk;
	// Authentincation
	char nonce[MAXNONCELEN+1], cnonce[MAXNONCELEN+1];
	char realm[MAXREALMLEN+1], opaque[MAXOPAQUELEN+1];
	char password[MAXNAMELEN+1];
	unsigned nc;
	char qop, authtype;

	// Auto attendant
	struct {
		int f;
		unsigned sendinterval, lastsendtm, readlen, msgid;
	} aa;

private:
	CRITICAL_SECTION cs;
};

struct SIPSTACKSETTINGS {
	char username[MAXNAMELEN+1], password[MAXNAMELEN+1];
	char authusername[MAXNAMELEN+1];	// Optional, if empty, username will be used instead
	char server[MAXADDRLEN+1], domain[MAXADDRLEN+1];

	bool clir, refuseclir, undernat;
	unsigned remotenat;	// 0 no remote NAT, 1 set remote RTP send addr to RTP recv addr, 2 set remote RTP send addr to SIP recv addr
	unsigned supported_codecs;
	int notifyinterval;
	struct sockaddr_in natserveraddr, stunserveraddr;
	unsigned registrationinterval;
	long bindaddr;
};

class SIPSTACK : public IVPSTACK {
public:
	SIPSTACK(VPAUDIODATAFACTORY *vpaf = 0, const char *stackname = 0);
	~SIPSTACK();
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
	// Servers
	int SetNATServer(const char *addr);	// Find NAT type using vPhone servers
	int SetSTUNServer(const char *addr);	// Find NAT type using STUN servers
	int SetServers(const char *list);
	int Logon(const char *username, const char *password);
	int Logoff();
	int IsLoggedOn();
	const char *LogonName();
	const char *LogonNumber();
	// Setup
	void SetSupportedBearersMask(unsigned bc_mask) {}
	void SetAudioDataFactory(VPAUDIODATAFACTORY *af);
	int SetCodecsMask(unsigned mask) { settings.supported_codecs = mask; return 0; }
	void SetBindAddr(long bindaddr);
	void SetRemoteNAT(int remotenat) { settings.remotenat = remotenat; }

	// Misc
	int SendKeypad(VPCALL vpcall, int key);
	int GetStackType() { return STACKTYPE_SIP; }
	int EnumCalls(VPCALL *vpcalls, unsigned maxvpcalls, unsigned mask);
	void SetBandwidth(int kbps) { tx.maxkbps = kbps; }
	int GetTransceiverBandwidths(int *rx, int *tx);
	int ConferenceCalls(VPCALL *vpcalls, int ncalls, bool detach);
	int Hold(VPCALL vpcall);
	int Resume(VPCALL vpcall);
	int Transfer(VPCALL vpcall1, VPCALL vpcall2);
	int SetNotifyInterval(int ms) { settings.notifyinterval = ms; return 0; }

	int GetCallRemoteName(VPCALL vpcall, char *username);
	int GetCallRemoteNumber(VPCALL vpcall, char *number);
	int GetCallRemoteAddress(VPCALL vpcall, char *address);
	int GetCallRemoteAddress(VPCALL vpcall, struct sockaddr_in *addr);
	int GetCallRemoteSubAddr(VPCALL vpcall, char *subaddr) { *subaddr = 0; return VPERR_UNSUPPORTEDCALL; }
	int GetCallLocalName(VPCALL vpcall, char *username);
	int GetCallLocalNumber(VPCALL vpcall, char *number);
	int GetCallLocalSubAddr(VPCALL vpcall, char *subaddr) { *subaddr = 0; return VPERR_UNSUPPORTEDCALL; }
	int GetCallBearer(VPCALL vpcall) { return BC_VOICE; }
	int GetCallCodec(VPCALL vpcall);
	int GetCallTimes(VPCALL vpcall, time_t *start_time, unsigned *length_ms);
	int	SendAudio(VPCALL vpcall, int codec, unsigned timestamp, const void *buf, int len, bool wait = false);
	int GetCallStatus(VPCALL vpcall);
	int GetSIPReason(VPCALL vpcall);
	int GetSIPReasonText(VPCALL vpcall, char *reasontext);
	int GetReasonLine(VPCALL vpcall, char *reasonline);

protected:
	// Takes the decision on what to do on an incoming call, returns 0 = refuse, 1 = accept, 2 = alert, <0 = refuse with -reason
	virtual int IncomingCall(const struct sockaddr_in *addr, SIP_ELEMENTS *ie);
	virtual void Notify(int message, unsigned param1, unsigned param2);
	virtual int NotifyRc(int message, unsigned param1, unsigned param2);
	// Audio
	virtual void CallEstablished(SIPCALLDATA *vpc);
	virtual void CallEnded(SIPCALLDATA *vpc);
	virtual void RTPPacket(SIPCALLDATA *vpc, const BYTE *buf, int len);
	virtual void RTPKeypad(SIPCALLDATA *vpc, const BYTE *buf, int len);

	// Calls
	SIPCALLDATA *LockCallL(VPCALL vpcall, int line);
	SIPCALLDATA *LockCallL(int index, int line);

	void (*NotifyRoutine)(void *param, unsigned message, unsigned param1, unsigned param2);
	int (*SyncNotifyRoutine)(void *param, unsigned message, unsigned param1, unsigned param2);
	void *NotifyParam, *SyncNotifyParam;

	// Transceiver
	int Send(const struct sockaddr_in *addr,  const void *buf, int len);
	
	// Util
	void CopyCallFields(SIP_ELEMENTS *se, SIPCALLDATA *vpc, bool answer);
	void BuildLocalSEPart(SIP_ELEMENTS *se);
	void BuildLocalAddr();
	void BuildLocalCallData(SIPCALLDATA *vpc);
	void BuildVia(SIPCALLDATA *vpc);
	void RefuseCall(const struct sockaddr_in *addr, SIP_ELEMENTS *ie, int reason);
	void AcceptCall(VPCALL vpcall, int answer);
	void DisconnectAck(const struct sockaddr_in *addr, SIP_ELEMENTS *se, int cmd);
	void InfoAck(const struct sockaddr_in *addr, SIP_ELEMENTS *se);
	void UnknownMethod(const struct sockaddr_in *addr, SIP_ELEMENTS *se);
	virtual void RegisterAck(const struct sockaddr_in *addr, SIP_ELEMENTS *se, int reason);
	SIPCALLDATA *FindCall(const char *callid);
	unsigned PickSeqNumber();
	void SendInviteAck(const struct sockaddr_in *addr, SIP_ELEMENTS *se);
	void SendInviteNAck(const struct sockaddr_in *addr, SIP_ELEMENTS *se);
	int SendKeypad(VPCALL vpcall, int key, int len, bool end);

	// Settings
	struct SIPSTACKSETTINGS settings;
	BYTE fulllocaladdr[6];
	char sfulllocaladdr[MAXSIPADDRLEN+1];
	unsigned gseqnumber;

	// Misc
	long nthreads;
	int ncalls;
	VPAUDIODATAFACTORY *vpaudiodatafactory;

private:
	RWLOCK lock;
	void ReadLockL(int line);
	void ReadUnlockL(int line);
	void WriteLockL(int line);
	void WriteUnlockL(int line);
	void ConnectThread(void *vpcall1);
	void DisconnectThread(void *vpcall1);
	void IncomingCallThread(void *vpcall1);
	void PeriodicalUpdateServerAndAwake();
	VPCALL FindRegisterCall();

	int nw, nr;
	SIPCALLDATA *calls[MAXCALLS];
	unsigned lastvpcallvalue;
	unsigned nextregistertm;

	// Transceiver
	void ReceiveThread(void *dummy);
	void TransmitThread(void *dummy);
	void ProcessReceiverMessage(const struct sockaddr_in *addr, char *buf, int buflen);
	void ProcessServerMessage(const struct sockaddr_in *addr, char *buf, int buflen);
	void RTPReceiver(VPCALL vpcall);
	
	LBTransmitter tx;
	ratecounter rxxc;
	SOCKET sock;

	// Server
	bool loggedon, natresolved;
};

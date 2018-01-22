#include "vpstack.h"
#include "portability.h"
#include "ielements.h"
#include "netutil.h"
#include "util.h"
#include "udpstream.h"

#define MAXCALLS 100

#define Lock() LockL(__LINE__)
#define Unlock() UnlockL(__LINE__)
#define ReadLock() ReadLockL(__LINE__)
#define ReadUnlock() ReadUnlockL(__LINE__)
#define WriteLock() WriteLockL(__LINE__)
#define WriteUnlock() WriteUnlockL(__LINE__)
#define LockCall(a) LockCallL(a, __LINE__)

#define CONFSTATUS_IDLE 0
#define CONFSTATUS_WAITING 1
#define CONFSTATUS_USERALERTED 2
#define CONFSTATUS_ACCEPTED 4
#define CONFSTATUS_INITIATED 5
#define CONFSTATUS_ACTIVE 6

#define CONFSLAVESTATUS_IDLE 0
#define CONFSLAVESTATUS_ALERTNECESSARY 1	// This is for the call connected to the master
#define CONFSLAVESTATUS_USERALERTED 2	// This is for the call connected to the master
#define CONFSLAVESTATUS_USERCONFIRMED 3	// This is for the call connected to the master
#define CONFSLAVESTATUS_INITIATED 4	// This is for the call connected to the master
#define CONFSLAVESTATUS_CONNECTING 5	// This is for the call connected to the slave
#define CONFSLAVESTATUS_CONNECTED 6	// This is for the call connected to the slave

#define HEADERLEN 7
#define MAXCHATQUEUE 5
#define VPHONEPORT 11675
#define DEFAULT_MTU 548
#define LONG_MTU 1448
#define MAXBACKWORKFORVIDEO 1000
#define AWAKESENDINTERVAL 2000		// One awake packet every 2 seconds
#define DEFAULTSERVERUPDATEINTERVAL 24000
#define STACKVERSION 0x02000000
#define MAXVIDEOFRAMELEN 64000

class VPSTACK;

struct ACKDATAGRAM {
	struct sockaddr_in addr;
	void *buf;
	int buflen;
	unsigned short connectionid;
	unsigned expiretm;
	unsigned nextsendtm;
	unsigned seqnumber;
	int param1, param2;
};

struct CONFERENCEDATA {
	int ncalls;
	bool detach;
	VPCALL *vpcalls;
};

class VPCALLDATA;

struct UDPSTREAMDATA {
	VPSTACK *vpstack;
	VPCALL vpcall;
	VPCALLDATA *vpc;
	struct sockaddr_in addr;
	unsigned short connectionid;
};

class VPCALLDATA
{
public:
	VPCALLDATA() { Zero(*this); InitializeCriticalSection(&cs); }
	~VPCALLDATA() { DeleteCriticalSection(&cs); }
	void LockL(int line);
	void UnlockL(int line);
	void InUse() { inuse++; }

	// Main statuses
	VPCALL vpcall;
	BYTE status;
	bool cantx, sendcallrequests, restartconnectthread, proxybound;
	unsigned starttm;

	// Addresses
	struct sockaddr_in addr, publicaddr, privateaddr;
	WORD connectionid;

	// Misc
	unsigned awaketm, token, clearattm;
	int bc, offline, addrtype, retry, altport;
	bool remotelyconferenced, remotelyheld;
	bool conferenced, held, hasdisconnectmessage, useproxy;
	unsigned seqnumber;
	int mtu;
	time_t start_time;
	unsigned start_ticks;

	// SMS
	char message[MAXMESSAGELEN+1];
	unsigned missedcallscount, missedcalltime;
	unsigned smstype, smsid;
	BYTE missedcallbc;

	// Party data
	char address[MAXADDRLEN+1], subaddress[MAXSUBADDRLEN+1], destnumber[MAXNUMBERLEN+1];
	char ourname[MAXNAMELEN+1], ournumber[MAXNUMBERLEN+1], oursubaddress[MAXSUBADDRLEN+1];
	char connectedname[MAXNAMELEN+1], connectednumber[MAXNUMBERLEN+1], connectedsubaddress[MAXSUBADDRLEN+1];
	char connectedserveraccesscode[MAXSERVERACCESSCODELEN+1];
	char sessionid[MAXSESSIONIDLEN+1], sessionsecret[MAXSESSIONSECRETLEN+1];

	// Audio
	int framesperpacket, codec;
	unsigned short audioseqno;
	unsigned rxtimestamp;
	VPAUDIODATA *audio;

	// File transfer, chat and keypad
	unsigned nfiles, nbytes;
	int rtfencoding, rtfdatalen;
	char rtfdata[MAXTEXTLEN+1];
	int chatqueuelen, chatretry;
	struct {
		int rtfdatalen;
		unsigned seqnumber;
		char *rtfdata;
		int rtfencoding;
	} chatqueue[MAXCHATQUEUE];
	unsigned lastkeypadseqnumber;

	// Video
	VPVIDEODATA *video;
	struct {
		unsigned fourcc, width, height, quality, framerate;
	} videofmt;
	struct {
		BYTE *buf;
		DWORD map[4];
		BYTE *parity;
		int paritylen, size, fragments, fraglen;
		WORD timestamp;
		BYTE keyframe;
		BYTE curseq;
		bool firstkeyframe;
	} videopkt;
	BYTE videoseq;

	// Authentication for calls to servers
	BYTE authdata[16];
	char addauthdata;

	// Conferences and call transfers
	struct {
		CONFERENCEDATA *conferencedata;
		VPCALL discondisccall;
		unsigned token;
		int status;
	} mconf;	// master
	struct {
		struct sockaddr_in masteraddr;
		int status;
		bool detach, step0acked;
		int bc;
		unsigned token;
		VPCALL vpcalls[MAXVPARTIES];
	} sconf;	// slave
	
	// Auto attendant
	struct {
		int f;
		unsigned sendinterval, lastsendtm, readlen, msgid;
	} aa;

	// Auth calls
	struct {
		bool acked, reqauth, authenticated;
		int reason;
		int quota;
		BYTE authdata[16];
	} gacd;

	// File transfer
	struct {
		int f;
		char accepted;	// remote side answered 1=wait(useralerted), 2=accepted, 3=refused
		char receiving;	// state, 1=sendfile request received, 2=receiving accepted, 3=receiving in progress (PKT_FILEINFO received)
		char sending;	// state, 1=sending in progress
		char path[MAX_PATH];
		unsigned length, pos, startpos;
		unsigned starttm, tmout, lastnotify;
		UDPStream *udpstream;
	} ft;

	void *havi;	// AVI recording
	struct {
		HWINDOW hParent;
		int childid;
		RECTANGLE rect;
		bool hidden;
		bool set;
	} vparam;
	void *hwav;	// WAV recording

	QOSDATA aqos, vqos;

private:
	CRITICAL_SECTION cs;
	int inuse;
	bool deleting;

	friend class VPSTACK;
};

enum {RES_IDLE, RES_FOUND, RES_NOTFOUND, RES_OFFLINE, RES_ABORTED};

class VPSTACK : public IVPSTACK {
public:
	VPSTACK(VPAUDIODATAFACTORY *vpaf = 0, const char *stackname = 0, VPVIDEODATAFACTORY *vpvf = 0);
	~VPSTACK();
	int Init();
	int SetNotifyRoutine(void (*notify)(void *param, unsigned, unsigned, unsigned), void *param);
	int SetSyncNotifyRoutine(int (*notify)(void *param, unsigned, unsigned, unsigned), void *param);
	// Create a call
	VPCALL CreateCall();
	// Free a call
	void FreeCall(VPCALL vpcall);
	// Attempt a connection with address, bearer bc, on success returns a VPCALL
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
	int Hold(VPCALL vpcall);
	int Resume(VPCALL vpcall);
	int AddToConference(VPCALL vpcall);
	int RemoveFromConference(VPCALL vpcall);
	int ConferenceAll();
	int StopConference();
	int AcceptConference(VPCALL vpcall, bool accept);
	// Conferences and call transfers, detach detaches the calls from us after setup complete (for call transfer)
	int ConferenceCalls(VPCALL *vpcalls, int ncalls, bool detach);
	int StopConference(VPCALL vpcall);
	int DropFromConference(VPCALL vpcall);
	// Auto attendant
	int SendAudioMessage(VPCALL vpcall, const char *file, unsigned msgid);
	// Servers
	int SetServers(const char *list);
	int SetNATServer(const char *addr);
	int Logon(const char *username, const char *password);
	int Logon(const char *serveraccesscode, const BYTE *accountsecret);
	int Logoff();
	int GetLogonData(char *serveraccesscode, BYTE *accountsecret);
	int IsLoggedOn();
	const char *LogonName();
	const char *LogonNumber();
	// Setup
	void SetSupportedBearersMask(unsigned bc_mask) { settings.supportedbearers = bc_mask; }
	void SetCodec(unsigned preferredcodec, unsigned supportedcodecs) { settings.preferredcodec = preferredcodec; settings.supportedcodecs = supportedcodecs; }
	void SetAudioDataFactory(VPAUDIODATAFACTORY *af);
	void SetVideoDataFactory(VPVIDEODATAFACTORY *vf);

	// Misc
	int SendKeypad(VPCALL vpcall, int key);
	int GetStackType() { return STACKTYPE_VP; }
	int NetworkQuality();
	int GetQosData(VPCALL vpcall, QOSDATA *qos);
	int GetVQosData(VPCALL vpcall, QOSDATA *qos);
	int GetAudioParameters(VPCALL vpcall, int *codec, int *framesperpacket);
	int SetCodecsMask(unsigned mask) { settings.supportedcodecs = mask; return 0; }
	void SetBandwidth(int kbps) { tx.maxkbps = kbps; }
	int GetCallStatus(VPCALL vpcall);
	int SetPublicAddr(const char *addr);
	int GetTransceiverBandwidths(int *rx, int *tx);
	int CallForwardingRequest(int op, const char *number);
	int GetCallForwardingStatus(int *op, char *number);
	int MeasureBandwidth();
	int SetNotifyInterval(int ms) { settings.notifyinterval = ms; return 0; }
	int SetAPNsToken(const void *token, int tokensize);

	// Fill vpcalls with at most maxvpcalls of calls with bearer filtered by mask
	// Returns number of calls filled, or total number of calls present if vpcalls == 0
	int EnumCalls(VPCALL *vpcalls, unsigned maxvpcalls, unsigned mask);
	void PrintCalls();

	// Get information about calls, ncalls contains the size of the vpcalls array on input,
	// the number of filled entries on output
	int GetConferencePeers(VPCALL vpcall, VPCALL *vpcalls, int *ncalls);
	int GetLocallyConferencedCalls(VPCALL vpcall, VPCALL *vpcalls, int *ncalls);
	// Get the call that will be disconnected when vpcall is disconnected; when linkedcall
	// becomes established, it is connected to vpcall and then the two calls detached from us
	int GetLinkedCall(VPCALL vpcall, VPCALL *linkedcall);
	int GetCallRemoteName(VPCALL vpcall, char *username);
	int GetCallRemoteNumber(VPCALL vpcall, char *number);
	int GetCallRemoteAddress(VPCALL vpcall, char *address);
	int GetCallRemoteSubAddr(VPCALL vpcall, char *subaddr);
	int GetCallRemoteServerAccessCode(VPCALL vpcall, char *serveraccesscode);
	int GetCallLocalName(VPCALL vpcall, char *username);
	int GetCallLocalNumber(VPCALL vpcall, char *number);
	int GetCallLocalSubAddr(VPCALL vpcall, char *subaddr);
	int GetCallBearer(VPCALL vpcall);
	int GetCallCodec(VPCALL vpcall);
	int GetCallTimes(VPCALL vpcall, time_t *start_time, unsigned *length_ms);
	int IsCallOffline(VPCALL vpcall);
	int IsCallWithProxy(VPCALL vpcall);
	int IsHeld(VPCALL vpcall);
	int IsConferenced(VPCALL vpcall);
	int SetCallDisconnectMessage(VPCALL vpcall, const char *text);
	// SMS data
	int GetMissedCallsData(VPCALL vpcall, unsigned *missedcallscount, unsigned *missedcalltime, unsigned *missedcallbc);
	int GetCallText(VPCALL vpcall, char *text);
	int SetCallText(VPCALL vpcall, const char *text);
	int SetMissedCallBC(VPCALL vpcall, int bc);
	int SetSMSType(VPCALL vpcall, unsigned type);
	int GetSMSType(VPCALL vpcall, unsigned *type);
	int SetSMSId(VPCALL vpcall, unsigned id);
	int GetSMSId(VPCALL vpcall, unsigned *id);

	int	SendAudio(VPCALL vpcall, int codec, unsigned timestamp, const void *buf, int len, bool wait = false);
	int SendVideo(VPCALL vpcall, WORD timestamp, void *data, int size, bool keyframe);

	// Settings
	void SetBindPort(int port) { settings.port = port; }
	int GetBindPort() { return settings.port; }
	void SetUserStatus(int userstatus) { settings.userstatus = userstatus; }
	int GetUserStatus() { return settings.userstatus; }

	// Vphone protocol specific
	int AskOnline(AOL *aol);
	int QueryAccountInfo();
	int ServerTransfer(int op, const char *path, bool amactive, const char *vname); // path is file path, apart for op=TCPFT_RXMST, where path is a directory; amactive is only used for op=TCPFT_TXGREETING
	int UserSearch(const char *name, const char *country, unsigned flags, unsigned param);

	// Chat
	int SendChat(VPCALL vpcall, const char *text);
	int GetChatText(VPCALL vpcall, char *text, int textsize);

	// File transfer
	int SendFile(VPCALL vpcall, const char *path);
	int AbortFileTransfer(VPCALL vpcall);
	int AcceptFile(VPCALL vpcall, int accept);
	int GetCallFilePath(VPCALL vpcall, char *path);
	int SetCallFilePath(VPCALL vpcall, const char *path);
	int SetCallNFiles(VPCALL vpcall, unsigned nfiles, unsigned nbytes);
	int GetCallNFiles(VPCALL vpcall, unsigned *nfiles, unsigned *nbytes);
	int GetFileProgress(VPCALL vpcall, unsigned *current, unsigned *total);

	// Video
	int SetDefaultVideoParameters(unsigned fourcc, unsigned width, unsigned height, unsigned framerate, unsigned quality);
	int GetDefaultVideoParameters(unsigned *fourcc, unsigned *width, unsigned *height, unsigned *framerate, unsigned *quality);
	int SetCallVideoParameters(VPCALL vpcall, unsigned fourcc, unsigned width, unsigned height, unsigned framerate, unsigned quality);
	int GetCallVideoParameters(VPCALL vpcall, unsigned *fourcc, unsigned *width, unsigned *height, unsigned *framerate, unsigned *quality);
	int RecordAVIFile(const char *path);
	int StopAVIRecord();
	int RecordWAVFile(const char *path);
	int StopWAVRecord();
	int GetRecorderCall(VPCALL *vpcall);
	int SetVideoWindowData(VPCALL vpcall, HWINDOW hParent, int childid, const RECTANGLE *rect, bool hidden);
	int SetVideoWindowFullScreen(VPCALL vpcall, int fs);
	int CaptureVideoImage(VPCALL vpcall, void **image, int *w, int *h);

protected:
	// Takes the decision on what to do on an incoming call, returns 0 = refuse, 1 = accept, 2 = alert, <0 = refuse with -reason
	virtual int IncomingCall(const struct sockaddr_in *addr, IE_ELEMENTS *ie);
	// There is too much backwork for sending, this should trigger some drop in the source
	virtual void NotifyBackworkHigh();
	virtual void Notify(int message, unsigned param1, unsigned param2);
	virtual int NotifyRc(int message, unsigned param1, unsigned param2);
	virtual void NotifyRefusedCall(const struct sockaddr_in *addr, IE_ELEMENTS *ie, int reason);
	// Audio
	virtual void CallEstablished(VPCALL vpcall);
	virtual void ProcessAudioMessage(const struct sockaddr_in *addr, BYTE *buf, int buflen);
	virtual void ProcessVideoMessage(const struct sockaddr_in *addr, BYTE *buf, int buflen);
	virtual int CheckDeliverVideoFrame(VPCALLDATA *vpc);

	// Extra messages
	virtual void GetAuthCallDataAck(const struct sockaddr_in *addr, IE_ELEMENTS *ie) {}

	// Calls
	VPCALLDATA *LockCallL(VPCALL vpcall, int line);
	VPCALLDATA *LockInUseCall(VPCALL vpcall);
	VPCALLDATA *LockCallL(int index, int line);
	VPCALLDATA *FindCall(const struct sockaddr_in *addr, int connectionid, int bc);
	VPCALLDATA *FindCallByToken(const struct sockaddr_in *addr, unsigned token);
	VPCALLDATA *FindCallByConfToken(const struct sockaddr_in *addr1, const struct sockaddr_in *addr2, unsigned token);
	VPCALLDATA *FindCallOrDisconnect(const struct sockaddr_in *addr, int connectionid, int bc);
	VPCALLDATA *FindMasterCall(const struct sockaddr_in *addr, unsigned token);
	int FindConferenceCalls(const struct sockaddr_in *addr, unsigned token, int bc, VPCALL *calls);	// Searched calls on slave
	int FindConferencedCalls(unsigned token, int bc, VPCALL *calls);	// Searches calls on master
	int AssignConnectionId(VPCALL vpcall, const struct sockaddr_in *addr);
	void DelayedClearCall(VPCALL vpcall);
	void CleanConferenceCalls(const struct sockaddr_in *addr, unsigned token);
	virtual void LocalConference(VPCALL vpcall) {}
	int ConferenceVideoCalls();

	// Misc
	void AddIE_Addresses(OUTMSG *outmsg);
	void RefuseCall(const struct sockaddr_in *addr, IE_ELEMENTS *ie, int reason);
	void AcceptCall(VPCALL vpcall, int answer, int reason, unsigned seqnumber);
	int DecideCodec(IE_ELEMENTS *ie);
	void (*NotifyRoutine)(void *param, unsigned message, unsigned param1, unsigned param2);
	int (*SyncNotifyRoutine)(void *param, unsigned message, unsigned param1, unsigned param2);
	void *NotifyParam, *SyncNotifyParam;
	void NetworkQualityAudio(VPCALLDATA *vpc, WORD seqnumber, WORD ts, int pktlen);
	void NetworkQualityVideo(VPCALLDATA *vpc, BYTE seqnumber, bool keyframe, BYTE fragment, WORD ts, int pktlen);
	void SendMTUTest(int connectionid, struct sockaddr_in *addr);
	void RecordAVIStartThread(void *dummy);
	void MeasureBandwidthThread(void *dummy);
	void NATDiscoverThread(void *dummy);

	// Apple Push Notification Service
	void *apntoken;
	int apntokensize;

	// File transfer
	void SendFileThread(void *vpcall1);
	int RxFile(VPCALLDATA *vpc, void *buf, int len);
	int TerminateFTReceive(VPCALLDATA *vpc);
	void SendFile2(VPCALLDATA *vpc);
	friend int rxsendrtn(void *param, void *buf, int len);
	friend int rxrecvrtn(void *param, void *buf, int len);
	friend int txsendrtn(void *param, void *buf, int len);
	friend int txrecvrtn(void *param, void *buf, int len);

	// Transceiver
	int Send(const struct sockaddr_in *addr, OUTMSG *msg);
	int Send(const struct sockaddr_in *addr,  const void *buf, int len);
	int vPResolve(const char *address, char *resolvednumber, sockaddr_in *addr, sockaddr_in *publicaddr, sockaddr_in *privateaddr, unsigned token, int bc, int offline, int codec, int supportedcodecs, VPCALL vpcall = 0);
	int SendPacketReliable(VPCALL vpcall, OUTMSG *msg, unsigned seqnumber, int (VPSTACK::*deferredproc)(int, int) = 0, int param1 = 0, int param2 = 0);
	void ReliableSender();
	void ReliableReceiver(const struct sockaddr_in *addr, IE_ELEMENTS *ie);

	// Settings
	struct VPSTACKSETTINGS settings;

	// Server logon
	unsigned instancetoken;

	// Misc
	long nthreads;
//	int ncalls;
	VPAUDIODATAFACTORY *vpaudiodatafactory;
	VPVIDEODATAFACTORY *vpvideodatafactory;
	
//private:
	RWLOCK lock;
	void ReadLockL(int line);
	void ReadUnlockL(int line);
	void WriteLockL(int line);
	void WriteUnlockL(int line);
	void ConnectThread(void *vpcall1);
	void IncomingCallThread(void *vpcall1);
	void DoConferenceCallThread(void *vpcall1);
	void ChatThread(void *vpcall1);

	CRITICAL_SECTION ctokencs;
	int nw, nr;
	VPCALLDATA *vpcalls[MAXCALLS];
	unsigned lastawaketm;
	unsigned seqnumber;
	unsigned smsseqnumber;
	unsigned lastvpcallvalue;

	// Transceiver
	void ReceiveThread(void *dummy);
	void TransmitThread(void *dummy);
	void CreateConferenceThread(void *conferencedata1);
	void ProcessReceiverMessage(const struct sockaddr_in *addr, BYTE *buf, int buflen);
	void ProcessServerMessage(const struct sockaddr_in *addr, BYTE *buf, int buflen);
	void AutoAttendantProcess();
	void CallForwardingThread(void *param);
	
	LBTransmitter tx;
	ratecounter rxxc;
	SOCKET sock;
	ACKDATAGRAM *adl[MAXACKDATAGRAMS];
	unsigned adllen, adltm;
	CRITICAL_SECTION adlcs;
	VPCALL recordvpcall;

	// Bandwidth and NAT
	bool measuringbandwidth;
	char natserverslist[MAXMESSAGELEN+1];
	struct {
		struct sockaddr_in addrs[2];
		bool nonat[2], received[2], restricted[2], fullcone[2], discoverrunning;
		int type;
		unsigned token;
	} nat;

	// Lately found seqnumbers
	int SeqNumberLatelyFound(const struct sockaddr_in *addr, unsigned seqnumber);
	struct {
		struct sockaddr_in addr;
		unsigned seqnumber;
		unsigned tm;
	} seqnumbers[MAXSEQNUMBERS];
	unsigned nseqnumbers, lastseqnumberscleantm;

	// Server logon
	void ResolveServer();
	void UsernameLogon();
	void PeriodicalUpdateServerAndAwake();

	unsigned notifyackawaitedtm;
	unsigned lastserverupdatetm;
	unsigned stackversion, lastversion;
	int notifyackfailed;
	bool allowlogon, loggedon;
	struct sockaddr_in localhostpublicaddr;
	struct sockaddr_in serveraddr;
	unsigned localhostaddr;

	RESOLVER resolvers[MAXRESOLVERS];
	CRITICAL_SECTION resolvercs;

	// Online users and notifications
	DWORD encodednumbers[135];
	int nencodednumbers;
	bool queryonlineack;
	unsigned lastnotifylogonseqnumber, lastnotifylogonaddr;
	struct {
		DWORD numbers[130], srvid;
		DWORD seqnumber;
		int N;
		int operation;
		bool working, stop;
	} aoldata;
	void AOLThread(void *aol);

	// Server TCP transfers
	struct {
		long working;
		SOCKET sk;
	} dvm;
	void ServerTCPTransfer(void *data);
	filetime lastgreetingsent;
	int lastamactive;
	bool cfinprogress;
};

inline void AddAckIEs(OUTMSG *outmsg, IE_ELEMENTS *ie)
{
	if(ie->seqnumberpresent)
		outmsg->AddIE_DWord(IE_SEQNUMBER, ie->seqnumber);
	if(ie->tokenpresent)
		outmsg->AddIE_DWord(IE_TOKEN, ie->token);
	if(ie->transparentdata)
		outmsg->AddIE_Binary(IE_TRANSPARENTDATA, (BYTE *)ie->transparentdata, ie->transparentdatalen);
}

#define TSTART	InterlockedIncrement(&nthreads);
#define TRETURN	{InterlockedDecrement(&nthreads); return; }

#ifndef _VPSTACK_H_INCLUDED_
#define _VPSTACK_H_INCLUDED_

#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable: 4100 )
#endif

#include <time.h>
#include "vprotocol.h"

enum {STACKTYPE_VP, STACKTYPE_SIP, STACKTYPE_ISDN, STACKTYPE_GSM};

#define REASON_LOGGEDON 1000
#define REASON_VERSIONOLD 1001
#define REASON_SERVERRESET 1002

// Errors
#define VPERR_INVALIDCALL -1
#define VPERR_NOMORECALLSAVAILABLE -2
#define VPERR_TCPIPERROR -3
#define VPERR_INVALIDADDR -4
#define VPERR_LOOPCALL -5
#define VPERR_INVALIDSTATUS -6
#define VPERR_NOTLOGGEDON -7
#define VPERR_INCOMPATIBLECALLS -8
#define VPERR_FILENOTFOUND -9
#define VPERR_INVALIDFILEFORMAT -10
#define VPERR_UNSUPPORTEDCALL -11
#define VPERR_UNSUPPORTEDBEARER -12
#define VPERR_INVALIDPARAMETER -13
#define VPERR_TIMEOUT -14

// Define statuses
#define VPCS_IDLE 0
#define VPCS_CONNECTING 1
#define VPCS_CALLPROCEEDING 2
#define VPCS_ALERTED 3
#define VPCS_INCALLPROCEEDING 4
#define VPCS_INALERTED 5
#define VPCS_INCONNECTING 6
#define VPCS_HELD 7
#define VPCS_CONNECTED 8
#define VPCS_DISCONNECTING 9
#define VPCS_TERMINATED 10
#define VPCS_RECORDING 11	// Special status: this call is for recording, not a true call

/* Calls:
Calls are created with CreateCall, Connect or automatically, which are notified by VPMSG_NEWCALL
Calls are freed with FreeCall
 */

// Messages
#define VPMSG_BASE 0
#define VPMSG_SERVERSLISTCHANGED	(VPMSG_BASE+1)	// Servers list has been updated. not useful for end user
#define VPMSG_SERVERSTATUS			(VPMSG_BASE+2)	// Something has happened during the connection with the server, the first parameter contains a reason
#define VPMSG_CALLFORWARDING		(VPMSG_BASE+3)	// Call forwarding status has been updated
#define VPMSG_SERVERTRANSFERFINISHED (VPMSG_BASE+4)	// The ServerTransfer function has finished
// Calls related, the first parameter is always a VPCALL
#define VPMSG_INVALIDADDR			(VPMSG_BASE+5)	// The address parameter is wrong
#define VPMSG_SERVERNOTRESPONDING	(VPMSG_BASE+6)	// The server is not respoding (no connection to the Internet?)
#define VPMSG_RESOLVED				(VPMSG_BASE+7)	// The name has been resolved to an IP address
#define VPMSG_USERALERTED			(VPMSG_BASE+8)	// The user has been alerted
#define VPMSG_CALLREFUSED			(VPMSG_BASE+9)	// The remote side has refused the call
#define VPMSG_CALLACCEPTED			(VPMSG_BASE+10)	// Outgoing call established (not necessarily connected to the user, but audio is available from the network)
#define VPMSG_CALLACCEPTEDBYUSER	(VPMSG_BASE+11)	// Outgoing call established, connected with the user (sent after the msg above)
#define VPMSG_CONNECTFINISHED		(VPMSG_BASE+12)	// Connect finished, successfully or unsuccessfully
#define VPMSG_CALLESTABLISHED		(VPMSG_BASE+13)	// Incoming call established
#define VPMSG_CALLSETUPFAILED		(VPMSG_BASE+14) // The call setup has failed (usually a NAT or firewall problem)
#define VPMSG_CONNECTTIMEOUT		(VPMSG_BASE+15) // The call setup has failed because of a timeout (usually a NAT or firewall problem)
#define VPMSG_CALLDISCONNECTED		(VPMSG_BASE+16)	// The call has been disconnected by the remote side, the second parameter is the reason
#define VPMSG_CALLENDED				(VPMSG_BASE+17)	// Calls *have* to be freed after the reception of this message
#define VPMSG_NEWCALL				(VPMSG_BASE+18)	// A new call has been received
#define VPMSG_KEYPAD                (VPMSG_BASE+19)	// Keypad data (i.e. DMTF) has been received from the other side (second parameter)
#define VPMSG_AUDIOMSGCOMPLETE		(VPMSG_BASE+20)	// SendAudioMessage has finished, the second parameter is msgid
#define VPMSG_REMOTELYCONFERENCED	(VPMSG_BASE+21)	// The remote side has put us in voice conference (voice conference is local audio mixing, not a multicall conference)
#define VPMSG_REMOTELYUNCONFERENCED	(VPMSG_BASE+22)	// The remote side has put us out of voice conference
#define VPMSG_REMOTELYHELD			(VPMSG_BASE+23)	// The remote side has held our call
#define VPMSG_REMOTELYRESUMED		(VPMSG_BASE+24)	// The remote side has resumed our call
#define VPMSG_CONFERENCEESTABLISHED	(VPMSG_BASE+25)	// ConferenceCalls has finished correctly (this is a multicall conference, required for video and used also for call transfer (second param=1))
#define VPMSG_CONFERENCEFAILED		(VPMSG_BASE+26)	// ConferenceCalls has failed (second parameter is 1 for call transfer (detach=1))
#define VPMSG_CONFERENCEREQ			(VPMSG_BASE+27)	// A multicall conference has been requested by the remote side (call transfer is always accepted, so this message is not notified)
#define VPMSG_CONFERENCECALLESTABLISHED	(VPMSG_BASE+28)	// We have finished the connection with everybody involved in a multicall conference
#define VPMSG_LOCALCONFERENCEREQ	(VPMSG_BASE+29)	// Not used by VPSTACK
#define VPMSG_CONFERENCEEND			(VPMSG_BASE+30)	// The multicall conference has ended
#define VPMSG_CALLTRANSFERRED		(VPMSG_BASE+31)	// The first parameter vpcall has been transferred to the second parameter vpcall
#define VPMSG_CHAT					(VPMSG_BASE+32)	// Incoming chat text
#define VPMSG_CHATACK				(VPMSG_BASE+33)	// The sent chat text has been acknowledged
#define	VPMSG_SENDFILEACK_REFUSED	(VPMSG_BASE+34)	// The file sent with SendFile has been refused
#define	VPMSG_SENDFILEACK_USERALERTED	(VPMSG_BASE+35)	// The file sent with SendFile has been notified to the user, which should accept or refuse it
#define	VPMSG_SENDFILEACK_ACCEPTED	(VPMSG_BASE+36)	// The file sent with SendFile has been accepted (the file transfer will start)
#define	VPMSG_FTPROGRESS			(VPMSG_BASE+37)	// More bytes have been sent or received via file transfer
#define	VPMSG_FTTIMEOUT				(VPMSG_BASE+38)	// There was a timeout sending a file with SendFile
#define	VPMSG_FTTXCOMPLETE			(VPMSG_BASE+39)	// SendFile has finished
#define	VPMSG_FTRXCOMPLETE			(VPMSG_BASE+40)	// A receive file operation has finished
#define	VPMSG_SENDFILEREQ			(VPMSG_BASE+41)	// A file transfer operation to us has been requested, it should be accepted or refused
#define	VPMSG_SENDFILESUCCESS		(VPMSG_BASE+42)	// A file transfer operation has finished successfully (valid in directions)
#define	VPMSG_SENDFILEFAILED		(VPMSG_BASE+43)	// A file transfer operation has filed (valid in both directions)
#define VPMSG_HELD					(VPMSG_BASE+44)	// Only used in SIP, a Hold operation has succeeded, the call is held
#define VPMSG_RESUMED				(VPMSG_BASE+45)	// Only used in SIP, a Resume operationh has succeeded, the call is active
#define VPMSG_REQUESTREFUSED		(VPMSG_BASE+46)	// Only used in SIP (request can be hold/resume/transfer)
// These don't have a VPCALL as the first parameter
#define VPMSG_QUERYONLINEACK		(VPMSG_BASE+47)	// First parameter is 1: more data available, first parameter 0: AskOnline finished; second parameter, pointer to aol structure
#define VPMSG_CREDIT				(VPMSG_BASE+48)	// Quota on the server: first parameter is 1/10000th of dollar, second parameter is SMS units or -1, if SMS units are not used
#define VPMSG_MEASUREBANDWIDTH		(VPMSG_BASE+49)	// Second parameter is bandwidth in kbps or 0
#define VPMSG_NATTYPE				(VPMSG_BASE+50)	// Second parameter is NAT type
// ISDN specific messages
#define VPMSG_DISPLAYINFO			(VPMSG_BASE+100)
#define VPMSG_ISDNINFO				(VPMSG_BASE+101)
#define VPMSG_SSERROR				(VPMSG_BASE+102)
// Messages that require an answer or pass pointers, usually you should return 0
#define VPMSG_NOTIFYLOGON			(VPMSG_BASE+200)	// Return 1 if you don't want to be notified for this user (first parameter is a string) anymore
#define VPMSG_NEWVOICEMSG			(VPMSG_BASE+201)	// New voice message, first parameter is number, second is name of the caller
#define VPMSG_ABUPDATE				(VPMSG_BASE+202)	// First parameter is username, second is number, sent during name or number resolution
#define VPMSG_USERSEARCH			(VPMSG_BASE+203)	// First parameter is param passed to UserSearch, the second is the result (a string)
// VPVIDEO messages
#define VPMSG_WINDOWCLOSED			(VPMSG_BASE+300)	// The user has closed the video window
#define VPMSG_VIDEOSTARTERROR		(VPMSG_BASE+301)	// Error starting video, first parameter is stack, second is vpcall
#define VPMSG_VIDEOSTARTOK			(VPMSG_BASE+302)	// Video started, first parameter is stack, second is vpcall

#define MAXPATH 255
#define MAXRESOLVERS 10
#define MAXSEQNUMBERS 100	// Maximum numbers of lately seen sequential numbers
#define MAXACKDATAGRAMS 1000	// Maximum length of ackdatagrams queue

enum {NAT_UNKNOWN, NAT_NONAT, NAT_NONAT_PACKETLOSS, NAT_FIREWALL,
NAT_FULLCONE, NAT_RESTRICTEDCONE, NAT_PORTRESTRICTEDCONE, NAT_SYMMETRIC, NAT_INCONSISTENT};

#define OPERATORFLAG_3G	1

#define NotifyOnCall(message, vpcall, param2) Notify(message, (unsigned)vpcall, param2)

typedef void *VPCALL;

enum ADDRESSTYPE {ADDRT_INVALID, ADDRT_VNUMBER, ADDRT_PSTN, ADDRT_SPECIAL, ADDRT_VNAME, ADDRT_IPADDR};

void PlainNumber(const char *vpnumber, char *plainnumber);
ADDRESSTYPE AddressType(const char *address);
ADDRESSTYPE CanonicFormAddress(char *address);
int IsAddressVName(const char *address);

typedef struct {
	int rate, bytes, lasttm;
} ratecounter;

struct QOSDATA {
	unsigned pkts, lostpackets, curjitter, maxjitter, duplicated, outoforderpackets, kbps, latepackets, quality, silence;
	unsigned mintsdiff, maxtsdiff, mintsdiff2, maxtsdiff2;
	unsigned goodframes, keyframes, lostframes, lostkeyframes;
	unsigned short seqnumber, ts;
	unsigned char fragment, lostframe;
	bool keyframe;
	ratecounter xc, good, bad;
};

#ifdef  UNICODE
#include <tchar.h>
typedef TCHAR tchar;
#else
typedef char tchar;
#endif

struct RECTANGLE {
	int left, top, right, bottom;
};

typedef void *HWINDOW;

struct VPSTACKSETTINGS {
	char serveraccesscode[MAXSERVERACCESSCODELEN+1], usernum[MAXNUMBERLEN+1], username[MAXNAMELEN+1], password[MAXNAMELEN+1];
	char serverslist[MAXMESSAGELEN+1];
	char localhostsaddr[100];
	bool autolocalhostaddr;
	
	char cfnumber[MAXNUMBERLEN+1];
	int cfop;
	int port;
	int userstatus;
	bool forceport, clir, refuseclir;
	unsigned char accountsecret[16];
	int notifyinterval;

	unsigned supportedbearers;
	// Audio
	unsigned supportedcodecs;
	int preferredcodec;
	// Video
	unsigned fourcc, width, height, quality, framerate;
	bool txvideoparity;
};

class IVPSTACK;

class VPAUDIODATA
{
public:
	virtual ~VPAUDIODATA() {}
	virtual void AudioFrame(int codec, unsigned timestamp, const unsigned char *buf, int buflen) = 0;
	virtual int ChangeCodec(int codec) { return VPERR_UNSUPPORTEDCALL; }
	virtual int Save(const tchar *path) { return VPERR_UNSUPPORTEDCALL; }
};

#ifndef mmioFOURCC
#define mmioFOURCC(a,b,c,d) ((a) | (b)<<8 | (c)<<16 | (d)<<24)
#endif

class VPVIDEODATA
{
public:
	virtual ~VPVIDEODATA() {}
	virtual void VideoFrame(unsigned short timestamp, const unsigned char *buf, int buflen, bool keyframe) = 0;
	virtual int SetVideoWindowData(HWINDOW hParent, int childid, const RECTANGLE *rect, bool hidden) { return VPERR_UNSUPPORTEDCALL; }
	virtual int SetFullScreen(int fs) { return VPERR_UNSUPPORTEDCALL; }
	virtual int Start() { return VPERR_UNSUPPORTEDCALL; }
	virtual int Create(IVPSTACK *vps, VPCALL vpcall, unsigned fourccrx, unsigned xresrx, unsigned yresrx, unsigned fourcc, unsigned xres, unsigned yres, unsigned framerate, unsigned quality) { return VPERR_UNSUPPORTEDCALL; }
	virtual int SetNotifyRoutine(void (*notify)(void *param, unsigned, unsigned, unsigned), void *param) { return VPERR_UNSUPPORTEDCALL; }
	virtual int CaptureImage(void **image, int *w, int *h) { return VPERR_UNSUPPORTEDCALL; }
};

class VPAUDIODATAFACTORY
{
public:
	virtual VPAUDIODATA *New(IVPSTACK *vps, VPCALL vpcall, int codec) = 0;
};

class VPVIDEODATAFACTORY
{
public:
	virtual VPVIDEODATA *New(IVPSTACK *vps, VPCALL vpcall, unsigned fourccrx, unsigned xresrx, unsigned yresrx, unsigned fourcc, unsigned xres, unsigned yres, unsigned framerate, unsigned videoquality) = 0;
};

struct AOLNUM {
	unsigned srvid;
	unsigned num;
	int online;
};

enum {NOTIFICATIONREQ_NONE, NOTIFICATIONREQ_DISABLE, NOTIFICATIONREQ_ENABLE};
enum {AOL_OFFLINE=1, AOL_ONLINE=2, AOL_LIMITED=4, AOL_WEBCAMPRESENT=5, AOL_LIMITED_WEBCAMPRESENT=6};
enum {USERSTATUS_ONLINE=0, USERSTATUS_SHOWOFFLINE=1, USERSTATUS_OFFLINE=2, USERSTATUS_LIMITED=3,
USERSTATUS_WEBCAMPRESENT=4, USERSTATUS_LIMITED_WEBCAMPRESENT=5};
enum {TCPFT_RXMSG, TCPFT_TXGREETING, TCPFT_TXAB, TCPFT_RXAB, TCPFT_TXMYDETAILS, TCPFT_RXMYDETAILS,
	TCPFT_RXCONTACTS, TCPFT_TXCONTACTS, TCPFT_RXPICT, TCPFT_RXVPICT, TCPFT_TXPICT, TCPFT_RXFILE,
	TCPFT_TXFILE};

#define USERSEARCH_VNAME 1
#define USERSEARCH_VNUMBER 2
#define USERSEARCH_FIRSTNAME 4
#define USERSEARCH_LASTNAME 8
#define USERSEARCH_EMAIL 16
#define USERSEARCH_PHONENUMBER 32


struct AOL {
	int N, notificationreqop;
	AOLNUM *nums;
};

class IVPSTACK {
public:
	virtual ~IVPSTACK() {};
	virtual int Init() = 0;
	virtual int SetNotifyRoutine(void (*notify)(void *param, unsigned, unsigned, unsigned), void *param) = 0;
	virtual int SetSyncNotifyRoutine(int (*notify)(void *param, unsigned, unsigned, unsigned), void *param) = 0;
	// Create a call
	virtual VPCALL CreateCall() = 0;
	// Free a call
	virtual void FreeCall(VPCALL vpcall) = 0;
	// Attempt a connection with address, bearer bc, on success returns a VPCALL
	virtual int Connect(VPCALL *vpcall, const char *address, int bc) = 0;
	// Attempt a connection with an already allocated call
	virtual int Connect(VPCALL vpcall, const char *address, int bc) = 0;
	// Set the destination address of a call
	virtual int SetCallAddress(VPCALL vpcall, const char *address) = 0;
	// Set the source of the call
	virtual int SetCallLocalName(VPCALL vpcall, const char *username) = 0;
	virtual int SetCallLocalNumber(VPCALL vpcall, const char *number) = 0;
	// Disconnect the call
	virtual int Disconnect(VPCALL vpcall, int reason) = 0;
	// Answer a call in alerted state
	virtual int AnswerCall(VPCALL vpcall) = 0;
	virtual int Hold(VPCALL vpcall) { return VPERR_UNSUPPORTEDCALL; }
	virtual int Resume(VPCALL vpcall) { return VPERR_UNSUPPORTEDCALL; }
	virtual int AddToConference(VPCALL vpcall) { return VPERR_UNSUPPORTEDCALL; }
	virtual int RemoveFromConference(VPCALL vpcall) { return VPERR_UNSUPPORTEDCALL; }
	virtual int ConferenceAll() { return VPERR_UNSUPPORTEDCALL; }
	virtual int StopConference() { return VPERR_UNSUPPORTEDCALL; }
	virtual int AcceptConference(VPCALL vpcall, bool accept) { return VPERR_UNSUPPORTEDCALL; }

	// Conferences and call transfers, detach detaches the calls from us after setup complete (for call transfer)
	virtual int ConferenceCalls(VPCALL *vpcalls, int ncalls, bool detach) { return VPERR_UNSUPPORTEDCALL; }
	virtual int StopConference(VPCALL vpcall) { return VPERR_UNSUPPORTEDCALL; }
	virtual int DropFromConference(VPCALL vpcall) { return VPERR_UNSUPPORTEDCALL; }
	// Auto attendant
	virtual int SendAudioMessage(VPCALL vpcall, const char *file, unsigned msgid) { return VPERR_UNSUPPORTEDCALL; }
	// Servers
	virtual int SetServers(const char *list) { return 0; }
	virtual int SetNATServer(const char *addr) { return VPERR_UNSUPPORTEDCALL; }	// Find NAT type using vPhone servers
	virtual int SetSTUNServer(const char *addr) { return VPERR_UNSUPPORTEDCALL; }	// Find NAT type using STUN servers
	virtual int Logon(const char *username, const char *password) { return 0; }
	virtual int Logon(const char *serveraccesscode, const unsigned char *accountsecret) { return VPERR_UNSUPPORTEDCALL; }
	virtual int Logoff() { return 0; }
	virtual int GetLogonData(char *serveraccesscode, unsigned char *accountsecret) { return VPERR_UNSUPPORTEDCALL; }
	virtual int IsLoggedOn() { return 0; }
	virtual const char *LogonName() { return ""; }
	virtual const char *LogonNumber() { return ""; }

	// Setup
	virtual void SetSupportedBearersMask(unsigned bc_mask) {}
	virtual void SetCodec(unsigned preferredcodec, unsigned supportedcodecs) {}
	virtual void SetAudioDataFactory(VPAUDIODATAFACTORY *af) {}
	virtual void SetVideoDataFactory(VPVIDEODATAFACTORY *vf) {}

	// Misc
	virtual int SendKeypad(VPCALL vpcall, int key) = 0;
	virtual int GetStackType() = 0;
	virtual char *StackName() { return stackname; }	// Get stack name pointer
	virtual int NetworkQuality() { return VPERR_UNSUPPORTEDCALL; }
	virtual int SignalLevel(int *dBm) { return VPERR_UNSUPPORTEDCALL; }
	virtual int GetOperatorName(char *name, int namesize, unsigned *flags) { return VPERR_UNSUPPORTEDCALL; }
	virtual int GetQosData(VPCALL vpcall, QOSDATA *qos) { return VPERR_UNSUPPORTEDCALL; }
	virtual int GetVQosData(VPCALL vpcall, QOSDATA *qos) { return VPERR_UNSUPPORTEDCALL; }
	virtual int GetAudioParameters(VPCALL vpcall, int *codec, int *framesperpacket) { return VPERR_UNSUPPORTEDCALL; }
	virtual int SetCodecsMask(unsigned mask) { return VPERR_UNSUPPORTEDCALL; }
	virtual void SetBandwidth(int kbps) {}
	virtual int GetCallStatus(VPCALL vpcall) = 0;
	virtual int GetSIPReason(VPCALL vpcall) { return VPERR_UNSUPPORTEDCALL; }
	virtual int GetSIPReasonText(VPCALL vpcall, char *reasontext) { return VPERR_UNSUPPORTEDCALL; }
	virtual int SetPublicAddr(const char *addr) { return VPERR_UNSUPPORTEDCALL; }
	virtual int GetTransceiverBandwidths(int *rx, int *tx) { return VPERR_UNSUPPORTEDCALL; }
	virtual int CallForwardingRequest(int op, const char *number) { return VPERR_UNSUPPORTEDCALL; }
	virtual int GetCallForwardingStatus(int *op, char *number) { return VPERR_UNSUPPORTEDCALL; }
	virtual int MeasureBandwidth() { return VPERR_UNSUPPORTEDCALL; }
	virtual int SetNotifyInterval(int ms) { return VPERR_UNSUPPORTEDCALL; }
	virtual int SetAPNsToken(const void *token, int tokensize) { return VPERR_UNSUPPORTEDCALL; }

	// Fill vpcalls with at most maxvpcalls of calls with bearer filtered by mask
	// Returns number of calls filled, or total number of calls present if vpcalls == 0
	virtual int EnumCalls(VPCALL *vpcalls, unsigned maxvpcalls, unsigned mask) = 0;
	virtual void PrintCalls() {}

	// Get information about calls, ncalls contains the size of the vpcalls array on input,
	// the number of filled entries on output
	virtual int GetConferencePeers(VPCALL vpcall, VPCALL *vpcalls, int *ncalls) { return VPERR_UNSUPPORTEDCALL; }
	virtual int GetLocallyConferencedCalls(VPCALL vpcall, VPCALL *vpcalls, int *ncalls) { return VPERR_UNSUPPORTEDCALL; }
	// Get the call that will be disconnected when vpcall is disconnected; when linkedcall
	// becomes established, it is connected to vpcall and then the two calls detached from us
	virtual int GetLinkedCall(VPCALL vpcall, VPCALL *linkedcall) { return VPERR_UNSUPPORTEDCALL; }
	virtual int GetCallRemoteName(VPCALL vpcall, char *username) = 0;
	virtual int GetCallRemoteNumber(VPCALL vpcall, char *number) = 0;
	virtual int GetCallRemoteAddress(VPCALL vpcall, char *address) = 0;
	virtual int GetCallRemoteSubAddr(VPCALL vpcall, char *subaddr) = 0;
	virtual int GetCallRemoteServerAccessCode(VPCALL vpcall, char *serveraccesscode) { return VPERR_UNSUPPORTEDCALL; }
	virtual int GetCallLocalName(VPCALL vpcall, char *username) = 0;
	virtual int GetCallLocalNumber(VPCALL vpcall, char *number) = 0;
	virtual int GetCallLocalSubAddr(VPCALL vpcall, char *subaddr) = 0;
	virtual int GetCallBearer(VPCALL vpcall) = 0;
	virtual int GetCallCodec(VPCALL vpcall) = 0;
	virtual int GetCallTimes(VPCALL vpcall, time_t *start_time, unsigned *length_ms) = 0;
	virtual int IsCallOffline(VPCALL vpcall) { return VPERR_UNSUPPORTEDCALL; }	// Is call directed towards vPData (answering machine)
	virtual int IsCallWithProxy(VPCALL vpcall) { return VPERR_UNSUPPORTEDCALL; }	// Does the call use a proxy
	virtual int IsHeld(VPCALL vpcall) { return VPERR_UNSUPPORTEDCALL; }	// 1 = held, 2 = remotely held, 3 = both
	virtual int IsConferenced(VPCALL vpcall) { return VPERR_UNSUPPORTEDCALL; }	// 1 = conferenced, 2 = remotely conferenced, 3 = both
	virtual int SetCallDisconnectMessage(VPCALL vpcall, const char *text) { return VPERR_UNSUPPORTEDCALL; }
	// SMS data
	virtual int GetMissedCallsData(VPCALL vpcall, unsigned *missedcallscount, unsigned *missedcalltime, unsigned *missedcallbc) { return VPERR_UNSUPPORTEDCALL; }
	virtual int GetCallText(VPCALL vpcall, char *text) { *text = 0; return VPERR_UNSUPPORTEDCALL; }
	virtual int SetCallText(VPCALL vpcall, const char *text) { return VPERR_UNSUPPORTEDCALL; }
	virtual int SetMissedCallBC(VPCALL vpcall, int bc) { return VPERR_UNSUPPORTEDCALL; }
	virtual int SetSMSType(VPCALL vpcall, unsigned type) { return VPERR_UNSUPPORTEDCALL; }
	virtual int GetSMSType(VPCALL vpcall, unsigned *type) { return VPERR_UNSUPPORTEDCALL; }
	virtual int SetSMSId(VPCALL vpcall, unsigned id) { return VPERR_UNSUPPORTEDCALL; }
	virtual int GetSMSId(VPCALL vpcall, unsigned *id) { return VPERR_UNSUPPORTEDCALL; }

	virtual int	SendAudio(VPCALL vpcall, int codec, unsigned timestamp, const void *buf, int len, bool wait = false) = 0;
	virtual int SendVideo(VPCALL vpcall, unsigned short timestamp, void *data, int size, bool keyframe) { return VPERR_UNSUPPORTEDCALL; }

	// Settings
	virtual void SetBindAddr(long bindaddr) {}
	virtual void SetBindPort(int port) {}
	virtual int GetBindPort() { return VPERR_UNSUPPORTEDCALL; }
	virtual void SetUserStatus(int userstatus) {}
	virtual int GetUserStatus() { return VPERR_UNSUPPORTEDCALL; }

	// Vphone protocol specific
	virtual int AskOnline(AOL *aol) { return VPERR_UNSUPPORTEDCALL; }
	virtual int QueryAccountInfo() { return VPERR_UNSUPPORTEDCALL; }
	virtual int ServerTransfer(int op, const char *path, bool amactive, const char *vname = 0) { return VPERR_UNSUPPORTEDCALL; }
	virtual int UserSearch(const char *name, const char *country, unsigned flags, unsigned param) { return VPERR_UNSUPPORTEDCALL; }

	// Chat
	virtual int SendChat(VPCALL vpcall, const char *text) { return VPERR_UNSUPPORTEDCALL; }
	virtual int GetChatText(VPCALL vpcall, char *text, int textsize) { return VPERR_UNSUPPORTEDCALL; }

	// File transfer
	virtual int SendFile(VPCALL vpcall, const char *path) { return VPERR_UNSUPPORTEDCALL; }
	virtual int AbortFileTransfer(VPCALL vpcall) { return VPERR_UNSUPPORTEDCALL; }
	virtual int AcceptFile(VPCALL vpcall, int accept) { return VPERR_UNSUPPORTEDCALL; }
	virtual int GetCallFilePath(VPCALL vpcall, char *path) { return VPERR_UNSUPPORTEDCALL; }
	virtual int SetCallFilePath(VPCALL vpcall, const char *path) { return VPERR_UNSUPPORTEDCALL; }
	virtual int SetCallNFiles(VPCALL vpcall, unsigned nfiles, unsigned nbytes) { return VPERR_UNSUPPORTEDCALL; }
	virtual int GetCallNFiles(VPCALL vpcall, unsigned *nfiles, unsigned *nbytes) { return VPERR_UNSUPPORTEDCALL; }
	virtual int GetFileProgress(VPCALL vpcall, unsigned *current, unsigned *total) { return VPERR_UNSUPPORTEDCALL; }

	// Video
	virtual int SetDefaultVideoParameters(unsigned fourcc, unsigned width, unsigned height, unsigned framerate, unsigned quality) { return VPERR_UNSUPPORTEDCALL; }
	virtual int GetDefaultVideoParameters(unsigned *fourcc, unsigned *width, unsigned *height, unsigned *framerate, unsigned *quality) { return VPERR_UNSUPPORTEDCALL; }
	virtual int SetCallVideoParameters(VPCALL vpcall, unsigned fourcc, unsigned width, unsigned height, unsigned framerate, unsigned quality) { return VPERR_UNSUPPORTEDCALL; }
	virtual int GetCallVideoParameters(VPCALL vpcall, unsigned *fourcc, unsigned *width, unsigned *height, unsigned *framerate, unsigned *quality) { return VPERR_UNSUPPORTEDCALL; }
	virtual int RecordAVIFile(const char *path) { return VPERR_UNSUPPORTEDCALL; }
	virtual int StopAVIRecord() { return VPERR_UNSUPPORTEDCALL; }
	virtual int RecordWAVFile(const char *path) { return VPERR_UNSUPPORTEDCALL; }
	virtual int StopWAVRecord() { return VPERR_UNSUPPORTEDCALL; }
	virtual int GetRecorderCall(VPCALL *vpcall) { return VPERR_UNSUPPORTEDCALL; }
	virtual int SetVideoWindowData(VPCALL vpcall, HWINDOW hParent, int childid, const RECTANGLE *rect, bool hidden) { return VPERR_UNSUPPORTEDCALL; }
	virtual int SetVideoWindowFullScreen(VPCALL vpcall, int fs) { return VPERR_UNSUPPORTEDCALL; }
	virtual int CaptureVideoImage(VPCALL vpcall, void **image, int *w, int *h) { return VPERR_UNSUPPORTEDCALL; }

	char stackname[MAXNAMELEN+1];
};

class IAVIPLAYER {
public:
	virtual ~IAVIPLAYER() {}
	virtual int PlayAVIFile(const char *path) = 0;
	virtual int PlayWAVFile(const char *path, int loop) = 0;
	virtual int Stop() = 0;
	virtual int Progress() = 0;	// In thousands
	virtual int WaitEnd() = 0;
	virtual int SetWindowData(HWINDOW hWnd, const RECTANGLE *rect) = 0;
	virtual int SetVideoControl(int control, int value) = 0;
	virtual int SetAudioDevice(const char *dev) = 0;
	virtual int Pause(int pause) = 0;
};

// Implementations
// Stacks
IVPSTACK *CreateVPSTACK(VPAUDIODATAFACTORY *vpaf = 0, const char *stackname = 0, VPVIDEODATAFACTORY *vpvf = 0);
IVPSTACK *CreateSIPSTACK(VPAUDIODATAFACTORY *vpaf = 0, const char *stackname = 0);
IVPSTACK *CreateISDNSTACK(VPAUDIODATAFACTORY *vpaf = 0, const char *stackname = 0);
IVPSTACK *CreateGSMSTACK(VPAUDIODATAFACTORY *vpaf = 0, const char *stackname = 0);

// Misc
IAVIPLAYER *CreateAVIPLAYER();

// Audio
int VPMIXINGAUDIO_Init();
int VPMIXINGAUDIO_GetVuMeters(int *in_cent_of_dB, int *out_cent_of_dB);
int VPMIXINGAUDIO_SetLevel(int recording, int level);
int VPMIXINGAUDIO_SetMicBoost(int on);
int VPMIXINGAUDIO_EnumerateDevices(char devices[][100], int *ndevices, int recording);
int VPMIXINGAUDIO_SetAutoThreshold(int thre);
int VPMIXINGAUDIO_Mute(int m);
int VPMIXINGAUDIO_SetAudioDevice(int recording, const char *device);
VPAUDIODATA *CreateVPMIXINGAUDIO(IVPSTACK *vps, VPCALL vpcall, int codec);
#ifdef TARGET_OS_IPHONE                                                                                                     
bool VPMIXINGAUDIO_GetMute();
void VPMIXINGAUDIO_SetMute(bool AMute);
bool VPMIXINGAUDIO_GetSpeaker();
void VPMIXINGAUDIO_SetSpeaker(bool);
void VPMIXINGAUDIO_Stop();
#endif   

#ifdef UNICODE
int VPMIXINGAUDIO_SetAudioDevice(int recording, const tchar *device);
int VPMIXINGAUDIO_EnumerateDevices(tchar devices[][100], int *ndevices, int recording);
#endif

enum {AES_NONE,	// No acoustic echo suppression
	AES_LOW,	// Acoustic echo suppression, low aggressiveness, for handset
	AES_HIGH};	// Acoustic echo suppression, high aggressiveness, for hands free

enum {AEC_NONE,	// No acoustic echo cancellation
	AEC_SHORT = 1024,	// Short tail acoustic echo cancellation, when audioout->audioin (acoustic+lost in OS) delay is low, 1024/8000 seconds
	AEC_LONG = 4096};	// Long tail acoustic echo cancellation, when audioout->audioin (acoustic+lost in OS) delay is high, 4096/8000 seconds

int VPSIMPLEAUDIO_GetVuMeters(int *in_cent_of_dB, int *out_cent_of_dB);
int VPSIMPLEAUDIO_SetLevel(int recording, int level);
int VPSIMPLEAUDIO_SetMicBoost(int on);
int VPSIMPLEAUDIO_EnumerateDevices(tchar devices[][100], int *ndevices, int recording);
int VPSIMPLEAUDIO_SetAutoThreshold(int thre);
int VPSIMPLEAUDIO_Mute(int m);
int VPMIXINGAUDIO_SetAcousticEchoSuppression(int type);
int VPMIXINGAUDIO_SetAcousticEchoCancellation(int type);
int VPMIXINGAUDIO_RecordToFile(const char *path);
VPAUDIODATA *CreateVPSIMPLEAUDIO(IVPSTACK *vps, VPCALL vpcall, int codec);

// Video
int VPVIDEO_Init();
int VPVIDEO_SetCaptureDevice(const char *device);
int VPVIDEO_EnumerateCaptureDevices(char devices[][100], int *ndevices);
VPVIDEODATA *CreateVPVIDEO(IVPSTACK *vps, VPCALL vpcall, unsigned fourccrx, unsigned xresrx, unsigned yresrx, unsigned fourcc, unsigned xres, unsigned yres, unsigned framerate, unsigned quality);

#ifdef UNICODE
int VPVIDEO_SetCaptureDevice(const tchar *device);
int VPVIDEO_EnumerateCaptureDevices(tchar devices[][100], int *ndevices);
#endif

class VPMIXINGAUDIOFACTORY : public VPAUDIODATAFACTORY
{
public:
	VPAUDIODATA *New(IVPSTACK *vps, VPCALL vpcall, int codec) { return CreateVPMIXINGAUDIO(vps, vpcall, codec); }
};

class VPSIMPLEAUDIOFACTORY : public VPAUDIODATAFACTORY
{
public:
	VPAUDIODATA *New(IVPSTACK *vps, VPCALL vpcall, int codec) { return CreateVPSIMPLEAUDIO(vps, vpcall, codec); }
};

class VPVIDEOFACTORY : public VPVIDEODATAFACTORY
{
public:
	VPVIDEODATA *New(IVPSTACK *vps, VPCALL vpcall, unsigned fourccrx, unsigned xresrx, unsigned yresrx, unsigned fourcc, unsigned xres, unsigned yres, unsigned framerate, unsigned quality);
};


extern char stackhomedir[MAXPATH];
int VPInit();
int VPFreeBuffer(void *buffer);

#ifdef _WIN32
#pragma warning( pop )
#endif

#endif

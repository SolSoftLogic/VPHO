#ifndef _CAPI_H_INCLUDED_
#define _CAPI_H_INCLUDED_

#define CAPISUBCMD_REQ 0x80
#define CAPISUBCMD_CONF 0x81
#define CAPISUBCMD_IND 0x82
#define CAPISUBCMD_RESP 0x83

#define CAPICMD_ALERT 0x01	// Only REQ
#define CAPICMD_CONNECT 0x02
#define CAPICMD_CONNECT_ACTIVE 0x03	// Only IND
#define CAPICMD_CONNECT_B3_ACTIVE 0x83	// Only IND
#define CAPICMD_CONNECT_B3 0x82
#define CAPICMD_DATA_B3 0x86
#define CAPICMD_DISCONNECT_B3 0x84
#define CAPICMD_DISCONNECT 0x04
#define CAPICMD_FACILITY 0x80
#define CAPICMD_INFO 0x08
#define CAPICMD_LISTEN 0x05 // Only REQ
#define CAPICMD_MANUFACTURER 0xff
#define CAPICMD_RESET_B3 0x87
#define CAPICMD_SELECT_B_PROTOCOL 0x41

#define BPROTOCOL_HDLC 0x000100
#define BPROTOCOL_TRANSPARENT 0x000101
#define BPROTOCOL X75 0x020000

#define CIP_VOICE 1
#define CIP_UDI 2
#define CIP_3KHZAUDIO 4
#define CIP_ISDNAUDIO 16

#define CAPISS_SUPPORT 0
#define CAPISS_LISTEN 1
#define CAPISS_HOLD 2
#define CAPISS_RETRIEVE 3
#define CAPISS_SUSPEND 4
#define CAPISS_RESUME 5
#define CAPISS_ECT 6
#define CAPISS_3PTYBEGIN 7
#define CAPISS_3PTYEND 8
#define CAPISS_CFACTIVATE 9
#define CAPISS_CFDEACTIVATE 10
#define CAPISS_CFIPARAMETERS 11
#define CAPISS_CFINUMBERS 12
#define CAPISS_CD 13

#define CAPISSS_HOLDRETRIEVE 1
#define CAPISSS_TP 2
#define CAPISSS_ECT 4
#define CAPISSS_3PTY 8
#define CAPISSS_CF 0x10
#define CAPISSS_CD 0x20
#define CAPISSS_MCID 0x40
#define CAPISSS_CCBS 0x80
#define CAPISSS_MWI 0x100
#define CAPISSS_CCNR 0x200

#define MAXNUMLEN 100
#define MAXUSERINFOLEN 140
#define MAXLINES 64

#define CB_BASE 0
#define CB_CONNECTERROR	(CB_BASE)
#define CB_INCOMINGCALL	(CB_BASE+1)
#define CB_CONNECTEDEB3	(CB_BASE+2)
#define CB_CONNECTED	(CB_BASE+3)
#define CB_DISCONNECTED	(CB_BASE+4)
#define CB_USERDISCONNECTED	(CB_BASE+5)
#define CB_CALLPROGRESS	(CB_BASE+6)
#define CB_DATA			(CB_BASE+7)
#define CB_SSERROR		(CB_BASE+8)
#define CB_HELD			(CB_BASE+9)
#define CB_RECONNECTED	(CB_BASE+10)
#define CB_3PTYBEGIN	(CB_BASE+11)
#define CB_3PTYEND		(CB_BASE+12)
#define CB_ECT			(CB_BASE+13)
#define CB_SUSPENDED	(CB_BASE+14)
#define CB_MISSED		(CB_BASE+15)
#define CB_CHARGINGINFO	(CB_BASE+16)
#define CB_DISPLAYINFO	(CB_BASE+17)
#define CB_USERUSERINFO	(CB_BASE+18)
#define CB_ALERTCONF    (CB_BASE+19)

#define CAPIFLAG_EARLYB3 1
#define CAPIFLAG_SCREEN 2

enum {CS_IDLE,
CSC_CONNECTINITIATED, CSC_RESUME, CSC_ALERTING,
CSC_PLCIVALID, CSC_ALERTINGNCCIVALID, CSC_NCCIVALID, CSC_NCCIVALIDFROMHELD, CSC_CONNECTEDEB3, CSC_CONNECTED, CSC_3PTY, CSC_HELD, CSC_PHELD,
CSL_ALERTING, CSL_PLCIVALID, CSL_NCCIVALID, CSL_NCCIVALIDFROMHELD, CSL_CONNECTED, CSL_3PTY, CSL_HELD, CSL_PHELD, CSD_B3,
CSC_ECT, CSL_ECT, CSC_SUSPEND, CSL_SUSPEND, CS_RESERVED
};

typedef struct {

	// To application
	char *callednumber, *callingnumber, *LLC, clir;
	WORD CIP;
	DWORD PLCI;

	// From application
	WORD reject;
	DWORD protocol;
	int alert, colr;

	// Both
	char *userinfo;
	int userinfolen;
	int line;
} INCOMINGCALLDATA;

#ifdef __cplusplus
extern "C" {
#endif

// Library
int CAPI_Load();
void CAPI_Unload();
int CAPI_BaseInit(int maxlogicalchannels, int maxbufsize);
int CAPI_BaseExit();
int CAPI_AlertReq(int handle, DWORD PLCI);
int CAPI_AlertReqUUS1(int handle, DWORD PLCI, char *uus1, int uus1len);
int CAPI_ConnectReq(int handle, int controller, int CIP, int bprotocol, char *number, char *ownnumber, char *LLC, int screen, char *userinfo);
int CAPI_ConnectResp(int handle, DWORD PLCI, int Reject, int bprotocol, char *connectednumber, int screen);
int CAPI_ConnectActiveResp(int handle, DWORD PLCI);
int CAPI_ConnectB3ActiveResp(int handle, DWORD NCCI);
int CAPI_ConnectB3Req(int handle, DWORD PLCI);
int CAPI_ConnectB3Resp(int handle, DWORD NCCI);
int CAPI_DataB3Req(int handle, DWORD NCCI, void *data, int datalen, int datahandle);
int CAPI_DataB3Resp(int handle, DWORD NCCI, int datahandle);
int CAPI_DisconnectB3Req(int handle, DWORD NCCI);
int CAPI_DisconnectB3Resp(int handle, DWORD NCCI);
int CAPI_DisconnectReq(int handle, DWORD PLCI);
int CAPI_DisconnectResp(int handle, DWORD PLCI);
int CAPI_InfoReq(int handle, DWORD PLCI, char *callednumber, BYTE *additionalinfo, int additionalinfolen);
int CAPI_InfoResp(int handle, DWORD PLCI);
int CAPI_ListenReq(int handle, int controller, DWORD infomask, DWORD CIPmask);
int CAPI_FacilityReq(int handle, DWORD NCCI, int function, DWORD param, char *number);
int CAPI_FacilityResp(int handle, DWORD NCCI, int function);
int CAPI_SelectBProtocolReq(int handle, DWORD PLCI, int bprotocol);
const char *CAPI_SSFunctionString(int function);
const char *CAPI_ErrorString(int error);

// Call management callbacks
void CAPI_AlertConf(int handle, DWORD PLCI, WORD info);
void CAPI_ConnectConf(int handle, DWORD PLCI, WORD info);
void CAPI_ConnectB3Conf(int handle, DWORD NCCI, WORD info);
void CAPI_DataB3Conf(int handle, DWORD NCCI, WORD info, int datahandle);
void CAPI_DisconnectB3Conf(int handle, DWORD NCCI, WORD info);
void CAPI_DisconnectConf(int handle, DWORD PLCI, WORD info);
void CAPI_FacilityConf(int handle, DWORD NCCI, WORD info, WORD function, WORD ssinfo, DWORD supported);
void CAPI_ListenConf(int handle, DWORD controller, WORD info);
void CAPI_SelectBProtocolConf(int handle, DWORD PLCI, WORD info);
void CAPI_ConnectInd(int handle, DWORD PLCI, WORD CIP, char *callednumber, char *callingnumber, char *LLC, int clir, char *userinfo, int userinfolen);
void CAPI_ConnectActiveInd(int handle, DWORD PLCI, char *connectednumber, char *LLC);
void CAPI_ConnectB3ActiveInd(int handle, DWORD NCCI);
void CAPI_ConnectB3Ind(int handle, DWORD NCCI);
void CAPI_DataB3Ind(int handle, DWORD NCCI, void *data, int datalen, int datahandle);
void CAPI_DisconnectB3Ind(int handle, DWORD NCCI, WORD reason);
void CAPI_DisconnectInd(int handle, DWORD PLCI, WORD reason);
void CAPI_FacilityInd(int handle, DWORD NCCI, int function, WORD info);
void CAPI_InfoInd(int handle, DWORD PLCI, WORD infonumber, BYTE *infoelement, int infoelementlen);

// Call management
int CAPI_Init(int maxbufsize, int earlyb3);
int CAPI_Exit();
DWORD CAPI_SSSupport(int controller);
int CAPI_CallBack(unsigned line, unsigned msg, DWORD param1, DWORD param2);
int CAPI_Dial(unsigned line, int controller, int CIP, int bprotocol, char *number, char *ownnumber, char *LLC, unsigned flags, char *userinfo);
int CAPI_Disconnect(unsigned line);
int CAPI_Accept(unsigned line, int reject, int protocol, char *ownnumber, int screen);
int CAPI_SendData(unsigned line, void *data, int datalen);
int CAPI_Hold(unsigned line);
int CAPI_Retrieve(unsigned line);
int CAPI_3PtyBegin(unsigned activeline, unsigned heldline);
int CAPI_3PtyEnd(unsigned line);
int CAPI_ECT(unsigned line1, unsigned line2);
int CAPI_Suspend(unsigned line, char *callid);
int CAPI_Resume(unsigned line, int controller, int bprotocol, char *callid);
int CAPI_CD(unsigned line, char *number, int screen);
int CAPI_WaitingFacilityInd(unsigned line);
int CAPI_ReserveLine(unsigned line, int reserve);
int CAPI_AvailableLines();
int CAPI_FindFreeLine();
int CAPI_KeyPad(unsigned line, char *keypad);
int CAPI_OverlapNumber(unsigned line, char *number);

#ifdef __cplusplus
}
#endif

#endif

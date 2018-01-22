#include <windows.h>
#include <stdio.h>
#include <process.h>
#include <malloc.h>
#include "capi.h"
#include "neterror.h"

/****************************************/
/* First part, library                  */
/****************************************/

static DWORD Glob_ApplId;
static HINSTANCE hCapiDll;
static int available_b_channels;
static DWORD (APIENTRY * CAPIRegister) (DWORD MessageBufferSize,
							  DWORD maxLogicalConnection,
							  DWORD maxBDataBlocks,
							  DWORD maxBDataLen,
							  DWORD *pApplID);
static DWORD (APIENTRY * CAPIRelease) (DWORD ApplID);
static DWORD (APIENTRY * CAPIPutMessage) (DWORD ApplID,PVOID pCAPIMessage);
static DWORD (APIENTRY * CAPIGetMessage) (DWORD ApplID,PVOID * ppCAPIMessage);
static DWORD (APIENTRY * CAPIWaitForSignal) (DWORD ApplID);
static VOID (APIENTRY * CAPIGetManufacturer) (char * SzBuffer);
static DWORD (APIENTRY * CAPIGetVersion) (DWORD * pCAPIMajor,
								   DWORD * pCAPIMinor,
								   DWORD * pManufacturerMajor,
								   DWORD * pManufacturerMinor);
static DWORD (APIENTRY * CAPIGetSerialNumber) (char * SzBuffer);
static DWORD (APIENTRY * CAPIGetProfile) (PVOID SzBuffer,DWORD CtrlNr);
static DWORD (APIENTRY * CAPIInstalled) (VOID);

static int CAPI_Load()
{
	if(hCapiDll)
		return 0;
	if(!(hCapiDll = LoadLibrary("CAPI2032.DLL")))
		return -1;
	CAPIRegister = (void *)GetProcAddress(hCapiDll, (LPSTR)1);
	CAPIRelease = (void *)GetProcAddress(hCapiDll, (LPSTR)2);
	CAPIPutMessage = (void *)GetProcAddress(hCapiDll, (LPSTR)3);
	CAPIGetMessage = (void *)GetProcAddress(hCapiDll, (LPSTR)4);
	CAPIWaitForSignal = (void *)GetProcAddress(hCapiDll, (LPSTR)5);
	CAPIGetManufacturer = (void *)GetProcAddress(hCapiDll, (LPSTR)6);
	CAPIGetVersion = (void *)GetProcAddress(hCapiDll, (LPSTR)7);
	CAPIGetSerialNumber = (void *)GetProcAddress(hCapiDll, (LPSTR)8);
	CAPIGetProfile = (void *)GetProcAddress(hCapiDll, (LPSTR)9);
	CAPIInstalled = (void *)GetProcAddress(hCapiDll, (LPSTR)10);
	return 0;
}

static void CAPI_Unload()
{
	if(hCapiDll)
	{
		CAPIRegister = 0;
		CAPIRelease = 0;
		CAPIPutMessage = 0;
		CAPIGetMessage = 0;
		CAPIWaitForSignal = 0;
		CAPIGetManufacturer = 0;
		CAPIGetVersion = 0;
		CAPIGetSerialNumber = 0;
		CAPIGetProfile = 0;
		CAPIInstalled = 0;
		FreeLibrary(hCapiDll);
		hCapiDll = 0;
	}
}

int CAPI_BaseInit(int maxlogicalchannels, int maxbufsize)
{
	unsigned char buffer[64];
	int controllers, i;

	if(CAPI_Load())
		return -1;
	if(CAPIInstalled())
		return -2;
	if(CAPIGetProfile(buffer, 0))
		return -3;
	if(!*buffer)
		return -4;	// No controllers
	controllers = *(unsigned short *)buffer;
	if(CAPIRegister(1024 + 1024 * maxlogicalchannels, maxlogicalchannels, 7, maxbufsize, &Glob_ApplId))
		return -5;
	if(CAPIGetProfile(buffer, 1))
		return -3;
	available_b_channels = 0;
	for(i = 0; i < controllers; i++)
		available_b_channels += *(unsigned short *)(buffer + 2);
	return *buffer;
}

int CAPI_BaseExit()
{
	CAPIRelease(Glob_ApplId);
	CAPI_Unload();
	return 0;
}

static int CAPI_PutMessage(int handle, int Command, int SubCommand, void *Params, int ParamsLen)
{
	BYTE msg[256];

	*(WORD *)msg = (WORD)(ParamsLen + 8);
	*(WORD *)(msg + 2) = (WORD)Glob_ApplId;
	msg[4] = (BYTE)Command;
	msg[5] = (BYTE)SubCommand;
	*(WORD *)(msg + 6) = (WORD)(handle);
	memcpy(msg + 8, Params, ParamsLen);
	if(available_b_channels)
		return CAPIPutMessage(Glob_ApplId, msg);
	else return -1;
}

int CAPI_AlertReq(int handle, DWORD PLCI)
{
	BYTE params[5];

	*(DWORD *)params = PLCI;
	params[4] = 0;
	return CAPI_PutMessage(handle, CAPICMD_ALERT, CAPISUBCMD_REQ, params, 5);
}

int CAPI_AlertReqUUS1(int handle, DWORD PLCI, char *uus1, int uus1len)
{
	BYTE params[256];

	*(DWORD *)params = PLCI;
	params[4] = (BYTE)(uus1len + 6);
	params[5] = 0;
	params[6] = 0;
	params[7] = (BYTE)uus1len;
	memcpy(params + 8, uus1, uus1len);
	params[8 + uus1len] = 0;
	params[9 + uus1len] = 0;
	return CAPI_PutMessage(handle, CAPICMD_ALERT, CAPISUBCMD_REQ, params, 10 + uus1len);
}

int CAPI_ConnectReq(int handle, int controller, int CIP, int bprotocol, char *number, char *ownnumber, char *LLC, int screen, char *userinfo)
{
	BYTE Params[256], *p;
	int msglen = 0;
	char number2[100], callingnumber[100], callednumber[100], callingsubaddress[100], calledsubaddress[100], keypad[100];

	if(p = strchr(number, ':'))
	{
		memcpy(number2, number, p - number);
		number2[p - number] = 0;
		strcpy(keypad, p + 1);
	} else {
		strcpy(number2, number);
		*keypad = 0;
	}
	if(p = strchr(number2, '.'))
	{
		memcpy(callednumber, number2, p - number2);
		callednumber[p - number2] = 0;
		strcpy(calledsubaddress, p + 1);
	} else {
		strcpy(callednumber, number2);
		*calledsubaddress = 0;
	}
	if(ownnumber)
	{
		if(p = strchr(ownnumber, '.'))
		{
			memcpy(callingnumber, ownnumber, p - ownnumber);
			callingnumber[p - ownnumber] = 0;
			strcpy(callingsubaddress, p + 1);
		} else {
			strcpy(callingnumber, ownnumber);
			*callingsubaddress = 0;
		}
	} else {
		*callingnumber = 0;
		*callingsubaddress = 0;
	}
	msglen = 8 + strlen(callednumber);
	*(DWORD *)Params = controller;	// Controller
	*(WORD *)(Params + 4) = CIP;	// CIP
	Params[6] = strlen(callednumber) + 1;	// Called party struct length
	Params[7] = 0x80;				// Numbering plane identification
	memcpy(Params + 8, callednumber, strlen(callednumber));	// Number
	p = Params + 8 + strlen(callednumber);
	if(*callingnumber)
	{
		p[0] = strlen(callingnumber) + 2;
		p[1] = 0x00;
		p[2] = screen ? 0xa0 : 0x80;
		strcpy(p + 3, callingnumber);
		msglen += p[0] + 1;
		p += p[0] + 1;
	} else if(screen)
	{
		p[0] = 2;
		p[1] = 0x00;
		p[2] = 0xa0;
		p += 3;
		msglen += 3;
	} else {
		*p++ = 0;				// Calling number
		msglen++;
	}
	if(*calledsubaddress)
	{
		p[0] = strlen(calledsubaddress) + 2;
		p[1] = 0x80;
		p[2] = 0x50;
		strcpy(p + 3, calledsubaddress);
		msglen += p[0] + 1;
		p += p[0] + 1;
	} else {
		*p++ = 0;
		msglen++;
	}
	if(*callingsubaddress)
	{
		p[0] = strlen(callingsubaddress) + 2;
		p[1] = 0x80;
		p[2] = 0x50;
		strcpy(p + 3, callingsubaddress);
		msglen += p[0] + 1;
		p += p[0] + 1;
	} else {
		*p++ = 0;
		msglen++;
	}
	p[0] = 9;						// B protocol struct length
	*(WORD *)(p + 1) = bprotocol & 0xff;
	*(WORD *)(p + 3) = (bprotocol >> 8) & 0xff;
	*(WORD *)(p + 5) = (bprotocol >> 16) & 0xff;
	p[7] = 0;						// No configuration 1
	p[8] = 0;						// No configuration 2
	p[9] = 0;						// No configuration 3
	p[10] = 0;						// No bearer capability
	p += 11;
	msglen += 11;
	if(LLC && *LLC)
	{
		p[0] = strlen(LLC);
		if(p[0] == 1)
			p[0] = 2;
		strcpy(p + 1, LLC);
		msglen += p[0] + 1;
		p += p[0] + 1;
	} else {
		*p++ = 0;
		msglen++;
	}
	*p++ = 0;					// no HLC
	msglen++;
	if(userinfo && *userinfo || *keypad)
	{
		p[1] = 0;
		if(*keypad)
		{
			p[2] = strlen(keypad);
			memcpy(p + 3, keypad, p[2]);
		} else p[2] = 0;
		if(userinfo && *userinfo)
		{
			p[3 + p[2]] = strlen(userinfo) + 1;
			p[0] = p[2] + p[3 + p[2]] + 4;
			p[4 + p[2]] = 4;	// IA 5 digits
			strcpy(p + 5 + p[2], userinfo);
			p[4 + p[3 + p[2]] + p[2]] = 0;
			p[5 + p[3 + p[2]] + p[2]] = 0;
		} else {
			p[3 + p[2]] = 0;
			p[4 + p[2]] = 0;
			p[5 + p[2]] = 0;
			p[0] = p[2] + 5;
		}
		msglen += p[0] + 1;
	} else {
		*p++ = 0;
		msglen++;
	}
	return CAPI_PutMessage(handle, CAPICMD_CONNECT, CAPISUBCMD_REQ, Params, msglen);
}

int CAPI_ConnectResp(int handle, DWORD PLCI, int reject, int bprotocol, char *connectednumber, int screen)
{
	BYTE params[256], *p;

	*(DWORD *)params = PLCI;
	*(WORD *)(params + 4) = (WORD)reject;
	if(reject)
	{
		memset(params + 6, 0, 5);
		return CAPI_PutMessage(handle, CAPICMD_CONNECT, CAPISUBCMD_RESP, params, 11);
	}
	params[6] = 10;						// B protocol struct length
	p = params + 7;
	*(WORD *)p = bprotocol & 0xff;
	*(WORD *)(p + 2) = (bprotocol >> 8) & 0xff;
	*(WORD *)(p + 4) = (bprotocol >> 16) & 0xff;
	p[6] = p[7] = p[8] = p[9] = 0;
	p += 10;
	if(connectednumber && *connectednumber)
	{
		p[0] = strlen(connectednumber) + 2;
		p[1] = 0x00;
		p[2] = screen ? 0xa0 : 0x80;
		strcpy(p + 3, connectednumber);
		p += p[0] + 1;
	} else if(screen)
	{
		p[0] = 2;
		p[1] = 0x00;
		p[2] = 0xa0;
		p += 3;
	} else *p++ = 0;
	p[0] = p[1] = p[2] = 0;
	p += 3;
	return CAPI_PutMessage(handle, CAPICMD_CONNECT, CAPISUBCMD_RESP, params, p - params);
}

int CAPI_ConnectActiveResp(int handle, DWORD PLCI)
{
	return CAPI_PutMessage(handle, CAPICMD_CONNECT_ACTIVE, CAPISUBCMD_RESP, &PLCI, 4);
}

int CAPI_ConnectB3ActiveResp(int handle, DWORD NCCI)
{
	return CAPI_PutMessage(handle, CAPICMD_CONNECT_B3_ACTIVE, CAPISUBCMD_RESP, &NCCI, 4);
}

int CAPI_ConnectB3Req(int handle, DWORD PLCI)
{
	BYTE params[5];

	*(DWORD *)params = PLCI;
	params[4] = 0;
	return CAPI_PutMessage(handle, CAPICMD_CONNECT_B3, CAPISUBCMD_REQ, params, 5);
}

int CAPI_ConnectB3Resp(int handle, DWORD NCCI)
{
	BYTE params[7];

	*(DWORD *)params = NCCI;
	params[4] = params[5] = params[6] = 0;
	return CAPI_PutMessage(handle, CAPICMD_CONNECT_B3, CAPISUBCMD_RESP, params, 7);
}

int CAPI_DataB3Req(int handle, DWORD NCCI, void *data, int datalen, int datahandle)
{
	BYTE params[14];

	*(DWORD *)params = NCCI;
	*(DWORD *)(params + 4) = (DWORD)data;
	*(WORD *)(params + 8) = (WORD)datalen;
	*(WORD *)(params + 10) = (WORD)datahandle;
	*(WORD *)(params + 12) = 0;
	return CAPI_PutMessage(handle, CAPICMD_DATA_B3, CAPISUBCMD_REQ, params, 14);
}

int CAPI_DataB3Resp(int handle, DWORD NCCI, int datahandle)
{
	BYTE params[6];

	*(DWORD *)params = NCCI;
	*(WORD *)(params + 4) = (WORD)datahandle;
	return CAPI_PutMessage(handle, CAPICMD_DATA_B3, CAPISUBCMD_RESP, params, 6);
}

int CAPI_DisconnectB3Req(int handle, DWORD NCCI)
{
	BYTE params[5];

	*(DWORD *)params = NCCI;
	params[4] = 0;
	return CAPI_PutMessage(handle, CAPICMD_DISCONNECT_B3, CAPISUBCMD_REQ, params, 5);
}

int CAPI_DisconnectB3Resp(int handle, DWORD NCCI)
{
	return CAPI_PutMessage(handle, CAPICMD_DISCONNECT_B3, CAPISUBCMD_RESP, &NCCI, 4);
}

int CAPI_DisconnectReq(int handle, DWORD PLCI)
{
	BYTE params[5];

	*(DWORD *)params = PLCI;
	params[4] = 0;
	return CAPI_PutMessage(handle, CAPICMD_DISCONNECT, CAPISUBCMD_REQ, params, 5);
}

int CAPI_DisconnectResp(int handle, DWORD PLCI)
{
	return CAPI_PutMessage(handle, CAPICMD_DISCONNECT, CAPISUBCMD_RESP, &PLCI, 4);
}

int CAPI_InfoReq(int handle, DWORD PLCI, char *callednumber, BYTE *additionalinfo, int additionalinfolen)
{
	BYTE params[100];

	*(DWORD *)params = PLCI;
	if(callednumber)
	{
		params[4] = strlen(callednumber) + 1;
		params[5] = 0x80;
		memcpy(params + 6, callednumber, params[4]);
	} else params[4] = 0;
	if(additionalinfolen)
	{
		params[5 + params[4]] = additionalinfolen;
		memcpy(params + 6 + params[4], additionalinfo, additionalinfolen);
	} else params[5 + params[4]] = 0;
	return CAPI_PutMessage(handle, CAPICMD_INFO, CAPISUBCMD_REQ, params, 6 + params[4] + additionalinfolen);
}

int CAPI_InfoResp(int handle, DWORD PLCI)
{
	return CAPI_PutMessage(handle, CAPICMD_INFO, CAPISUBCMD_RESP, &PLCI, 4);
}

int CAPI_ListenReq(int handle, int controller, DWORD infomask, DWORD CIPmask)
{
	BYTE params[18];

	*(DWORD *)params = controller;
	*(DWORD *)(params + 4) = infomask;
	*(DWORD *)(params + 8) = CIPmask;
	*(DWORD *)(params + 12) = 0;
	params[16] = 0;
	params[17] = 0;
	return CAPI_PutMessage(handle, CAPICMD_LISTEN, CAPISUBCMD_REQ, params, 18);
}

int CAPI_FacilityReq(int handle, DWORD NCCI, int function, DWORD param, char *number)
{
	BYTE params[100], *p;

	*(DWORD *)params = NCCI;
	*(WORD *)(params + 4) = 0x0003; // Supplementary services
	*(WORD *)(params + 7) = (WORD)function;
	p = params + 9;
	switch(function)
	{
	case CAPISS_SUPPORT:
	case CAPISS_HOLD:
	case CAPISS_RETRIEVE:
		*p++ = 0;
		break;
	case CAPISS_LISTEN:
	case CAPISS_ECT:
	case CAPISS_3PTYBEGIN:
	case CAPISS_3PTYEND:
		*p++ = 4;
		*(DWORD *)p = param;
		p += 4;
		break;
	case CAPISS_SUSPEND:
	case CAPISS_RESUME:
		*p++ = 1 + strlen(number);
		*p++ = strlen(number);
		strcpy(p, number);
		p += strlen(number);
		break;
	case CAPISS_CD:
		*p++ = 7 + strlen(number);
		*(WORD *)p = (WORD)param;	// Display of own number allowed
		p += 2;
		*p++ = 3 + strlen(number);
		*p++ = 1;		// Public party number
		*p++ = 0;		// Default numbering plane
		if(param)
			*p++ = 0x80;	// Presentation allowed
		else *p++ = 0xa0;	// Presentation not allowed
		strcpy(p, number);
		p += strlen(number);
		*p++ = 0;
		break;
	default:
		return -1;
	}
	params[6] = (p - (params + 9)) + 2;
	return CAPI_PutMessage(handle, CAPICMD_FACILITY, CAPISUBCMD_REQ, params, 7 + params[6]);
}

int CAPI_FacilityResp(int handle, DWORD NCCI, int function)
{
	BYTE params[100];

	*(DWORD *)params = NCCI;
	*(WORD *)(params + 4) = 0x0003; // Supplementary services
	params[6] = 3;
	*(WORD *)(params + 7) = (WORD)function;
	params[9] = 0;
	return CAPI_PutMessage(handle, CAPICMD_FACILITY, CAPISUBCMD_RESP, params, 10);
}

int CAPI_SelectBProtocolReq(int handle, DWORD PLCI, int bprotocol)
{
	BYTE params[14], *p;

	*(DWORD *)params = PLCI;
	params[4] = 9;						// B protocol struct length
	p = params + 5;
	*(WORD *)p = bprotocol & 0xff;
	*(WORD *)(p + 2) = (bprotocol >> 8) & 0xff;
	*(WORD *)(p + 4) = (bprotocol >> 16) & 0xff;
	p[6] = p[7] = p[8] = 0;
	return CAPI_PutMessage(handle, CAPICMD_SELECT_B_PROTOCOL, CAPISUBCMD_REQ, params, 14);
}

int CAPI_WaitForSignal()
{
	return CAPIWaitForSignal(Glob_ApplId);
}

int CAPI_GetMessage()
{
	BYTE *msg, params[256], *p;
	int Command, SubCommand, paramslen, handle, datahandle;
	DWORD NCCI;
	WORD info;

	if(CAPIGetMessage(Glob_ApplId, &msg))
		return -1;
	paramslen = *(WORD *)msg - 8;
	if(paramslen > 256)
		paramslen = 256;
	Command = msg[4];
	SubCommand = msg[5];
	handle = *(WORD *)(msg + 6);
	memcpy(params, msg + 8, paramslen);
	if(SubCommand == CAPISUBCMD_IND)
	switch(Command)	// Indications
	{
	case CAPICMD_CONNECT:
		NCCI = *(DWORD *)params;
		{
			WORD CIP;
			char callingnumber[MAXNUMLEN], callednumber[MAXNUMLEN], LLC[30], userinfo[MAXUSERINFOLEN];
			int clir, userinfolen;

			CIP = *(WORD *)(params + 4);
			p = params + 6;
			if(*p > 1)
			{
				int ns = p[1] & 0x30;	// Numbering plane identifier
										// 0 - standard, 1 - international, 2 - national
				if(ns == 0x10)
					ns = 2;
				else if(ns == 0x20)
					ns = 1;
				else ns = 0;
				strcpy(callednumber, "00");
				sprintf(callednumber + ns, "%*.*s", *p - 1, *p - 1, p + 2);
			} else *callednumber = 0;
			p = params + 6 + params[6] + 1;
			if(*p > 2)
			{
				int ns = p[1] & 0x30;	// Numbering plane identifier
										// 0 - standard, 1 - international, 2 - national
				if(ns == 0x10)
					ns = 2;
				else if(ns == 0x20)
					ns = 1;
				else ns = 0;
				strcpy(callingnumber, "00");
				sprintf(callingnumber + ns, "%*.*s", *p - 2, *p - 2, p + 3);
			} else *callingnumber = 0;
			if(p[2] & 32)
				clir = 1;
			else clir = 0;
			p += *p + 1;	// Called party subaddress
			if(*p > 2)
				sprintf(callednumber + strlen(callednumber), ".%*.*s", *p - 2, *p - 2, p + 3);
			p += *p + 1;	// Calling party subaddress
			if(*p > 2)
				sprintf(callingnumber + strlen(callingnumber), ".%*.*s", *p - 2, *p - 2, p + 3);
			p += *p + 1;	// BC
			p += *p + 1;	// LLC
			memcpy(LLC, p + 1, *p);
			LLC[*p] = 0;
			p += *p + 1;	// HLC
			p += *p + 1;	// Additional info
			userinfolen = 0;
			if(*p)
			{
				p++;			// B channel info
				p += *p + 1;	// Keypad facility
				p += *p + 1;	// User-user data
				if(*p)
				{
					memcpy(userinfo, p + 1, *p);
					userinfolen = *p;
				}
			}
			CAPI_ConnectInd(handle, NCCI, CIP, callednumber, callingnumber, LLC, clir, userinfo, userinfolen);
			break;
		}
	case CAPICMD_CONNECT_ACTIVE:
		NCCI = *(DWORD *)params;
		{
			char connectednumber[MAXNUMLEN], LLC[30];
			
			p = params + 4;
			if(*p > 1)
			{
				int ns = p[1] & 0x30;	// Numbering plane identifier
										// 0 - standard, 1 - international, 2 - national
				if(ns == 0x10)
					ns = 2;
				else if(ns == 0x20)
					ns = 1;
				else ns = 0;
				strcpy(connectednumber, "00");
				sprintf(connectednumber + ns, "%*.*s", *p - 2, *p - 2, p + 3);
			} else *connectednumber = 0;
			p += *p + 1; // Connected party number
			p += *p + 1; // Connected party subaddress
			memcpy(LLC, p + 1, *p);
			LLC[*p] = 0;
			CAPI_ConnectActiveInd(handle, NCCI, connectednumber, LLC);
		}
		break;
	case CAPICMD_CONNECT_B3_ACTIVE:
		NCCI = *(DWORD *)params;
		CAPI_ConnectB3ActiveInd(handle, NCCI);
		break;
	case CAPICMD_CONNECT_B3:
		NCCI = *(DWORD *)params;
		CAPI_ConnectB3Ind(handle, NCCI);
		break;
	case CAPICMD_DATA_B3:
		NCCI = *(DWORD *)params;
		{
			DWORD data;
			int datalen, datahandle;

			data = *(DWORD *)(params + 4);
			datalen = *(WORD *)(params + 8);
			datahandle = *(WORD *)(params + 10);
			CAPI_DataB3Ind(handle, NCCI, (void *)data, datalen, datahandle);
		}
		break;
	case CAPICMD_DISCONNECT_B3:
		NCCI = *(DWORD *)params;
		info = *(WORD *)(params + 4);
		CAPI_DisconnectB3Ind(handle, NCCI, info);
		break;
	case CAPICMD_DISCONNECT:
		NCCI = *(DWORD *)params;
		info = *(WORD *)(params + 4);
		CAPI_DisconnectInd(handle, NCCI, info);
		break;
	case CAPICMD_FACILITY:
		NCCI = *(DWORD *)params;
		if(*(WORD *)(params + 4) == 3 && params[6] >= 5)
		{
			int function = *(WORD *)(params + 7);
			info = *(WORD *)(params + 10);
			
			CAPI_FacilityInd(handle, NCCI, function, info);
		} 
		break;
	case CAPICMD_INFO:
		NCCI = *(DWORD *)params;
		info = *(WORD *)(params + 4);
		if(!(info & 0x8000))
			CAPI_InfoInd(handle, NCCI, info, params + 7, params[6]);
		else CAPI_InfoInd(handle, NCCI, info, 0, 0);
		break;
	} else switch(Command)	// Confirmations
	{
	case CAPICMD_ALERT:
		NCCI = *(DWORD *)params;
		info = *(WORD *)(params + 4);
		CAPI_AlertConf(handle, NCCI, info);
		break;
	case CAPICMD_CONNECT:
		NCCI = *(DWORD *)params;
		info = *(WORD *)(params + 4);
		CAPI_ConnectConf(handle, NCCI, info);
		break;
	case CAPICMD_CONNECT_B3:
		NCCI = *(DWORD *)params;
		info = *(WORD *)(params + 4);
		CAPI_ConnectB3Conf(handle, NCCI, info);
		break;
	case CAPICMD_DATA_B3:
		NCCI = *(DWORD *)params;
		datahandle = *(WORD *)(params + 4);
		info = *(WORD *)(params + 6);
		CAPI_DataB3Conf(handle, NCCI, info, datahandle);
		break;
	case CAPICMD_DISCONNECT_B3:
		NCCI = *(DWORD *)params;
		info = *(WORD *)(params + 4);
		CAPI_DisconnectB3Conf(handle, NCCI, info);
		break;
	case CAPICMD_DISCONNECT:
		NCCI = *(DWORD *)params;
		info = *(WORD *)(params + 4);
		CAPI_DisconnectConf(handle, NCCI, info);
		break;
	case CAPICMD_FACILITY:
		NCCI = *(DWORD *)params;
		info = *(WORD *)(params + 4);
		if(*(WORD *)(params + 6) == 0x0003 && params[8] >= 5 && params[11] >= 2) // Supplementary services
		{
			WORD function = *(WORD *)(params + 9);
			WORD ssinfo = *(WORD *)(params + 12);
			DWORD supported = 0;
			if(function == 0 && params[11] >= 6)
				supported = *(DWORD *)(params + 14);
			CAPI_FacilityConf(handle, NCCI, info, function, ssinfo, supported);
		}
		break;
	case CAPICMD_LISTEN:
		NCCI = *(DWORD *)params;
		info = *(WORD *)(params + 4);
		CAPI_ListenConf(handle, NCCI, info);
		break;
	case CAPICMD_SELECT_B_PROTOCOL:
		NCCI = *(DWORD *)params;
		info = *(WORD *)(params + 4);
		CAPI_SelectBProtocolConf(handle, NCCI, info);
		break;
	}
	return 0;
}

static const char *ssfunctions[14] = {
	"Support", "Listen", "Hold", "Retrieve", "Suspend", "Resume", "Explicit Call Transfer",
	"Three party begin", "Three party end", "Call forwarding activate", "Call forwarding deactivate",
	"Call forwarding get parameters", "Call forwarding get numbers", "Call deflection"};

const char *CAPI_SSFunctionString(int function)
{
	if(function >= 0 && function <= 13)
		return ssfunctions[function];
	else return "Unknown";
}

static const char *capi20errors[9] = {"Unknown (0x2000)", "Message not supported in current state",
"Illegal controller/PLCI/NCCI", "No physical link connection available", "No network control connection available", "No Listen resources available",
"No fax resources available", "Illegal message parameter coding", "No interconnection resources available"};

static const char *capi30errors[18] = {"Unknown (0x3000)",
"B1 protocol not supported", "B2 protocol not supported", "B3 protocol non supported",
"B1 protocol parameter not supported", "B2 protocol parameter not supported", "B3 protocol parameter not supported",
"B protocol combination not supported", "NCPI not supported", "CIP Value unknown", "Flags not supported (reserved bits)",
"Facility not supported", "Data length not supported by current protocol", "Reset procedure not supported by current protocol",
"TEI assignment failed/supplementary service not supported", "Unsupported interoperability", "Request not allowed in this state",
"Facility specific function not supported"};

static const char *capi36errors[12] = {"Not subscribed to service (0x3600)", "Not subscribed to service (0x3601)", "Not available (0x3602)", "Not available", "Not implemented",
"Not available (0x3605)", "Invalid served user number", "Invalid call state", "Basic service not provided", "Not an incoming call",
"Supplementary services interaction not allowed", "Resource not available"};

static const char *capi37errors[8] = {"Duplicate invocation", "Unrecognized operation", "Mystyped argument", "Resource limitation",
"Initiator releasing", "Unrecognized linked id", "Linked response unexpected", "Unexpected child operation"};

static const char *initerrors[5] = {"CAPI2032.DLL not found", "CAPI not installed", "CAPI GetProfile failed", "No ISDN controllers found", "CAPI Register failed"};

const char *CAPI_ErrorString(int error)
{
	static char s[300];

	if((error & 0xff00) == 0x2000 && (error & 0xff) <= 0x08)
		return capi20errors[error & 0xff];
	else if((error & 0xff00) == 0x3000 && (error & 0xff) <= 0x11)
		return capi30errors[error & 0xff];
	else if((error & 0xff00) == 0x3400)
		return neterrors[error & 0x7f];
	else if(error == 0x3301)
		return "Protocol error layer 1 (check cable/NT)";
	else if(error == 0x3302)
		return "Protocol error layer 2";
	else if(error == 0x3303)
		return "Protocol error layer 3";
	else if(error == 0x3304)
		return "Another application got that call";
	else if(error == 0x3305)
		return "Rejected by Supplementary Services Supervision";
	else if(error == 0x0001)
		return "NCPI not supported by current protocol, NCPI ignored";
	else if(error == 0x0002)
		return "Flags not supported by current protocol, flags ignored";
	else if(error == 0x0003)
		return "Alert already sent by another application";
	else if((error & 0xff00) == 0x3600 && (error & 0xff) <= 0x11)
		return capi36errors[error & 0xff];
	else if((error & 0xff00) == 0x3700 && (error & 0xff) <= 0x11)
		return capi37errors[error & 0xff];
	else if(-5 <= error && error <= -1)
		return initerrors[-error - 1];
	else {
		sprintf(s, "Unknown (0x%04x)", error);
		return s;
	}	
}

/****************************************/
/* Second part, call management         */
/****************************************/

typedef struct {
	char status, controller, earlyb3;
	char ownnumber[MAXNUMLEN], connectednumber[MAXNUMLEN];
	DWORD NCCI, ECT_PLCI;
	unsigned linkedline;
	int cause;
	void *buffers[8];
	char head, tail;

	// For temporary use
	int connectresphandle, resumebprotocol;
	char suspendcallid[9];
} LINESTATUS;

DWORD sssupport[256];

LINESTATUS lines[MAXLINES];
static int controllers, listenconfacknowledged, maxbuffersize, terminated, sssupportacknowledged;

// GetMessage callbacks: callbacks

void CAPI_AlertConf(int handle, DWORD PLCI, WORD info)
{
	if(info && info != 0x0003)	// Another application took the call is only a warning
		CAPI_CallBack(handle, CB_CONNECTERROR, info, 0);
	else if(!info || info == 0x0003)
		CAPI_CallBack(handle, CB_ALERTCONF, 0, 0);
}

void CAPI_ConnectConf(int handle, DWORD PLCI, WORD info)
{
	if(info)
	{
		CAPI_CallBack(handle, CB_CONNECTERROR, info, 0);
		CAPI_CallBack(handle, CB_DISCONNECTED, 0, 0);
		lines[handle].status = 0;
	} else {
		lines[handle].status = CSC_PLCIVALID;
		lines[handle].NCCI = PLCI;
	}
}

void CAPI_ConnectB3Conf(int handle, DWORD NCCI, WORD info)
{
	if(info)
	{
		CAPI_DisconnectReq(handle, lines[handle].NCCI & 0xffff);
		CAPI_CallBack(handle, CB_CONNECTERROR, info, 0);
	} else {
		lines[handle].NCCI = NCCI;
		if(lines[handle].status == CSC_ALERTING)
			lines[handle].status = CSC_ALERTINGNCCIVALID;
		else if(lines[handle].status == CSC_PLCIVALID)
			lines[handle].status = CSC_NCCIVALID;
		else if(lines[handle].status == CSL_PLCIVALID)
			lines[handle].status = CSL_NCCIVALID;
		else if(lines[handle].status == CSC_HELD)	
			lines[handle].status = CSC_NCCIVALIDFROMHELD;
		else if(lines[handle].status == CSL_HELD)
			lines[handle].status = CSL_NCCIVALIDFROMHELD;
		// Should not occur
		else lines[handle].status = CSC_NCCIVALID;
	}
}

void CAPI_DataB3Conf(int handle, DWORD NCCI, WORD info, int datahandle)
{
	if(!info)
		lines[handle].head = (lines[handle].head + 1) % 8;
}

void CAPI_DisconnectB3Conf(int handle, DWORD NCCI, WORD info)
{
}

void CAPI_DisconnectConf(int handle, DWORD PLCI, WORD info)
{
}

void CAPI_FacilityConf(int handle, DWORD NCCI, WORD info, WORD function, WORD ssinfo, DWORD supported)
{
	if((info || ssinfo) && lines[handle].NCCI == (NCCI & 0xffff) &&
		(lines[handle].status == CSC_ECT || lines[handle].status == CSL_ECT ||
			lines[handle].status == CSC_SUSPEND || lines[handle].status == CSL_SUSPEND))
	{
		if(lines[handle].status == CSC_ECT || lines[handle].status == CSC_SUSPEND)
			lines[handle].status = CSC_PLCIVALID;
		else lines[handle].status = CSL_PLCIVALID;
		CAPI_ConnectB3Req(handle, lines[handle].NCCI);
	}
	if(!info && function == CAPISS_SUPPORT)
	{
		sssupport[NCCI & 0xff] = supported;
		sssupportacknowledged++;
	} else if(info)
		CAPI_CallBack(handle, CB_SSERROR, function, info);
	else if(ssinfo)
		CAPI_CallBack(handle, CB_SSERROR, function, ssinfo);
}

void CAPI_ListenConf(int handle, DWORD controller, WORD info)
{
	listenconfacknowledged++;
}

void CAPI_SelectBProtocolConf(int handle, DWORD PLCI, WORD info)
{
	if(info)
	{
		CAPI_CallBack(handle, CB_SSERROR, 5, info);
	} else CAPI_ConnectB3Req(handle, PLCI);
}

// GetMessage callbacks: indications

void CAPI_ConnectInd(int handle, DWORD PLCI, WORD CIP, char *callednumber, char *callingnumber, char *LLC, int clir, char *userinfo, int userinfolen)
{
	INCOMINGCALLDATA icd;
	int i;

	icd.callednumber = callednumber;
	icd.callingnumber = callingnumber;
	icd.userinfo = userinfo;
	icd.userinfolen = userinfolen;
	icd.LLC = LLC;
	icd.CIP = CIP;
	icd.clir = (char)clir;
	icd.PLCI = PLCI;
	// Find a free line
	for(i = 0; i < MAXLINES; i++)
		if(!lines[i].status)
		{
			icd.line = i;
			break;
		}
	if(i == MAXLINES)
	{
		CAPI_ConnectResp(handle, PLCI, 3, 0, callednumber, 1);	// User busy (no line avalable)
		return;
	}
	lines[icd.line].status = CSL_ALERTING;
	lines[icd.line].NCCI = PLCI;	// Used by CAPI_CD called inside the CallBack
	if(CAPI_CallBack(icd.line, CB_INCOMINGCALL, (DWORD)&icd, 0))
	{
		if(!icd.reject)
		{
			if(icd.line != i)
				lines[i].status = 0;
			if(!lines[icd.line].status || lines[icd.line].status == CSL_ALERTING)
			{		
				strcpy(lines[icd.line].connectednumber, callingnumber);
				strcpy(lines[icd.line].ownnumber, callednumber);
				lines[icd.line].NCCI = PLCI;
				lines[icd.line].controller = (char)(PLCI & 0xff);
				lines[icd.line].earlyb3 = 0;
				lines[icd.line].cause = 0;
				if(icd.alert == 2)
				{
					lines[icd.line].status = CSL_ALERTING;
					lines[icd.line].connectresphandle = handle;
					CAPI_AlertReqUUS1(icd.line, PLCI, icd.userinfo, icd.userinfolen);
				} else if(icd.alert)
				{
					lines[icd.line].status = CSL_ALERTING;
					lines[icd.line].connectresphandle = handle;
					CAPI_AlertReq(icd.line, PLCI);
				} else {
					lines[icd.line].status = CSL_PLCIVALID;
					CAPI_ConnectResp(handle, PLCI, 0, icd.protocol, callednumber, icd.colr);
				}
			} else {
				lines[i].status = 0;
				CAPI_ConnectResp(handle, PLCI, 3, 0, callednumber, icd.colr);	// User busy (no line avalable)
			}
		} else {
			lines[i].status = 0;
			CAPI_ConnectResp(handle, PLCI, icd.reject, 0, callednumber, icd.colr);
		}
	} else {
		lines[i].status = 0;
		CAPI_ConnectResp(handle, PLCI, 1, 0, callednumber, icd.colr);	// Ignore
	}
}

void CAPI_ConnectActiveInd(int handle, DWORD PLCI, char *connectednumber, char *LLC)
{
	int i;

	CAPI_ConnectActiveResp(handle, PLCI);
	for(i = 0; i < MAXLINES; i++)
		if(lines[i].status == CSC_PLCIVALID && lines[i].NCCI == PLCI)
		{
			if(*connectednumber)
				strcpy(lines[i].connectednumber, connectednumber);
			CAPI_ConnectB3Req(i, PLCI);
			break;
		} else if((lines[i].status == CSC_ALERTINGNCCIVALID || lines[i].status == CSC_CONNECTEDEB3) && (lines[i].NCCI & 0xffff) == PLCI)
		{
			if(*connectednumber)
				strcpy(lines[i].connectednumber, connectednumber);
			lines[i].tail = lines[i].head = 0;
			lines[i].status = CSC_CONNECTED;
			CAPI_CallBack(i, CB_CONNECTED, (DWORD)lines[i].connectednumber, 0);
			break;
		}
}

void CAPI_ConnectB3ActiveInd(int handle, DWORD NCCI)
{
	int i;

	CAPI_ConnectB3ActiveResp(handle, NCCI);
	for(i = 0; i < MAXLINES; i++)
		if((lines[i].status == CSC_NCCIVALID || lines[i].status == CSL_NCCIVALID) && lines[i].NCCI == NCCI)
		{
			lines[i].tail = lines[i].head = 0;
			lines[i].cause = 0;
			lines[i].status = lines[i].status == CSC_NCCIVALID ? CSC_CONNECTED : CSL_CONNECTED;
			CAPI_CallBack(i, CB_CONNECTED, (DWORD)lines[i].connectednumber, 0);
			break;
		} else if((lines[i].status == CSC_NCCIVALIDFROMHELD || lines[i].status == CSL_NCCIVALIDFROMHELD) && lines[i].NCCI == NCCI)
		{
			lines[i].tail = lines[i].head = 0;
			lines[i].cause = 0;
			lines[i].status = lines[i].status == CSC_NCCIVALIDFROMHELD ? CSC_CONNECTED : CSL_CONNECTED;
			CAPI_CallBack(i, CB_RECONNECTED, (DWORD)lines[i].connectednumber, 0);
			break;
		} else if(lines[i].status == CSC_ALERTINGNCCIVALID && lines[i].NCCI == NCCI)
		{
			lines[i].status = CSC_CONNECTEDEB3;
			CAPI_CallBack(i, CB_CONNECTEDEB3, (DWORD)lines[i].connectednumber, 0);
			break;
		}
}

void CAPI_ConnectB3Ind(int handle, DWORD NCCI)
{
	int i;

	CAPI_ConnectB3Resp(handle, NCCI);
	for(i = 0; i < MAXLINES; i++)
		if(lines[i].status == CSL_PLCIVALID && lines[i].NCCI == (NCCI & 0xffff))
		{
			lines[i].NCCI = NCCI;
			lines[i].status = CSL_NCCIVALID;
			break;
		}
}

void CAPI_DataB3Ind(int handle, DWORD NCCI, void *data, int datalen, int datahandle)
{
	int i;

	CAPI_DataB3Resp(handle, NCCI, datahandle);
	for(i = 0; i < MAXLINES; i++)
		if((lines[i].status >= CSC_ALERTINGNCCIVALID && lines[i].status <= CSC_3PTY ||
			lines[i].status >= CSL_NCCIVALID && lines[i].status <= CSL_3PTY) && lines[i].NCCI == NCCI)
		{
			CAPI_CallBack(i, CB_DATA, (DWORD)data, datalen);
			break;
		}
}

void CAPI_DisconnectB3Ind(int handle, DWORD NCCI, WORD reason)
{
	int i;

	CAPI_DisconnectB3Resp(handle, NCCI);
	for(i = 0; i < MAXLINES; i++)
		if(lines[i].status == CSD_B3 && lines[i].NCCI == NCCI)
		{
			lines[i].NCCI &= 0xffff;
			CAPI_DisconnectReq(i, lines[i].NCCI);
		} else if((lines[i].status == CSC_ECT || lines[i].status == CSL_ECT) && lines[i].NCCI == NCCI)
		{
			lines[i].NCCI &= 0xffff;
			CAPI_FacilityReq(i, lines[i].NCCI, CAPISS_ECT, lines[i].ECT_PLCI, 0);
		} else if((lines[i].status == CSC_SUSPEND || lines[i].status == CSL_SUSPEND) && lines[i].NCCI == NCCI)
		{
			lines[i].NCCI &= 0xffff;
			CAPI_FacilityReq(i, lines[i].NCCI, CAPISS_SUSPEND, 0, lines[i].suspendcallid);
		} else if(lines[i].status == CSC_CONNECTEDEB3 && lines[i].NCCI == NCCI)
		{
			lines[i].status = CSC_PLCIVALID;
			lines[i].NCCI &= 0xffff;
		}
}

void CAPI_DisconnectInd(int handle, DWORD PLCI, WORD reason)
{
	int i;

	if(reason == 0x3400)
		reason = 0;
	CAPI_DisconnectResp(handle, PLCI);
	for(i = 0; i < MAXLINES; i++)
	{
		if((lines[i].status == CSC_PHELD || lines[i].status == CSL_PHELD) && (lines[i].NCCI & 0xffff) == PLCI)
		{
			if(lines[lines[i].linkedline].status == CSC_3PTY)
			{
				lines[lines[i].linkedline].status = CSC_CONNECTED;
				CAPI_CallBack(lines[i].linkedline, CB_3PTYEND, i, 0);
			} else if(lines[lines[i].linkedline].status == CSL_3PTY)
			{
				lines[lines[i].linkedline].status = CSL_CONNECTED;
				CAPI_CallBack(lines[i].linkedline, CB_3PTYEND, i, 0);
			}
		} else if((lines[i].status == CSC_3PTY || lines[i].status == CSL_3PTY) && (lines[i].NCCI & 0xffff) == PLCI)
		{
			if(lines[lines[i].linkedline].status == CSC_PHELD)
			{
				lines[lines[i].linkedline].status = CSC_HELD;
				CAPI_CallBack(i, CB_3PTYEND, lines[i].linkedline, 0);
			} else if(lines[lines[i].linkedline].status == CSL_PHELD)
			{
				lines[lines[i].linkedline].status = CSL_HELD;
				CAPI_CallBack(i, CB_3PTYEND, lines[i].linkedline, 0);
			}
		}
		if(lines[i].status == CSL_ALERTING && (lines[i].NCCI & 0xffff) == PLCI)
			CAPI_CallBack(i, CB_MISSED, reason, 0);
		if((lines[i].status && lines[i].status != CSC_CONNECTINITIATED && lines[i].status != CSC_RESUME && lines[i].status != CS_RESERVED)
			&& (lines[i].NCCI & 0xffff) == PLCI)
		{
			lines[i].status = 0;
			CAPI_CallBack(i, CB_DISCONNECTED, reason, 0);
		}
	}
}

void CAPI_FacilityInd(int handle, DWORD NCCI, int function, WORD info)
{
	int i;

	CAPI_FacilityResp(handle, NCCI, function);
	for(i = 0; i < MAXLINES; i++)
		switch(function)
		{
		case CAPISS_HOLD:
			if(lines[i].status == CSC_CONNECTED && (NCCI & 0xffff) == (lines[i].NCCI & 0xffff))
			{
				if(!info)
				{
					lines[i].status = CSC_HELD;
					lines[i].NCCI &= 0xffff;
					CAPI_CallBack(i, CB_HELD, 0, 0);
				} else CAPI_CallBack(i, CB_SSERROR, function, info);
			} else if(lines[i].status == CSL_CONNECTED && (NCCI & 0xffff) == (lines[i].NCCI & 0xffff))
			{
				if(!info)
				{
					lines[i].status = CSL_HELD;
					lines[i].NCCI &= 0xffff;
					CAPI_CallBack(i, CB_HELD, 0, 0);
				} else CAPI_CallBack(i, CB_SSERROR, function, info);
			}
			break;
		case CAPISS_RETRIEVE:
			if((lines[i].status == CSC_HELD || lines[i].status == CSL_HELD) && (NCCI & 0xffff) == (lines[i].NCCI & 0xffff))
				if(!info)
					CAPI_ConnectB3Req(i, NCCI);
				else CAPI_CallBack(i, CB_SSERROR, function, info);
			break;
		case CAPISS_3PTYBEGIN:
			if((lines[i].status == CSC_CONNECTED || lines[i].status == CSL_CONNECTED) && NCCI == (lines[i].NCCI & 0xffff))
			{
				if(!info)
				{
					if(lines[i].status == CSC_CONNECTED)
						lines[i].status = CSC_3PTY;
					else lines[i].status = CSL_3PTY;
					lines[lines[i].linkedline].status = lines[lines[i].linkedline].status == CSC_HELD ? CSC_PHELD : CSL_PHELD;
					CAPI_CallBack(i, CB_3PTYBEGIN, lines[i].linkedline, 0);
				} else CAPI_CallBack(i, CB_SSERROR, function, info);
			}
			break;
		case CAPISS_3PTYEND:
			if((lines[i].status == CSC_3PTY || lines[i].status == CSL_3PTY) && NCCI == (lines[i].NCCI & 0xffff))
			{
				if(!info)
				{
					if(lines[i].status == CSC_3PTY)
						lines[i].status = CSC_CONNECTED;
					else lines[i].status = CSL_CONNECTED;
					lines[lines[i].linkedline].status = lines[lines[i].linkedline].status == CSC_PHELD ? CSC_HELD : CSL_HELD;
					CAPI_CallBack(i, CB_3PTYEND, lines[i].linkedline, 0);
				} else CAPI_CallBack(i, CB_SSERROR, function, info);
			}
			break;
		case CAPISS_ECT:
			if((lines[i].status == CSC_ECT || lines[i].status == CSL_ECT) && NCCI == (lines[i].NCCI & 0xffff))
			{
				if(!info)
				{
					CAPI_CallBack(i, CB_ECT, lines[i].linkedline, 0);
					CAPI_Disconnect(lines[i].linkedline);
					CAPI_Disconnect(i);
				} else {
					CAPI_CallBack(i, CB_SSERROR, function, info);
					if(lines[i].status == CSC_ECT)
						lines[i].status = CSC_PLCIVALID;
					else lines[i].status = CSL_PLCIVALID;
					CAPI_ConnectB3Req(i, lines[i].NCCI);
				}
			}
			break;
		case CAPISS_SUSPEND:
				if((lines[i].status == CSC_SUSPEND || lines[i].status == CSL_SUSPEND)
				&& NCCI == lines[i].NCCI)
			{
				if(!info)
					CAPI_CallBack(i, CB_SUSPENDED, 0, 0);
				else {
					if(lines[i].status == CSC_SUSPEND)
						lines[i].status = CSC_PLCIVALID;
					else lines[i].status = CSL_PLCIVALID;
					CAPI_ConnectB3Req(i, lines[i].NCCI);
					CAPI_CallBack(i, CB_SSERROR, function, info);
				}
			}
			break;
		case CAPISS_RESUME:
			if(lines[i].status == CSC_RESUME)
			{
				if(!info)
				{
					lines[i].NCCI = NCCI;
					lines[i].controller = (char)(NCCI & 0xff);
					lines[i].status = CSC_PLCIVALID;
					CAPI_SelectBProtocolReq(i, NCCI, lines[i].resumebprotocol);
				} else {
					lines[i].status = 0;
					CAPI_CallBack(i, CB_SSERROR, function, info);
					CAPI_CallBack(i, CB_DISCONNECTED, info, 0);
				}
			}
			break;
		case CAPISS_CD:
			if(info && !i)
				CAPI_CallBack(i, CB_SSERROR, function, info);
			break;
		}
}

void CAPI_InfoInd(int handle, DWORD PLCI, WORD infonumber, BYTE *infoelement, int infoelementlen)
{
	int i;

	CAPI_InfoResp(handle, PLCI);
	for(i = 0; i < MAXLINES; i++)
	{
		if(lines[i].status == CSC_PLCIVALID && lines[i].NCCI == PLCI)
		{
			if(infonumber >= 0x8000 && infonumber <= 0x800f ||
				infonumber == 0x1e || infonumber == 0x27)	// Call proceeding
			{
				if(lines[i].earlyb3)
				{
					CAPI_ConnectB3Req(i, PLCI);
					lines[i].status = CSC_ALERTING;
				}
			}
		}
		if(lines[i].status >= CSC_RESUME && lines[i].status != CS_RESERVED && (lines[i].NCCI & 0xffff) == PLCI)
		{
			if(infonumber == 0x28) // Display
				CAPI_CallBack(i, CB_DISPLAYINFO, (DWORD)infoelement, infoelementlen);
			else if(infonumber == 0x7e) // User-user
				CAPI_CallBack(i, CB_USERUSERINFO, (DWORD)infoelement, infoelementlen);
		}
		if((lines[i].status >= CSC_ALERTING && lines[i].status <= CSC_PHELD ||
			lines[i].status >= CSL_PLCIVALID && lines[i].status <= CSL_PHELD) &&
			(lines[i].NCCI & 0xffff) == PLCI)
		{
			CAPI_CallBack(i, CB_CALLPROGRESS, infonumber, 0);
			if(infonumber >= 0x8040 && infonumber <= 0x804f)
				CAPI_CallBack(i, CB_USERDISCONNECTED, lines[i].cause ? lines[i].cause | 0x3400 : 0, 0);
			if(infonumber == 0x0008 && infoelementlen == 2 && infoelement[0] & 0x80)
				lines[i].cause = infoelement[1];
			if((infonumber & 0xc000) == 0x4000 && infoelementlen >= 4)
				CAPI_CallBack(i, CB_CHARGINGINFO, *(DWORD *)infoelement, 0);
		}
	}
}

// Main routines

static void CAPIThread(void *dummy)
{
	for(;;)
	{
		if(CAPI_WaitForSignal())
			break;
		CAPI_GetMessage();
		if(listenconfacknowledged == controllers && terminated)
			break;
	}
}

int CAPI_Init(int maxbufsize, int earlyb3)
{
	int rc, i, j;
	char *buf;
	unsigned tmout;

	terminated = 0;
	maxbuffersize = maxbufsize;
	if((rc = CAPI_BaseInit(MAXLINES, maxbufsize)) < 0)
		return rc;

	memset(lines, 0, sizeof(lines));
	buf = (char *)malloc(maxbufsize * 8 * MAXLINES);
	for(i = 0; i < MAXLINES; i++)
		for(j = 0; j < 8; j++)
			lines[i].buffers[j] = buf + maxbufsize * (j + 8 * i);
	controllers = rc;
	for(i = 0; i < controllers; i++)
		CAPI_ListenReq(0, i + 1, earlyb3 ? 0x24d : 0x04d, 0x70116); //0x10016 //0xfffffffe
	// Info listen:
	// Early B3, charge, user-user, display, cause
	rc = _beginthread(CAPIThread, 0, 0);
	SetThreadPriority((HANDLE)rc, THREAD_PRIORITY_TIME_CRITICAL);
	sssupportacknowledged = 0;
	for(i = 0; i < controllers; i++)
		CAPI_FacilityReq(0, i + 1, CAPISS_SUPPORT, 0, 0);
	tmout = GetTickCount() + 1000;
	while(sssupportacknowledged < controllers && GetTickCount() < tmout)
		Sleep(1);
//	CAPI_FacilityReq(0, 1, CAPISS_LISTEN, 0x1f, 0);
	return 0;
}

int CAPI_Exit()
{
	int i;
	unsigned timeout = GetTickCount() + 3000;

	if(controllers == 0)
		return -1;
	for(i = 0; i < MAXLINES; i++)
		CAPI_Disconnect(i);
	listenconfacknowledged = 0;
	terminated = 1;
	for(i = 0; i < controllers; i++)
		CAPI_ListenReq(0, i + 1, 0, 0);
	for(;;)
	{
		for(i = 0; i < MAXLINES; i++)
			if(lines[i].status)
				break;
		if(i == MAXLINES && listenconfacknowledged == controllers || GetTickCount() > timeout)
			break;
		Sleep(10);
	}
	CAPI_BaseExit();
	free(lines[0].buffers[0]);
	controllers = 0;
	return 0;
}

DWORD CAPI_SSSupport(int controller)
{
	if(controller >= 0 && controller <= controllers)
		return sssupport[controller];
	else return 0;
}

int CAPI_Dial(unsigned line, int controller, int CIP, int bprotocol, char *number, char *ownnumber, char *LLC, unsigned flags, char *userinfo)
{
	if(line >= MAXLINES || (lines[line].status && lines[line].status != CS_RESERVED))
		return -1;
	lines[line].earlyb3 = (flags & CAPIFLAG_EARLYB3) ? 1 : 0;
	lines[line].status = CSC_CONNECTINITIATED;
	lines[line].controller = controller;
	lines[line].cause = 0;
	if(ownnumber)
		strcpy(lines[line].ownnumber, ownnumber);
	else lines[line].ownnumber[0] = 0;
	strcpy(lines[line].connectednumber, number);
	return CAPI_ConnectReq(line, lines[line].controller, CIP, bprotocol, number, ownnumber, LLC, flags & CAPIFLAG_SCREEN, userinfo);
}

int CAPI_Disconnect(unsigned line)
{
	if(line >= MAXLINES)
		return -1;
	if(!lines[line].status)
		return -1;
	if(lines[line].status >= CSC_ALERTINGNCCIVALID && lines[line].status <= CSC_CONNECTED ||
		lines[line].status >= CSL_NCCIVALID && lines[line].status <= CSL_CONNECTED)
	{
		lines[line].status = CSD_B3;
		CAPI_DisconnectB3Req(line, lines[line].NCCI);
	} else if(lines[line].status != CSC_CONNECTINITIATED && lines[line].status != CSC_RESUME && lines[line].status != CS_RESERVED)
		CAPI_DisconnectReq(line, lines[line].NCCI & 0xffff);
	else {
		lines[line].status = CS_IDLE;
		CAPI_CallBack(line, CB_DISCONNECTED, 0, 0);
	}
	return 0;
}

int CAPI_Accept(unsigned line, int reject, int protocol, char *ownnumber, int screen)
{
	if(line >= MAXLINES)
		return -1;
	if(lines[line].status != CSL_ALERTING)
		return -1;
	if(!reject)
	{
		lines[line].status = CSL_PLCIVALID;
		CAPI_ConnectResp(lines[line].connectresphandle, lines[line].NCCI, 0, protocol, ownnumber, screen);
	} else {
		lines[line].status = CSD_B3;
		CAPI_ConnectResp(lines[line].connectresphandle, lines[line].NCCI, reject, 0, ownnumber, screen);
	}
	return 0;
}

int CAPI_SendData(unsigned line, void *data, int datalen)
{
	if(line >= MAXLINES)
		return -1;
	if(datalen > maxbuffersize)
		return -1;
	if((lines[line].tail + 1) % 8 == lines[line].head)
		return -2;
	if(lines[line].status == CSC_CONNECTED || lines[line].status == CSL_CONNECTED ||
		lines[line].status == CSC_3PTY || lines[line].status == CSL_3PTY)
	{
		memcpy(lines[line].buffers[lines[line].tail], data, datalen);
//		((char *)lines[line].buffers[lines[line].tail])[0] = lines[line].tail;
//		((char *)lines[line].buffers[lines[line].tail])[1] = lines[line].head;
		CAPI_DataB3Req(line, lines[line].NCCI, lines[line].buffers[lines[line].tail], datalen, 0);
		lines[line].tail = (lines[line].tail + 1) % 8;
		return 0;
	} else return -1;
}

int CAPI_Hold(unsigned line)
{
	if(line >= MAXLINES)
		return -1;
	if(lines[line].status != CSC_CONNECTED && lines[line].status != CSL_CONNECTED)
		return -1;
	CAPI_FacilityReq(line, lines[line].NCCI, CAPISS_HOLD, 0, 0);
	return 0;
}

int CAPI_Retrieve(unsigned line)
{
	if(line >= MAXLINES)
		return -1;

	if(lines[line].status == CSC_SUSPEND || lines[line].status == CSL_SUSPEND ||
		lines[line].status == CSC_ECT || lines[line].status == CSL_ECT)
	{
		CAPI_ConnectB3Req(line, lines[line].NCCI & 0xffff);
		return 0;
	}
	if(lines[line].status != CSC_HELD && lines[line].status != CSL_HELD)
		return -1;
	CAPI_FacilityReq(line, lines[line].NCCI, CAPISS_RETRIEVE, 0, 0);
	return 0;
}

int CAPI_3PtyBegin(unsigned activeline, unsigned heldline)
{
	if(activeline >= MAXLINES || heldline >= MAXLINES ||
		!(lines[activeline].status == CSC_CONNECTED || lines[activeline].status == CSL_CONNECTED) ||
		!(lines[heldline].status == CSC_HELD || lines[heldline].status == CSL_HELD))
		return -1;
	lines[activeline].linkedline = heldline;
	lines[heldline].linkedline = activeline;
	CAPI_FacilityReq(activeline, lines[activeline].NCCI, CAPISS_3PTYBEGIN, lines[heldline].NCCI, 0);
	return 0;
}

int CAPI_3PtyEnd(unsigned line)
{
	if(line >= MAXLINES)
		return -1;
	if(lines[line].status == CSC_3PTY || lines[line].status == CSL_3PTY)
		return CAPI_FacilityReq(line, lines[line].NCCI, CAPISS_3PTYEND, lines[lines[line].linkedline].NCCI, 0);
	else if(lines[line].status == CSC_PHELD || lines[line].status == CSL_PHELD)
		return CAPI_FacilityReq(line, lines[lines[line].linkedline].NCCI, CAPISS_3PTYEND, lines[line].NCCI, 0);
	else return -1;
}

int CAPI_ECT(unsigned line1, unsigned line2)
{
	if(line1 >= MAXLINES || line2 >= MAXLINES)
		return -1;
	if(lines[line1].status != CSC_CONNECTED && lines[line1].status != CSL_CONNECTED)
		return -1;
	if(lines[line2].status != CSC_HELD && lines[line2].status != CSL_HELD)
		return -1;
	if(lines[line1].status == CSC_CONNECTED)
		lines[line1].status = CSC_ECT;
	else lines[line1].status = CSL_ECT;
	lines[line1].ECT_PLCI = lines[line2].NCCI & 0xffff;
	lines[line1].linkedline = line2;
	CAPI_DisconnectB3Req(line1, lines[line1].NCCI);
	return 0;
}

int CAPI_Suspend(unsigned line, char *callid)
{
	if(line >= MAXLINES)
		return -1;
	if(lines[line].status != CSC_CONNECTED && lines[line].status != CSL_CONNECTED)
		return -1;
	if(lines[line].status == CSC_CONNECTED)
		lines[line].status = CSC_SUSPEND;
	else lines[line].status = CSL_SUSPEND;
	strncpy(lines[line].suspendcallid, callid, 8);
	lines[line].suspendcallid[8] = 0;
	CAPI_DisconnectB3Req(line, lines[line].NCCI);
	return 0;
}

int CAPI_Resume(unsigned line, int controller, int bprotocol, char *callid)
{
	char callid2[9];

	if(line >= MAXLINES || lines[line].status)
		return -1;
	lines[line].status = CSC_RESUME;
	lines[line].resumebprotocol = bprotocol;
	lines[line].earlyb3 = 0;
	lines[line].cause = 0;
	lines[line].controller = controller;
	*lines[line].ownnumber = 0;
	*lines[line].connectednumber = 0;
	strcpy(callid2, callid);
	callid2[8] = 0;
	CAPI_FacilityReq(line, controller, CAPISS_RESUME, 0, callid2);
	return 0;
}

int CAPI_CD(unsigned line, char *number, int screen)
{
	if(line >= MAXLINES || lines[line].status != CSL_ALERTING)
		return -1;
	CAPI_FacilityReq(line, lines[line].NCCI, CAPISS_CD, screen, number);
	return 0;
}

int CAPI_WaitingFacilityInd(unsigned line)
{
	if(line >= MAXLINES)
		return 0;
	if(lines[line].status == CSC_SUSPEND || lines[line].status == CSL_SUSPEND ||
		lines[line].status == CSC_ECT || lines[line].status == CSL_ECT)
		return 1;
	return 0;
}

int CAPI_ReserveLine(unsigned line, int reserve)
{
	if((int)line == -1 && reserve == 1)
	{
		for(line = 0; line < MAXLINES; line++)
			if(lines[line].status == CS_IDLE)
			{
				lines[line].status = CS_RESERVED;
				return line;
			}
		return -1;
	}
	if(line >= MAXLINES)
		return -1;
	if(reserve)
	{
		if(lines[line].status == CS_IDLE)
		{
			lines[line].status = CS_RESERVED;
			return 0;
		} else return -1;
	} else {
		if(lines[line].status == CS_RESERVED)
		{
			lines[line].status = CS_IDLE;
			return 0;
		} else return -1;
	}
}

int CAPI_AvailableLines()
{
	int i, usedlines = 0;

	for(i = 0; i < MAXLINES; i++)
		if(lines[i].status >= CSC_ALERTINGNCCIVALID && lines[i].status <= CSC_3PTY ||
			lines[i].status >= CSL_NCCIVALID && lines[i].status <= CSL_3PTY)
			usedlines++;
	if(usedlines > available_b_channels)
		return 0;
	else return available_b_channels - usedlines;
}

int CAPI_FindFreeLine()
{
	int i, usedlines = 0;

	for(i = 0; i < MAXLINES; i++)
		if(!lines[i].status)
			return i;
	return -1;
}

int CAPI_KeyPad(unsigned line, char *keypad)
{
	BYTE buf[50];
	int len;

	if(line >= MAXLINES)
		return -1;
	if(lines[line].status < CSC_RESUME || lines[line].status == CS_RESERVED)
		return -1;
	len = strlen(keypad);
	if(len > 30)
		return -1;
	buf[0] = 0;
	buf[1] = len;
	memcpy(buf + 2, keypad, len);
	buf[2 + len] = 0;
	buf[3 + len] = 0;
	buf[4 + len] = 0;
	return CAPI_InfoReq(line, lines[line].NCCI, 0, buf, len + 5);
}

int CAPI_OverlapNumber(unsigned line, char *number)
{
	if(line >= MAXLINES)
		return -1;
	if(lines[line].status != CSC_CONNECTEDEB3)
		return -1;
	return CAPI_InfoReq(line, lines[line].NCCI, number, 0, 0);
}

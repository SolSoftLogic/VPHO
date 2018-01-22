#ifndef _IELEMENTS_INCLUDED_
#define _IELEMENTS_INCLUDED_

#ifndef DEFAULT_MTU
#define DEFAULT_MTU 548
#endif

class OUTMSG {
public:
	OUTMSG(unsigned mtu, BYTE type);
	void Reset(BYTE type);
	int AddIE_RecordEnd();
	int AddIE_String(BYTE type, const char *s);
	int AddIE_DWord(BYTE type, DWORD w);
	int AddIE_Word(BYTE type, WORD w);
	int AddIE_Byte(BYTE type, BYTE b);
	int AddIE_2Bytes(BYTE type, BYTE b1, BYTE b2);
	int AddIE_3Bytes(BYTE type, BYTE b1, BYTE b2, BYTE b3);
	int AddIE_BC(BYTE bc, BYTE codec, BYTE flags, DWORD codecs);
	int AddIE_Video(DWORD fourcc, WORD xres, WORD yres);
	int AddIE_InAddr(BYTE type, DWORD addr, WORD port);
	int AddIE_MultiInAddr(BYTE type, struct sockaddr_in *addr, int count);
	int AddIE_MissedCall(DWORD tm, BYTE count, BYTE bc);
	int AddIE_Text(BYTE encoding, char *data, int datalen);
	int AddIE_EncodedNumbers(const DWORD *numbers, int num);
	int AddIE_Binary(BYTE type, const BYTE *data, unsigned datalen);

	BYTE msg[1500];
	BYTE *ptr;
	unsigned avail, mtu;
};

struct IE_ELEMENTS {
	char *packetbuf;
	unsigned packetlen;
	DWORD seqnumber;
	WORD connectionid, mtu;
	BYTE bc;
	BYTE codec;
	BYTE bc_flags;
	DWORD bc_codecs;
	BYTE rtfencoding;
	DWORD fourcc;
	WORD xres, yres;
	BYTE answer;
	BYTE reason;
	BYTE operation;
	BYTE confsetupstep;
	BYTE calltransfer;
	DWORD filelength, filecrc, startpos;
	struct sockaddr_in hostaddr, privateaddr, publicaddr, hostaddr_array[MAXVPARTIES], reqaddr, reqaddr2;
	BYTE callrequest;
	DWORD version;
	DWORD token;
	DWORD smstype, smsid;
	DWORD nfiles, nbytes;
	char *srcnum, *srcname, *destname, *srcsubaddr, *destnum, *destsubaddr, *message,
		*filename, *deflnum, *serveraccesscode, *fileslist, *sessionid, *sessionsecret, *serverslist,
		*firstname, *lastname, *email, *phonenumber, *country;
	char *transparentdata;
	int transparentdatalen;
	char *rtfdata;
	int rtfdatalen;
	BYTE callstatus;
	WORD codepage;
	DWORD missedcalltime;
	BYTE missedcallscount, missedcallbc;
	BYTE nencodednumbers;
	DWORD *encodednumbersptr;
	BYTE *authdata;
	BYTE offline;
	DWORD timestamp, expiration;
	DWORD serverid;
	DWORD userstatus;
	DWORD serial;
	char nullchar;
	bool seqnumberpresent, tokenpresent, callidpresent, smsquotapresent;
	BYTE keypad;
	BYTE callid[16];
	BYTE protversion;
	int quota, smsquota;
};

int GetIE_Elements(BYTE *msg, unsigned len, IE_ELEMENTS *ie);
IE_ELEMENTS *DupIE(IE_ELEMENTS *ie);
void FreeIE(IE_ELEMENTS *ie);

#endif

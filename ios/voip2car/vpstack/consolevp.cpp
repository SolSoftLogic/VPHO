#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "vpstack.h"

void ArgcArgvParam(char *p, int *argc, char ***argv);
void FreeArgcArgv();
void notifythread(void *dummy);

VPMIXINGAUDIOFACTORY vpmixingaudiofactory;
IVPSTACK *vp;
AOL aol;
unsigned notifythreadid;
char greeting[MAXPATH];

#ifdef INCLUDEVIDEO
HWINDOW hParent;
int childid;
RECTANGLE videorect;
VPVIDEODATA *localvideo;
IAVIPLAYER *aviplayer;

class VPOURVIDEOFACTORY : public VPVIDEOFACTORY
{
public:
	VPVIDEODATA *New(IVPSTACK *vps, VPCALL vpcall, unsigned fourccrx, unsigned xresrx, unsigned yresrx, unsigned fourcc, unsigned xres, unsigned yres, unsigned framerate, unsigned quality);
} vpvideofactory;

VPVIDEODATA *VPOURVIDEOFACTORY::New(IVPSTACK *vps, VPCALL vpcall, unsigned fourccrx, unsigned xresrx, unsigned yresrx, unsigned fourcc, unsigned xres, unsigned yres, unsigned framerate, unsigned quality)
{
	VPVIDEODATA *vd = CreateVPVIDEO(vps, vpcall, fourccrx, xresrx, yresrx, fourcc, xres, yres, framerate, quality);
	vd->SetVideoWindowData(hParent, childid, &videorect, false);
	vd->Start();
	return vd;
}
#endif

class VPOURAUDIOFACTORY : public VPAUDIODATAFACTORY
{
public:
	VPAUDIODATA *New(IVPSTACK *vps, VPCALL vpcall, int codec);
} vpaudiofactory;

VPAUDIODATA *VPOURAUDIOFACTORY::New(IVPSTACK *vps, VPCALL vpcall, int codec)
{
	VPAUDIODATA *va = CreateVPMIXINGAUDIO(vps, vpcall, codec);
	if(*greeting)
	{
		char path[MAXPATH];
		SYSTEMTIME t;

		GetLocalTime(&t);
		sprintf(path, "%04d%02d%02d-%02d%02d%02d.wav", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
		va->Save(path);
		vps->SendAudioMessage(vpcall, greeting, 0);
		*greeting = 0;
	}
	return va;
}

int Cmds(int argc, char **argv);

int main(int argc, char **argv)
{
	char s[300], *p;

	//generatelog = LOG_FULLMEDIAPACKETS;
#ifdef _DEBUG
	generatelog = LOG_PACKETS;
#endif
	VPInit();
	VPMIXINGAUDIO_Init();
#ifdef INCLUDEVIDEO
	VPVIDEO_Init();
#endif
#ifdef _WIN32
	_beginthread(notifythread, 0, 0);
#endif
	if(Cmds(argc - 1, argv + 1))
		return -1;
	printf("> ");
	while(fgets(s, sizeof(s), stdin))
	{
		p = strchr(s, '\n');
		if(p)
			*p = 0;
		ArgcArgvParam(s, &argc, &argv);
		if(Cmds(argc, argv))
		{
			FreeArgcArgv();
			break;
		}
		FreeArgcArgv();
		printf("> ");
	}
	if(vp)
		delete vp;
	return 0;
}

char *argv1[30];
int argc1;

void ArgcArgvParam(char *p, int *argc, char ***argv)
{
	char *r;

	if(!argc1)
	{
		while(*p && argc1 < 30)
		{
			while(*p == ' ')
				p++;
			if(*p == '\"')
			{
				r = strchr(p + 1, '\"');
				if(r)
				{
					argv1[argc1] = (char *)malloc(r - p);
					memcpy(argv1[argc1], p + 1, r - p - 1);
					argv1[argc1][r - p - 1] = 0;
					p = r + 1;
				} else {
					argv1[argc1] = strdup(p + 1);
					p = p + strlen(p);
				}
			} else {
				r = strchr(p, ' ');
				if(r)
				{
					argv1[argc1] = (char *)malloc(r - p + 1);
					memcpy(argv1[argc1], p, r - p);
					argv1[argc1][r - p] = 0;
					p = r + 1;
				} else {
					if(*p)
					{
						argv1[argc1] = strdup(p);
						p = p + strlen(p);
					} else argc1--;
				}
			}
			argc1++;
		}
	}
	*argc = argc1;
	*argv = argv1;
}

void FreeArgcArgv()
{
	int i;

	for(i = 0; i < argc1; i++)
		free(argv1[i]);
	argc1 = 0;
}

const char *Reason(int reason)
{
	static const char *reasons[36] = {
	"NORMAL", "BEARER_NOT_SUPPORTED",  "CODEC_NOT_SUPPORTED",  "CALLPROCESSING",  "MISSINGCLID", 
	"CALLDEFLECTION", "BUSY", "FILESAME", "TIMEOUT", "DISCONNECTED", "AUTHERROR", "NOTFOUND", 
	"SYNTAXERROR", "INVALIDNUMBER", "STORAGEEXCEEDED", "ALREADYCONNECTED", "PROTOCOLNOTSUPPORTED", 
	"SERVERREDIRECTION", "ADMINERROR", "ACCOUNTBLOCKED", "CALLTRANSFERCOMPLETE", "NOQUOTA", 
	"USEROFFLINE", "SERVERNOTRESPONDING", "SERVERNOTRUNNING", "VPHONENOTRESPONDING",
	"VPHONENOTRUNNING", "CALLPROCEEDING", "CONNECTIONLOST", "WRONGVERSION", "EARLYAUDIO", 
	"ANOTHERPORT", "ANOTHERADDR", "CANNOTNOTIFY", "NOTKNOWN", "LOOPCALL"};

	if(reason >= 0 && reason <= 35)
		return reasons[reason];
	if(reason == REASON_LOGGEDON)
		return "Logged on";
	if(reason == REASON_VERSIONOLD)
		return "Version old";
	if(reason == REASON_SERVERRESET)
		return "No server running on the specified host";
	return "Unknown";

}

const char *VPError(int err)
{
	static const char *errors[14] = {"Invalid call", "No more calls available", "TCP/IP error",
	"Invalid address", "Calling yourself", "Invalid status", "Not logged on", "Incompatible calls",
	"File not found", "Invalid file format", "Unsupported call", "Unsupported bearer", "Invalid parameter",
	"Timeout"};
	
	if(err >= -14 && err <= -1)
		return errors[-err - 1];
	return "Unknown";
}

int syncnotifyprint(void *uparam, unsigned msg, unsigned param1, unsigned param2)
{
	int rc = 0;
	printf("\r");
	switch(msg)
	{
	case VPMSG_NOTIFYLOGON:	
		printf("Num='%s' logged on\n", (char *)param1);
		break;
	case VPMSG_NEWVOICEMSG:
		printf("New voice message from num='%s', name='%s'\n", (char *)param1, (char *)param2);
		break;
	case VPMSG_ABUPDATE:
		printf("User '%s' has number '%s'\n", (char *)param1, (char *)param2);
		break;
	case VPMSG_USERSEARCH:
		printf("User search returned:\n%s", param2);
		break;
	}
	printf("> ");
	fflush(stdout);
	return rc;
}

void notifyprint(void *uparam, unsigned msg, unsigned pcall, unsigned param)
{
	char s[300];
	VPCALL vpcall = (VPCALL)pcall;

	printf("\r");
	switch(msg)
	{
	case VPMSG_SERVERSTATUS:
		printf("Server status: %s (%x)\n", Reason(pcall), param);
		break;
	case VPMSG_INVALIDADDR:
		printf("User not found for call %d\n", pcall);
		break;
	case VPMSG_CALLFORWARDING:
		{
			int op;

			vp->GetCallForwardingStatus(&op, s);
			if(op == 1)
				printf("Call forwarding disabled\n");
			else printf("Call forwarding status: %s to %s\n", op == 3 ? "Enabled on offline" : "Enabled", s);
		}
		break;
	case VPMSG_SERVERTRANSFERFINISHED:
		printf("Server transfer op %d finished with error %d\n", pcall, param);
		break;
	case VPMSG_SERVERNOTRESPONDING:
		printf("Server not responding for call %d\n", pcall);
		break;
	case VPMSG_RESOLVED:
		printf("Address for call %d found\n", pcall);
		break;
	case VPMSG_USERALERTED:
		printf("User alerted for call %d\n", pcall);
		break;
	case VPMSG_CALLREFUSED:
		printf("The user has not accepted the call %d, reason=%s\n", pcall, Reason(param));
		vp->GetCallText(vpcall, s);
		if(*s)
			printf("The user left a message: %s\n", s);
		break;
	case VPMSG_CALLACCEPTED:
		printf("User accepted call %d\n", pcall);
		break;
	case VPMSG_CALLACCEPTEDBYUSER:
		printf("User explicitly accepted call %d\n", pcall);
		break;
	case VPMSG_CONNECTFINISHED:
		printf("Connection procedure finished for call %d\n", pcall);
		break;
	case VPMSG_CALLESTABLISHED:
		printf("Call %d established\n", pcall);
		break;
	case VPMSG_CALLSETUPFAILED:
		printf("Call setup failed for call %d\n", pcall);
		break;
	case VPMSG_CONNECTTIMEOUT:
		printf("Remote device not responding for call %d\n", pcall);
		break;
	case VPMSG_CALLDISCONNECTED:
		printf("The remote user has terminated the call %d, reason %s\n", pcall, Reason(param));
		break;
	case VPMSG_CALLENDED:
		printf("Call %d ended, reason %s\n", pcall, Reason(param));
		vp->FreeCall(vpcall);
		break;
	case VPMSG_NEWCALL:
		if(vp->GetCallBearer(vpcall) == BC_SMS)
		{
			char text[MAXTEXTLEN+1], name[MAXNAMELEN+1];
			unsigned n, tm, bc;
			static const char *bearernames[] = {"voice", "video", "chat", "sms", "file transfer", "video", "video message"};

			vp->GetCallRemoteName(vpcall, name);
			if(!vp->GetMissedCallsData(vpcall, &n, &tm, &bc) && n)
			{
				sprintf(text, "You have been called %d times by %s. The last call was a %s call and was made on ",
					n, name, bc > 6 ? "unknown" : bearernames[bc]);
				utimetotext(tm, text + strlen(text));
				printf("%s.\n", text);
			} else {
				unsigned smstype, smsid;

				vp->GetCallRemoteName(vpcall, name);
				vp->GetCallText(vpcall, text);
				vp->GetSMSType(vpcall, &smstype);
				vp->GetSMSId(vpcall, &smsid);
				printf("SMS (type %d, id %d) from %s: %s\n", smstype, smsid, name, text);
			}
			vp->FreeCall(vpcall);
		} else {
			char name[MAXNAMELEN+1];
			
			vp->GetCallRemoteName(vpcall, name);
			if(vp->GetCallStatus(vpcall) == VPCS_CONNECTED)
				printf("Automatic new call %d from %s\n", pcall, name);
			else printf("Incoming new call %d from %s\n", pcall, name);
		}
		break;
	case VPMSG_KEYPAD:
		printf("Keypad '%c' (0x%X) received from call %d\n", param, param, pcall);
		break;
	case VPMSG_REMOTELYCONFERENCED:
		printf("Call %d remotely conferenced\n", pcall);
		break;
	case VPMSG_REMOTELYUNCONFERENCED:
		printf("Call %d remotely taken out from conference\n", pcall);
		break;
	case VPMSG_REMOTELYHELD:
		printf("Call %d remotely held\n", pcall);
		break;
	case VPMSG_REMOTELYRESUMED:
		printf("Call %d remotely resumed\n", pcall);
		break;
	case VPMSG_CONFERENCEESTABLISHED:
		printf("Call %d added to conference (we are master), detach=%d\n", pcall, param);
		break;
	case VPMSG_CONFERENCEFAILED:
		printf("Call %d could not be added to conference (we are master), detach=%d\n", pcall, param);
		break;
	case VPMSG_CONFERENCEREQ:
		printf("Call %d is asking us to be added to a conference\n", pcall);
		vp->AcceptConference(vpcall, true);
		break;
	case VPMSG_CONFERENCECALLESTABLISHED:
		printf("Call %d in conference (we are slave)\n", pcall);
		break;
	case VPMSG_CONFERENCEEND:
		printf("Call %d (master) has terminated the conference (we are slave)\n", pcall);
		break;
	case VPMSG_CALLTRANSFERRED:
		printf("Call %d has been transferred to the new call %d\n", pcall, param);
		break;
	case VPMSG_CHAT:
		vp->GetChatText(vpcall, s, 300);
		printf("Chat message: %s\n", s);
		break;
	case VPMSG_CHATACK:
		vp->GetChatText(vpcall, s, 300);
		printf("Chat message sent: %s\n", s);
		break;
	case VPMSG_SENDFILEACK_REFUSED:
		printf("File transfer refused (call=%d)\n", pcall);
		break;
	case VPMSG_SENDFILEACK_USERALERTED:
		printf("File transfer user alerted  (call=%d)\n", pcall);
		break;
	case VPMSG_SENDFILEACK_ACCEPTED:
		printf("File transfer accepted (call=%d)\n", pcall);
		break;
	case VPMSG_FTPROGRESS:
		{
			unsigned cur, tot;

			vp->GetFileProgress(vpcall, &cur, &tot);
			printf("Progress %u/%u\n", cur, tot);
			break;
		}
		break;
	case VPMSG_FTTIMEOUT:
		printf("File transfer timeout (call=%d)\n", pcall);
		break;
	case VPMSG_FTTXCOMPLETE:
	case VPMSG_FTRXCOMPLETE:
		printf("File transfer complete (call=%d)\n", pcall);
		break;
	case VPMSG_SENDFILEREQ:
		vp->AcceptFile(vpcall, 1);
		break;
	case VPMSG_SENDFILESUCCESS:
		printf("File transfer finished successfully (call=%d)\n", pcall);
		break;
	case VPMSG_SENDFILEFAILED:
		printf("File transfer finished successfully (call=%d)\n", pcall);
		break;
	case VPMSG_QUERYONLINEACK:
		if(!pcall)	// Query online ack finished
		{
			int i;

			for(i = 0; i < aol.N; i++)
				printf("%03d%07d: %s\n", (int)aol.nums[i].srvid, (int)aol.nums[i].num, aol.nums[i].online == AOL_OFFLINE ? "Offline" :
			aol.nums[i].online == AOL_ONLINE ? "Online" : aol.nums[i].online == AOL_LIMITED ? "Accepts only contacts" : "Unknown");
			free(aol.nums);
			aol.nums = 0;
		}
		break;
	case VPMSG_CREDIT:
		printf("Credit is %.2f $\n", pcall / 10000.0);
		if(param != (unsigned)-1)
			printf("SMS credit is %d units\n", param);
		break;
#ifdef INCLUDEVIDEO
	case VPMSG_WINDOWCLOSED:
		if(localvideo)
		{
			delete localvideo;
			localvideo = 0;
		}
		break;
	case VPMSG_VIDEOSTARTOK:
		printf("Video started (call %d)\n", param);
		break;
	case VPMSG_VIDEOSTARTERROR:
		printf("Video could not start (call %d)\n", param);
		break;
#endif
	default:
		printf("\rMessage %u, pcall %u, param %u\n", msg, pcall, param);
	}
	printf("> ");
	fflush(stdout);
}

#ifdef _WIN32
void NotifyRoutineVP(void *param, unsigned message, unsigned param1, unsigned param2)
{
	PostThreadMessage(notifythreadid, WM_APP + message, param1, param2);
}

void notifythread(void *dummy)
{
	MSG msg;

	notifythreadid = GetCurrentThreadId();
	while(GetMessage(&msg, 0, 0, 0))
	{
		if(msg.message >= WM_APP && msg.message < WM_APP+1000)
			notifyprint(0, msg.message - WM_APP, msg.wParam, msg.lParam);
	}
}
#endif

int Cmds(int argc, char **argv)
{
	int rc;
	VPCALL call;

	if(argc < 1)
		return 0;
	if(!stricmp(argv[0], "register"))
	{
		if(argc >= 4 && argc <= 5 && !stricmp(argv[1], "vp"))
		{
			if(vp)
				delete vp;
			vp = CreateVPSTACK(&vpaudiofactory);
			if(argc == 5)
				vp->SetServers(argv[4]);
#ifdef INCLUDEVIDEO
			vp->SetVideoDataFactory(&vpvideofactory);
			vp->SetDefaultVideoParameters(mmioFOURCC('H','2','6','4'), 160, 120, 10, 0x35);
#endif
#ifdef _WIN32
			vp->SetNotifyRoutine(NotifyRoutineVP, 0);
#else
			vp->SetNotifyRoutine(notifyprint, 0);
#endif
			vp->SetSyncNotifyRoutine(syncnotifyprint, 0);
			if(vp->Init())
			{
				puts("Cannot initialize vphone stack");
				delete vp;
				vp = 0;
				return 0;
			}
			vp->SetSupportedBearersMask((1<<BC_VOICE) | (1<<BC_SMS) | (1<<BC_AUDIOVIDEO) | (1<<BC_CHAT) | (1<<BC_FILE) | (1<<BC_VIDEOMSG));
			printf("Logging on as %s\n", argv[2]);
			vp->Logon(argv[2], argv[3]);
		} else if(argc >= 5 && argc <= 6 && !stricmp(argv[1], "sip"))
		{
			if(vp)
				delete vp;
			vp = CreateSIPSTACK(&vpmixingaudiofactory);
#ifdef _WIN32
			vp->SetNotifyRoutine(NotifyRoutineVP, 0);
#else
			vp->SetNotifyRoutine(notifyprint, 0);
#endif
			vp->SetSyncNotifyRoutine(syncnotifyprint, 0);
			if(vp->Init())
			{
				puts("Cannot initialize SIP stack");
				delete vp;
				vp = 0;
				return 0;
			}
			vp->SetSupportedBearersMask((1<<BC_VOICE));
			vp->SetServers(argv[4]);
			if(argc == 6)
				vp->SetSTUNServer(argv[5]);
			vp->Logon(argv[2], argv[3]);
#ifdef _WIN32
		} else if(argc == 3 && !stricmp(argv[1], "gsm"))
		{
			if(vp)
				delete vp;
			vp = CreateGSMSTACK(&vpaudiofactory);
			vp->SetServers(argv[2]);
			vp->SetNotifyRoutine(NotifyRoutineVP, 0);
			vp->SetNotifyRoutine(notifyprint, 0);
			vp->SetSyncNotifyRoutine(syncnotifyprint, 0);
			if(vp->Init())
			{
				puts("Cannot initialize GSM stack");
				delete vp;
				vp = 0;
				return 0;
			}
#endif
		}
	} else if(!stricmp(argv[0], "quit"))
		return -1;
	else if(!stricmp(argv[0], "dial") && argc == 2)
	{
		if(!vp)
		{
			printf("Register first\n");
			return 0;
		}
		rc = vp->Connect(&call, argv[1], BC_VOICE);
		if(!rc)
			printf("Connecting call %d\n", (int)call);
		else printf("Error: %s\n", VPError(rc));
	} 
	else if(!stricmp(argv[0], "sendchat") && argc == 3)
	{
		if(!vp)
		{
			printf("Register first\n");
			return 0;
		}
		if(vp->GetStackType() != STACKTYPE_VP)
		{
			printf("Only VPSTACK supports this call\n");
			return 0;
		}
		rc = vp->SendChat((VPCALL)atoi(argv[1]), argv[2]);
		if(!rc)
			printf("Sending chat to %d\n", atoi(argv[1]));
		else printf("Error: %s\n", VPError(rc));
	}
	else if(!stricmp(argv[0], "sendfile") && argc == 3)
	{
		if(!vp)
		{
			printf("Register first\n");
			return 0;
		}
		if(vp->GetStackType() != STACKTYPE_VP)
		{
			printf("Only VPSTACK supports this call\n");
			return 0;
		}
		rc = vp->SendFile((VPCALL)atoi(argv[1]), argv[2]);
		if(!rc)
			printf("Sending file to %d\n", atoi(argv[1]));
		else printf("Error: %s\n", VPError(rc));
	}
	else if(!stricmp(argv[0], "filecall") && argc == 2)
	{
		if(!vp)
		{
			printf("Register first\n");
			return 0;
		}
		rc = vp->Connect(&call, argv[1], BC_FILE);
		if(!rc)
			printf("Connecting call %d\n", (int)call);
		else printf("Error: %s\n", VPError(rc));
	}
	else if(!stricmp(argv[0], "chatcall") && argc == 2)
	{
		if(!vp)
		{
			printf("Register first\n");
			return 0;
		}
		rc = vp->Connect(&call, argv[1], BC_CHAT);
		if(!rc)
			printf("Connecting call %d\n", (int)call);
		else printf("Error: %s\n", VPError(rc));
	}
	else if(!stricmp(argv[0], "callforwarding") && argc >= 2 && argc <= 3)
	{
		if(!vp)
		{
			printf("Register first\n");
			return 0;
		}
		rc = vp->CallForwardingRequest(atoi(argv[1]), argc == 3 ? argv[2] : "");
		if(!rc)
			printf("Call forwarding request sent\n");
		else printf("Error: %s\n", VPError(rc));
	}
	else if(!stricmp(argv[0], "transfer") && argc == 3)
	{
		if(!vp)
		{
			printf("Register first\n");
			return 0;
		}
		VPCALL vpcalls[2];
		vpcalls[0] = (VPCALL)atoi(argv[1]);
		vpcalls[1] = (VPCALL)atoi(argv[2]);
		rc = vp->ConferenceCalls(vpcalls, 2, true);
		if(!rc)
			printf("Call transfer initiated\n");
		else printf("Error: %s\n", VPError(rc));
	} else if(!stricmp(argv[0], "confadd") && argc == 2)
	{
		if(!vp)
		{
			printf("Register first\n");
			return 0;
		}
		rc = vp->AddToConference((VPCALL)atoi(argv[1]));
		if(!rc)
			printf("Call added to conference\n");
		else printf("Error: %s\n", VPError(rc));
	} else if(!stricmp(argv[0], "confrm") && argc == 2)
	{
		if(!vp)
		{
			printf("Register first\n");
			return 0;
		}
		rc = vp->RemoveFromConference((VPCALL)atoi(argv[1]));
		if(!rc)
			printf("Call removed from conference\n");
		else printf("Error: %s\n", VPError(rc));
	} else if(!stricmp(argv[0], "confall"))
	{
		if(!vp)
		{
			printf("Register first\n");
			return 0;
		}
		rc = vp->ConferenceAll();
		if(!rc)
			printf("All conferenced\n");
		else printf("Error: %s\n", VPError(rc));
	} else if(!stricmp(argv[0], "confstop"))
	{
		if(!vp)
		{
			printf("Register first\n");
			return 0;
		}
		rc = vp->StopConference();
		if(!rc)
			printf("Conference stopped\n");
		else printf("Error: %s\n", VPError(rc));
	}
#ifdef INCLUDEVIDEO
	else if(!stricmp(argv[0], "conference") && argc > 2)
	{
		if(!vp)
		{
			printf("Register first\n");
			return 0;
		}
		VPCALL vpcalls[10];
		int i;

		for(i = 0; i < argc-1; i++)
			vpcalls[i] = (VPCALL)atoi(argv[i+1]);
		rc = vp->ConferenceCalls(vpcalls, argc-1, false);
		if(!rc)
			printf("Conference initiated\n");
		else printf("Error: %s\n", VPError(rc));
	}
	else if(!stricmp(argv[0], "capturevideoimage") && argc == 3)
	{
		void *image;
		int w, h;

		if(!vp)
		{
			printf("Register first\n");
			return 0;
		}
		rc = vp->CaptureVideoImage((VPCALL)atoi(argv[1]), &image, &w, &h);
		if(!rc)
		{
			// Create a BMP file
			int f = openfile(argv[2], O_WRONLY | O_TRUNC | O_CREAT);
			if(f != -1)
			{
				char bm[2];

				struct {
					unsigned size, reserved, bits;;
					unsigned biSize;
					unsigned biWidth, biHeight;
					unsigned short biPlanes, biBitCount;
					unsigned biCompression, biSizeImage;
					unsigned notused[4];
				} bmpheader;
				bm[0] = 'B';
				bm[1] = 'M';
				memset(&bmpheader, 0, sizeof bmpheader);
				bmpheader.size = sizeof(bmpheader)+2+3*w*h;
				bmpheader.bits = sizeof(bmpheader)+2;
				bmpheader.biSize = 40;
				bmpheader.biWidth = w;
				bmpheader.biHeight = h;
				bmpheader.biPlanes = 1;
				bmpheader.biBitCount = 24;
				bmpheader.biSizeImage = w*h;
				writefile(f, bm, 2);
				writefile(f, &bmpheader, sizeof bmpheader);
				writefile(f, image, w*h*3);
				closefile(f);
			} else printf("Error creating file\n");
			VPFreeBuffer(image);
		}
	}
	else if(!stricmp(argv[0], "videocall") && argc == 2)
	{
		if(!vp)
		{
			printf("Register first\n");
			return 0;
		}
		rc = vp->Connect(&call, argv[1], BC_AUDIOVIDEO);
		if(!rc)
			printf("Connecting call %d\n", (int)call);
		else printf("Error: %s\n", VPError(rc));
	} 
	else if(!stricmp(argv[0], "localvideo"))
	{
		if(localvideo)
		{
			delete localvideo;
			localvideo = 0;
		} else {
#ifdef INCLUDEAVC
			localvideo = CreateVPVIDEO(0, 0, mmioFOURCC('V','P','3','1'), 176, 144, mmioFOURCC('V','P','3','1'), 176, 144, 10, 0x122);
			//localvideo = CreateVPVIDEO(0, 0, mmioFOURCC('H','2','6','4'), 176, 144, mmioFOURCC('H','2','6','4'), 176, 144, 10, 0x122);
#else
			localvideo = CreateVPVIDEO(0, 0, mmioFOURCC('V','P','3','1'), 176, 144, mmioFOURCC('V','P','3','1'), 176, 144, 10, 0x122);
#endif
			localvideo->SetNotifyRoutine(NotifyRoutineVP, 0);
			if(localvideo->Start())
			{
				delete localvideo;
				localvideo = 0;
				printf("Cannot start video\n");
			}
		}
	}
	else if(!stricmp(argv[0], "listvideodev"))
	{
		int n = 10, i;
		TCHAR cdev[10][100];
		if(!VPVIDEO_EnumerateCaptureDevices(cdev, &n) && n)
		{
			for(i = 0; i < n; i++)
				printf("%d: %s\n", i, cdev[i]);
		}
	} else if(!stricmp(argv[0], "usevideodev") && argc == 2)
	{
		int n = 10, i = atoi(argv[1]);
		TCHAR cdev[10][100];
		if(!VPVIDEO_EnumerateCaptureDevices(cdev, &n) && n && i >= 0 && i < n)
		{
			printf("Using %s\n", cdev[i]);
			VPVIDEO_SetCaptureDevice(cdev[i]);
		}
	} else if(!stricmp(argv[0], "avirec") && argc == 2)
	{
		if(!vp)
		{
			printf("Register first\n");
			return 0;
		}
		if(vp->GetStackType() != STACKTYPE_VP)
		{
			printf("Only VPSTACK supports this call\n");
			return 0;
		}
		rc = vp->RecordAVIFile(argv[1]);
		if(!rc)
		{
			printf("Recording initiated\n");
		} else printf("Error: %s\n", VPError(rc));
	} else if(!stricmp(argv[0], "wavrec") && argc == 2)
	{
		if(!vp)
		{
			printf("Register first\n");
			return 0;
		}
		if(vp->GetStackType() != STACKTYPE_VP)
		{
			printf("Only VPSTACK supports this call\n");
			return 0;
		}
		rc = vp->RecordWAVFile(argv[1]);
		if(!rc)
			printf("Recording initiated\n");
		else printf("Error: %s\n", VPError(rc));
	} else if(!stricmp(argv[0], "loglevel") && argc == 2)
		generatelog = atoi(argv[1]);
	else if(!stricmp(argv[0], "recordcall") && argc == 2)
	{
		if(!stricmp(argv[1], "stop"))
			VPMIXINGAUDIO_RecordToFile(0);
		else VPMIXINGAUDIO_RecordToFile(argv[1]);
	} else if(!stricmp(argv[0], "avistop") || !stricmp(argv[0], "wavstop"))
	{
		if(!vp)
		{
			printf("Register first\n");
			return 0;
		}
		if(vp->GetStackType() != STACKTYPE_VP)
		{
			printf("Only VPSTACK supports this call\n");
			return 0;
		}
		rc = vp->StopAVIRecord();
		if(!rc)
			printf("Recording finished\n");
		else printf("Error: %s\n", VPError(rc));
	} else if(!stricmp(argv[0], "aviplay") && argc == 2)
	{
		RECT rect;

		if(aviplayer)
			delete aviplayer;
		aviplayer = CreateAVIPLAYER();
		rect.left = rect.top = 0;
		rect.right = 176;
		rect.bottom = 144;
		aviplayer->SetWindowData((HWINDOW)GetDesktopWindow(), (RECTANGLE *)&rect);
		rc = aviplayer->PlayAVIFile(argv[1]);
		if(!rc)
		{
			printf("Play initiated\n");
			{
				MSG msg;
				while(GetMessage(&msg, 0, 0, 0))
				{
					DispatchMessage(&msg);
					if(msg.message == WM_APP)
						break;
				}
			}
		} else printf("Error opening file\n");
	}
	else if(!stricmp(argv[0], "wavplay") && argc == 2)
	{
		RECT rect;

		if(aviplayer)
			delete aviplayer;
		aviplayer = CreateAVIPLAYER();
		rect.left = rect.top = 0;
		rect.right = 176;
		rect.bottom = 144;
		aviplayer->SetWindowData(GetDesktopWindow(), (RECTANGLE *)&rect);
		rc = aviplayer->PlayWAVFile(argv[1], 0);
		if(!rc)
			printf("Play initiated\n");
		else printf("Error opening file\n");
	} else if(!stricmp(argv[0], "playstop"))
	{
		if(aviplayer)
			delete aviplayer;
	}
#endif
	else if(!stricmp(argv[0], "disc") && argc == 2)
	{
		if(!vp)
		{
			printf("Register first\n");
			return 0;
		}
		rc = vp->Disconnect((VPCALL)atoi(argv[1]), REASON_NORMAL);
		if(!rc)
			printf("Disconnecting call %d\n", atoi(argv[1]));
		else printf("Error: %s\n", VPError(rc));
	} else if(!stricmp(argv[0], "answer") && argc == 3)
	{
		if(!vp)
		{
			printf("Register first\n");
			return 0;
		}
		strcpy(greeting, argv[2]);
		rc = vp->AnswerCall((VPCALL)atoi(argv[1]));
		if(!rc)
			printf("Answering call %d\n", atoi(argv[1]));
		else printf("Error: %s\n", VPError(rc));
	} else if(!stricmp(argv[0], "answer") && argc == 2)
	{
		if(!vp)
		{
			printf("Register first\n");
			return 0;
		}
		rc = vp->AnswerCall((VPCALL)atoi(argv[1]));
		if(!rc)
			printf("Answering call %d\n", atoi(argv[1]));
		else printf("Error: %s\n", VPError(rc));
	} else if(!stricmp(argv[0], "sms") && argc >= 2)
	{
		if(!vp)
		{
			printf("Register first\n");
			return 0;
		}
		call = vp->CreateCall();
		if(call)
		{
			if(argc == 3)
				vp->SetCallText(call, argv[2]);
			else vp->SetMissedCallBC(call, BC_VOICE);
			rc = vp->Connect(call, argv[1], BC_SMS);
			if(!rc)
				printf("Sending message %d\n", (int)call);
			else printf("Error: %s\n", VPError(rc));
		} else printf("Error: Cannot create call\n");
	} else if(!stricmp(argv[0], "queryonline") && argc >= 2)
	{
		int i;

		if(!vp)
		{
			printf("Register first\n");
			return 0;
		}
		if(vp->GetStackType() != STACKTYPE_VP)
		{
			printf("Only VPSTACK supports this call\n");
			return 0;
		}
		aol.notificationreqop = NOTIFICATIONREQ_ENABLE;
		aol.N = 0;
		if(aol.nums)
			free(aol.nums);
		aol.nums = (AOLNUM *)malloc(sizeof(AOLNUM) * (argc-1));
		for(i = 1; i < argc; i++)
		{
			if(strlen(argv[i]) == 10)
			{
				aol.nums[aol.N].srvid = (argv[i][0] - '0') * 100 + (argv[i][1] - '0') * 10 + (argv[i][2] - '0');
				aol.nums[aol.N].num = atoi(argv[i]+3);
				aol.nums[aol.N].online = 0;
				aol.N++;
			}
		}
		vp->AskOnline(&aol);
	} else if(!stricmp(argv[0], "aes") && argc == 2)
	{
		if(!stricmp(argv[1], "low"))
			VPMIXINGAUDIO_SetAcousticEchoSuppression(AES_LOW);
		else if(!stricmp(argv[1], "high"))
			VPMIXINGAUDIO_SetAcousticEchoSuppression(AES_HIGH);
		else VPMIXINGAUDIO_SetAcousticEchoSuppression(AES_NONE);
	} else if(!stricmp(argv[0], "aec") && argc == 2)
	{
		if(!stricmp(argv[1], "short"))
			rc=  VPMIXINGAUDIO_SetAcousticEchoCancellation(AEC_SHORT);
		else if(!stricmp(argv[1], "long"))
			rc =VPMIXINGAUDIO_SetAcousticEchoCancellation(AEC_LONG);
		else rc = VPMIXINGAUDIO_SetAcousticEchoCancellation(AEC_NONE);
		if(rc)
			printf("Acoustic Echo Cancellation not built in\n");
	} else if(!stricmp(argv[0], "credit"))
	{
		if(!vp)
		{
			printf("Register first\n");
			return 0;
		}
		if(vp->GetStackType() != STACKTYPE_VP)
		{
			printf("Only VPSTACK supports this call\n");
			return 0;
		}
		vp->QueryAccountInfo();
	} else if(!stricmp(argv[0], "servertransfer") && argc >= 3 && argc <= 4)
	{
		if(!vp)
		{
			printf("Register first\n");
			return 0;
		}
		if(vp->GetStackType() != STACKTYPE_VP)
		{
			printf("Only VPSTACK supports this call\n");
			return 0;
		}
		vp->ServerTransfer(atoi(argv[1]), argv[2], false, argc == 4 ? argv[3] : 0);
	} else if(!stricmp(argv[0], "usersearch") && argc >= 3 && argc <= 4)
	{
		if(!vp)
		{
			printf("Register first\n");
			return 0;
		}
		if(vp->GetStackType() != STACKTYPE_VP)
		{
			printf("Only VPSTACK supports this call\n");
			return 0;
		}
		if(argc == 3)
			vp->UserSearch(argv[1], 0, atoi(argv[2]), 0);
		else vp->UserSearch(argv[1], argv[3], atoi(argv[2]), 0);
	} else printf("Unrecognized command\n");
	return 0;
}


//  Copyright __MyCompanyName__ 2010. All rights reserved.

#import "iostestAppDelegate.h"
#import "../../vpstack/vpstack.h" 
#import "../../vpstack/sip.h"
#import "../../vpstack/mixingaudio.h"
#import <UIKit/UIKit.h>


#import "ApplicationManager.h"


#import "GetTextController.h"



// globals...
VPMIXINGAUDIOFACTORY vpmixingaudiofactory;
IVPSTACK			*vp = NULL;
VPCALL				gvpcall;
id					iostest = nil;
AOL					aol;
VPCALL				chatCall;
VPCALL				fileCall;
BOOL				chatConnected = NO;

// global functions
void VPRoutineNotify(void *param, unsigned msg, unsigned param1, unsigned param2) {
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	[iostest notifyPrint:0 message:msg call:param1 parameter:param2];
	[pool release];
}

int VPRoutineSyncNotify(void *uparam, unsigned msg, unsigned param1, unsigned param2) {
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	int ret = [iostest syncNotifyPrint:uparam message:msg parameter1:param1 parameter2:param2]; 
	[pool release];
	return ret;
}


@implementation iostestAppDelegate
@synthesize window;

- (void)applicationDidFinishLaunching:(UIApplication *)application {
    // Override point for customization after application launch
    [window makeKeyAndVisible];
}

- (id)init {  
	[super init];
	iostest = self;
	return self; 
}

- (void)dealloc {
    [window release];
    [super dealloc];
}

- (void)awakeFromNib {

	VPInit();
	VPMIXINGAUDIO_Init();
	//VPVIDEO_Init();
//#if TARGET_IPHONE_SIMULATOR
//	[self vpRegister:"voip2cartest" password:"voip2car"];
//#else
//	[self vpRegister:"VIC_TEST2" password:"klaz197"];
//#endif
}

- (NSString*)Reason:(int)reasonCode {

	static const NSString* reasons[36] = {
			@"NORMAL",			@"BEARER_NOT_SUPPORTED",	@"CODEC_NOT_SUPPORTED",  
			@"CALLPROCESSING",	@"MISSINGCLID", 			@"CALLDEFLECTION", 
			@"BUSY",			@"FILESAME",				@"TIMEOUT", 
			@"DISCONNECTED",	@"AUTHERROR",				@"NOTFOUND", 
			@"SYNTAXERROR",		@"INVALIDNUMBER",			@"STORAGEEXCEEDED", 
			@"ALREADYCONNECTED", @"PROTOCOLNOTSUPPORTED",	@"SERVERREDIRECTION", 
			@"ADMINERROR",		@"ACCOUNTBLOCKED",			@"CALLTRANSFERCOMPLETE",
			@"NOQUOTA",			@"USEROFFLINE",				@"SERVERNOTRESPONDING", 
			@"SERVERNOTRUNNING", @"VPHONENOTRESPONDING",	@"VPHONENOTRUNNING", 
			@"CALLPROCEEDING",	@"CONNECTIONLOST",			@"WRONGVERSION", 
			@"EARLYAUDIO",		@"ANOTHERPORT",				@"ANOTHERADDR", 
			@"CANNOTNOTIFY",	@"NOTKNOWN",				@"LOOPCALL"
	};
		
	if(reasonCode >= 0 && reasonCode <= 35)
		return reasons[reasonCode];
	if(reasonCode == REASON_LOGGEDON)
		return @"Logged on";
	if(reasonCode == REASON_VERSIONOLD)
		return @"Version old";
	if(reasonCode == REASON_SERVERRESET)
		return @"No server running on the specified host";
	return @"Unknown";
}

- (NSString*)VPError:(int)error {
	
	static const NSString *errors[14] = {
		@"Invalid call",	@"No more calls available",		@"TCP/IP error",
		@"Invalid address", @"Calling yourself",			@"Invalid status", 
		@"Not logged on",	@"Incompatible calls",			@"File not found", 
		@"Invalid file format", @"Unsupported call",		@"Unsupported bearer", 
		@"Invalid parameter",	@"Timeout"
	};
	
	if(error >= -14 && error <= -1)
		return errors[-error - 1];
	return @"Unknown";
}

- (void)logEntryDisplay {
	
	NSDate *today = [NSDate date];
	NSDateFormatter *dateFormat = [[[NSDateFormatter alloc] init] autorelease];
	
	[dateFormat setDateFormat:@"HH:mm:ss"];
	NSString *dateString = [dateFormat stringFromDate:today];
	
	[textView 
	 setText:[NSString stringWithFormat:@"%@> %@\n%@",
			  dateString, stringLogEntry, [textView text]]
	 ];
}

- (void)logEntry:(NSString*)item {
	stringLogEntry = item;
	NSLog(stringLogEntry);
	[self performSelectorOnMainThread:@selector(logEntryDisplay) withObject:nil waitUntilDone:true];
}	

- (int)syncNotifyPrint:(void *)uparam message:(unsigned)msg parameter1:(unsigned)param1 parameter2:(unsigned)param2 {
	
	int rc = 0;
	switch(msg)
	{
		case VPMSG_NOTIFYLOGON:	
			[loginButton setHidden: YES];
			[self logEntry:[NSString stringWithFormat:@"Num='%s' logged on", (const char *)param1]];
			break;
		case VPMSG_NEWVOICEMSG:
			[self logEntry:[NSString stringWithFormat:@"New voice message from num='%s', name='%s'", (const char *)param1, (const char *)param2]];
			break;
		case VPMSG_ABUPDATE:
			[self logEntry:[NSString stringWithFormat:@"User '%s' has number '%s'", (const char *)param1, (const char *)param2]];
			break;
	}
	return rc;
}


- (void)notifyPrint:(void *)uparam message:(unsigned)msg call:(unsigned)pcall parameter:(unsigned)param {

	char s[300];
	VPCALL vpcall = (VPCALL)pcall;
	
	switch(msg)
	{
		case VPMSG_SERVERSTATUS:
			[self logEntry:[NSString stringWithFormat:
							@"Server status: %@ (%x)", [self Reason:pcall], param]];
			if (vp->IsLoggedOn())
				[loginButton performSelectorOnMainThread:@selector(removeFromSuperview) withObject:nil waitUntilDone:NO];
			break;
		case VPMSG_INVALIDADDR:
			[self logEntry:[NSString stringWithFormat:@"User not found for call %d", pcall]];
			break;
		case VPMSG_CALLFORWARDING:
		{
			int op;
			vp->GetCallForwardingStatus(&op, s);
			if(op == 1) {
				[self logEntry:@"Call forwarding disabled"];
			} else {
				[self logEntry:[NSString stringWithFormat:@"Call forwarding status: %s to %s", op == 3 ? "Enabled on offline" : "Enabled", s]];
			}
			break;
		}
		case VPMSG_SERVERTRANSFERFINISHED:
			[self logEntry:[NSString stringWithFormat:@"Server transfer op %d finished with error %d", pcall, param]];
			break;
		case VPMSG_SERVERNOTRESPONDING:
			[self logEntry:[NSString stringWithFormat:@"Server not responding for call %d", pcall]];
			break;
		case VPMSG_RESOLVED:
			[self logEntry:[NSString stringWithFormat:@"Address for call %d found", pcall]];
			break;
		case VPMSG_USERALERTED:
			[self logEntry:[NSString stringWithFormat:@"User alerted for call %d", pcall]];
			break;
		case VPMSG_CALLREFUSED: {
			[self logEntry:[NSString stringWithFormat:@"The user has not accepted the call %d, reason=%@", pcall, [self Reason:param]]];
			vp->GetCallText(vpcall, s);
			if(*s) {
				[self logEntry:[NSString stringWithFormat:@"The user left a message: %s", s]];
			}
			break;
		}
		case VPMSG_CALLACCEPTED:
			[self logEntry:[NSString stringWithFormat:@"User accepted call %d", pcall]];
			break;
		case VPMSG_CALLACCEPTEDBYUSER:
			[self logEntry:[NSString stringWithFormat:@"User explicitly accepted call %d", pcall]];
			break;
		case VPMSG_CONNECTFINISHED:
			[self logEntry:[NSString stringWithFormat:@"Connection procedure finished for call %d", pcall]];
			if (vp->GetCallBearer(vpcall) == BC_FILE){
				NSLog(@"%@", [[NSBundle mainBundle] pathForResource:@"Info" ofType:@"plist"]);
				vp->SendFile(vpcall, [[[NSBundle mainBundle] pathForResource:@"Info" ofType:@"plist"] UTF8String]);
			}
			break;
		case VPMSG_CALLESTABLISHED:
			if (vp->GetCallBearer(vpcall) == BC_CHAT){
				chatConnected = YES;
				chatCall = vpcall;
			}
			[self logEntry:[NSString stringWithFormat:@"Call %d established", pcall]];
			[SharedManager.soundManager stopRingTone];
			break;
		case VPMSG_CALLSETUPFAILED:
			[self logEntry:[NSString stringWithFormat:@"Call setup failed for call %d", pcall]];
			break;
		case VPMSG_CONNECTTIMEOUT:
			[self logEntry:[NSString stringWithFormat:@"Remote device not responding for call %d", pcall]];
			break;
		case VPMSG_CALLDISCONNECTED:
			[self logEntry:[NSString stringWithFormat:@"The remote user has terminated the call %d, reason %@", 
							pcall, [self Reason:param]]];
			break;
		case VPMSG_CALLENDED: {
			[self logEntry:[NSString stringWithFormat:@"Call %d ended, reason %@", pcall, [self Reason:param]]];
			vp->FreeCall(vpcall);
			if(vpcall == gvpcall) {
				gvpcall = 0;
			}
			if (vpcall == chatCall){
				chatConnected = NO;
			}
			break;
		}
		case VPMSG_NEWCALL:
			if(vp->GetCallBearer(vpcall) == BC_SMS) {
				char text[MAXTEXTLEN+1], name[MAXNAMELEN+1];
				unsigned n, tm, bc;
				static const char *bearernames[] = {"voice", "video", "chat", "sms", "file transfer", "video", "video message"};
				
				vp->GetCallRemoteName(vpcall, name);
				if(!vp->GetMissedCallsData(vpcall, &n, &tm, &bc) && n)
				{
					sprintf(text, "You have been called %d times by %s. The last call was a %s call and was made on ",
							n, name, bc > 6 ? "unknown" : bearernames[bc]);
					utimetotext(tm, text + strlen(text));
					[self logEntry:[NSString stringWithFormat:@"%s.", text]];
				} else {
					vp->GetCallRemoteName(vpcall, name);
					vp->GetCallText(vpcall, text);
					[self logEntry:[NSString stringWithFormat:@"SMS from %s: %s.", name, text]];
				}
				vp->FreeCall(vpcall);
			} else {
				char name[MAXNAMELEN+1];
				
				vp->GetCallRemoteName(vpcall, name);
				if(vp->GetCallStatus(vpcall) == VPCS_CONNECTED) {
					[self logEntry:[NSString stringWithFormat:@"Automatic new call %d from %s", pcall, name]];
				} else {
					[self logEntry:[NSString stringWithFormat:@"Incoming new call %d from %s", pcall, name]];
				}
				if (vp->GetCallBearer(vpcall) == BC_FILE){
					fileCall = vpcall;
				}
				else {
					gvpcall = vpcall;	
				}
				[SharedManager.soundManager playRingTone];
			}
			break;
		case VPMSG_KEYPAD:
			[self logEntry:[NSString stringWithFormat:@"Keypad '%c' (0x%X) received from call %d", param, param, pcall]];
			break;
		case VPMSG_REMOTELYCONFERENCED:
			[self logEntry:[NSString stringWithFormat:@"Call %d remotely conferenced", pcall]];
			break;
		case VPMSG_REMOTELYUNCONFERENCED:
			[self logEntry:[NSString stringWithFormat:@"Call %d remotely taken out from conference", pcall]];
			break;
		case VPMSG_REMOTELYHELD:
			[self logEntry:[NSString stringWithFormat:@"Call %d remotely held", pcall]];
			break;
		case VPMSG_REMOTELYRESUMED:
			[self logEntry:[NSString stringWithFormat:@"Call %d remotely resumed", pcall]];
			break;
		case VPMSG_CONFERENCEESTABLISHED:
			[self logEntry:[NSString stringWithFormat:@"Call %d added to conference (we are master), detach=%d", pcall, param]];
			break;
		case VPMSG_CONFERENCEFAILED:
			[self logEntry:[NSString stringWithFormat:@"Call %d could not be added to conference (we are master), detach=%d", pcall, param]];
			break;
		case VPMSG_CONFERENCEREQ:
			[self logEntry:[NSString stringWithFormat:@"Call %d is asking us to be added to a conference", pcall]];
			vp->AcceptConference(vpcall, true);
			break;
		case VPMSG_CONFERENCECALLESTABLISHED:
			[self logEntry:[NSString stringWithFormat:@"Call %d in conference (we are slave)", pcall]];
			break;
		case VPMSG_CONFERENCEEND:
			[self logEntry:[NSString stringWithFormat:@"Call %d (master) has terminated the conference (we are slave)", pcall]];
			break;
		case VPMSG_CALLTRANSFERRED:
			[self logEntry:[NSString stringWithFormat:@"Call %d has been transferred to the new call %d", pcall, param]];
			break;
		case VPMSG_CHAT:
			vp->GetChatText(vpcall, s, 300);
			[self logEntry:[NSString stringWithFormat:@"Chat message: %s", s]];
			break;
		case VPMSG_CHATACK:
			vp->GetChatText(vpcall, s, 300);
			[self logEntry:[NSString stringWithFormat:@"Chat message sent: %s", s]];
			break;
		case VPMSG_SENDFILEACK_REFUSED:
			[self logEntry:[NSString stringWithFormat:@"File transfer refused (call=%d)", pcall]];
			break;
		case VPMSG_SENDFILEACK_USERALERTED:
			[self logEntry:[NSString stringWithFormat:@"File transfer user alerted  (call=%d)", pcall]];
			break;
		case VPMSG_SENDFILEACK_ACCEPTED:
			[self logEntry:[NSString stringWithFormat:@"File transfer accepted (call=%d)", pcall]];
			break;
		case VPMSG_FTPROGRESS:
		{
			unsigned cur, tot;
			vp->GetFileProgress(vpcall, &cur, &tot);
			[self logEntry:[NSString stringWithFormat:@"Progress %u/%u", cur, tot]];
			break;
		}
			break;
		case VPMSG_FTTIMEOUT:
			[self logEntry:[NSString stringWithFormat:@"File transfer timeout (call=%d)", pcall]];
			break;
		case VPMSG_FTTXCOMPLETE:
		case VPMSG_FTRXCOMPLETE:
			[self logEntry:[NSString stringWithFormat:@"File transfer complete (call=%d)", pcall]];
			break;
		case VPMSG_SENDFILEREQ:
			[self logEntry:[NSString stringWithFormat:@"File transfer request (call=%d)", pcall]];
			vp->AcceptFile(vpcall, 1);
			break;
		case VPMSG_SENDFILESUCCESS:
			[self logEntry:[NSString stringWithFormat:@"File transfer finished successfully (call=%d)", pcall]];
			break;
		case VPMSG_SENDFILEFAILED:
			[self logEntry:[NSString stringWithFormat:@"File transfer finished successfully (call=%d)", pcall]];
			break;
		case VPMSG_QUERYONLINEACK:
			if(!pcall)	// Query online ack finished
			{
				int i;
				for(i = 0; i < aol.N; i++) {
					[self logEntry:[NSString stringWithFormat:@"%03d%07d: %s", 
									(int)aol.nums[i].srvid, (int)aol.nums[i].num, 
									aol.nums[i].online == AOL_OFFLINE ? "Offline" : aol.nums[i].online == AOL_ONLINE ? "Online" : aol.nums[i].online == AOL_LIMITED ? "Accepts only contacts" : "Unknown"]];					
				}
				free(aol.nums);
				aol.nums = 0;
			}
			break;
		case VPMSG_CREDIT:
			[self logEntry:[NSString stringWithFormat:@"Credit is %.2f $", pcall / 10000.0]];
			if(param != (unsigned)-1)
				[self logEntry:[NSString stringWithFormat:@"SMS credit is %d units", param]];
			break;
		case VPMSG_WINDOWCLOSED:
/*			if(localvideo) {
				delete localvideo;
				localvideo = 0;
			}
*/			break;
		case VPMSG_VIDEOSTARTOK:
			[self logEntry:[NSString stringWithFormat:@"Video started (call %d)", param]];
			break;
		case VPMSG_VIDEOSTARTERROR:
			[self logEntry:[NSString stringWithFormat:@"Video could not start (call %d)", param]];
			break;
		default:
			[self logEntry:[NSString stringWithFormat:@"Message %u, pcall %u, param %u", msg, pcall, param]];
	}
}

- (int)vpRegister:(const char *)user password:(const char *)password {
	
	if(vp) {
		delete vp;
	}
	
	vp = CreateVPSTACK(&vpmixingaudiofactory);
#ifdef INCLUDEVIDEO
	vp->SetVideoDataFactory(&vpvideofactory);
#endif
	
	vp->SetNotifyRoutine(VPRoutineNotify, 0);
	vp->SetSyncNotifyRoutine(VPRoutineSyncNotify, 0);
	
	if(vp->Init()) {
		[self logEntry:[NSString stringWithFormat:@"Cannot initialize vphone stack"]];
		delete vp;
		vp = 0;
		return 0;
	}
	
	vp->SetSupportedBearersMask((1<<BC_VOICE) | (1<<BC_SMS) /*| (1<<BC_AUDIOVIDEO)*/ | (1<<BC_CHAT) | (1<<BC_FILE));
	[self logEntry:[NSString stringWithFormat:@"Logging on as %s", user]];
	vp->Logon(user, password);
	
	[SharedManager start];
	
	return 0;
}

#pragma mark -
#pragma mark Actions

- (void) textPicker:(id)sender finishedWithText:(NSDictionary*)result{
	if ([result objectForKey:@"SMS text"]){
		if (vp){
			VPCALL chCall = vp->CreateCall();
			vp->SetCallText(chCall,[[result objectForKey:@"SMS text"] UTF8String]);
			int rc = vp->Connect(chCall, [[result objectForKey:@"Address"] UTF8String], BC_SMS);
			
			
			if(!rc) {
				[self logEntry:[NSString stringWithFormat:@"Sending SMS %d", (int)gvpcall]];
			} else {
				[self logEntry:[NSString stringWithFormat:@"Error send SMS: %@", [self VPError:rc]]];
			}
		}
		return;
	}
	
	if ([result objectForKey:@"Chat Address"]){
		int rc = vp->Connect(&chatCall, [[result objectForKey:@"Chat Address"] UTF8String], BC_CHAT);
		
		if(!rc) {
			[self logEntry:[NSString stringWithFormat:@"Connecting to chat %d", (int)chatCall]];
			chatConnected = YES;
		} else {
			[self logEntry:[NSString stringWithFormat:@"Error connecting to chat: %@", [self VPError:rc]]];
		}
		
		return;
	}
	
	if ([result objectForKey:@"Chat Message"]){
		int rc = vp->SendChat(chatCall, [[result objectForKey:@"Chat Message"] UTF8String]);
		
		if(!rc) {
			[self logEntry:[NSString stringWithFormat:@"Send chat %d", (int)chatCall]];
			chatConnected = YES;
		} else {
			[self logEntry:[NSString stringWithFormat:@"Error sending chat: %@", [self VPError:rc]]];
		}
		
		return;
	}
	
	if ([result objectForKey:@"File recipient"]){
		fileCall = vp->CreateCall();

		int rc = vp->Connect(fileCall, [[result objectForKey:@"File recipient"] UTF8String], BC_FILE);
		if(!rc) {
			[self logEntry:[NSString stringWithFormat:@"Connect for sending File %d", (int)gvpcall]];
		} else {
			[self logEntry:[NSString stringWithFormat:@"Error connect for send File: %@", [self VPError:rc]]];
		}
		
		NSString *path = [NSString stringWithFormat:@"%@/%@.plist",[[NSBundle mainBundle] bundlePath],@"Info"];
		
		char* rawString = (char *)[path UTF8String];
		//		rc = vp->SendFile(fileCall, [[[NSBundle mainBundle] pathForResource:@"iostest-Info" ofType:@"plist"] UTF8String]);
		rc = vp->SendFile(fileCall, rawString);
		if(!rc) {
			[self logEntry:[NSString stringWithFormat:@"Sending File %d", (int)gvpcall]];
		} else {
			[self logEntry:[NSString stringWithFormat:@"Error send File: %@", [self VPError:rc]]];
		}
		
		return;
	}
	
	if ([result objectForKey:@"Login"]){
		
		[self vpRegister:[[result objectForKey:@"Login"] UTF8String] password:[[result objectForKey:@"Password"] UTF8String]];
		return;
	}
	
	[sender release];
}

- (IBAction)clickLogin:(id)sender{
	GetTextController* controller = [[GetTextController alloc] initWithNibName:@"GetTextController" bundle:nil];
	[controller setFields:[NSArray arrayWithObjects:@"Login",@"Password", nil]];
	
	[controller.view setFrame:CGRectMake(0.0f, 20.0f, 320.0f, 460.0f)];
	controller.delegate = self;
	[window addSubview:controller.view];
}

- (IBAction)clickFile:(id)sender{
	GetTextController* controller = [[GetTextController alloc] initWithNibName:@"GetTextController" bundle:nil];
	[controller setFields:[NSArray arrayWithObjects:@"File recipient", nil]];
	
	[controller.view setFrame:CGRectMake(0.0f, 20.0f, 320.0f, 460.0f)];
	controller.delegate = self;
	[window addSubview:controller.view];
}

- (IBAction)clickSMS:(id)sender{
	GetTextController* controller = [[GetTextController alloc] initWithNibName:@"GetTextController" bundle:nil];
	[controller setFields:[NSArray arrayWithObjects:@"Address", @"SMS text", nil]];
	
	[controller.view setFrame:CGRectMake(0.0f, 20.0f, 320.0f, 460.0f)];
	controller.delegate = self;
	[window addSubview:controller.view];
}


- (IBAction)clickChat:(id)sender{
	GetTextController* controller = [[GetTextController alloc] initWithNibName:@"GetTextController" bundle:nil];
	if (!chatConnected){
		[controller setFields:[NSArray arrayWithObjects:@"Chat Address", nil]];
	}
	else {
		[controller setFields:[NSArray arrayWithObjects:@"Chat Message", nil]];
	}

	
	[controller.view setFrame:CGRectMake(0.0f, 20.0f, 320.0f, 460.0f)];
	controller.delegate = self;
	[window addSubview:controller.view];
	return;
	if (vp){
		if (!chatConnected){
			int rc = vp->Connect(&chatCall, [[textEdit text] UTF8String], BC_CHAT);
			
			if(!rc) {
				[self logEntry:[NSString stringWithFormat:@"Connecting to chat %d", (int)chatCall]];
				chatConnected = YES;
			} else {
				[self logEntry:[NSString stringWithFormat:@"Error connecting to chat: %@", [self VPError:rc]]];
			}
		}
		else {
			int rc = vp->SendChat(chatCall, "test chat");
			if(!rc) {
				[self logEntry:[NSString stringWithFormat:@"Send chat %d", (int)chatCall]];
				chatConnected = YES;
			} else {
				[self logEntry:[NSString stringWithFormat:@"Error sending chat: %@", [self VPError:rc]]];
			}
		}

	}
}

- (IBAction)clickCall:(id)sender {
	
	[textEdit resignFirstResponder];
	if (vp != NULL) {
		
		NSString *s = [textEdit text];
		
		int rc = vp->Connect(&gvpcall, [s UTF8String], BC_VOICE);
		if(!rc) {
			[self logEntry:[NSString stringWithFormat:@"Connecting call %d", (int)gvpcall]];
		} else {
			[self logEntry:[NSString stringWithFormat:@"Error: %@", [self VPError:rc]]];
		}
	}
}

- (IBAction)clickHangup:(id)sender {

	if(!gvpcall && !chatCall) {
		return;
	}

	if (vp != NULL) {
		int rc = 0;
		if (chatCall){
			rc = vp->Disconnect(chatCall, REASON_NORMAL);
		}
		else {
			rc = vp->Disconnect(gvpcall, REASON_NORMAL);
		}
		
		[SharedManager.soundManager stopRingTone];
	
		if(!rc) {
			[self logEntry:[NSString stringWithFormat:@"Disconnecting call %d", gvpcall]];
		} else {
			[self logEntry:[NSString stringWithFormat:@"Error: %@", [self VPError:rc]]];
		}
	} 
}

- (IBAction)clickAnswer:(id)sender {
	if (vp != NULL) {
		if (fileCall){
			vp->AnswerCall(fileCall);
			return;
		}
		
		if (gvpcall)
			vp->AnswerCall(gvpcall);
	}
}

- (IBAction)clickClear:(id)sender {
	[textView setText:@""];
}

- (IBAction)clickQuit:(id)sender
{
	exit(0);
}

- (IBAction)textFieldReturn:(id)sender {
	[sender resignFirstResponder];
}


@end

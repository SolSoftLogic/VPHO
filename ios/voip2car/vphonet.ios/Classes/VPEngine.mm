//
//  VPEngine.mm
//  Vphonet

#import "VPSound.h"
#import "VPEngine.h"
#import "VPProfile.h"
#import "VPContactList.h"


NSString *const VPEngineNotification   = @"VPEngineNotification";
NSString *const OnSmsNotification      = @"OnSmsNotification";
NSString *const OnSearchNotification   = @"OnSearchNotification";

//NSString *const VPEngineAsyncNotification = @"VPEngineAsyncNotification"; 

@implementation VPEngine

static VPEngine *vphonetEngineSharedInstance = nil;

void VPRoutineNotify(void *param, unsigned msg, unsigned param1, unsigned param2);
int VPRoutineSyncNotify(void *uparam, unsigned msg, unsigned param1, unsigned param2);

#pragma mark ---- VPEngine singleton object methods ----

+ (VPEngine*)sharedInstance 
{
	@synchronized(self) 
	{
		if (vphonetEngineSharedInstance == nil) 
		{
			[[self alloc] init]; // assignment not done here
			VPInit();
			VPMIXINGAUDIO_Init();
		}
	}
	return vphonetEngineSharedInstance;
}

+ (id)allocWithZone:(NSZone *)zone 
{
	@synchronized(self) 
	{
		if (vphonetEngineSharedInstance == nil) 
		{
			vphonetEngineSharedInstance= [super allocWithZone:zone];
			return vphonetEngineSharedInstance;  // assignment and return on first allocation
		}
	}
	return nil; // on subsequent allocation attempts return nil
}

- (id)copyWithZone:(NSZone *)zone
{
	return self;
}

- (id)retain 
{
	return self;
}

- (unsigned)retainCount 
{
	return UINT_MAX;  // denotes an object that cannot be released
}

- (void)release 
{
	[self->callEvent release];
	
	//do nothing
}

- (id)autorelease 
{
	return self;
}

#pragma mark ---- VPEngine object methods ----

- (id) init
{
    self = [super init];
    
    return self;
}

- (void) dealloc
{
	[super dealloc];	
}


- (int) login:(NSString*)user password:(NSString*)password
{
	if(vp) {
#if !TARGET_IPHONE_SIMULATOR
		[[Camcorder sharedInstance] setVPSTACK:0];
#endif	
        vp->Logoff();
        
		delete vp;
	}
	
	vp = CreateVPSTACK(&vpmixingaudiofactory/*, 0, &vpvideofactory*/);
	vp->SetVideoDataFactory(&vpvideofactory);
	
	vp->SetNotifyRoutine(VPRoutineNotify, 0);
	vp->SetSyncNotifyRoutine(VPRoutineSyncNotify, 0);
	
	if(vp->Init()) {
		VP_LOG_MSG(VP_LOG_ERROR, @"Cannot initialize vphone stack");
		delete vp;
		vp = 0;
		return 0;
	}
	
	vp->SetSupportedBearersMask((1<<BC_VOICE) | (1<<BC_SMS) | (1<<BC_AUDIOVIDEO) | (1<<BC_CHAT) | (1<<BC_FILE));
	
	VP_LOG_MSG(VP_LOG_DEBUG, @"Logging on as %@", user);
	vp->Logon([user UTF8String], [password UTF8String]);
	
#if !TARGET_IPHONE_SIMULATOR
	[[Camcorder sharedInstance] setVPSTACK:vp];
#endif	
	
	return 0;
}

- (int) logoff
{
	if(vp) {
        vp->Logoff();
		delete vp;
		vp = nil;
	}
	return 0;
}


- (NSString*)vpError:(int)error {
	
	static NSString *errors[14] = {
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

- (VPCALL) makeCall:(NSString*)user bearer:(int)bc
{
    VPCALL call = 0;
    if (vp != NULL) 
    {
		int rc;
        rc = vp->Connect(&call, [user UTF8String], bc);
		
        
		if(!rc) {
			[[VPSound singleton] playRingBackTone];
			VP_LOG_MSG(VP_LOG_DEBUG, @"Connecting call %d", (int)call);
		} else {
			VP_LOG_MSG(VP_LOG_ERROR, @"Error: %@", [self vpError:rc]);
            return 0;
		}
	}
   
	return call;
}

- (unsigned) getCallBearer:(VPCALL)call
{
    if (!vp) return 0;

    return vp->GetCallBearer(call);
}

- (int)getCallStatus:(VPCALL)call
{
    if (!vp) return 0;
    
    return vp->GetCallStatus(call);
}

- (int) sendContactAdded:(NSString*)number
{
    VPCALL  call = vp->CreateCall();
	
	vp->SetCallText(call, "I have added you to my contacts");
	
	vp->SetSMSType(call, SMSTYPE_CONTACTADDED);
	
    int result = vp->Connect(call, [number cStringUsingEncoding:NSASCIIStringEncoding], BC_SMS);
	
    if (result != 0)
        vp->FreeCall(call);
	
	return result;
}


- (int) sendSMS:(NSString*)number text:(NSString *)text
{
    VPCALL  call = vp->CreateCall();
	
	vp->SetSMSType(call, SMSTYPE_NORMAL);
	vp->SetCallText(call, [text cStringUsingEncoding:NSUTF8StringEncoding]);
	NSLog(@"Send sms %@: %@", number, text);
	
    int result = vp->Connect(call, [number cStringUsingEncoding:NSUTF8StringEncoding], BC_SMS);
	
    if (result != 0)
        vp->FreeCall(call);
	
	return result;
}



- (int) sendContactAck:(NSString*)number
{
    VPCALL  call = vp->CreateCall();
	
	vp->SetCallText(call, "I have accepted your invitation and added you to my contacts");
	
	vp->SetSMSType(call, SMSTYPE_CONTACTADDED_ACK);
	
    int result = vp->Connect(call, [number cStringUsingEncoding:NSASCIIStringEncoding], BC_SMS);
	
    if (result != 0)
        vp->FreeCall(call);
	
	return result;
}

- (int) sendContactNack:(NSString*)number
{
    VPCALL  call = vp->CreateCall();
	
	vp->SetCallText(call, "I have not accepted your invitation");
	
	vp->SetSMSType(call, SMSTYPE_CONTACTADDED_NACK);
	
    int result = vp->Connect(call, [number cStringUsingEncoding:NSASCIIStringEncoding], BC_SMS);
	
    if (result != 0)
        vp->FreeCall(call);
	
	return result;
}



- (int) sendChatMsg:(VPCALL)call text:(NSString*)text
{
    int rc = vp->SendChat(call, [text cStringUsingEncoding:NSUTF8StringEncoding]);
    if (!rc) 
    {
        VP_LOG_MSG(VP_LOG_DEBUG, @"Send chat message %@ to %d", text, (int)call);
    }
    else 
    {
		VP_LOG_MSG(VP_LOG_ERROR, @"Error sending chat: %@", [self vpError:rc]);
    }
    
	return rc;
}


- (NSString*) getChatMessage:(VPCALL)call
{
    if (!vp) return nil;
    char s[300] = {'\0'};
    
    vp->GetChatText(call, s, 300);
    
    return [NSString stringWithCString:s encoding:NSUTF8StringEncoding];
    
}

- (NSString*) getFilePath:(VPCALL)call
{
    if (!vp) return nil;
    char s[MAX_PATH] = {'\0'};
	
	
    vp->GetCallFilePath(call, s);
    
    return [NSString stringWithCString:s encoding:NSUTF8StringEncoding];
}

- (int) sendFile:(VPCALL)call file:(NSString*)file
{
	if (vp)
		return vp->SendFile(call, [file cStringUsingEncoding:NSUTF8StringEncoding]);
	return 0;
}

- (int) acceptFile:(VPCALL)vpcall
{
	if (vp)
		vp->AcceptFile(vpcall, 1);
	return 0;
}

- (int) rejectFile:(VPCALL)vpcall
{
	if (vp)
		vp->AcceptFile(vpcall, 0);
	return 0;
}




- (int) answer:(VPCALL)vpcall
{
	if (vp)
		vp->AnswerCall(vpcall);
	return 0;
}

- (int) hangup:(VPCALL)vpcall
{
	if (vp != NULL) 
	{
		vp->Disconnect(vpcall, REASON_NORMAL);
       
        // Will be freed in the VPMSG_CALLENDED notification
        //vp->FreeCall(vpcall);
	} 
	
	return 0;
}

- (int) hold:(VPCALL)vpcall
{
	if (vp) 
	{
		vp->Hold(vpcall);
		
	} 
	
	return 0;
}

- (int) resume:(VPCALL)vpcall
{
	if (vp) 
	{
		vp->Resume(vpcall);
	} 
	
	return 0;
}

- (BOOL) isHold:(VPCALL)vpcall
{
	if (vp)
	{
		return vp->IsHeld(vpcall) == 1;
	}
	return NO;
}

- (BOOL) isMute
{
	return VPMIXINGAUDIO_GetMute() ? YES : NO;
}

- (void) mute:(BOOL)on
{
	
	VPMIXINGAUDIO_SetMute(on);
}

- (BOOL) isSpeaker
{
	return VPMIXINGAUDIO_GetSpeaker() ? YES : NO;
}

- (void) speaker:(BOOL)on
{
	
	VPMIXINGAUDIO_SetSpeaker(on);
}


- (NSString*) getRemoteCallName:(VPCALL)vpcall
{
	if (!vp) return nil;
	char name[MAXNAMELEN+1];
	vp->GetCallRemoteName(vpcall, name);
	
	return [NSString stringWithCString:name encoding:NSUTF8StringEncoding];
}

- (NSString*) getLogonName
{
	if (!vp) return nil;
	
	return [NSString stringWithCString:vp->LogonName() encoding:NSUTF8StringEncoding];
}

- (NSString*) getLogonNumber
{
	if (!vp) return nil;
	
	return [NSString stringWithCString:vp->LogonNumber() encoding:NSUTF8StringEncoding];
}

- (void)askVpnumber:(NSString*)name
{
    if (!vp) return;
    
   // if ([name isEqualToString:[[VPEngine sharedInstance] getLogonName]])
   //    return [[VPEngine sharedInstance] getLogonNumber];
    
    VPCALL  tmpcall = vp->CreateCall();
    int ret = vp->Connect(tmpcall, [name cStringUsingEncoding:NSASCIIStringEncoding], BC_NOTHING);
    if (ret != 0)
        vp->FreeCall(tmpcall);
}

- (void) askOnline:(NSArray*)numbers
{
    if (!vp) return;

	aol.notificationreqop = NOTIFICATIONREQ_ENABLE;                                                          
	aol.N = 0; 
	if (aol.nums) {
		free(aol.nums);
	}
	aol.nums = (AOLNUM *)malloc(sizeof(AOLNUM) * [numbers count]);                                                  
	for (NSString *number in numbers) 
	{                                                                                                        
		const char *num = [number cStringUsingEncoding: NSUTF8StringEncoding];//NSASCIIStringEncoding];
		aol.nums[aol.N].srvid = (num[0] - '0') * 100 + (num[1] - '0') * 10 + (num[2] - '0');                                                                                                                  
		aol.nums[aol.N].num = atoi(num + 3);  
	
		aol.nums[aol.N].online = 0;                                                              
		aol.N++;                                                                                 
	}                                                                                                        
	vp->AskOnline(&aol);               
    
}

- (int) userStatus
{
    if (!vp) return -1;
	
	return vp->GetUserStatus();
}

- (void) setUserStatus:(int)status
{
    if (!vp) return;
	
	NSLog(@"Set user status: %d", status);
	vp->SetUserStatus(status);
}

- (int) loadPhoto:(NSString*)number
{
	if (!vp) return -1;
	
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *file = [NSString stringWithFormat:@"%@/photos/profile.jpg", [paths objectAtIndex:0]];
	
    return vp->ServerTransfer(TCPFT_RXVPICT, [file cStringUsingEncoding:NSUTF8StringEncoding], true, [number cStringUsingEncoding:NSUTF8StringEncoding]);
	
}


- (int) loadPhoto:(NSString*)number lastUpdate:(NSString*)update
{
	if (!vp) return -1;
	
	if (([update length] == 0) || ([update isEqualToString:@"(null)"])) {
		return -1;
	}
	
	
	NSError *error;
	BOOL isDir;
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *file = [NSString stringWithFormat:@"%@/photos/%@.jpg", [paths objectAtIndex:0], update];
	
	NSFileManager *fileManager = [NSFileManager defaultManager];
	if (![fileManager fileExistsAtPath:[NSString stringWithFormat:@"%@/photos", [paths objectAtIndex:0]] isDirectory:&isDir]) 
	{
		if(![fileManager createDirectoryAtPath:[NSString stringWithFormat:@"%@/photos", [paths objectAtIndex:0]] withIntermediateDirectories:YES attributes:nil error:&error])
			NSLog(@"%@", error);
	}

    return vp->ServerTransfer(TCPFT_RXVPICT, [file cStringUsingEncoding:NSUTF8StringEncoding], true, [number cStringUsingEncoding:NSUTF8StringEncoding]);
	
}


- (int) updatePhoto:(NSString*)file
{
	if (!vp) return -1;
	
	NSLog(@"Update photo: %@", file);
	
    return vp->ServerTransfer(TCPFT_TXPICT, [file cStringUsingEncoding:NSUTF8StringEncoding], true, nil);// [[profile userName] cStringUsingEncoding:NSUTF8StringEncoding]);
	
}


- (int) postContacts:(VPContactList*)contacts
{
	NSError       **err;
	NSArray *path = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *file = [NSString stringWithFormat:@"%@/contacts.txt", [path objectAtIndex:0]];
	
    if(![[NSFileManager defaultManager] fileExistsAtPath:file])
    {
		[[NSFileManager defaultManager] createFileAtPath:file contents:nil attributes:nil];
    }
	
	NSString *data = @"";
	
	for (VPContact* contact in contacts.contactsOnline) {
		data = [data stringByAppendingFormat:@"%@\t", contact.userName]; // 1
		data = [data stringByAppendingFormat:@"%@\t", contact.number];					// 2
		data = [data stringByAppendingFormat:@"%@\t", contact.firstName];				// 3
		data = [data stringByAppendingFormat:@"%@\t", contact.lastName];				// 4
		data = [data stringByAppendingFormat:@"%@\t", contact.email];					// 5
		data = [data stringByAppendingFormat:@"%@\t", contact.country];				// 6	
		data = [data stringByAppendingFormat:@"%@\t", ([contact.state length] == 0) ? @" ": contact.state];					// 7
		data = [data stringByAppendingFormat:@"%@\t", ([contact.city length] == 0) ? @" ": contact.city];					// 8	
		data = [data stringByAppendingFormat:@"%@\t", ([contact.birthday length] == 0) ? @"00000000": contact.birthday];				// 9
		data = [data stringByAppendingFormat:@"%@\t", ([contact.gender length] == 0) ? @"M": contact.gender];					// 10
		data = [data stringByAppendingFormat:@"%@\t", ([contact.phoneHome length] == 0) ? @" " : contact.phoneHome];				// 11
		data = [data stringByAppendingFormat:@"%@\t", ([contact.phoneOffice length] == 0) ? @" " : contact.phoneOffice];			// 12
		data = [data stringByAppendingFormat:@"%@\t", ([contact.phoneMobile length] == 0) ? @" " : contact.phoneMobile];			// 13
		data = [data stringByAppendingFormat:@"%@\t", ([contact.photoUpdate length] == 0) ? @" " : contact.photoUpdate];			// 14
		data = [data stringByAppendingFormat:@"%d\r\n", 0];								// 15
	}
	
	for (VPContact* contact in contacts.contactsOffline) {
		data = [data stringByAppendingFormat:@"%@\t", contact.userName]; // 1
		data = [data stringByAppendingFormat:@"%@\t", contact.number];					// 2
		data = [data stringByAppendingFormat:@"%@\t", contact.firstName];				// 3
		data = [data stringByAppendingFormat:@"%@\t", contact.lastName];				// 4
		data = [data stringByAppendingFormat:@"%@\t", contact.email];					// 5
		data = [data stringByAppendingFormat:@"%@\t", contact.country];				// 6	
		data = [data stringByAppendingFormat:@"%@\t", ([contact.state length] == 0) ? @" ": contact.state];					// 7
		data = [data stringByAppendingFormat:@"%@\t", ([contact.city length] == 0) ? @" ": contact.city];					// 8	
		data = [data stringByAppendingFormat:@"%@\t", ([contact.birthday length] == 0) ? @"00000000": contact.birthday];				// 9
		data = [data stringByAppendingFormat:@"%@\t", ([contact.gender length] == 0) ? @"M": contact.gender];					// 10
		data = [data stringByAppendingFormat:@"%@\t", ([contact.phoneHome length] == 0) ? @" " : contact.phoneHome];				// 11
		data = [data stringByAppendingFormat:@"%@\t", ([contact.phoneOffice length] == 0) ? @" " : contact.phoneOffice];			// 12
		data = [data stringByAppendingFormat:@"%@\t", ([contact.phoneMobile length] == 0) ? @" " : contact.phoneMobile];			// 13
		data = [data stringByAppendingFormat:@"%@\t", ([contact.photoUpdate length] == 0) ? @" " : contact.photoUpdate];			// 14
		data = [data stringByAppendingFormat:@"%d\r\n", 0];								// 15
	}
	
	[data writeToFile:file atomically:NO encoding:NSUTF8StringEncoding error:err];
	
    return vp->ServerTransfer(TCPFT_TXCONTACTS, [file cStringUsingEncoding:NSUTF8StringEncoding], true, [[[VPProfile singleton] userName] cStringUsingEncoding:NSUTF8StringEncoding]);
	
}



- (int) postProfile:(VPProfile*)profile;
{
   // if (!vp) return -1;
	
	NSError       **err;
	NSArray *path = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *file = [NSString stringWithFormat:@"%@/profile.txt", [path objectAtIndex:0]];
	
    if(![[NSFileManager defaultManager] fileExistsAtPath:file])
    {
        [[NSFileManager defaultManager] createFileAtPath:file contents:nil attributes:nil];
    }
	
/*	
	@property (nonatomic, copy) NSString *userName;
	@property (nonatomic, copy) NSString *password;
	@property (nonatomic, copy) NSString *number;
	@property (nonatomic, copy) NSString *firstName;
	@property (nonatomic, copy) NSString *lastName;
	@property (nonatomic, copy) NSString *email;
	@property (nonatomic, copy) NSString *country;
	@property (nonatomic, copy) NSString *countryName;
	@property (nonatomic, copy) NSString *countryCode;
	@property (nonatomic, copy) NSString *state;
	@property (nonatomic, copy) NSString *birthday;
	@property (nonatomic, copy) NSString *gender;
	@property (nonatomic, copy) NSString *phoneHome;
	@property (nonatomic, copy) NSString *phoneOffice;
	@property (nonatomic, copy) NSString *phoneMobile;
	@property (nonatomic, copy)	NSString *phoneVirtual;
	@property (nonatomic, copy)	NSString *phoneForward;
	@property (nonatomic, copy) NSString *photoUpdate;
	@property (nonatomic, copy)	NSString *statusMessage;
	@property (nonatomic, assign) NSUInteger flags;
	@property (nonatomic, assign) NSUInteger status;
*/
	
	NSString *data = [NSString stringWithFormat:@"%@\t", profile.userName]; // 1
	data = [data stringByAppendingFormat:@"%@\t", profile.number];					// 2
	data = [data stringByAppendingFormat:@"%@\t", profile.firstName];				// 3
	data = [data stringByAppendingFormat:@"%@\t", profile.lastName];				// 4
	data = [data stringByAppendingFormat:@"%@\t", profile.email];					// 5
	data = [data stringByAppendingFormat:@"%@\t", profile.country];				// 6	
	data = [data stringByAppendingFormat:@"%@\t", ([profile.state length] == 0) ? @" ": profile.state];					// 7
	data = [data stringByAppendingFormat:@"%@\t", ([profile.city length] == 0) ? @" ": profile.city];					// 8	
	data = [data stringByAppendingFormat:@"%@\t", ([profile.birthday length] == 0) ? @"00000000": profile.birthday];				// 9
	data = [data stringByAppendingFormat:@"%@\t", ([profile.gender length] == 0) ? @"M": profile.gender];					// 10
	data = [data stringByAppendingFormat:@"%@\t", ([profile.phoneHome length] == 0) ? @" " : profile.phoneHome];				// 11
	data = [data stringByAppendingFormat:@"%@\t", ([profile.phoneOffice length] == 0) ? @" " : profile.phoneOffice];			// 12
	data = [data stringByAppendingFormat:@"%@\t", ([profile.phoneMobile length] == 0) ? @" " : profile.phoneMobile];			// 13
	data = [data stringByAppendingFormat:@"%@\t", ([profile.photoUpdate length] == 0) ? @" " : profile.photoUpdate];			// 14
	data = [data stringByAppendingFormat:@"%d\r\n", 0];								// 15
	
	
	/*
	 Vname: 20 characters max
	 Vnumber: 10 digits
	 First Name: 30 characters max
	 Last Name: 30 characters max
	 E-Mail address: 30 characters max
	 Country: 2 letters (international country code) or empty
	 State: 30 characters max
	 City: 30 characters max
	 Birthday: in the form YYYYMMDD or empty
	 Gender: M, F or emtpy (empty string)
	 Home Phone: 20 characters max
	 Office Phone: 20 characters max
	 Mobile Phone: 20 characters max
	 Last picture upload date and time: in the form YYYYMMDDhhmmss or empty, UTC
	 Flags: unsigned 32 bits number formatted in decimal form
	
	Flags is the sum of these:
	1 Blocked (I will refuse all the calls from this user)
	2 Hidden (this user is not searchable in white pages, apart by vname/vnumber)
	4 Symmetric (we are present in this user’s contacts, so we can obtain this user’s status)
	 */
	
	

	[data writeToFile:file atomically:NO encoding:NSUTF8StringEncoding error:err];

    return vp->ServerTransfer(TCPFT_TXCONTACTS, [file cStringUsingEncoding:NSUTF8StringEncoding], true, [[profile userName] cStringUsingEncoding:NSUTF8StringEncoding]);
	
}





- (int) downloadAddressBook
{
    if (!vp) return -1;
	
	NSArray *path = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *file = [NSString stringWithFormat:@"%@/contacts.txt", [path objectAtIndex:0]];

	VPProfile *profile = [VPProfile singleton];
	
    return vp->ServerTransfer(TCPFT_RXCONTACTS, [file cStringUsingEncoding:NSUTF8StringEncoding], true, [[profile userName] cStringUsingEncoding:NSUTF8StringEncoding]);

}


- (int) addToConference:(VPCALL)call
{
    if (!vp) return -1;
    
    return vp->AddToConference(call);
}

- (int) conferenceCalls:(VPCALL*)calls:(int)count:(BOOL)detach
{
    if (!vp) return -1;

    int ret = vp->ConferenceCalls(calls, count, detach);

    
    return ret;
}

- (int) stopConference
{
        
   if (!vp) return -1;
   
   int ret = vp->StopConference();
   
   return ret;
    
}


- (int)userSearch:(NSString*)search inCountry:(NSString*)country
{
	if (!vp) return -1;

	int result = vp->UserSearch([search cStringUsingEncoding:NSUTF8StringEncoding], nil, USERSEARCH_VNAME | USERSEARCH_VNUMBER | USERSEARCH_FIRSTNAME | USERSEARCH_LASTNAME | USERSEARCH_EMAIL , 55); 
	
	NSLog(@"Search string for vp->UserSearch: %@", search);
	
//	int result = vp->UserSearch([search UTF8String], nil,//[country UTF8String], 
//								USERSEARCH_VNAME | USERSEARCH_VNUMBER | USERSEARCH_FIRSTNAME | USERSEARCH_LASTNAME | USERSEARCH_EMAIL | USERSEARCH_PHONENUMBER , 55); 
	return result;
}


- (void) smsNotification:(NSUInteger)smsType number:(NSString*)number name:(NSString*)name message:(NSString*)message
{
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    
    NSMutableDictionary *userInfo = [[[NSMutableDictionary alloc] initWithCapacity:3] autorelease];
	[userInfo setObject:[NSNumber numberWithInt:smsType] forKey:@"type"];
	[userInfo setObject:[NSString stringWithString:number] forKey:@"number"];
	[userInfo setObject:[NSString stringWithString:name] forKey:@"name"];
	[userInfo setObject:[NSString stringWithString:message] forKey:@"message"];
	
    // post synchronous (on main thread)
	NSNotification *note = [NSNotification notificationWithName:OnSmsNotification object:nil userInfo:userInfo];
	[[NSNotificationCenter defaultCenter] performSelectorOnMainThread:@selector(postNotification:) withObject:note 
														waitUntilDone:NO];
    
    
	
    [pool release];
    
}

- (void) searchNotification:(NSString*)result
{
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    
    NSMutableDictionary *userInfo = [[[NSMutableDictionary alloc] initWithCapacity:1] autorelease];
	[userInfo setObject:[NSString stringWithString:result] forKey:@"result"];
	
    // post synchronous (on main thread)
	NSNotification *note = [NSNotification notificationWithName:OnSearchNotification object:nil userInfo:userInfo];
	[[NSNotificationCenter defaultCenter] performSelectorOnMainThread:@selector(postNotification:) withObject:note 
														waitUntilDone:NO];

    [pool release];
}




#pragma mark ---- VPEngine notification support ----

- (NSString*)reasonCodeToString:(int)reasonCode {
	
	static NSString* reasons[36] = {
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

- (int)_syncNotify:(void *)uparam message:(unsigned)msg parameter1:(unsigned)param1 parameter2:(unsigned)param2 {
	
	int rc = 0;
	switch(msg)
	{
		case VPMSG_USERSEARCH:
		{
			[self searchNotification:[NSString stringWithCString:(const char*)param2 encoding:NSUTF8StringEncoding]]; 
			//			NSLog(@"Search result (%d): %@", [str length],str);
		}
			break;
		case VPMSG_NOTIFYLOGON:	
			VP_LOG_MSG(VP_LOG_DEBUG, @"Num='%s' logged on", (const char *)param1);
			break;
		case VPMSG_NEWVOICEMSG:
			VP_LOG_MSG(VP_LOG_DEBUG, @"New voice message from num='%s', name='%s'", 
					   (const char *)param1, (const char *)param2);
			break;
		case VPMSG_ABUPDATE:
        {
//			VP_LOG_MSG(VP_LOG_DEBUG, @"User '%s' has number '%s'", (const char *)param1, (const char *)param2);

			break;
        }
	}
	return rc;
}

- (NSString *)pathForTemporaryFileWithPrefix:(NSString *)prefix
{
    NSString *  result;
    CFUUIDRef   uuid;
    CFStringRef uuidStr;
    
    uuid = CFUUIDCreate(NULL);
    assert(uuid != NULL);
    
    uuidStr = CFUUIDCreateString(NULL, uuid);
    assert(uuidStr != NULL);
    
    result = [NSTemporaryDirectory() stringByAppendingPathComponent:[NSString stringWithFormat:@"%@-%@", prefix, uuidStr]];
    assert(result != nil);
    
    CFRelease(uuidStr);
    CFRelease(uuid);
    
    return result;
}




- (void)_notify:(void *)uparam message:(unsigned)msg call:(unsigned)pcall parameter:(unsigned)param {
	
	char s[300];
	VPCALL vpcall = (VPCALL)pcall;
	
	switch(msg)
	{
		case VPMSG_USERSEARCH:
		{
			[self searchNotification:[NSString stringWithCString:(const char*)param encoding:NSUTF8StringEncoding]]; 
//			NSLog(@"Search result (%d): %@", [str length],str);
		}
			break;
		case VPMSG_SERVERSTATUS:
		{
			VP_LOG_MSG(VP_LOG_DEBUG, 
							@"Server status: %@ (%x)", [self reasonCodeToString:pcall], param);
			
			if (pcall == REASON_LOGGEDON) {
				NSAutoreleasePool *pool;
				pool = [[NSAutoreleasePool alloc] init];
				/* use the string */
				VPProfile* profile = [[[VPProfile alloc] initFromDatabase] autorelease];
				if ((profile.lastLogin == 0) && ([profile.userName length] != 0)) {
					[[VPEngine sharedInstance] postProfile:profile];
				}
				[VPContactList sharedInstance];
				[[VPEngine sharedInstance] downloadAddressBook]; // Request contact list
				
				[pool release];
			}
			
//			if (vp->IsLoggedOn())
//				[loginButton performSelectorOnMainThread:@selector(removeFromSuperview) withObject:nil waitUntilDone:NO];

		}
			break;
		case VPMSG_INVALIDADDR:
			VP_LOG_MSG(VP_LOG_DEBUG, @"User not found for call %d", pcall);
			break;
		case VPMSG_CALLFORWARDING:
		{
			int op;
			vp->GetCallForwardingStatus(&op, s);
			if(op == 1) {
				VP_LOG_MSG(VP_LOG_DEBUG, @"Call forwarding disabled");
			} else {
				VP_LOG_MSG(VP_LOG_DEBUG, @"Call forwarding status: %s to %s", op == 3 ? "Enabled on offline" : "Enabled", s);
			}
			break;
		}
		case VPMSG_SERVERTRANSFERFINISHED:
			VP_LOG_MSG(VP_LOG_DEBUG, @"Server transfer op %d finished with error %d", pcall, param);
			break;
		case VPMSG_SERVERNOTRESPONDING:
			VP_LOG_MSG(VP_LOG_DEBUG, @"Server not responding for call %d", pcall);
			break;
		case VPMSG_RESOLVED:
			VP_LOG_MSG(VP_LOG_DEBUG, @"Address for call %d found", pcall);
			break;
		case VPMSG_USERALERTED:{
			VP_LOG_MSG(VP_LOG_DEBUG, @"User alerted for call %d", pcall);
			int bc = vp->GetCallBearer(vpcall);
			if ((bc == BC_VOICE) || (bc == BC_VIDEO) || (bc == BC_AUDIOVIDEO))
			{
				[self->callEvent release], self->callEvent = nil;
				self->callEvent = [[HistoryAVCallEvent alloc] initAudioCallEvent:[[VPEngine sharedInstance] getRemoteCallName:vpcall]];
				self->callEvent.status = HistoryEventStatus_Missed;
			}
		}
			break;
		case VPMSG_CALLREFUSED: {
			[[VPSound singleton] stopRingBackTone];
			VP_LOG_MSG(VP_LOG_DEBUG, @"The user has not accepted the call %d, reason=%@", 
					   pcall, [self reasonCodeToString:param]);
			vp->GetCallText(vpcall, s);
			if(*s) {
				VP_LOG_MSG(VP_LOG_DEBUG, @"The user left a message: %s", s);
			}
			break;
		}
		case VPMSG_CALLACCEPTED: {
			[[VPSound singleton] stopRingBackTone];
			VP_LOG_MSG(VP_LOG_DEBUG, @"User accepted call %d", pcall);
			int bc = vp->GetCallBearer(vpcall);
			if ((bc == BC_VOICE) || (bc == BC_VIDEO) || (bc == BC_AUDIOVIDEO))
			{
				if(self->callEvent){
					self->callEvent.status = HistoryEventStatus_Outgoing;
				}
			}
		}
			break;
		case VPMSG_CALLACCEPTEDBYUSER:
			VP_LOG_MSG(VP_LOG_DEBUG, @"User explicitly accepted call %d", pcall);
			break;
		case VPMSG_CONNECTFINISHED:
			[[VPSound singleton] stopRingBackTone];
			VP_LOG_MSG(VP_LOG_DEBUG, @"Connection procedure finished for call %d", pcall);
			//if (vp->GetCallBearer(vpcall) == BC_FILE){
			//	NSString* fileName = [callFiles objectForKey:[NSNumber numberWithInt:(int)vpcall]];
			//	vp->SendFile(vpcall, [fileName cStringUsingEncoding:NSUTF8StringEncoding]);
			//}
			
			break;
		case VPMSG_CALLESTABLISHED: {
			[[VPSound singleton] stopRingBackTone];
			int bc = vp->GetCallBearer(vpcall);
			if (bc == BC_CHAT){
//				chatConnected = YES;
//				chatCall = vpcall;
			}
			if ((bc == BC_VOICE) || (bc == BC_VIDEO) || (bc == BC_AUDIOVIDEO))
			{
				if(self->callEvent){
					self->callEvent.status = HistoryEventStatus_Incoming;
				}
			}

			
			VP_LOG_MSG(VP_LOG_DEBUG, @"Call %d established", pcall);
		}
			break;
		case VPMSG_CALLSETUPFAILED:
			VP_LOG_MSG(VP_LOG_DEBUG, @"Call setup failed for call %d", pcall);
			break;
		case VPMSG_CONNECTTIMEOUT:
			VP_LOG_MSG(VP_LOG_DEBUG, @"Remote device not responding for call %d", pcall);
			break;
		case VPMSG_CALLDISCONNECTED:
			VP_LOG_MSG(VP_LOG_DEBUG, @"The remote user has terminated the call %d, reason %@", 
							pcall, [self reasonCodeToString:param]);
			break;
		case VPMSG_CALLENDED: {
			[[VPSound singleton] stopRingBackTone];
			VP_LOG_MSG(VP_LOG_DEBUG, @"Call %d ended, reason %@", pcall, [self reasonCodeToString:param]);
			
			if(self->callEvent){
//				if(self->dateSeconds>0){
					self->callEvent.end = [[NSDate date] timeIntervalSince1970];
//				}
				[[VPHistory singleton] addEvent:self->callEvent];
				[self->callEvent release], self->callEvent = nil;
			}
			
            vp->FreeCall(vpcall);
			break;
		}
		case VPMSG_NEWCALL:
			if(vp->GetCallBearer(vpcall) == BC_SMS) {
				NSAutoreleasePool *pool;
				pool = [[NSAutoreleasePool alloc] init];

				char text[MAXTEXTLEN+1];
				char number[MAXNAMELEN+1];
				char name[MAXNAMELEN+1];
				
				unsigned n, tm, bc;
				static const char *bearernames[] = {"voice", "video", "chat", "sms", "file transfer", "video", "video message"};
				
				vp->GetCallRemoteName(vpcall, name);
				if(!vp->GetMissedCallsData(vpcall, &n, &tm, &bc) && n)
				{
					sprintf(text, "You have been called %d times by %s. The last call was a %s call and was made on ",
							n, name, bc > 6 ? "unknown" : bearernames[bc]);
					utimetotext(tm, text + strlen(text));
					VP_LOG_MSG(VP_LOG_DEBUG, @"%s.", text);
				} else {
					vp->GetCallRemoteName(vpcall, name);
					vp->GetCallRemoteNumber(vpcall, number);
					vp->GetCallText(vpcall, text);
					unsigned typeSMS = 0;
					vp->GetSMSType(vpcall, &typeSMS);
					switch (typeSMS) {
						case SMSTYPE_NORMAL:
							VP_LOG_MSG(VP_LOG_DEBUG, @"SMSTYPE_NORMAL from %s: %s", name, text);
							break;
						case SMSTYPE_MISSEDCALLS:
							VP_LOG_MSG(VP_LOG_DEBUG, @"SMSTYPE_MISSEDCALLS from %s: %s", name, text);
							break;
						case SMSTYPE_DELIVERYREPORT:
							VP_LOG_MSG(VP_LOG_DEBUG, @"SMSTYPE_DELIVERYREPORT from %s: %s", name, text);
							break;
						case SMSTYPE_CONTACTADDED:
							VP_LOG_MSG(VP_LOG_DEBUG, @"SMSTYPE_CONTACTADDED from %s: %s", name, text);
							break;
						case SMSTYPE_CONTACTADDED_ACK:
							VP_LOG_MSG(VP_LOG_DEBUG, @"SMSTYPE_CONTACTADDED_ACK from %s: %s", name, text);
							break;
						case SMSTYPE_CONTACTADDED_NACK:
							VP_LOG_MSG(VP_LOG_DEBUG, @"SMSTYPE_CONTACTADDED_NACK from %s: %s", name, text);
							break;
						case SMSTYPE_NEWFILE:
							VP_LOG_MSG(VP_LOG_DEBUG, @"SMSTYPE_NEWFILE from %s: %s", name, text);
							break;
						case SMSTYPE_FILEDELIVERED:
							VP_LOG_MSG(VP_LOG_DEBUG, @"SMSTYPE_FILEDELIVERED from %s: %s", name, text);
							break;
						default:	
							VP_LOG_MSG(VP_LOG_DEBUG, @"SMS from %s: %s.", name, text);
							break;
					}
					
					
					[self smsNotification:typeSMS 
								   number:[NSString stringWithCString:number encoding:NSUTF8StringEncoding]
								   name:[NSString stringWithCString:name encoding:NSUTF8StringEncoding]
									message:[NSString stringWithCString:text encoding:NSUTF8StringEncoding]];
				}
				vp->FreeCall(vpcall);
				[pool release];
			} else {
				char name[MAXNAMELEN+1];
				
				vp->GetCallRemoteName(vpcall, name);
				if(vp->GetCallStatus(vpcall) == VPCS_CONNECTED) {
					VP_LOG_MSG(VP_LOG_DEBUG, @"Automatic new call %d from %s", pcall, name);
				} else {
					VP_LOG_MSG(VP_LOG_DEBUG, @"Incoming new call %d from %s", pcall, name);
				}
				int bc = vp->GetCallBearer(vpcall);
				if (bc == BC_FILE){

				}
				else if ((bc == BC_VOICE) || (bc == BC_VIDEO) || (bc == BC_AUDIOVIDEO)) {
					// create call event
					[self->callEvent release], self->callEvent = nil;
					self->callEvent = [[HistoryAVCallEvent alloc] initAudioCallEvent:[[VPEngine sharedInstance] getRemoteCallName:vpcall]];
					self->callEvent.status = HistoryEventStatus_Missed;
				}
			}
			break;
		case VPMSG_KEYPAD:
			VP_LOG_MSG(VP_LOG_DEBUG, @"Keypad '%c' (0x%X) received from call %d", param, param, pcall);
			break;
		case VPMSG_REMOTELYCONFERENCED:
			VP_LOG_MSG(VP_LOG_DEBUG, @"Call %d remotely conferenced", pcall);
			break;
		case VPMSG_REMOTELYUNCONFERENCED:
			VP_LOG_MSG(VP_LOG_DEBUG, @"Call %d remotely taken out from conference", pcall);
			break;
		case VPMSG_REMOTELYHELD:
			VP_LOG_MSG(VP_LOG_DEBUG, @"Call %d remotely held", pcall);
			break;
		case VPMSG_REMOTELYRESUMED:
			VP_LOG_MSG(VP_LOG_DEBUG, @"Call %d remotely resumed", pcall);
			break;
		case VPMSG_CONFERENCEESTABLISHED:
			VP_LOG_MSG(VP_LOG_DEBUG, @"Call %d added to conference (we are master), detach=%d", pcall, param);
			break;
		case VPMSG_CONFERENCEFAILED:
			VP_LOG_MSG(VP_LOG_DEBUG, @"Call %d could not be added to conference (we are master), detach=%d", pcall, param);
			break;
		case VPMSG_CONFERENCEREQ:
			VP_LOG_MSG(VP_LOG_DEBUG, @"Call %d is asking us to be added to a conference", pcall);
			vp->AcceptConference(vpcall, true);
			break;
		case VPMSG_CONFERENCECALLESTABLISHED:
			VP_LOG_MSG(VP_LOG_DEBUG, @"Call %d in conference (we are slave)", pcall);
			break;
		case VPMSG_CONFERENCEEND:
			VP_LOG_MSG(VP_LOG_DEBUG, @"Call %d (master) has terminated the conference (we are slave)", pcall);
			break;
		case VPMSG_CALLTRANSFERRED:
			VP_LOG_MSG(VP_LOG_DEBUG, @"Call %d has been transferred to the new call %d", pcall, param);
			break;
		case VPMSG_CHAT:
			vp->GetChatText(vpcall, s, 300);
			VP_LOG_MSG(VP_LOG_DEBUG, @"Chat message: %s", s);
			break;
		case VPMSG_CHATACK:
			vp->GetChatText(vpcall, s, 300);
			VP_LOG_MSG(VP_LOG_DEBUG, @"Chat message sent: %s", s);
			break;
		case VPMSG_SENDFILEACK_REFUSED:
			VP_LOG_MSG(VP_LOG_DEBUG, @"File transfer refused (call=%d)", pcall);
			break;
		case VPMSG_SENDFILEACK_USERALERTED:
			VP_LOG_MSG(VP_LOG_DEBUG, @"File transfer user alerted  (call=%d)", pcall);
			break;
		case VPMSG_SENDFILEACK_ACCEPTED:
			VP_LOG_MSG(VP_LOG_DEBUG, @"File transfer accepted (call=%d)", pcall);
			break;
		case VPMSG_FTPROGRESS:
		{
			unsigned cur, tot;
			vp->GetFileProgress(vpcall, &cur, &tot);
			VP_LOG_MSG(VP_LOG_DEBUG, @"Progress %u/%u", cur, tot);
			break;
		}
			break;
		case VPMSG_FTTIMEOUT:
			VP_LOG_MSG(VP_LOG_DEBUG, @"File transfer timeout (call=%d)", pcall);
			break;
		case VPMSG_FTTXCOMPLETE:
		case VPMSG_FTRXCOMPLETE:
			VP_LOG_MSG(VP_LOG_DEBUG, @"File transfer complete (call=%d)", pcall);
			break;
		case VPMSG_SENDFILEREQ:
		{
			NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

//			NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
//			NSString *documentsDirectory = [paths objectAtIndex:0];
			
//			- (NSString *)pathForTemporaryFileWithPrefix:(NSString *)prefix;
			
			NSString *file = [NSString stringWithFormat:@"%@/%@", [self pathForTemporaryFileWithPrefix:@""], [self getFilePath:vpcall]];
			VP_LOG_MSG(VP_LOG_DEBUG, @"File transfer request (call=%d, dir=%@)", pcall, file);
			vp->SetCallFilePath(vpcall, [file cStringUsingEncoding:NSUTF8StringEncoding]);
			vp->AcceptFile(vpcall, 1);

			[pool release];
		}
			break;
		case VPMSG_SENDFILESUCCESS:
			VP_LOG_MSG(VP_LOG_DEBUG, @"File transfer finished successfully (call=%d)", pcall);
			//[callFiles removeObjectForKey:[NSNumber numberWithInt:(int)vpcall]];
			break;
		case VPMSG_SENDFILEFAILED:
			VP_LOG_MSG(VP_LOG_DEBUG, @"File transfer finished failed (call=%d)", pcall);
			//[callFiles removeObjectForKey:[NSNumber numberWithInt:(int)vpcall]];
			break;
		case VPMSG_QUERYONLINEACK:
			{
				NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
				if(!pcall)	// Query online ack finished
				{
					int i;
					
					for(i = 0; i < aol.N; i++)
					{
						NSLog(@"Ack %03d%07d status %d", (int)aol.nums[i].srvid, (int)aol.nums[i].num, (int) aol.nums[i].online);
						VPContact *contact = [[VPContactList sharedInstance] contactByNumber:[NSString stringWithFormat:@"%03d%07d", (int)aol.nums[i].srvid, (int)aol.nums[i].num]];
						if (contact)
						{
							contact.status = aol.nums[i].online;
							if (contact.flags != 4) {
								contact.status = 4;	
							}
						}
					}
					free(aol.nums);
					aol.nums = 0;
				}
				[pool release];
			}
			break;
		case VPMSG_CREDIT:
			VP_LOG_MSG(VP_LOG_DEBUG, @"Credit is %.2f $", pcall / 10000.0);
			if(param != (unsigned)-1)
				VP_LOG_MSG(VP_LOG_DEBUG, @"SMS credit is %d units", param);
			break;
		case VPMSG_VIDEOSTARTOK:
			VP_LOG_MSG(VP_LOG_DEBUG, @"Video started (call %d)", param);
			break;
		case VPMSG_VIDEOSTARTERROR:
			VP_LOG_MSG(VP_LOG_DEBUG, @"Video could not start (call %d)", param);
			break;
		default:
			VP_LOG_MSG(VP_LOG_DEBUG, @"Message %u, pcall %u, param %u", msg, pcall, param);
	}
}


- (void) _postNotification: (unsigned)msg param1:(unsigned)param1 param2:(unsigned)param2
{
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    
    NSMutableDictionary *userInfo = [[[NSMutableDictionary alloc] initWithCapacity:3] autorelease];
	[userInfo setObject:[NSNumber numberWithUnsignedLong:msg] forKey:@"MSG"];
	[userInfo setObject:[NSNumber numberWithUnsignedLong:param1] forKey:@"PARAM1"];
	[userInfo setObject:[NSNumber numberWithUnsignedLong:param2] forKey:@"PARAM2"];

    // post notification asynchronous 
//    [[NSNotificationCenter defaultCenter] postNotificationName:VPEngineAsyncNotification
//                                         object:nil userInfo:userInfo];
    

    // post synchronous (on main thread)
	NSNotification *note = [NSNotification notificationWithName:VPEngineNotification object:nil userInfo:userInfo];
	[[NSNotificationCenter defaultCenter] performSelectorOnMainThread:@selector(postNotification:) withObject:note 
                                         waitUntilDone:NO];
    
    

    [pool release];
    
}



void VPRoutineNotify(void *param, unsigned msg, unsigned param1, unsigned param2) 
{
//     NSLog(@"VPRoutineNotify: 0x%08x", pthread_self());
	// Post notification(s)
    [[VPEngine sharedInstance] _postNotification:msg param1:param1 param2:param2];
    
    // Process
	[[VPEngine sharedInstance] _notify:0 message:msg call:param1 parameter:param2];
	
}

int VPRoutineSyncNotify(void *uparam, unsigned msg, unsigned param1, unsigned param2) 
{
//    NSLog(@"VPRoutineSyncNotify: 0x%08x", pthread_self());
    // Post notification(s)
    [[VPEngine sharedInstance] _postNotification:msg param1:param1 param2:param2];
    
	return [[VPEngine sharedInstance] _syncNotify:uparam message:msg parameter1:param1 parameter2:param2]; 
}

+ (NSString *)GetUUID
{
	CFUUIDRef theUUID = CFUUIDCreate(NULL);
	CFStringRef string = CFUUIDCreateString(NULL, theUUID);
	CFRelease(theUUID);
	return [(NSString *)string autorelease];
}

+ (NSString *)generateFileName:(NSString*)fileExtension
{
	
	NSDate* date = [NSDate date];
	NSDateFormatter* formatter = [[NSDateFormatter alloc] init];
	
	// The following line formats the date in "7/20/10" format.
	[formatter setDateStyle:NSDateFormatterShortStyle];
	
	// This will produce a time that looks like "12:15:36".
	[formatter setTimeStyle:NSDateFormatterMediumStyle];
	[formatter setAMSymbol:@"AM"];
	[formatter setPMSymbol:@"PM"];
	
	NSString *theTimeDateOne = [[formatter stringFromDate:date] stringByReplacingOccurrencesOfString:@"/" withString:@"-"];
	NSString *theTimeDateTwo = [theTimeDateOne stringByReplacingOccurrencesOfString:@":" withString:@"."];
	NSString *theTimeDateThree = [theTimeDateTwo stringByReplacingOccurrencesOfString:@" AM" withString:@"AM"];
	NSString *theTimeDateFour = [theTimeDateThree stringByReplacingOccurrencesOfString:@" PM" withString:@"PM"];
	NSString *theTimeDateFive = [theTimeDateFour stringByReplacingOccurrencesOfString:@" " withString:@"_"];
	NSString *theTimeDate = [NSString stringWithFormat:@"%@", theTimeDateFive];
	//NSString *fileNameExt = [theTimeDate stringByAppendingPathExtension:fileExtension];//], fileExtension];
	
//stringByAppendingPathExtension:
//	
	return [NSString stringWithString:[theTimeDate stringByAppendingPathExtension:fileExtension]];
	// The file name produced would be in the format "7-20-10_12.15.36PM.png".
	// Uncomment the following line to see the file name in the Debugger Console. (Command+Shift+R)
	//NSLog(@"Image File Name: %@", theFileNameExt);
	
							 
	// This of course returns the file name we created.
//	return [NSString stringWithFormat:@"%@", fileNameExt];
}




@end

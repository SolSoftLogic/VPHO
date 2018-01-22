//
//  VPContactList.m
//  Vphonet

#import "VPEngine.h"
#import "VPContactList.h"
#import "VPDatabase.h"

static VPContactList* __vpContactList = nil;



//#define _INSERT_DEBUG_CONTACTS 1

@implementation VPContact

@synthesize regCode;
@synthesize userName;
@synthesize number;
@synthesize firstName;
@synthesize lastName;
@synthesize email;
@synthesize country;
@synthesize state;
@synthesize city;
@synthesize birthday;
@synthesize gender;
@synthesize phoneHome;
@synthesize phoneOffice;
@synthesize phoneMobile;
@synthesize photoUpdate;			
@synthesize flags;

@synthesize status;
@synthesize online;
//@synthesize countryName;
//@synthesize countryCode;

//@synthesize cid, username, vpnumber, sdial, firstName, lastName,
//          country, state, homepage, email, mobile, office, status;	

- (id) init
{
    self = [self initWithId:0];
    
    return self;
}

- (id) initWithId:(unsigned)contactId;
{
    self = [super init];
    if (self)
    {
		userName	= [[NSString alloc] initWithString:@""];
		number		= [[NSString alloc] initWithString:@""];
		firstName	= [[NSString alloc] initWithString:@""];
		lastName	= [[NSString alloc] initWithString:@""];
		email		= [[NSString alloc] initWithString:@""];
		country		= [[NSString alloc] initWithString:@""];
		state		= [[NSString alloc] initWithString:@""];
		city		= [[NSString alloc] initWithString:@""];
		birthday	= [[NSString alloc] initWithString:@""];
		gender		= [[NSString alloc] initWithString:@""];
		phoneHome	= [[NSString alloc] initWithString:@""];
		phoneOffice = [[NSString alloc] initWithString:@""];
		phoneMobile = [[NSString alloc] initWithString:@""];
		photoUpdate = [[NSString alloc] initWithString:@""];
		flags = 0;
		
		status = AOL_OFFLINE;
    }
    
    return self;
}

- (void) dealloc
{   
	[userName  release];
	[number  release];
	[firstName  release];
	[lastName  release];
	[email  release];
	[country  release];
	[state  release];
	[city  release];
	[birthday  release];
	[gender  release];
	[phoneHome  release];
	[phoneOffice  release];
	[phoneMobile  release];
	[photoUpdate  release];

	/*	
    [username  release];
    [vpnumber  release];
    [sdial     release];
    [firstName release];
    [lastName  release];
    [country   release];	
    [state     release];	
    [homepage  release];
    [email     release];	
    [mobile    release];	
    [office    release];
*/    
    [super dealloc];
}

- (NSString*) photoImageName
{
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *file = [NSString stringWithFormat:@"%@/photos/%@.jpg",[paths objectAtIndex:0], photoUpdate];
	NSFileManager *fm = [NSFileManager defaultManager]; 
	
	if ([fm fileExistsAtPath:file]){
		return file;
	}	
	
	return nil;
}

- (NSString*) statusImageName
{
	switch (status) {
		case AOL_ONLINE:
		case AOL_WEBCAMPRESENT:
			return @"status_green.png";
			break;
		case AOL_OFFLINE:	
			return @"status_red.png";
			break;
		case AOL_LIMITED:
		case AOL_LIMITED_WEBCAMPRESENT:
			return @"status_yellow.png";
			break;
		default:
			break;
	}
	
	return @"status_unknown.png";
	
}

- (NSString*) statusName
{
	switch (status) {
		case AOL_ONLINE:
		case AOL_WEBCAMPRESENT:
			return NSLocalizedString(@"Online", @"");
			break;
		case AOL_OFFLINE:	
			return NSLocalizedString(@"Offline", @"");
			break;
		case AOL_LIMITED:
		case AOL_LIMITED_WEBCAMPRESENT:
			return NSLocalizedString(@"Limited", @"");
			break;
		default:
			break;
	}
	
	return NSLocalizedString(@"Unknown", @"");
}


@end;

@implementation VPContactList

NSString *OnContactListCompleteNotification = @"OnContactListCompleteNotification";


@synthesize contactsOnline;
@synthesize contactsOffline;
//static VPContactList *vpContactListSharedInstance = nil;

#pragma mark ---- VPContactList singleton object methods ----

+ (VPContactList*)sharedInstance 
{
	@synchronized(__vpContactList) 
	{
		if(__vpContactList == nil)
		{
			__vpContactList = [[VPContactList alloc] init];
		}
	}
	
	return __vpContactList;
}

/*
+ (id)allocWithZone:(NSZone *)zone 
{
	@synchronized(self) 
	{
		if (vpContactListSharedInstance == nil) 
		{
			vpContactListSharedInstance= [super allocWithZone:zone];
			return vpContactListSharedInstance;  // assignment and return on first allocation
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
	//do nothing
}
 
- (id)autorelease 
{
	return self;
}
*/
#pragma mark ---- VPContactList object methods ----

- (id) init
{
    self = [ super init ];
    
    contactsOnline = [[NSMutableArray alloc] init];
    contactsOffline = [[NSMutableArray alloc] init];
    
    [[NSNotificationCenter defaultCenter] addObserver:self 
                                          selector:@selector(vpStackNotification:) 
                                          name:VPEngineNotification
                                          object:nil];
    
    return self;
}

- (void) downloadAddressBook
{
	NSError*error = nil;
	
	NSArray *path = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *file = [NSString stringWithFormat:@"%@/contacts.txt", [path objectAtIndex:0]];
	
	NSString *string = [NSString stringWithContentsOfFile:file encoding:NSUTF8StringEncoding error:&error];
	
	[self loadFromContactList:string];
	
	[[NSNotificationCenter defaultCenter] postNotificationName:OnContactListCompleteNotification object:nil userInfo:nil];	
	
}


- (NSMutableArray*) onlineContacts
{
    return contactsOnline;
}

- (NSMutableArray*) offlineContacts
{
    return contactsOffline;
}

- (void) refreshOnlineStatus
{
	NSMutableArray *numbers = [[NSMutableArray alloc] init];
    for (VPContact* contact in contactsOffline)
    {
        if ([contact.number length] != 0)
        {
            [numbers addObject:contact.number];
        }
    }
    
    for (VPContact* contact in contactsOnline)
    {
        if ([contact.number length] != 0)
        {
            [numbers addObject:contact.number];
        }
    }
    
	if ([numbers count])
    {
        [[VPEngine sharedInstance] askOnline:numbers];
    }
	
	
}

/*
- (VPContact*) contactByUsername:(NSString*)username
{
    VPContact *ret = nil;
    for (VPContact *contact in contacts)
    {
        if ([contact.userName isEqualToString:username])
        {
            ret = contact;
            break;
        }
    }
    return ret;
}
*/

- (VPContact*) contactByNumber:(NSString*)number
{
    for (VPContact *contact in contactsOnline)
    {
        if ([contact.number isEqualToString:number])
        {
            return contact;
        }
    }

	for (VPContact *contact in contactsOffline)
    {
        if ([contact.number isEqualToString:number])
        {
            return contact;
        }
    }

	
    return nil;    
}


- (void) addOrReplaceContact:(VPContact*)contact
{
/*	
    // Update local cache
    unsigned i = 0;
    for (; i < [contacts count]; i++) 
    {
        if ([((VPContact*)[contacts objectAtIndex:i]).userName isEqualToString:contact.userName])
        {
            [contacts replaceObjectAtIndex:i withObject:contact];
            break;
        }
    }

    if (i == [contacts count]) [contacts addObject:contact];
*/    
	/*
    // Update database
    NSString *sql = [NSString stringWithFormat:
                     @"INSERT OR REPLACE INTO contact(id,owner,username,vpnumber,sdial"
                       ",firstName,lastName,country,state,homepage,email,mobile,office)"
                      "VALUES(%@, %d, '%@','%@','%@','%@','%@','%@','%@','%@','%@','%@','%@')",
                     (contact.cid?[NSString stringWithFormat:@"%d", contact.cid]:@"NULL"), 
                     owner, contact.username, contact.vpnumber, contact.sdial,
                     contact.firstName, contact.lastName, contact.country, contact.state,
                     contact.homepage, contact.email, contact.mobile, contact.office
                     ];
    [[VPDatabase sharedInstance] exec:sql];
	 */
}

#pragma mark -
#pragma mark  Auxiliary rotines
- (void) update
{
/*	
    if (!contacts) return;
    
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    
    
    NSMutableArray *numbers = [[NSMutableArray alloc] initWithCapacity:[contacts count]];
    for (VPContact *contact in contacts)
    {
        if (![contact.number length])
        {
//            NSLog(@"ask vpnumber for %@", contact.username);
            [[VPEngine sharedInstance] askVpnumber:contact.userName];
        }
        else 
        {
            [numbers addObject:contact.number];
        }
    }
    
    if (![numbers count])
        // fire timer with fake number
        [numbers addObject:@"123"];
    
    if ([numbers count])
    {
		// TODO: Где-то Interlocking. Комментирую. Файл принимается.
        [[VPEngine sharedInstance] askOnline:numbers aol:new AOL];
    }
    
    [numbers release];
    
    [pool release];
*/ 
}

- (void) update2
{
//    [NSThread detachNewThreadSelector:@selector(updateThread) toTarget:self withObject:nil];
}


#pragma mark -
#pragma mark Notifications

- (void) vpStackNotification:(NSNotification*)notification
{		 	
	unsigned msg	= 	[[[notification userInfo] objectForKey:@"MSG"] unsignedIntValue];
	unsigned param1 = 	[[[notification userInfo] objectForKey:@"PARAM1"] unsignedIntValue];
	unsigned param2 = 	[[[notification userInfo] objectForKey:@"PARAM2"] unsignedIntValue];
    
	switch(msg)
	{
		case VPMSG_SERVERTRANSFERFINISHED:
        {
			
			if (param1 == TCPFT_RXCONTACTS) {
				[[VPContactList sharedInstance] downloadAddressBook];
			}
//			VPMSG_SERVERTRANSFERFINISHED
			

			//NSLog(@"Server transfer finished %s", (const char*)a1);
		    //NSLog(@"  param1: %@", [NSString stringWithCString:(const char*)a1 encoding:NSUTF8StringEncoding]);
			//NSLog(@"  param2: %@", [NSString stringWithCString:(const char*)a2 encoding:NSUTF8StringEncoding]);
			
            if (param2 == 0)
            {
/*				
                NSArray *res = [[VPDatabase sharedInstance] query:
                                [NSString stringWithFormat:@"SELECT id FROM user WHERE username='%@'",
                                 [[VPEngine sharedInstance] getLogonName]]];
                
                if (!res) return;

                owner = [[[res objectAtIndex:0] objectAtIndex:0] intValue];
                
                NSArray *documentPaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
                NSString *path = [NSString stringWithFormat:@"%@/addressbook.sql",[documentPaths objectAtIndex:0]];
                
                NSMutableString *sql = [NSMutableString stringWithString:[[NSString alloc] initWithContentsOfFile:path]];
                [sql replaceOccurrencesOfString:@"'%1'" withString:[[res objectAtIndex:0] objectAtIndex:0] 
                     options:NSCaseInsensitiveSearch  range:NSMakeRange(0, [sql length])];
                
                //[[VPDatabase sharedInstance] exec:sql];
                
                NSLog(@"Contact list synchronized");
 */
                /*
#ifdef _INSERT_DEBUG_CONTACTS
                VPContact *contact = [[VPContact alloc] init];
                contact.username = @"voip2cartest";
                [[VPContactList sharedInstance] addOrReplaceContact:contact];
                
                contact.username = @"oleg_mavr";
                [[VPContactList sharedInstance] addOrReplaceContact:contact];
                
                contact.username = @"oleg_mavr1";
                [[VPContactList sharedInstance] addOrReplaceContact:contact];

                contact.username = @"VICTEST";
                [[VPContactList sharedInstance] addOrReplaceContact:contact];
                
                [contact release];
#endif            
                 */
/*				
                // Fill contacts
                [contacts removeAllObjects];
                NSArray *tuples = [[VPDatabase sharedInstance] query:
                                   [NSString stringWithFormat:
                                    @"SELECT id, username, vpnumber, sdial, firstName, lastName"
                                    ",country, state, homepage, email, mobile, office"
                                    " FROM contact WHERE owner=%d", owner]];
                if (tuples)
                {
                    for (NSArray *row in tuples) 
                    {
                        VPContact *contact = [[VPContact alloc] initWithId:[[row objectAtIndex:0] intValue]];
						
                    
                        contact.username  = [row objectAtIndex:1];
                        contact.vpnumber  = [row objectAtIndex:2];
                        contact.sdial     = [row objectAtIndex:3];                    
                        contact.firstName = [row objectAtIndex:4];
                        contact.lastName  = [row objectAtIndex:5];	
                        contact.country   = [row objectAtIndex:6];	
                        contact.state     = [row objectAtIndex:7];	                    
                        contact.homepage  = [row objectAtIndex:8];
                        contact.email     = [row objectAtIndex:9];	
                        contact.mobile    = [row objectAtIndex:10];	
                        contact.office    = [row objectAtIndex:11];
                    
                        [contacts addObject:contact];
                    }
 */            
                }
            
            
            [self update];
//            [[VPEngine sharedInstance] _postNotification:VPMMSG_CONTACTLISTSYNCHRONIZED param1:0 param2:0];

        }
        break;  
            
        case VPMSG_ABUPDATE:
        {
//            VPContact *contact = [[VPContactList sharedInstance] contactByUsername:[NSString stringWithCString:(const char*)param1 encoding:NSUTF8StringEncoding]];
//            if (contact)
//            {
//                contact.number = [NSString stringWithCString:(const char*)param2 encoding:NSUTF8StringEncoding];
//                [[VPContactList sharedInstance] addOrReplaceContact:contact];
//            }
        }
        break;
        case VPMSG_QUERYONLINEACK:
        {
			
		}	
        break;
            
            
    }
}

- (void) loadFromSearchList:(NSString*)list
{
	[contactsOnline removeAllObjects]; 
	[contactsOffline removeAllObjects];
	
	NSArray* records = [list componentsSeparatedByString:@"\r\n"];
	
	for (NSString *record in records) 
	{
		NSArray* fields = [record componentsSeparatedByString:@"\t"];
		if ([fields count] == 15) {
			//for (NSString *field in fields)
			//					{
			VPContact* contact = [[VPContact alloc] init];
//			contact.regCode = [fields objectAtIndex:0];
			contact.userName = [fields objectAtIndex:0];
			contact.number = [fields objectAtIndex:1];
			contact.firstName = [fields objectAtIndex:2];
			contact.lastName = [fields objectAtIndex:3];				
			contact.email = [fields objectAtIndex:4];					
			contact.country = [fields objectAtIndex:5];					
			contact.state = [fields objectAtIndex:6];
			contact.city = [fields objectAtIndex:7];
			contact.birthday = [fields objectAtIndex:8];
			contact.gender = [fields objectAtIndex:9];
			contact.phoneHome = [fields objectAtIndex:10];				
			contact.phoneOffice = [fields objectAtIndex:11];			
			contact.phoneMobile = [fields objectAtIndex:12];			
			contact.photoUpdate = [fields objectAtIndex:13];			
			contact.flags = [[fields objectAtIndex:14] intValue];	
			
			[contactsOnline addObject:contact]; 
			
			[self loadPhoto:contact.userName lastUpdate:contact.photoUpdate];

		}
	}
	
}

- (void) loadFromContactList:(NSString*)list
{
	[contactsOnline removeAllObjects]; 
	[contactsOffline removeAllObjects];
	
	NSArray* records = [list componentsSeparatedByString:@"\r\n"];
	
	for (NSString *record in records) 
	{
		NSArray* fields = [record componentsSeparatedByString:@"\t"];
		if ([fields count] == 15) {
			if (record == [records objectAtIndex:0]) {
				VPProfile* profile = [VPProfile singleton];
				profile.userName = [fields objectAtIndex:0];
				profile.number = [fields objectAtIndex:1];
				profile.firstName = [fields objectAtIndex:2];
				profile.lastName = [fields objectAtIndex:3];				
				profile.email = [fields objectAtIndex:4];					
				profile.country = [fields objectAtIndex:5];					
				profile.state = [fields objectAtIndex:6];
				profile.city = [fields objectAtIndex:7];
				profile.birthday = [fields objectAtIndex:8];
				profile.gender = [fields objectAtIndex:9];
				profile.phoneHome = [fields objectAtIndex:10];				
				profile.phoneOffice = [fields objectAtIndex:11];			
				profile.phoneMobile = [fields objectAtIndex:12];			
				profile.photoUpdate = [fields objectAtIndex:13];			
				profile.flags = [[fields objectAtIndex:14] intValue];	
				[[VPProfile singleton] update];
			} else {
				VPContact* contact = [[VPContact alloc] init];
				//contact.regCode = [fields objectAtIndex:0];
				contact.userName = [fields objectAtIndex:0];
				contact.number = [fields objectAtIndex:1];
				contact.firstName = [fields objectAtIndex:2];
				contact.lastName = [fields objectAtIndex:3];				
				contact.email = [fields objectAtIndex:4];					
				contact.country = [fields objectAtIndex:5];					
				contact.state = [fields objectAtIndex:6];
				contact.city = [fields objectAtIndex:7];
				contact.birthday = [fields objectAtIndex:8];
				contact.gender = [fields objectAtIndex:9];
				contact.phoneHome = [fields objectAtIndex:10];				
				contact.phoneOffice = [fields objectAtIndex:11];			
				contact.phoneMobile = [fields objectAtIndex:12];			
				contact.photoUpdate = [fields objectAtIndex:13];			
				contact.flags = [[fields objectAtIndex:14] intValue];	
				
				[contactsOffline addObject:contact];
				
				[self loadPhoto:contact.userName lastUpdate:contact.photoUpdate];
				
			}
		}
	}
	
}

- (void)loadPhoto:(NSString*)number lastUpdate:(NSString*)update 
{
	
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *file = [NSString stringWithFormat:@"%@/photos/%@.jpg",[paths objectAtIndex:0], update];
	NSFileManager *fm = [NSFileManager defaultManager]; 
	
	if (![fm fileExistsAtPath:file]){
		[[VPEngine sharedInstance] loadPhoto:number lastUpdate:update];
	}	
}



@end

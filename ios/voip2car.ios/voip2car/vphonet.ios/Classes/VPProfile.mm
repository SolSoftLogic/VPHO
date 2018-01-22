//
//  VPProfile.mm
//  Vphonet
//
//  Created by uncle on 25.02.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import "VPDatabase.h"
#import "VPProfile.h"
#import "VPEngine.h"

static VPProfile* __vpProfile = nil;

@implementation VPCountry

@synthesize name;
@synthesize code;
@synthesize iso2;



-(id) initFromDatabase:(NSString*) withCode
{
	self = [super init];
	
	NSArray* result = [[VPDatabase sharedInstance] query:
					   [NSString stringWithFormat:@"SELECT code, name, iso2 FROM countries WHERE iso2 = '%@'", withCode]];
	
	if (result)
	{
		for (NSArray *row in result) 
		{
			self.code = [row objectAtIndex:0];
			self.name = [row objectAtIndex:1];
			self.iso2 = [row objectAtIndex:2];
		}
	}
	return self;
}


@end

@implementation VPProfile


@synthesize userName;
@synthesize password;
@synthesize number;
@synthesize firstName;
@synthesize lastName;
@synthesize email;
@synthesize country;
@synthesize countryName;
@synthesize countryCode;
@synthesize state;
@synthesize city;
@synthesize birthday;
@synthesize gender;
@synthesize phoneHome;
@synthesize phoneOffice;
@synthesize phoneMobile;
@synthesize phoneVirtual;
@synthesize phoneForward;
@synthesize photoUpdate;
@synthesize flags;
@synthesize status;
@synthesize statusMessage;
@synthesize lastLogin;


+(VPProfile*) singleton {
	@synchronized(__vpProfile) {
		if(__vpProfile == nil) {
			__vpProfile = [[VPProfile alloc] init];
		}
	}
	return __vpProfile;
}


- (void) dealloc
{   
	[userName release];
	[password release];
	[number release];
	[firstName release];
	[lastName release];
	[email release];
	[country release];
	[state release];
	[birthday release];
	[gender release];
	[phoneHome release];
	[phoneOffice release];
	[phoneMobile release];
	[photoUpdate release];
	[statusMessage release];
    
    [super dealloc];
}

+ (VPProfile *) read
{
//	int ServerTransfer(TCPFT_RXCONTACTS, const char *path, bool amactive, const char *vname = 0);
	
	VPProfile *profile = nil;
	NSArray *result = [[VPDatabase sharedInstance] query:
						@"SELECT id, first_name, last_name FROM profiles LIMIT 1"];
	if (result)
	{
		for (NSArray *row in result) 
		{
			profile = [[VPProfile alloc] init];
			
			profile.firstName  = [row objectAtIndex:1];
			profile.lastName   = [row objectAtIndex:2];
		}
	}
	return profile;
}

- (void) write
{
	
}


+(void) update:(NSString*)login withPassword:(NSString*)pwd 
{
	
	NSString* sql = [NSString stringWithFormat:@"SELECT login, password, first_name, last_name FROM profiles WHERE login = '%@' AND password = '%@'",
							  login,
							  pwd];
	NSArray* result = [[VPDatabase sharedInstance] query:sql];
	
	if ([result count] > 0) {
		
		for (NSArray *row in result) 
		{
			[VPProfile singleton].userName = [row objectAtIndex:0];
			[VPProfile singleton].password = [row objectAtIndex:1];
			[VPProfile singleton].firstName = [row objectAtIndex:2];
			[VPProfile singleton].lastName = [row objectAtIndex:3];
		}
		
		sql = [@"UPDATE profiles " 
							  stringByAppendingFormat:@"SET login_at = %f WHERE login = '%@' AND password = '%@'",
							  [[NSDate date] timeIntervalSince1970], login, pwd];
	} else {
		sql = [NSString stringWithFormat:@"INSERT INTO profiles (login, password, login_at) VALUES ('%@', '%@', %f)",
							 login, pwd, [[NSDate date] timeIntervalSince1970]];
	}
	
	[[VPDatabase sharedInstance] exec: sql];
}


-(void) update
{
	
	NSString* sql;
	VPProfile* profile = [VPProfile singleton];
		
	sql = @"UPDATE profiles SET ";
	sql = [sql stringByAppendingFormat:@"login_at = %f, ",[[NSDate date] timeIntervalSince1970]];
	sql = [sql stringByAppendingFormat:@"number = '%@', ", profile.number];
	sql = [sql stringByAppendingFormat:@"first_name = '%@', ", profile.firstName];
	sql = [sql stringByAppendingFormat:@"last_name = '%@', ", profile.lastName];
	sql = [sql stringByAppendingFormat:@"email = '%@', ", profile.email];
	sql = [sql stringByAppendingFormat:@"country = '%@', ", profile.country];
	sql = [sql stringByAppendingFormat:@"state = '%@', ", profile.state];
	sql = [sql stringByAppendingFormat:@"birthday = '%@', ", profile.birthday];
	sql = [sql stringByAppendingFormat:@"gender = '%@', ", profile.gender];
	sql = [sql stringByAppendingFormat:@"phoneHome = '%@', ", profile.phoneHome];
	sql = [sql stringByAppendingFormat:@"phoneOffice = '%@', ", profile.phoneOffice];
	sql = [sql stringByAppendingFormat:@"phoneMobile = '%@', ", profile.phoneMobile];
	sql = [sql stringByAppendingFormat:@"photoUpdate = '%@', ", profile.photoUpdate];
	sql = [sql stringByAppendingFormat:@"flags = %d, ", profile.flags];
	sql = [sql stringByAppendingFormat:@"status = %d ", profile.status];
	sql = [sql stringByAppendingFormat:@"WHERE login = '%@' AND password = '%@'", profile.userName, profile.password];
		
		
	[[VPDatabase sharedInstance] exec: sql];
}





+(void) insert:(VPProfile*) profile
{
	
	NSError *err = nil;
	
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *file = [NSString stringWithFormat:@"%@/photos/profile.jpg",[paths objectAtIndex:0]];
	NSFileManager *fm = [NSFileManager defaultManager]; 
	[fm removeItemAtPath:file error:&err];
	
	NSString* sqlStatement = [@"INSERT INTO profiles (number, login, password, first_name, last_name, email, country, state, phoneHome, photoUpdate, status, login_at) VALUES" 
							  stringByAppendingFormat:@"('%@', '%@',  '%@',	   '%@',	   '%@',	  '%@',  '%@',	  '%@',  '%@',      '%@' ,       0,       0)",
							  profile.number,
							  profile.userName,
							  profile.password,
							  profile.firstName,
							  profile.lastName,
							  profile.email,
							  profile.country,
							  ([profile.state length] == 0) ? @"" : profile.state,
							  profile.phoneHome,
							  ([profile.photoUpdate length] == 0) ? @"" : profile.photoUpdate];
	
	[[VPDatabase sharedInstance] exec: sqlStatement];
	
}


+ (void) clear
{
	NSString* sqlStatement = @"DELETE FROM profiles";
	[[VPDatabase sharedInstance] exec: sqlStatement];
}


-(id) initFromDatabase
{
	self = [super init];
	
//	NSString* sqlStatement = [@"INSERT INTO profiles (number, login, password, first_name, last_name, email, country, state, phoneHome, photoUpdate, status, login_at) VALUES" 

	
	
	NSArray* result = [[VPDatabase sharedInstance] query:@"SELECT number, login, password, first_name, last_name, email, country, state, phoneHome, photoUpdate, status, login_at FROM profiles ORDER BY login_at"];
	
	if (result)
	{
		for (NSArray *row in result) 
		{
			self.number	  = [row objectAtIndex:0];
			self.userName = [row objectAtIndex:1];
			self.password = [row objectAtIndex:2];
			self.firstName = [row objectAtIndex:3];
			self.lastName = [row objectAtIndex:4];
			self.email = [row objectAtIndex:5];
			self.country = [row objectAtIndex:6];
			self.state = [row objectAtIndex:7];
			self.phoneHome = [row objectAtIndex:8];
			self.photoUpdate = [row objectAtIndex:9];
			self.status = [[row objectAtIndex:9] intValue];
			self.lastLogin = [[row objectAtIndex:9] doubleValue];
			//			call.start = [[row objectAtIndex:5] doubleValue];
		}
	}
	return self;
}

- (NSString*) statusImageName
{
//	int status = [[VPEngine sharedInstance] userStatus];
	
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
//	int status = [[VPEngine sharedInstance] userStatus];
	
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



@end

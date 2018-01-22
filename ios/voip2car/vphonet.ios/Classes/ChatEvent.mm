//
//  ChatEvent.m
//  Vphonet
//
//  Created by uncle on 05.05.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import "VPProfile.h"
#import "VPDatabase.h"
#import "ChatEvent.h"


@implementation ChatEvent

@synthesize date;
@synthesize from;
@synthesize to;
@synthesize attachment;
@synthesize message;
@synthesize status;
@synthesize progress;

-(void) dealloc {
	[date dealloc];
	[from dealloc];
	[to dealloc];
	[attachment dealloc];
	[message dealloc];
	[super dealloc];
}

-(NSString*) attachmentFileForContact:(NSString*)contact
{
//	NSError *error;
	NSArray *path = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *file = [NSString stringWithFormat:@"%@/chats/%@/%@", [path objectAtIndex:0], contact, attachment];

	return file;
}

@end


@implementation ChatMessages

@synthesize list;

- (id) init
{
    self = [super init];
    
    list = [[NSMutableArray alloc] init];
	
	return self;
}


-(void) dealloc {
	[list dealloc];
	[super dealloc];
}

	
-(void) load:(NSString*)contact
{
	[list removeAllObjects];
	
	NSError *error;
	NSArray *path = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *file = [NSString stringWithFormat:@"%@/chats/%@/%@.txt", [path objectAtIndex:0], contact, contact];
	
	NSString *data = [NSString stringWithContentsOfFile:file encoding:NSUTF8StringEncoding error:&error];
	
	NSArray* records = [data componentsSeparatedByString:@"\r\n"];
	
	for (NSString *record in records) 
	{
		NSArray* fields = [record componentsSeparatedByString:@"\t"];
		if ([fields count] == 5) {
			ChatEvent* event = [[ChatEvent alloc] init];
			
			event.date = [fields objectAtIndex:0];
			event.from = [fields objectAtIndex:1];
			event.to = [fields objectAtIndex:2];
			event.attachment = [fields objectAtIndex:3];
			event.message = [fields objectAtIndex:4];
			[list addObject:event]; 
		}
	}

}
	
-(void) save:(NSString*)contact
{
	NSString *data = @"";
	
	for (ChatEvent* event in list) {
		data = [data stringByAppendingFormat:@"%@\t", event.date]; 
		data = [data stringByAppendingFormat:@"%@\t", event.from];					
		data = [data stringByAppendingFormat:@"%@\t", event.to];				
		data = [data stringByAppendingFormat:@"%@\t", event.attachment];				
		data = [data stringByAppendingFormat:@"%@\r\n", event.message];					
	}

	NSError *error;
	NSArray *path = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *file = [NSString stringWithFormat:@"%@/chats/%@/%@.txt", [path objectAtIndex:0], contact, contact];
	
	if (![data writeToFile:file atomically:NO encoding:NSUTF8StringEncoding error:&error]) {
		NSLog(@"%@", error);
	}
	
}
	
-(void) add:(NSString*)from to:(NSString*)to attachment:(NSString*)attachment message:(NSString*)message
{
	NSDate* date= [NSDate date]; 
	NSDateFormatter *formatter=[[[NSDateFormatter alloc] init] autorelease]; 
	[formatter setDateStyle:NSDateFormatterMediumStyle]; 
	[formatter setTimeStyle:NSDateFormatterShortStyle]; 
	
	ChatEvent* event = [[ChatEvent alloc] init];
	
	event.date = [formatter stringFromDate:date];
	event.from = [NSString stringWithString:from];
	event.to = [NSString stringWithString:to];
	event.attachment = [NSString stringWithString:attachment];
	event.message = [NSString stringWithString:message];
	
	event.status = [NSString stringWithString:@"S"];
	event.progress = 0.0;
	[list addObject:event];
	
	[ChatMessages save:from to:to attachment:attachment message:message];
	
}

+(void) save:(NSString*)from to:(NSString*)to attachment:(NSString*)attachment message:(NSString*)message
{
	NSDate* date= [NSDate date]; 
	NSDateFormatter *formatter=[[[NSDateFormatter alloc] init] autorelease]; 
	[formatter setDateStyle:NSDateFormatterMediumStyle]; 
	[formatter setTimeStyle:NSDateFormatterShortStyle]; 
	NSString *data = @"";
	data = [data stringByAppendingFormat:@"%@\t", [formatter stringFromDate:date]]; 
	data = [data stringByAppendingFormat:@"%@\t", from];					
	data = [data stringByAppendingFormat:@"%@\t", to];				
	data = [data stringByAppendingFormat:@"%@\t", attachment];				
	data = [data stringByAppendingFormat:@"%@\r\n", message];					
	
	NSString *contact = @"";
	
	if ([from isEqualToString:[[VPProfile singleton] number]]) {
		contact = [NSString stringWithString:to];
	} else {
		contact = [NSString stringWithString:from];
	}
	
	
	NSError *error;
	NSArray *path = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *file = [NSString stringWithFormat:@"%@/chats/%@/%@.txt", [path objectAtIndex:0], contact, contact];
	
	NSFileManager *fileManager = [NSFileManager defaultManager];
	if (![fileManager fileExistsAtPath:file]) {
		if(![fileManager createDirectoryAtPath:[NSString stringWithFormat:@"%@/chats/%@", [path objectAtIndex:0], contact] withIntermediateDirectories:YES attributes:nil error:&error])
			NSLog(@"%@", error);
		if (![data writeToFile:file atomically:YES encoding:NSUTF8StringEncoding error:&error]) {
			NSLog(@"%@", error);
		}
	} else {
		NSFileHandle *fileHandle = [NSFileHandle fileHandleForWritingAtPath:file];
		[fileHandle seekToEndOfFile];
		NSData *textData = [data dataUsingEncoding:NSUTF8StringEncoding];
		[fileHandle writeData:textData];
		[fileHandle closeFile];
	}
}


+(NSUInteger) activityUnReadMessages
{
	NSString* sql		= [NSString stringWithFormat:@"SELECT SUM(unread) FROM chats ORDER BY last DESC LIMIT 15"];
	
	NSArray* records	= [[VPDatabase sharedInstance] query:sql];
	
	if ([records count] > 0) 
	{
		NSUInteger result = 0;
		for (NSArray *row in records) 
		{
			result += [[row objectAtIndex:0] intValue];
		}
		return result;
	}
	
	return 0;
}

+(BOOL) activityReset:(NSString*)number
{
	NSString* sqlUpdate = [NSString stringWithFormat:@"UPDATE chats SET unread = 0 WHERE vpnumber = '%@'", number];
	return [[VPDatabase sharedInstance] exec: sqlUpdate];
}

+(BOOL) activityDelete:(NSString*)number
{
	NSError *error;
	NSArray *path = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *file = [NSString stringWithFormat:@"%@/chats/%@", [path objectAtIndex:0], number];
	
	NSFileManager *fileManager = [NSFileManager defaultManager];
	if (![fileManager removeItemAtPath:file error:&error]) {
		NSLog(@"Remove chats error: %@", error);
	}
		
	NSString* sqlUpdate = [NSString stringWithFormat:@"DELETE FROM chats WHERE vpnumber = '%@'", number];
	return [[VPDatabase sharedInstance] exec: sqlUpdate];
}



+(void) activityUpdate:(NSString*)number withMessage:(NSString*)message  
{
	NSString* sql		= [NSString stringWithFormat:@"SELECT vpnumber FROM chats WHERE vpnumber = '%@'", number];
	NSArray* records	= [[VPDatabase sharedInstance] query:sql];
	
	if ([records count] > 0) 
	{
		NSString* sqlUpdate = [NSString stringWithFormat:@"UPDATE chats SET last = %f, last_message = '%@', unread = unread + 1 WHERE vpnumber = '%@'",
							   [[NSDate date] timeIntervalSince1970], message, number];
		[[VPDatabase sharedInstance] exec: sqlUpdate];
	} else {
	
		NSString* sqlInsert = [NSString stringWithFormat:@"INSERT INTO chats (vpnumber, last_message, last, unread) "
													"VALUES ('%@', '%@', %f, 1)", number, message, [[NSDate date] timeIntervalSince1970]];
		[[VPDatabase sharedInstance] exec: sqlInsert];
	}
}

/*
+(NSArray*) activities  
{
	NSString* sql = [NSString stringWithFormat:@"SELECT vpnumber FROM chats ORDER BY last DESC LIMIT 15"];
	
	NSArray* records = [[VPDatabase sharedInstance] query:sql];
	NSMutableArray* result = [NSArray array];
	
	if ([records count] > 0) {
		
		for (NSArray *row in records) 
		{
			[result addObject:[NSString stringWithString:[row objectAtIndex:0]]];
		}
	}
	return result;
}
*/

	
@end
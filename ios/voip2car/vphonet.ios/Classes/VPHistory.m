//
//  VPHistory.m
//  Vphonet
//
//  Created by uncle on 26.02.11.
//  Copyright 2011. All rights reserved.
//

#import "VPDatabase.h"
#import "VPHistory.h"

static VPHistory* __vpHistory = nil;


@implementation VPHistory

+(VPHistory*) singleton
{
	@synchronized(__vpHistory) 
	{
		if(__vpHistory == nil)
		{
			__vpHistory = [[VPHistory alloc] init];
		}
	}
	
	return __vpHistory;
}

-(id)init{
	if((self = [super init])){
		self->events = [[NSMutableArray alloc] init];
		self->eventsByDays = [[NSMutableDictionary alloc] init];
	}
	return self;
}

-(void)dealloc{
	[self->eventsByDays dealloc];
	[self->events dealloc];
	
	[super dealloc];
}



-(BOOL) isLoadingHistory{
	return self->loading;
}

-(BOOL) load
{
	[self->events removeAllObjects];
	[self->eventsByDays removeAllObjects];
	
	NSArray *result = [[VPDatabase sharedInstance] query:
					   @"SELECT id, seen, status, type, remote, start, end FROM calls ORDER BY start DESC"];
	if (result)
	{
		for (NSArray *row in result) 
		{
			HistoryEvent *call = [[HistoryEvent alloc] init];
			
			call.seen = [[row objectAtIndex:1] boolValue];
			call.status = [[row objectAtIndex:2] intValue];
			call.type = [[row objectAtIndex:3] intValue];
			call.remote = [row objectAtIndex:4];
			call.start = [[row objectAtIndex:5] doubleValue];
			call.end = [[row objectAtIndex:6] doubleValue];
			
			[events addObject:call];
			
			
			NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
			dateFormatter.dateFormat = @"yyyy-MM-dd";
			NSString *dayKey = [dateFormatter stringFromDate:[NSDate dateWithTimeIntervalSince1970:call.start]];
			[dateFormatter release];
			
			NSMutableArray *day = [eventsByDays objectForKey:dayKey];
			
			if (day == NULL)
			{
				day = [[NSMutableArray alloc] init];
				[eventsByDays setObject:day forKey:dayKey];
			}
			
			[day addObject:call];
		}
	}
	return [result lastObject] != nil;
}

-(BOOL) addEvent: (HistoryEvent*)event{	
	switch (event.type) {
		case HistoryEventType_Audio:
		case HistoryEventType_AudioVideo:
		{
			NSString* sqlStatement = [@"insert into calls (seen,status,type,remote,start,end) values" 
									  stringByAppendingFormat:@"(%d,%d,%d,'%@',%f,%f)",
									  event.seen ? 1 : 0,
									  (int)event.status,
									  (int)event.type,
									  event.remote,
									  event.start,
									  event.end];
			BOOL result = [[VPDatabase sharedInstance] exec: sqlStatement];
			return result;
//			if(success){
//				[self->events insertObject:event atIndex:0];
//				[[NSNotificationCenter defaultCenter] postNotificationName:@"HistoryChanged" object:self];
//			}

			break;
		}
//			return [self databaseAddAVCall: (HistoryAVCallEvent*)event];
		default:
			return NO;
	}
	return NO;
}

-(BOOL) updateEvent: (HistoryEvent*)event{
	return NO;
}

-(BOOL) deleteEvent: (HistoryEvent*)event{
	return NO;
}

-(BOOL) clear{
	
	BOOL result = [[VPDatabase sharedInstance] exec:@"delete from calls"];

	if(result){
		[self->events removeAllObjects];
		[[NSNotificationCenter defaultCenter] postNotificationName:@"HistoryChanged" object:self];
	}
	
	return result;
}

-(NSMutableArray*)events{
	return self->events;
}

-(NSMutableDictionary*)eventsByDays{
	return self->eventsByDays;
}


@end

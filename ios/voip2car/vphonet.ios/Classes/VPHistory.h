//
//  VPHistory.h
//  Vphonet
//
//  Created by uncle on 26.02.11.
//  Copyright 2011. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "HistoryEvent.h"


@interface VPHistory : NSObject {
	BOOL				loading;
	NSMutableArray		*events;
	NSMutableDictionary	*eventsByDays;
}

+(VPHistory*) singleton;
-(BOOL) load;
-(BOOL) isLoadingHistory;
-(BOOL) addEvent:	(HistoryEvent*)event;
-(BOOL) updateEvent: (HistoryEvent*)event;
-(BOOL) deleteEvent: (HistoryEvent*)event;
-(BOOL) clear;

-(NSMutableArray*)events;
-(NSMutableDictionary*)eventsByDays;

@end

//
//  HistoryEvent.m
//  Vphonet
//
//  Created by uncle on 25.02.11.
//  Copyright 2011. All rights reserved.
//

#import "HistoryEvent.h"


@implementation HistoryEvent

@synthesize type;
@synthesize seen;
@synthesize status;
@synthesize remote;
@synthesize start;
@synthesize end;


-(HistoryEvent*)initWithType: (HistoryEventType_t)_type andRemote: (NSString*)_remote{
	if((self = [super init])){
		self->type = _type;
		self->remote = [_remote retain];
		
		self->start = [[NSDate date] timeIntervalSince1970];
		self->end = self->start;
		self->status = HistoryEventStatus_Missed;
	}
	return self;
}

@end


@implementation HistoryAVCallEvent

-(HistoryAVCallEvent*)initAudioCallEvent: (NSString*)_remote
{
	if((self = (HistoryAVCallEvent*)[super initWithType:HistoryEventType_Audio andRemote:_remote]))
	{
	}
	return self;
}

-(HistoryAVCallEvent*)initAudioVideoCallEvent: (NSString*)_remote
{
	if((self = (HistoryAVCallEvent*)[super initWithType:HistoryEventType_AudioVideo andRemote:_remote]))
	{
	}
	return self;
}

@end
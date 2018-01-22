//
//  HistoryEvent.h
//  Vphonet
//
//  Created by uncle on 25.02.11.
//  Copyright 2011. All rights reserved.
//

#import <Foundation/Foundation.h>

typedef enum HistoryEventStatus_e
{
	HistoryEventStatus_Outgoing,
	HistoryEventStatus_Incoming,
	HistoryEventStatus_Missed,
	HistoryEventStatus_Failed
}
HistoryEventStatus_t;

typedef enum HistoryEventType_e
{
	HistoryEventType_Audio,
	HistoryEventType_AudioVideo,
	HistoryEventType_SMS,
	HistoryEventType_Chat,
	HistoryEventType_FileTransfer
}
HistoryEventType_t;

@interface HistoryEvent : NSObject {
	HistoryEventType_t		type;
	BOOL					seen;
	HistoryEventStatus_t	status;
	NSString*				remote;
	NSTimeInterval			start;
	NSTimeInterval			end;
}
/*
@property(readonly,assign)	HistoryEventType_t		type;
@property(readwrite,assign) BOOL					seen;
@property(readwrite,assign) HistoryEventStatus_t	status;
@property(readonly,retain)	NSString*				remoteParty;
@property(readwrite,assign) NSTimeInterval			start;
@property(readwrite,assign) NSTimeInterval			end;
*/
@property(readwrite,assign)	HistoryEventType_t		type;
@property(readwrite,assign) BOOL					seen;
@property(readwrite,assign) HistoryEventStatus_t	status;
@property(readwrite,retain)	NSString*				remote;
@property(readwrite,assign) NSTimeInterval			start;
@property(readwrite,assign) NSTimeInterval			end;

-(HistoryEvent*)initWithType: (HistoryEventType_t)type andRemote: (NSString*)remote;

@end


@interface HistoryAVCallEvent : HistoryEvent {
}

-(HistoryAVCallEvent*)initAudioCallEvent: (NSString*)remote;
-(HistoryAVCallEvent*)initAudioVideoCallEvent: (NSString*)remote;

@end


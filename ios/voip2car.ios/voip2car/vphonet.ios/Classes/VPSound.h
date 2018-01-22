//
//  SoundManager.h
//  iostest
//
//  Created by uncle on 22.01.11.
//  Copyright 2011. All rights reserved.
//


#import <Foundation/Foundation.h>
#import <AVFoundation/AVAudioPlayer.h>
#include "AudioUnit/AudioUnit.h"
#include <AudioToolbox/AudioServices.h>
#include <AudioToolbox/AudioToolbox.h>


@protocol PSound 

-(BOOL) start;
-(BOOL) stop;

-(void) playDTMF:(char) ATone;
-(void) playTone:(UInt16) freq1:(UInt16) freq2: (UInt16) duration: (UInt16) silence;

-(void) playRingTone;
-(void) stopRingTone;
-(void) playRingBackTone;
-(void) stopRingBackTone;
-(void) playDialTone;
-(void) stopDialTone;
-(void) playBusyTone;
-(void) stopBusyTone;

-(void) playRingBackTone;
-(void) stopRingBackTone;

-(void) playNewEvent;
-(void) stopNewEvent;

-(void) playConnectionChanged:(BOOL) connected;
-(void) stopConnectionChanged:(BOOL) connected;

@end



@interface VPSound : NSObject<PSound> {
	
	AVAudioPlayer  *playerRingTone;
	


@private
	AudioStreamBasicDescription mDesc;								///< Default audio descriptions
	AudioQueueRef				mToneQueue;						
	AudioQueueBufferRef			mToneBuffer;		

}


//-(void) playDTMF:(char) tone;
+(VPSound*) singleton;

@end


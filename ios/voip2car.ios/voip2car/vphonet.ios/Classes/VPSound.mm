//
//  SoundManager.m
//  iostest
//
//  Created by uncle on 22.01.11.
//  Copyright 2011. All rights reserved.
//

/* 
 // DTMF frequencies as per http://en.wikipedia.org/wiki/Dual-tone_multi-frequency_signaling
 
 { '0', '+', 941,1336 }, 
 { '1', '+', 697,1209 }, 
 { '2', '+', 697,1336 }, 
 { '3', '+', 697,1477 }, 
 { '4', '+', 770,1209 }, 
 { '5', '+', 770,1336 }, 
 { '6', '+', 770,1477 }, 
 { '7', '+', 852,1209 }, 
 { '8', '+', 852,1336 }, 
 { '9', '+', 852,1477 }, 
 { '*', '+', 941,1209 }, 
 { '#', '+', 941,1477 }, 
 { 'A', '+', 697,1633 }, 
 { 'B', '+', 770,1633 }, 
 { 'C', '+', 852,1633 }, 
 { 'D', '+', 941,1633 }, 
 { 'a', '+', 697,1633 }, 
 { 'b', '+', 770,1633 }, 
 { 'c', '+', 852,1633 }, 
 { 'd', '+', 941,1633 }, 
 
*/


#import "VPSound.h"
#include "../../vpstack/ios/tone_generator.h"

static VPSound* __vpSound = nil;


@implementation VPSound


+(VPSound*) singleton{
	@synchronized(__vpSound) {
		if(__vpSound == nil){
			__vpSound = [[VPSound alloc] init];
			AudioSessionInitialize(NULL, NULL, NULL, NULL);
		}
	}
	
	return __vpSound;
	
}


-(void) dealloc{
	
	if(playerRingTone){
		if(playerRingTone.playing)[playerRingTone stop];
		[playerRingTone release];
	}
	
	[self stop];
	
	[super dealloc];
}

void __handle_one(void *userData, AudioQueueRef AQueue, AudioQueueBufferRef ABuffer) 
{
}


void __handle_infinity(void *userData, AudioQueueRef AQueue, AudioQueueBufferRef ABuffer) 
{
	AudioQueueEnqueueBuffer(AQueue, ABuffer, 0, NULL);
}


#define TONE_DURATION 200 // in millisecond

-(BOOL) start{
	
	mDesc.mSampleRate		= 16000;
	mDesc.mFormatID			= kAudioFormatLinearPCM;
	mDesc.mFormatFlags		= kAudioFormatFlagIsSignedInteger | kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked; 
	mDesc.mChannelsPerFrame	= 1;
	mDesc.mFramesPerPacket	= 1;
	mDesc.mBitsPerChannel	= 16;
	mDesc.mBytesPerPacket	= mDesc.mBitsPerChannel / 8 * mDesc.mChannelsPerFrame;
	mDesc.mBytesPerFrame	= mDesc.mBytesPerPacket * mDesc.mFramesPerPacket;
	mDesc.mReserved			= 0;
	
	return YES;
}

-(BOOL) stop{
	OSStatus err = noErr;
	if (mToneQueue)
	{
		err = AudioQueueStop(mToneQueue, true);
		if (err)
		{
			NSLog(@"ERROR: AudioQueueStop (stop): %@", err);
			return false;
		}
	}
	
	if (mToneQueue) 
	{
		if(mToneBuffer)
		{
			AudioQueueFreeBuffer(mToneQueue, mToneBuffer);
			mToneBuffer = NULL;
		}
		
		AudioQueueDispose(mToneQueue, true);
		mToneQueue = NULL;
	}

	
	return YES;
}


-(void) playTone:(UInt16) freq1:(UInt16) freq2: (UInt16) duration: (UInt16) silence{
	OSStatus err = noErr;

	Tones tone(40, 16000);
	tone.Generate('+', freq1, freq1, duration);
	tone.Silence(silence);
	
	// Create the playback audio queue 16000 per sec 
	err = AudioQueueNewOutput(&mDesc, __handle_infinity, NULL, NULL, NULL, 0, &mToneQueue);
	if (err) 
	{
		NSLog(@"ERROR: AudioQueueNewOutput (start): %@", err);
		return;
	}
	err = AudioQueueAllocateBuffer(mToneQueue, tone.size() * sizeof(short), &mToneBuffer);
	if (err) 
	{
		NSLog(@"ERROR: AudioQueueAllocateBuffer (start): %@", err);
		return;
	}
	mToneBuffer->mAudioDataByteSize = tone.size() * sizeof(short);
	memcpy(mToneBuffer->mAudioData, tone.data(), mToneBuffer->mAudioDataByteSize);
	
	err = AudioQueueEnqueueBuffer(mToneQueue, mToneBuffer, 0, NULL);
	if (err) 
	{
		NSLog(@"ERROR: AudioQueueEnqueueBuffer (Tone): %@", err);
		return;
	}
	
	err = AudioQueueStart(mToneQueue, NULL);
	if (err) 
	{
		NSLog(@"ERROR: AudioQueueStart (Tone): %@", err);
		return;
	}
}



-(void) playRingTone{
	[self stopRingTone];
	
	UInt32 audioRouteOverride =  kAudioSessionOverrideAudioRoute_Speaker;
	AudioSessionSetProperty(kAudioSessionProperty_OverrideAudioRoute,                         
							sizeof (audioRouteOverride),                                      
							&audioRouteOverride);

	
	if(!playerRingTone){
		NSURL *url = [NSURL fileURLWithPath:[NSString stringWithFormat:@"%@/ringtone.m4r", [[NSBundle mainBundle] resourcePath]]];
		
		NSError *error;
		playerRingTone = [[AVAudioPlayer alloc] initWithContentsOfURL:url error:&error];
		if (playerRingTone == nil){
			NSLog(@"Failed to create audio player (RingTone): %@", error);
			return;
		}
	}
	
	playerRingTone.numberOfLoops = -1;
	[playerRingTone play];


}

-(void) stopRingTone{
	UInt32 audioRouteOverride =  kAudioSessionOverrideAudioRoute_None;
	AudioSessionSetProperty(kAudioSessionProperty_OverrideAudioRoute,                         
							sizeof (audioRouteOverride),                                      
							&audioRouteOverride);
	if(playerRingTone && playerRingTone.playing){
		[playerRingTone stop];
	}
}


-(void) playRingBackTone{
	[self stop];
	[self playTone:440 :480 :2000 :4000];
	//Generate("440+480:2-4");
}

-(void) stopRingBackTone{
	[self stop];
}


/**
 * Generate PCM data for 1 second of US standard dial tone 
 * of 350/440hz 
 */
-(void) playDialTone{
	//[self stop];
	[self playTone:350 :440 :1000 :1000];
}

-(void) stopDialTone{
	[self stop];
}

/**
 * Generate PCM data for a single cadence of the US standard busy tone
 * of 480/620hz for 1/2 second, 1/2 second of silence
 */
-(void) playBusyTone{
	[self stop];
	[self playTone:480 :620 :500 :500];
}

-(void) stopBusyTone{
	[self stop];
}


-(void) playDTMF:(char) ATone{
	//if(mToneQueue){
		OSStatus err = noErr;
		
		[self stop];

		Tones tone(40, 16000);
	
		switch(ATone)
		{
			case '0' :
				tone.Generate('+', 941, 1336, TONE_DURATION);
				break;
			case '1' :
				tone.Generate('+', 697,1209, TONE_DURATION); 
				break;
			case '2' :
				tone.Generate('+', 697,1336, TONE_DURATION); 
				break;
			case '3' :
				tone.Generate('+', 697,1477, TONE_DURATION);
				break;
			case '4' :
				tone.Generate('+', 770,1209, TONE_DURATION);
				break;
			case '5' :
				tone.Generate('+', 770,1336, TONE_DURATION);
				break;
			case '6' :
				tone.Generate('+', 770,1477, TONE_DURATION);
				break;
			case '7' :
				tone.Generate('+', 852,1209, TONE_DURATION);
				break;
			case '8' :
				tone.Generate('+', 852,1336, TONE_DURATION);
				break;
			case '9' :
				tone.Generate('+', 852,1477, TONE_DURATION);
				break;
			case '*' :
				tone.Generate('+', 941,1209, TONE_DURATION);
				break;
			case '#' :
				tone.Generate('+', 941,1477, TONE_DURATION);
				break;
			case 'A' :
				tone.Generate('+', 697,1633, TONE_DURATION);
				break;
			case 'B' :
				tone.Generate('+', 770,1633, TONE_DURATION);
				break;
			case 'C' :
				tone.Generate('+', 852,1633, TONE_DURATION);
				break;
			case 'D' :
				tone.Generate('+', 941,1633, TONE_DURATION);
				break;
		}
		
		// Create the playback audio queue 16000 per sec 
		err = AudioQueueNewOutput(&mDesc, __handle_one, NULL, NULL, NULL, 0, &mToneQueue);
		if (err) 
		{
			NSLog(@"ERROR: AudioQueueNewOutput (start): %@", err);
			return;
		}
		err = AudioQueueAllocateBuffer(mToneQueue, tone.size() * sizeof(short), &mToneBuffer);
		if (err) 
		{
			NSLog(@"ERROR: AudioQueueAllocateBuffer (start): %@", err);
			return;
		}
		mToneBuffer->mAudioDataByteSize = tone.size() * sizeof(short);
		memcpy(mToneBuffer->mAudioData, tone.data(), mToneBuffer->mAudioDataByteSize);
		
		err = AudioQueueEnqueueBuffer(mToneQueue, mToneBuffer, 0, NULL);
		if (err) 
		{
			NSLog(@"ERROR: AudioQueueEnqueueBuffer (Tone): %@", err);
			return;
		}
		
		err = AudioQueueStart(mToneQueue, NULL);
		if (err) 
		{
			NSLog(@"ERROR: AudioQueueStart (Tone): %@", err);
			return;
		}
		
	//}	
}



-(void) playNewEvent{
}

-(void) stopNewEvent{
}


-(void) playConnectionChanged:(BOOL) connected{
}

-(void) stopConnectionChanged:(BOOL) connected{
}


@end


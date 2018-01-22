//
//  SoundManager.m
//  iostest
//
//  Created by uncle on 22.01.11.
//  Copyright 2011. All rights reserved.
//


#import "SoundManager.h"


@implementation SoundManager


-(void) dealloc{
	
	if(playerDTMF){
		if(playerDTMF.playing)[playerDTMF stop];
		[playerDTMF release];
	}
	
	if(playerRingBackTone){
		if(playerRingBackTone.playing)[playerRingBackTone stop];
		[playerRingBackTone release];
	}
	
	if(playerRingTone){
		if(playerRingTone.playing)[playerRingTone stop];
		[playerRingTone release];
	}
	
	if(playerEvent){
		if(playerEvent.playing)[playerEvent stop];
		[playerEvent release];
	}
	
	if(playerConn){
		if(playerConn.playing)[playerConn stop];
		[playerConn release];
	}
	
	[super dealloc];
}


-(BOOL) start{
	return YES;
}

-(BOOL) stop{
	return YES;
}

-(void) playDTMF:(int) number{
}

-(void) stopDTMF{
}


-(void) playRingTone{
	
	if(!playerRingTone){
		NSURL *url = [NSURL fileURLWithPath:[NSString stringWithFormat:@"%@/ring.wav", [[NSBundle mainBundle] resourcePath]]];
		
		NSError *error;
		playerRingTone = [[AVAudioPlayer alloc] initWithContentsOfURL:url error:&error];
		if (playerRingTone == nil){
			NSLog(@"ERROR: create audio player (RingTone): %@", error);
			return;
		}
	}
	
	playerRingTone.numberOfLoops = -1;
	[playerRingTone play];
}

-(void) stopRingTone{
	if(playerRingTone && playerRingTone.playing){
		[playerRingTone stop];
	}
}


-(void) playRingBackTone{
	if(!playerRingBackTone){
		NSURL *url = [NSURL fileURLWithPath:[NSString stringWithFormat:@"%@/ringbacktone.wav", [[NSBundle mainBundle] resourcePath]]];
		
		NSError *error;
		playerRingBackTone = [[AVAudioPlayer alloc] initWithContentsOfURL:url error:&error];
		if (playerRingBackTone == nil){
			NSLog(@"ERROR: create audio player (RingBackTone): %@", error);
			return;
		}
	}
	
	playerRingBackTone.numberOfLoops = -1;
	[playerRingBackTone play];
}

-(void) stopRingBackTone{
	if(playerRingBackTone && playerRingBackTone.playing){
		[playerRingBackTone stop];
	}
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


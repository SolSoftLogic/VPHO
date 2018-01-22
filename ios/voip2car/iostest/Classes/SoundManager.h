//
//  SoundManager.h
//  iostest
//
//  Created by uncle on 22.01.11.
//  Copyright 2011. All rights reserved.
//


#import <Foundation/Foundation.h>
#import <AVFoundation/AVAudioPlayer.h>

#import "PSoundManager.h"

@interface SoundManager : NSObject<PSoundManager> {
	
	AVAudioPlayer  *playerDTMF;
	AVAudioPlayer  *playerRingBackTone;
	AVAudioPlayer  *playerRingTone;
	AVAudioPlayer  *playerEvent;
	AVAudioPlayer  *playerConn;
}

@end


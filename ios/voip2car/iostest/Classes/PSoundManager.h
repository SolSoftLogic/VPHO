//
//  PSoundManager.h
//  iostest
//
//  Created by uncle on 22.01.11.
//  Copyright 2011. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "PManager.h"

@protocol PSoundManager <PManager>

-(void) playDTMF:(int) number;
-(void) stopDTMF;

-(void) playRingTone;
-(void) stopRingTone;

-(void) playRingBackTone;
-(void) stopRingBackTone;

-(void) playNewEvent;
-(void) stopNewEvent;

-(void) playConnectionChanged:(BOOL) connected;
-(void) stopConnectionChanged:(BOOL) connected;

@end

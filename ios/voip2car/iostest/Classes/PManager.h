/*
 *  PManager.h
 *  iostest
 *
 *  Created by uncle on 22.01.11.
 *  Copyright 2011. All rights reserved.
 *
 */


#import <UIKit/UIKit.h>


/// Abstract protocol for application managers.
@protocol PManager

-(BOOL) start;
-(BOOL) stop;

@end


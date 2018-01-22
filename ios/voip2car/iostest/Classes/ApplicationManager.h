//
//  ApplicationManager.h
//  iostest
//
//  Created by uncle on 22.01.11.
//  Copyright 2011. All rights reserved.
//


#import <Foundation/Foundation.h>

#import "PSoundManager.h"

#define SharedManager [ApplicationManager appManager]

@interface ApplicationManager : NSObject {
	NSObject<PSoundManager>* soundManager;
}

-(BOOL) start;
-(BOOL) stop;

// managers
@property(readonly, retain) NSObject<PSoundManager>* soundManager;

// singleton
+(ApplicationManager*) appManager;

@end


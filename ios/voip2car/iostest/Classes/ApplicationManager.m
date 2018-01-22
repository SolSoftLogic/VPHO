//
//  ApplicationManager.m
//  iostest
//
//  Created by uncle on 22.01.11.
//  Copyright 2011. All rights reserved.
//

#import "ApplicationManager.h"

#import "SoundManager.h"


static ApplicationManager* __appManager = nil;

@interface ApplicationManager(Multithreading)
-(void)dummyCoCoaThread;
@end

@implementation ApplicationManager(Multithreading)
-(void)dummyCoCoaThread {
}
@end


@implementation ApplicationManager

@synthesize soundManager;

-(id) init{
	self = [super init];
	
	if(self){
		self->soundManager = [[SoundManager alloc] init];
	}
	
	return self;
}


-(BOOL) start{
	BOOL ret = YES;
	
	/* http://developer.apple.com/library/mac/#documentation/Cocoa/Reference/Foundation/Classes/NSAutoreleasePool_Class/Reference/Reference.html
	 */
	[NSThread detachNewThreadSelector:@selector(dummyCoCoaThread) toTarget:self withObject:nil];
	if([NSThread isMultiThreaded]){
		NSLog(@"Working in multithreaded mode ;)");
	}
	else{
		NSLog(@"NOT working in multithreaded mode ;(");
	}
	
	ret &= [self.soundManager start];
	
	return ret;
}

-(BOOL) stop{
	BOOL ret = YES;
	
	ret &= [self.soundManager stop];
	
	return ret;
}

+(ApplicationManager*) appManager{
	@synchronized(__appManager) {
		if(__appManager == nil){
			__appManager = [[ApplicationManager alloc] init];
		}
	}
	
	return __appManager;
	
}

-(void)dealloc{
	[self->soundManager release];
	
	[super dealloc];
}

@end



//
//  Manager.m
//  iostest
//
//  Created by uncle on 22.01.11.
//  Copyright 2011. All rights reserved.
//

#import "Manager.h"

@implementation Manager


-(BOOL) start{
	[NSException raise:NSInternalInconsistencyException 
				format:@"You must override %@ in a superclass", NSStringFromSelector(_cmd)];
	return NO;
}

-(BOOL) stop{
	[NSException raise:NSInternalInconsistencyException 
				format:@"You must override %@ in a superclass", NSStringFromSelector(_cmd)];
	return NO;
}

@end



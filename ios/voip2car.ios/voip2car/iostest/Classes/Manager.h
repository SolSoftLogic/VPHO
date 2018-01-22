//
//  Manager.h
//  iostest
//
//  Created by uncle on 22.01.11.
//  Copyright 2011. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "PManager.h"

@interface Manager : NSObject<PManager> {
	
	
}

-(BOOL) start;
-(BOOL) stop;

@end

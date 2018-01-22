//
//  VPCountries.h
//  Vphonet
//
//  Created by uncle on 14.04.11.
//  Copyright 2011. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface Country : NSObject {
	NSInteger				dbid;
	NSString*				code;
	NSString*				name;
	NSString*				iso2;
}

@property(readwrite)		NSInteger dbid;
@property(readwrite,retain)	NSString* code;
@property(readwrite,retain)	NSString* name;
@property(readwrite,retain)	NSString* iso2;

@end


///< Country list
@interface VPCountries : NSObject {

	BOOL				loading;
	NSMutableArray		*list;

}

+(VPCountries*) singleton;
-(BOOL) load;
-(BOOL) isLoading;

-(NSMutableArray*)list;


@end

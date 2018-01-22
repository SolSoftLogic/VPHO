//
//  VPCountries.m
//  Vphonet
//
//  Created by uncle on 14.04.11.
//  Copyright 2011. All rights reserved.
//

#import "VPDatabase.h"
#import "VPCountries.h"


static VPCountries* __vpCountries = nil;

@implementation Country
@synthesize dbid;
@synthesize code;
@synthesize name;
@synthesize iso2;
@end


@implementation VPCountries


+(VPCountries*) singleton {
	@synchronized(__vpCountries) {
		if(__vpCountries == nil) {
			__vpCountries = [[VPCountries alloc] init];
		}
	}
	return __vpCountries;
}

-(id)init{
	if((self = [super init])){
		self->list = [[NSMutableArray alloc] init];
		self->loading = [self load];
	}
	return self;
}

-(void)dealloc{
	[self->list dealloc];
	[super dealloc];
}



-(BOOL) isLoading{
	return self->loading;
}

-(BOOL) load
{
	[self->list removeAllObjects];
	
	NSArray *result = [[VPDatabase sharedInstance] query:
					   @"SELECT id, code, name, iso2 FROM countries ORDER BY name ASC"];
	if (result)
	{
		for (NSArray *row in result) 
		{
			Country *country = [[Country alloc] init];
			
			country.dbid = [[row objectAtIndex:0] intValue];
			country.code = [row objectAtIndex:1];
			country.name = [row objectAtIndex:2];
			country.iso2 = [row objectAtIndex:3];
			
			[list addObject:country];
		}
	}
	return [result lastObject] != nil;
}

-(NSMutableArray*)list
{
	return self->list;
}


@end

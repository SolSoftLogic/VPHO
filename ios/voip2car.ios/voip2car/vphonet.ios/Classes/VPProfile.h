//
//  VPProfile.h
//  Vphonet
//
//  Created by uncle on 25.02.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface VPCountry : NSObject {
@private
	NSString *iso2;
	NSString *name;
	NSString *code;
}

@property (nonatomic, retain) NSString *iso2;
@property (nonatomic, retain) NSString *name;
@property (nonatomic, retain) NSString *code;

-(id) initFromDatabase:(NSString*) withCode;

@end


@interface VPProfile : NSObject {
 
//@private	
	NSString *userName;
	NSString *password;
	NSString *number;
	NSString *firstName;
    NSString *lastName;
    NSString *email;
    NSString *country;
    NSString *countryName;
    NSString *countryCode;
    NSString *state;
    NSString *city;
    NSString *birthday;
    NSString *gender;
    NSString *phoneHome;
    NSString *phoneOffice;
    NSString *phoneMobile;
	NSString *phoneForward;
	NSString *photoUpdate;
	NSUInteger flags;
	NSUInteger status;
	NSString *statusMessage;
	double	lastLogin;
}

@property (nonatomic, copy) NSString *userName;
@property (nonatomic, copy) NSString *password;
@property (nonatomic, copy) NSString *number;
@property (nonatomic, copy) NSString *firstName;
@property (nonatomic, copy) NSString *lastName;
@property (nonatomic, copy) NSString *email;
@property (nonatomic, copy) NSString *country;
@property (nonatomic, copy) NSString *countryName;
@property (nonatomic, copy) NSString *countryCode;
@property (nonatomic, copy) NSString *state;
@property (nonatomic, copy) NSString *city;
@property (nonatomic, copy) NSString *birthday;
@property (nonatomic, copy) NSString *gender;
@property (nonatomic, copy) NSString *phoneHome;
@property (nonatomic, copy) NSString *phoneOffice;
@property (nonatomic, copy) NSString *phoneMobile;
@property (nonatomic, copy)	NSString *phoneVirtual;
@property (nonatomic, copy)	NSString *phoneForward;
@property (nonatomic, copy) NSString *photoUpdate;
@property (nonatomic, copy)	NSString *statusMessage;
@property (nonatomic, assign) NSUInteger flags;
@property (nonatomic, assign) NSUInteger status;
@property (nonatomic, assign) double lastLogin;


+(VPProfile*) singleton;


+ (VPProfile *) read;
-(id) initFromDatabase;

+(void) insert:(VPProfile*) profile;
+(void) update:(NSString*)login withPassword:(NSString*)pwd;
-(void) update;

+ (void) clear;

- (NSString*) statusImageName;
- (NSString*) statusName;






@end

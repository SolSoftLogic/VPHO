//
//  ChatEvent.h
//  Vphonet
//
//  Created by uncle on 05.05.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface ChatEvent : NSObject {
	
	NSString	*date;
	NSString	*from;
	NSString	*to;
	NSString	*attachment;
	NSString	*message;
	
	// un storage
	NSString	*status;
	float		progress;
}

@property (nonatomic, retain) NSString *date;
@property (nonatomic, retain) NSString *from;
@property (nonatomic, retain) NSString *to;
@property (nonatomic, retain) NSString *attachment;
@property (nonatomic, retain) NSString *message;

@property (nonatomic, retain) NSString *status;
@property (nonatomic)		  float		progress;

-(NSString*) attachmentFileForContact:(NSString*) contact;


@end


// List of messages
@interface ChatMessages : NSObject {
		NSMutableArray *list;
}

@property (nonatomic, retain) NSMutableArray *list;



-(void) load:(NSString*)contact;
-(void) save:(NSString*)contact;
-(void) add:(NSString*)from 
		 to:(NSString*)to 
 attachment:(NSString*)attachment 
	message:(NSString*)message;

@end


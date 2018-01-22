//
//  ChatsActivityViewController.h
//  Vphonet
//
//  Created by uncle on 25.04.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "VPContactList.h"


@interface ActvityContact : NSObject {
	
	VPContact*	contact;
	NSUInteger	unReadMessages;
	NSString	*lastMessage;

}

@property (nonatomic, retain)	VPContact*	contact;			
@property (nonatomic, assign)	NSUInteger	unReadMessages;
@property (nonatomic, copy)		NSString	*lastMessage;			

@end


@interface ChatsActivityViewController : UITableViewController {
	
	NSMutableArray *activities;
    
}

@end

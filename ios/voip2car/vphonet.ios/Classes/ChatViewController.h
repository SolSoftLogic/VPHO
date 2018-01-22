//
//  ChatViewController.h
//  Vphonet
//
//  Created by uncle on 05.06.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "ChatEvent.h"
#import "VPContactList.h"


@interface ChatViewController : UIViewController <UITextFieldDelegate, UITableViewDelegate, UITableViewDataSource> {
	
	VPContact		*contact;
	ChatMessages	*messages;
//    IBOutlet UIView			*chatEntryView;
    IBOutlet UITextField	*chatEntry;
    IBOutlet UITableView	*tableView;
    
}

@property (nonatomic, retain) VPContact	*contact;
@property (nonatomic, retain) ChatMessages	*messages;
//@property (nonatomic, retain) IBOutlet UIView		*chatEntryView;
@property (nonatomic, retain) IBOutlet UITextField	*chatEntry;
@property (nonatomic, retain) IBOutlet UITableView	*tableView;

- (IBAction) sendMessage:(id)sender;


@end

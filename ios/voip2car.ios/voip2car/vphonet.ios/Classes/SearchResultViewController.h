//
//  SearchResultViewController.h
//  Vphonet
//
//  Created by uncle on 01.05.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "VPContactList.h"



@interface SearchResultViewController : UITableViewController {

//@private
	VPContactList *contacts;
}

@property (nonatomic, retain) VPContactList *contacts;

- (id) init;
- (NSString *)placeHolderIcon:(bool)male;
- (void) loadContacts:(NSString *)list;


@end

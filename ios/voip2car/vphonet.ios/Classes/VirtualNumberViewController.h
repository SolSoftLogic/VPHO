//
//  VirtualNumberViewController.h
//  Vphonet
//
//  Created by uncle on 12.05.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "UITableViewTextFieldCell.h"

@interface VirtualNumberViewController : UITableViewController {
	
	UISwitch					*switchCtl;
	UITableViewTextFieldCell	*cellTextField;
}

@property (nonatomic, retain, readonly) UISwitch *switchCtl;

- (void) done:(id)sender;


@end

//
//  CallForwardViewController.h
//  IP-Phone
//
//  Created by uncle on 16.02.11.
//  Copyright 2011. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "UITableViewTextFieldCell.h"

@interface CallForwardViewController : UITableViewController {
  
	UISwitch					*switchCtl;
	UITableViewTextFieldCell	*cellTextField;
}

@property (nonatomic, retain, readonly) UISwitch *switchCtl;

@end

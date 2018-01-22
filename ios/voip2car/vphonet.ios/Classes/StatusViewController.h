//
//  StatusViewController.h
//  IP-Phone
//
//  Created by uncle on 15.02.11.
//  Copyright 2011. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "UITableViewTextFieldCell.h"


@interface StatusViewController : UITableViewController {
    
	UITableViewTextFieldCell *cellTextField;
	NSUInteger status;
}

@property(assign, nonatomic) NSUInteger status;

@end

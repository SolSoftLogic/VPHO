//
//  FullNameViewController.h
//  Vphonet
//
//  Created by uncle on 14.05.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "VPProfile.h"
#import "UITableViewTextFieldCell.h"

@interface FullNameViewController : UITableViewController<UITextFieldDelegate> {

	UITableViewTextFieldCell *cellFirstName;
	UITableViewTextFieldCell *cellLastName;
	
@private
	VPProfile *profile;
    
}

@end

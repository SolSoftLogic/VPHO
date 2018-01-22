//
//  PhonesViewController.h
//  Vphonet
//
//  Created by uncle on 14.05.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "VPProfile.h"
#import "UITableViewTextFieldCell.h"

@interface PhonesViewController : UITableViewController {
    
	
	UITableViewTextFieldCell *cellTextField;
	
@private
	VPProfile *profile;
}

@end

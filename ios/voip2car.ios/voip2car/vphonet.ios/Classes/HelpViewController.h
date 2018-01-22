//
//  HelpViewController.h
//  Vphonet
//
//  Created by uncle on 12.05.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "UITableViewFieldCell.h"
#import "UITableViewMemoCell.h"


@interface HelpViewController : UITableViewController {
    
	UITableViewFieldCell *cellFullName;
	UITableViewFieldCell *cellSubject;
	UITableViewMemoCell *cellMessage;
}

@end

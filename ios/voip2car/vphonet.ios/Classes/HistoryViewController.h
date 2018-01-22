//
//  HistoryViewController.h
//  IP-Phone
//
//  Created by uncle on 10.02.11.
//  Copyright 2011. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "DialerViewController.h"

@interface HistoryViewController : UITableViewController<UIAlertViewDelegate> {
	
	NSDateFormatter* dateFormatterDuration;
	NSDateFormatter* dateFormatterDate;
	
	UIBarButtonItem* barButtonClear;
	UIBarButtonItem* barButtonRemove;
	
	NSObject<DialerViewControllerDelegate> *delegateDialer;
	
	//IBOutlet UIToolbar* mytoolbar;
}

-(IBAction) onBarButtonClearClick:(id)sender;

@property (retain, nonatomic) NSObject<DialerViewControllerDelegate> *delegateDialer;
//@property (nonatomic, retain) IBOutlet UIToolbar* mytoolbar;

@end

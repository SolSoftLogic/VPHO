//
//  BirthdayViewController.h
//  Vphonet
//
//  Created by uncle on 14.05.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface BirthdayViewController : UIViewController {
	
	IBOutlet UIDatePicker *picker;
    
}

@property (nonatomic, retain) IBOutlet UIDatePicker *picker;

- (void) done;

@end

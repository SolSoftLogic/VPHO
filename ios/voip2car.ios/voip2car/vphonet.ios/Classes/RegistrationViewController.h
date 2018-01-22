//
//  RegistrationViewController.h
//  Vphonet
//
//  Created by uncle on 10.04.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "VPProfile.h"

@interface RegistrationViewController : UITableViewController <UITextFieldDelegate> {
    
	
	UITextField *textUserName;
	UITextField *textPassword;
	UITextField *textConfirmPassword;
	UITextField *textFirstName;
	UITextField *textLastName;
	UITextField *textEMail;
	
	VPProfile*	profile;
	
}

@property (nonatomic, retain, readonly) UITextField	*textUserName;


@end

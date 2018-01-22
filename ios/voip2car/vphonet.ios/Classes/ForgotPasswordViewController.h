//
//  ForgotPasswordViewController.h
//  Vphonet
//
//  Created by uncle on 09.04.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <MessageUI/MessageUI.h>
#import <MessageUI/MFMailComposeViewController.h>


@interface ForgotPasswordViewController : UIViewController <MFMailComposeViewControllerDelegate, UIAlertViewDelegate, UITextFieldDelegate> {
    
	UITextField *textField;
	UIButton    *buttonOk;
}

@property (nonatomic, retain) IBOutlet UITextField *textField;
@property (nonatomic, retain) IBOutlet UIButton    *buttonOk;

- (IBAction) onSendEmail: (id)sender;
- (IBAction) onSendEmail: (id)sender;
- (IBAction) onClickOk: (id)sender;

@end

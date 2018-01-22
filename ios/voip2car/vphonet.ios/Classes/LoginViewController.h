//
//  LoginViewController.h
//  Vphonet

#import <UIKit/UIKit.h>
#import "ContactsViewController.h"
#import "IFTweetLabel.h"


extern NSString *IFTweetLabelURLNotification;
extern NSString *OnRegistrationCompleteNotification;

@interface LoginViewController : UITableViewController <UITextFieldDelegate> {
	UITextField					*user;
	UITextField					*password;
	UIButton					*loginButton;
	IFTweetLabel				*labelForgotPassword;
	IBOutlet UIView				*loginProgressView;
	IBOutlet UITabBarController *tabBarController;
	BOOL						authError;
	NSTimer						*timer;
}


@property (nonatomic, retain/*, readonly*/) UITextField	*user;
@property (nonatomic, retain/*, readonly*/) UITextField	*password;

- (void) performLogin:(id)sender;


@end

//
//  iostestAppDelegate.h
//  iostest
//
//  Created by mordred on 12.12.10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface iostestAppDelegate : NSObject <UIApplicationDelegate> {
	IBOutlet UIWindow *window;
    IBOutlet UITextView *textView;
    IBOutlet UITextField *textEdit;
	NSString *stringLogEntry;
	IBOutlet UIButton *callButton;
	IBOutlet UIButton *loginButton;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;

- (id)init;
- (void)awakeFromNib;
- (void)dealloc;
- (void)applicationDidFinishLaunching:(UIApplication *)application;

- (IBAction)clickCall:(id)sender;
- (IBAction)clickHangup:(id)sender;
- (IBAction)clickAnswer:(id)sender;
- (IBAction)clickClear:(id)sender;
- (IBAction)clickQuit:(id)sender;
- (IBAction)clickSMS:(id)sender;
- (IBAction)clickChat:(id)sender;
- (IBAction)clickFile:(id)sender;
- (IBAction)textFieldReturn:(id)sender;

- (IBAction)clickLogin:(id)sender;

- (void)logEntry:(NSString*)item;
- (void)logEntryDisplay;

- (NSString*)Reason:(int)reasonCode;
- (NSString*)VPError:(int)error;
- (int)syncNotifyPrint:(void *)uparam message:(unsigned)msg parameter1:(unsigned)param1 parameter2:(unsigned)param2;
- (void)notifyPrint:(void *)uparam message:(unsigned)msg call:(unsigned)pcall parameter:(unsigned)param;
- (int)vpRegister:(const char *)user password:(const char *)password;

@end


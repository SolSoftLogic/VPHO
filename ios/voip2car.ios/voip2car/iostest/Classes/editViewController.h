//
//  editViewController.h
//  iostest
//
//  Created by mordred on 26.12.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface editViewController : UIViewController {
	UITextField    *textField;
}

@property (nonatomic, retain) IBOutlet UITextField *textField;
- (IBAction)textFieldReturn:(id)sender;
- (IBAction)backgroundTouched:(id)sender;
- (NSString*)text;
@end

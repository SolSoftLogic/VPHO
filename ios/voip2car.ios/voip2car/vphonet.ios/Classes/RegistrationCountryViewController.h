//
//  RegistrationCountryViewController.h
//  Vphonet
//
//  Created by uncle on 07.05.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import <UIKit/UIKit.h>
//#import "EditingTableViewCell.h"
#import "VPProfile.h"
#import "UITableViewFieldCell.h"
#import "CountryListViewController.h"



@interface RegistrationCountryViewController : UITableViewController <SelectCountryDelegate, UITableViewDelegate, UITextFieldDelegate> {
	
@private	
	UITableViewFieldCell		*cellCountry;
	UITableViewFieldCell		*cellPhone;
	UITextField					*hiddenTextField;
	VPProfile					*profile;
	
}

//@property (nonatomic, retain, readonly) UITextField	*hiddenTextField;

@property(nonatomic, retain) VPProfile *profile;
//@property (nonatomic, assign) IBOutlet EditingTableViewCell *editingTableViewCell;

- (void) postRegistrationInfo:(VPProfile*)_profile;

@end


@interface NSURLRequest (DummyInterface)
+ (BOOL)allowsAnyHTTPSCertificateForHost:(NSString*)host;
+ (void)setAllowsAnyHTTPSCertificate:(BOOL)allow forHost:(NSString*)host;
@end

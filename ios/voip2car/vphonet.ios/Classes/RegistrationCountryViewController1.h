//
//  RegistrationCountryViewController.h
//  Vphonet
//
//  Created by uncle on 10.04.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "EditingTableViewCell.h"
#import "RegistrationInfo.h"
#import "CountryListViewController.h"



@interface RegistrationCountryViewController : UITableViewController <SelectCountryDelegate, UITableViewDelegate, UITextFieldDelegate> {

	//UITextField	*hiddenTextField;
	
@private	
	RegistrationInfo* registrationInfo;
//	EditingTableViewCell *editingTableViewCell;

}

//@property (nonatomic, retain, readonly) UITextField	*hiddenTextField;

@property(nonatomic, retain) RegistrationInfo *registrationInfo;
@property (nonatomic, assign) IBOutlet EditingTableViewCell *editingTableViewCell;

- (void) postRegistrationInfo:(RegistrationInfo *)_info;

@end


@interface NSURLRequest (DummyInterface)
+ (BOOL)allowsAnyHTTPSCertificateForHost:(NSString*)host;
+ (void)setAllowsAnyHTTPSCertificate:(BOOL)allow forHost:(NSString*)host;
@end



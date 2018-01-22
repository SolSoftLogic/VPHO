//
//  FullProfileViewController.h
//  Vphonet
//
//  Created by uncle on 19.02.11.
//  Copyright 2011. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "VPProfile.h"
#import "UITableViewFieldCell.h"
#import "CountryListViewController.h"


@interface FullProfileViewController : UITableViewController <UITableViewDelegate, SelectCountryDelegate> {
  
	IBOutlet UIView		*tableHeaderView;    
	IBOutlet UIButton	*photoView; 
	IBOutlet UIButton	*buttonName; 
	IBOutlet UIButton	*backgroundHeader; 
	
	UITableViewFieldCell *cellPhone;
	UITableViewFieldCell *cellBirthday;
	UITableViewFieldCell *cellGender;
	UITableViewFieldCell *cellState;
	UITableViewFieldCell *cellCountry;
	
@protected
	VPProfile *profile;
}

@property (retain, nonatomic) IBOutlet UIView	*tableHeaderView;
@property (retain, nonatomic) IBOutlet UIButton	*photoView;
@property (retain, nonatomic) IBOutlet UIButton *buttonName;
@property (retain, nonatomic) IBOutlet UIButton *backgroundHeader;

- (IBAction) selectName:(id)sender;

- (void) done:(id)sender;

@end

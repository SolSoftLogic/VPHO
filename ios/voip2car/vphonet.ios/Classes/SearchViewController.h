//
//  SearchViewController.h
//  Vphonet
//
//  Created by uncle on 20.04.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

//#import "VPEngine.h"

#import <UIKit/UIKit.h>
#import "UITableViewTextFieldCell.h"
#import "UITableViewButtonCell.h"
#import "CountryListViewController.h"
#import "SearchResultViewController.h"

@interface SearchViewController : UITableViewController <SelectCountryDelegate> {

    UITableViewTextFieldCell	*cellTextField;
    UITableViewButtonCell		*cellButton;
	NSString					*selectedCountry;

}

@property (nonatomic, retain) IBOutlet UIView *cellSearchField;
@property (nonatomic, retain) NSString *selectedCountry;

- (void) search:(id)sender;
- (void) onSearchNotification:(NSNotification*)notification;

@end

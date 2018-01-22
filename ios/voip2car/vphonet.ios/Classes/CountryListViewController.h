//
//  CountryListViewController.h
//  Vphonet
//
//  Created by uncle on 10.04.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "VPCountries.h"
#import "VPProfile.h"

@protocol SelectCountryDelegate;


@interface CountryListViewController : UITableViewController {
    
@private	
	VPProfile* profile;
	id <SelectCountryDelegate> delegate;
}

@property(nonatomic, assign) id <SelectCountryDelegate> delegate;	

@property(nonatomic, retain) VPProfile *profile;

@end


@protocol SelectCountryDelegate <NSObject>
// recipe == nil on cancel
- (void)countryListViewController:(CountryListViewController *)countryListViewController didSelectCountry:(Country *)country;

@end



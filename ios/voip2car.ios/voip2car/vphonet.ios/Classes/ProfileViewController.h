//
//  ProfileViewController.h
//  IP-Phone
//
//  Created by uncle on 09.02.11.
//  Copyright 2011. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "VPProfile.h"
#import "CellMyProfileHeader.h"

@interface ProfileViewController : UITableViewController <UINavigationControllerDelegate, UIImagePickerControllerDelegate, UITextFieldDelegate, UIActionSheetDelegate> {
	
	CellMyProfileHeader	*cellProfileHeader;

	VPProfile*			profile;
	//NSUInteger			status;

}

@property (assign, nonatomic) NSUInteger status;



- (void) onChoicePhotoClick: (id)sender;
- (IBAction) settings: (id)sender;



@end

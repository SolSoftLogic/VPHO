//
//  SearchProfileViewController.h
//  Vphonet
//
//  Created by uncle on 02.05.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "VPContactList.h"
#import "CellProfileHeader.h"
#import "UITableViewTextFieldCell.h"
#import "UITableViewButtonCell.h"
#import "EditingTableViewCell.h"
#import "UITableViewFieldCell.h"

@interface SearchProfileViewController : UITableViewController <UITableViewDelegate, UINavigationControllerDelegate, UIImagePickerControllerDelegate, UITextFieldDelegate, UIActionSheetDelegate> {
	
	CellProfileHeader	*cellProfileHeader;

	UITableViewFieldCell *cellCountry;
	UITableViewFieldCell *cellState;
	UITableViewFieldCell *cellBirthday;
	UITableViewFieldCell *cellGender;
	
	UITableViewButtonCell *cellButtonAdd;
	bool					withAddContact;
	
	VPContact *contact;
    
}

@property (nonatomic, retain) VPContact *contact;
@property bool withAddContact;


- (void) onAdd:(id)sender;


//@property (nonatomic, retain) UITableViewFieldCell *cellFirstName;
//@property (nonatomic, retain) EditingTableViewCell *cellLastName;
//@property (nonatomic, retain) EditingTableViewCell *cellCountry;
//@property (nonatomic, retain) EditingTableViewCell *cellGender;

//@property (nonatomic, retain) UITableViewCell *urlCell, *usernameCell, *passwordCell;

//@property (nonatomic, retain) IBOutlet UIView *tableHeaderView;

@end

//
//  UITableViewFieldCell.h
//  Vphonet
//
//  Created by uncle on 06.05.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface UITableViewFieldCell : UITableViewCell {
	
	UILabel *label;
	UITextField *textField;
}

@property (nonatomic, retain) IBOutlet UILabel		*label;
@property (nonatomic, retain) IBOutlet UITextField	*textField;

@end

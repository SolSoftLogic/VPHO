//
//  UITableViewMemoCell.h
//  Vphonet
//
//  Created by uncle on 12.05.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface UITableViewMemoCell : UITableViewCell {

    
	UILabel *label;
	UITextView *textField;

}

@property (nonatomic, retain) IBOutlet UILabel		*label;
@property (nonatomic, retain) IBOutlet UITextView	*textField;


@end

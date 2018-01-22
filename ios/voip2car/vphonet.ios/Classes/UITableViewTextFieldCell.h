//
//  UITableViewTextFieldCell.h
//  Vphonet
//
//  Created by uncle on 30.04.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import <Foundation/Foundation.h>



@interface UITableViewTextFieldCell : UITableViewCell {
	
	UITextField *textField;
    
}

@property (nonatomic, retain) IBOutlet UITextField *textField;


@end

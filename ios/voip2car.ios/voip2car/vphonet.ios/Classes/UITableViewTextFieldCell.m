//
//  UITableViewTextFieldCell.m
//  Vphonet
//
//  Created by uncle on 30.04.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import "UITableViewTextFieldCell.h"


@implementation UITableViewTextFieldCell 

@synthesize textField;


- (id)initWithStyle:(UITableViewCellStyle)style reuseIdentifier:(NSString *)reuseIdentifier {
    if ((self = [super initWithStyle:style reuseIdentifier:reuseIdentifier])) {
        // Initialization code
    }
    return self;
}

- (void)setSelected:(BOOL)selected animated:(BOOL)animated {
    [super setSelected:selected animated:animated];
}

- (void)dealloc {
    self.textField = nil;
    [super dealloc];
}



@end

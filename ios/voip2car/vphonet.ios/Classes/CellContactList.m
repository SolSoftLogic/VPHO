//
//  CellContactList.m
//  Vphonet
//
//  Created by uncle on 24.05.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import "CellContactList.h"


@implementation CellContactList

@synthesize		imagePhoto;
@synthesize		imageStatus;
@synthesize		label;

- (id)initWithStyle:(UITableViewCellStyle)style reuseIdentifier:(NSString *)reuseIdentifier
{
    self = [super initWithStyle:style reuseIdentifier:reuseIdentifier];
    if (self) {
        // Initialization code
    }
    return self;
}

- (void)setSelected:(BOOL)selected animated:(BOOL)animated
{
    [super setSelected:selected animated:animated];

    // Configure the view for the selected state
}

- (void)dealloc
{
    [super dealloc];
}

@end

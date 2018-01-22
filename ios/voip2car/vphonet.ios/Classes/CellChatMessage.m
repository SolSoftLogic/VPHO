//
//  CellChatMessage.m
//  Vphonet
//
//  Created by uncle on 06.06.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import "CellChatMessage.h"


@implementation CellChatMessage


@synthesize labelName;
@synthesize labelDate;
@synthesize labelStatus;
@synthesize labelMessage;
@synthesize background;
@synthesize imageLeftBeak;
@synthesize imageRightBeak;

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
}

- (void)dealloc
{
    [super dealloc];
}

@end

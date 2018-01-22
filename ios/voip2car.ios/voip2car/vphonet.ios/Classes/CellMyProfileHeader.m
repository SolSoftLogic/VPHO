//
//  CellMyProfileHeader.m
//  Vphonet
//
//  Created by uncle on 26.05.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import "CellMyProfileHeader.h"


@implementation CellMyProfileHeader

@synthesize tableHeaderView;
@synthesize photoView;
@synthesize labelName;
@synthesize labelStatus;
@synthesize imageStatus;
@synthesize buttonAdd;

@synthesize backgroundName;
@synthesize backgroundHeader;
@synthesize backgroundStatus;



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
   	[tableHeaderView release];
	[photoView release];
	[labelName release];
	[labelStatus release];
	[imageStatus release];

	[super dealloc];
}

@end

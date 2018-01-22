//
//  CellProfileHeader.m
//  Vphonet
//
//  Created by uncle on 03.05.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import "UICustomButton.h"
#import "CellProfileHeader.h"
//#import "VphonetAppDelegate.h"


@implementation CellProfileHeader

@synthesize labelName;
@synthesize labelEMail;
@synthesize buttonPhoto;
@synthesize statusImage;
@synthesize statusLabel;

@synthesize backgroundName;
@synthesize backgroundHeader;

- (id)initWithStyle:(UITableViewCellStyle)style reuseIdentifier:(NSString *)reuseIdentifier
{
    self = [super initWithStyle:style reuseIdentifier:reuseIdentifier];
    if (self) {
        // Initialization code
		UIView *backView = [[[UIView alloc] initWithFrame:CGRectZero] autorelease];
		backView.backgroundColor = [UIColor clearColor];
		
		[UICustomButton setButtonYellowBorder: buttonPhoto];
		
		

		
		self.backgroundView = backView;
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
	[labelName release];
	[labelEMail release];
	[buttonPhoto release];
	[statusImage release];
	[backgroundName release];
	[backgroundHeader release];
    [super dealloc];
}

@end

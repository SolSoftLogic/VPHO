//
//  UITableViewButtonCell.m
//  Vphonet
//
//  Created by uncle on 30.04.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import "UITableViewButtonCell.h"


@implementation UITableViewButtonCell

@synthesize button;

- (id)initWithStyle:(UITableViewCellStyle)style reuseIdentifier:(NSString *)reuseIdentifier {
    if ((self = [super initWithStyle:style reuseIdentifier:reuseIdentifier])) {
		
		UIView *backView = [[[UIView alloc] initWithFrame:CGRectZero] autorelease];
		backView.backgroundColor = [UIColor clearColor];
		
		self.backgroundView = backView;

        // Initialization code
    }
    return self;
}


- (void)setSelected:(BOOL)selected animated:(BOOL)animated {
    [super setSelected:selected animated:animated];
}

- (void)dealloc {
//	if (button) {
//		[button release];
//		self.button = nil;
//	}
    [super dealloc];
}


@end

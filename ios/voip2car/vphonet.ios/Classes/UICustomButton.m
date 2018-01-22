//
//  UICustomButton.m
//  Vphonet
//
//  Created by uncle on 14.06.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import "UICustomButton.h"


@implementation UICustomButton

+ (void) setButtonYellowBorder:(UIButton *)button
{
	[[button layer] setCornerRadius:12.0f];
	[[button layer] setMasksToBounds:YES];
	[[button layer] setBorderColor:[[UIColor paleYellowColor] CGColor]];
	[[button layer] setBackgroundColor:[[UIColor whiteColor] CGColor]];
	[[button layer] setBorderWidth:1.0]; 
}




@end

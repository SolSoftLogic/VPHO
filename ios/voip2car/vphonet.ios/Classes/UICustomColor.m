//
//  UICustomColor.m
//  Vphonet
//
//  Created by uncle on 11.06.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import "UICustomColor.h"


@implementation UIColor (Extensions)

+ (UIColor *)paleYellowColor {
	return [UIColor colorWithRed:240.0/255.0 green:191.0/255.0 blue:27.0/255.0 alpha:1.0];
}

+ (UIColor *)paleYellowLiteColor {
	return [UIColor colorWithRed:255.0/255.0 green:247.0/255.0 blue:187.0/255.0 alpha:0.4];
}


+ (UIColor *)titleColor {
	return [UIColor colorWithRed:0.25 green:0.25 blue:0.25 alpha:0.5];
}


@end


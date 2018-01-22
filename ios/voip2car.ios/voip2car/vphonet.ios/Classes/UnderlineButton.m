//
//  UnderlineButton.m
//  Vphonet
//
//  Created by uncle on 09.04.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import "UnderlineButton.h"


@implementation UnderlineButton 

- (id)initWithFrame:(CGRect)frame {
	if ((self = [super initWithFrame:frame])) {
		// Initialization code
    }
	return self;
}

/*
- (void)drawRect:(CGRect)rect {
    CGContextRef ctx = UIGraphicsGetCurrentContext();
    CGContextSetRGBStrokeColor(ctx, 207.0f/255.0f, 91.0f/255.0f, 44.0f/255.0f, 1.0f); // RGBA
    CGContextSetLineWidth(ctx, 1.0f);
	
    CGContextMoveToPoint(ctx, 0, self.bounds.size.height - 1);
    CGContextAddLineToPoint(ctx, self.bounds.size.width, self.bounds.size.height - 1);
	
    CGContextStrokePath(ctx);
	
    [super drawRect:rect];  
}
 
 CGContextRef context = UIGraphicsGetCurrentContext();
 
 CGContextSetStrokeColorWithColor(context, self.currentTitleColor.CGColor);
 
 // Draw them with a 1.0 stroke width.
 CGContextSetLineWidth(context, 1.0);
 
 CGFloat baseline = rect.size.height + self.titleLabel.font.descender + 2;
 
 // Draw a single line from left to right
 CGContextMoveToPoint(context, 0, baseline);
 CGContextAddLineToPoint(context, rect.size.width, baseline);
 CGContextStrokePath(context); 
*/

- (void)drawRect:(CGRect)rect {
	[super drawRect:rect];
	CGContextRef context = UIGraphicsGetCurrentContext();
	
	if (self.highlighted) {
		CGContextSetStrokeColorWithColor(context, [UIColor cyanColor].CGColor);
	}
	else
	{
		CGContextSetStrokeColorWithColor(context, self.currentTitleColor.CGColor);
	}
//	CGContextSetRGBStrokeColor(context, 62.0/255.0, 62.0/255.0, 62.0/255.0, 1.0);
	
	// Draw them with a 1.0 stroke width.
	CGContextSetLineWidth(context, 1.0f);
	CGFloat baseline = rect.size.height + self.titleLabel.font.descender + 2;
	
	// Draw a single line from left to right
	CGContextMoveToPoint(context, 0, baseline);
	CGContextAddLineToPoint(context, rect.size.width, baseline/*rect.size.height*/);
	CGContextStrokePath(context);
}

- (void)dealloc {
	[super dealloc];
}

@end

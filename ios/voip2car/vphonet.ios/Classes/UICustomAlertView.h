//
//  UICustomAlertView.h
//  Vphonet
//
//  Created by uncle on 04.06.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>


@interface UICustomAlertView : UIAlertView {
    
}


- (void)show;

+ (void) setBackgroundColor:(UIColor *) background 
			withStrokeColor:(UIColor *) stroke;

@end

/*
UIAlertView *theAlert = [[[UIAlertView alloc] initWithTitle:@"Atention"
													message: @"YOUR MESSAGE HERE", nil)
												   delegate:nil cancelButtonTitle:@"OK" otherButtonTitles:nil] autorelease];

[theAlert show];

UILabel *theTitle = [theAlert valueForKey:@"_titleLabel"];
[theTitle setTextColor:[UIColor redColor]];

UILabel *theBody = [theAlert valueForKey:@"_bodyTextLabel"];
[theBody setTextColor:[UIColor blueColor]];

UIImage *theImage = [UIImage imageNamed:@"Background.png"];    
theImage = [theImage stretchableImageWithLeftCapWidth:16 topCapHeight:16];
CGSize theSize = [theAlert frame].size;

UIGraphicsBeginImageContext(theSize);    
[theImage drawInRect:CGRectMake(0, 0, theSize.width, theSize.height)];    
theImage = UIGraphicsGetImageFromCurrentImageContext();    
UIGraphicsEndImageContext();

[[theAlert layer] setContents:[theImage CGImage]];
*/
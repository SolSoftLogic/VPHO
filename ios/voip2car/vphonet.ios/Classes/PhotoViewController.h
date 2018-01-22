//
//  PhotoViewController.h
//  Vphonet
//
//  Created by uncle on 07.06.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface PhotoViewController : UIViewController {
@private
	NSString	*fileName;
	UIImageView *imageView;
}

@property(nonatomic, retain) NSString	 *fileName;
@property(nonatomic, retain) UIImageView *imageView;


@end

//
//  PhotoViewController.m
//  Vphonet
//
//  Created by uncle on 07.06.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import "PhotoViewController.h"

@implementation PhotoViewController

@synthesize fileName;
@synthesize imageView;

- (void)loadView {
	self.title = @"Photo";
	
    imageView = [[UIImageView alloc] initWithFrame:[UIScreen mainScreen].applicationFrame];
    imageView.autoresizingMask = UIViewAutoresizingFlexibleHeight | UIViewAutoresizingFlexibleWidth;
    imageView.contentMode = UIViewContentModeScaleAspectFit;
    imageView.backgroundColor = [UIColor blackColor];
    
    self.view = imageView;
}


- (void)viewWillAppear:(BOOL)animated {
	imageView.image = [UIImage imageWithContentsOfFile:fileName];
}


- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    return (interfaceOrientation != UIInterfaceOrientationPortraitUpsideDown);
}


- (void)dealloc {
    [imageView release];
    [fileName release];
    [super dealloc];
}


@end

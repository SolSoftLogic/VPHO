//
//  CellChatAttachment.m
//  Vphonet
//
//  Created by uncle on 07.06.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import "CellChatAttachment.h"
#import "PhotoViewController.h"

@implementation CellChatAttachment

@synthesize imagePhoto;
@synthesize buttonOpen;
@synthesize buttonForward;
@synthesize progressBar;

@synthesize navigationController;
@synthesize fileName;

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
    [super dealloc];
}

- (IBAction) onOpenClick: (id)sender
{
		// If in editing state, then display an image picker; if not, create and push a photo view controller.
//		if (self.editing) {
//			UIImagePickerController *imagePicker = [[UIImagePickerController alloc] init];
//			imagePicker.delegate = self;
//			[self presentModalViewController:imagePicker animated:YES];
//			[imagePicker release];
//		} else {	
	PhotoViewController *photoViewController = [[PhotoViewController alloc] init];
	photoViewController.hidesBottomBarWhenPushed = YES;
	photoViewController.fileName = fileName;
	[self.navigationController pushViewController:photoViewController animated:YES];
	[photoViewController release];
//		}
//	}
	
}

- (IBAction) onForwardClick: (id)sender
{
	
}


@end

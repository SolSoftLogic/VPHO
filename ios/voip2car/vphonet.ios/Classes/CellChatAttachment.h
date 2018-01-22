//
//  CellChatAttachment.h
//  Vphonet
//
//  Created by uncle on 07.06.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "CellChatMessage.h"

@interface CellChatAttachment : CellChatMessage {
	IBOutlet UIButton		*buttonOpen;	
	IBOutlet UIButton		*buttonForward;	
	IBOutlet UIImageView	*imagePhoto;
	IBOutlet UIProgressView	*progressBar;
	
	UINavigationController	*navigationController;
	NSString				*fileName;
	
}

@property (nonatomic, retain) IBOutlet UIButton			*buttonOpen;	
@property (nonatomic, retain) IBOutlet UIButton			*buttonForward;	
@property (nonatomic, retain) IBOutlet UIImageView		*imagePhoto;
@property (nonatomic, retain) IBOutlet UIProgressView	*progressBar;

@property (nonatomic, retain) UINavigationController	*navigationController;
@property (nonatomic, retain) NSString					*fileName;


- (IBAction) onOpenClick: (id)sender;
- (IBAction) onForwardClick: (id)sender;

@end

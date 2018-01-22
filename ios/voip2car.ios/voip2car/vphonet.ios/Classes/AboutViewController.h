//
//  AboutViewController.h
//  Vphonet
//
//  Created by uncle on 13.05.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <MessageUI/MessageUI.h>
#import <MessageUI/MFMailComposeViewController.h>


@interface AboutViewController : UIViewController <MFMailComposeViewControllerDelegate> {
    
}

- (IBAction) onSendEmail: (id)sender;



@end

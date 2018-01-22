//
//  GenderViewController.h
//  Vphonet
//
//  Created by uncle on 14.05.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface GenderViewController : UITableViewController <UITableViewDelegate> {

@private	
	NSUInteger gender;

}


@property(assign, nonatomic) NSUInteger gender;

- (void) done:(id)sender;


@end

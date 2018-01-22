//
//  GetTextController.h
//  iostest
//
//  Created by Vladimir on 1/17/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@protocol GetTextDelegate

- (void) textPicker:(id)sender finishedWithText:(NSDictionary*)result;

@end


@interface GetTextController : UIViewController {
	NSArray* fields;
	id<GetTextDelegate> delegate;
}

@property (nonatomic, retain) NSArray* fields;
@property (nonatomic, assign) id<GetTextDelegate> delegate;

- (IBAction) okClick:(id)sender;
- (IBAction) cancelClick:(id)sender;

@end

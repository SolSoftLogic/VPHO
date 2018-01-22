//
//  GetTextController.m
//  iostest
//
//  Created by Vladimir on 1/17/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "GetTextController.h"


@implementation GetTextController

@synthesize fields, delegate;

- (IBAction) okClick:(id)sender{
	NSMutableDictionary *result = [NSMutableDictionary dictionary];
	
	for (int i = 0; i < [fields count]; ++i) {
		UITextField* txt = (UITextField*)[self.view viewWithTag:10+i];
		[result setObject:txt.text forKey:[fields objectAtIndex:i]];
	}
	
	[self.view removeFromSuperview];
	[delegate textPicker:self finishedWithText:[[result retain] autorelease]];
}

- (IBAction) cancelClick:(id)sender{
	[self.view removeFromSuperview];
}


/*
// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
}
*/

- (void) viewWillAppear:(BOOL)animated{
	[super viewWillAppear: animated];
	int y = 60.0f;
	int i = 0;
	for (NSString* field in fields) {
		UITextField *txt = [[UITextField alloc] initWithFrame:CGRectMake(40.0f, y, 240.0f, 30.0f)];
		txt.autocapitalizationType = UITextAutocapitalizationTypeNone;
		txt.borderStyle = UITextBorderStyleRoundedRect;
		txt.placeholder = field;
		[self.view addSubview: txt];
		txt.tag = 10 + i;
		++i;
		y+=40.0f;
				
#if TARGET_IPHONE_SIMULATOR
		if ([field isEqualToString:@"Login"])
			txt.text = @"voip2cartest";
		if ([field isEqualToString:@"Password"])
			txt.text = @"voip2car";
//		[self vpRegister:"voip2cartest" password:"voip2car"];
#else
		if ([field isEqualToString:@"Login"])
			txt.text = @"VIC_TEST2";
		if ([field isEqualToString:@"Password"])
			txt.text = @"klaz197";
//		[self vpRegister:"VIC_TEST2" password:"klaz197"];
#endif
		
	}
}



- (void)dealloc {
	[fields release];
    [super dealloc];
}


@end

//
//  BirthdayViewController.m
//  Vphonet
//
//  Created by uncle on 14.05.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import "VphonetAppDelegate.h"
#import "VPProfile.h"
#import "BirthdayViewController.h"


@implementation BirthdayViewController

@synthesize picker;

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
    }
    return self;
}

- (void)dealloc
{
    [super dealloc];
}

- (void)didReceiveMemoryWarning
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
}

#pragma mark - View lifecycle

- (void)viewDidLoad
{
    [super viewDidLoad];
	
	[self setCustomBackground:@"background.png"];
//	[self.tableView setSeparatorColor:[UIColor paleYellowColor]];
	[self setTitle:NSLocalizedString(@"Birthday", @"")];
	
	self.navigationItem.rightBarButtonItem  = [[[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"Done", @"")
																				style: UIBarButtonItemStyleDone
																			   target: self 
																			   action:@selector(done:)] autorelease];
	
	NSDateFormatter *dateFormatterSection = [[[NSDateFormatter alloc] init] autorelease];
	[dateFormatterSection setDateStyle:NSDateFormatterNoStyle];
	[dateFormatterSection setDateFormat:@"yyyyMMdd"];
	
	VPProfile *profile = [VPProfile singleton];
	
	if (([profile.birthday length] != 0) && (![profile.birthday isEqualToString:@"00000000"])) {
		self.picker.date = [dateFormatterSection dateFromString:profile.birthday];
	} else {
		self.picker.date = [dateFormatterSection dateFromString:@"19700101"];
	}
	
//	NSDateFormatter *dateFormatterSection = [[[NSDateFormatter alloc] init] autorelease];
//	[dateFormatterSection setDateStyle:NSDateFormatterNoStyle];
//	[dateFormatterSection setDateFormat:@"yyyy-MM-dd"];

	

    // Do any additional setup after loading the view from its nib.
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}

- (void) done:(id)sender
{
	VPProfile *profile = [VPProfile singleton];

	NSDateFormatter *dateFormatterSection = [[[NSDateFormatter alloc] init] autorelease];
	[dateFormatterSection setDateStyle:NSDateFormatterNoStyle];
	[dateFormatterSection setDateFormat:@"yyyyMMdd"];

	profile.birthday = [dateFormatterSection stringFromDate:picker.date];
	
	NSLog(@"Birthday %@", profile.birthday);
	[self.navigationController popViewControllerAnimated:YES];
}



@end

//
//  DialerViewController.m
//  IP-Phone
//
//  Created by uncle on 09.02.11.
//  Copyright 2011. All rights reserved.
//

#import "VPSound.h"
#import "VphonetAppDelegate.h"
#import "CallViewController.h"
#import "DialerViewController.h"

@implementation DialerViewController

@synthesize buttonZero;
@synthesize buttonOne;
@synthesize buttonTwo;
@synthesize buttonThree;
@synthesize buttonFour;
@synthesize buttonFive;
@synthesize buttonSix;
@synthesize buttonSeven;
@synthesize buttonEight;
@synthesize buttonNine;
@synthesize buttonStar;
@synthesize buttonSharp;


@synthesize textFieldAddress;
@synthesize buttonPickContact;

@synthesize buttonCall;
@synthesize buttonCountries;
@synthesize buttonDel;

/*
 // The designated initializer.  Override if you create the controller programmatically and want to perform customization that is not appropriate for viewDidLoad.
 - (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
 if ((self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil])) {
 // Custom initialization
 }
 return self;
 }
 */


// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
	
	[super viewDidLoad];
	
	self.textFieldAddress.text = @"";
	[self.buttonOne.layer setBorderWidth:1.0]; 
	[self.buttonOne.layer setBorderColor: [[UIColor colorWithWhite:0.5 alpha:0.7] CGColor]];
	[self.buttonTwo.layer setBorderWidth:1.0];
	[self.buttonTwo.layer setBorderColor: [[UIColor colorWithWhite:0.5 alpha:0.7] CGColor]];
	[self.buttonZero.layer setBorderWidth:1.0];
	[self.buttonZero.layer setBorderColor: [[UIColor colorWithWhite:0.5 alpha:0.7] CGColor]];
	[self.buttonThree.layer setBorderWidth:1.0];
	[self.buttonThree.layer setBorderColor: [[UIColor colorWithWhite:0.5 alpha:0.7] CGColor]];
	[self.buttonFour.layer setBorderWidth:1.0];
	[self.buttonFour.layer setBorderColor: [[UIColor colorWithWhite:0.5 alpha:0.7] CGColor]];
	[self.buttonFive.layer setBorderWidth:1.0];
	[self.buttonFive.layer setBorderColor: [[UIColor colorWithWhite:0.5 alpha:0.7] CGColor]];
	[self.buttonSix.layer setBorderWidth:1.0];
	[self.buttonSix.layer setBorderColor: [[UIColor colorWithWhite:0.5 alpha:0.7] CGColor]];
	[self.buttonSeven.layer setBorderWidth:1.0];
	[self.buttonSeven.layer setBorderColor: [[UIColor colorWithWhite:0.5 alpha:0.7] CGColor]];
	[self.buttonEight.layer setBorderWidth:1.0];
	[self.buttonEight.layer setBorderColor: [[UIColor colorWithWhite:0.5 alpha:0.7] CGColor]];
	[self.buttonNine.layer setBorderWidth:1.0];
	[self.buttonNine.layer setBorderColor: [[UIColor colorWithWhite:0.5 alpha:0.7] CGColor]];
	[self.buttonStar.layer setBorderWidth:1.0];
	[self.buttonStar.layer setBorderColor: [[UIColor colorWithWhite:0.5 alpha:0.7] CGColor]];
	[self.buttonSharp.layer setBorderWidth:1.0];
	[self.buttonSharp.layer setBorderColor: [[UIColor colorWithWhite:0.5 alpha:0.7] CGColor]];

	[self.buttonDel.layer setBorderWidth:1.0];
	[self.buttonDel.layer setBorderColor: [[UIColor colorWithWhite:0.5 alpha:0.7] CGColor]];
	[self.buttonCall.layer setBorderWidth:1.0];
	[self.buttonCall.layer setBorderColor: [[UIColor colorWithWhite:0.5 alpha:0.7] CGColor]];
	[self.buttonCountries.layer setBorderWidth:1.0];
	[self.buttonCountries.layer setBorderColor: [[UIColor colorWithWhite:0.5 alpha:0.7] CGColor]];

}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
    
//	VphonetAppDelegate *appDelegate = (VphonetAppDelegate *)[[UIApplication sharedApplication] delegate];
//    [appDelegate.tabBarController.navigationController setNavigationBarHidden:YES animated:NO];

}

/*
 // Override to allow orientations other than the default portrait orientation.
 - (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
 // Return YES for supported orientations
 return (interfaceOrientation == UIInterfaceOrientationPortrait);
 }
 */

- (void)didReceiveMemoryWarning {
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}

#pragma mark UITextFieldDelegate

- (BOOL)textFieldShouldReturn:(UITextField *)textField {
	
	[textField resignFirstResponder];
	
	return YES;
	
}

- (IBAction) onKeyboardClick: (id)sender{
	NSInteger tag = ((UIButton*)sender).tag;
	

	if ([self.textFieldAddress.text length] > 16)
		return;
	
	switch (tag) {
		case 10:
			self.textFieldAddress.text = [self.textFieldAddress.text stringByAppendingString:@"*"];
			break;
		case 11:
			self.textFieldAddress.text = [self.textFieldAddress.text stringByAppendingString:@"#"];
			break;
		default:
			
			self.textFieldAddress.text = [self.textFieldAddress.text stringByAppendingString:[NSString stringWithFormat:@"%d", tag]];
			break;
	}
	
	
	switch (tag) {
		case 0: [[VPSound singleton] playDTMF:'0'];
				break;
		case 1: [[VPSound singleton] playDTMF:'1'];
			break;
		case 2: [[VPSound singleton] playDTMF:'2'];
			break;
		case 3: [[VPSound singleton] playDTMF:'3'];
			break;
		case 4: [[VPSound singleton] playDTMF:'4'];
			break;
		case 5: [[VPSound singleton] playDTMF:'5'];
			break;
		case 6: [[VPSound singleton] playDTMF:'6'];
			break;
		case 7: [[VPSound singleton] playDTMF:'7'];
			break;
		case 8: [[VPSound singleton] playDTMF:'8'];
			break;
		case 9: [[VPSound singleton] playDTMF:'9'];
			break;
		case 10: [[VPSound singleton] playDTMF:'*'];
			break;
		case 11: [[VPSound singleton] playDTMF:'#'];
			break;
	}
	 

	
	
	
}

- (IBAction) onPickContactClick: (id)sender{
}

- (IBAction) onAVCallClick: (id)sender{
	if([self.textFieldAddress.text length] ==0){
		return;
	}
	
	/*
	VPCALL call = [[VPEngine sharedInstance] makeCall:self.textFieldAddress.text bearer:BC_VOICE];
    
    CallViewController *controller = [[CallViewController alloc] initWithCall:call];
	
    [self.navigationController pushViewController:controller animated:YES];
    [controller release]; 
	*/ 
}

- (IBAction) onDelClick: (id)sender{
	NSString* val = self.textFieldAddress.text;
	if([val length] >0){
		self.textFieldAddress.text = [val substringToIndex:([val length]-1)];
	}
}


// DialerViewControllerDelegate
-(void)setAddress:(NSString*)address{
	if(address){
		self.textFieldAddress.text = address;
	}
}

- (void)dealloc {
	[buttonZero dealloc];
	[buttonOne dealloc];
	[buttonTwo dealloc];
	[buttonThree dealloc];
	[buttonFour dealloc];
	[buttonFive dealloc];
	[buttonSix dealloc];
	[buttonSeven dealloc];
	[buttonEight dealloc];
	[buttonNine dealloc];
	[buttonStar dealloc];
	[buttonSharp dealloc];
	
	[textFieldAddress dealloc];
	[buttonPickContact dealloc];
	
	[buttonCall dealloc];
	[buttonCountries dealloc];
	[buttonDel dealloc];
	
    [super dealloc];
}


@end




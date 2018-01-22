//
//  ForgotPasswordViewController.m
//  Vphonet
//
//  Created by uncle on 09.04.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import "VphonetAppDelegate.h"
#import "ForgotPasswordViewController.h"


@implementation ForgotPasswordViewController

@synthesize textField;
@synthesize buttonOk;


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
	[textField release];
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
	
//	self.tableView.backgroundColor = [UIColor clearColor];
	
	self.title = NSLocalizedString(@"Forgot Password", @"");
	
	UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"Back", @"") style:UIBarButtonItemStylePlain target:nil action:nil];
	self.navigationItem.backBarButtonItem = backButton;
	[backButton release];
}

- (void)viewWillAppear:(BOOL)animated
{
	UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"Back", @"") style:UIBarButtonItemStylePlain target:nil action:nil];
	self.navigationItem.backBarButtonItem = backButton;
	[backButton release];

    [super viewWillAppear:animated];
}


- (IBAction) onClickOk: (id)sender;
{
	
	if ([textField.text length] == 0) 
	{
		UIAlertView *alert = [[UIAlertView alloc] 
							  initWithTitle:NSLocalizedString(@"Error", @"")
							  message:NSLocalizedString(@"Please, enter User Name or Vphone number", @"")
							  delegate:self cancelButtonTitle:nil 
							  otherButtonTitles:NSLocalizedString(@"OK", @""), nil]; 
		
		[alert show];
		[alert release];
		return;
	}
		
	NSURL *url = [NSURL URLWithString:[NSString stringWithFormat:@"https://vphonet.com/myaccount/forgot_password/vname/%@", [textField.text stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]]];
	NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:url 
														cachePolicy:NSURLRequestReloadIgnoringLocalCacheData 
														timeoutInterval:30.0];
	[request setHTTPMethod:@"GET"];
	
	[[UIApplication sharedApplication] setNetworkActivityIndicatorVisible:YES]; 

	NSLog(@"HTTP: host=%@ path=%@", [url host],[url path]);

	
	[NSURLRequest setAllowsAnyHTTPSCertificate:YES forHost:[url host]];

//	[NSURLRequest setAllowsAnyHTTPSCertificate:YES forHost:[url host]];
		
	NSHTTPURLResponse * returnResponse = nil; 
	NSError * returnError = nil;
		
	NSData *returnData = [NSURLConnection sendSynchronousRequest:request
												returningResponse:&returnResponse
															error:&returnError]; 
	int statusCode = returnResponse.statusCode;
		
	NSString	*str = [[[NSString alloc] initWithData:returnData encoding:NSUTF8StringEncoding] autorelease]; 
		
	NSLog(@"HTTP Return: %@. Status code: %d. Error:%@", str, statusCode, [returnError localizedDescription]);

	if (statusCode != 200)
	{
		UIAlertView *alert = [[UIAlertView alloc] 
							  initWithTitle:NSLocalizedString(@"Failed request", @"")
							  message:[NSString stringWithFormat:@"%@\n%@", str, [returnError localizedDescription]]
							  delegate:self cancelButtonTitle:nil 
							  otherButtonTitles:NSLocalizedString(@"OK", @""), nil]; 
		
		[alert show];
		[alert release];
		return;
	} else if ([str isEqualToString:@"rc=1"])
	{
		NSLog(@"User not found.");
		UIAlertView *alert = [[UIAlertView alloc] 
							  initWithTitle:NSLocalizedString(@"Forgot password", @"")
							  message:NSLocalizedString(@"User name not found.", @"")
							  delegate:self cancelButtonTitle:nil 
							  otherButtonTitles:NSLocalizedString(@"OK", @""), nil]; 
		
		[alert show];
		[alert release];
		return;
	}

//	NSArray *result = [str componentsSeparatedByString:@"&"];
//	NSString* rc = nil;
//	NSString* email = nil;
	
	if ([str hasPrefix:@"rc=0"])
	{
		NSLog(@"Send email ");
		UIAlertView *alert = [[UIAlertView alloc] 
							  initWithTitle:NSLocalizedString(@"Forgot password", @"")
							  message:NSLocalizedString(@"Send email successful", @"")
							  delegate:self cancelButtonTitle:nil 
							  otherButtonTitles:NSLocalizedString(@"OK", @""), nil]; 
		
		[alert show];
		[alert release];
		[self.navigationController popToRootViewControllerAnimated:YES];// popToRootViewControllerAnimated:YES];
		//			return;
	}	
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

#pragma mark - Events

-(void)displayComposerSheet {
	// Displays an email composition interface inside the application. Populates all the Mail fields. 
	
	MFMailComposeViewController *picker = [[MFMailComposeViewController alloc] init];
	picker.mailComposeDelegate = self;
	
	[picker setSubject:@"Forgot Password"];
	
	
	// Set up recipients
	NSArray *toRecipients = [NSArray arrayWithObject:@"info@vphonet.com"]; 
//	NSArray *ccRecipients = [NSArray arrayWithObjects:@"second@example.com", @"third@example.com", nil]; 
//	NSArray *bccRecipients = [NSArray arrayWithObject:@"fourth@example.com"]; 
	
	[picker setToRecipients:toRecipients];
//	[picker setCcRecipients:ccRecipients];  
//	[picker setBccRecipients:bccRecipients];
	
	// Attach an image to the email
//	NSString *path = [[NSBundle mainBundle] pathForResource:@"rainy" ofType:@"png"];
//	NSData *myData = [NSData dataWithContentsOfFile:path];
//	[picker addAttachmentData:myData mimeType:@"image/png" fileName:@"rainy"];
	
	// Fill out the email body text
	NSString *emailBody = @"Please send my Password!";
	[picker setMessageBody:emailBody isHTML:NO];
	
	[self presentModalViewController:picker animated:YES];
	[picker release];
}

-(void)launchMailAppOnDevice {
	
	// Launches the Mail application on the device.
	NSString *recipients = @"mailto:info@vphonet.com&subject=Forgot password!";
//	NSString *recipients = @"mailto:info@vphonet.com&subject=Forgot password!";
	NSString *body = @"&body=Please send my Password!";
	
	NSString *email = [NSString stringWithFormat:@"%@%@", recipients, body];
	email = [email stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
	
	[[UIApplication sharedApplication] openURL:[NSURL URLWithString:email]];
}

- (IBAction) onSendEmail: (id)sender
{
	
	Class mailClass = (NSClassFromString(@"MFMailComposeViewController"));
	if (mailClass != nil)
	{
        // We must always check whether the current device is configured for sending emails
        if ([mailClass canSendMail])
        {
			[self displayComposerSheet];
        }
        else
        {
			[self launchMailAppOnDevice];
        }
	}
	else
	{
        [self launchMailAppOnDevice];
	}
	
	//	if (!NSClassFromString(@"MFMailComposeViewController")){
	// Put code that handles OS 2.x version
	//		return;
	//	}
/*	
	[self dismissModalViewControllerAnimated:YES];
	// some more code here
	[self performSelector:@selector(sendEmail) withObject:nil afterDelay:0.45];
	
	
	if (![MFMailComposeViewController canSendMail])
	{
		UIAlertView *alert = [[UIAlertView alloc] initWithTitle:NSLocalizedString(@"Email", @"") 
														message:NSLocalizedString(@"Failed to send E-mail.Please set an E-mail account and try again", @"")
													   delegate:self cancelButtonTitle:NSLocalizedString(@"OK", @"") otherButtonTitles: nil];
		[alert show];
		[alert release];		
	} else
	{
		
		MFMailComposeViewController *picker = [[MFMailComposeViewController alloc] init];
		
		picker.mailComposeDelegate = self;
		
		[picker setSubject:@"Forgot password"];
		
		// Set up recipients
		NSArray *toRecipients = [NSArray arrayWithObject:@"info@vphonet.com"]; 
		
		[picker setToRecipients:toRecipients];
		[self presentModalViewController:picker animated:YES];
		[picker release];
	}
*/	
}




-(IBAction)onDismissKeyboard: (id)sender {
	[sender resignFirstResponder];
}


- (BOOL)textFieldShouldReturn:(UITextField *)textField {
	//We resign the first responder
	//This closes the keyboard
	[textField resignFirstResponder];
	
	//Do what ever other fanciness you want
	//TODO: fanciness
	
	//Return YES to confirm the UITextField is returning
	return YES;
}


// Dismisses the email composition interface when users tap Cancel or Send. Proceeds to update the message field with the result of the operation.
- (void)mailComposeController:(MFMailComposeViewController*)controller didFinishWithResult:(MFMailComposeResult)result error:(NSError*)error 
{	
	
	NSString *message = NULL;
	// Notifies users about errors associated with the interface
	switch (result)
	{
		case MFMailComposeResultCancelled:
			//message = [NSString stringWithString:@"Result: canceled"];
			break;
		case MFMailComposeResultSaved:
			//message = [NSString stringWithString:@"Result: saved"];
			break;
		case MFMailComposeResultSent:
			message = [NSString stringWithString:@"Message successful send."];
			break;
		case MFMailComposeResultFailed:
			message = [NSString stringWithString:[error localizedDescription]];
			break;
		default:
			message = [NSString stringWithString:[error localizedDescription]];
			break;
	}
	
	if (message) {
		UIAlertView *alert = [[UIAlertView alloc] 
							  initWithTitle:@"Send E-Mail"
							  message:message
							  delegate:self cancelButtonTitle:nil 
							  otherButtonTitles:@"OK", nil]; 
		
		[alert show];
		[alert release];
	}
	else
	{
		[self dismissModalViewControllerAnimated:YES];
	}
}


/*
- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex
{
//	[self dismissModalViewControllerAnimated:YES];
}
*/


@end

//
//  SettingsViewController.m
//  Vphonet
//
//  Created by uncle on 12.05.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import "VphonetAppDelegate.h"
#import "HelpViewController.h"
#import "AboutViewController.h"
#import "SettingsViewController.h"
#import "UICustomAlertView.h"


@implementation SettingsViewController

- (id)initWithStyle:(UITableViewStyle)style
{
    self = [super initWithStyle:style];
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

#define kSCNavBarImageTag 6183746
#define kSCNavBarColor [UIColor colorWithRed:0.7 green:0.7 blue:0.7 alpha:1.0]

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
	/*
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self)
    {
        // this will appear as the title in the navigation bar
        CGRect frame = CGRectMake(0, 0, 400, 44);
        UILabel *label = [[[UILabel alloc] initWithFrame:frame] autorelease];
        label.backgroundColor = [UIColor clearColor];
        label.font = [UIFont boldSystemFontOfSize:20.0];
        label.shadowColor = [UIColor colorWithWhite:0.0 alpha:0.5];
        label.textAlignment = UITextAlignmentCenter;
        label.textColor = [UIColor yellowColor];
        self.navigationItem.titleView.backgroundColor = [UIColor redColor];
        label.text = NSLocalizedString(@"PageThreeTitle", @"");
    }
	*/
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
	
	
	[self setTitle:NSLocalizedString(@"Settings", @"")];
	[self setCustomBackground:@"background.png"];
	[self.tableView setSeparatorColor:[UIColor paleYellowColor]];

	/*
	 CGRect frame = CGRectMake(0, 0, 400, 44);
	 UILabel *label = [[[UILabel alloc] initWithFrame:frame] autorelease];
	 label.backgroundColor = [UIColor clearColor];
	 label.font = [UIFont boldSystemFontOfSize:20.0];
	 label.shadowColor = [UIColor colorWithWhite:0.0 alpha:0.5];
	 label.textAlignment = UITextAlignmentCenter;
	 label.textColor = [UIColor redColor];
	 self.navigationItem.titleView = label;
	 label.text = NSLocalizedString(@"PageThreeTitle", @"");
	 //[label release];
*/

	/*
	UIImageView *imageView = (UIImageView *)[self.navigationController.navigationBar viewWithTag:kSCNavBarImageTag];
    if (imageView == nil)
    {
        imageView = [[UIImageView alloc] initWithImage:
                     [UIImage imageNamed:@"TitleYellow.png"]];
        [imageView setTag:kSCNavBarImageTag];
//		[self.navigationController.navigationBar addSubview:imageView];
        [self.navigationController.navigationBar insertSubview:imageView atIndex:1];
		
        [imageView release];
    }
*/	 
		self.navigationController.navigationBar.tintColor = kSCNavBarColor;
//	self.navigationItem.titleView.backgroundColor = [UIColor redColor];
	
	
//	tintColor = [UIColor colorWithPatternImage:[UIImage imageNamed:@"TitleYellow.png" ]];
	
//	[UIColor colorWithPatternImage:image];
	
	
//	self.navigationController.navigationBar.backgroundColor  = [UIColor colorWithPatternImage:[UIImage imageNamed:@"TitleYellow.png" ]];
	
	/*
	// Allocate bitmap context
	CGContextRef bitmapContext = CGBitmapContextCreate(NULL, 320, 480, 8, 4 * 320, CGColorSpaceCreateDeviceRGB(), kCGImageAlphaNoneSkipFirst);
	// Draw Gradient Here
	CGContextDrawLinearGradient(bitmapContext, myGradient, CGPointMake(0.0f, 0.0f), CGPointMake(320.0f, 480.0f), );
	// Create a CGImage from context
	CGImageRef cgImage = CGBitmapContextCreateImage(bitmapContext);
	// Create a UIImage from CGImage
	UIImage *uiImage = [UIImage imageWithCGImage:cgImage];
	// Release the CGImage
	CGImageRelease(cgImage);
	// Release the bitmap context
	CGContextRelease(bitmapContext);
	// Create the patterned UIColor and set as background color
	[targetView setBackgroundColor:[UIColor colorWithPatternImage:image]];
	*/
//	myBar.tintColor = [UIColor greenColor];

    // Uncomment the following line to preserve selection between presentations.
    // self.clearsSelectionOnViewWillAppear = NO;
 
    // Uncomment the following line to display an Edit button in the navigation bar for this view controller.
    // self.navigationItem.rightBarButtonItem = self.editButtonItem;
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
}

- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
}

- (void)viewWillDisappear:(BOOL)animated
{
    [super viewWillDisappear:animated];
}

- (void)viewDidDisappear:(BOOL)animated
{
    [super viewDidDisappear:animated];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    // Return the number of sections.
    return 2;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    // Return the number of rows in the section.
	if (section == 0) {
		return 3;
	}
	
	return 2;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *CellIdentifier = @"Cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
        cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier] autorelease];
    }
	cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;

	
	switch (indexPath.section) {
		case 0:
			switch (indexPath.row) {
				case 0:
					cell.textLabel.text = NSLocalizedString(@"Help", @"");
					break;
				case 1:
					cell.textLabel.text = NSLocalizedString(@"About", @"");
					break;
				case 2:
					cell.textLabel.text = NSLocalizedString(@"Change Account", @"");
					break;
			}
			break;
		case 1:
			switch (indexPath.row) {
				case 0:
					cell.textLabel.text = NSLocalizedString(@"Chat Settings", @"");
					break;
//				case 1:
//					cell.textLabel.text = NSLocalizedString(@"Call Settings", @"");
//					break;
				case 1:
					cell.textLabel.text = NSLocalizedString(@"Tell a friend", @"");
					break;
			}
			break;
		default:
			break;
	}
    
    return cell;
}

/*
// Override to support conditional editing of the table view.
- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath
{
    // Return NO if you do not want the specified item to be editable.
    return YES;
}
*/

/*
// Override to support editing the table view.
- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath
{
    if (editingStyle == UITableViewCellEditingStyleDelete) {
        // Delete the row from the data source
        [tableView deleteRowsAtIndexPaths:[NSArray arrayWithObject:indexPath] withRowAnimation:UITableViewRowAnimationFade];
    }   
    else if (editingStyle == UITableViewCellEditingStyleInsert) {
        // Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view
    }   
}
*/

/*
// Override to support rearranging the table view.
- (void)tableView:(UITableView *)tableView moveRowAtIndexPath:(NSIndexPath *)fromIndexPath toIndexPath:(NSIndexPath *)toIndexPath
{
}
*/

/*
// Override to support conditional rearranging of the table view.
- (BOOL)tableView:(UITableView *)tableView canMoveRowAtIndexPath:(NSIndexPath *)indexPath
{
    // Return NO if you do not want the item to be re-orderable.
    return YES;
}
*/

#pragma mark - Table view delegate
- (NSString *)tableView:(UITableView *)tableView titleForFooterInSection:(NSInteger)section {

	if (section == 0) {
		return NSLocalizedString(@"If you will change account you will need to remember your password to login again.", @"");
	}
	return nil;
}




- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    // Navigation logic may go here. Create and push another view controller.
    /*
     <#DetailViewController#> *detailViewController = [[<#DetailViewController#> alloc] initWithNibName:@"<#Nib name#>" bundle:nil];
     // ...
     // Pass the selected object to the new view controller.
     [self.navigationController pushViewController:detailViewController animated:YES];
     [detailViewController release];
     */
	if (indexPath.section == 0)
	{
		switch (indexPath.row) {
			case 0:
			{
				HelpViewController *detailViewController = [[HelpViewController alloc] initWithNibName:@"HelpViewController" bundle:nil];
				
				UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"Back", @"") style: UIBarButtonItemStyleBordered target: nil action: nil];
				[[self navigationItem] setBackBarButtonItem: backButton];
				[backButton release];
				
				[self.navigationController pushViewController:detailViewController animated:YES];
				[detailViewController release];
			}
				break;
			case 1:
			{
				AboutViewController *detailViewController = [[AboutViewController alloc] initWithNibName:@"AboutViewController" bundle:nil];
				
				UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"Back", @"") style: UIBarButtonItemStyleBordered target: nil action: nil];
				[[self navigationItem] setBackBarButtonItem: backButton];
				[backButton release];
				
				[self.navigationController pushViewController:detailViewController animated:YES];
				[detailViewController release];
			}
				break;
			case 2:
			{
				UICustomAlertView *alert = [[UICustomAlertView alloc] 
									  initWithTitle:NSLocalizedString(@"Logout from Vphonet", @"")
									  message:NSLocalizedString(@"Are your sure you want to logout ?", @"")
									  delegate:self cancelButtonTitle:NSLocalizedString(@"Cancel", @"") 
									  otherButtonTitles:NSLocalizedString(@"Logout", @""), nil]; 
				
				[alert show];
				[alert release];
			}
				
				break;
			default:
				break;
		}
	} else if (indexPath.section == 1)
	{
		switch (indexPath.row) {
			case 0:
				break;
			case 1:
			{
				UIActionSheet *actionSheet = [[UIActionSheet alloc] initWithTitle:NSLocalizedString(@"Tell a friend", @"")
															  delegate:self
													 cancelButtonTitle:NSLocalizedString(@"Cancel", @"")
												destructiveButtonTitle:nil 
													 otherButtonTitles:NSLocalizedString(@"By SMS Message", @""),
																		NSLocalizedString(@"By E-Mail Message", @""),
																		nil];
				actionSheet.tag = 1;
				actionSheet.actionSheetStyle = UIActionSheetStyleDefault;
				VphonetAppDelegate *appDelegate = (VphonetAppDelegate *)[[UIApplication sharedApplication] delegate];
				
				[actionSheet showInView:[appDelegate window]];	// show from our table view (pops up in the middle of the table)
				[actionSheet release];

			}
				break;
			default:
				break;
		}
	}
	
}


-(void)displayComposerSheet {
	MFMailComposeViewController *picker = [[MFMailComposeViewController alloc] init];
	picker.mailComposeDelegate = self;
	[picker setTitle:NSLocalizedString(@"Tell a friend", @"")];
	[picker setSubject:NSLocalizedString(@"Vphonet is cool service", @"")];
	// Fill out the email body text
	NSString *emailBody = NSLocalizedString(@"Please registration", @"");
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


- (void)actionSheet:(UIActionSheet *)popup clickedButtonAtIndex:(NSInteger)buttonIndex {
	
	if (buttonIndex == 1)// By E-Mail
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
	}
}


- (void)mailComposeController:(MFMailComposeViewController*)controller didFinishWithResult:(MFMailComposeResult)result error:(NSError*)error 
{	
	[self dismissModalViewControllerAnimated:YES];
}



- (IBAction) onSendEmail: (id)sender
{
}	




@end

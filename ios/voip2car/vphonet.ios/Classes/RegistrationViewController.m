//
//  RegistrationViewController.m
//  Vphonet
//
//  Created by uncle on 10.04.11.
//  Copyright 2011 Lundlay. All rights reserved.
//


#import "RegistrationCountryViewController.h"
#import "RegistrationViewController.h"

#define kLeftMargin				110.0
#define kTextFieldHeight		30.0
#define kTextFieldWidth			180.0


@implementation RegistrationViewController

@synthesize textUserName;


- (id)initWithStyle:(UITableViewStyle)style
{
    self = [super initWithStyle:style];
    if (self) {
    }
    return self;
}

- (void)dealloc
{
	[textUserName release];
	[textPassword release];
	[textConfirmPassword release];
	[textFirstName release];
	[textLastName release];
	[textEMail release];
	
	[profile dealloc];
	
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

	self.title	= NSLocalizedString(@"Registration", @"");
	profile = [[VPProfile alloc] init];
	
	self.parentViewController.view.backgroundColor = [UIColor colorWithPatternImage:[UIImage imageNamed:@"background.png"]];
	self.tableView.backgroundColor = [UIColor clearColor];
	[self.tableView setSeparatorColor:[UIColor colorWithRed:240.0/255.0 green:191.0/255.0 blue:27.0/255.0 alpha:1.0]];

}

- (void)viewDidUnload
{
    [super viewDidUnload];
	
	[textUserName release];
//	textUserName = nil;
	
	[textPassword release];
//	textPassword = nil;
	
	[textConfirmPassword release];
//	textConfirmPassword = nil;
	
	[textFirstName release];
//	textFirstName = nil;
	
	[textLastName release];
//	textLastName = nil;
	
	[textEMail release];
//	textEMail = nil;
	
	[profile dealloc];
//	profile = nil;

	
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
//	UIBarButtonItem *temporaryBarButtonItem = [[UIBarButtonItem alloc] init];
//	temporaryBarButtonItem.title = NSLocalizedString(@"Back", @"");
//	self.navigationItem.backBarButtonItem = temporaryBarButtonItem;
//	[temporaryBarButtonItem release];

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
//#warning Potentially incomplete method implementation.
    // Return the number of sections.
    return 3;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
//#warning Incomplete method implementation.
    // Return the number of rows in the section.
	
    return ((section == 2) ? 1 : 3);
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section
{
	switch (section) {
		case 0:
			return NSLocalizedString(@"Create Account", @"");
			break;
//		case 1:
//			return @" ";
//			break;
			
		default:
			break;
	}
	return @" ";
}


- (UITextField *)textFieldNormal
{
	UITextField *textFieldNormal;
	
	CGRect frame = CGRectMake(kLeftMargin, 8.0, kTextFieldWidth, kTextFieldHeight);
	textFieldNormal = [[UITextField alloc] initWithFrame:frame];
	
	textFieldNormal.borderStyle = UITextBorderStyleNone;// UITextBorderStyleRoundedRect;
	textFieldNormal.textColor = [UIColor blackColor];
	textFieldNormal.font = [UIFont systemFontOfSize:17.0];
	textFieldNormal.backgroundColor = [UIColor whiteColor];
	textFieldNormal.autocorrectionType = UITextAutocorrectionTypeNo;	// no auto correction support	
	textFieldNormal.keyboardType = UIKeyboardTypeDefault;	// use the default type input method (entire keyboard)
//	textFieldNormal.returnKeyType = UIReturnKeyDone;	
//	textFieldNormal.clearButtonMode = UITextFieldViewModeWhileEditing;	// has a clear 'x' button to the right	
//	textFieldNormal.tag = kViewTag;		// tag this control so we can remove it later for recycled cells	
	textFieldNormal.delegate = self;	// let us be the delegate so we know when the keyboard's "Done" button is pressed
	

	return textFieldNormal;
}

- (UITextField *)textUserName
{
	if (textUserName == nil) {
		textUserName = [self textFieldNormal];
		[textUserName setPlaceholder:NSLocalizedString(@"Required", @"")];
	}
	
	return textUserName; 
}


- (UITextField *)textPassword
{
	if (textPassword == nil) {
		textPassword = [self textFieldNormal];
		textPassword.secureTextEntry = YES;
		[textPassword setPlaceholder:NSLocalizedString(@"Required", @"")];
	}
	
	return textPassword; 
}

- (UITextField *)textConfirmPassword
{
	if (textConfirmPassword == nil) {
		textConfirmPassword = [self textFieldNormal];
		textConfirmPassword.secureTextEntry = YES;
		[textConfirmPassword setPlaceholder:NSLocalizedString(@"Required",@"")];
	}
	
	return textConfirmPassword; 
}

- (UITextField *)textFirstName
{
	if (textFirstName == nil) {
		textFirstName = [self textFieldNormal];
		textFirstName.autocorrectionType = UIKeyboardTypeNamePhonePad;	// no auto correction support
	}
	
	return textFirstName; 
}

- (UITextField *)textLastName
{
	if (textLastName == nil) {
		textLastName = [self textFieldNormal];
		textLastName.autocorrectionType = UIKeyboardTypeNamePhonePad;	// no auto correction support
	}
	
	return textLastName; 
}

- (UITextField *)textEMail
{
	if (textEMail == nil) {
		textEMail = [self textFieldNormal];
		textEMail.keyboardType = UIKeyboardTypeEmailAddress;	// use the default type input method (entire keyboard)
		textEMail.autocorrectionType = UIKeyboardTypeEmailAddress;	// no auto correction support
		[textEMail setPlaceholder:NSLocalizedString(@"Required",@"")];
	}
	return textEMail; 
}


- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *CellIdentifier = @"Cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
        cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier] autorelease];
		cell.selectionStyle = UITableViewCellSelectionStyleNone;
    }
    
    // Configure the cell...
	switch (indexPath.section) {
		case 0:
			switch (indexPath.row) {
				case 0:	
					cell.textLabel.text = NSLocalizedString(@"User name:", @"");
					[cell.contentView addSubview:[self textUserName]];
					break;
				case 1:	
					cell.textLabel.text = NSLocalizedString(@"Password:", @"");
					[cell.contentView addSubview:[self textPassword]];
					break;
				case 2:	cell.textLabel.text = NSLocalizedString(@"Confirm:", @"");
					[cell.contentView addSubview:[self textConfirmPassword]];
					break;
			}
			break;
		case 1:
			switch (indexPath.row) {
				case 0:	cell.textLabel.text = NSLocalizedString(@"First Name:", @"");
					[cell.contentView addSubview:[self textFirstName]];
					break;
				case 1:	cell.textLabel.text = NSLocalizedString(@"Last Name:", @"");
					[cell.contentView addSubview:[self textLastName]];
					break;
				case 2:	cell.textLabel.text = NSLocalizedString(@"E-Mail:", @"");
					[cell.contentView addSubview:[self textEMail]];
					break;
			}
			break;
		case 2:
			{
				cell.textLabel.text = NSLocalizedString(@"Continue",@"");
				cell.textLabel.textAlignment = UITextAlignmentCenter;
				cell.selectionStyle = UITableViewCellSelectionStyleBlue;
			}
					break;
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

- (UIView *)tableView:(UITableView *)tableView viewForHeaderInSection:(NSInteger)section {
	UIView *containerView = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 320, 40)];
//	containerView.backgroundColor = [UIColor groupTableViewBackgroundColor];
	containerView.backgroundColor = [UIColor clearColor];
	CGRect labelFrame = CGRectMake(20, 2, 320, 30);
	if(section == 0) {
		labelFrame.origin.y = 13;
	UILabel *label = [[UILabel alloc] initWithFrame:labelFrame];
	label.backgroundColor = [UIColor clearColor];
	label.font = [UIFont boldSystemFontOfSize:17];
	label.shadowColor = [UIColor colorWithWhite:1.0 alpha:1];
	label.shadowOffset = CGSizeMake(0, 1);
	label.textColor = [UIColor colorWithRed:0.265 green:0.294 blue:0.367 alpha:1.000];
	label.text = [self tableView:tableView titleForHeaderInSection:section];
	[containerView addSubview:label];

	CGRect subLabelFrame = CGRectMake(20, 32, 320, 30);
	UILabel *subLabel = [[UILabel alloc] initWithFrame:subLabelFrame];
	subLabel.backgroundColor = [UIColor clearColor];
	subLabel.font = [UIFont boldSystemFontOfSize:14];
	subLabel.shadowColor = [UIColor colorWithWhite:1.0 alpha:1];
	subLabel.shadowOffset = CGSizeMake(0, 1);
	subLabel.textColor = [UIColor colorWithRed:0.265 green:0.294 blue:0.367 alpha:1.000];
	subLabel.text = NSLocalizedString(@"Type your new account information", @"");
	[containerView addSubview:subLabel];
	}
	/*
	if(section == 1) {
		UIButton *abutton = [UIButton buttonWithType: UIButtonTypeContactAdd];
		abutton.frame = CGRectMake(270,0 , 40, 40);
		[abutton addTarget: self action: @selector(addPage:)
		  forControlEvents: UIControlEventTouchUpInside];
		[containerView addSubview:abutton];
	}
	*/
	
	return containerView;
}

- (CGFloat)tableView:(UITableView *)tableView heightForHeaderInSection:(NSInteger)section {
	if(section == 0)
		return 47+15;
	else {
		return 5;
	}
	
}


- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
	
	NSString* message = nil;
	UIView* control = nil;
	NSIndexPath* path = nil;
	NSIndexPath* path1 = nil;
	NSCharacterSet *letters = [NSCharacterSet letterCharacterSet];
	
	
	if (indexPath.section == 2) {
		if ([textUserName.text length] < 6) {
			message = NSLocalizedString(@"User Name mininimum 6 characters and must starts with letter", @"");
			control = textUserName;
			path = [NSIndexPath indexPathForRow:0 inSection:0];
		} else if (![letters characterIsMember:[textUserName.text characterAtIndex:0]]){
			message = NSLocalizedString(@"User Name mininimum 6 characters and must starts with letter", @"");
			control = textUserName; 
			path = [NSIndexPath indexPathForRow:0 inSection:0];
		} else if ([textPassword.text length] < 6) {
			message = NSLocalizedString(@"Password mininimum 6 characters", @"");
			control = textPassword;
			path = [NSIndexPath indexPathForRow:1 inSection:0];
		} else if (([textConfirmPassword.text length] < 6) || ([textConfirmPassword.text compare:[textPassword text]] != NSOrderedSame)) {
			message = NSLocalizedString(@"Password does not match", @"");
			control = textPassword; 
			path = [NSIndexPath indexPathForRow:1 inSection:0];
			path1 = [NSIndexPath indexPathForRow:2 inSection:0];
		} else if ([textEMail.text length] == 0 ) {
			message = NSLocalizedString(@"Please enter E-Mail", @"");
			control = textEMail; 
			path = [NSIndexPath indexPathForRow:2 inSection:1];
		} else {
			NSString *charactersFromFirstName = nil;
			NSString *opa = self.textFirstName.text;
			NSScanner *scannerFirstName = [NSScanner scannerWithString:opa]; 
			[scannerFirstName scanCharactersFromSet:[NSCharacterSet characterSetWithCharactersInString:@"01234567890"] 
										 intoString:&charactersFromFirstName];
				
			NSString *charactersFromLastName = nil; 
			NSScanner *scannerLastName = [NSScanner scannerWithString:self.textLastName.text]; 
			[scannerLastName scanCharactersFromSet:[NSCharacterSet characterSetWithCharactersInString:@"01234567890"] 
										 intoString:&charactersFromLastName];
			
			if (([textFirstName.text length] > 0) && ([charactersFromFirstName length] > 0)) {
				message = NSLocalizedString(@"The First Name must only contain characters", @"");
				control = textFirstName; 
				path = [NSIndexPath indexPathForRow:0 inSection:1];
			} else if (([textLastName.text length] > 0) && ([charactersFromLastName length] > 0)) {
				message = NSLocalizedString(@"The Last Name must only contain characters", @"");
				control = textLastName; 
				path = [NSIndexPath indexPathForRow:1 inSection:1];
			} else {
					NSString *emailRegex = @"[A-Z0-9a-z._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,4}"; 
					NSPredicate *emailTest = [NSPredicate predicateWithFormat:@"SELF MATCHES %@", emailRegex]; 
					
					if (([textEMail.text length] != 0) && (![emailTest evaluateWithObject:textEMail.text]))
					{
						message = NSLocalizedString(@"Bad E-Mail", @"");
						control = textEMail; 
						path = [NSIndexPath indexPathForRow:2 inSection:1];
					}
			}
		
		}
		
		for (int s = 0; s < [tableView numberOfSections]; s++) {
			for (int r = 0; r < [tableView numberOfRowsInSection:s]; r++) {
				[self.tableView selectRowAtIndexPath:[NSIndexPath indexPathForRow:r inSection:s] 
											animated:NO scrollPosition:UITableViewScrollPositionTop ];

				UITableViewCell* c = [self.tableView cellForRowAtIndexPath:[NSIndexPath indexPathForRow:r inSection:s]];
				if (c) {
					c.textLabel.textColor = [UIColor blackColor];
				}
			}
		}
		
		/*
		 
		if (indexPath.row % 2)
		{
			[cell setBackgroundColor:[UIColor colorWithRed:.8 green:.8 blue:1 alpha:1]];
		}
		else 
			[cell setBackgroundColor:[UIColor clearColor]];
		 
		*/

		
		if (message) {
			if (path1) {
				[self.tableView selectRowAtIndexPath:path1 
											animated:NO scrollPosition:UITableViewScrollPositionTop ];
				
				UITableViewCell* cell;
				cell = [self.tableView cellForRowAtIndexPath:path1];
				if (cell)
					cell.textLabel.textColor = [UIColor redColor];
				
				//				[cell setSelected:YES animated:NO];
				
				//				[self.tableView select:[cell]];// selectRowAtIndexPath:[NSIndexPath indexPathForRow:r inSection:s] 
				//animated:NO scrollPosition:UITableViewScrollPositionTop ];
				cell.textLabel.textColor = [UIColor redColor];
			}
			
			if (path) {
				[self.tableView selectRowAtIndexPath:path 
											animated:NO scrollPosition:UITableViewScrollPositionTop ];

				UITableViewCell* cell;
				cell = [self.tableView cellForRowAtIndexPath:path];
				if (cell) {
					cell.textLabel.textColor = [UIColor redColor];
				}
				cell.textLabel.textColor = [UIColor redColor];
			}
			
			UIAlertView *alert = [[UIAlertView alloc] 
								  initWithTitle:NSLocalizedString(@"Error", @"")
								  message:message
								  delegate:self cancelButtonTitle:nil 
								  otherButtonTitles:NSLocalizedString(@"OK", @""), nil]; 
			
			[alert show];
			[alert release];
			
		}

		//			3.First Name and Last Name: only character.
		//			4.E-Mail: string first then a then "." then character
		
	
		if (control)
			[control becomeFirstResponder]; // Set focus
		
		if ((control != nil) || (message != nil))
			return;
		
		profile.userName = textUserName.text;
		profile.password = [textPassword text];
		profile.lastName = textLastName.text;
		profile.firstName = textFirstName.text;
		profile.email	= textEMail.text;
		
		[[[self tableView] cellForRowAtIndexPath:indexPath] setSelected:YES animated:YES];
		RegistrationCountryViewController *countryViewController = [[RegistrationCountryViewController alloc] initWithNibName:@"RegistrationCountryViewController" bundle:nil];
		
		// Pass the selected object to the new view controller.
		UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"Back", @"") style: UIBarButtonItemStyleBordered target: nil action: nil];
		[[self navigationItem] setBackBarButtonItem: backButton];
		[backButton release];

		
		
		countryViewController.profile = profile;
		[self.navigationController pushViewController:countryViewController animated:YES];
		//[countryViewController release];
		[[[self tableView] cellForRowAtIndexPath:indexPath] setSelected:NO animated:YES];
	}
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



/*
- (UIView *) tableView:(UITableView *)tableView viewForHeaderInSection:(NSInteger)section 
{
//	UIView *headerView = [[[UIView alloc] initWithFrame:CGRectMake(0, 0, tableView.bounds.size.width, 100)] autorelease];
//	if (section == integerRepresentingYourSectionOfInterest)
//		[headerView setBackgroundColor:[UIColor redColor]];
//	else 
//		[headerView setBackgroundColor:[UIColor clearColor]];

	//Please choose your country and type your phone number
	
	UILabel *label = [[[UILabel alloc] initWithFrame:CGRectMake(10, 3, tableView.bounds.size.width - 10, 18)] autorelease];
	label.text = @"Section Header Text Here";
	label.textColor = [UIColor colorWithRed:1.0 green:1.0 blue:1.0 alpha:0.75];
	label.backgroundColor = [UIColor clearColor];
//	[headerView addSubview:label];
	
//	return headerView;
}
*/


@end

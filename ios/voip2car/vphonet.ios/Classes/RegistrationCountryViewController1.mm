//
//  RegistrationCountryViewController.m
//  Vphonet
//
//  Created by uncle on 10.04.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import "VPEngine.h"
#import "LoginViewController.h"
#import "CountryListViewController.h"
#import "EditingTableViewCell.h"
#import "RegistrationCountryViewController.h"


@implementation RegistrationCountryViewController


//@synthesize hiddenTextField;
@synthesize registrationInfo;
@synthesize editingTableViewCell;
/*

- (id)initWithRegistrationInfo:(UITableViewStyle)style registrationInfo:(RegistrationInfo*)info
{
	if ((self = [super initWithStyle:style]))
	{
		regInfo = info;
	}
	return self;	
	
}
*/

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
//	[hiddenTextField release];
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
	
	self.title = NSLocalizedString(@"Registration C", @"");
	self.tableView.hidden = NO;
	
	//UIBarButtonItem *temporaryBarButtonItem = [[UIBarButtonItem alloc] init];
	//temporaryBarButtonItem.title = NSLocalizedString(@"Back", @"");
	//self.navigationItem.backBarButtonItem = temporaryBarButtonItem;
	//[temporaryBarButtonItem release];
	/*
	if (hiddenTextField == nil)
	{
		hiddenTextField = [[UITextField alloc] initWithFrame: CGRectZero];
		hiddenTextField.delegate = self;
		hiddenTextField.keyboardType = UIKeyboardTypeDefault;		
	}
	*/
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
	//[hiddenTextField becomeFirstResponder];
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
    return ((section == 0) ? 2 : 1);
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section
{
	switch (section) {
		case 0:
			return NSLocalizedString(@"Your phone number", @"");
			break;
			//		case 1:
			//			return @" ";
			//			break;
			
		default:
			break;
	}
	return @" ";
}


- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
	static NSString *RegistrationCountryEditCellIdentifier = @"RegistrationCountryEditCellIdentifier";
	static NSString *RegistrationCountryCellIdentifier = @"RegistrationCountryCellIdentifier";
//	static NSString *RegistrationCountryCellIdentifier = @"RegistrationCountryCellIdentifier";
//	static NSString *RegistrationCountryCellIdentifier = @"RegistrationCountryCellIdentifier";
    UITableViewCell *result = nil;
	
	if (indexPath.section == 0)
	{
		EditingTableViewCell *cell = (EditingTableViewCell *)[tableView dequeueReusableCellWithIdentifier:RegistrationCountryEditCellIdentifier];
		if (cell == nil) {
			[[NSBundle mainBundle] loadNibNamed:@"EditingTableViewCell" owner:self options:nil];
			cell = editingTableViewCell;
			self.editingTableViewCell = nil;
			cell.selectionStyle = UITableViewCellSelectionStyleNone;
		}
		
		if (indexPath.row == 0) {
			cell.label.text = NSLocalizedString(@"Country:", @"");
			cell.textField.text = registrationInfo.country;
			cell.textField.placeholder = NSLocalizedString(@"Select country", @"");
			cell.textField.enabled = FALSE;
			cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
			cell.selectionStyle = UITableViewCellSelectionStyleBlue; 
		}
		else if (indexPath.row == 1) {
			cell.label.text = NSLocalizedString(@"Number:", @"");
			if (registrationInfo.code != nil)
			{
				cell.textField.text = [NSString stringWithFormat:@"+%@ ",registrationInfo.code];
				[cell.textField becomeFirstResponder];
			}
			cell.textField.borderStyle = UITextBorderStyleNone;
			cell.textField.placeholder = NSLocalizedString(@"Phone number", @"");
			cell.textField.keyboardType = UIKeyboardTypeNamePhonePad;// UIKeyboardTypePhonePad;// UIKeyboardTypeNumberPad;// UIKeyboardTypeDecimalPad;
			[cell.textField becomeFirstResponder];
		}
		result = cell;
	} else {
		UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:RegistrationCountryCellIdentifier];
		if (cell == nil) {
			cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:RegistrationCountryCellIdentifier] autorelease];
			cell.selectionStyle = UITableViewCellSelectionStyleNone;
		}
		cell.textLabel.text = @"Done";
		cell.textLabel.textAlignment = UITextAlignmentCenter;
		cell.selectionStyle = UITableViewCellSelectionStyleBlue;
		result = cell;
	}

	
	
/*    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
        cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier] autorelease];
		cell.selectionStyle = UITableViewCellSelectionStyleNone;
    }
	
	switch (indexPath.section) {
		case 0:
			switch (indexPath.row) {
				case 0:	
					cell.textLabel.text = ;
					cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
					break;
				case 1:	
					cell.textLabel.text = @"Number:";
					break;
			}
			break;
		case 1:
			cell.textLabel.text = @"Done";
			cell.textLabel.textAlignment = UITextAlignmentCenter;
			cell.selectionStyle = UITableViewCellSelectionStyleBlue;
			break;
	}
*/    
    return result;
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

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
	if ((indexPath.section == 0) && (indexPath.row == 0)) {
		[[[self tableView] cellForRowAtIndexPath:indexPath] setSelected:YES animated:YES];
		CountryListViewController *countryViewController = [[CountryListViewController alloc] initWithNibName:@"CountryListViewController" bundle:nil];
		countryViewController.delegate = self;
		countryViewController.registrationInfo = registrationInfo;
		[self.navigationController pushViewController:countryViewController animated:YES];
		[countryViewController release];
		[[[self tableView] cellForRowAtIndexPath:indexPath] setSelected:NO animated:YES];
		return;
	}
	
	if (indexPath.section == 1) // Done
	{
		NSLog(@"userName: %@ password: %@ first: %@ last: %@ email: %@ country: %@ phone: %@",
			  registrationInfo.userName, registrationInfo.password, registrationInfo.firstName, 
			  registrationInfo.lastName, registrationInfo.eMail, registrationInfo.country, registrationInfo.phoneNumber);
		[self postRegistrationInfo:registrationInfo];
//		NSString* test = registrationInfo.country;
	}
		
}



- (UIView *)tableView:(UITableView *)tableView viewForHeaderInSection:(NSInteger)section {
	UIView *containerView = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 320, 40)];
	containerView.backgroundColor = [UIColor groupTableViewBackgroundColor];
	CGRect labelFrame = CGRectMake(18, 2, 320, 30);
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
		
		CGRect subLabelFrame = CGRectMake(20, 32, 300, 45);
		UILabel *subLabel = [[UILabel alloc] initWithFrame:subLabelFrame];
		subLabel.backgroundColor = [UIColor clearColor];
		subLabel.font = [UIFont boldSystemFontOfSize:14];
		subLabel.shadowColor = [UIColor colorWithWhite:1.0 alpha:1];
		subLabel.shadowOffset = CGSizeMake(0, 1);
		subLabel.textColor = [UIColor colorWithRed:0.265 green:0.294 blue:0.367 alpha:1.000];
		subLabel.text = NSLocalizedString(@"Please choose your country and type your phone number", @"");
		subLabel.numberOfLines = 2;
		subLabel.lineBreakMode = UILineBreakModeWordWrap;
		[containerView addSubview:subLabel];
	}
	return containerView;
}

- (CGFloat)tableView:(UITableView *)tableView heightForHeaderInSection:(NSInteger)section {
	if(section == 0)
		return 47+30;
	else {
		return 10;
	}
}


- (void)countryListViewController:(CountryListViewController *)countryListViewController didSelectCountry:(Country *)country {
    if (country) {  
		
		registrationInfo.country = country.name;
		registrationInfo.code = country.code;
		[self.tableView reloadData];
		
        // Show the recipe in a new view controller
//        [self showRecipe:recipe animated:NO];
    }
    
    // Dismiss the modal add recipe view controller
//    [self dismissModalViewControllerAnimated:YES];
}


-(void)postRegistrationInfo:(RegistrationInfo *)_info {
	//	NSMutableURLRequest *req = [NSMutableURLRequest requestWithURL:
	//								[NSURL URLWithString:@"https://vphonet.com/getregistration2.php"]]; 
	//	[req setHTTPMethod:@"POST"];
	
	//	NSString *contentType = [NSString stringWithString:@""];
	//	[req setValue:contentType forHTTPHeaderField:@"Content-type"]; 
	//	NSMutableData *postBody = [NSMutableData data];
	if ([_info.code length] == 0) 
	{
		UIAlertView *alert = [[UIAlertView alloc] 
							  initWithTitle:NSLocalizedString(@"Error", @"")
							  message:NSLocalizedString(@"Please, select country", @"")
							  delegate:self cancelButtonTitle:nil 
							  otherButtonTitles:NSLocalizedString(@"OK", @""), nil]; 
		
		[alert show];
		[alert release];
		return;
	}

	
	
	
	NSURL *url = [NSURL URLWithString:@"https://vphonet.com/getregistration2.php"];
	NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:url 
                                                           cachePolicy:NSURLRequestReloadIgnoringLocalCacheData 
                                                       timeoutInterval:30.0]; 
    [request setHTTPMethod:@"POST"];
	[request setValue:@"application/x-www-form-urlencoded" forHTTPHeaderField:@"Content-Type"];
	
	//Implement request_body for send request here username and password set into the body.
	NSString *body = [NSString stringWithFormat:@"frm_first_name=%@",[_info.firstName stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]];

	body = [body stringByAppendingString:@"&frm_middle_name="];
	body = [body stringByAppendingFormat:[NSString stringWithFormat:@"&frm_last_name=%@",[_info.lastName stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]]];
	body = [body stringByAppendingFormat:[NSString stringWithFormat:@"&frm_email=%@", [_info.eMail stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]]];
	body = [body stringByAppendingString:@"&frm_privacy=true"];// false or true; true means that user details should be private*
    body = [body stringByAppendingString:@"&frm_deal=1"];
	body = [body stringByAppendingString:@"&frm_day=1"];
	body = [body stringByAppendingString:@"&frm_month=1"];
	body = [body stringByAppendingString:@"&frm_year=1970"];
	body = [body stringByAppendingFormat:[NSString stringWithFormat:@"&frm_country=%@", [_info.code stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]]];
//	body = [body stringByAppendingFormat:@"&frm_country=380"];
	body = [body stringByAppendingFormat:@"&frm_statetext=registration"];
	body = [body stringByAppendingFormat:[NSString stringWithFormat:@"&frm_tokenid=%@", [VPEngine GetUUID]]];
//										  [[[UIDevice currentDevice] uniqueIdentifier] stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]]]; //a random string
	body = [body stringByAppendingString:@"&frm_address="];
	body = [body stringByAppendingString:@"&frm_city="];
	body = [body stringByAppendingString:@"&frm_zip="];
	body = [body stringByAppendingString:@"&frm_phone_pref="];
	body = [body stringByAppendingFormat:@"&frm_phone=%@", [_info.phoneNumber stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]];
	body = [body stringByAppendingString:@"&frm_cell_pref="];
	body = [body stringByAppendingString:@"&frm_cell="];
	body = [body stringByAppendingString:@"&frm_gender=0"];
	body = [body stringByAppendingString:@"&frm_lang=1"];
	body = [body stringByAppendingFormat:@"&frm_username=%@", [_info.userName stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]];// same as username
	body = [body stringByAppendingFormat:@"&frm_password=%@", [_info.password stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]];// same as password
	body = [body stringByAppendingString:@"&frm_PIN=1234"];
    body = [body stringByAppendingString:@"&frID_adm=37"];// a server specific id (should be hardwired inside the software)
    body = [body stringByAppendingString:@"&frNSite=37"];// a server specific id (should be hardwired inside the software)
    body = [body stringByAppendingString:@"&frIDtype=120"]; //a server specific id (should be hardwired inside the software)
    body = [body stringByAppendingString:@"&frm_newinst=1"]; //if the user has never received a promotion, this field should be transmitted with the value 1, otherwise it should not be transmitted.
	
	NSLog(@"%@",body);
	
	[request setHTTPBody:[body dataUsingEncoding:NSUTF8StringEncoding]];

    [[UIApplication sharedApplication] setNetworkActivityIndicatorVisible:YES]; 
	
	[NSURLRequest setAllowsAnyHTTPSCertificate:YES forHost:[url host]];
	
	NSHTTPURLResponse * returnResponse = nil; 
	NSError * returnError = nil;
	
	NSData *returnData = [NSURLConnection sendSynchronousRequest:request
											   returningResponse:&returnResponse
														   error:&returnError]; 
	int statusCode = returnResponse.statusCode;
	
	NSString	*str = [[[NSString alloc] initWithData:returnData
										   encoding:NSUTF8StringEncoding] autorelease]; 

	NSLog(@"HTTP Return: %@. Status code: %d. Error:%@", str, statusCode, [returnError localizedDescription]);
	
	if ([str isEqualToString:@"rc=99999"])
	{
		NSLog(@"Username is already in use");
		UIAlertView *alert = [[UIAlertView alloc] 
							  initWithTitle:NSLocalizedString(@"Failed registration", @"")
							  message:NSLocalizedString(@"Username is already in use", @"")
							  delegate:self cancelButtonTitle:nil 
							  otherButtonTitles:NSLocalizedString(@"OK", @""), nil]; 
		
		[alert show];
		[alert release];
		[self.navigationController popViewControllerAnimated:YES];// popToRootViewControllerAnimated:YES];
		return;
	} else if ([str isEqualToString:@"rc=99998"])
	{
		NSLog(@"The email was already used for another account");
		UIAlertView *alert = [[UIAlertView alloc] 
							  initWithTitle:NSLocalizedString(@"Failed registration", @"")
							  message:NSLocalizedString(@"The email was already used for another account", @"")
							  delegate:self cancelButtonTitle:nil 
							  otherButtonTitles:NSLocalizedString(@"OK", @""), nil]; 
		
		[alert show];
		[alert release];
		[self.navigationController popViewControllerAnimated:YES];// popToRootViewControllerAnimated:YES];
//		[self.navigationController popToRootViewControllerAnimated:YES];
		return;
	} else if (statusCode != 200)
	{
		UIAlertView *alert = [[UIAlertView alloc] 
							  initWithTitle:NSLocalizedString(@"Failed request", @"")
							  message:[NSString stringWithFormat:@"%@\n%@", str, [returnError localizedDescription]]
							  delegate:self cancelButtonTitle:nil 
							  otherButtonTitles:NSLocalizedString(@"OK", @""), nil]; 
		
		[alert show];
		[alert release];
		[self.navigationController popViewControllerAnimated:YES];// popToRootViewControllerAnimated:YES];
//		[self.navigationController popToRootViewControllerAnimated:YES];
		return;
	}

	//	NSString *list = @"Norman, Stanley, Fletcher";
	NSArray *result = [str componentsSeparatedByString:@"&"];
	
	NSString* rc = nil;
	NSString* regcode = nil;
	NSString* purchaseid = nil;
	NSString* vp_number = nil;
	
	//rc=0
	//regcode=M7WJ-FSZTR-U7DG
	//purchaseid=VP252-44598
	//vp_number=252-945-6583
	for(NSString * item in result) {
		if ([item hasPrefix:@"rc="]) {
			rc = [item stringByReplacingOccurrencesOfString:@"rc=" withString:@""];
		} else if ([item hasPrefix:@"regcode="]) {
			regcode = [item stringByReplacingOccurrencesOfString:@"regcode=" withString:@""];
		} else if ([item hasPrefix:@"purchaseid="]) {
			purchaseid = [item stringByReplacingOccurrencesOfString:@"purchaseid=" withString:@""];
		} else if ([item hasPrefix:@"vp_number="]) {
			vp_number = [item stringByReplacingOccurrencesOfString:@"vp_number=" withString:@""];
		}
	}	
	
    NSLog(@"rc:%@, regcode:%@, purchaseid:%@, vp_number:%@", rc, regcode, purchaseid, vp_number);
	
	if (![rc isEqualToString:@"0"]) {
		UIAlertView *alert = [[UIAlertView alloc] 
							  initWithTitle:NSLocalizedString(@"Server error", @"")
							  message:[NSString stringWithFormat:@"rc = %@", rc]
							  delegate:self cancelButtonTitle:nil 
							  otherButtonTitles:NSLocalizedString(@"OK", @""), nil]; 
		
		[alert show];
		[alert release];
		return;
	}
	
	NSLog(@"View Controllers : %@",[self.navigationController viewControllers]);
	
	[RegistrationInfo insert:_info];
	
	[self.navigationController popToRootViewControllerAnimated:YES];

	NSDictionary *regInfo = [NSDictionary dictionaryWithObjectsAndKeys:
									  _info.userName, @"login", _info.password, @"password", nil];
	[[NSNotificationCenter defaultCenter] postNotificationName:OnRegistrationCompleteNotification object:nil userInfo:regInfo];	
	
	
}

-(void)postConfirm:(RegistrationInfo *)_info {
	
}

- (BOOL)connection:(NSURLConnection *)connection canAuthenticateAgainstProtectionSpace:(NSURLProtectionSpace *)protectionSpace {
	return [protectionSpace.authenticationMethod isEqualToString:NSURLAuthenticationMethodServerTrust];
}

- (void)connection:(NSURLConnection *)connection didReceiveAuthenticationChallenge:(NSURLAuthenticationChallenge *)challenge {
	//if ([challenge.protectionSpace.authenticationMethod isEqualToString:NSURLAuthenticationMethodServerTrust])
	//	if ([trustedHosts containsObject:challenge.protectionSpace.host])
	//		[challenge.sender useCredential:[NSURLCredential credentialForTrust:challenge.protectionSpace.serverTrust] forAuthenticationChallenge:challenge];
	
	[challenge.sender continueWithoutCredentialForAuthenticationChallenge:challenge];
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

- (BOOL)textField:(UITextField *)textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)string {
	
//	if (textField == hiddenTextField) {
//		return false;
//	}
	return true;
}





@end

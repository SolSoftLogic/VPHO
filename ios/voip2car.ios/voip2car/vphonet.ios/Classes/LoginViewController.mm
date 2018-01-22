//
//  LoginViewController.m
//  Vphonet
#import "VphonetAppDelegate.h"
#import "VPEngine.h"
#import "VPProfile.h"
#import "LoginViewController.h"
#import "ForgotPasswordViewController.h"
#import "RegistrationViewController.h"
#import "VPDatabase.h"
#import "VPContactList.h"

@implementation LoginViewController

@synthesize user;
@synthesize password;


NSString *OnRegistrationCompleteNotification = @"OnRegistrationCompleteNotification";

#pragma mark -
#pragma mark View lifecycle

/*

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    if ((self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil])) {
		// Custom initialization
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(handleTweetNotification:) name:IFTweetLabelURLNotification object:nil];
    }
    return self;
}
*/ 


- (void)viewDidLoad {
    [super viewDidLoad];
	

//	[self.tableView setSeparatorColor:[UIColor clearColor]];
//	self.view.backgroundColor = [UIColor colorWithPatternImage:[UIImage imageNamed:@"background.png"]];
	self.title = NSLocalizedString(@"Vphonet Login", @"");
	
	[[NSNotificationCenter defaultCenter] addObserver:self 
											 selector:@selector(handleTweetNotification:) 
												 name:IFTweetLabelURLNotification 
											   object:nil];

	[[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(registrationComplete:) 
                                                 name:OnRegistrationCompleteNotification
                                               object:nil];

	[self setCustomBackground:@"background.png"];

	self.tableView.backgroundColor = [UIColor clearColor];
	[self.tableView setSeparatorColor:[UIColor paleYellowColor]];
	
	
//	UIBarButtonItem *temporaryBarButtonItem = [[UIBarButtonItem alloc] init];
//	temporaryBarButtonItem.title = NSLocalizedString(@"Back", @"");
//	self.navigationItem.backBarButtonItem = nil;
	
//	[temporaryBarButtonItem release];

/*	
	self.navigationItem.leftBarButtonItem 
		= [ [ [ UIBarButtonItem alloc ] 
		 initWithTitle:@"?" 
		 style: UIBarButtonItemStylePlain
		 target: self 
		 action:@selector(forgot) ]
	   autorelease ];	
*/
	self.navigationItem.rightBarButtonItem = 
		[[UIBarButtonItem alloc] initWithBarButtonSystemItem: UIBarButtonSystemItemAdd target:self action:@selector(register)];
	
}



- (void) viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
    
    [loginProgressView removeFromSuperview];
	[loginButton setEnabled:YES];
	[user setEnabled:YES];
	[password setEnabled:YES];
	
	[user becomeFirstResponder]; // Set focus

	[self.tabBarController.navigationController setNavigationBarHidden:YES animated:NO];
	[self.navigationController setNavigationBarHidden:NO animated:NO];

	self.navigationItem.hidesBackButton = YES;
	
//	self.navigationItem.leftBarButtonItem.hidden = YES;
	//leftBarButtonItem.hidden = YES). I can only set 'enabled' property. Anybody know how to control the hide and show property of the leftBarButtonItem, ...

//	[self.navigationItem.leftBarButtonItem setEnabled:YES];
//	[self.navigationItem.rightBarButtonItem setEnabled:YES];
	
	[[NSNotificationCenter defaultCenter] addObserver:self 
											 selector:@selector(keyboardDidShow:) 
												 name:UIKeyboardDidShowNotification 
											   object:nil];


	[[NSNotificationCenter defaultCenter] postNotificationName:UIKeyboardDidShowNotification object:nil];
	
}



 - (void)viewWillDisappear:(BOOL)animated {
	 [[NSNotificationCenter defaultCenter] removeObserver:self
													 name:UIKeyboardDidShowNotification
												   object:nil];
	 [super viewWillDisappear:animated];
}
 



- (void)forgot
{
}

- (void)register
{
	RegistrationViewController *registrationViewController = [[RegistrationViewController alloc] initWithNibName:@"RegistrationViewController" bundle:nil];
	// ...
	// Pass the selected object to the new view controller.
	[self.navigationController pushViewController:registrationViewController animated:YES];
	
	UIBarButtonItem *temporaryBarButtonItem = [[UIBarButtonItem alloc] init];
	temporaryBarButtonItem.title = NSLocalizedString(@"Back", @"");
	self.navigationItem.backBarButtonItem = temporaryBarButtonItem;
	[temporaryBarButtonItem release];

	
	[registrationViewController release];
}



/*
- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
}
*/
/*
- (void)viewWillDisappear:(BOOL)animated {
    [super viewWillDisappear:animated];
}
*/
/*
- (void)viewDidDisappear:(BOOL)animated {
    [super viewDidDisappear:animated];
}
*/
/*
// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations.
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}
*/


#pragma mark -
#pragma mark Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    // Return the number of sections.
    return 2;
}


- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    // Return the number of rows in the section.
    switch (section) {
        case(0):
			return 2;
			break;
		case(1):
			return 1;
			break;
	}
	
	return 0;	
}


// Customize the appearance of table view cells.
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    
	NSString *cellIdentifier = [ NSString stringWithFormat: @"%d:%d", [ indexPath indexAtPosition: 0 ],
								[ indexPath indexAtPosition:1 ] ];
	
    UITableViewCell *cell = [ tableView dequeueReusableCellWithIdentifier: cellIdentifier ];
    if (cell == nil) {
        cell = [ [ [ UITableViewCell alloc ] initWithFrame: CGRectZero reuseIdentifier: cellIdentifier ] autorelease ];
		cell.selectionStyle = UITableViewCellSelectionStyleNone;
		float fieldWidth = 120.0f;
		if (self.view.frame.size.width == 768.0f)
			fieldWidth += 40;
		
		switch ([ indexPath indexAtPosition: 0]) {
			case(0):
				switch([ indexPath indexAtPosition: 1]) {
					case(0):
                    {
						user = [[UITextField alloc] initWithFrame:CGRectMake(fieldWidth, 12.0, 170.0, 50.0) ];
                        NSString *usr = [[NSUserDefaults standardUserDefaults] objectForKey:@"usr"];
                        if (usr) [user setText:usr];
						[user setAutocapitalizationType:UITextAutocapitalizationTypeNone];
						[user setAutocorrectionType:UITextAutocorrectionTypeNo];
						[user setPlaceholder:NSLocalizedString(@"Required", @"")];
						[cell addSubview: user];
						cell.textLabel.text = NSLocalizedString(@"User Name:", @"");
						[user becomeFirstResponder]; // Set focus
						user.returnKeyType =  UIReturnKeyNext;
						user.delegate = self;

						break;
                    }
					case(1):
                    {
						password = [[UITextField alloc] initWithFrame:CGRectMake(fieldWidth, 12.0, 170.0, 50.0) ]; 
						password.secureTextEntry = YES;
                        NSString *pswd = [[NSUserDefaults standardUserDefaults] objectForKey:@"pswd"];
                        if (pswd) [password setText:pswd];
						[password setAutocapitalizationType:UITextAutocapitalizationTypeNone];
						[password setAutocorrectionType:UITextAutocorrectionTypeNo];
						[password setPlaceholder:NSLocalizedString(@"Required", @"")];
						[cell addSubview: password];
						cell.textLabel.text = NSLocalizedString(@"Password:", @"");
						password.returnKeyType =  UIReturnKeyNext;
						password.delegate = self;
						break;
                    }
				}
				break;
			case(1):
				switch([ indexPath indexAtPosition: 1]) {
					case(0):
					{
						//cell.backgroundView.
						
						//labelForgotPassword.shadowColor = [UIColor grayColor];
						//labelForgotPassword.shadowOffset = CGSizeMake(1,1);
						//labelForgotPassword.textColor = [UIColor blueColor];
						
						UIView* backgroundView = [[[UIView alloc] initWithFrame:CGRectZero] autorelease];
						/*
						backgroundView.backgroundColor = [ UIColor whiteColor ];
						backgroundView.layer.cornerRadius = 10;
						[backgroundView.layer setBorderColor: [[UIColor lightGrayColor] CGColor]];
						[backgroundView.layer setBorderWidth: 1.2];
						 */
						cell.backgroundView = backgroundView;
						//for ( UIView* view in cell.contentView.subviews ) 
						//{
						//	view.backgroundColor = [ UIColor clearColor ];
						//}
						labelForgotPassword = [[IFTweetLabel alloc] initWithFrame:CGRectMake(20.0, 0.0, 300.0, 40.0)];
						[labelForgotPassword setFont:[UIFont systemFontOfSize:14.0]];

						labelForgotPassword.linksEnabled = true;
						//labelForgotPassword.lineBreakMode = UILineBreakModeWordWrap;
						labelForgotPassword.numberOfLines = 2; // 2 lines ; 0 - dynamical number of lines
						
						//labelForgotPassword.text = @"This is a #test of regular expressions with http://example.com links as used in @Twitterrific. HTTP://CHOCKLOCK.COM APPROVED OF COURSE.";
						labelForgotPassword.text = NSLocalizedString(@"If you cannot remember you password and need help to restore it, please click here.", @"");
						labelForgotPassword.backgroundColor = [UIColor clearColor];
						[labelForgotPassword.layer setBorderColor:[[UIColor clearColor] CGColor]];
						
//						loginButton =  [UIButton buttonWithType:UIButtonTypeRoundedRect]; 
//						[loginButton setFrame:CGRectMake(10.0, 0.0, 300.0, 50.0)];		
//						[loginButton setTitle:@"Login" forState:UIControlStateNormal];
//						[loginButton addTarget:self action:@selector(performLogin:) forControlEvents:UIControlEventTouchUpInside];
						[cell addSubview: labelForgotPassword];
						
						break;
					}
					 
				}
				break;
		}
	}
	
    return cell;
	
	
	/*
    static NSString *CellIdentifier = @"Cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
        cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier] autorelease];
    }
    
    // Configure the cell...
    
    return cell;
	*/
}

/*
- (CGFloat)tableView:(UITableView *)tableView heightForHeaderInSection:(NSInteger)section
{
	CGFloat height;
	switch(section)
	{
		case 0:
		{
			height = 100.0;
			break;
		}
	}
	return height;
}
*/

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section {
    switch (section) {
        case 0:
            return @" ";
        default:
            return nil;
    }
}


- (CGFloat)tableView:(UITableView *)tableView heightForHeaderInSection:(NSInteger)section {
    return 30;
}
 
/*
- (UIView *)tableView:(UITableView *)tableView viewForHeaderInSection:(NSInteger)section
{
	
	UIView *label = [[UIView alloc] initWithFrame:CGRectZero]; //CGRectMake(0,0,0,0)];
	return [label autorelease];
}
*/
			

- (void) performLogin:(id)sender
{
	if ([user.text length] == 0) 
		return;
	
	if ([password.text length] == 0) 
	{
		UIAlertView *alert = [[UIAlertView alloc] 
							  initWithTitle:NSLocalizedString(@"Error", @"")
							  message:NSLocalizedString(@"Please, enter password", @"")
							  delegate:self cancelButtonTitle:nil 
							  otherButtonTitles:NSLocalizedString(@"OK", @""), nil]; 
		
		[alert show];
		[alert release];
		[password becomeFirstResponder];
		return;
	}
	
	
	
    [[NSUserDefaults standardUserDefaults] setObject:user.text forKey:@"usr"];
    [[NSUserDefaults standardUserDefaults] setObject:password.text forKey:@"pswd"];
    
    
	//	[loginProgressView setFrame:CGRectMake(15.0f, 125.0f, 290.0f, 40.0f)];
	//	[loginButton setEnabled:NO];
	//[labelForgotPassword setHidden:YES];

	[user setEnabled:YES];
	[password setEnabled:YES];
	
	[self.navigationItem.leftBarButtonItem setEnabled:NO];
	[self.navigationItem.rightBarButtonItem setEnabled:NO];
	//	[self.view addSubview:loginProgressView];
	
	authError = NO;
	[[NSNotificationCenter defaultCenter] addObserver:self 
											 selector:@selector(vpStackNotification:) 
												 name:VPEngineNotification
											   object:nil];		
	
	timer = [NSTimer scheduledTimerWithTimeInterval: 10.0f                    
											 target:self                                                      
										   selector:@selector(loginTimeout)
										   userInfo:nil                                 
											repeats:NO];
	
	
	[[VPEngine sharedInstance] login:user.text password:password.text];
	
	//password.text = @"";
	[[NSUserDefaults standardUserDefaults] setObject:@"" forKey:@"pswd"];
//	[user becomeFirstResponder];

}

- (void) loginTimeout
{
	[timer invalidate];
	timer = nil;
	
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	
	[loginProgressView removeFromSuperview];
	[loginButton setEnabled:YES];
	[user setEnabled:YES];
	[password setEnabled:YES];
	[self.navigationItem.leftBarButtonItem setEnabled:YES];
	[self.navigationItem.rightBarButtonItem setEnabled:YES];
	
	
	UIAlertView *alert = [[UIAlertView alloc] 
							  initWithTitle:NSLocalizedString(@"Error", @"")
							  message:NSLocalizedString(@"Operation timed out", @"")
							  delegate:self cancelButtonTitle:nil 
							  otherButtonTitles:NSLocalizedString(@"OK", @""), nil]; 
		
	[alert show];
	[user becomeFirstResponder]; // Set focus
}


- (BOOL)textFieldShouldReturn:(UITextField *)textField {
	
	if ([user.text length] == 0 ) {
		return false;
	}
	
	//[user becomeFirstResponder];
	
	//[textField resignFirstResponder];
	
	[self performLogin:self];
	
/*	
    [[NSUserDefaults standardUserDefaults] setObject:user.text forKey:@"usr"];
    [[NSUserDefaults standardUserDefaults] setObject:password.text forKey:@"pswd"];
    
    
//	[loginProgressView setFrame:CGRectMake(15.0f, 125.0f, 290.0f, 40.0f)];
//	[loginButton setEnabled:NO];
	[user setEnabled:NO];
	[password setEnabled:NO];
	[self.navigationItem.leftBarButtonItem setEnabled:NO];
	[self.navigationItem.rightBarButtonItem setEnabled:NO];
//	[self.view addSubview:loginProgressView];
	
	authError = NO;
	[[NSNotificationCenter defaultCenter] addObserver:self 
											 selector:@selector(vpStackNotification:) 
												 name:VPEngineNotification
											   object:nil];		
	
	timer = [NSTimer scheduledTimerWithTimeInterval: 10.0f                    
											 target:self                                                      
										   selector:@selector(loginTimeout)
										   userInfo:nil                                 
											repeats:NO];
	
	
	[[VPEngine sharedInstance] login:user.text password:password.text];
*/
	return false;
}
/*
// Override to support conditional editing of the table view.
- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the specified item to be editable.
    return YES;
}
*/


/*
// Override to support editing the table view.
- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath {
    
    if (editingStyle == UITableViewCellEditingStyleDelete) {
        // Delete the row from the data source.
        [tableView deleteRowsAtIndexPaths:[NSArray arrayWithObject:indexPath] withRowAnimation:UITableViewRowAnimationFade];
    }   
    else if (editingStyle == UITableViewCellEditingStyleInsert) {
        // Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view.
    }   
}
*/


/*
// Override to support rearranging the table view.
- (void)tableView:(UITableView *)tableView moveRowAtIndexPath:(NSIndexPath *)fromIndexPath toIndexPath:(NSIndexPath *)toIndexPath {
}
*/


/*
// Override to support conditional rearranging of the table view.
- (BOOL)tableView:(UITableView *)tableView canMoveRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the item to be re-orderable.
    return YES;
}
*/


#pragma mark -
#pragma mark Table view delegate

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    // Navigation logic may go here. Create and push another view controller.
    /*
    <#DetailViewController#> *detailViewController = [[<#DetailViewController#> alloc] initWithNibName:@"<#Nib name#>" bundle:nil];
    // ...
    // Pass the selected object to the new view controller.
    [self.navigationController pushViewController:detailViewController animated:YES];
    [detailViewController release];
    */
}

#pragma mark -
#pragma mark Notifications

- (void) vpStackNotification:(NSNotification*)notification
{		 	
	unsigned msg	= 	[[[notification userInfo] objectForKey:@"MSG"] unsignedIntValue];
	unsigned param1 = 	[[[notification userInfo] objectForKey:@"PARAM1"] unsignedIntValue];
//	unsigned param2 = 	[[[notification userInfo] objectForKey:@"PARAM2"] unsignedIntValue];

	if (timer)
	{
		[timer invalidate];
		timer = nil;
	}
	
	NSString *error = nil;
		
	switch(msg)
	{
		case VPMSG_SERVERSTATUS:
				[labelForgotPassword setHidden:NO];
				if (param1 == REASON_NOTFOUND)
				{
					
					error = NSLocalizedString(@"Wrong user name or password", @"");
				}
				else if (param1 == REASON_AUTHERROR)
				{
					// HACK: ignore if occured first time
					if (authError)
						error = NSLocalizedString(@"Wrong user name or password", @"");
					else 
					{
						authError = YES;
						return;
					}

				} else if (param1 == REASON_NORMAL) {
				}
			break;
        case VPMSG_SERVERTRANSFERFINISHED :
			if (param1 == TCPFT_RXCONTACTS) {
				[VPProfile update:user.text withPassword:password.text];
				
				// Switch to contact list view
				[self dismissModalViewControllerAnimated:YES];
				
				// [self.navigationController pushViewController:tabBarController animated:YES];
				
				//            UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"Logout", @"") style:UIBarButtonItemStylePlain target:nil action:nil];
				//          self.navigationItem.backBarButtonItem = backButton;
				//        [backButton release];
				
				[[NSNotificationCenter defaultCenter] removeObserver:self
																name:VPEngineNotification
															  object:nil];
				
				[[VPContactList sharedInstance] downloadAddressBook];
			}
			break;	

/*			
        case VPMMSG_CONTACTLISTSYNCHRONIZED:
        {
			// Save into database last succsefull login];

			NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
			NSString *file = [NSString stringWithFormat:@"%@/contacts.txt",
									[paths objectAtIndex:0]];
			
			
			//    NSLog(@"Trying to download address book to %@", path);
			
			[[VPEngine sharedInstance] downloadAddressBook:file];
	

            // Switch to contact list view
			[self dismissModalViewControllerAnimated:YES];

           
           // [self.navigationController pushViewController:tabBarController animated:YES];
            
//            UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"Logout", @"") style:UIBarButtonItemStylePlain target:nil action:nil];
  //          self.navigationItem.backBarButtonItem = backButton;
    //        [backButton release];
            
            [[NSNotificationCenter defaultCenter] removeObserver:self
                                                  name:VPEngineNotification
                                                  object:nil];
            
            return;
        }
		break;
*/            
        default:
            return;
	}
    
	

		
	if (error)
	{
        [loginProgressView removeFromSuperview];
        [loginButton setEnabled:YES];
        [user setEnabled:YES];
        [password setEnabled:YES];
        [self.navigationItem.leftBarButtonItem setEnabled:YES];
        [self.navigationItem.rightBarButtonItem setEnabled:YES];
        
        [[NSNotificationCenter defaultCenter] removeObserver:self
                                                        name:VPEngineNotification
                                                      object:nil];
        
		UIAlertView *alert = [[UIAlertView alloc] 
							  initWithTitle:NSLocalizedString(@"Error", @"")
							  message:error
							  delegate:self cancelButtonTitle:nil 
							  otherButtonTitles:NSLocalizedString(@"OK", @""), nil]; 
		
		[alert show];
		[user becomeFirstResponder]; // Set focus
		return;
	}
	
    // Insert (new?) user into database
//    [[VPDatabase sharedInstance] exec:
  //      [NSString stringWithFormat:@"INSERT OR IGNORE INTO user VALUES(NULL, '%@', '%@')",
    //                            [[VPEngine sharedInstance] getLogonName], [[VPEngine sharedInstance] getLogonNumber]]];
    
    // Synch contact list
//    [[VPEngine sharedInstance] downloadAddressBook];    
}

- (void)handleTweetNotification:(NSNotification *)notification
{
	ForgotPasswordViewController *detailViewController = [[ForgotPasswordViewController alloc] initWithNibName:@"ForgotPasswordViewController" bundle:nil];
     // ...
	    // Pass the selected object to the new view controller.
	[self.navigationController pushViewController:detailViewController animated:YES];

	UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"Back", @"") style:UIBarButtonItemStylePlain target:nil action:nil];
    self.navigationItem.backBarButtonItem = backButton;
    [backButton release];
	
	[detailViewController release];
	

}


- (void)registrationComplete:(NSNotification *)notification
{
	
	if ([[notification name] isEqualToString:OnRegistrationCompleteNotification])
        NSLog (@"Successfully received the notification!");
	
	NSDictionary *userInfo = [notification userInfo];
	
	[[NSUserDefaults standardUserDefaults] setObject:[userInfo valueForKey:@"login"] forKey:@"usr"];
    [[NSUserDefaults standardUserDefaults] setObject:[userInfo valueForKey:@"password"] forKey:@"pswd"];
	
	user.text = [userInfo valueForKey:@"login"];
	password.text = [userInfo valueForKey:@"password"];
	
	//OnRegistrationCompleteNotification
	
	[self.tableView	reloadData];
	[self performLogin:self];
}



#pragma mark -
#pragma mark Memory management

- (void)didReceiveMemoryWarning {
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Relinquish ownership any cached data, images, etc. that aren't in use.
}

- (void)viewDidUnload {
    // Relinquish ownership of anything that can be recreated in viewDidLoad or on demand.
    // For example: self.myOutlet = nil;
}


- (void)dealloc {
    [super dealloc];
	
	[[NSNotificationCenter defaultCenter] removeObserver:self
													name:VPEngineNotification
												  object:nil];
	
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)keyboardDidShow:(NSNotification*)notification {
	//The UIWindow that contains the keyboard view
	UIWindow* tempWindow;
	//Because we can’t get access to the UIKeyboard through the SDK we will just use UIView.
	//UIKeyboard is a subclass of UIView anyways
	UIView* keyboard;
	//Check each window in our application
	for(int c = 0; c < [[[UIApplication sharedApplication] windows] count]; c++)
	{
		//Get a reference of the current window
		tempWindow = [[[UIApplication sharedApplication] windows] objectAtIndex:c];
		//Get a reference of the current view
		for(int i = 0; i < [tempWindow.subviews count]; i++)
		{
			keyboard = [tempWindow.subviews objectAtIndex:i];
			if (([[keyboard description] hasPrefix:@"<UIKeyboard"] == YES) || ([[keyboard description] hasPrefix:@"<UIPeripheralHostView"] == YES ))
			{
				//Keyboard is now a UIView reference to the UIKeyboard we want. From here we can add a subview
				//to th keyboard like a new button
				UIButton* done = [UIButton buttonWithType:UIButtonTypeCustom];
				done.frame = CGRectMake(241, 173, 77, 42);
				[done setTitle:NSLocalizedString(@"Login", @"") forState:UIControlStateNormal];
				[done setTitleColor:[UIColor whiteColor] forState:UIControlStateNormal];
				done.titleLabel.font = [UIFont boldSystemFontOfSize:16.0f];
				done.titleLabel.shadowOffset = CGSizeMake(1.0,1.0);
  //          shadowBlur:2.0
				
				[done setBackgroundImage:[UIImage imageNamed:@"login_up.png"] forState:UIControlStateNormal];
				[done setBackgroundImage:[UIImage imageNamed:@"login_down.png"] forState:UIControlStateHighlighted];

				[done addTarget:self action:@selector(performLogin:) forControlEvents:UIControlEventTouchUpInside];
				[keyboard addSubview:done];
				return;
			}
		}
	}
}



/*
- (void)keyboardWillShow:(NSNotification *)note {  
    // create custom button
    UIButton *doneButton = [UIButton buttonWithType:UIButtonTypeCustom];
    doneButton.frame = CGRectMake(0, 163, 106, 53);
    doneButton.adjustsImageWhenHighlighted = NO;
    [doneButton setImage:[UIImage imageNamed:@"login_up.png"] forState:UIControlStateNormal];
    [doneButton setImage:[UIImage imageNamed:@"login_down.png"] forState:UIControlStateHighlighted];
    [doneButton addTarget:self action:@selector(doneButton:) forControlEvents:UIControlEventTouchUpInside];
	
    // locate keyboard view
    UIWindow* tempWindow = [[[UIApplication sharedApplication] windows] objectAtIndex:1];
    UIView* keyboard;
    for(int i=0; i<[tempWindow.subviews count]; i++) 
	{
        keyboard = [tempWindow.subviews objectAtIndex:i];
        // keyboard view found; add the custom button to it
//        if([[keyboard description] hasPrefix:@"<UIKeyboard"] == YES)
  //          [keyboard addSubview:doneButton];
				if([[keyboard description] hasPrefix:@"<UIKeyboard"] == YES)// || [[keyboard description] hasPrefix:@"<UIPeripheralHostView"] == YES )
				{
									//Keyboard is now a UIView reference to the UIKeyboard we want. From here we can add a subview
									//to th keyboard like a new button
								    UIButton* done = [UIButton buttonWithType:UIButtonTypeCustom];
									done.frame = CGRectMake(214, 163, 106, 53);
									done.titleLabel.text = @"Done";
									[done addTarget:self action:@selector(back) forControlEvents:UIControlEventTouchUpInside];
									[keyboard addSubview:done];
//				 	[keyboard addSubview:doneButton];
				}

		
    }
}
*/
/*
— (void)loadView {
	[ super loadView];
	
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(keyboardWillShow:)
												 name:UIKeyboardWillShowNotification
											   object:nil];
	
	// добавим поле для ввода с нашей клавиатурой
	tField = [[ UITextField alloc ] initWithFrame:CGRectMake(200, 185, 110, 30)];
	tField.keyboardType = UIKeyboardTypeNumberPad;
	
	[ self.view addSubview:tField ];
	
}
*/ 
/* Добавление кнопки
В момент появления UIKeyboardTypeNumberPad добавляем кнопку «Done".

— (void)keyboardDidShow:(NSNotification *)note {

// создаем пользовательскую кнопку
UIButton *doneButton = [UIButton buttonWithType:UIButtonTypeCustom];
doneButton.frame = CGRectMake(0, 163, 106, 53);
doneButton.adjustsImageWhenHighlighted = NO;
[doneButton setImage:[UIImage imageNamed:@"doneup.png"] forState:UIControlStateNormal];
[doneButton setImage:[UIImage imageNamed:@"donedown.png"] forState:UIControlStateHighlighted];
[doneButton addTarget:self action:@selector(doneButton:) forControlEvents:UIControlEventTouchUpInside];

// находим представление клавиатуры
UIWindow* tempWindow = [[[UIApplication sharedApplication] windows] objectAtIndex:1];
UIView* keyboard;
for(int i=0; i<[tempWindow.subviews count]; i++) {
keyboard = [tempWindow.subviews objectAtIndex:i];
// keyboard view found; add the custom button to it
if([[keyboard description] hasPrefix:@"<UIKeyboard"] == YES || [[keyboard description] hasPrefix:@"<UIPeripheralHostView"] == YES )
[keyboard addSubview:doneButton];

}
}
Убираем клавиатуру по нажатию  "Done".

— (void)doneButton:(id)sender {
[tField resignFirstResponder];
}
Чистим ресурсы.

— (void)dealloc {
[[NSNotificationCenter defaultCenter] removeObserver:self];
[ tField release ];
[super dealloc];
}

*/
@end


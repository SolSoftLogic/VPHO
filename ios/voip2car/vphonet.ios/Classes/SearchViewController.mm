//
//  SearchViewController.m
//  Vphonet
//
//  Created by uncle on 20.04.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import "VphonetAppDelegate.h"
#import "VPEngine.h"
#import "VPContactList.h"
#import "SearchViewController.h"


@implementation SearchViewController

@synthesize cellSearchField;
@synthesize selectedCountry;

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
    [cellTextField release]; cellTextField = nil;
	
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

	self.title = NSLocalizedString(@"Search", @"");
	[self setCustomBackground:@"background.png"];
	

    [[NSNotificationCenter defaultCenter] addObserver:self 
											 selector:@selector(onSearchNotification:) 
												 name:OnSearchNotification
											   object:nil];
	
	
    [[NSNotificationCenter defaultCenter] addObserver:self 
											 selector:@selector(vpEngineNotification:) 
												 name:VPEngineNotification
											   object:nil];

	
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
    return 3;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    // Return the number of rows in the section.
	switch (section) {
		case 0:
			return 2;
			break;
		case 1:
			return 1;
			break;
	}
    return 0;
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section
{
	switch (section) {
		case 0:
			return NSLocalizedString(@"Search for a contact:", @"");
			break;
		case 2:
			return NSLocalizedString(@"You can search by Full Name,\nE-mail, Vphonet Name or Vphonet Number.", @"");
			break;
			//		case 1:
			//			return @" ";
			//			break;
			
		default:
			break;
	}
	return @"";
}

/*
- (NSString *)tableView:(UITableView *)tableView titleForFooterInSection:(NSInteger)section
{
	switch (section) {
		case 1:
			return NSLocalizedString(@"You can search by Full Name,\nE-mail, Vphonet Name or Vphonet Number.", @"");
			break;
		default:
			break;
	}
	return @"";
}
*/

/*
- (UIView *)tableView:(UITableView *)tableView viewForHeaderInSection:(NSInteger)section {
	UIView *containerView = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 320, 40)];
	containerView.backgroundColor = [UIColor clearColor];
	CGRect labelFrame = CGRectMake(20, 2, 320, 30);
	labelFrame.origin.y = 13;
	UILabel *label = [[UILabel alloc] initWithFrame:labelFrame];
	label.backgroundColor = [UIColor clearColor];
	label.font = [UIFont boldSystemFontOfSize:17];
	if(section == 2) {
		label.font = [UIFont boldSystemFontOfSize:14];
		label.numberOfLines = 2;
	}
	label.shadowColor = [UIColor colorWithWhite:1.0 alpha:1];
	label.shadowOffset = CGSizeMake(0, 1);
	label.textColor = [UIColor colorWithRed:0.265 green:0.294 blue:0.367 alpha:1.000];
	label.text = [self tableView:tableView titleForHeaderInSection:section];
	label.lineBreakMode = UILineBreakModeWordWrap;
	label.textAlignment = UITextAlignmentLeft;
	[containerView addSubview:label];
	
	return containerView;
}
*/ 
 
/*
- (CGFloat)tableView:(UITableView *)tableView heightForFooterInSection:(NSInteger)section {
	if(section == 0)
		return 47+15;
	else {
		return 5;
	}
	
}
*/


/*
- (UIView *)tableView:(UITableView *)tableView viewForHeaderInSection:(NSInteger)section
{
	// create the parent view that will hold header Label
	UIView* customView = [[UIView alloc] initWithFrame:CGRectMake(10.0, 0.0, 300.0, 44.0)];
	
	// create the button object
	UILabel * headerLabel = [[UILabel alloc] initWithFrame:CGRectZero];
	headerLabel.backgroundColor = [UIColor clearColor];
	headerLabel.opaque = NO;
	headerLabel.textColor = [UIColor blackColor];
	headerLabel.highlightedTextColor = [UIColor whiteColor];
	headerLabel.font = [UIFont boldSystemFontOfSize:20];
	headerLabel.frame = CGRectMake(10.0, 0.0, 300.0, 44.0);
	
	// If you want to align the header text as centered
	// headerLabel.frame = CGRectMake(150.0, 0.0, 300.0, 44.0);
	
//	headerLabel.text = <Put here whatever you want to display> // i.e. array element
	[customView addSubview:headerLabel];
	
	return customView;
}
*/

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *cellIdentifier = @"сell";

	UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:cellIdentifier];
	if (cell == nil) {
		cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:cellIdentifier] autorelease];
	}
    
	UITableViewCell *result = nil;
	
	if (indexPath.section == 0)
	{
		if (indexPath.row == 0) {
			
			if(cellTextField == nil) {
				NSArray *topLevelObjects = [[NSBundle mainBundle] loadNibNamed:@"UITableViewTextFieldCell" owner:nil options:nil];
				for(id currentObject in topLevelObjects)
				{
					if([currentObject isKindOfClass:[UITableViewTextFieldCell class]])
					{
						cellTextField = (UITableViewTextFieldCell *)currentObject;
						break;
					}
				}
			}

			[cellTextField retain];
			cellTextField.selectionStyle = UITableViewCellSelectionStyleNone;
			return cellTextField;
			
		}
		else if (indexPath.row == 1) {
			if ([selectedCountry length] == 0)
				cell.textLabel.text = NSLocalizedString(@"Choose country", @"");
			else
				cell.textLabel.text = selectedCountry;
				
			cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
			cell.selectionStyle = UITableViewCellSelectionStyleBlue; 
		}
		result = cell;
	} else {
		if(cellButton == nil) {
			NSArray *topLevelObjects = [[NSBundle mainBundle] loadNibNamed:@"UITableViewButtonCell" owner:nil options:nil];
			for(id currentObject in topLevelObjects)
			{
				if([currentObject isKindOfClass:[UITableViewButtonCell class]])
				{
					cellButton = (UITableViewButtonCell *)currentObject;
					UIView *backView = [[[UIView alloc] initWithFrame:CGRectZero] autorelease];
					backView.backgroundColor = [UIColor clearColor];
					cellButton.backgroundView = backView;
					[backView release];
					break;
				}
			}
		}
		
		[cellButton.button setTitle:NSLocalizedString(@"Search", @"") forState:UIControlStateNormal];

		[cellButton.button addTarget:self action:@selector(search:) forControlEvents:UIControlEventTouchUpInside];
		
		[cellButton retain];
		cellButton.selectionStyle = UITableViewCellSelectionStyleNone;
		
		return cellButton; 
		result = cell;
	}
	
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
	
	if ((indexPath.section == 0) && (indexPath.row == 1)) {
		[[[self tableView] cellForRowAtIndexPath:indexPath] setSelected:YES animated:YES];

		CountryListViewController *countryViewController = [[CountryListViewController alloc] initWithNibName:@"CountryListViewController" bundle:nil];
		countryViewController.delegate = self;
		UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"Back", @"") style: UIBarButtonItemStyleBordered target: nil action: nil];
		[[self navigationItem] setBackBarButtonItem: backButton];
		[backButton release];
		[self.navigationController pushViewController:countryViewController animated:YES];
		[countryViewController release];
		[[[self tableView] cellForRowAtIndexPath:indexPath] setSelected:NO animated:YES];
		return;
	}
}

- (void)countryListViewController:(CountryListViewController *)countryListViewController didSelectCountry:(Country *)country {

    if (country) {
		selectedCountry = [NSString stringWithString:country.name];
		[self.tableView reloadData];
    }

}

/*
int UserSearch(const char *name, const char *country, unsigned flags, unsigned param);
*/

- (void) search:(id)sender {
	
	if ([cellTextField.textField.text length] == 0) {
		return;
	}
	
	[[VPEngine sharedInstance] userSearch:[NSString stringWithString:cellTextField.textField.text] inCountry:nil];
}

#pragma mark -
#pragma mark Notifications

- (void) onSearchNotification:(NSNotification*)notification
{
	NSString *str = [[notification userInfo] objectForKey:@"result"];

	NSLog(@"Search result notification (%d): %@", [str length],str);
	
	if ([str length] == 0) {
		UIAlertView *alert = [[UIAlertView alloc] 
							  initWithTitle:NSLocalizedString(@"Search", @"")
							  message:NSLocalizedString(@"Users not found, search again.", @"")
							  delegate:self cancelButtonTitle:nil 
							  otherButtonTitles:NSLocalizedString(@"OK", @""), nil]; 
		[alert show];
		[alert release];
		
		
	} else {
		UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"Back", @"") style: UIBarButtonItemStyleBordered target: nil action: nil];
		[[self navigationItem] setBackBarButtonItem: backButton];
		[backButton release];
		
		SearchResultViewController *searchResultViewController = [[SearchResultViewController alloc] initWithNibName:@"SearchResultViewController" bundle:nil];
		[searchResultViewController loadContacts:str];
		//	searchResultViewController.delegate = self;
		//		countryViewController.registrationInfo = registrationInfo;
		[self.navigationController pushViewController:searchResultViewController animated:YES];
	}
	
}

- (void) vpEngineNotification:(NSNotification*)notification
{		 	
/*	
	unsigned msg	= 	[[[notification userInfo] objectForKey:@"MSG"] unsignedIntValue];
	unsigned param1 = 	[[[notification userInfo] objectForKey:@"PARAM1"] unsignedIntValue];
	unsigned param2 = 	[[[notification userInfo] objectForKey:@"PARAM2"] unsignedIntValue];
    
    switch(msg)
	{
		case VPMSG_USERSEARCH:
        {
//			NSString *str1 = [NSString stringWithCString:(const char*)param1 encoding:NSUTF8StringEncoding];
			NSString *str = [NSString stringWithCString:(const char*)param2 encoding:NSUTF8StringEncoding];
//			NSLog(@"Search utf8 %@", str1);
			NSLog(@"Search result (%d): %@", [str length],str);
			
			if ([str length] == 0) {
				UIAlertView *alert = [[UIAlertView alloc] 
									  initWithTitle:NSLocalizedString(@"Search", @"")
									  message:NSLocalizedString(@"Users not found, search again.", @"")
									  delegate:self cancelButtonTitle:nil 
									  otherButtonTitles:NSLocalizedString(@"OK", @""), nil]; 
				[alert show];
				[alert release];

				
			} else {
				UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"Back", @"") style: UIBarButtonItemStyleBordered target: nil action: nil];
				[[self navigationItem] setBackBarButtonItem: backButton];
				[backButton release];
				
				SearchResultViewController *searchResultViewController = [[SearchResultViewController alloc] initWithNibName:@"SearchResultViewController" bundle:nil];
				[searchResultViewController loadContacts:str];
				//	searchResultViewController.delegate = self;
				//		countryViewController.registrationInfo = registrationInfo;
				[self.navigationController pushViewController:searchResultViewController animated:YES];
			}
        }
			break;  
    }
*/    
}




@end

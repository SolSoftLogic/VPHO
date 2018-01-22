//
//  FullProfileViewController.mm
//  Vphonet
//
//  Created by uncle on 19.02.11.
//  Copyright 2011. All rights reserved.
//

#import "VphonetAppDelegate.h"
#import "VPProfile.h"
#import "PhonesViewController.h"
#import "GenderViewController.h"
#import "BirthdayViewController.h"
#import "CountryListViewController.h"
#import "StateViewController.h"
#import "FullNameViewController.h"
#import "FullProfileViewController.h"


@implementation FullProfileViewController

@synthesize tableHeaderView;
@synthesize photoView;
@synthesize buttonName;
@synthesize backgroundHeader;


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

- (void)viewDidLoad
{
    [super viewDidLoad];
	
	self.tableView = nil;   
    UITableView *newTableView = [[UITableView alloc] initWithFrame:CGRectZero style:UITableViewStyleGrouped];
//	newTableView.scrollEnabled = NO;
    self.tableView = newTableView; 
    [newTableView release];
	
	profile = [VPProfile singleton];
	
	//self.tableView.
	[self updatePhoto];
	
    // Uncomment the following line to display an Edit button in the navigation bar for this view controller.
	self.navigationItem.rightBarButtonItem = self.editButtonItem;
	//self.editButtonItem.action = @selector(done:);
	
//	self.tableView.style = UITableViewStyleGrouped;
	
	if (tableHeaderView == nil) {
		[[NSBundle mainBundle] loadNibNamed:@"FullProfileHeaderView" owner:self options:nil];
        self.tableView.tableHeaderView = tableHeaderView;
		self.tableView.tableHeaderView.backgroundColor = [UIColor clearColor];
        self.tableView.allowsSelectionDuringEditing = YES;
//		self.tableView.backgroundColor = [UIColor clearColor];	
    }
	
	[VphonetAppDelegate setButtonYellowBorder: photoView];
	[VphonetAppDelegate setButtonYellowBorder: backgroundHeader];
	[VphonetAppDelegate setButtonYellowBorder: buttonName];


	[self setTitle:NSLocalizedString(@"Full Profile", @"")];
	[self setCustomBackground:@"background.png"];
	[self.tableView setSeparatorColor:[UIColor paleYellowColor]];


}

- (void)updatePhoto {
//	self.labelLastName.text = [VPProfile singleton].lastName;
//	self.labelFirstName.text = [VPProfile singleton].firstName;
	
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *file = [NSString stringWithFormat:@"%@/photos/profile.jpg",[paths objectAtIndex:0]];
	NSFileManager *fm = [NSFileManager defaultManager]; 
	
	if ([fm fileExistsAtPath:file])
		//	[photoView.imageView setImage:[UIImage imageWithData:imageData]];
		[photoView setBackgroundImage:[UIImage imageWithContentsOfFile:file] forState:UIControlStateNormal];
	else
		[photoView setBackgroundImage:[UIImage imageNamed:@"profile_picture.png"] forState:UIControlStateNormal];
	
	[[VPEngine sharedInstance] updatePhoto:file];
	
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
	[self.tableView reloadData];
	[self updatePhoto];

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
    return 5;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    // Return the number of rows in the section.
    return 1;
}

- (void)setEditing:(BOOL)editing animated:(BOOL)animated {
	[super setEditing:editing animated:YES];
	[self.tableView reloadData];
}

- (CGFloat)tableView:(UITableView *)tableView heightForHeaderInSection:(NSInteger)section
{
	return 0.5;
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
	return 44.0;
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section 
{
	return @" ";
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *CellIdentifier = @"UITableViewFieldCell";
	
	if (([profile.firstName length] != 0) && ([profile.lastName length] != 0)) {
		[buttonName setTitle:[NSString stringWithFormat:@"%@\n%@", profile.firstName, profile.lastName] forState:UIControlStateNormal];
	} else if ([profile.firstName length] != 0) {
		[buttonName setTitle:[NSString stringWithFormat:@"%@", profile.firstName] forState:UIControlStateNormal];		
	} else if ([profile.lastName length] != 0) {
		[buttonName setTitle:[NSString stringWithFormat:@"%@", profile.lastName] forState:UIControlStateNormal];
	}
	
	
	//
	//
	//
	//
    
//    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
//    if (cell == nil) {
//        cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier] autorelease];
//    }
	
	UITableViewFieldCell *cell = (UITableViewFieldCell*)[tableView dequeueReusableCellWithIdentifier:CellIdentifier];
	if(cell == nil) {
		NSArray *topLevelObjects = [[NSBundle mainBundle] loadNibNamed:@"UITableViewFieldCell" owner:nil options:nil];
		for(id currentObject in topLevelObjects)
		{
			if([currentObject isKindOfClass:[UITableViewFieldCell class]])
			{
				cell = (UITableViewFieldCell *)currentObject;
				break;
			}
		}
	}
	

	
	//cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
	//VPProfile *profile = [VPProfile singleton];

//	if (tableView.editing) {
		switch (indexPath.section) {
			case 0:
			{
				if(cellPhone == nil) {
					cellPhone = cell;
//					NSArray *topLevelObjects = [[NSBundle mainBundle] loadNibNamed:@"UITableViewFieldCell" owner:nil options:nil];
//					for(id currentObject in topLevelObjects)
//					{
//						if([currentObject isKindOfClass:[UITableViewFieldCell class]])
//						{
//							cellPhone = (UITableViewFieldCell *)currentObject;
//							break;
//						}
//					}
				}
				
				[cellPhone retain];
				cellPhone.label.text = NSLocalizedString(@"Phone:", @"");
				cellPhone.textField.placeholder = NSLocalizedString(@"Add new Phone", @"");
				cellPhone.textField.enabled = false;
				cellPhone.textField.text = [profile phoneHome];
				cellPhone.selectionStyle = UITableViewCellSelectionStyleBlue;
				cellPhone.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
				//			cellFullName.textField.placeholder = NSLocalizedString(@"Type a phone number", @"");
				return cellPhone;
			}
				break;
			case 1:
			{
				if(cellBirthday == nil) {
					cellBirthday = cell;
//					NSArray *topLevelObjects = [[NSBundle mainBundle] loadNibNamed:@"UITableViewFieldCell" owner:nil options:nil];
//					for(id currentObject in topLevelObjects)
//					{
//						if([currentObject isKindOfClass:[UITableViewFieldCell class]])
//						{
//							cellBirthday = (UITableViewFieldCell *)currentObject;
//							break;
//						}
//					}
				}
				
				[cellBirthday retain];
				cellBirthday.label.text =  NSLocalizedString(@"Birthday:", @"");
				cellBirthday.textField.placeholder = NSLocalizedString(@"Add Birthday", @"");
				cellBirthday.textField.enabled = false;
				
				
				
				if ([profile.birthday length] != 0) {
					NSDateFormatter *dateFormatterSection = [[[NSDateFormatter alloc] init] autorelease];
					[dateFormatterSection setDateStyle:NSDateFormatterNoStyle];
					[dateFormatterSection setDateFormat:@"yyyyMMdd"];
					NSDate *date = [dateFormatterSection dateFromString:profile.birthday];
					
					NSDateFormatter *dateFormatter = [[[NSDateFormatter alloc] init] autorelease];
					[dateFormatter setDateStyle:NSDateFormatterMediumStyle];
					[dateFormatter setTimeStyle:NSDateFormatterNoStyle];
					cellBirthday.textField.text = [dateFormatter stringFromDate:date];
					
				}
				
				
				cellBirthday.selectionStyle = UITableViewCellSelectionStyleBlue;
				cellBirthday.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
				//			cellFullName.textField.placeholder = NSLocalizedString(@"Type a phone number", @"");
				return cellBirthday;
//				cell.textLabel.text = NSLocalizedString(@"Birthday", @"");
				break;
			}
			case 2:
				if(cellGender == nil) {
					cellGender = cell;
				}
				
				[cellGender retain];
				cellGender.label.text =  NSLocalizedString(@"Gender:", @"");
				cellGender.textField.placeholder = NSLocalizedString(@"Add Gender", @"");
				cellGender.textField.enabled = false;
				
				if ([[profile gender] isEqualToString:@"M"]) {
					cellGender.textField.text = NSLocalizedString(@"Male", @"");
				} else if ([[profile gender] isEqualToString:@"F"]) {
					cellGender.textField.text = NSLocalizedString(@"Female", @"");
				} else {
					cellGender.textField.text = @"";
				}
				
				cellGender.selectionStyle = UITableViewCellSelectionStyleBlue;
				cellGender.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
				return cellGender;
				break;
			case 3:
				if(cellCountry == nil) {
					cellCountry = cell;
				}
				
				[cellCountry retain];
				cellCountry.label.text =  NSLocalizedString(@"Country:", @"");
				cellCountry.textField.placeholder = NSLocalizedString(@"Select country", @"");
				cellCountry.textField.enabled = false;
				cellCountry.textField.text = [profile countryName];
				cellCountry.selectionStyle = UITableViewCellSelectionStyleBlue;
				cellCountry.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
				return cellCountry;
				break;
			case 4:
				if(cellState == nil) {
					cellState = cell;
				}
				
				[cellState retain];
				cellState.label.text =  NSLocalizedString(@"State:", @"");
				cellState.textField.placeholder = NSLocalizedString(@"Select state", @"");
				cellState.textField.enabled = false;
				cellState.textField.text = [profile state];
				cellState.selectionStyle = UITableViewCellSelectionStyleBlue;
				cellState.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
				return cellState;
				break;
			default:
				break;
		}
/*	} else
	{
		switch (indexPath.section) {
			case 0:
				cell.textLabel.text = NSLocalizedString(@"1", @"");
				break;
			case 1:
				cell.textLabel.text = NSLocalizedString(@"2", @"");
				break;
			case 2:
				cell.textLabel.text = NSLocalizedString(@"3", @"");
				break;
			case 3:
				cell.textLabel.text = NSLocalizedString(@"4", @"");
				break;
			case 4:
				cell.textLabel.text = NSLocalizedString(@"5", @"");
				break;
				
			default:
				break;
		}
	}
 */		
    
    // Configure the cell...
    
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

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    // Navigation logic may go here. Create and push another view controller.
//	if (indexPath.section == 0) {
		switch (indexPath.section) {
			case 0:
			{
//				UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"Back", @"") style: UIBarButtonItemStyleBordered target: nil action: nil];
//				[[self navigationItem] setBackBarButtonItem: backButton];
//				[backButton release];
				
				PhonesViewController *viewController = [[PhonesViewController alloc] initWithStyle:UITableViewStyleGrouped];
				[self.navigationController pushViewController:viewController animated:YES];
				[viewController release];
			}	
				break;
			case 1:
			{
//				UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"Back", @"") style: UIBarButtonItemStyleBordered target: nil action: nil];
//				[[self navigationItem] setBackBarButtonItem: backButton];
//				[backButton release];

				BirthdayViewController *detailViewController = [[BirthdayViewController alloc] initWithNibName:@"BirthdayViewController" bundle:nil];
				[self.navigationController pushViewController:detailViewController animated:YES];
				[detailViewController release];
			}
				break;
			case 2:
			{
//				UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"Back", @"") style: UIBarButtonItemStyleBordered target: nil action: nil];
//				[[self navigationItem] setBackBarButtonItem: backButton];
//				[backButton release];

				GenderViewController *viewController = [[GenderViewController alloc] initWithStyle:UITableViewStyleGrouped];
				[self.navigationController pushViewController:viewController animated:YES];
				[viewController release];
			}
				break;
			case 3:
			{
//				UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"Back", @"") style: UIBarButtonItemStyleBordered target: nil action: nil];
//				[[self navigationItem] setBackBarButtonItem: backButton];
//				[backButton release];

				CountryListViewController *viewController = [[CountryListViewController alloc] initWithNibName:@"CountryListViewController" bundle:nil];
				viewController.delegate = self;
				
				[self.navigationController pushViewController:viewController animated:YES];
				[viewController release];
			}
				break;
			case 4:
			{
//				UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"Back", @"") style: UIBarButtonItemStyleBordered target: nil action: nil];
//				[[self navigationItem] setBackBarButtonItem: backButton];
//				[backButton release];
				
				StateViewController *viewController = [[StateViewController alloc] initWithStyle:UITableViewStyleGrouped];
				[self.navigationController pushViewController:viewController animated:YES];
				[viewController release];
			}	
				break;
	

			default:
				break;
		}
//	}
}

- (UITableViewCellEditingStyle)tableView:(UITableView *)tableView editingStyleForRowAtIndexPath:(NSIndexPath *)indexPath {
 //   NSArray* items = ...;
	
   // if( indexPath.row == [items count] )
        return UITableViewCellEditingStyleInsert;
	
   // return UITableViewCellEditingStyleDelete;
}


- (void) done:(id)sender 
{
	if (self.tableView.editing) {
		[self.tableView setEditing:YES animated:YES];
		[self.navigationController popViewControllerAnimated:YES];
	} else {
		[self.tableView setEditing:YES animated:YES];
		[self setEditing:YES];
	}
}

- (IBAction) selectName:(id)sender
{
	//UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"Back", @"") style: UIBarButtonItemStyleBordered target: nil action: nil];
//	[[self navigationItem] setBackBarButtonItem: backButton];
//	[backButton release];
	
	FullNameViewController *viewController = [[FullNameViewController alloc] initWithStyle:UITableViewStyleGrouped];
	[self.navigationController pushViewController:viewController animated:YES];
	[viewController release];
	
}


- (void)countryListViewController:(CountryListViewController *)countryListViewController didSelectCountry:(Country *)country {

    if (country) {  
		profile.country = [NSString stringWithString:country.iso2];
		profile.countryName = [NSString stringWithString:country.name]; 
		
		[self.tableView reloadData];
    }
    
    // Dismiss the modal add recipe view controller
	//    [self dismissModalViewControllerAnimated:YES];
}





@end

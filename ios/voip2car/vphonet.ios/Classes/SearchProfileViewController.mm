//
//  SearchProfileViewController.m
//  Vphonet
//
//  Created by uncle on 02.05.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import "VphonetAppDelegate.h"
#import "CellProfileHeader.h"
#import "SearchProfileViewController.h"
#import "UITableViewFieldCell.h"
#import "CountryListViewController.h"


@implementation SearchProfileViewController

@synthesize contact;
@synthesize withAddContact;


- (id)initWithNibName:(NSString*)aNibName bundle:(NSBundle*)aBundle
{
    self = [super initWithNibName:aNibName bundle:aBundle]; // The UIViewController's version of init
    if (self) {
		withAddContact = true;
    }
    return self;
}

- (void)dealloc
{
	if (cellProfileHeader)
	{
		[cellProfileHeader release];
		cellProfileHeader = nil;
	}

	if (cellState) {
		[cellState release];
		cellState = nil;
	}
	
	if (cellBirthday) {
		[cellBirthday release];
		cellBirthday = nil;
	}

	if (cellCountry) {
		[cellCountry release];
		cellCountry = nil;
	}

	if (cellGender) {
		[cellGender release];
		cellGender = nil;
	}
	
	/*
	if (cellButtonAdd) {
		[cellButtonAdd release];
		cellButtonAdd = nil;
	}
	 */
	
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


	self.tableView = nil;   
    UITableView *newTableView = [[UITableView alloc] initWithFrame:CGRectZero style:UITableViewStyleGrouped];
	newTableView.scrollEnabled = NO;
    self.tableView = newTableView; 
	[self.tableView setSeparatorColor:[UIColor paleYellowColor]];
    [newTableView release];
	
	self.tableView.backgroundColor = [UIColor clearColor];	

	/*
    if (tableHeaderView == nil) {
		
		[[NSBundle mainBundle] loadNibNamed:@"ProfileHeaderView" owner:self options:nil];
        self.tableView.tableHeaderView = tableHeaderView;
        self.tableView.allowsSelectionDuringEditing = YES;
		self.tableView.backgroundColor = [UIColor clearColor];
		//self.labelLastName.text = profile.lastName;
		//self.labelFirstName.text = profile.firstName;
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
	
	self.title = [NSString stringWithFormat:@"%@ %@", contact.firstName, contact.lastName];//NSLocalizedString(@"Search Result", @"");

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
    return  withAddContact ? 3 : 2;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    // Return the number of rows in the section.
	switch (section) {
		case 0: return 1;
		case 1: return 4;
		case 2: return 1;
	}
	
    return 0;
}

- (CGFloat)tableView:(UITableView *)tableView heightForHeaderInSection:(NSInteger)section {
	switch (section) {
		case 0: return 5;
		case 1: return 2;
		case 2: return 2;
	}
	return 0;
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section {
	
	return @" ";
	/*
	switch (section) {
		case 1: 
			return @"Forward your calls when your Account is:";
			break;
		case 2: 
			return @"Phone Number:";
			break;
		case 3: 
			return @"You need to purchase Calling Credit to forward calls to regular phones";
			break;
			
		default:
			break;
	}
	return nil;
	*/ 
}


- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
	
	
	switch(indexPath.section)
	{
		case 0:
			if (indexPath.row == 0) {
				if(cellProfileHeader == nil) {
					NSArray *topLevelObjects = [[NSBundle mainBundle] loadNibNamed:@"CellProfileHeader" owner:nil options:nil];
					for(id currentObject in topLevelObjects)
					{
						if([currentObject isKindOfClass:[CellProfileHeader class]])
						{
							cellProfileHeader = (CellProfileHeader *)currentObject;
							
							[VphonetAppDelegate setButtonYellowBorder:cellProfileHeader.backgroundHeader];
							[VphonetAppDelegate setButtonYellowBorder:cellProfileHeader.backgroundName];
							break;
						}
					}
				}

				[cellProfileHeader retain];
				cellProfileHeader.selectionStyle = UITableViewCellSelectionStyleNone;
				
				
				[cellProfileHeader.labelName setText:[NSString stringWithFormat:@"%@ %@", contact.firstName, contact.lastName]];
				[cellProfileHeader.labelEMail setText: contact.email];
				
				
				
//				[UIImage imageNamed:@"profile_picture.png"];
				[[cellProfileHeader statusImage] setImage:[UIImage imageNamed:[contact statusImageName]]];
				[[cellProfileHeader statusLabel] setText:[NSString stringWithString:[contact statusName]]];
				
				NSString* photoName = [contact photoImageName];
				
				[VphonetAppDelegate setButtonYellowBorder:cellProfileHeader.buttonPhoto];

				if ([photoName length] != 0) {
					[cellProfileHeader.buttonPhoto setBackgroundImage:[UIImage imageWithContentsOfFile:photoName] forState:UIControlStateNormal];
				} else 
				{
					[cellProfileHeader.buttonPhoto setBackgroundImage:[UIImage imageNamed:@"profile_picture.png"] forState:UIControlStateNormal];
				}

				
				
				return cellProfileHeader;
			}
			break;
		case 1:
		{
			static NSString *cellIdentifier = @"UITableViewFieldCell";
			UITableViewFieldCell* cell = (UITableViewFieldCell *)[tableView dequeueReusableCellWithIdentifier:cellIdentifier];
			if (cell == nil) {
				NSArray *topLevelObjects =[[NSBundle mainBundle] loadNibNamed:@"UITableViewFieldCell" owner:nil options:nil];
				for(id currentObject in topLevelObjects)
				{
					if([currentObject isKindOfClass:[UITableViewFieldCell class]])
					{
						cell = (UITableViewFieldCell *)currentObject;
						break;
					}
				}

			}
			cell.selectionStyle = UITableViewCellSelectionStyleNone;
			[cell.textField setEnabled:NO];
			[cell.textField setTextAlignment:UITextAlignmentRight];
			
			if (indexPath.row == 0)
			{
				VPCountry* country = [[VPCountry alloc] initFromDatabase:contact.country];
				cellCountry = cell;
				cellCountry.label.text = NSLocalizedString(@"Country:", @"");
				[cellCountry.textField setText:country.name];
				[cellCountry retain];
				[country release];
				return cellCountry;
			} else if (indexPath.row == 1)
			{
				cellState = cell;
				cellState.label.text = NSLocalizedString(@"State:", @"");
				[cellState.textField setText:contact.state];
				[cellState retain];
				return cellState;
			} else if (indexPath.row == 2)
			{
				cellBirthday = cell;
				cellBirthday.label.text = NSLocalizedString(@"Bithday:", @"");
				
				if ([contact.birthday length] != 0) {
					NSDateFormatter *dateFormatterSection = [[[NSDateFormatter alloc] init] autorelease];
					[dateFormatterSection setDateStyle:NSDateFormatterNoStyle];
					[dateFormatterSection setDateFormat:@"yyyyMMdd"];
					NSDate *date = [dateFormatterSection dateFromString:contact.birthday];
					
					NSDateFormatter *dateFormatter = [[[NSDateFormatter alloc] init] autorelease];
					[dateFormatter setDateStyle:NSDateFormatterMediumStyle];
					[dateFormatter setTimeStyle:NSDateFormatterNoStyle];
					cellBirthday.textField.text = [dateFormatter stringFromDate:date];
				}

				[cellBirthday retain];
				return cellBirthday;
			} else if (indexPath.row == 3)
			{
				cellGender = cell;
				cellGender.label.text = NSLocalizedString(@"Gender:", @"");

				if ([contact.gender isEqualToString:@"M"]) {
					[cellGender.textField setText:NSLocalizedString(@"Male", @"")];
				} else if ([contact.gender isEqualToString:@"F"]) {
					[cellGender.textField setText:NSLocalizedString(@"Female", @"")];
				}
				[cellGender retain];
				return cellGender;
			}
		}
			break;
		case 2:
			if (indexPath.row == 0)
			{
				static NSString *cellIdentifier = @"AddButtonCell";
				cellButtonAdd = (UITableViewButtonCell *)[tableView dequeueReusableCellWithIdentifier:cellIdentifier];
				if(cellButtonAdd == nil) {
					NSArray *topLevelObjects = [[NSBundle mainBundle] loadNibNamed:@"UITableViewButtonCell" owner:nil options:nil];
					for(id currentObject in topLevelObjects)
					{
						if([currentObject isKindOfClass:[UITableViewButtonCell class]])
						{
							cellButtonAdd = (UITableViewButtonCell *)currentObject;
							UIView *backView = [[[UIView alloc] initWithFrame:CGRectZero] autorelease];
							backView.backgroundColor = [UIColor clearColor];
							cellButtonAdd.backgroundView = backView;
							[backView release];
							break;
						}
					}
				}
				[cellButtonAdd.button addTarget:self action:@selector(onAdd:) forControlEvents:UIControlEventTouchUpInside];
				[cellButtonAdd.button setTitle:NSLocalizedString(@"Add to Contact", @"") forState:UIControlStateNormal];
				[cellButtonAdd retain];
				cellButtonAdd.selectionStyle = UITableViewCellSelectionStyleNone;
				return cellButtonAdd;
			}			
	}
    
    
    return nil;
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
	if ((indexPath.section == 0) && (indexPath.row == 0)) {
		return 110;
	}
	
	return 44;
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


- (void) onAdd:(id)sender {
	
	if ([contact.number isEqualToString:[[VPProfile singleton] number]]) { 
		return; // Don't add self.
	}
	
	[[VPContactList sharedInstance] addContact:contact];
	
	
	[self.navigationController popToRootViewControllerAnimated:YES];// popToRootViewControllerAnimated:YES];
}



#pragma mark - Table view delegate

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
}

@end

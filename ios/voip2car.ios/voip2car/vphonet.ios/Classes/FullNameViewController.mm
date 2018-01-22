//
//  FullNameViewController.m
//  Vphonet
//
//  Created by uncle on 14.05.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import "VphonetAppDelegate.h"
#import "FullNameViewController.h"


@implementation FullNameViewController

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
	
	[self setCustomBackground:@"background.png"];
	[self.tableView setSeparatorColor:[UIColor paleYellowColor]];

	
	[self setTitle:NSLocalizedString(@"First Name", @"")];
	
	profile = [VPProfile singleton];
	
	self.title = NSLocalizedString(@"State", @"");
	self.navigationItem.rightBarButtonItem  = [[[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"Done", @"")
																				style: UIBarButtonItemStyleDone
																			   target: self 
																			   action:@selector(done:)] autorelease];


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
	//[self.tableView reloadData];
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

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section {
	
	if (section == 0) {
		return NSLocalizedString(@"First Name:", @"");
	}
	
	return NSLocalizedString(@"Last Name:", @"");
}


- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    // Return the number of rows in the section.
    return 1;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
//    static NSString *CellIdentifier = @"Cell";
    
//    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
//    if (cell == nil) {
//        cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier] autorelease];
//    }
	
	if (indexPath.section == 0) {
		if(cellFirstName == nil) {
			NSArray *topLevelObjects = [[NSBundle mainBundle] loadNibNamed:@"UITableViewTextFieldCell" owner:nil options:nil];
			for(id currentObject in topLevelObjects)
			{
				if([currentObject isKindOfClass:[UITableViewTextFieldCell class]])
				{
					cellFirstName = (UITableViewTextFieldCell *)currentObject;
					break;
				}
			}
		}
		cellFirstName.textField.text = [NSString stringWithString:profile.firstName];
		
		[cellFirstName retain];
		cellFirstName.selectionStyle = UITableViewCellSelectionStyleNone;
		return cellFirstName;
	} else if (indexPath.section == 1) {
		if(cellLastName == nil) {
			NSArray *topLevelObjects = [[NSBundle mainBundle] loadNibNamed:@"UITableViewTextFieldCell" owner:nil options:nil];
			for(id currentObject in topLevelObjects)
			{
				if([currentObject isKindOfClass:[UITableViewTextFieldCell class]])
				{
					cellLastName = (UITableViewTextFieldCell *)currentObject;
					break;
				}
			}
		}
		cellLastName.textField.text = profile.lastName;
		
		[cellLastName retain];
		cellLastName.selectionStyle = UITableViewCellSelectionStyleNone;
		return cellLastName;
	}
	

    return nil;
    // Configure the cell...
    
//    return cell;
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
    /*
     <#DetailViewController#> *detailViewController = [[<#DetailViewController#> alloc] initWithNibName:@"<#Nib name#>" bundle:nil];
     // ...
     // Pass the selected object to the new view controller.
     [self.navigationController pushViewController:detailViewController animated:YES];
     [detailViewController release];
     */
}

- (void) done:(id)sender 
{
	VPProfile *prof = [VPProfile singleton];
	prof.firstName = [NSString stringWithString:cellFirstName.textField.text];
	prof.lastName = [NSString stringWithString:cellLastName.textField.text];
	
	[self.navigationController popViewControllerAnimated:YES];
}


@end

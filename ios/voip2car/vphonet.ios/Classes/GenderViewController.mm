//
//  GenderViewController.m
//  Vphonet
//
//  Created by uncle on 14.05.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import "VphonetAppDelegate.h"
#import "VPProfile.h"
#import "GenderViewController.h"


@implementation GenderViewController

@synthesize gender;

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

	
	self.title = NSLocalizedString(@"Gender", @"");
	
	self.navigationItem.rightBarButtonItem  = [[[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"Done", @"")
																				style: UIBarButtonItemStyleDone
																			   target: self 
																			   action:@selector(done:)] autorelease];


    // Uncomment the following line to preserve selection between presentations.
    // self.clearsSelectionOnViewWillAppear = NO;
 
    // Uncomment the following line to display an Edit button in the navigation bar for this view controller.
//     self.navigationItem.rightBarButtonItem = self. .editButtonItem;
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
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    // Return the number of rows in the section.
    return 3;
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section {
	
	return NSLocalizedString(@"Select your Gender:", @"");
}


- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *CellIdentifier = @"BirthdayCell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
        cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier] autorelease];
    }
	
	VPProfile *profile = [VPProfile singleton];
    
    // Configure the cell...
	if (indexPath.section == 0) {
		switch (indexPath.row) {
			case 0:
				cell.imageView.image = [UIImage imageNamed:@"male-and-female.png"];
				cell.textLabel.text = NSLocalizedString(@"I don't know", @"");
				if ((![profile.gender isEqualToString:@"F"]) && (![profile.gender isEqualToString:@"M"])) {
					cell.accessoryType = UITableViewCellAccessoryCheckmark;
				}
				break;
			case 1:
				cell.imageView.image = [UIImage imageNamed:@"female.png"];
				cell.textLabel.text = NSLocalizedString(@"Female", @"");
				if ([profile.gender isEqualToString:@"F"]) {
					cell.accessoryType = UITableViewCellAccessoryCheckmark;
				}
				break;
			case 2:
				cell.imageView.image = [UIImage imageNamed:@"male.png"];
				cell.textLabel.text = NSLocalizedString(@"Male", @"");
				if ([profile.gender isEqualToString:@"M"]) {
					cell.accessoryType = UITableViewCellAccessoryCheckmark;
				}
				break;
				
			default:
				break;
		}
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

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
	UITableViewCell *cell =[tableView cellForRowAtIndexPath:indexPath];
	if (cell != nil) {
		
		
		for (int i= 0; i < 3; i++) {
			UITableViewCell *row = [tableView cellForRowAtIndexPath:[NSIndexPath indexPathForRow:i inSection:0]];
			if (row)
				row.accessoryType = UITableViewCellAccessoryNone;
		}
		
		cell.accessoryType = UITableViewCellAccessoryCheckmark;
		gender = indexPath.row;
	}
}

- (void) done:(id)sender
{
	VPProfile *profile = [VPProfile singleton];
	
	switch (gender) {
	case 0:
			profile.gender = @"";
		break;
	case 1:
			profile.gender = @"F";
		break;
	case 2:
			profile.gender = @"M";
		break;
	}
	
	[self.navigationController popViewControllerAnimated:YES];
}


@end

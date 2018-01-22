//
//  StatusViewController.m
//  IP-Phone
//
//  Created by uncle on 15.02.11.
//  Copyright 2011. All rights reserved.
//

#import "VphonetAppDelegate.h"
#import "VPEngine.h"
#import "VPProfile.h"
#import "StatusViewController.h"


@implementation StatusViewController

@synthesize status;

- (id) init
{
    [super init];
    
    
    return self;
}


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
	
	[self setTitle:NSLocalizedString(@"Change Status", @"")];
	[self setCustomBackground:@"background.png"];
	[self.tableView setSeparatorColor:[UIColor paleYellowColor]];

	
    // Uncomment the following line to preserve selection between presentations.
    // self.clearsSelectionOnViewWillAppear = NO;
 
    // Uncomment the following line to display an Edit button in the navigation bar for this view controller.
    // self.navigationItem.rightBarButtonItem = self.editButtonItem;
    self.navigationItem.rightBarButtonItem  = [[[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"Done", @"")
																				style:UIBarButtonItemStyleDone
																			   target:self 
																			   action:@selector(done:)] autorelease];	
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}

- (void)viewWillAppear:(BOOL)animated
{
	status = [[VPEngine sharedInstance] userStatus];
	
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
    return (section == 1 ? 1 : 4);
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section {
	
	return (section == 1 ? NSLocalizedString(@"Add a Status Message", @"") : NSLocalizedString(@"Change your account status:", @""));
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *CellIdentifier = @"Cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
        cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier] autorelease];
    }
    
	if (indexPath.section == 0) {
		switch (indexPath.row) {
			case 0:
				cell.textLabel.text = NSLocalizedString(@"Online", @"");
				cell.imageView.image = [UIImage imageNamed:@"status_green.png"];
				
				if ((status == AOL_ONLINE) || (status == AOL_WEBCAMPRESENT)) {
					cell.accessoryType = UITableViewCellAccessoryCheckmark;
				}
				
				break;
			case 1:
				cell.textLabel.text = NSLocalizedString(@"Offline", @"");
				cell.imageView.image = [UIImage imageNamed:@"status_red.png"];

				if (status == AOL_OFFLINE) {
					cell.accessoryType = UITableViewCellAccessoryCheckmark;
				}
				break;
			case 2:
				cell.textLabel.text = NSLocalizedString(@"Invisible", @"");
				cell.imageView.image = [UIImage imageNamed:@"status_yellow.png"];
				
				if ((status == AOL_LIMITED) || (status == AOL_LIMITED_WEBCAMPRESENT)) {
					cell.accessoryType = UITableViewCellAccessoryCheckmark;
				}
				
				break;
			case 3:
			{
				cell.textLabel.text = NSLocalizedString(@"Unavailable", @"");
				cell.imageView.image = [UIImage imageNamed:@"status_gray.png"];
				switch (status) {
					case AOL_ONLINE:
					case AOL_WEBCAMPRESENT:
					case AOL_OFFLINE:	
					case AOL_LIMITED:
					case AOL_LIMITED_WEBCAMPRESENT:
						break;
					default:
						cell.accessoryType = UITableViewCellAccessoryCheckmark;
						break;
				}
			}
				break;
			default:
				break;
		}
		
	} else {
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
		cellTextField.textField.placeholder = NSLocalizedString(@"Type a message", @"");
		cellTextField.textField.text = [[VPProfile singleton] statusMessage];
		return cellTextField;
	}
	
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
	if (indexPath.section != 0) {
		return;
	}
	
	UITableViewCell *cell =[tableView cellForRowAtIndexPath:indexPath];
	if (cell != nil) {
		
		
		for (int i= 0; i < 4; i++) {
			UITableViewCell *row = [tableView cellForRowAtIndexPath:[NSIndexPath indexPathForRow:i inSection:0]];
			if (row)
				row.accessoryType = UITableViewCellAccessoryNone;
		}
		
		cell.accessoryType = UITableViewCellAccessoryCheckmark;
		
		switch (indexPath.row) {
			case 0:
				status = AOL_ONLINE;
				break;
			case 1:
				status = AOL_OFFLINE;
				break;
			case 2:
				status = AOL_LIMITED;
				break;
			default:
				status = 0;
				break;
		}
		
	}

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
	if (cellTextField)
		[[VPProfile singleton] setStatusMessage:cellTextField.textField.text];

	[[VPProfile singleton] setStatus:status];
	[[VPEngine sharedInstance] setUserStatus:status];
	
	
	[self.navigationController popViewControllerAnimated:YES];
//    [self finishedWithContact:nil];
}


@end

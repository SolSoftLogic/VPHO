//
//  CallForwardViewController.m
//  IP-Phone
//
//  Created by uncle on 16.02.11.
//  Copyright 2011. All rights reserved.
//

#import "VPProfile.h"
#import "VphonetAppDelegate.h"
#import "CallForwardViewController.h"


@implementation CallForwardViewController

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
	[switchCtl release];

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
	
	[self setTitle:NSLocalizedString(@"Call Forward", @"")];
	[self setCustomBackground:@"background.png"];
	[self.tableView setSeparatorColor:[UIColor paleYellowColor]];


	self.navigationItem.rightBarButtonItem  = [[[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"Done", @"")
																				style: UIBarButtonItemStyleDone
																			   target: self 
																			   action:@selector(done:)] autorelease];	

	
    // Uncomment the following line to display an Edit button in the navigation bar for this view controller.
//     self.navigationItem.rightBarButtonItem = self.editButtonItem;
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
	[switchCtl release];
    switchCtl = nil;

}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
	
	VPProfile *profile = [VPProfile	singleton];
	self.switchCtl.on = [profile.phoneForward length] != 0;
	[self.tableView reloadData];

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

// Return the number of sections.
- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    // Return the number of sections.
	if ([switchCtl isOn]) {
		return 2;
	}
	
	return 1;	
}

// Return the number of rows in the section.
- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
	return 1;
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section {

	switch (section) {
		case 1: 
			return @"Phone Number:";
			break;
		default:
			break;
	}
	return nil;
}

/*
- (UIView *)tableView:(UITableView *)tableView viewForHeaderInSection:(NSInteger)section {
	UIView *containerView = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 320, 40)];
	containerView.backgroundColor = [UIColor clearColor];//[UIColor groupTableViewBackgroundColor];

	UILabel *label = [[UILabel alloc] initWithFrame:CGRectMake(10, 2, 320, 40)];
	label.numberOfLines = 0;
	label.lineBreakMode = UILineBreakModeWordWrap;
	label.backgroundColor = [UIColor clearColor];
	label.font = [UIFont systemFontOfSize:14];
	label.textColor = [UIColor blackColor]; //colorWithRed:0.265 green:0.294 blue:0.367 alpha:1.000];
	label.text = [self tableView:tableView titleForHeaderInSection:section];

	[containerView addSubview:label];

	return containerView;
}
*/ 

- (CGFloat)tableView:(UITableView *)tableView heightForHeaderInSection:(NSInteger)section {
	if(section != 0)
		return 47-11;
	return 0;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *CellIdentifier = @"Cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
        cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier] autorelease];
    }
    
    // Configure the cell...
	switch (indexPath.section) {
		case 0:
			cell.textLabel.text = NSLocalizedString(@"Call forward:", @"");
			if (indexPath.row == 0)
			{
//				switchCtl.on = on;
				[cell.contentView addSubview:self.switchCtl];
			}
			break;
		case 1:
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
			cellTextField.textField.placeholder = NSLocalizedString(@"Type a phone number", @"");
			cellTextField.textField.text = [[VPProfile singleton] phoneForward];
			return cellTextField;
		default:
			break;
	}

    return cell;
}

- (UISwitch *)switchCtl
{
    if (switchCtl == nil) 
    {
        CGRect frame = CGRectMake(198.0, 8.0, 94.0, 27.0);
        switchCtl = [[UISwitch alloc] initWithFrame:frame];
        [switchCtl addTarget:self action:@selector(switchAction:) forControlEvents:UIControlEventValueChanged];
        
        // in case the parent view draws with a custom color or gradient, use a transparent color
       switchCtl.backgroundColor = [UIColor clearColor];
    }
    return switchCtl;
}

- (void)switchAction:(id)sender
{
	[self.tableView beginUpdates];
	if ([switchCtl isOn])
		[self.tableView insertSections:[NSIndexSet indexSetWithIndex:1] withRowAnimation:UITableViewRowAnimationFade];
	else	
		[self.tableView deleteSections:[NSIndexSet indexSetWithIndex:1] withRowAnimation:UITableViewRowAnimationFade];
	
	[self.tableView endUpdates];	
	[self.tableView reloadData];
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
	VPProfile *profile = [VPProfile singleton];
	
	if ([switchCtl isOn]) {
		profile.phoneForward = [NSString stringWithString:cellTextField.textField.text];
	} else {
		profile.phoneForward = @"";
	}
	
	[self.navigationController popViewControllerAnimated:YES];
}


@end

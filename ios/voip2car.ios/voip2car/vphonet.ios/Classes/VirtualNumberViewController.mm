//
//  VirtualNumberViewController.m
//  Vphonet
//
//  Created by uncle on 12.05.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import "VphonetAppDelegate.h"
#import "VPProfile.h"
#import "VirtualNumberViewController.h"


@implementation VirtualNumberViewController


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
	
	self.navigationItem.rightBarButtonItem  = [[[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"Done", @"")
																				style: UIBarButtonItemStyleDone
																			   target: self 
																			   action:@selector(done:)] autorelease];	

	[self setTitle:NSLocalizedString(@"Virtual Number", @"")];
	[self setCustomBackground:@"background.png"];
	[self.tableView setSeparatorColor:[UIColor paleYellowColor]];

    // Uncomment the following line to preserve selection between presentations.
    // self.clearsSelectionOnViewWillAppear = NO;
 
    // Uncomment the following line to display an Edit button in the navigation bar for this view controller.
    // self.navigationItem.rightBarButtonItem = self.editButtonItem;
}

- (void)viewDidUnload
{
    [super viewDidUnload];
	[switchCtl release];
    switchCtl = nil;
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
	VPProfile *profile = [VPProfile	singleton];
	self.switchCtl.on = [profile.phoneVirtual length] != 0;
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

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    // Return the number of sections.
	if ([switchCtl isOn]) {
		return 2;
	}
	
	return 1;	
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    // Return the number of rows in the section.
    return 1;
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

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section {
	
	switch (section) {
		case 1: 
			return NSLocalizedString(@"Enter Number:", @"") ;
			break;
		default:
			break;
	}
	return nil;
}

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
	
	VPProfile *profile = [VPProfile singleton];
    
    // Configure the cell...
	switch (indexPath.section) {
		case 0:
			cell.textLabel.text = NSLocalizedString(@"Virtual Number:", @"");
			if (indexPath.row == 0) 
			{
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
			cellTextField.textField.text = profile.phoneVirtual;
			//			cellTextField.textField.text = [[VPProfile singleton] statusMessage];
			return cellTextField;
			
			//			if (indexPath.row == 0)
			//			{
			//				cell.textLabel.text = @"Offline";
			//				cell.imageView.image = [UIImage imageNamed:@"on.png"];
			//			}
			//			else
			//			{
			//				cell.textLabel.text = @"Offline and Online";
			//				cell.imageView.image = [UIImage imageNamed:@"off.png"];
			//			}
			//			break;
			//		case 2:
			//			cell.textLabel.text = @"+972(0)3962-5455";
			//			cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
			//			break;
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
		profile.phoneVirtual = [NSString stringWithString:cellTextField.textField.text];
	} else {
		profile.phoneVirtual = @"";
	}
		
	[self.navigationController popViewControllerAnimated:YES];
}


@end

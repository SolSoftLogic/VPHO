//
//  HelpViewController.m
//  Vphonet
//
//  Created by uncle on 12.05.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import "VphonetAppDelegate.h"
#import "HelpViewController.h"


@implementation HelpViewController

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
	
	
	[self setTitle:NSLocalizedString(@"Help", @"")];
	[self setCustomBackground:@"background.png"];
	[self.tableView setSeparatorColor:[UIColor paleYellowColor]];

	
	//self.tableView.backgroundColor = [UIColor clearColor];
	
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
		case 2:
			return 1;
			break;
			
		default:
			break;
	}
    return 0;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *CellIdentifier = @"Cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
        cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier] autorelease];
    }
    
	if (indexPath.section == 0) {
		if (indexPath.row == 0) {
			if(cellFullName == nil) {
				NSArray *topLevelObjects = [[NSBundle mainBundle] loadNibNamed:@"UITableViewFieldCell" owner:nil options:nil];
				for(id currentObject in topLevelObjects)
				{
					if([currentObject isKindOfClass:[UITableViewFieldCell class]])
					{
						cellFullName = (UITableViewFieldCell *)currentObject;
						break;
					}
				}
			}
			
			[cellFullName retain];
			cellFullName.label.text = NSLocalizedString(@"Full Name:", @"");
			cellFullName.selectionStyle = UITableViewCellSelectionStyleNone;
//			cellFullName.textField.placeholder = NSLocalizedString(@"Type a phone number", @"");
			return cellFullName;
		} else if (indexPath.row == 1) {
			if(cellSubject == nil) {
				NSArray *topLevelObjects = [[NSBundle mainBundle] loadNibNamed:@"UITableViewFieldCell" owner:nil options:nil];
				for(id currentObject in topLevelObjects)
				{
					if([currentObject isKindOfClass:[UITableViewFieldCell class]])
					{
						cellSubject = (UITableViewFieldCell *)currentObject;
						break;
					}
				}
			}
			
			[cellSubject retain];
			cellSubject.label.text = NSLocalizedString(@"Subject:", @"");
			cellSubject.selectionStyle = UITableViewCellSelectionStyleNone;
			//			cellFullName.textField.placeholder = NSLocalizedString(@"Type a phone number", @"");
			return cellSubject;
		}
	} else if (indexPath.section == 1) {
		if(cellMessage == nil) {
			NSArray *topLevelObjects = [[NSBundle mainBundle] loadNibNamed:@"UITableViewMemoCell" owner:nil options:nil];
			for(id currentObject in topLevelObjects)
			{
				if([currentObject isKindOfClass:[UITableViewMemoCell class]])
				{
					cellMessage = (UITableViewMemoCell *)currentObject;
					break;
				}
			}
		}
		
		[cellMessage retain];
		cellMessage.label.text = NSLocalizedString(@"Message:", @"");
		cellMessage.selectionStyle = UITableViewCellSelectionStyleNone;
		return cellMessage;
	} else if (indexPath.section == 2) {
		cell.textLabel.text = NSLocalizedString(@"Send", @"");
		cell.textLabel.textAlignment = UITextAlignmentCenter;
		cell.selectionStyle = UITableViewCellSelectionStyleBlue;

	}
		
	
    // Configure the cell...
    
    return cell;
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
	if (indexPath.section == 1) {
		return 145;
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

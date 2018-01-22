//
//  SearchResultViewController.m
//  Vphonet
//
//  Created by uncle on 01.05.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import "SearchResultViewController.h"
#import "SearchProfileViewController.h"

@implementation SearchResultViewController

@synthesize contacts;
//
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
		contacts = [[VPContactList alloc] init];
    }
    return self;
}

/*
- (id)initWithStyle:(UITableViewStyle)style
{
    self = [super initWithStyle:style];
    if (self) {
	}
 return self;
}
*/
- (void)dealloc
{
	[contacts release];
    [super dealloc];
}

- (void) loadContacts:(NSString *)list
{
	[contacts loadFromSearchList:list];
	
	
	
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

	
	self.title = NSLocalizedString(@"Search Result", @"");

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
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    // Return the number of rows in the section.
    return [[contacts onlineContacts] count];
}

- (NSString *)placeHolderIcon:(bool)male
{
	int x = (arc4random() % 6 + 1);
	
	if (male) {
		return [NSString stringWithFormat:@"male%d.png",x];
	}
	
	return [NSString stringWithFormat:@"female%d.png",x];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *CellIdentifier = @"Cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
        cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleSubtitle reuseIdentifier:CellIdentifier] autorelease];
		cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
    }
    
    // Configure the cell...
	switch(indexPath.section)
	{
		case 0:
			
			VPContact *contact = [[contacts onlineContacts] objectAtIndex:[indexPath row]];
			if (contact) {
				cell.textLabel.text = [NSString stringWithFormat:@"%@ %@", contact.firstName, contact.lastName];
				cell.detailTextLabel.text = contact.userName;
				
				NSString *photoName;
				
				photoName = [contact photoImageName];
				
				if ([photoName length] == 0) {
					cell.imageView.image = [UIImage imageNamed:[NSString stringWithString:[self placeHolderIcon:[contact.gender isEqualToString:@"M"]]]];
				} else 
				{
					cell.imageView.image = [UIImage imageWithContentsOfFile:[NSString stringWithString:photoName]];
				}
				
				
				// Contact photo
				//			UIImageView *imgView = [[UIImageView alloc] initWithFrame:CGRectMake(2.0f, 2.0f, 50.0f, 50.0f)];
				//			imgView.image = [UIImage imageNamed:@"profile_picture.png"];
				//			[cell addSubview:imgView];
				//			[imgView release];

				
				
				//			cell.imageView.image = leftIcon;
				
				//			if (indexPath.row == 0) {
//				cell.textLabel.text = @"Status";
//				cell.detailTextLabel.text = @"Online";
				//			}

			}
			
			//NSString *contactName = [NSString stringWithFormat:@"%@ %@", 
			//                         contact.firstName, contact.lastName];
			
			
			
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
    // Navigation logic may go here. Create and push another view controller.
	
	UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"Back", @"") style: UIBarButtonItemStyleBordered target: nil action: nil];
	[[self navigationItem] setBackBarButtonItem: backButton];
	[backButton release];
	
	SearchProfileViewController *searchProfileViewController = [[SearchProfileViewController alloc] initWithNibName:@"SearchProfileViewController" bundle:nil];
	searchProfileViewController.contact = (VPContact*)[[self.contacts onlineContacts] objectAtIndex:indexPath.row];
	
	[self.navigationController pushViewController:searchProfileViewController animated:YES];
	[searchProfileViewController release];
	
}

@end

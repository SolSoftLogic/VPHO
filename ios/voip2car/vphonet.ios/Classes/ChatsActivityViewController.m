//
//  ChatsActivityViewController.m
//  Vphonet
//
//  Created by uncle on 25.04.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import "UICustomColor.h"
#import "CustomBadge.h"
#import "ChatEvent.h"
#import "VPDatabase.h"
#import "ChatViewController.h"
#import "ChatsActivityViewController.h"

@implementation ActvityContact 

@synthesize contact;
@synthesize unReadMessages;
@synthesize lastMessage;

@end


@implementation ChatsActivityViewController

-(void)awakeFromNib{
	[super awakeFromNib];
	activities = [[NSMutableArray alloc] init];
}

- (void)dealloc
{
    [super dealloc];
	[activities dealloc];
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
	
	[self setTitle:NSLocalizedString(@"Chat activity", @"")];

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
	
	[activities removeAllObjects];

	NSString* sql = [NSString stringWithFormat:@"SELECT vpnumber, unread, last_message FROM chats ORDER BY last DESC LIMIT 15"];
	
	NSArray* records = [[VPDatabase sharedInstance] query:sql];
	
	if ([records count] > 0) 
	{
		for (NSArray *row in records) 
		{
			VPContact *contact = [[VPContactList sharedInstance] contactByNumber:[row objectAtIndex:0]];
			
			NSLog(@"%@ - %d  %@", [row objectAtIndex:0], [[row objectAtIndex:1] intValue], [row objectAtIndex:2]);
			
			if (contact) {
				
				ActvityContact* activity = [[ActvityContact alloc] init];
				activity.contact = contact;
				activity.unReadMessages	= [[row objectAtIndex:1] intValue];
				activity.lastMessage		= [row objectAtIndex:2];
				
				[activities addObject:activity];
			}
		}
	}
	
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
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return [activities count];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *CellIdentifier = @"Cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
        cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleSubtitle reuseIdentifier:CellIdentifier] autorelease];
    }
	
	ActvityContact* activity = (ActvityContact*)[activities objectAtIndex:indexPath.row];
	
	if (activity) {
		cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
		
		if (activity.unReadMessages > 0) {
			CustomBadge *customBadge1 = [CustomBadge customBadgeWithString:[NSString stringWithFormat:@"%d", activity.unReadMessages]];
			[customBadge1 setFrame:CGRectMake(cell.frame.size.height - customBadge1.frame.size.width + customBadge1.frame.size.width/2 - 4, 
											  cell.frame.size.height - customBadge1.frame.size.height,
											  customBadge1.frame.size.width, customBadge1.frame.size.height)];
			[cell addSubview:customBadge1];
		}

		NSString *contactPhoto = [activity.contact photoImageName];
		cell.imageView.image = [UIImage imageWithContentsOfFile:contactPhoto];
		cell.textLabel.text = [NSString stringWithFormat:@"%@ %@", activity.contact.firstName, activity.contact.lastName];
		cell.detailTextLabel.text = activity.lastMessage;
		
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
- (NSIndexPath *)tableView:(UITableView *)tableView willSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
	
	[[[tableView cellForRowAtIndexPath:indexPath] textLabel] setTextColor:[UIColor blackColor]];

	
	return indexPath;
}



- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
	[tableView deselectRowAtIndexPath:indexPath animated:NO];
	
	ActvityContact* activity = [activities objectAtIndex:[indexPath row]];
	
	ChatViewController *controller = [[ChatViewController alloc] initWithNibName:@"ChatViewController" bundle:nil];
	
	controller.contact = activity.contact;
	
	UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"Back", @"") style: UIBarButtonItemStyleBordered target: nil action: nil];
	[[self navigationItem] setBackBarButtonItem: backButton];
	[backButton release];
	
    [self.navigationController pushViewController:controller animated:YES];
    [controller release]; 

}

@end

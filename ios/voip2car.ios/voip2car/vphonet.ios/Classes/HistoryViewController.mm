//
//  HistoryViewController.m
//  IP-Phone
//
//  Created by uncle on 10.02.11.
//  Copyright 2011. All rights reserved.
//

#import "VPContactList.h"
#import "VPHistory.h"
#import "VphonetAppDelegate.h"
#import "HistoryViewController.h"
#import "WhattodoViewController.h"


@implementation HistoryViewController

@synthesize delegateDialer;
//@synthesize mytoolbar;

- (id)initWithStyle:(UITableViewStyle)style
{
    self = [super initWithStyle:style];
    if (self) {
        // Custom initialization
    }
    return self;
}


- (void)dealloc {
	
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	
	[dateFormatterDuration release];
	[dateFormatterDate release];
	
	[barButtonClear release];
	[barButtonRemove release];
	
	[super dealloc];
}


-(void)awakeFromNib{
	[super awakeFromNib];
	
	[[NSNotificationCenter defaultCenter] addObserver:self 
											 selector:@selector(vpEngineNotification:) 
												 name:VPEngineNotification
											   object:nil];

	self->dateFormatterDuration = [[NSDateFormatter alloc] init];
	[self->dateFormatterDuration setDateFormat:@"mm:ss"];
	
	self->dateFormatterDate = [[NSDateFormatter alloc] init];
	[self->dateFormatterDate setTimeStyle:NSDateFormatterShortStyle];
	[self->dateFormatterDate setDateStyle:NSDateFormatterNoStyle];
	
	
	
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section
{
	NSString		*key = [[[[VPHistory singleton] eventsByDays] allKeys] objectAtIndex:section];

	NSDateFormatter *dateFormatterSection = [[NSDateFormatter alloc] init];
	[dateFormatterSection setDateStyle:NSDateFormatterNoStyle];
	[dateFormatterSection setDateFormat:@"yyyy-MM-dd"];
	NSDate *date = [dateFormatterSection dateFromString:key];

//	NSCalendar *gregorian = [[NSCalendar alloc]
//							 initWithCalendarIdentifier:NSGregorianCalendar];

//	NSDateComponents *comp = [gregorian components:  (NSYearCalendarUnit | NSMonthCalendarUnit |  NSDayCalendarUnit) fromDate: myDate];
//	NSDate *myNewDate = [gregorian dateFromComponents:comps];
//	[comps release];
//	[gregorian release];
	
	NSCalendar *cal = [NSCalendar currentCalendar];
	NSDateComponents *components = [cal components:(NSYearCalendarUnit | NSMonthCalendarUnit | NSDayCalendarUnit) fromDate:[[NSDate alloc] init]];

	NSDate *today = [cal dateFromComponents:components];;//[[NSDate alloc] init];
	NSDate *yesterday = [today dateByAddingTimeInterval:-86400.0];
//	NSDate *yesterday = [date dateByAddingTimeInterval:-86400.0];
	
	if ([date compare:today] == NSOrderedSame)
		return @"Today";
	else if ([date compare:yesterday] == NSOrderedSame)
		return @"Yesterday";

	[dateFormatterSection setDateStyle:NSDateFormatterShortStyle];
	
//	[];
	
	return [dateFormatterSection stringFromDate:date];
	
//	[date release];
//	[dateFormatterSection release];
	
//	return key;
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
//	NSLog(@"Sections: %d", [[[[VPHistory singleton] eventsByDays] allKeys] count]);
	return [[[[VPHistory singleton] eventsByDays] allKeys] count];
}


- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {	

	NSObject		*key = [[[[VPHistory singleton] eventsByDays] allKeys] objectAtIndex:section];
	NSMutableArray	*calls = [[[VPHistory singleton] eventsByDays] objectForKey:key];
	
	return [calls count];
//	return [[[VPHistory singleton] events] count];
}


- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
	
    static NSString *CellIdentifier = @"Cell";
	
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
		cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1 reuseIdentifier:CellIdentifier] autorelease];
    }
	

	NSObject		*key = [[[[VPHistory singleton] eventsByDays] allKeys] objectAtIndex:indexPath.section];
	NSMutableArray	*calls = [[[VPHistory singleton] eventsByDays] objectForKey:key];

	HistoryEvent* event = (HistoryEvent*)[calls objectAtIndex:indexPath.row];
//	HistoryEvent* event = (HistoryEvent*)[[[VPHistory singleton] events] objectAtIndex:indexPath.row];
	
	
	
    // Set up the cell
//	HistoryEvent* event = (HistoryEvent*)[[[VPHistory singleton] events] objectAtIndex:indexPath.row];
		
	NSString* date = [self->dateFormatterDate stringFromDate:[NSDate dateWithTimeIntervalSince1970:event.start]];
	
	NSString* duration = [self->dateFormatterDuration stringFromDate:[NSDate dateWithTimeIntervalSince1970:(event.end - event.start)]];

	cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
	cell.textLabel.text = event.remote;
	cell.detailTextLabel.text = [NSString stringWithFormat:@"%@", date, duration];
	cell.detailTextLabel.font = [UIFont systemFontOfSize:12];
	
	switch(event.type){
		case HistoryEventType_Audio:
		case HistoryEventType_AudioVideo:
		{	
			switch (event.status) {
				case HistoryEventStatus_Outgoing:
					cell.imageView.image = [UIImage imageNamed:@"outgoing.png"];
					break;
				case HistoryEventStatus_Incoming:
					cell.imageView.image = [UIImage imageNamed:@"incoming.png"];
					break;
				case HistoryEventStatus_Missed:
				case HistoryEventStatus_Failed:
				default:
					cell.imageView.image = [UIImage imageNamed:@"missed.png"];
					break;
			}
			break;
		}
		case HistoryEventType_SMS:
		case HistoryEventType_Chat:
		case HistoryEventType_FileTransfer:
		default:
			break;
	}
	
	return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {

	
	HistoryEvent* event = (HistoryEvent*)[[[VPHistory singleton] events] objectAtIndex:indexPath.row];
	VPContact *contact = NULL;
	
	for(VPContact *item in [[VPContactList sharedInstance] allContacts]) {
		if ([item.userName caseInsensitiveCompare:event.remote] == NSOrderedSame)
		{
			contact = item;
			break;
		}
	}
	
	WhattodoViewController *controller = [[WhattodoViewController alloc] initWithVPContact:contact];
	
	[self.navigationController setNavigationBarHidden:YES animated:NO];
    [self.tabBarController.navigationController setNavigationBarHidden:YES animated:NO];
	[self.navigationController setNavigationBarHidden:NO animated:NO];
	// hide main navigaion bar
	//[self.tabBarController.navigationController setNavigationBarHidden:YES animated:NO];
	
	// show tabbar navigation bar
	//[self.navigationController setNavigationBarHidden:NO animated:NO];
	//self.navigationItem.title = @"What to do?";
	[self.navigationController pushViewController:controller animated:YES];
	
	[controller release]; 
	
	UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithTitle:@"Back" style:UIBarButtonItemStylePlain target:nil action:nil];
	self.navigationItem.backBarButtonItem = backButton;
	[backButton release];
	
}


- (void)viewDidLoad {
    [super viewDidLoad];
    // Uncomment the following line to add the Edit button to the navigation bar.
    // self.navigationItem.rightBarButtonItem = self.editButtonItem;
	
	[self setTitle:NSLocalizedString(@"Call History", @"")];

	
	[self.navigationController setNavigationBarHidden:YES animated:NO];

	[[VPHistory singleton] load];

}

- (void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];

    [self.tabBarController.navigationController setNavigationBarHidden:YES animated:NO];
	[self.navigationController setNavigationBarHidden:NO animated:NO];
}

- (void)viewWillDisappear:(BOOL)animated {
    [super viewWillDisappear:animated];
}


/*


//Initialize the array.
listOfItems = [[NSMutableArray alloc] init];

NSArray *countriesToLiveInArray = [NSArray arrayWithObjects:@"Iceland", @"Greenland", @"Switzerland", @"Norway", @"New Zealand", @"Greece", @"Rome", @"Ireland", nil];
NSDictionary *countriesToLiveInDict = [NSDictionary dictionaryWithObject:countriesToLiveInArray forKey:@"Countries"];

NSArray *countriesLivedInArray = [NSArray arrayWithObjects:@"India", @"U.S.A", nil];
NSDictionary *countriesLivedInDict = [NSDictionary dictionaryWithObject:countriesLivedInArray forKey:@"Countries"];

[listOfItems addObject:countriesToLiveInDict];
[listOfItems addObject:countriesLivedInDict];
*/
 
/*
 // Override to support editing the list
 - (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath {
 
 if (editingStyle == UITableViewCellEditingStyleDelete) {
 // Delete the row from the data source
 [tableView deleteRowsAtIndexPaths:[NSArray arrayWithObject:indexPath] withRowAnimation:YES];
 }   
 if (editingStyle == UITableViewCellEditingStyleInsert) {
 // Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view
 }   
 }
 */


/*
 // Override to support conditional editing of the list
 - (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath {
 // Return NO if you do not want the specified item to be editable.
 return YES;
 }
 */


/*
 // Override to support rearranging the list
 - (void)tableView:(UITableView *)tableView moveRowAtIndexPath:(NSIndexPath *)fromIndexPath toIndexPath:(NSIndexPath *)toIndexPath {
 }
 */


/*
 // Override to support conditional rearranging of the list
 - (BOOL)tableView:(UITableView *)tableView canMoveRowAtIndexPath:(NSIndexPath *)indexPath {
 // Return NO if you do not want the item to be re-orderable.
 return YES;
 }
 */

/*
 - (void)viewWillAppear:(BOOL)animated {
 [super viewWillAppear:animated];
 }
 */
/*
 - (void)viewDidAppear:(BOOL)animated {
 [super viewDidAppear:animated];
 }
 */
/*
 - (void)viewWillDisappear:(BOOL)animated {
 }
 */
/*
 - (void)viewDidDisappear:(BOOL)animated {
 }
 */



- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}


- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning]; // Releases the view if it doesn't have a superview
    // Release anything that's not essential, such as cached data
}

#pragma mark -
#pragma mark Notifications

- (void) vpEngineNotification:(NSNotification*)notification
{		 	
	unsigned msg	= 	[[[notification userInfo] objectForKey:@"MSG"] unsignedIntValue];
	//  unsigned param1 = 	[[[notification userInfo] objectForKey:@"PARAM1"] unsignedIntValue];
	//	unsigned param2 = 	[[[notification userInfo] objectForKey:@"PARAM2"] unsignedIntValue];
    
    switch(msg)
	{
		case VPMSG_CALLENDED:
        {
			[[VPHistory singleton] load];
			[self.tableView reloadData];
        }
			break;  
    }
    
}



- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex{
	if (buttonIndex == 1){
		[[VPHistory singleton] clear];
	}
}

-(IBAction) onBarButtonClearClick:(id)sender {
	[self.tableView reloadData];
	
	UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Clear Call History" message:@"Are you sure you want to delete all calls?" delegate:self cancelButtonTitle:@"No" otherButtonTitles:@"Yes", nil];
	[alert show];
	[alert release];
}



@end

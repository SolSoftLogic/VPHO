//
//  ContactsViewController.mm
//  Vphonet

#import <objc/runtime.h>
#import "UICustomAlertView.h"
#import "VPEngine.h"
#import "VPContactList.h"
#import "VPProfile.h"
#import "VphonetAppDelegate.h"
#import "ContactsViewController.h"
#import "WhattodoViewController.h"
#import "SearchViewController.h"
#import "CellContactList.h"

@implementation ContactsViewController



- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self)
    {
    }
	
    return self;
}


- (id)initWithStyle:(UITableViewStyle)style
{
    self = [super initWithStyle:style];
    if (self) {
        // this will appear as the title in the navigation bar
        CGRect frame = CGRectMake(0, 0, 400, 44);
        UILabel *label = [[[UILabel alloc] initWithFrame:frame] autorelease];
        label.backgroundColor = [UIColor clearColor];
        label.font = [UIFont boldSystemFontOfSize:20.0];
        label.shadowColor = [UIColor colorWithWhite:0.0 alpha:0.5];
        label.textAlignment = UITextAlignmentCenter;
        label.textColor = [UIColor yellowColor];
        self.navigationItem.titleView = label;
        label.text = NSLocalizedString(@"PageThreeTitle", @"");
    }
    return self;
}

#pragma mark -
#pragma mark View lifecycle

- (void)viewDidLoad {
    [super viewDidLoad];

    [[NSNotificationCenter defaultCenter] addObserver:self 
                                          selector:@selector(vpEngineNotification:) 
                                          name:VPEngineNotification
                                          object:nil];
	
	[[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(contactListComplete:) 
                                                 name:OnContactListCompleteNotification
                                               object:nil];
	
	[[NSNotificationCenter defaultCenter] addObserver:self 
                                             selector:@selector(onSmsNotification:) 
                                                 name:OnSmsNotification
                                               object:nil];	


	
	
	self.title	= NSLocalizedString(@"Contact List", @"");
	
	[self.tabBarController.navigationController setNavigationBarHidden:YES animated:NO];
	[self.navigationController setNavigationBarHidden:NO animated:NO];

	
	self.navigationItem.rightBarButtonItem = 
	[[UIBarButtonItem alloc] initWithBarButtonSystemItem: UIBarButtonSystemItemAdd target:self action:@selector(addContact)];
	
//	UIBarButtonItemStyleBordered
//	UIBarButtonItemStyleDone
	
	self.navigationItem.leftBarButtonItem = 
	[[[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"Logout", @"") style:UIBarButtonItemStylePlain target:self action:@selector(logout)] autorelease];	
	
}


- (void) logout
{
	[VPProfile clear];
	[[VPEngine sharedInstance] logoff];
	
	VphonetAppDelegate *appDelegate = (VphonetAppDelegate *)[[UIApplication sharedApplication] delegate];
	[appDelegate showLogin:YES];
	
}

- (void) addContact
{
	SearchViewController *searchViewController = [[SearchViewController alloc] initWithNibName:@"SearchViewController" bundle:nil];
	// ...
	// Pass the selected object to the new view controller.
	UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"Back", @"") style: UIBarButtonItemStyleBordered target: nil action: nil];
	[[self navigationItem] setBackBarButtonItem: backButton];
	[backButton release];

	
	[self.navigationController pushViewController:searchViewController animated:YES];
	UIBarButtonItem *temporaryBarButtonItem = [[UIBarButtonItem alloc] init];
	temporaryBarButtonItem.title = NSLocalizedString(@"Back", @"");
	self.parentViewController.navigationItem.backBarButtonItem = temporaryBarButtonItem;
	[temporaryBarButtonItem release];
	
	[searchViewController release];
}

- (void) reload 
{
	[self.tableView reloadData];
}

/*
- (void)setTitle:(NSString *)title
{
    [super setTitle:title];
    UILabel *titleView = (UILabel *)self.navigationItem.titleView;
    if (!titleView) {
        titleView = [[UILabel alloc] initWithFrame:CGRectZero];
        titleView.backgroundColor = [UIColor clearColor];
        titleView.font = [UIFont boldSystemFontOfSize:20.0];
        titleView.shadowColor = [UIColor colorWithWhite:0.0 alpha:0.5];
		
        titleView.textColor = [UIColor darkTextColor]; // Change to desired color
		
        self.navigationItem.titleView = titleView;
        [titleView release];
    }
    titleView.text = title;
    [titleView sizeToFit];
}
*


- (void)viewWillAppear:(BOOL)animated {
    
	[self reloadData];

	[super viewWillAppear: animated];
	
//	[self.navigationController.navigationBar setTintColor:[UIColor colorWithPatternImage:[UIImage imageNamed:@"TitleYellow.png"]]];
//	[self.navigationController.navigationBar setTintColor:[UIColor colorWithRed:0.94 green:0.75f blue:0.10f alpha:0.0f]];	
    ;
//	[self.tabBarController.navigationController setNavigationBarHidden:YES animated:NO];
//	[self.navigationController setNavigationBarHidden:NO animated:NO];

//    [self.navigationController setNavigationBarHidden:YES animated:NO];
//    [self.tabBarController.navigationController setNavigationBarHidden:NO animated:NO];

//	self.tabBarController.navigationItem.rightBarButtonItem = 
//	[[UIBarButtonItem alloc] initWithBarButtonSystemItem: UIBarButtonSystemItemAdd target:self action:@selector(addContact)];

	
	
//	self.tabBarController.navigationItem.title = NSLocalizedString(@"Contact List", @"");

    [self reload];
}


- (void)viewDidAppear:(BOOL)animated
{
	hasAppeared = YES;
	
	[super viewDidAppear: animated];
	
	[self checkEmpty];
}

- (void)viewDidUnload
{
	if (emptyOverlay)
	{
		self.tableView.scrollEnabled = scrollWasEnabled;
		[emptyOverlay removeFromSuperview];
		emptyOverlay = nil;
	}
}

- (void) reloadData
{
	[self.tableView reloadData];
	
	if (hasAppeared &&
		[self respondsToSelector: @selector(makeEmptyOverlayView)])
		[self checkEmpty];
}

- (UIView*)makeEmptyOverlayView {
	UIImageView *image = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"contacts_empty.png"]];
	return image;
}

- (void) checkEmpty
{
	BOOL isEmpty(YES);
	
	id<UITableViewDataSource> src(self.tableView.dataSource);
	NSInteger sections(1);
	if ([src respondsToSelector: @selector(numberOfSectionsInTableView:)])
		sections = [src numberOfSectionsInTableView: self.tableView];
	for (int i(0); i<sections; ++i)
	{
		NSInteger rows([src tableView: self.tableView numberOfRowsInSection: i]);
		if (rows)
			isEmpty = NO;
	}
	
	if (!isEmpty != !emptyOverlay)
	{
		if (isEmpty)
		{
			scrollWasEnabled = self.tableView.scrollEnabled;
			self.tableView.scrollEnabled = NO;
			emptyOverlay = [self makeEmptyOverlayView];
			[self.tableView addSubview: emptyOverlay];
			[emptyOverlay release];
		}
		else
		{
			self.tableView.scrollEnabled = scrollWasEnabled;
			[emptyOverlay removeFromSuperview];
			emptyOverlay = nil;
		}
	}
	else if (isEmpty)
	{
		// Make sure it is still above all siblings.
		[emptyOverlay retain];
		[emptyOverlay removeFromSuperview];
		[self.tableView addSubview: emptyOverlay];
		[emptyOverlay release];
	}
}


- (void)viewWillDisappear:(BOOL)animated {
    [super viewWillDisappear:animated];
}

/*
- (void)viewDidDisappear:(BOOL)animated {
    [super viewDidDisappear:animated];
}
*/
/*
// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations.
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}
*/


#pragma mark -
#pragma mark Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    // Return the number of sections.
    return 2;
}


- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    // Return the number of rows in the section.
	if (section == 0) {
		return [[[VPContactList sharedInstance] onlineContacts] count];
	}
	
    return [[[VPContactList sharedInstance] offlineContacts] count];
}


- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section{
	if (section == 0) {
		return NSLocalizedString(@"Online", @"");
	}
	return NSLocalizedString(@"Offline", @"");
}

- (CGFloat)tableView:(UITableView *)tableView heightForHeaderInSection:(NSInteger)section {
	if(section == 0)
		return 0;
	
	return tableView.sectionHeaderHeight;
}


- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
	return 54.0;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    
	
//	static NSString *CellIdentifier = @"Cell";
	
	static NSString *cellIdentifier = @"cellContactList";
	CellContactList* cell = (CellContactList *)[tableView dequeueReusableCellWithIdentifier:cellIdentifier];
	if (cell == nil) {
		NSArray *topLevelObjects =[[NSBundle mainBundle] loadNibNamed:@"CellContactList" owner:nil options:nil];
		for(id currentObject in topLevelObjects)
		{
			if([currentObject isKindOfClass:[CellContactList class]])
			{
				cell = (CellContactList *)currentObject;
				break;
			}
		}
		
		cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
//		cell.selectionStyle = UITableViewCellSelectionStyleNone;
	}

	
	
  //  UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
//    if (cell == nil) {
//		cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1 reuseIdentifier:CellIdentifier] autorelease];
//    }
	
	NSMutableArray *contacts = nil;
	
	if (indexPath.section == 0) {
		contacts = [[VPContactList sharedInstance] onlineContacts];
	} else if (indexPath.section == 1) {
		contacts = [[VPContactList sharedInstance] offlineContacts];
	}

	
	VPContact* contact = (VPContact*)[contacts objectAtIndex:indexPath.row];
	
	if (contact) {
		if ([contact.photoUpdate length] != 0 )
		{
			NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
			NSString *file = [NSString stringWithFormat:@"%@/photos/%@.jpg",[paths objectAtIndex:0], contact.photoUpdate];
			NSFileManager *fm = [NSFileManager defaultManager]; 
			
			if ([fm fileExistsAtPath:file])
				cell.imagePhoto.image = [UIImage imageWithContentsOfFile:file];
			else
				cell.imagePhoto.image = [UIImage imageNamed:@"profile_picture.png"];
			
		} else
		{
			cell.imagePhoto.image = [UIImage imageNamed:@"profile_picture.png"];
		}
		
		cell.imageStatus.image = [UIImage imageNamed:[contact statusImageName]];
		cell.label.text = [NSString stringWithFormat:@"%@ %@", contact.firstName, contact.lastName];
	}
	
    
    return cell;

	
	
//	NSString *cellIdentifier = [ NSString stringWithFormat: @"%d:%d", [ indexPath indexAtPosition: 0 ],
//								[ indexPath indexAtPosition:1 ] ];
	
/*	
    UITableViewCell *cell = nil;//[ tableView dequeueReusableCellWithIdentifier:cellIdentifier ];
    if (cell == nil) {
		
		cell = [ [ [ UITableViewCell alloc ] initWithFrame: CGRectZero reuseIdentifier: cellIdentifier ] autorelease ];
		cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
		
		switch ([ indexPath indexAtPosition: 0]) {
			case(0):
			{
				[[VPContactList sharedInstance] onlineContacts];

				
                VPContact *contact = [[[VPContactList sharedInstance] allContacts] objectAtIndex: [ indexPath indexAtPosition: 1 ] ];
				//NSString *contactName = [NSString stringWithFormat:@"%@ %@", 
                //                         contact.firstName, contact.lastName];
                
				// Contact photo
				UIImageView *imgView = [[UIImageView alloc] initWithFrame:CGRectMake(2.0f, 2.0f, 50.0f, 50.0f)];
				imgView.image = [UIImage imageNamed:@"profile_picture.png"];
				[cell addSubview:imgView];
				[imgView release];
				
				// Contact status
				imgView = [[UIImageView alloc] initWithFrame:CGRectMake(55.0f, 11.0f, 32.0f, 32.0f)];
				imgView.image = [UIImage imageNamed:(contact.status==AOL_ONLINE?@"status_green.png":@"status_red.png")];
				[cell addSubview:imgView];
				[imgView release];
				
				// Contact name
				UILabel *label = [[UILabel alloc] initWithFrame:CGRectMake(90.0f, 2.0f, 230.0f, 50.0f)];
				
                if ([contact.firstName length] || [contact.lastName length])
                {
                    label.text = [NSString stringWithFormat:@"%@ %@", contact.firstName, contact.lastName];
                }
                else 
                {
                    label.text = [NSString stringWithString:contact.userName];
                }
				[cell addSubview:label];
				[label release];
				
                break;
			}
		}
	}
*/	
//    return cell;
}


/*
// Override to support conditional editing of the table view.
- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the specified item to be editable.
    return YES;
}
*/


/*
// Override to support editing the table view.
- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath {
    
    if (editingStyle == UITableViewCellEditingStyleDelete) {
        // Delete the row from the data source.
        [tableView deleteRowsAtIndexPaths:[NSArray arrayWithObject:indexPath] withRowAnimation:UITableViewRowAnimationFade];
    }   
    else if (editingStyle == UITableViewCellEditingStyleInsert) {
        // Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view.
    }   
}
*/


/*
// Override to support rearranging the table view.
- (void)tableView:(UITableView *)tableView moveRowAtIndexPath:(NSIndexPath *)fromIndexPath toIndexPath:(NSIndexPath *)toIndexPath {
}
*/


/*
// Override to support conditional rearranging of the table view.
- (BOOL)tableView:(UITableView *)tableView canMoveRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the item to be re-orderable.
    return YES;
}
*/

#pragma mark -
#pragma mark Table view delegate

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {

	
	NSMutableArray *contacts = nil;
	
	if (indexPath.section == 0) {
		contacts = [[VPContactList sharedInstance] onlineContacts];
	} else if (indexPath.section == 1) {
		contacts = [[VPContactList sharedInstance] offlineContacts];
	}
	
	;
	
	VPContact* contact = (VPContact*)[contacts objectAtIndex:indexPath.row];
	if (contact) {
		UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"Back", @"") style:UIBarButtonItemStylePlain target:nil action:nil];
		self.navigationItem.backBarButtonItem = backButton;
		[backButton release];

		WhattodoViewController *controller = [[WhattodoViewController alloc] initWithVPContact:contact];
		self.navigationItem.title = [NSString stringWithFormat:@"%@ %@", contact.firstName, contact.lastName];
		[self.navigationController pushViewController:controller animated:YES];
		
		[controller release]; 
	}

}

#pragma mark -
#pragma mark Notifications

- (void) contactListComplete:(NSNotification*)notification
{
	[self.tableView reloadData];
}



- (void) vpEngineNotification:(NSNotification*)notification
{		 	
	unsigned msg	= 	[[[notification userInfo] objectForKey:@"MSG"] unsignedIntValue];
//    unsigned param1 = 	[[[notification userInfo] objectForKey:@"PARAM1"] unsignedIntValue];
//	unsigned param2 = 	[[[notification userInfo] objectForKey:@"PARAM2"] unsignedIntValue];
    
    switch(msg)
	{
			
		case VPMSG_ABUPDATE:
        {
			[self.tableView reloadData];
			[[VPEngine sharedInstance] postContacts:[VPContactList sharedInstance]];
			
			//            VPContact *contact = [[VPContactList sharedInstance] contactByUsername:[NSString stringWithCString:(const char*)param1 encoding:NSUTF8StringEncoding]];
			//            if (contact)
			//            {
			//                contact.number = [NSString stringWithCString:(const char*)param2 encoding:NSUTF8StringEncoding];
			//                [[VPContactList sharedInstance] addOrReplaceContact:contact];
			//            }
        }

			
//		case VPMMSG_CONTACTLISTMODIFIED:
  //      {
//			if (param1 == TCPFT_RXCONTACTS) {
	//			[[VPContactList sharedInstance] downloadAddressBook];
//			}
	//	}
	//		break;	
			
			
//		case VPMMSG_CONTACTLISTMODIFIED:
//        {
//			[self.tableView reloadData];
			//            [self reload];
  //      }
    //    break;  
    }
    
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
	[[VPContactList sharedInstance] refreshOnlineStatus];
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


#pragma mark -
#pragma mark Memory management

- (void)didReceiveMemoryWarning {
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    // Relinquish ownership any cached data, images, etc. that aren't in use.
}


- (void)dealloc {
    [super dealloc];
     NSLog(@"Contact list view: REMOVE observer");
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

#define ALERT_CONTACTADDED 200
static char alertDictionary;

- (void) onSmsNotification:(NSNotification*)notification
{		 	
	unsigned typeSMS = [[[notification userInfo] objectForKey:@"type"] unsignedIntValue];
	
	switch (typeSMS) {
		case SMSTYPE_CONTACTADDED:
		{
			UICustomAlertView *alert = [[UICustomAlertView alloc] 
										initWithTitle:[NSString stringWithFormat:NSLocalizedString(@"Request from %@", @""),[[notification userInfo] objectForKey:@"number"]]
										message:[[notification userInfo] objectForKey:@"message"]
										delegate:self cancelButtonTitle:nil
										otherButtonTitles:NSLocalizedString(@"Ignore", @""), NSLocalizedString(@"Accept", @""), nil]; 
			NSMutableDictionary *userInfo = [[[NSMutableDictionary alloc] initWithCapacity:1] autorelease];
			[userInfo setObject:[[notification userInfo] objectForKey:@"number"] forKey:@"number"];

			objc_setAssociatedObject(alert, &alertDictionary, userInfo, OBJC_ASSOCIATION_RETAIN);
			[alert setTag:ALERT_CONTACTADDED];
			[alert show];
			[alert release];
		}
			break;
		case SMSTYPE_CONTACTADDED_ACK:
		{
			
		}
			break;
		case SMSTYPE_CONTACTADDED_NACK:
		{
			
		}
			break;
		case SMSTYPE_NEWFILE:
			break;
		case SMSTYPE_FILEDELIVERED:
			break;
		case SMSTYPE_NORMAL:
			break;
		case SMSTYPE_MISSEDCALLS:
			break;
		case SMSTYPE_DELIVERYREPORT:
			break;
		default:	
			break;
	}
}


- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex
{
	if (alertView.tag == ALERT_CONTACTADDED) {
		NSDictionary *userInfo = objc_getAssociatedObject(alertView, &alertDictionary);
		if (buttonIndex == 1) // accepted
		{
			VPContact *contact = [[VPContact alloc] init];
			contact.number = [userInfo objectForKey:@"number"];
			
			[[[VPContactList sharedInstance] contactsOffline] addObject:contact];
			[[VPEngine sharedInstance] postContacts:[VPContactList sharedInstance]];
			[[VPEngine sharedInstance] sendContactAck:contact.number];
			/*
			if(UserAccepts(name))
			{
				AddToMyContacts(name);
				ExportDatabase();
				vpstack->ServerTransfer(TCPFT_TXCONTACTS, exportedfilepath, false, 0);
				VPCALL call2 = vpstack->CreateCall();
				vpstack->SetCallText(call, "I have accepted your invitation and added 
									 you to my contacts");
				vpstack->SetSMSType(SMSTYPE_CONTACTADDED_ACK);
				vpstack->Connect(call, name, BC_SMS);
			} else {
				VPCALL call2 = vpstack->CreateCall();
				vpstack->SetCallText(call, "I have not accepted your invitation");
				vpstack->SetSMSType(SMSTYPE_CONTACTADDED_NACK);
				vpstack->Connect(call, name, BC_SMS);
			}
			*/ 
		} else {
			[[VPEngine sharedInstance] sendContactAck:[userInfo objectForKey:@"number"]];
		}
	}
}


@end


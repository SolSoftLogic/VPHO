//
//  ConferenceViewController.mm
//  Vphonet
//

#import "VPEngine.h"
#import "ContactSelectorViewController.h"


@implementation ContactSelectorViewController

@synthesize delegate;

#pragma mark -
#pragma mark Initialization

- (id) init
{
    [super init];
    
    self.navigationItem.leftBarButtonItem 
    = [ [ [ UIBarButtonItem alloc ] 
         initWithTitle:@"Cancel" 
         style: UIBarButtonItemStylePlain
         target: self 
         action:@selector(cancel) ]
       autorelease ];	
    
    
    return self;
}

/*
- (id)initWithStyle:(UITableViewStyle)style {
    // Override initWithStyle: if you create the controller programmatically and want to perform customization that is not appropriate for viewDidLoad.
    self = [super initWithStyle:style];
    if (self) {
        // Custom initialization.
    }
    return self;
}
*/


#pragma mark -
#pragma mark View lifecycle

- (void)viewDidLoad {
    [super viewDidLoad];
	
	
	
	self.navigationItem.title = NSLocalizedString(@"Settings", @"");
	
	

    // Uncomment the following line to display an Edit button in the navigation bar for this view controller.
    // self.navigationItem.rightBarButtonItem = self.editButtonItem;
    [self.tableView reloadData];

}


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
    [super viewWillDisappear:animated];
}
*/
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

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
	return 54.0;
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    // Return the number of sections.
    return 1;
}




- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    // Return the number of rows in the section.
    return [[[VPContactList sharedInstance] allContacts] count];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    
	NSString *cellIdentifier = [ NSString stringWithFormat: @"%d:%d", [ indexPath indexAtPosition: 0 ],
								[ indexPath indexAtPosition:1 ] ];
	
    UITableViewCell *cell = nil;//[ tableView dequeueReusableCellWithIdentifier:cellIdentifier ];
    if (cell == nil) {
		
		cell = [ [ [ UITableViewCell alloc ] initWithFrame: CGRectZero reuseIdentifier: cellIdentifier ] autorelease ];
		
		switch ([ indexPath indexAtPosition: 0]) {
			case(0):
			{
                VPContact *contact = [[[VPContactList sharedInstance] allContacts] objectAtIndex: [ indexPath indexAtPosition: 1 ] ];
				//NSString *contactName = [NSString stringWithFormat:@"%@ %@", 
                //                         contact.firstName, contact.lastName];
                
				// Contact photo
				UIImageView *imgView = [[UIImageView alloc] initWithFrame:CGRectMake(2.0f, 2.0f, 50.0f, 50.0f)];
				imgView.image = [UIImage imageNamed:@"default_contact.png"];
				[cell addSubview:imgView];
				[imgView release];
				
				// Contact status
				imgView = [[UIImageView alloc] initWithFrame:CGRectMake(55.0f, 11.0f, 32.0f, 32.0f)];
				imgView.image = [UIImage imageNamed:(contact.status == AOL_ONLINE?@"online3.png":@"offline3.png")];
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
	
    return cell;
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

- (void) finishedWithContact:(VPContact*)contact
{
    if (delegate)
        [delegate contactSelector:self.navigationController finishedWithContact:contact];
    
    [self.navigationController.view removeFromSuperview];
    
}

- (void) cancel
{
    [self finishedWithContact:nil];
}

#pragma mark -
#pragma mark Table view delegate
- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    
    VPContact *contact = [[[VPContactList sharedInstance] allContacts] objectAtIndex:[indexPath row]];
    
    [self finishedWithContact:contact];
}


#pragma mark -
#pragma mark Memory management

- (void)didReceiveMemoryWarning {
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Relinquish ownership any cached data, images, etc. that aren't in use.
}

- (void)viewDidUnload {
    // Relinquish ownership of anything that can be recreated in viewDidLoad or on demand.
    // For example: self.myOutlet = nil;
}


- (void)dealloc {
    [super dealloc];
}


@end


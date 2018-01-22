//
//  WhattodoViewController.mm
//  Vphonet

#import "VPEngine.h"
#import "CallViewController.h"
#import "ChatViewController.h"
#import "WhattodoViewController.h"
#import "VphonetAppDelegate.h"
#import "SearchProfileViewController.h"
#import "UICustomAlertView.h"


@implementation WhattodoViewController

@synthesize name;
@synthesize email;
@synthesize statusImage;
@synthesize statusLabel;
@synthesize buttonPhoto;

@synthesize buttonAdd;
@synthesize buttonChat;
@synthesize buttonFile;
@synthesize buttonVideo;
@synthesize buttonVoice;
@synthesize backgroundName;
@synthesize backgroundButton;
@synthesize backgroundHeader;

// The designated initializer.  Override if you create the controller programmatically and want to perform customization that is not appropriate for viewDidLoad.
/*
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization.
    }
    return self;
}
*/

- (id) initWithVPContact:(VPContact*)vpcontact
{
    self = [super initWithNibName:@"WhattodoViewController" bundle:nil];
    contact = [vpcontact retain];
//    VphonetAppDelegate *appDelegate = (VphonetAppDelegate *)[[UIApplication sharedApplication] delegate];
//	tabbarDelegate = appDelegate.tabBarController.delegate;
  //  [appDelegate.tabBarController setDelegate:self];
    
    return self;
}


// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
	
	 UIBarButtonItem* deleteButton = [[[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemTrash target:self action:@selector(onDeleteContact:)] autorelease];
	
	self.navigationItem.rightBarButtonItem = deleteButton;
	
	[self setTitle:[NSString stringWithFormat:@"%@ %@", contact.firstName, contact.lastName]];
	self.view.backgroundColor = [UIColor colorWithPatternImage:[UIImage imageNamed:@"background.png"]];
	
	[[backgroundHeader layer] setCornerRadius:12.0f];
	[[backgroundHeader layer] setMasksToBounds:YES];
	[[backgroundHeader layer] setBorderColor:[[UIColor paleYellowColor] CGColor]];
	[[backgroundHeader layer] setBackgroundColor:[[UIColor whiteColor] CGColor]];
	[[backgroundHeader layer] setBorderWidth:1.0]; 


	[[backgroundName layer] setCornerRadius:12.0f];
	[[backgroundName layer] setMasksToBounds:YES];
	[[backgroundName layer] setBorderColor:[[UIColor paleYellowColor] CGColor]];
	[[backgroundName layer] setBackgroundColor:[[UIColor whiteColor] CGColor]];
	[[backgroundName layer] setBorderWidth:1.0]; 

	[[backgroundButton layer] setCornerRadius:12.0f];
	[[backgroundButton layer] setMasksToBounds:YES];
	[[backgroundButton layer] setBorderColor:[[UIColor paleYellowColor] CGColor]];
	[[backgroundButton layer] setBackgroundColor:[[UIColor whiteColor] CGColor]];
	[[backgroundButton layer] setBorderWidth:1.0]; 


	[buttonAdd setTitle:NSLocalizedString(@"View Full Profile", @"") forState:UIControlStateNormal];
}


- (void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];
    
    self.navigationItem.title = NSLocalizedString(@"What to do?", @"");
    
    if ([contact.firstName length] || [contact.lastName length])
        name.text = [NSString stringWithFormat:@"%@ %@", contact.firstName, contact.lastName];
    else 
        name.text = contact.userName;
    
    email.text = contact.email;
	
	[VphonetAppDelegate setButtonYellowBorder:buttonPhoto];
	[buttonPhoto setBackgroundImage:[UIImage imageNamed:@"profile_picture.png"] forState:UIControlStateNormal];
	
	if ([contact.photoUpdate length] != 0 )
	{
		NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
		NSString *file = [NSString stringWithFormat:@"%@/photos/%@.jpg",[paths objectAtIndex:0], contact.photoUpdate];
		NSFileManager *fm = [NSFileManager defaultManager]; 
		
		if ([fm fileExistsAtPath:file])
			[buttonPhoto setBackgroundImage:[UIImage imageWithContentsOfFile:file] forState:UIControlStateNormal];
	}

    statusImage.image = [UIImage imageNamed:[contact statusImageName]];
    statusLabel.text = [NSString stringWithString:[contact statusName]];

}

- (void)viewWillDisappear:(BOOL)animated {
    [super viewWillDisappear:animated];
}

- (IBAction) onVoiceClick:(id)sender
{
    [self.navigationController popViewControllerAnimated:NO];
    
    CallViewController *controller = [[CallViewController alloc] initWithContact:contact bearer:BC_VOICE];
    [self.view.window addSubview:controller.view];
}

- (IBAction) onChatClick:(id)sender
{
	
	ChatViewController *controller = [[ChatViewController alloc] initWithNibName:@"ChatViewController" bundle:nil];

	controller.contact = contact;
	
//	self.navigationItem.title = @"your title";
//    ChatViewController *controller = [[ChatViewController alloc] initWithContact:contact];
	
	UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"Back", @"") style: UIBarButtonItemStyleBordered target: nil action: nil];
	[[self navigationItem] setBackBarButtonItem: backButton];
	[backButton release];
 
    [self.navigationController pushViewController:controller animated:YES];
    [controller release]; 
}

- (IBAction) onVideoClick:(id)sender
{
    [self.navigationController popViewControllerAnimated:NO];
    CallViewController *controller = [[CallViewController alloc] initWithContact:contact bearer:BC_AUDIOVIDEO];
    [self.view.window addSubview:controller.view];
}



- (IBAction) onFileClick:(id)sender
{
	UIActionSheet *actionSheet = [[UIActionSheet alloc] initWithTitle:NSLocalizedString(@"Share files", @"")
												  delegate:self
										 cancelButtonTitle:@"Cancel" 
									destructiveButtonTitle:nil 
										 otherButtonTitles:	NSLocalizedString(@"Take Photo or Video", @""),
															NSLocalizedString(@"Choose Existing", @""),
															NSLocalizedString(@"Audio Note", @""),
														   nil];

	actionSheet.actionSheetStyle = UIActionSheetStyleDefault;
	VphonetAppDelegate *appDelegate = (VphonetAppDelegate *)[[UIApplication sharedApplication] delegate];
	
	[actionSheet showInView:[appDelegate window]];	// show from our table view (pops up in the middle of the table)
	[actionSheet release];

}

- (IBAction) onAddClick:(id)sender
{
	SearchProfileViewController *searchProfileViewController = [[SearchProfileViewController alloc] initWithNibName:@"SearchProfileViewController" bundle:nil];
	searchProfileViewController.contact = contact;
	searchProfileViewController.withAddContact = false;

	
	UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"Back", @"") style: UIBarButtonItemStyleBordered target: nil action: nil];
	[[self navigationItem] setBackBarButtonItem: backButton];
	[backButton release];
	
	
	[self.navigationController pushViewController:searchProfileViewController animated:YES];
	
	[searchProfileViewController release];
}

- (IBAction) onDeleteContact:(id)sender
{
	UICustomAlertView *alert = [[UICustomAlertView alloc] 
						  initWithTitle:NSLocalizedString(@"Delete contact", @"")
						  message:NSLocalizedString(@"Are you sure ?", @"")
						  delegate:self cancelButtonTitle:NSLocalizedString(@"No", @"") 
						  otherButtonTitles:NSLocalizedString(@"Yes", @""), nil]; 
	
	[alert show];
	[alert release];
	
}

- (void)actionSheet:(UIActionSheet *)popup clickedButtonAtIndex:(NSInteger)buttonIndex {
	
	if (imagePickerController == nil)
		imagePickerController = [[UIImagePickerController alloc] init];
	imagePickerController.delegate = self;
	imagePickerController.view.hidden = NO;
	
	if ((buttonIndex == 0) && (popup.numberOfButtons == 3)) {
		// Set source to the Camera
		imagePickerController.sourceType =   UIImagePickerControllerSourceTypeCamera;
		[self presentModalViewController:imagePickerController animated:YES];
	}
	else if ((buttonIndex == 1))// && (popup.numberOfButtons == 3))
	{
		imagePickerController.sourceType =  UIImagePickerControllerSourceTypePhotoLibrary; //UIImagePickerControllerSourceTypeSavedPhotosAlbum;
		[self presentModalViewController:imagePickerController animated:YES];
	} 
	
}


- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex {
	if (buttonIndex == 1) {
		[[VPContactList sharedInstance] deleteContact:contact.number];
		[self.navigationController popViewControllerAnimated:YES];
	}
}

// from PickImageAppDelegate.m
- (void)imagePickerController:(UIImagePickerController *)picker 
		didFinishPickingImage:(UIImage *)image
				  editingInfo:(NSDictionary *)editingInfo
{
	// Dismiss the image selection, hide the picker and send file
	[picker dismissModalViewControllerAnimated:YES];
    imagePickerController.view.hidden = YES;
	
	NSArray *path = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *fileName = [VPEngine generateFileName:@"png"];
    NSString *imagePath = [NSString stringWithFormat:@"%@/chats/%@/%@", [path objectAtIndex:0], contact.number, fileName];

//	NSString *imagePath = [[[VPEngine sharedInstance] pathForTemporaryFileWithPrefix:@"img"] stringByAppendingPathExtension:@"png"]; 
	
	NSData *imageData = UIImagePNGRepresentation(image);
	
	// This actually creates the image at the file path specified, with the image's NSData.
	[[NSFileManager defaultManager] createFileAtPath:imagePath contents:imageData attributes:nil];
	
	VPCALL call = [[VPEngine sharedInstance] makeCall:contact.userName bearer:BC_FILE];
	
	VphonetAppDelegate *appDelegate = (VphonetAppDelegate *)[[UIApplication sharedApplication] delegate];
	[appDelegate addCallAndFile:call file:imagePath];
	
	
	ChatViewController *controller = [[ChatViewController alloc] initWithNibName:@"ChatViewController" bundle:nil];
	controller.contact = contact;
	[controller.messages add:[[VPProfile singleton] number] to:contact.number attachment:fileName message:@""];
	
	
	
    [self.navigationController pushViewController:controller animated:YES];
	

    [controller release]; 
	
	//[[VPEngine sharedInstance] sendFile:call file:imagePath];
}

- (void)imagePickerControllerDidCancel:(UIImagePickerController *)picker
{
	[picker dismissModalViewControllerAnimated:YES];	
}


#pragma mark -
#pragma mark UITabBarDelegate
/*
- (void)tabBarController:(UITabBarController *)tabBarController didSelectViewController:(UIViewController *)viewController
{
    // return back to contacts view
//    if ([NSStringFromClass([viewController class]) isEqualToString:@"CallViewController"])
        [self.navigationController popViewControllerAnimated:NO];
}
*/

#pragma mark -
#pragma mark Memory management

/*
// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations.
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}
*/

- (void)didReceiveMemoryWarning {
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc. that aren't in use.
}

- (void)viewDidUnload {
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}


- (void)dealloc {

    [contact release];
	[imagePickerController release];
	[formatter release];
    
  //  VphonetAppDelegate *appDelegate = (VphonetAppDelegate *)[[UIApplication sharedApplication] delegate];
//    [appDelegate.tabBarController setDelegate:tabbarDelegate];
    
    [super dealloc];
    
}


@end

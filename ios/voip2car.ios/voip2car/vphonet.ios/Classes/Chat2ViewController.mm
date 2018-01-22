//
//  ChatViewController.mm
//  Vphonet
//

#import "VPEngine.h"
#import "VphonetAppDelegate.h"
#import "Chat2ViewController.h"


@implementation Chat2ViewController


@synthesize contentView;
@synthesize chatEntry;
@synthesize chatEntryView;

- (id) initWithContact:(VPContact*)vpcontact
{
    self = [super initWithNibName:@"ChatViewController" bundle:nil];
    contact = [vpcontact retain];
	//    VphonetAppDelegate *appDelegate = (VphonetAppDelegate *)[[UIApplication sharedApplication] delegate];
	//	tabbarDelegate = appDelegate.tabBarController.delegate;
	//  [appDelegate.tabBarController setDelegate:self];
    
    return self;
}

/*
- (id)initWithCall:(VPCALL)call
{
    self = [super initWithNibName:@"ChatViewController" bundle:nil];
    vpcall = call;
    
    [[NSNotificationCenter defaultCenter] addObserver:self 
                                          selector:@selector(vpEngineNotification:) 
                                          name:VPEngineNotification
                                          object:nil];		
    return self;
}
*/

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


// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
    
	[self setTitle:[NSString stringWithFormat:@"%@ %@", contact.firstName, contact.lastName]];
	
	self.view.backgroundColor = [UIColor colorWithPatternImage:[UIImage imageNamed:@"background.png"]];
	[contentView setBackgroundColor:[UIColor colorWithPatternImage:[UIImage imageNamed:@"background.png"]]];
	
	
    [self.view insertSubview:chatEntryView aboveSubview:contentView];
    [self chatEntryDidClick:nil];
	
    [chatEntry becomeFirstResponder];
}

- (void)viewWillDisappear:(BOOL)animated {
    [super viewWillDisappear:animated];
    
  //  [[VPEngine sharedInstance] hangup:vpcall];
    [chatEntry resignFirstResponder];
}

/*
// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations.
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}
*/


- (IBAction) chatEntryDidClick:(id)sender
{

    CGRect rect = chatEntryView.frame;
    rect.origin.y = 480.0-36-44-20-216;
    [chatEntryView setFrame:rect];
    
    rect = contentView.frame;
    rect.size.height = 480.0-44-20-216;
    [contentView setFrame:rect];
    

    rect.origin.y = y;
    rect.size.height = y + 10;
    [contentView scrollRectToVisible:rect animated:YES];
}

- (void) appendMessage:(id)sender text:(NSString*)text
{
    // calculate "size" of text
    UIFont *font = [UIFont fontWithName:@"Helvetica-Bold" size:14.0f];

    CGSize textSize = [text sizeWithFont:font];
    unsigned textRows = textSize.width / 245 + 2;
    
    CGRect rect = CGRectMake(0.0, y, 315, textRows*textSize.height + 5);
    y += textRows*textSize.height + 10;

    UIButton *btn = [[UIButton alloc] initWithFrame:rect];
    
    [btn setTitle:text forState:UIControlStateNormal];
    btn.contentEdgeInsets = UIEdgeInsetsMake(0.0f, 10.0f, 0.0f, 0.0f);
    
    if (sender)
        [btn setBackgroundImage:[UIImage imageNamed:@"chatfromleft.png"] forState:UIControlStateNormal];
    else
    {
        [btn setBackgroundImage:[UIImage imageNamed:@"chatfromright.png"] forState:UIControlStateNormal];        
        rect.origin.x = 70.0f;
        [btn setFrame:rect];
    }

    btn.titleLabel.font = font;
    [btn setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
    [btn setEnabled:NO];
	
    btn.contentHorizontalAlignment	=  UIControlContentHorizontalAlignmentLeft;
    btn.contentVerticalAlignment	= UIControlContentVerticalAlignmentCenter;
    btn.titleLabel.lineBreakMode	= UILineBreakModeWordWrap;
	
    [contentView addSubview:btn];
	
	UIImageView* chatView = [[UIImageView alloc] initWithFrame:rect]; 
	[chatView setImage:[UIImage imageNamed:@"chatfromright.png"]];
	
    [btn release];
    
    [contentView setContentSize:CGSizeMake(320, y + rect.size.height+10)];
}


- (UIImage *) getScaledImage:(UIImage *)img insideButton:(UIButton *)btn {
    // Check which dimension (width or height) to pay respect to and
    // calculate the scale factor
    CGFloat imgRatio = img.size.width / img.size.height, 
	btnRatio = btn.frame.size.width / btn.frame.size.height,
	scaleFactor = (imgRatio > btnRatio 
				   ? img.size.width / btn.frame.size.width
				   : img.size.height / btn.frame.size.height);
				   
   // Create image using scale factor
   UIImage *scaledImg = [UIImage imageWithCGImage:[img CGImage]
											scale:scaleFactor
									  orientation:UIImageOrientationUp];
   return scaledImg;
}

- (void) appendFile:(id)sender filePath:(NSString*)filePath
{
    CGRect rect = CGRectMake(0.0, y, 250, 80);//textRows*textSize.height + 5);
    y += 80 + 10;
	
    UIButton *btn = [[UIButton alloc] initWithFrame:rect];
    
    if (sender)
	{
		btn.contentEdgeInsets = UIEdgeInsetsMake(10.0f, 10.0f, 5.0f, 5.0f);
        [btn setBackgroundImage:[UIImage imageNamed:@"chatfromleft.png"] forState:UIControlStateNormal];
	}
    else
    {
		btn.contentEdgeInsets = UIEdgeInsetsMake(5.0f, 5.0f, 10.0f, 10.0f);
        [btn setBackgroundImage:[UIImage imageNamed:@"chatfromright.png"] forState:UIControlStateNormal];        
        rect.origin.x = 70.0f;
        [btn setFrame:rect];
    }
	
	btn.contentMode = UIViewContentModeScaleAspectFit;
	
	UIImage *scaledImg = [self getScaledImage:[UIImage imageWithContentsOfFile:filePath] insideButton:btn];
	[btn setImage:scaledImg forState:UIControlStateNormal];

    [contentView addSubview:btn];
    [btn release];
    
    [contentView setContentSize:CGSizeMake(320, y + rect.size.height+10)];
}


- (IBAction) sendMessage:(id)sender
{
    if ([chatEntry.text length])
    {
//        [[VPEngine sharedInstance] sendChatMsg:vpcall text:chatEntry.text];
        [self appendMessage:sender text:chatEntry.text];
    }
    
    CGRect rect = chatEntryView.frame;
    rect.origin.y = 480.0-49-36-44-22;
    [chatEntryView setFrame:rect];
    
    
    rect = contentView.frame;
    rect.size.height = 367.0;
    [contentView setFrame:rect];
    
    chatEntry.text = @"";
}


- (void) vpEngineNotification:(NSNotification*)notification
{		 	
	unsigned msg	= 	[[[notification userInfo] objectForKey:@"MSG"] unsignedIntValue];
    unsigned param1 = 	[[[notification userInfo] objectForKey:@"PARAM1"] unsignedIntValue];
	//unsigned param2 = 	[[[notification userInfo] objectForKey:@"PARAM2"] unsignedIntValue];
    
 //   if ((VPCALL)param1 != vpcall) return;
    
	NSLog(@"*** Notification message %d",msg);
	
	switch(msg)
	{
        case VPMSG_CHAT:
        {
            
			NSString *msg;// = [[VPEngine sharedInstance] getChatMessage:vpcall];
            if (msg && [msg length])
            {
                [self appendMessage:nil text:msg];
            }
        }
        break;
		case VPMSG_USERALERTED:
			[self appendMessage:nil text:@"Start file transfer ..."];
			break;
		case VPMSG_SENDFILEACK_USERALERTED:	
			break;
		case VPMSG_SENDFILEACK_ACCEPTED:
		{
//			NSString *file = [[[VPEngine sharedInstance] getFilePath:vpcall] lastPathComponent];
//			[self appendMessage:nil text:[NSString  stringWithFormat:@"Sending file %@", file]];
		}
			break;
		case VPMSG_FTPROGRESS:
			NSLog(@"Progress");
			break;
		case VPMSG_SENDFILEREQ:
		{
//			[[VPEngine sharedInstance] acceptFile:vpcall];
			//NSString *file = [[[VPEngine sharedInstance] getFilePath:vpcall] lastPathComponent];
			//[self appendMessage:nil text:[NSString  stringWithFormat:@"Accept file %@ ...", file]];
		}
			break;
		case VPMSG_FTTXCOMPLETE:
        {
			VphonetAppDelegate *appDelegate = (VphonetAppDelegate *)[[UIApplication sharedApplication] delegate];
//			[self appendFile:self filePath:[appDelegate getCallFileName:vpcall]];
        } 
			break;
        case    VPMSG_FTRXCOMPLETE:
		{
			VphonetAppDelegate *appDelegate = (VphonetAppDelegate *)[[UIApplication sharedApplication] delegate];
//			[self appendFile:nil filePath:[appDelegate getCallFileName:vpcall]];
		}
			break;
//		case VPMSG_SENDFILESUCCESS:
//			[self appendMessage:nil text:@"File transfer finished successfully ..."];
//			break;
		case VPMSG_SENDFILESUCCESS:
		{
			VPCALL call = (VPCALL)param1;
//			[self appendFile:nil fileName:[[VPEngine sharedInstance] getFilePath:call]];
			/*			
			 ChatViewController *controller = [[ChatViewController alloc] initWithFile:call];
			 
			 [tabBarController.navigationController setNavigationBarHidden:NO animated:NO];
			 [tabBarController.navigationController pushViewController:controller animated:YES];
			 [controller release];
			 */
		}
			break;
	
			
	}
}


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
    [super dealloc];
    
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}


@end

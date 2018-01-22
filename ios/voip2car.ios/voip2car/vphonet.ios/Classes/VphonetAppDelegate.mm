//
//  VphonetAppDelegate.m
//  Vphonet

#import "VPSound.h"
#import "VPDatabase.h"
#import "VPProfile.h"
#import "VphonetAppDelegate.h"
#import "LoginViewController.h"
#import "CallViewController.h"
#import "ChatViewController.h"
#import "UICustomAlertView.h"


@implementation UIColor (Extensions)

+ (UIColor *)paleYellowColor {
	return [UIColor colorWithRed:240.0/255.0 green:191.0/255.0 blue:27.0/255.0 alpha:1.0];
}

+ (UIColor *)titleColor {
	return [UIColor colorWithRed:0.25 green:0.25 blue:0.25 alpha:0.5];
}


@end


@implementation UINavigationBar (CustomImage)
- (void)drawRect:(CGRect)rect {
	UIImage *image = [UIImage imageNamed: @"TitleYellow.png"];
	[image drawInRect:CGRectMake(0, 0, self.frame.size.width, self.frame.size.height)];
}
@end

/*
@implementation UIViewController (CustomTitle)

- (void)viewDidLoad {
    //[super viewDidLoad];
    UILabel *titleView = (UILabel *)self.navigationItem.titleView;
    if (!titleView) {
        titleView = [[UILabel alloc] initWithFrame:CGRectZero];
        titleView.backgroundColor = [UIColor clearColor];
        titleView.font = [UIFont boldSystemFontOfSize:20.0];
        titleView.shadowColor = [UIColor colorWithWhite:0.0 alpha:0.5];
		
        titleView.textColor = [UIColor colorWithRed:0.25 green:0.25 blue:0.25 alpha:0.5]; // Change to desired color
		
        self.navigationItem.titleView = titleView;
        [titleView release];
    }
    titleView.text = self.navigationItem.title;
    [titleView sizeToFit];
	self.navigationController.navigationBar.tintColor = [UIColor colorWithRed:.6 green:.6 blue:.6 alpha:0.5];
}

@end
*/

@implementation UIViewController (CustomTitle)
- (void)setTitle:(NSString *)title
{
//    [super setTitle:title];
    UILabel *titleView = (UILabel *)self.navigationItem.titleView;
    if (!titleView) {
        titleView = [[UILabel alloc] initWithFrame:CGRectZero];
        titleView.backgroundColor = [UIColor clearColor];
        titleView.font = [UIFont boldSystemFontOfSize:20.0];
        titleView.shadowColor = [UIColor colorWithWhite:0.0 alpha:0.5];
		
        titleView.textColor = [UIColor colorWithRed:0.25 green:0.25 blue:0.25 alpha:0.5]; // Change to desired color
		
        self.navigationItem.titleView = titleView;
        [titleView release];
    }
    titleView.text = title;
    [titleView sizeToFit];
	self.navigationController.navigationBar.tintColor = [UIColor colorWithRed:.6 green:.6 blue:.6 alpha:0.5];
}

- (void)setCustomBackground:(NSString*)image
{
	self.view.backgroundColor = [UIColor colorWithPatternImage:[UIImage imageNamed:image]];
//	self.tableView.backgroundColor = [UIColor clearColor];
//	[self.tableView setSeparatorColor:[UIColor paleYellowColor]];
}


@end

@implementation UINavigationController (CustomTitle)
- (void)setTitle:(NSString *)title
{
    [super setTitle:title];
    UILabel *titleView = (UILabel *)self.navigationItem.titleView;
    if (!titleView) {
        titleView = [[UILabel alloc] initWithFrame:CGRectZero];
        titleView.backgroundColor = [UIColor clearColor];
        titleView.font = [UIFont boldSystemFontOfSize:20.0];
        titleView.shadowColor = [UIColor colorWithWhite:0.0 alpha:0.5];
		
        titleView.textColor = [UIColor colorWithRed:0.25 green:0.25 blue:0.25 alpha:0.5]; // Change to desired color
		
        self.navigationItem.titleView = titleView;
        [titleView release];
    }
    titleView.text = title;
    [titleView sizeToFit];
	self.navigationController.navigationBar.tintColor = [UIColor colorWithRed:.6 green:.6 blue:.6 alpha:0.5];
}
@end

@implementation UITableViewController (CustomTitle)
- (void)setTitle:(NSString *)title
{
    [super setTitle:title];
    UILabel *titleView = (UILabel *)self.navigationItem.titleView;
    if (!titleView) {
        titleView = [[UILabel alloc] initWithFrame:CGRectZero];
        titleView.backgroundColor = [UIColor clearColor];
        titleView.font = [UIFont boldSystemFontOfSize:20.0];
        titleView.shadowColor = [UIColor colorWithWhite:0.0 alpha:0.5];
		
        titleView.textColor = [UIColor colorWithRed:0.25 green:0.25 blue:0.25 alpha:0.5]; // Change to desired color
		
        self.navigationItem.titleView = titleView;
        [titleView release];
    }
    titleView.text = title;
    [titleView sizeToFit];
	self.navigationController.navigationBar.tintColor = [UIColor colorWithRed:.6 green:.6 blue:.6 alpha:0.5];
}

- (void)setCustomBackground:(NSString*)image
{
	self.parentViewController.view.backgroundColor = [UIColor colorWithPatternImage:[UIImage imageNamed:image]];
	self.tableView.backgroundColor = [UIColor clearColor];
	[self.tableView setSeparatorColor:[UIColor paleYellowColor]];
}


- (void)viewDidLoad {
    [super viewDidLoad];
}

@end





@implementation VphonetAppDelegate

@synthesize window=_window;

@synthesize tabBarController=_tabBarController;

//@synthesize window;
//@synthesize navigationController;


#pragma mark -
#pragma mark Application lifecycle

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {    

    // Localization
	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
	NSArray *languages = [defaults objectForKey:@"AppleLanguages"];
	NSString *currentLanguage = [languages objectAtIndex:0];
	
	NSLog(@"Current Locale: %@", [[NSLocale currentLocale] localeIdentifier]);
	NSLog(@"Current language: %@", currentLanguage);


	[UICustomAlertView setBackgroundColor:[UIColor paleYellowColor] 
					withStrokeColor:[UIColor whiteColor]];
	
    // Override point for customization after application launch.

    incommingAlerts = [[NSMutableArray alloc] init];
	callsAndFiles = [[NSMutableDictionary alloc] init];
	
	[[NSNotificationCenter defaultCenter] addObserver:self 
										  selector:@selector(vpStackNotification:) 
										  name:VPEngineNotification
										  object:nil];
	
	[[NSNotificationCenter defaultCenter] addObserver:self 
                                             selector:@selector(onSmsNotification:) 
                                                 name:OnSmsNotification
                                               object:nil];	


	
	self.window.rootViewController = self.tabBarController;
	[self.window makeKeyAndVisible];
	
	VPProfile* info = [[[VPProfile alloc] initFromDatabase] autorelease];
	if ([[info userName] length] > 0 ) {
		[[VPEngine sharedInstance] login:[info userName] password:[info password]];
		[VPProfile singleton].userName = [info userName];
		[VPProfile singleton].password = [info password];
	}
	else
	{
		[self showLogin:NO];
	}
	
//	VphonetAppDelegate *appDelegate = (VphonetAppDelegate*) [[UIApplication sharedApplication] delegate];    

	//	[self.window makeKeyAndVisible];
	
	
	[[VPSound singleton] start];
	[VPDatabase sharedInstance];

    return YES;
}


- (void)showLogin:(bool)clearFields {
	LoginViewController *loginViewController = [[LoginViewController alloc]  initWithNibName:@"LoginViewController" bundle:nil];
	if (clearFields == YES) {
		loginViewController.user.text = @"";
		[[loginViewController user] setText:@""];
		[[loginViewController password] setText:@""];
	}
	
	UINavigationController *navController = [[UINavigationController alloc] initWithRootViewController:loginViewController];
	[loginViewController release];
	
	navController.modalTransitionStyle = UIModalTransitionStyleCoverVertical;
	[self.tabBarController presentModalViewController:navController animated:NO];
	[navController release];
}

- (void)dealloc {
	[callsAndFiles release];
	[_window release];
	[_tabBarController release];

	//    [tabBarController release];
//	[navigationController release];
//    [window release];
    [super dealloc];
}



- (void)applicationWillResignActive:(UIApplication *)application {
    /*
     Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
     Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
     */
}


- (void)applicationDidEnterBackground:(UIApplication *)application {
    /*
     Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later. 
     If your application supports background execution, called instead of applicationWillTerminate: when the user quits.
     */
}


- (void)applicationWillEnterForeground:(UIApplication *)application {
    /*
     Called as part of  transition from the background to the inactive state: here you can undo many of the changes made on entering the background.
     */
}


- (void)applicationDidBecomeActive:(UIApplication *)application {
    /*
     Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
     */
}


- (void)applicationWillTerminate:(UIApplication *)application {
    /*
     Called when the application is about to terminate.
     See also applicationDidEnterBackground:.
     */
}


#pragma mark -
#pragma mark UITabBarControllerDelegate methods

/*
// Optional UITabBarControllerDelegate method.
- (void)tabBarController:(UITabBarController *)tabBarController didSelectViewController:(UIViewController *)viewController {
}
*/

/*
// Optional UITabBarControllerDelegate method.
- (void)tabBarController:(UITabBarController *)tabBarController didEndCustomizingViewControllers:(NSArray *)viewControllers changed:(BOOL)changed {
}
*/
#pragma mark -
#pragma mark Notifications

- (void) onSmsNotification:(NSNotification*)notification
{		 	
	unsigned typeSMS = [[[notification userInfo] objectForKey:@"type"] unsignedIntValue];
	
	switch (typeSMS) {
		case SMSTYPE_CONTACTADDED:
			break;
		case SMSTYPE_CONTACTADDED_ACK:
			break;
		case SMSTYPE_CONTACTADDED_NACK:
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

- (void) vpStackNotification:(NSNotification*)notification
{		 	
	unsigned msg	= 	[[[notification userInfo] objectForKey:@"MSG"] unsignedIntValue];
	unsigned param1 = 	[[[notification userInfo] objectForKey:@"PARAM1"] unsignedIntValue];
	//unsigned param2 = 	[[[notification userInfo] objectForKey:@"PARAM2"] unsignedIntValue];
		
	NSLog(@"--  %d", msg);
	
	switch(msg)
	{
		case VPMSG_SERVERSTATUS:
		//	if (param1 == REASON_NORMAL) {
		//		self.window.rootViewController = self.tabBarController;
		//		[self.window makeKeyAndVisible];
		//	}
			break;
			
			
		case VPMSG_NEWCALL:
		{
			 VPCALL call = (VPCALL)param1;
             unsigned bc = [[VPEngine sharedInstance] getCallBearer:call];
             if (bc == BC_VIDEO || bc == BC_AUDIOVIDEO || bc == BC_VOICE)
             {
                 if ([[VPEngine sharedInstance] getCallStatus:call] != VPCS_CONNECTED) 
                 {
					 [[VPSound singleton] playRingTone];
                     UIAlertView *alert = [[UIAlertView alloc] 
                        initWithTitle:[NSString stringWithFormat:@"Vphonet - %@", bc == BC_VOICE ? @"Voice" : @"Video"]
                        message:[NSString stringWithFormat:@"Call from %@", 
                                    [[VPEngine sharedInstance] getRemoteCallName:call]]
                        delegate:self cancelButtonTitle:@"Cancel"
                        otherButtonTitles:@"Answer", nil]; 
                     alert.tag = (NSInteger)call;
                     [incommingAlerts addObject:alert];
                     [alert show];
                 }
             }
             else if (bc == BC_CHAT)
             {
                 [[VPEngine sharedInstance] answer:call];
                 ChatViewController *controller = [[ChatViewController alloc] initWithCall:call];
                 
//                 [tabBarController.navigationController setNavigationBarHidden:NO animated:NO];
  //               [tabBarController.navigationController pushViewController:controller animated:YES];
                 [controller release];
             }
             else if (bc == BC_FILE)
             {
                 if ([[VPEngine sharedInstance] getCallStatus:call] != VPCS_CONNECTED) 
                 {
					 [[VPSound singleton] playRingTone];
                     UIAlertView *alert = [[UIAlertView alloc] 
										   initWithTitle:[NSString stringWithFormat:@"Vphonet - %@", @"File"]
										   message:[NSString stringWithFormat:@"File from %@", 
													[[VPEngine sharedInstance] getRemoteCallName:call]]
										   delegate:self cancelButtonTitle:@"Cancel"
										   otherButtonTitles:@"Accept", nil]; 
                     alert.tag = (NSInteger)call;
                     [incommingAlerts addObject:alert];
                     [alert show];
                 }
             }

		}
        break;	
	    case VPMSG_CALLENDED: 
			{
				[[VPSound singleton] stopRingTone];
				VPCALL call = (VPCALL)param1;
			   for (UIAlertView *alert in incommingAlerts)
			   {
				   if (alert.tag == (NSInteger)call)
				   {
					   [alert dismissWithClickedButtonIndex:0 animated:YES];
					   [self alertView:alert clickedButtonAtIndex:0];
					   break;
				   }
			   }
			}
			break;
		case VPMSG_CONNECTFINISHED:
			{
				VPCALL call = (VPCALL)param1;
				unsigned bc = [[VPEngine sharedInstance] getCallBearer:call];
				if (bc == BC_FILE)
				{
					NSString* fileName = [callsAndFiles objectForKey:[NSNumber numberWithInt:(int)call]];
					[[VPEngine sharedInstance] sendFile:call file:fileName];
				}
				
			}
			break;
		case VPMSG_SENDFILEACK_ACCEPTED:
			{
//				UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Send File"
//												   message:@"Progress"
//												  delegate:self 
//										 cancelButtonTitle:@"Cancel" 
//										 otherButtonTitles: nil];
//				
//				[alert show];
			}
			break;
		case VPMSG_USERALERTED://VPMSG_SENDFILEACK_USERALERTED: //
			{	
				VPCALL call = (VPCALL)param1;
				unsigned bc = [[VPEngine sharedInstance] getCallBearer:call];
				if (bc == BC_FILE)
				{
					UIAlertView *alert = nil;
					for (UIAlertView *item in incommingAlerts)
					{
						if (item.tag == (NSInteger)call)
						{
							alert = item;
							break;
						}
					}
					
					if (alert == nil) {
						alert = [[UIAlertView alloc] initWithTitle:@"Send File"
														   message:@""
														  delegate:self 
												 cancelButtonTitle:@"Cancel" 
											  otherButtonTitles: nil];
//						UIProgressView *progressFile = [[UIProgressView alloc] initWithFrame:CGRectMake(30, 50, 220, 50)];
//						progressFile.progressViewStyle = UIProgressViewStyleDefault;
//						progressFile.progress = 50.0f;
//						[alert addSubview:progressFile];
						alert.tag = (NSInteger)call;
						[incommingAlerts addObject:alert];
					}
					alert.message = @"Waiting user accepted ...";
					[alert show];
				}  
			}
			break;
		case VPMSG_SENDFILEREQ:
			{
				VPCALL call = (VPCALL)param1;
				unsigned bc = [[VPEngine sharedInstance] getCallBearer:call];
				if (bc == BC_FILE)
				{
					UIAlertView *alert = nil;
					for (UIAlertView *item in incommingAlerts)
					{
						if (item.tag == (NSInteger)call)
						{
							alert = item;
							break;
						}
					}
					
					if (alert == nil) {
						alert = [[UIAlertView alloc] initWithTitle:@"Recieve File"
														   message:@""
														  delegate:self 
												 cancelButtonTitle:@"Cancel" 
												 otherButtonTitles: nil];
						UIProgressView *progressFile = [[UIProgressView alloc] initWithFrame:CGRectMake(30, 55, 220, 50)];
						progressFile.progressViewStyle = UIProgressViewStyleDefault;
						progressFile.progress = 0.0f;
						[alert addSubview:progressFile];
						alert.tag = (NSInteger)call;
						[incommingAlerts addObject:alert];
					}
					alert.message = @" "; // If message nil, height don't include alert.message 
					[alert show];
				}  
			}
			break;
		case VPMSG_FTTXCOMPLETE:
			{
			} 
			break;
        case    VPMSG_FTRXCOMPLETE:
			{
			}
			break;
		case VPMSG_SENDFILESUCCESS:
			{
				VPCALL call = (VPCALL)param1;
				ChatViewController *controller = [[ChatViewController alloc] initWithCall:call];
				
//				[tabBarController.navigationController setNavigationBarHidden:NO animated:NO];
//				[tabBarController.navigationController pushViewController:controller animated:YES];
				[controller release];

				for (UIAlertView *alert in incommingAlerts)
				{
					if (alert.tag == (NSInteger)call)
					{
						[alert dismissWithClickedButtonIndex:0 animated:YES];
						[self alertView:alert clickedButtonAtIndex:0];
						break;
					}
				}
			}
			break;
		case VPMSG_SENDFILEFAILED:
			{
				UIAlertView* alert = [[UIAlertView alloc] initWithTitle:@"Send File"
												   message:@"Failed !!!"
												  delegate:self 
										 cancelButtonTitle:@"Ok" 
										 otherButtonTitles: nil];
				[alert show];
				[alert release];
			}
			break;
	}
}


#pragma mark -
#pragma mark AlertView delegate
- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex
{
	
	
    VPCALL call = (VPCALL)alertView.tag;
	
	
	if (call)
	{
		unsigned bc = [[VPEngine sharedInstance] getCallBearer:call];
		if (buttonIndex == 1) // Answer
		{
			[[VPSound singleton] stopRingTone];
			[[VPEngine sharedInstance] answer:call];
			if (bc == BC_VIDEO || bc == BC_AUDIOVIDEO || bc == BC_VOICE)
			{
//				CallViewController *callViewController = [[CallViewController alloc] initWithCall:call];
				//[window addSubview:callViewController.view];
			} else if (bc == BC_FILE)
			{
				[[VPEngine sharedInstance] answer:call];
				
				[self addCallAndFile:call file:[[VPEngine sharedInstance] getFilePath:call]];
				 
				NSLog(@"*** %@", [[VPEngine sharedInstance] getFilePath:call]);
				//[[VPEngine sharedInstance] acceptFile:call];
				//ChatViewController *controller = [[ChatViewController alloc] initWithCall:call];
				
				//[tabBarController.navigationController setNavigationBarHidden:NO animated:NO];
				//[tabBarController.navigationController pushViewController:controller animated:YES];
				//[controller release];
			}
		}
		else 
		{
//			if (bc == BC_FILE)
//				[[VPEngine sharedInstance] rejectFile:call];
//			else	
				[[VPEngine sharedInstance] hangup:call];
		}

	}
	
    [incommingAlerts removeObject:alertView];
    
}
#pragma mark -
#pragma mark General functions
- (void) addCallAndFile:(VPCALL)call file:(NSString*)file
{
	[callsAndFiles setObject:file forKey:[NSNumber numberWithInt:(int)call]];
}

- (NSString*) getCallFileName:(VPCALL)call
{
	return [callsAndFiles objectForKey:[NSNumber numberWithInt:(int)call]];
}




#pragma mark -
#pragma mark Memory management

- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application {
    /*
     Free up as much memory as possible by purging cached data objects that can be recreated (or reloaded from disk) later.
     */
}

+ (void) setButtonYellowBorder:(UIButton *)button
{
	[[button layer] setCornerRadius:12.0f];
	[[button layer] setMasksToBounds:YES];
	[[button layer] setBorderColor:[[UIColor paleYellowColor] CGColor]];
	[[button layer] setBackgroundColor:[[UIColor whiteColor] CGColor]];
	[[button layer] setBorderWidth:1.0]; 
}



@end


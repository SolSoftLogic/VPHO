//
//  VideocallViewController.m
//  Vphonet

#import "vpstack/ios/video.h"
#import "VPEngine.h"

#import "VphonetAppDelegate.h"
#import "CallViewController.h"
#import "ContactSelectorViewController.h"

@implementation Caller
@synthesize call;
@synthesize callTime;
@synthesize videoView;
@synthesize callerNameLabel;
@synthesize timeLabel;
@end

@implementation CallViewController

@synthesize optionsCall;
@synthesize buttonHold;
@synthesize buttonMute;
@synthesize buttonVideo;
@synthesize buttonSpeaker;

static int numberOfActiveCalls = 0;

- (id) init
{
    self = [super init];
    
    [[UIApplication sharedApplication] setStatusBarStyle:UIStatusBarStyleBlackTranslucent];
    [UIApplication sharedApplication].idleTimerDisabled = YES;
    
    numberOfActiveCalls++;
    
    callers = [[NSMutableArray alloc] init];
    
    [[NSNotificationCenter defaultCenter] addObserver:self 
                                             selector:@selector(vpVideoDidStartNotification:) 
                                                 name:VPVideoDidStartNotification
                                               object:nil];	
    
    
    [[NSNotificationCenter defaultCenter] addObserver:self 
                                             selector:@selector(vpVideoDidFinishNotification:) 
                                                 name:VPVideoDidFinishNotification
                                               object:nil];		
    
    
    
    
    [[NSNotificationCenter defaultCenter] addObserver:self 
                                             selector:@selector(vpEngineNotification:) 
                                                 name:VPEngineNotification
                                               object:nil];		
    
    return self;
        
}

- (id) initWithCall:(VPCALL)call
{
    self = [self init];
    if (self)
	{
		// Add call to array. 
        // We still don't know is it video or just voice call.
        Caller *caller = [[[Caller alloc] init] autorelease];
        caller.call = call;
        [callers addObject:caller];
	}
	return self;
    
    
}

- (id)initWithContact:(VPContact*)vpcontact bearer:(unsigned)bearer
{
    self = [self init];
    contact = [vpcontact retain];
    VPCALL call = [[VPEngine sharedInstance] makeCall:contact.userName bearer:bearer];
    Caller *caller = [[[Caller alloc] init] autorelease];
    caller.call = call;
    [callers addObject:caller];	
    return self;
}

- (void) setupTopView
{
    topView = [[[UIView alloc] initWithFrame:CGRectMake(0.0f, 20.0f, 320.0f, 80.0f)] autorelease];
    [self.view addSubview:topView];
    
    // Background
    UIImageView *imgView = [[[UIImageView alloc] initWithFrame:CGRectMake(0.0f, 0.0f, 480.0f, 80.0f)] autorelease];
    imgView.image = [UIImage imageNamed:@"blackbg2.png"];
    [topView addSubview:imgView];
    
    // Caller name
    Caller *caller = [callers objectAtIndex:0];
    caller.callerNameLabel = [[[UILabel alloc] initWithFrame:CGRectMake(18.0f, 5.0f, 211.0f, 37.0f)] autorelease];
    caller.callerNameLabel.font = [UIFont systemFontOfSize:24.0f];
    [caller.callerNameLabel setTextAlignment:UITextAlignmentCenter];
    [caller.callerNameLabel setOpaque:NO];
    [caller.callerNameLabel setBackgroundColor:[UIColor colorWithRed:0.0 green:0.0f blue:0.0f alpha:0.0f ]];
    [caller.callerNameLabel setTextColor:[UIColor whiteColor]];
    NSString *name;
    if (contact)
    {
        if ([contact.firstName length] || [contact.lastName length])
            name = [NSString stringWithFormat:@"%@ %@", contact.firstName, contact.lastName];
        else 
            name = [NSString stringWithString:contact.userName];
    }
    else 
    {
        name = [[VPEngine sharedInstance] getRemoteCallName:caller.call];
    }
    caller.callerNameLabel.text = name;
    [imgView addSubview:caller.callerNameLabel];
    
    // Timing
    caller.timeLabel = [[[UILabel alloc] initWithFrame:CGRectMake(29.0f, 43.0f, 190.0f, 24.0f)] autorelease];
    caller.timeLabel.font = [UIFont systemFontOfSize:20.0f];
    [caller.timeLabel setTextAlignment:UITextAlignmentCenter];
    [caller.timeLabel setOpaque:NO];
    [caller.timeLabel setBackgroundColor:[UIColor colorWithRed:0.0 green:0.0f blue:0.0f alpha:0.0f ]];
    [caller.timeLabel setTextColor:[UIColor whiteColor]];    
    [imgView addSubview:caller.timeLabel];
    
    
    if (contact)
        caller.timeLabel.text = @"Calling...";
    else 
        caller.timeLabel.text = @"00:00";    
}


- (void) setupBottomView
{
    bottomView = [[[UIView alloc] initWithFrame:CGRectMake(0.0, 436.0, 320.0, 44.0f)] autorelease];
    [self.view addSubview:bottomView];

    // Toolbar
    NSMutableArray *toolbarButtons = [[NSMutableArray alloc] init];
    UIToolbar *toolbar = [[[UIToolbar alloc] initWithFrame:CGRectMake(0.0f, 0.0f, 320.0, 44.0f)] autorelease];
    [toolbar setBarStyle:UIBarStyleBlackOpaque];
    
    // Fexibility (aligin endcall button)
    UIBarButtonItem *item  = [[[UIBarButtonItem alloc] 
                              initWithBarButtonSystemItem: 
                              UIBarButtonSystemItemFlexibleSpace
                              target:nil action:NULL] autorelease]; 
    [toolbarButtons addObject:item];
    
    // End call button
    UIButton *endCallButton =  [[UIButton buttonWithType:UIButtonTypeCustom] autorelease]; 
	[endCallButton setFrame:CGRectMake(23.0f, 6.0f, 274.0f, 33.0f)];		
    [endCallButton setBackgroundImage:[UIImage imageNamed:@"redButton.png"] forState:UIControlStateNormal];     
	[endCallButton setOpaque:NO];
    [endCallButton addTarget:self action:@selector(hangup:) forControlEvents:UIControlEventTouchUpInside];
     // "End Call" label and image
    UIImageView *imgView = [[[UIImageView alloc] initWithFrame:CGRectMake(76.0f, 2.0f, 40.0f, 27.0f)] autorelease];
    imgView.image = [UIImage imageNamed:@"endcall_icon.png"];
    [endCallButton addSubview:imgView];
    UILabel *label = [[[UILabel alloc] initWithFrame:CGRectMake(126.0f, 5.0f, 70.0f, 21.0f)] autorelease];
    label.text = @"End Call";
    label.font = [UIFont boldSystemFontOfSize:17];
    [label setOpaque:NO];
    [label setBackgroundColor:[UIColor colorWithRed:0.0 green:0.0f blue:0.0f alpha:0.0f ]];
    [label setTextColor:[UIColor whiteColor]];
    [endCallButton addSubview:label];
    
    item  = [[UIBarButtonItem alloc] initWithCustomView:endCallButton];
    [toolbarButtons addObject:item];
    [item release];
    
    // Fexibility (aligin endcall button)
    item  = [[[UIBarButtonItem alloc] 
             initWithBarButtonSystemItem: 
             UIBarButtonSystemItemFlexibleSpace
             target:nil action:NULL] autorelease]; 
    [toolbarButtons addObject:item];
    [toolbar setItems:toolbarButtons animated:YES]; 
    [bottomView addSubview:toolbar];    
    
}


- (void)loadView {

	// main view
    UIView *mainView = [[[UIView alloc] initWithFrame:CGRectMake(0.0, 0.0, 320.0, 480.0f)] autorelease];
	self.view = mainView;

    // caller(s) video view(s)
    callerView = [[[UIImageView alloc] initWithFrame:CGRectMake(0.0, 0.0, 320.0, 436.0f)] autorelease];
    callerView.backgroundColor = [UIColor colorWithRed:15/256.0f green:15/256.0f blue:15/256.0f alpha:1.0f];
    callerView.image = [UIImage imageNamed:@"default_contact2.png"];
    [self.view addSubview:callerView];
    
	// button for screen click
	UIButton* screenButton = [[[UIButton alloc] initWithFrame:CGRectMake(0.0, 0.0, 320.0, 436.0f)] autorelease];
    screenButton.backgroundColor = [UIColor clearColor];
    //screenButton.backgroundColor = [UIColor blackColor];
	[screenButton addTarget:self action:@selector(onScreenClick:) forControlEvents:UIControlEventTouchDown];
    [self.view addSubview:screenButton];
	
	
	if (optionsCall == nil) {
		NSArray* nibViews =  [[NSBundle mainBundle] loadNibNamed:@"CallOptionsView" owner:self options:nil];
		optionsCall = [nibViews objectAtIndex:0];
		optionsCall.frame = CGRectMake(20.0, 140.0, 280.0, 210.0);
		optionsCall.layer.cornerRadius = 10;
		[optionsCall.layer setBorderColor: [[UIColor whiteColor] CGColor]];
		[optionsCall.layer setBorderWidth: 2.0];
		[optionsCall setHidden:YES];	
		[self.view addSubview:optionsCall];
    }
    
    [self setupTopView];
    [self setupBottomView];
}


- (void)onScreenClick:(id)sender
{
	if ([callers count] == 2)
    {
        [bottomView setHidden:![bottomView isHidden]];
    }
    else
	{
        Caller *first = [callers objectAtIndex:0];
		VPCALL call = first.call;
		if ([[VPEngine sharedInstance] getCallBearer:call] == BC_VOICE)
		{
			[buttonVideo setTitle:@"Video" forState:UIControlStateNormal];
		}
		else
		{
			[buttonVideo setTitle:@"Capture" forState:UIControlStateNormal];
		}
		
		if ([[VPEngine sharedInstance] isMute])
			[buttonMute setBackgroundColor: [UIColor colorWithRed:255/255.0f green:0/255.0f blue:0/255.0f alpha:1.0f]];
		else
			[buttonMute setBackgroundColor: [UIColor clearColor]];
		
		if ([[VPEngine sharedInstance] isSpeaker])
			[buttonSpeaker setBackgroundColor: [UIColor colorWithRed:255/255.0f green:0/255.0f blue:0/255.0f alpha:1.0f]];
		else
			[buttonSpeaker setBackgroundColor: [UIColor clearColor]];
		
	}
	
	[optionsCall setHidden:NO];
}

- (IBAction) onMuteClick: (id)sender
{
	if ([callers count])
	{
		[[VPEngine sharedInstance] mute:![[VPEngine sharedInstance] isMute]];
		if ([[VPEngine sharedInstance] isMute])
		{
			[buttonMute setBackgroundColor: [UIColor colorWithRed:255/255.0f green:0/255.0f blue:0/255.0f alpha:1.0f]];
		}
		else
		{
			[buttonMute setBackgroundColor: [UIColor clearColor]];
		}
		
	}

	[optionsCall setHidden:YES];	
}

- (IBAction) onKeypadClick: (id)sender
{
//	VphonetAppDelegate *appDelegate = (VphonetAppDelegate *)[[UIApplication sharedApplication] delegate];
//	[appDelegate.tabBarController setSelectedIndex:2];
	[optionsCall setHidden:YES];	
	[self.view removeFromSuperview];
	//TODO: Next ??? X.Z.
}

- (IBAction) onSpeekerClick: (id)sender
{
	if ([callers count])
	{
		[[VPEngine sharedInstance] speaker:![[VPEngine sharedInstance] isSpeaker]];
		if ([[VPEngine sharedInstance] isSpeaker])
		{
			[buttonSpeaker setBackgroundColor: [UIColor colorWithRed:255/255.0f green:0/255.0f blue:0/255.0f alpha:1.0f]];
		}
		else
		{
			[buttonSpeaker setBackgroundColor: [UIColor clearColor]];
		}
		
	}
		
	[optionsCall setHidden:YES];	
}

- (IBAction) onAddCallClick: (id)sender
{
    if ([callers count] > 1) return;

	ContactSelectorViewController *c = [[ContactSelectorViewController alloc] init];
    c.delegate = self;
    UINavigationController *navc = [[UINavigationController alloc] initWithRootViewController:c];
	[[navc view] setFrame:CGRectMake(0.0f, 480.0f, 320.0f, 480.0f)];
    [self.view addSubview:[navc view]];
    
    
	[UIView beginAnimations:nil context:NULL];
	[UIView setAnimationDuration:0.25f];
    
	[[navc view] setFrame:CGRectMake(0.0f, 0.0f, 320.0f, 480.0f)];
    
	[UIView commitAnimations];
    
    [optionsCall setHidden:YES];
	//TODO: Next ??? X.Z.
}

- (IBAction) onHoldClick: (id)sender
{
	if ([callers count])
	{
        Caller *first = [callers objectAtIndex:0];
		VPCALL call = first.call;
		if ([[VPEngine sharedInstance] isHold:call])
		{
			[buttonHold setBackgroundColor: [UIColor clearColor]];
			[[VPEngine sharedInstance] resume:call];
		}
		else
		{
			 //[UIColor colorWithRed:255/255.0 green:251/255.0 blue:204/255.0 alpha:1];
			[buttonHold setBackgroundColor: [UIColor colorWithRed:255/255.0f green:0/255.0f blue:0/255.0f alpha:1.0f]];
			[[VPEngine sharedInstance] hold:call];
		
		}

	}
	
	[optionsCall setHidden:YES];
}

- (IBAction) onCaptureClick: (id)sender
{
    if ([callers count] == 1)
    {
        Caller *first = [callers objectAtIndex:0];
        if ([[VPEngine sharedInstance] getCallBearer:first.call] == BC_VOICE) 
        {
            // VOICE caller, make video call
            if (!contact)
            {
                contact = [[VPContact alloc] init];
                contact.userName = [[VPEngine sharedInstance] getRemoteCallName:first.call];
            }
            CallViewController *controller = [[CallViewController alloc] initWithContact:contact bearer:BC_AUDIOVIDEO];

            [self.view.superview addSubview:controller.view];
            [self hangup:self];
        }
        else 
        {
            /*ALAssetsLibrary *library = [[ALAssetsLibrary alloc] init];
        
             [library writeImageToSavedPhotosAlbum:[image CGImage] orientation:(ALAssetOrientation)[image imageOrientation] completionBlock:^(NSURL *assetURL, NSError *error){
            if (error) {
                // TODO: error handling
            } else {
                // TODO: success handling
            }
             }];
             [library release];*/
         
            if (first.videoView)
            {
                UIImageWriteToSavedPhotosAlbum(first.videoView.image, nil, nil, nil);
            }
        }
    }

//	[optionsCall setHidden:YES];	
}

- (IBAction) onEndCallClick: (id)sender
{
	[optionsCall setHidden:YES];
}



- (void) updateGUI
{
	struct tm ptm;
	char timeFormatted[25];
    
    for (Caller *caller in callers)
    {
        time_t timediff = time(0) - caller.callTime;
        localtime_r(&timediff, &ptm);    
        if (timediff < 3600)
            strftime(timeFormatted, sizeof timeFormatted, "%M:%S", &ptm);    
        else 
            strftime(timeFormatted, sizeof timeFormatted, "%H:%M:%S", &ptm);
        
        
        [caller.timeLabel setText:[NSString stringWithCString:timeFormatted encoding:NSASCIIStringEncoding]];
        
    }
    
	    
    updateGUITimer = [NSTimer scheduledTimerWithTimeInterval:1.0f                    
                                                      target:self                                                      
                                                    selector:@selector(updateGUI)
                                                    userInfo:nil                                 
                                                     repeats:NO];
}


- (void)willRotateToInterfaceOrientation:(UIInterfaceOrientation)orientation duration:(NSTimeInterval)duration {
}


// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations.
	//return (interfaceOrientation == UIInterfaceOrientationLandscapeRight);   
	return (interfaceOrientation == UIInterfaceOrientationPortrait);
}

- (void) hangup:(id)sender
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	
	if (sender) // hang up initialized by us
	{
        if ([callers count] == 2) 
            [[VPEngine sharedInstance] stopConference];

		for (Caller *caller in callers)
		{
            VPCALL call = caller.call;
            caller.call = 0;
            NSLog(@"Hangup for %d call", call);
			[[VPEngine sharedInstance] hangup:call];
		}
	}
	
	[self.view removeFromSuperview];

    if (updateGUITimer)
    {
		[updateGUITimer invalidate];
        updateGUITimer = nil;
    }
    
/*    if (delegate)
        [delegate callFinished:self];
    
*/

	[self release];
}

- (void) conferenceDidStart
{
    [[UIApplication sharedApplication] setStatusBarHidden:YES];

    
    [callerView setFrame:CGRectMake(0.0f, 0.0f, 320.0f, 480.0f)];
    callerView.image = nil;
    
    // "remove" optionscall view
    [optionsCall setFrame:CGRectMake(0.0, 0.0, -1000.0, -1000.0)];
    
    
    // transform topView
    topView.transform = CGAffineTransformRotate(topView.transform, 3*M_PI/2);
    [topView setFrame:CGRectMake(0.0, 0.0, 60.0f, 480.0f)];
    UIImageView *imgView = [[topView subviews] objectAtIndex:0];
    [imgView setImage:[UIImage imageNamed:@"blackbg.png"]];
    [imgView setFrame:CGRectMake(0.0, 0.0f, 480.0f, 60.0f)];
    
    // trasform bottom view
    bottomView.transform = CGAffineTransformRotate(bottomView.transform, 3*M_PI/2);
    [bottomView setFrame:CGRectMake(320-44.0f, 0.0f, 44.0f, 480.0f)];
    [[[bottomView subviews] objectAtIndex:0] setFrame:CGRectMake(0.0, 0.0f, 480.0f, 44.0f)];
    [bottomView setHidden:YES];
    
    // transform camera preview
    cameraPreview.transform = CGAffineTransformRotate(cameraPreview.transform, M_PI/2);
    [cameraPreview setCenter:CGPointMake(480/2, 30)];// Frame:CGRectMake(44.f, 60.0f, 72.0f, 58.0f)];

    
    // transform callers view
    // 1st caller (left)
    Caller *caller = [callers objectAtIndex:0];
    UIView *v = [[[UIView alloc] initWithFrame:CGRectMake(60.0, 240.0f, 260.0f, 240.0f)] autorelease];
//    [v setImage:[UIImage imageNamed:@"calling_picture_background.png"]];
//    [v setBackgroundColor:[UIColor grayColor]];
    [v setClipsToBounds:YES];
    [callerView addSubview:v];
    
    float width = (260.0f * caller.videoView.bounds.size.width) / caller.videoView.bounds.size.height;
    NSLog(@"width: %f", width);
    caller.videoView.transform = CGAffineTransformScale(caller.videoView.transform, 
													   260.0f/caller.videoView.frame.size.width, 
													   width/caller.videoView.frame.size.height);
	[caller.videoView setFrame:CGRectMake(0.0f, 
                                          (240.0f - caller.videoView.frame.size.height)/2, 
                                          260/*caller.videoView.frame.size.width*/, caller.videoView.frame.size.height)];
    
    [caller.timeLabel setFrame:CGRectMake(
                                          caller.timeLabel.frame.origin.x,
                                          caller.timeLabel.frame.origin.y-10,
                                          caller.timeLabel.frame.size.width, 
                                          caller.timeLabel.frame.size.height)];
    [caller.callerNameLabel setFrame:CGRectMake(caller.callerNameLabel.frame.origin.x, 
                                           caller.callerNameLabel.frame.origin.y-10, 
                                           caller.callerNameLabel.frame.size.width, 
                                           caller.callerNameLabel.frame.size.height)];
      
    
    [v addSubview:caller.videoView];
    
    NSLog(@"caller.videoView. bounds(%f, %f, %fx%f)", 
          caller.videoView.bounds.origin.x, caller.videoView.bounds.origin.y,
          caller.videoView.bounds.size.width, caller.videoView.bounds.size.height);
    
    NSLog(@"caller.videoView. frame(%f, %f, %fx%f)", 
          caller.videoView.frame.origin.x, caller.videoView.frame.origin.y,
          caller.videoView.frame.size.width, caller.videoView.frame.size.height);
	
    NSLog(@"2nd caller");
    // 2nd caller (right)
    Caller *caller2 = [callers objectAtIndex:1];
    v = [[[UIView alloc] initWithFrame:CGRectMake(60.0, 0.0f, 260.0f, 240.0f)] autorelease];
//    [v setImage:[UIImage imageNamed:@"calling_picture_background.png"]];
//    [v setBackgroundColor:[UIColor grayColor]];
    [v setClipsToBounds:YES];
    [callerView addSubview:v];
    NSLog(@"1/ 2nd caller.videoView. bounds(%f, %f, %fx%f)", 
          caller2.videoView.bounds.origin.x, caller2.videoView.bounds.origin.y,
          caller2.videoView.bounds.size.width, caller2.videoView.bounds.size.height);
    
    NSLog(@"2/ 2nd caller.videoView. frame(%f, %f, %fx%f)", 
          caller2.videoView.frame.origin.x, caller2.videoView.frame.origin.y,
          caller2.videoView.frame.size.width, caller2.videoView.frame.size.height);
    
    width = (260.0f * caller2.videoView.bounds.size.width) / caller2.videoView.bounds.size.height;
    NSLog(@"width: %f", width);
    caller2.videoView.transform = CGAffineTransformRotate(caller2.videoView.transform, M_PI/2);
    caller2.videoView.transform = CGAffineTransformScale(caller2.videoView.transform, 
                                                        260.0f/caller2.videoView.frame.size.width, 
                                                        width/caller2.videoView.frame.size.height);
    [caller2.videoView setFrame:CGRectMake(0.0f, 
                                          (240.0f - caller2.videoView.frame.size.height)/2, 
                                          260/*caller.videoView.frame.size.width*/, caller2.videoView.frame.size.height)];
    
     
     
     [caller2.timeLabel setFrame:CGRectMake(caller.timeLabel.frame.origin.x + 240, 
                                            caller.timeLabel.frame.origin.y, 
                                            caller.timeLabel.frame.size.width, 
                                            caller.timeLabel.frame.size.height)];
    [imgView addSubview:caller2.timeLabel]; 
     [caller2.callerNameLabel setFrame:CGRectMake(caller.callerNameLabel.frame.origin.x + 240, 
                                                  caller.callerNameLabel.frame.origin.y, 
                                                  caller.callerNameLabel.frame.size.width, 
                                                  caller.callerNameLabel.frame.size.height)]; 
    

    [imgView addSubview:caller2.callerNameLabel]; 

    [v addSubview:caller2.videoView];
     
     
    NSLog(@"2nd caller.videoView. bounds(%f, %f, %fx%f)", 
          caller2.videoView.bounds.origin.x, caller2.videoView.bounds.origin.y,
          caller2.videoView.bounds.size.width, caller2.videoView.bounds.size.height);
    
    NSLog(@"2nd caller.videoView. frame(%f, %f, %fx%f)", 
          caller2.videoView.frame.origin.x, caller2.videoView.frame.origin.y,
          caller2.videoView.frame.size.width, caller2.videoView.frame.size.height);
    
    
    
    UIView *separator = [[[UIView alloc] initWithFrame:CGRectMake(60.0, 239.0f, 260.0f, 3.0f)] autorelease];
    [separator  setBackgroundColor:[UIColor blackColor]];
    [callerView addSubview:separator ];
    
    
    
    // hide/show endcall button 
    UIButton* screenButton = [[[UIButton alloc] initWithFrame:CGRectMake(60.0, 0.0, 260.0, 480.0f)] autorelease];
    screenButton.backgroundColor = [UIColor clearColor];
	[screenButton addTarget:self action:@selector(onScreenClick:) forControlEvents:UIControlEventTouchDown];
    [callerView addSubview:screenButton];
    
    
}


#pragma mark -
#pragma mark ContactSelectorViewControllerDelegate
- (void) contactSelector:(id)sender finishedWithContact:(VPContact*)selectedContact
{
    if (selectedContact)
    {
        VPCALL call = [[VPEngine sharedInstance] makeCall:selectedContact.userName bearer:BC_AUDIOVIDEO];
        NSLog(@"make call %d to user: %@", call, selectedContact.userName);

        Caller *caller = [[[Caller alloc] init] autorelease];
        caller.call = call;
        
        caller.callerNameLabel = [[UILabel alloc] initWithFrame:CGRectMake(0.0f, 0.0f, 0.0, 0.0)];
        [caller.callerNameLabel setTextAlignment:UITextAlignmentCenter];
        [caller.callerNameLabel setOpaque:NO];
        [caller.callerNameLabel setBackgroundColor:[UIColor colorWithRed:0.0 green:0.0f blue:0.0f alpha:0.0f ]];
        [caller.callerNameLabel setTextColor:[UIColor whiteColor]];
        caller.callerNameLabel.font = [UIFont systemFontOfSize:24.0f];
        VPContact *c = [[VPContactList sharedInstance] contactByUsername:[[VPEngine sharedInstance] getRemoteCallName:caller.call]];
        NSString *name = [NSString stringWithString:[[VPEngine sharedInstance] getRemoteCallName:caller.call]];
        if (c)
        {
            if ([c.firstName length] || [c.lastName length])
                name = [NSString stringWithFormat:@"%@ %@", c.firstName, c.lastName];
            else 
                name = [NSString stringWithString:c.userName];
        }
                          
        caller.callerNameLabel.text = name;
        
        caller.timeLabel = [[UILabel alloc] initWithFrame:CGRectMake(0.0f, 0.0f, 0.0, 0.0)];
        caller.timeLabel.font = [UIFont systemFontOfSize:20.0f];
        [caller.timeLabel setTextAlignment:UITextAlignmentCenter];
        [caller.timeLabel setOpaque:NO];
        [caller.timeLabel setBackgroundColor:[UIColor colorWithRed:0.0 green:0.0f blue:0.0f alpha:0.0f ]];
        [caller.timeLabel setTextColor:[UIColor whiteColor]];   
        caller.timeLabel.text = @"00:00";
        
        [callers addObject:caller];	
    }
    
    //[sender release];
}

#pragma mark -
#pragma mark Notifications

- (void) vpVideoDidStartNotification:(NSNotification*)notification
{		 	
	UIImageView *remoteVideoView = [[notification userInfo] objectForKey:@"IMAGEVIEW"];
	VPCALL call = (VPCALL)[[[notification userInfo] objectForKey:@"VPCALL"] unsignedIntValue];
	
    if (numberOfActiveCalls > 1)
    {
        NSLog(@"numberOfActiveCalls > 1");
        return;
    }
    
    // look for caller
    Caller *caller = nil;
    for (caller in callers)
    {
        if (caller.call == call) break;
    }
    
    if (!caller)
    {
        NSLog(@"No such call %d. Creating new one", call);
        caller = [[[Caller alloc] init] autorelease];
        caller.call = call;
        caller.callTime = time(0);
        
        caller.callerNameLabel = [[UILabel alloc] initWithFrame:CGRectMake(0.0f, 0.0f, 0.0, 0.0)];
        [caller.callerNameLabel setTextAlignment:UITextAlignmentCenter];
        [caller.callerNameLabel setOpaque:NO];
        [caller.callerNameLabel setBackgroundColor:[UIColor colorWithRed:0.0 green:0.0f blue:0.0f alpha:0.0f ]];
        [caller.callerNameLabel setTextColor:[UIColor whiteColor]];
        caller.callerNameLabel.font = [UIFont systemFontOfSize:24.0f];
        VPContact *c = [[VPContactList sharedInstance] contactByUsername:[[VPEngine sharedInstance] getRemoteCallName:caller.call]];
        NSString *name = [NSString stringWithString:[[VPEngine sharedInstance] getRemoteCallName:caller.call]];
        if (c)
        {
            if ([c.firstName length] || [c.lastName length])
                name = [NSString stringWithFormat:@"%@ %@", c.firstName, c.lastName];
            else 
                name = [NSString stringWithString:c.userName];
        }
        caller.callerNameLabel.text = name;
        
        caller.timeLabel = [[UILabel alloc] initWithFrame:CGRectMake(0.0f, 0.0f, 0.0, 0.0)];
        caller.timeLabel.font = [UIFont systemFontOfSize:20.0f];
        [caller.timeLabel setTextAlignment:UITextAlignmentCenter];
        [caller.timeLabel setOpaque:NO];
        [caller.timeLabel setBackgroundColor:[UIColor colorWithRed:0.0 green:0.0f blue:0.0f alpha:0.0f ]];
        [caller.timeLabel setTextColor:[UIColor whiteColor]];   
        caller.timeLabel.text = @"00:00";
        
        [callers addObject:caller];	
    }
    
    caller.videoView = remoteVideoView;
    
    NSLog(@"vpVideoDidStartNotification: for call: %d", call);

	if ([callers count] == 2) // it's conference.
	{
        NSLog(@"Preparing for conference");
		[self conferenceDidStart];
		return;
	}
	
	[callerView addSubview:remoteVideoView];
    
	// Rotate and scale remote video view
	remoteVideoView.transform = CGAffineTransformRotate(remoteVideoView.transform, M_PI/2);
	remoteVideoView.transform = CGAffineTransformScale(remoteVideoView.transform, 
													   436.0f/remoteVideoView.bounds.size.width, 
													   320.0f/remoteVideoView.bounds.size.height);
	[remoteVideoView setFrame:CGRectMake(0.0f, 0.0f, 320.0f, 436.0f)];
    NSLog(@"remoteVideoView. bounds(%f, %f, %fx%f)", 
          remoteVideoView.bounds.origin.x, remoteVideoView.bounds.origin.y,
          remoteVideoView.bounds.size.width, remoteVideoView.bounds.size.height);
    
    NSLog(@"remoteVideoView. frame(%f, %f, %fx%f)", 
          remoteVideoView.frame.origin.x, remoteVideoView.frame.origin.y,
          remoteVideoView.frame.size.width, remoteVideoView.frame.size.height);
	
    if (!cameraPreview)
    {
        cameraPreview = [[[UIView alloc] initWithFrame:CGRectMake(320-58-4, 4.0f, 58.0f, 72.0f)] autorelease]; 
        [topView addSubview:cameraPreview];
        
#if !TARGET_IPHONE_SIMULATOR
                
       [Camcorder sharedInstance].previewView.transform = CGAffineTransformScale(
                                        [Camcorder sharedInstance].previewView.transform,  
                                        54.0f/[Camcorder sharedInstance].previewView.frame.size.width, 
                                        68.0f/[Camcorder sharedInstance].previewView.frame.size.height);
  
        CALayer *layer = [[Camcorder sharedInstance].previewView.layer.sublayers objectAtIndex:0];
        layer.cornerRadius = 10;
        [layer setBorderColor: [[UIColor whiteColor] CGColor]];
        [layer setBorderWidth: 2.0];
        
        layer = [Camcorder sharedInstance].previewView.layer;
                 layer.cornerRadius = 10;
                 [layer setBorderColor: [[UIColor whiteColor] CGColor]];
                 [layer setBorderWidth: 2.0];
                 
        
       [[Camcorder sharedInstance].previewView setFrame:CGRectMake(2.0f, 2.0f, 54.0f, 68.0f)];


		[cameraPreview addSubview:[Camcorder sharedInstance].previewView];
       
        cameraPreview.layer.cornerRadius = 8;
        [cameraPreview.layer setBorderColor: [[UIColor whiteColor] CGColor]];
        [cameraPreview.layer setBorderWidth: 2.0f];
		NSLog(@"previewView bounds(%f, %f, %fx%f)", 
			  [Camcorder sharedInstance].previewView.bounds.origin.x, [Camcorder sharedInstance].previewView.bounds.origin.y,
			  [Camcorder sharedInstance].previewView.bounds.size.width, [Camcorder sharedInstance].previewView.bounds.size.height);
		
		NSLog(@"previewView  frame(%f, %f, %fx%f)", 
			  [Camcorder sharedInstance].previewView.frame.origin.x, [Camcorder sharedInstance].previewView.frame.origin.y,
			  [Camcorder sharedInstance].previewView.frame.size.width, [Camcorder sharedInstance].previewView.frame.size.height);
#else
        UIImageView *nocamera = [[[UIImageView alloc] initWithFrame:CGRectMake(0.0f, 0.0f, 58.0f, 72.0f)] autorelease];
        nocamera.image = [UIImage imageNamed:@"novideo_icon.png"];
        [nocamera setAlpha:0.6];
        [cameraPreview addSubview:nocamera];
#endif
	}
	
    
    /*
    // temporary:
    [NSTimer scheduledTimerWithTimeInterval: 5.0f                    
                                     target:self                                                      
                                   selector:@selector(conferenceDidStart)
                                   userInfo:nil                                 
                                    repeats:NO];

     */
}


- (void) vpVideoDidFinishNotification:(NSNotification*)notification
{		 	
//	NSLog(@"vpVideoDidFinishNotification:");
}


- (void) vpEngineNotification:(NSNotification*)notification
{		 	
	unsigned msg	= 	[[[notification userInfo] objectForKey:@"MSG"] unsignedIntValue];
	unsigned param1 = 	[[[notification userInfo] objectForKey:@"PARAM1"] unsignedIntValue];
	//unsigned param2 = 	[[[notification userInfo] objectForKey:@"PARAM2"] unsignedIntValue];

    VPCALL call = (VPCALL)param1;
    
	switch(msg)
	{
        case VPMSG_CALLACCEPTED:
        case VPMSG_CALLACCEPTEDBYUSER:
        case VPMSG_CALLESTABLISHED:
        {            
            // check if call belongs to us
            Caller *caller = [callers objectAtIndex:0];
            if (caller.call == call)
            {
                // That's call was accepted by remote user
                caller.callTime = time(0);
                [self updateGUI];
                return;
            }
            
            caller = [callers objectAtIndex:1];
            if (caller && caller.call == call)
            {
                caller.callTime = time(0);
                NSLog(@"Call %d accepted by remote user. Ask to conference", call);
                
                VPCALL *vpcalls = (VPCALL*)malloc(2);
                vpcalls[0] = ((Caller*)[callers objectAtIndex:0]).call;
                vpcalls[1] = ((Caller*)[callers objectAtIndex:1]).call;
                [[VPEngine sharedInstance] conferenceCalls:vpcalls :2 :NO];
//                [[VPEngine sharedInstance] addToConference:call];
             
                free(vpcalls);

            }

            break;
        }
        case VPMSG_CALLREFUSED:
        case VPMSG_CALLSETUPFAILED:
        case VPMSG_CALLDISCONNECTED:
        case VPMSG_CONNECTTIMEOUT:
        case VPMSG_CALLENDED: 
        {
            // check if call belongs to us
            Caller *caller = nil;
            for (caller in callers)
                if (caller.call == call)
                    break;
            
            if (!caller) return;
            NSLog(@"CallviewController. call ended (%d)", call);
            switch(msg)
            {
                case VPMSG_CALLREFUSED:
                    caller.timeLabel.text = @"Call refused";
                    break;
                case VPMSG_CALLSETUPFAILED:
                    caller.timeLabel.text = @"Call setup failed";
                    break;
                case VPMSG_CALLDISCONNECTED:
                    caller.timeLabel.text = @"Call disconnected";
                    break;
                case VPMSG_CONNECTTIMEOUT:
                    caller.timeLabel.text = @"Not responding";
                    break;
                case VPMSG_CALLENDED: 
                    caller.timeLabel.text = @"Call Ended";
                    break;
            }
            
            [[NSNotificationCenter defaultCenter] removeObserver:self];
            if (updateGUITimer)
            {
                [updateGUITimer invalidate];
                updateGUITimer = 0;
            }
            updateGUITimer = [NSTimer scheduledTimerWithTimeInterval: numberOfActiveCalls>1?0.0f:2.0f
                                                            target:self                                                      
                                                            selector:@selector(hangup:)
                                                            userInfo:nil                                 
                                                            repeats:NO];            
        }
        break;
	
	}
}



#pragma mark -
#pragma mark Memory management

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
    	
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	
    [contact release];
	[callers release];
	
    if (!--numberOfActiveCalls)
    {
        [[UIApplication sharedApplication] setStatusBarHidden:NO];
        [[UIApplication sharedApplication] setStatusBarStyle:UIStatusBarStyleDefault];
        [UIApplication sharedApplication].idleTimerDisabled = NO;
    }

    [super dealloc];

}


@end

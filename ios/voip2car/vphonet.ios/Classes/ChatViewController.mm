//
//  ChatViewController.m
//  Vphonet
//
//  Created by uncle on 05.06.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import "VphonetAppDelegate.h"
#import "VPProfile.h"
#import "ChatViewController.h"
#import "CellChatMessage.h"
#import "CellChatAttachment.h"


@implementation ChatViewController

//@synthesize chatEntryView;
@synthesize contact;
@synthesize messages;
@synthesize chatEntry;
@synthesize tableView;

/*
- (id)initWithStyle:(UITableViewStyle)style
{
    self = [super initWithStyle:style];
    if (self) {
        // Custom initialization
    }
    return self;
}
*/


 - (id) init
 {
 self = [super init];
 
	 messages = [[ChatMessages alloc] init];
 //messages = [[NSMutableArray alloc] init];
 
 return self;
 }



- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
		messages = [[ChatMessages alloc] init];
//		self.tableView.delegate = self;
        // Custom initialization.
    }
    return self;
}


- (void)dealloc
{
	[messages dealloc];
    [super dealloc];
}

/*
So you can call this function to insert the new row

- (void)insertRowsAtIndexPaths:(NSArray *)indexPaths withRowAnimation:(UITableViewRowAnimation)animation
like this

[tableView insertRowsAtIndexPaths:[NSIndexPath indexPathForRow:lastRow inSection:0] withRowAnimation:UITableViewRowAnimationNone];
lastRow++;
and then animate the table view to scroll to last row by calling this function.

- (void)scrollToRowAtIndexPath:(NSIndexPath *)indexPath atScrollPosition:(UITableViewScrollPosition)scrollPosition animated:(BOOL)animated
like this

[tableView scrollToRowAtIndexPath:[NSIndexPath indexPathForRow:lastRow inSection:0] atScrollPosition:UITableViewScrollPositionButtom animated:YES];
*/
 
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
	
	self.parentViewController.view.backgroundColor = [UIColor colorWithPatternImage:[UIImage imageNamed:@"background.png"]];
	
	self.view.backgroundColor = [UIColor clearColor];
//	[self.view insertSubview:self.tableView  aboveSubview:chatEntry];
//    [self chatEntryDidClick:nil];

//	[self.tableView setDelegate:self];
	self.tableView.backgroundColor = [UIColor clearColor];
	[self.tableView setSeparatorColor:[UIColor clearColor]];
	
	[[NSNotificationCenter defaultCenter] addObserver:self 
                                             selector:@selector(onSmsNotification:) 
                                                 name:OnSmsNotification
                                               object:nil];	

	[self setTitle:[NSString stringWithFormat:@"%@ %@", contact.firstName, contact.lastName]];
	
//	[self.view bringSubviewToFront:self.tableView];
	
//	[self.parentViewController.view addSubview:]
//	[self.view insertSubview:chatEntryView belowSubview:self.tableView];


    // Uncomment the following line to preserve selection between presentations.
    // self.clearsSelectionOnViewWillAppear = NO;
 
    // Uncomment the following line to display an Edit button in the navigation bar for this view controller.
    // self.navigationItem.rightBarButtonItem = self.editButtonItem;
	[messages load:[contact number]];
}

- (void)viewDidUnload
{
    [super viewDidUnload];
	
	[[NSNotificationCenter defaultCenter] removeObserver:self];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}

- (void)viewWillAppear:(BOOL)animated
{
	[ChatMessages activityReset:contact.number];
	
	VphonetAppDelegate *appDelegate = (VphonetAppDelegate *)[[UIApplication sharedApplication] delegate];
	[appDelegate activityUpdate];
	
	[tableView reloadData];
	
	if ([messages.list count] > 0) {
		[self.tableView scrollToRowAtIndexPath:[NSIndexPath indexPathForRow:([messages.list count]-1) inSection:0] atScrollPosition:UITableViewScrollPositionTop animated:YES];
	}
	

    // register for keyboard notifications
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyboardWillShow:) 
												 name:UIKeyboardWillShowNotification object:self.view.window]; 
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyboardWillHide:) 
												 name:UIKeyboardWillHideNotification object:self.view.window];
	
	[[NSNotificationCenter defaultCenter] addObserver:self 
											 selector:@selector(keyboardDidShow:) 
												 name:UIKeyboardDidShowNotification 
											   object:nil];
	
	
	[[NSNotificationCenter defaultCenter] postNotificationName:UIKeyboardDidShowNotification object:nil];

}

- (void)viewWillDisappear:(BOOL)animated
{
	// unregister for keyboard notifications while not visible.
    [[NSNotificationCenter defaultCenter] removeObserver:self name:UIKeyboardWillShowNotification object:nil]; 
	// unregister for keyboard notifications while not visible.
    [[NSNotificationCenter defaultCenter] removeObserver:self name:UIKeyboardWillHideNotification object:nil]; 

	[messages save:[contact number]];
	
}


/*
- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
}
*/

- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
}

/*
- (void)viewWillDisappear:(BOOL)animated
{
    [super viewWillDisappear:animated];
}
*/
- (void)viewDidDisappear:(BOOL)animated
{
    [super viewDidDisappear:animated];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}

/*
- (void)loadView
{
    UIView *view = [[[UIView alloc] init] autorelease];
    UITableView *tableView = [[[UITableView alloc] init] autorelease];
    tableView.dataSource = self;
    tableView.delegate = self;
    [view addSubview:tableView];
    self.view = view;
}
*/ 
#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    // Return the number of sections.
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return [messages.list count];
}


- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
	ChatEvent *event = [messages.list objectAtIndex:[indexPath row]];

	UIFont *font = [UIFont fontWithName:@"Helvetica-Bold" size:15.0f];
	
	CGSize maxSize = CGSizeMake(300, CGFLOAT_MAX);
	CGSize size = [event.message sizeWithFont:font constrainedToSize:maxSize lineBreakMode:UILineBreakModeWordWrap];
	
	if ([event.attachment length] == 0) {
		return 50.0 + size.height - [font lineHeight];
	}
	
	return 76.0;
	
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
	static NSString *cellChatMessageIdentifier = @"cellChatMessageIdentifier";
	static NSString *cellChatAttachmentIdentifier = @"cellChatAttachmentIdentifier";

	ChatEvent *event = [messages.list objectAtIndex:[indexPath row]];
	CellChatMessage *cell;
	
	if ([event.attachment length] != 0) {
		cell = (CellChatMessage*)[[self tableView] dequeueReusableCellWithIdentifier:cellChatAttachmentIdentifier];
		NSArray *topLevelObjects = [[NSBundle mainBundle] loadNibNamed:@"CellChatAttachment" owner:nil options:nil];
		
		for(id currentObject in topLevelObjects)
		{
			if([currentObject isKindOfClass:[CellChatAttachment class]])
			{
				cell = (CellChatMessage *)currentObject;
				break;
			}
		}
	} else {
		cell = (CellChatMessage*)[[self tableView] dequeueReusableCellWithIdentifier:cellChatMessageIdentifier];
		if (cell == nil){
			NSArray *topLevelObjects = [[NSBundle mainBundle] loadNibNamed:@"CellChatMessage" owner:nil options:nil];
			
			for(id currentObject in topLevelObjects)
			{
				if([currentObject isKindOfClass:[CellChatMessage class]])
				{
					cell = (CellChatMessage *)currentObject;
					break;
				}
			}
		}
	}
	
	cell.selectionStyle = UITableViewCellSelectionStyleNone;

	[[cell.background layer] setCornerRadius: 8.0f];
	[[cell.background layer] setMasksToBounds:YES];
	[[cell.background layer] setBorderWidth:2.0]; 
	
	NSString *sender = @"";
	if ([event.from isEqualToString:[[VPProfile singleton] number]]) {
		sender = [NSString stringWithFormat:@"%@ %@", [VPProfile singleton].firstName, [VPProfile singleton].lastName];
//		sender = [NSString stringWithString:event.to];
		[[cell.background layer] setBorderColor:[[UIColor colorWithRed:207.0/255.0 green:207.0/255.0 blue:207.0/255.0 alpha:1.0] CGColor]];
		[[cell.background layer] setBackgroundColor:[[UIColor whiteColor] CGColor]];
		[cell.imageLeftBeak setHidden:NO];
		[cell.imageRightBeak setHidden:YES];
	} else {
//		sender = [NSString stringWithString:event.from];
		sender = [NSString stringWithFormat:@"%@ %@", contact.firstName, contact.lastName];
		[[cell.background layer] setBorderColor:[[UIColor colorWithRed:235.0/255.0 green:221.0/255.0 blue:83.0/255.0 alpha:1.0] CGColor]];
		[[cell.background layer] setBackgroundColor:[[UIColor colorWithRed:254.0/255.0 green:250.0/255.0 blue:216.0/255.0 alpha:1.0] CGColor]];
		[cell.imageLeftBeak setHidden:YES];
		[cell.imageRightBeak setHidden:NO];
	}
	
	[cell.labelName setText:sender];	
	[cell.labelDate setText:event.date];	
	[cell.labelMessage setText:event.message];
	[cell.labelStatus setText:event.status];
	
	if([cell isKindOfClass:[CellChatAttachment class]])
	{
		CellChatAttachment *cellWithImage = (CellChatAttachment*)cell; 
		NSFileManager *fm = [NSFileManager defaultManager]; 
		
		if ([event.status isEqualToString:@"S"]) {
			[cellWithImage.buttonOpen setHidden:YES];
			[cellWithImage.buttonForward setHidden:YES];
			[cellWithImage.progressBar setHidden:NO];
			[cellWithImage.progressBar setProgress:event.progress];
		} else {
			[cellWithImage.buttonOpen setHidden:NO];
			[cellWithImage.buttonForward setHidden:NO];
			[cellWithImage.progressBar setHidden:YES];
		}
		
		if ([fm fileExistsAtPath:[event attachmentFileForContact:contact.number]]){
			cellWithImage.navigationController = self.navigationController;
			cellWithImage.fileName = [event attachmentFileForContact:contact.number];
			cellWithImage.imagePhoto.image = [UIImage imageWithContentsOfFile:[event attachmentFileForContact:contact.number]];
		}
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
    /*
     <#DetailViewController#> *detailViewController = [[<#DetailViewController#> alloc] initWithNibName:@"<#Nib name#>" bundle:nil];
     // ...
     // Pass the selected object to the new view controller.
     [self.navigationController pushViewController:detailViewController animated:YES];
     [detailViewController release];
     */
}


-(void) keyboardShow:(BOOL)show notification:(NSNotification *)notification
{
	CGRect keyboardEndFrame;
	[[notification.userInfo valueForKey:UIKeyboardFrameEndUserInfoKey] getValue:&keyboardEndFrame];
	CGFloat keyboardHeight;
	if ([[UIDevice currentDevice] orientation] == UIDeviceOrientationPortrait || [[UIDevice currentDevice] orientation] == UIDeviceOrientationPortraitUpsideDown) {
		keyboardHeight = keyboardEndFrame.size.height;
	}
	else {
		keyboardHeight = keyboardEndFrame.size.width;
	}
	
	CGRect frame = self.view.frame;
	
	if (show) {
		frame.size.height -= keyboardHeight - 49; // tab bar height
	} else {
        frame.size.height += keyboardHeight - 49; // tab bar height
	}
	
	
	[UIView beginAnimations:nil context:NULL];
	[UIView setAnimationBeginsFromCurrentState:YES];
	[UIView setAnimationDuration:0.3f];
	self.view.frame = frame;
	[UIView commitAnimations];
	
	if ([messages.list count] > 0) {
		[self.tableView scrollToRowAtIndexPath:[NSIndexPath indexPathForRow:([messages.list count]-1) inSection:0] atScrollPosition:UITableViewScrollPositionTop animated:YES];
	}

}


-(void) keyboardWillShow:(NSNotification *)notification
{
	[self keyboardShow:YES notification:notification];
}

- (void)keyboardWillHide:(NSNotification *)notification
{
	[self keyboardShow:NO notification:notification];
}


- (BOOL)textFieldShouldReturn:(UITextField *)textField {
	
	//if ([chatEntry.text length] == 0) {
	//	return false;
	//}
	
	[self sendMessage:self];
	
	[chatEntry resignFirstResponder];
	
	if ([messages.list count] > 0) {
		[self.tableView scrollToRowAtIndexPath:[NSIndexPath indexPathForRow:([messages.list count]-1) inSection:0] atScrollPosition:UITableViewScrollPositionTop animated:YES];
	}

	
	return true; 
}

- (IBAction) sendMessage:(id)sender
{
	
    if ([chatEntry.text length] == 0)
    {
		return;
    }
	
	[messages add:[[VPProfile singleton] number] to:contact.number attachment:@"" message:chatEntry.text];
	
	[[VPEngine sharedInstance] sendSMS:contact.number text:chatEntry.text];
	
	
	[tableView reloadData];
	chatEntry.text = @"";
	
	if ([messages.list count] > 0) {
		[self.tableView scrollToRowAtIndexPath:[NSIndexPath indexPathForRow:([messages.list count]-1) inSection:0] atScrollPosition:UITableViewScrollPositionTop animated:YES];
	}
	
}

- (void)keyboardDidShow:(NSNotification*)notification {
	UIWindow* tempWindow;
	UIView* keyboard;
	
	for(int c = 0; c < [[[UIApplication sharedApplication] windows] count]; c++)
	{
		tempWindow = [[[UIApplication sharedApplication] windows] objectAtIndex:c];
		for(int i = 0; i < [tempWindow.subviews count]; i++)
		{
			keyboard = [tempWindow.subviews objectAtIndex:i];
			if (([[keyboard description] hasPrefix:@"<UIKeyboard"] == YES) || ([[keyboard description] hasPrefix:@"<UIPeripheralHostView"] == YES ))
			{
				//Keyboard is now a UIView reference to the UIKeyboard we want. From here we can add a subview
				//to th keyboard like a new button
				UIButton* done = [UIButton buttonWithType:UIButtonTypeCustom];
				done.frame = CGRectMake(241, 173, 77, 42);
				[done setTitle:NSLocalizedString(@"Send", @"") forState:UIControlStateNormal];
				[done setTitleColor:[UIColor whiteColor] forState:UIControlStateNormal];
				done.titleLabel.font = [UIFont boldSystemFontOfSize:16.0f];
				done.titleLabel.shadowOffset = CGSizeMake(1.0,1.0);
				
				[done setBackgroundImage:[UIImage imageNamed:@"login_up.png"] forState:UIControlStateNormal];
				[done setBackgroundImage:[UIImage imageNamed:@"login_down.png"] forState:UIControlStateHighlighted];
				
				[done addTarget:self action:@selector(textFieldShouldReturn:) forControlEvents:UIControlEventTouchUpInside];
				[keyboard addSubview:done];
				return;
			}
		}
	}
}



- (void) onSmsNotification:(NSNotification*)notification
{		 	
	unsigned typeSMS = [[[notification userInfo] objectForKey:@"type"] unsignedIntValue];
	
	switch (typeSMS) {
		case SMSTYPE_CONTACTADDED:
		{
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
		{
			[messages add:[[notification userInfo] objectForKey:@"number"] to:[[VPProfile singleton] number] attachment:@"" message:[[notification userInfo] objectForKey:@"message"]];
			[tableView reloadData];
			if ([messages.list count] > 0) {
				[self.tableView scrollToRowAtIndexPath:[NSIndexPath indexPathForRow:([messages.list count]-1) inSection:0] atScrollPosition:UITableViewScrollPositionTop animated:YES];
			}
		}
			break;
		case SMSTYPE_MISSEDCALLS:
			break;
		case SMSTYPE_DELIVERYREPORT:
			break;
		default:	
			break;
	}
}


 

@end

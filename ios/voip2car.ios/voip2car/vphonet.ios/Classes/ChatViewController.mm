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
	[tableView reloadData];
    // register for keyboard notifications
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyboardWillShow:) 
												 name:UIKeyboardWillShowNotification object:self.view.window]; 
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyboardWillHide:) 
												 name:UIKeyboardWillHideNotification object:self.view.window];
	
	if ([messages.list count] > 0) {
		[self.tableView scrollToRowAtIndexPath:[NSIndexPath indexPathForRow:([messages.list count]-1) inSection:0] atScrollPosition:UITableViewScrollPositionTop animated:YES];
	}


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
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
	
	
	ChatEvent *event = [messages.list objectAtIndex:[indexPath row]];
	UIFont *font = [UIFont fontWithName:@"Helvetica-Bold" size:14.0f];
	
	CGSize maxSize = CGSizeMake(245, CGFLOAT_MAX);
	CGSize size = [event.message sizeWithFont:font constrainedToSize:maxSize lineBreakMode:UILineBreakModeWordWrap];

    
	static NSString *chatCellIdentifier = @"chatCellIdentifier";
    
    UITableViewCell *cell = [[self tableView] dequeueReusableCellWithIdentifier:chatCellIdentifier];
    if (cell == nil) {
        cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:chatCellIdentifier] autorelease];
		
		UIImageView *background = [[UIImageView alloc] initWithFrame:CGRectMake(0, 2, cell.frame.size.width, [self tableView:[self tableView] heightForRowAtIndexPath:indexPath]-4)];
		
		NSLog(@"bounds %f",cell.bounds.size.height);
		NSLog(@"frame %f",cell.frame.size.height);
		NSLog(@"height %f", [self tableView:[self tableView] heightForRowAtIndexPath:indexPath]);
		
		//CGRectMake(0,0, cell.frame.size.width,cell.frame.size.height)];
		NSString *sender = @"";
		if ([event.from isEqualToString:[[VPProfile singleton] number]]) {
			[background setImage:[UIImage imageNamed:@"chatfromleft.png"]];
			[background setContentMode:UIViewContentModeScaleToFill];
			sender = [NSString stringWithString:event.to];
		} else {
			[background setImage:[UIImage imageNamed:@"chatfromright.png"]];
			[background setContentMode:UIViewContentModeScaleToFill];
			sender = [NSString stringWithString:event.from];
		}
		
//		[labelName setText:sender];
//		[labelDate setText:event.date];
//		[labelIndicator setText:@""];
		
		
		//	cell.layer.shadowOffset = CGSizeMake(1, 0);
		//	cell.layer.shadowColor = [[UIColor blackColor] CGColor];
		//	cell.layer.shadowRadius = 5;
		//	cell.layer.shadowOpacity = .25;
		
		//UILabel *label = [[UILabel alloc] initWithFrame:CGRectMake(10, 4, 315, textRows * textSize.height + [titleFont lineHeight] + 4)];
		//[label setLineBreakMode:UILineBreakModeWordWrap];
		//[label setText:event.message];
		//[label setNumberOfLines:textRows];
		//[label setBackgroundColor:[UIColor clearColor]];
		
		//	[cell setContentMode:UIViewContentModeScaleToFill];
		[cell addSubview:background];
		
		//[cell addSubview:labelName];
		//[cell addSubview:labelDate];
		//[cell addSubview:labelIndicator];
		//[cell addSubview:label];
		
		
//		[background release];
    }
//	cell.selectionStyle = UITableViewCellSelectionStyleNone;
	
	
	
//    CGSize textSize = [event.message sizeWithFont:font];
//    unsigned textRows = textSize.width / 245 + 2;
    
//    CGRect rect = CGRectMake(5, 0, 315, textRows * textSize.height);

	UIFont *titleFont = [UIFont fontWithName:@"Helvetica-Oblique" size:13.0f];

	UILabel *labelName = [[UILabel alloc] initWithFrame:CGRectMake(10, 4, 150, 	[titleFont lineHeight])];
	UILabel *labelDate = [[UILabel alloc] initWithFrame:CGRectMake(160, 4, 290-160, [titleFont lineHeight])];
	UILabel *labelIndicator = [[UILabel alloc] initWithFrame:CGRectMake(290, 4, 18, [titleFont lineHeight])];
	
	[labelName setFont:titleFont];
	[labelDate setFont:titleFont];
	[labelIndicator setFont:titleFont];

	[labelDate setTextAlignment:UITextAlignmentRight];
	[labelIndicator setTextAlignment:UITextAlignmentRight];
	
	
	[labelName setTextColor:[UIColor grayColor]];
	[labelDate setTextColor:[UIColor grayColor]];
	[labelIndicator setTextColor:[UIColor grayColor]];

	
	[labelName setBackgroundColor:[UIColor clearColor]];
	[labelDate setBackgroundColor:[UIColor clearColor]];
	[labelIndicator setBackgroundColor:[UIColor clearColor]];
	
	

//	UIImageView *background = [[UIImageView alloc] initWithFrame:CGRectMake(5, 0, 315, textRows * textSize.height + [titleFont lineHeight])];
//	UIImageView *background = [[UIImageView alloc] initWithFrame:CGRectMake(0, 0, cell.frame.size.width, cell.frame.size.height)];
	UIImageView *background = [[UIImageView alloc] initWithFrame:CGRectMake(0, 2, cell.frame.size.width, [self tableView:[self tableView] heightForRowAtIndexPath:indexPath]-4)];
	
//	NSLog(@"bounds %f",cell.bounds.size.height);
//	NSLog(@"frame %f",cell.frame.size.height);
//	NSLog(@"height %f", [self tableView:[self tableView] heightForRowAtIndexPath:indexPath]);
	
							   //CGRectMake(0,0, cell.frame.size.width,cell.frame.size.height)];
	NSString *sender = @"";
	if ([event.from isEqualToString:[[VPProfile singleton] number]]) {
		[background setImage:[UIImage imageNamed:@"chatfromleft.png"]];
		[background setContentMode:UIViewContentModeScaleToFill];
		sender = [NSString stringWithString:event.to];
	} else {
		[background setImage:[UIImage imageNamed:@"chatfromright.png"]];
		[background setContentMode:UIViewContentModeScaleToFill];
		sender = [NSString stringWithString:event.from];
	}
	
	[labelName setText:sender];
	[labelDate setText:event.date];
	[labelIndicator setText:@""];
	
	
//	cell.layer.shadowOffset = CGSizeMake(1, 0);
//	cell.layer.shadowColor = [[UIColor blackColor] CGColor];
//	cell.layer.shadowRadius = 5;
//	cell.layer.shadowOpacity = .25;
	
	//UILabel *label = [[UILabel alloc] initWithFrame:CGRectMake(10, 4, 315, textRows * textSize.height + [titleFont lineHeight] + 4)];
	//[label setLineBreakMode:UILineBreakModeWordWrap];
	//[label setText:event.message];
	//[label setNumberOfLines:textRows];
	//[label setBackgroundColor:[UIColor clearColor]];
	
//	[cell setContentMode:UIViewContentModeScaleToFill];
//	[cell addSubview:background];
	
	//[cell addSubview:labelName];
	//[cell addSubview:labelDate];
	//[cell addSubview:labelIndicator];
	//[cell addSubview:label];
	
	
//
//	[background release];
	[labelName release];
	[labelDate release];
	[labelIndicator release];
	
    return cell;
}
*/

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
        frame.size.height += keyboardHeight + 49; // tab bar height
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
	
	if ([chatEntry.text length] == 0) {
		return false;
	}
	
	[messages add:[[VPProfile singleton] number] to:contact.number attachment:@"" message:chatEntry.text];

	[[VPEngine sharedInstance] sendSMS:contact.number text:chatEntry.text];
	[tableView reloadData];
	chatEntry.text = @"";
	
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
		//        [[VPEngine sharedInstance] sendChatMsg:vpcall text:chatEntry.text];
//        [self appendMessage:sender text:chatEntry.text];
    }
	
	
    
//    CGRect rect = chatEntryView.frame;
//    rect.origin.y = 480.0-49-36-44-22;
//    [chatEntryView setFrame:rect];
    
    
//    rect = contentView.frame;
//    rect.size.height = 367.0;
//    [contentView setFrame:rect];
    
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

//
//  ProfileViewController.m
//  IP-Phone
//
//  Created by uncle on 09.02.11.
//  Copyright 2011. All rights reserved.
//

#import "VPProfile.h"
#import "VphonetAppDelegate.h"
#import "StatusViewController.h"
#import "CallForwardViewController.h"
#import "FullProfileViewController.h"
#import "VirtualNumberViewController.h"
#import "SettingsViewController.h"
#import "ProfileViewController.h"



@implementation ProfileViewController

@synthesize status;

#pragma mark -
#pragma mark Constructors and Destructors

- (void)dealloc {
	
//	[profile release];
	
    [super dealloc];
}



#pragma mark -
#pragma mark View lifecycle
- (void)updatePhoto {
	
	self.title = [NSString stringWithFormat:NSLocalizedString(@"%@ %@ (me)", @""), profile.firstName, profile.lastName];
	
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *file = [NSString stringWithFormat:@"%@/photos/profile.jpg",[paths objectAtIndex:0]];
	NSString *last = [NSString stringWithFormat:@"%@/photos/%@.jpg",[paths objectAtIndex:0], profile.photoUpdate];
	NSFileManager *fm = [NSFileManager defaultManager]; 

	[cellProfileHeader.photoView.imageView setImage:nil];
	
	if ([fm fileExistsAtPath:file]){
		[cellProfileHeader.photoView setImage:[UIImage imageWithContentsOfFile:file] forState:UIControlStateNormal];
	} else	if ([fm fileExistsAtPath:last]){
		[cellProfileHeader.photoView setImage:[UIImage imageWithContentsOfFile:last] forState:UIControlStateNormal];
	} else {
		[cellProfileHeader.photoView setImage:[UIImage imageNamed:@"profile_picture.png"] forState:UIControlStateNormal];
	}
	
}



- (void)viewDidLoad {

    [super viewDidLoad];

	profile = [VPProfile singleton];
	
	
	self.tableView = nil;   
    UITableView *tv = [[UITableView alloc] initWithFrame:CGRectZero style:UITableViewStyleGrouped];
	tv.scrollEnabled = NO;
    self.tableView = tv; 
    [tv release];
	
	
	self.title = [NSString stringWithFormat:NSLocalizedString(@"%@ %@ (me)", @""), profile.firstName, profile.lastName];

/*	
    if (tableHeaderView == nil) {
		[[NSBundle mainBundle] loadNibNamed:@"ProfileHeaderView" owner:self options:nil];
        self.tableView.tableHeaderView = tableHeaderView;
        self.tableView.allowsSelectionDuringEditing = YES;
		self.tableView.backgroundColor = [UIColor clearColor];
        self.tableView.tableHeaderView.backgroundColor = [UIColor clearColor];
		
		[self updatePhoto];
    }
	
	[VphonetAppDelegate setButtonYellowBorder:backgroundHeader];
	[VphonetAppDelegate setButtonYellowBorder:backgroundName];
	[VphonetAppDelegate setButtonYellowBorder:backgroundStatus];	

*/	
	self.view.backgroundColor = [UIColor colorWithPatternImage:[UIImage imageNamed:@"background.png"]];

	[self setCustomBackground:@"background.png"];
	[self.tableView setSeparatorColor:[UIColor paleYellowColor]];
	
	
	UIBarButtonItem *settingsButton = [[UIBarButtonItem alloc] initWithImage:[UIImage imageNamed:@"settings.png" ] style:UIBarButtonItemStylePlain 
																	 target:self 
																	 action:@selector(settings:)];          
	self.navigationItem.rightBarButtonItem = settingsButton;
	[settingsButton release];

}





- (void)viewWillAppear:(BOOL)animated {
	
	/*
	self.status  = [[VPEngine sharedInstance] userStatus];
	
//	UITableViewCell *cell = [self.tableView cellForRowAtIndexPath:[NSIndexPath indexPathForRow:0 inSection:0]];
	
	switch (status) {
		case USERSTATUS_ONLINE:
			[cellProfileHeader.imageStatus setImage:[UIImage imageNamed: @"status_green.png"]];
			[cellProfileHeader.labelStatus setText:NSLocalizedString(@"Online", @"")];
			break;
		case USERSTATUS_SHOWOFFLINE:
			[cellProfileHeader.imageStatus setImage:[UIImage imageNamed: @"status_red.png"]];
			[cellProfileHeader.labelStatus setText:NSLocalizedString(@"Offline", @"")];
			break;
		case USERSTATUS_OFFLINE:
			[cellProfileHeader.imageStatus setImage:[UIImage imageNamed: @"status_yellow.png"]];
			[cellProfileHeader.labelStatus setText:NSLocalizedString(@"Invisible", @"")];
			break;
		default:
			[cellProfileHeader.imageStatus setImage:[UIImage imageNamed: @"status_gray.png"]];
			[cellProfileHeader.labelStatus setText:NSLocalizedString(@"Unavailable", @"")];
			////			if (cell) {
//				cell.detailTextLabel.text = NSLocalizedString(@"Unavailable", @"");
//			}
			break;
	}
 
	 */
	[self reloadInputViews];
	
    [super viewWillAppear:animated];
	
	[self.tableView reloadData];
}


- (void)viewDidAppear:(BOOL)animated {
	
    [super viewDidAppear:animated];
	
}

 
/*
- (void)viewWillDisappear:(BOOL)animated {
    [super viewWillDisappear:animated];
}
*/


- (void)viewDidDisappear:(BOOL)animated {
    [super viewDidDisappear:animated];
}

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
    return 3;
}


- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    // Return the number of rows in the section.
	switch (section) {
		case 0: return 1;
			break;
		case 1: return 3;
			break;
		case 2: return 2;
			break;
	}
	
    return 0;
}

// Customize the appearance of table view cells.
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    
    static NSString *CellIdentifier = @"Cell";
	
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
        cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1 /*UITableViewCellStyleDefault*/ reuseIdentifier:CellIdentifier] autorelease];
    }
	UIImage *leftIcon = [UIImage imageNamed:@"star.png"];
	
	//self.labelLastName.text = [VPProfile singleton].lastName;
	//self.labelFirstName.text = [VPProfile singleton].firstName;
	//VPProfile* profile = [VPProfile singleton];

	
	switch(indexPath.section)
	{
		case 0:
			if (indexPath.row == 0) {
				if(cellProfileHeader == nil) {
					NSArray *topLevelObjects = [[NSBundle mainBundle] loadNibNamed:@"CellMyProfileHeader" owner:nil options:nil];
					for(id currentObject in topLevelObjects)
					{
						if([currentObject isKindOfClass:[CellMyProfileHeader class]])
						{
							cellProfileHeader = (CellMyProfileHeader *)currentObject;
							[self updatePhoto];

							[[cellProfileHeader buttonAdd] addTarget:self action:@selector(onChoicePhotoClick:) forControlEvents:UIControlEventTouchDown];
							[[cellProfileHeader photoView] addTarget:self action:@selector(onChoicePhotoClick:) forControlEvents:UIControlEventTouchDown];
							
							[VphonetAppDelegate setButtonYellowBorder:cellProfileHeader.photoView];
							[VphonetAppDelegate setButtonYellowBorder:cellProfileHeader.backgroundHeader];
							[cellProfileHeader.backgroundHeader setHidden:YES];
							[VphonetAppDelegate setButtonYellowBorder:cellProfileHeader.backgroundName];
							[VphonetAppDelegate setButtonYellowBorder:cellProfileHeader.backgroundStatus];
							break;
						}
					}
				}
				
				[cellProfileHeader retain];
				cellProfileHeader.selectionStyle = UITableViewCellSelectionStyleNone;
				
				
				[cellProfileHeader.labelName setText:[NSString stringWithFormat:@"%@ %@", profile.firstName, profile.lastName]];
				
				cellProfileHeader.imageStatus.image = [UIImage imageNamed:[profile statusImageName]];
				
//				[[[cellProfileHeader  imageStatus] image] setImage:[UIImage imageNamed:[profile statusImageName]]];
				[[cellProfileHeader labelStatus] setText:[NSString stringWithString:[profile statusName]]];

//				[cellProfileHeader.labelEMail setText: contact.email];
				
				[cellProfileHeader.photoView setBackgroundImage:[UIImage imageNamed:@"profile_picture.png"] forState:UIControlStateNormal];
				
			}
			return cellProfileHeader;
			break;
		case 1:
		{
			cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
			cell.imageView.image = leftIcon;
			
			if (indexPath.row == 0) {
				cell.textLabel.text = NSLocalizedString(@"Status", @"");
				
				cell.detailTextLabel.text = [NSString stringWithString:[profile statusName]];
				
			}
			else if (indexPath.row == 1)
			{
				cell.textLabel.text = NSLocalizedString(@"Call forward", @"");
				if ([profile.phoneForward length] != 0) {
					cell.detailTextLabel.text = NSLocalizedString(@"Active", @"");
				} else {
					cell.detailTextLabel.text = nil;
				}
			}
			else if (indexPath.row == 2)
			{
				cell.textLabel.text = NSLocalizedString(@"Edit profile", @"");
				cell.detailTextLabel.text = nil;
			}
		}		
			break;
		case 2:
		{
			cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
			cell.imageView.image = leftIcon;
			cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
			if (indexPath.row == 0) {
				cell.textLabel.text = NSLocalizedString(@"Credit", @"");
				cell.detailTextLabel.text = @"0.00 $";
			}
			else if (indexPath.row == 1)
			{
				cell.textLabel.text = NSLocalizedString(@"Virtual number", @"");
				if ([profile.phoneVirtual length] != 0) {
					cell.detailTextLabel.text = NSLocalizedString(@"Active", @"");
				} else {
					cell.detailTextLabel.text = nil;
				}
			}
		}
			break;
	}
    
    
    return cell;
}

/*
-(CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
	return 100;
}
*/ 

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
    // Navigation logic may go here. Create and push another view controller.

	if (indexPath.section == 1) {
		switch (indexPath.row) {
			case 0: {
				StatusViewController *statusViewController = [[StatusViewController alloc] initWithStyle:UITableViewStyleGrouped];
				statusViewController.status = profile.status;
				UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"Back", @"") style: UIBarButtonItemStyleBordered target: nil action: nil];
				[[self navigationItem] setBackBarButtonItem: backButton];
				[backButton release];
				
				[self.navigationController pushViewController:statusViewController animated:YES];
				[statusViewController release];
				break;
			}
			case 1: {
				CallForwardViewController *callForwardViewController = [[CallForwardViewController alloc] initWithStyle:UITableViewStyleGrouped];
				UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"Back", @"") style: UIBarButtonItemStyleBordered target: nil action: nil];
				[[self navigationItem] setBackBarButtonItem: backButton];
				[backButton release];
				[self.navigationController pushViewController:callForwardViewController animated:YES];
				[callForwardViewController release];
				break;
			}
			case 2: {
				FullProfileViewController *fullProfileViewController = [[FullProfileViewController alloc] initWithStyle:UITableViewStyleGrouped];
				UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"Back", @"") style: UIBarButtonItemStyleBordered target: nil action: nil];
				[[self navigationItem] setBackBarButtonItem: backButton];
				[backButton release];
				[self.navigationController pushViewController:fullProfileViewController animated:YES];
				[fullProfileViewController release];
				break;
			}
		}
	}else if (indexPath.section == 2) {
		switch (indexPath.row) {
			case 0: {
				UIActionSheet *actionSheet = [[UIActionSheet alloc] initWithTitle:nil
																		 delegate:self
																cancelButtonTitle:@"Cancel" 
														   destructiveButtonTitle:nil 
																otherButtonTitles:@"Buy Credit", nil];
				actionSheet.actionSheetStyle = UIActionSheetStyleDefault;
				actionSheet.tag = 0;
				VphonetAppDelegate *appDelegate = (VphonetAppDelegate *)[[UIApplication sharedApplication] delegate];

				[actionSheet showInView:[appDelegate window]];	// show from our table view (pops up in the middle of the table)
				[actionSheet release];
			}	
			break;
			case 1: {
				VirtualNumberViewController *virtualNumberViewController = [[VirtualNumberViewController alloc] initWithStyle:UITableViewStyleGrouped];
				UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"Back", @"") style: UIBarButtonItemStyleBordered target: nil action: nil];
				[[self navigationItem] setBackBarButtonItem: backButton];
				[backButton release];
				[self.navigationController pushViewController:virtualNumberViewController animated:YES];
				[virtualNumberViewController release];
				break;
			}	
			break;
				
		}
	}
	
/*    
    <#DetailViewController#> *detailViewController = [[<#DetailViewController#> alloc] initWithNibName:@"<#Nib name#>" bundle:nil];
    // ...
    // Pass the selected object to the new view controller.
    [self.navigationController pushViewController:detailViewController animated:YES];
    [detailViewController release];
*/    
}

- (void)actionSheet:(UIActionSheet *)popup clickedButtonAtIndex:(NSInteger)buttonIndex {

	if ((popup.tag == 0) && (buttonIndex == 0))// Buy credit
	{
			[[UIApplication sharedApplication] openURL:[NSURL URLWithString:@"http://www.vphonet.com/prodsel"]];
	} else if (popup.tag == 1) { // Add Photo
		UIImagePickerController *imagePicker = [[UIImagePickerController alloc] init];
		imagePicker.delegate = self;
		
		if (buttonIndex == 0) {
			// Set source to the Library
			imagePicker.sourceType = UIImagePickerControllerSourceTypePhotoLibrary;
			[self presentModalViewController:imagePicker animated:YES];
		}
		else if ((buttonIndex == 1) && (popup.numberOfButtons == 3))
		{
			// Set source to the Camera
			imagePicker.sourceType =   UIImagePickerControllerSourceTypeCamera;
			[self presentModalViewController:imagePicker animated:YES];
		} // else Cancel
		
		[imagePicker release];
	}
}

- (void) onChoicePhotoClick: (id)sender;
{
	UIActionSheet *actionSheet = nil;
	if ([UIImagePickerController isSourceTypeAvailable:UIImagePickerControllerSourceTypeCamera]) {
		actionSheet = [[UIActionSheet alloc] initWithTitle:NSLocalizedString(@"Change Profile Picture", @"")
															 delegate:self
													cancelButtonTitle:NSLocalizedString(@"Cancel", @"") 
											   destructiveButtonTitle:nil 
													otherButtonTitles:NSLocalizedString(@"From Photo Library", @""),
																	NSLocalizedString(@"Capture From Camera", @""),
																	nil];
	} else {
		actionSheet = [[UIActionSheet alloc] initWithTitle:NSLocalizedString(@"Change Profile Picture",@"")
												  delegate:self
										 cancelButtonTitle:NSLocalizedString(@"Cancel", @"") 
									destructiveButtonTitle:nil 
										 otherButtonTitles:NSLocalizedString(@"From Photo Library", @""),
															nil];
		
	}
	actionSheet.tag = 1;
	actionSheet.actionSheetStyle = UIActionSheetStyleDefault;
	VphonetAppDelegate *appDelegate = (VphonetAppDelegate *)[[UIApplication sharedApplication] delegate];
	
	[actionSheet showInView:[appDelegate window]];	// show from our table view (pops up in the middle of the table)
	[actionSheet release];
	
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

- (IBAction) settings: (id)sender
{
	SettingsViewController *settingsViewController = [[SettingsViewController alloc] initWithStyle:UITableViewStyleGrouped];
	
	UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"Back", @"") style: UIBarButtonItemStyleBordered target: nil action: nil];
	[[self navigationItem] setBackBarButtonItem: backButton];
	[backButton release];
	
	[self.navigationController pushViewController:settingsViewController animated:YES];
	[settingsViewController release];

//	if (cellTextField)
//		[[VPProfile singleton] setStatusMessage:cellTextField.textField.text];
	
	//[self.navigationController popViewControllerAnimated:YES];
	//    [self finishedWithContact:nil];
}

-(UIImage *)resizeImage:(UIImage *)image width:(int)width height:(int)height {
	
	CGImageRef imageRef = [image CGImage];
	CGImageAlphaInfo alphaInfo = CGImageGetAlphaInfo(imageRef);
	CGColorSpaceRef colorSpaceInfo = CGImageGetColorSpace(imageRef);
	
	if (alphaInfo == kCGImageAlphaNone)
		alphaInfo = kCGImageAlphaNoneSkipLast;
	
	CGContextRef bitmap = CGBitmapContextCreate(NULL, width, height, CGImageGetBitsPerComponent(imageRef), CGImageGetBytesPerRow(imageRef), colorSpaceInfo, alphaInfo);
	CGContextDrawImage(bitmap, CGRectMake(0, 0, width, height), imageRef);
	CGImageRef ref = CGBitmapContextCreateImage(bitmap);
	UIImage *result = [UIImage imageWithCGImage:ref];
	
	CGContextRelease(bitmap);
	CGImageRelease(ref);
	
	return result;	
}

- (UIImage *)resizeImage:(UIImage *)image scaledToSize:(CGSize)newSize {
    UIGraphicsBeginImageContext(newSize);
    [image drawInRect:CGRectMake(0, 0, newSize.width, newSize.height)];
    UIImage *newImage = UIGraphicsGetImageFromCurrentImageContext();    
    UIGraphicsEndImageContext();
    return newImage;
}


- (void)imagePickerController:(UIImagePickerController *)picker didFinishPickingImage:(UIImage *)selectedImage editingInfo:(NSDictionary *)editingInfo {
	
	UIImage *photo = [self resizeImage:selectedImage scaledToSize:CGSizeMake(100,100)];
//	UIImage *photo = [self resizeImage:selectedImage width:100 height:100];
	NSData	*imageData = UIImageJPEGRepresentation(photo, 1.0);
	
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *path = [NSString stringWithFormat:@"%@/photos",[paths objectAtIndex:0]];
	NSFileManager *fm = [NSFileManager defaultManager]; 
	NSError *error;
	
	
	
	if (![fm createDirectoryAtPath:path withIntermediateDirectories:YES attributes:nil error:&error]) {
		NSLog(@"Error: Create directory failed");
	}
	
//	if ([fm shouldRemoveItemAtPath:[NSString stringWithFormat:@"%@/profile.jpg",path]]) {
		[fm removeItemAtPath:[NSString stringWithFormat:@"%@/profile.jpg",path] error:&error];
//	}
	
	[imageData writeToFile:[NSString stringWithFormat:@"%@/profile.jpg",path] atomically:NO];
	
	[[VPEngine sharedInstance] updatePhoto:[NSString stringWithFormat:@"%@/profile.jpg",path]];
	
//	[photoView.imageView setImage:[UIImage imageWithData:imageData]];
	[cellProfileHeader.photoView setImage:[UIImage imageWithContentsOfFile:[NSString stringWithFormat:@"%@/profile.jpg",path]] forState:UIControlStateNormal];
//	[photoView setBackgroundImage:[photo copy] forState:UIControlStateNormal];

	
//	[self.photoView.imageView setImage:selectedImage forState:UIControlStateNormal];
	
//	[tableHeaderView]; 

	
	// Delete any existing image.
//	NSManagedObject *oldImage = recipe.image;
//	if (oldImage != nil) {
//		[recipe.managedObjectContext deleteObject:oldImage];
//	}
	
    // Create an image object for the new image.
//	NSManagedObject *image = [NSEntityDescription insertNewObjectForEntityForName:@"Image" inManagedObjectContext:recipe.managedObjectContext];
//	recipe.image = image;
	
	// Set the image for the image managed object.
//	[image setValue:selectedImage forKey:@"image"];
	
	// Create a thumbnail version of the image for the recipe object.
//	CGSize size = selectedImage.size;
//	CGFloat ratio = 0;
//	if (size.width > size.height) {
//		ratio = 44.0 / size.width;
//	} else {
//		ratio = 44.0 / size.height;
//	}
//	CGRect rect = CGRectMake(0.0, 0.0, ratio * size.width, ratio * size.height);
	
//	UIGraphicsBeginImageContext(rect.size);
//	[selectedImage drawInRect:rect];
//	recipe.thumbnailImage = UIGraphicsGetImageFromCurrentImageContext();
//	UIGraphicsEndImageContext();
	
    [self dismissModalViewControllerAnimated:YES];
}

- (void)imagePickerControllerDidCancel:(UIImagePickerController *)picker {
    [self dismissModalViewControllerAnimated:YES];
}


- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
	if ((indexPath.section == 0) && (indexPath.row == 0)) {
		return 110;
	}
	
	return 40;
}

- (CGFloat)tableView:(UITableView *)tableView heightForHeaderInSection:(NSInteger)section
{
	return 5.0;
}



@end


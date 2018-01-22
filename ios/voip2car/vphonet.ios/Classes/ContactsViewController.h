//
//  ContactsViewController.h
//  Vphonet

#import <UIKit/UIKit.h>


@interface ContactsViewController : UITableViewController  {

	BOOL hasAppeared;
	BOOL scrollWasEnabled;
	UIView *emptyOverlay;

	IBOutlet UIView *optionsCall;    
}

//- (void) reloadData;
- (void) checkEmpty;


@end

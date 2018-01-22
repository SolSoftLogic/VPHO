//
//  ConferenceViewController.h
//  Vphonet
//
//

#import <UIKit/UIKit.h>
#import "VPContactList.h"

@protocol ContactSelectorViewControllerDelegate

- (void) contactSelector:(id)sender finishedWithContact:(VPContact*)contact;

@end


@interface ContactSelectorViewController : UITableViewController {
    id<ContactSelectorViewControllerDelegate> delegate;
}

@property (nonatomic, assign) id<ContactSelectorViewControllerDelegate> delegate;

- (id) init;

@end

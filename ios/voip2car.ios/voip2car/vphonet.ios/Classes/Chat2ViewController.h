//
//  ChatViewController.h
//  Vphonet


#import <UIKit/UIKit.h>

typedef void *VPCALL;

@interface Chat2ViewController : UIViewController {
//    VPCALL vpcall;
    
	VPContact *contact;

    float y;
    
    IBOutlet UIScrollView	*contentView;
    IBOutlet UIView			*chatEntryView;
    IBOutlet UITextField	*chatEntry;
}

@property (nonatomic, retain) IBOutlet UIScrollView *contentView;
@property (nonatomic, retain) IBOutlet UIView		*chatEntryView;
@property (nonatomic, retain) IBOutlet UITextField	*chatEntry;


- (id) initWithContact:(VPContact*)vpcontact;


- (IBAction) sendMessage:(id)sender;

- (IBAction) chatEntryDidClick:(id)sender;

@end

//
//  WhattodoViewController.h
//  Vphonet


#import <UIKit/UIKit.h>

#import "VPContactList.h"

@interface WhattodoViewController : UIViewController <UITabBarControllerDelegate, UIActionSheetDelegate, UIImagePickerControllerDelegate, UINavigationControllerDelegate> {
    IBOutlet UILabel *name;
    IBOutlet UILabel *email;
    IBOutlet UIImageView *statusImage;
    IBOutlet UILabel *statusLabel;
	IBOutlet UIButton *buttonPhoto;
	
	
    VPContact *contact;
    id tabbarDelegate;
	UIImagePickerController* imagePickerController;
	UIImageView* imageView;
	NSDateFormatter* formatter;

	IBOutlet UIButton *buttonAdd;
	IBOutlet UIButton *buttonVoice;
	IBOutlet UIButton *buttonVideo;
	IBOutlet UIButton *buttonChat;
	IBOutlet UIButton *buttonFile;
	
	IBOutlet UIButton *backgroundName;
	IBOutlet UIButton *backgroundHeader;
	IBOutlet UIButton *backgroundButton;
}

@property (nonatomic, retain) IBOutlet UILabel *name;
@property (nonatomic, retain) IBOutlet UILabel *email;
@property (nonatomic, retain) IBOutlet UIImageView *statusImage;
@property (nonatomic, retain) IBOutlet UILabel *statusLabel;
@property (nonatomic, retain) IBOutlet UIButton *buttonPhoto;


@property (nonatomic, retain) IBOutlet UIButton	*buttonAdd;
@property (nonatomic, retain) IBOutlet UIButton	*buttonVoice;
@property (nonatomic, retain) IBOutlet UIButton	*buttonVideo;
@property (nonatomic, retain) IBOutlet UIButton	*buttonChat;
@property (nonatomic, retain) IBOutlet UIButton	*buttonFile;

@property (nonatomic, retain) IBOutlet UIButton	*backgroundName;
@property (nonatomic, retain) IBOutlet UIButton	*backgroundHeader;
@property (nonatomic, retain) IBOutlet UIButton	*backgroundButton;

- (IBAction) onAddClick: (id)sender;
- (IBAction) onVoiceClick: (id)sender;
- (IBAction) onVideoClick: (id)sender;
- (IBAction) onChatClick: (id)sender;
- (IBAction) onFileClick: (id)sender;

- (id) initWithVPContact:(VPContact*)contact;

- (void)imagePickerController:(UIImagePickerController *)picker
        didFinishPickingImage:(UIImage *)image
				  editingInfo:(NSDictionary *)editingInfo;

- (void)imagePickerControllerDidCancel:(UIImagePickerController *)picker;



@end

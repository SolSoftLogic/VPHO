//
//  VideocallViewController.h
//  Vphonet
#import <UIKit/UIKit.h>
#import "HistoryEvent.h"
#import "VPContactList.h"
#import "ContactSelectorViewController.h"

@interface Caller : NSObject {
    VPCALL call;
    time_t callTime;
    
    // GUI
    UIImageView     *videoView;
    UILabel			*callerNameLabel;
    UILabel			*timeLabel;
}
@property (nonatomic, assign) VPCALL call;
@property (nonatomic, assign) time_t callTime;
@property (nonatomic, assign) UIImageView *videoView;
@property (nonatomic, assign) UILabel	  *callerNameLabel;
@property (nonatomic, assign) UILabel	  *timeLabel;

@end

@interface CallViewController : UIViewController <ContactSelectorViewControllerDelegate> {
	VPContact		*contact;
	NSMutableArray  *callers;

	NSTimer			*updateGUITimer;
    
	// GUI
    UIView          *topView; // callers info, timing, etc..
    UIView          *bottomView; // toolbar with "End Call" button
    UIView          *cameraPreview;
    UIImageView		*callerView;

	// Options popup
	UIView		*optionsCall;
	UIButton	*buttonHold;
	UIButton	*buttonMute;
	UIButton	*buttonSpeaker;
	UIButton	*buttonVideo;
	
}

@property (nonatomic, retain) IBOutlet UIView	*optionsCall;
@property (nonatomic, retain) IBOutlet UIButton *buttonHold;
@property (nonatomic, retain) IBOutlet UIButton *buttonMute;
@property (nonatomic, retain) IBOutlet UIButton *buttonSpeaker;
@property (nonatomic, retain) IBOutlet UIButton *buttonVideo;

- (IBAction) onMuteClick: (id)sender;
- (IBAction) onKeypadClick: (id)sender;
- (IBAction) onSpeekerClick: (id)sender;
- (IBAction) onAddCallClick: (id)sender;
- (IBAction) onHoldClick: (id)sender;
- (IBAction) onCaptureClick: (id)sender;
- (IBAction) onEndCallClick: (id)sender;



- (id)initWithCall:(VPCALL)call;

- (id)initWithContact:(VPContact*)vpcontact bearer:(unsigned)bearer;

- (void) hangup:(id)sender;

@end

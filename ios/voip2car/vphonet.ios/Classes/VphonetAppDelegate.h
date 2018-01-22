//
//  VphonetAppDelegate.h
//  Vphonet


#import <UIKit/UIKit.h>
#import "VPEngine.h"
#import "UICustomColor.h"

@interface UITableViewController (CustomTitle)
- (void)setCustomBackground:(NSString*)image;
@end

@interface UITabBar (ColorExtensions)
- (void)recolorItemsWithColor:(UIColor *)color shadowColor:(UIColor *)shadowColor shadowOffset:(CGSize)shadowOffset shadowBlur:(CGFloat)shadowBlur;
@end

@interface UITabBarItem (Private)
@property(retain, nonatomic) UIImage *selectedImage;
- (void)_updateView;
@end


@interface VphonetAppDelegate : NSObject <UIApplicationDelegate, UITabBarControllerDelegate> {

//	UIWindow				*window;
//	UINavigationController	*navigationController;
	
    NSMutableArray			*incommingAlerts;
	NSMutableDictionary		*callsAndFiles;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet UITabBarController *tabBarController;

//@property (nonatomic, retain) IBOutlet UINavigationController *navigationController;

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex;

- (void)showLogin:(bool)clearFields;

- (void) addCallAndFile:(VPCALL)call file:(NSString*)file;

- (NSString*) getCallFileName:(VPCALL)call;

+ (void) setButtonYellowBorder:(UIButton *)button;

- (void) activityUpdate;

@end

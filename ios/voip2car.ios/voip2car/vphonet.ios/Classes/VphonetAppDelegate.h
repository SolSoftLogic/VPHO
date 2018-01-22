//
//  VphonetAppDelegate.h
//  Vphonet


#import <UIKit/UIKit.h>
#import "VPEngine.h"

@interface UIColor (Extensions)
+ (UIColor *)paleYellowColor;
+ (UIColor *)titleColor;
@end


@interface UITableViewController (CustomTitle)
- (void)setCustomBackground:(NSString*)image;
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

@end

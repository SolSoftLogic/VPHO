//
//  VPEngine.h
//  Vphonet

#import <Foundation/Foundation.h>

#import "vpstack/vpstack.h" 
#import "vpstack/sip.h"
#import "vpstack/mixingaudio.h"
#import "vpstack/ios/video.h"
#import "VPLogger.h"
#import "VPHistory.h"
#import "VPProfile.h"
#import "VPContactList.h"

UIKIT_EXTERN NSString *const VPEngineNotification; // always sent on main thread. userInfo contains msg, param1, param2 
UIKIT_EXTERN NSString *const OnSmsNotification; // always sent on main thread. userInfo contains msg, param1, param2 
UIKIT_EXTERN NSString *const OnSearchNotification; // Search result 

/// Singleton
@interface VPEngine : NSObject {
	VPMIXINGAUDIOFACTORY	vpmixingaudiofactory;
	VPIOSVIDEODATAFACTORY	vpvideofactory;
	IVPSTACK				*vp;
	
	HistoryAVCallEvent*		callEvent;
	AOL						aol;
}

+ (VPEngine*) sharedInstance;

+ (NSString *)GetUUID;

+ (NSString *)generateFileName:(NSString*)fileExtension;

- (int) login:(NSString*)user password:(NSString*)password;

- (int) logoff;

- (VPCALL) makeCall:(NSString*)user bearer:(int)bc;

- (unsigned) getCallBearer:(VPCALL)call;

- (int)getCallStatus:(VPCALL)call;

- (int) sendSMS:(NSString*)number text:(NSString*)text;

- (int) sendContactAdded:(NSString*)number;
- (int) sendContactAck:(NSString*)number;
- (int) sendContactNack:(NSString*)number;


- (int) sendChatMsg:(VPCALL)call text:(NSString*)text;

- (int) sendFile:(VPCALL)call file:(NSString*)file;

- (NSString*) getChatMessage:(VPCALL)call;

- (NSString*) getFilePath:(VPCALL)call;

- (int) answer:(VPCALL)vpcall;

- (int) hangup:(VPCALL)vpcall;

- (int) acceptFile:(VPCALL)vpcall;

- (int) rejectFile:(VPCALL)vpcall;

- (NSString *)pathForTemporaryFileWithPrefix:(NSString *)prefix;

- (NSString*) getRemoteCallName:(VPCALL)vpcall;

- (NSString*) reasonCodeToString:(int)reasonCode;

- (NSString*) getLogonName;

- (NSString*) getLogonNumber;

- (void)askVpnumber:(NSString*)name;

- (void) askOnline:(NSArray*)numbers;

- (int) loadPhoto:(NSString*)number;

- (int) loadPhoto:(NSString*)number lastUpdate:(NSString*)update;

- (int) updatePhoto:(NSString*)file;

- (int) postProfile:(VPProfile*)profile;

- (int) postContacts:(VPContactList*)contacts;

- (int) downloadAddressBook;

- (int) addToConference:(VPCALL)call;

- (int) conferenceCalls:(VPCALL*)calls:(int)count:(BOOL)detach;

- (int) stopConference;

- (int) userSearch:(NSString*)search inCountry:(NSString*)country;

- (int) userStatus;

- (void) setUserStatus:(int)status;

- (int) hold:(VPCALL)vpcall; 
- (int) resume:(VPCALL)vpcall;  
- (BOOL) isHold:(VPCALL)vpcall;
- (BOOL) isMute;
- (void) mute:(BOOL)on;
- (BOOL) isSpeaker;
- (void) speaker:(BOOL)on;


@end

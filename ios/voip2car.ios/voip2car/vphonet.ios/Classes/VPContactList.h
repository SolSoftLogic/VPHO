//
//  VPContactList.h
//  Vphonet

#import <Foundation/Foundation.h>
//#import "VPEngine.h"

//#define VPMMSG_CONTACTLISTSYNCHRONIZED VPMSG_BASE+10002
//#define VPMMSG_CONTACTLISTMODIFIED     VPMSG_BASE+10003

extern NSString *OnContactListCompleteNotification;

@interface VPContact : NSObject {
/*
    unsigned cid;    
    NSString *username;
    NSString *vpnumber;
    NSString *sdial;
    NSString *firstName;
    NSString *lastName;	
    NSString *country;	
    NSString *state;	
    NSString *homepage;
    NSString *email;	
    NSString *mobile;	
    NSString *office;	
    unsigned status;
*/	
	NSString *regCode;
	NSString *userName;
	NSString *number;
	NSString *firstName;
    NSString *lastName;
    NSString *email;
    NSString *country;
    NSString *state;
    NSString *city;
    NSString *birthday;
    NSString *gender;
    NSString *phoneHome;
    NSString *phoneOffice;
    NSString *phoneMobile;
	NSString *photoUpdate;			
	NSUInteger flags;
	NSUInteger status;
	NSUInteger online;

	//	NSString *statusMessage;
//	double	lastLogin;

//	NSString *countryName;
//    NSString *countryCode;
}

- (id) initWithId:(unsigned)contactId;

- (NSString*) photoImageName;

- (NSString*) statusImageName;

- (NSString*) statusName;


//@property (nonatomic, readonly) unsigned cid;
//@property (nonatomic) unsigned status;


@property (nonatomic, copy) NSString *regCode;
@property (nonatomic, copy) NSString *userName;
@property (nonatomic, copy) NSString *number;
@property (nonatomic, copy) NSString *firstName;
@property (nonatomic, copy) NSString *lastName;
@property (nonatomic, copy) NSString *email;
@property (nonatomic, copy) NSString *country;
@property (nonatomic, copy) NSString *state;
@property (nonatomic, copy) NSString *city;
@property (nonatomic, copy) NSString *birthday;
@property (nonatomic, copy) NSString *gender;
@property (nonatomic, copy) NSString *phoneHome;
@property (nonatomic, copy) NSString *phoneOffice;
@property (nonatomic, copy) NSString *phoneMobile;
@property (nonatomic, copy) NSString *photoUpdate;			
@property (nonatomic, assign) NSUInteger flags;

@property (nonatomic, assign) NSUInteger status;
@property (nonatomic, assign) NSUInteger online;
//@property (nonatomic, copy) NSString *countryName;
//@property (nonatomic, copy) NSString *countryCode;


/*
@property (nonatomic, retain) NSString *username;
@property (nonatomic, retain) NSString *vpnumber;
@property (nonatomic, retain) NSString *sdial;

@property (nonatomic, retain) NSString *firstName;
@property (nonatomic, retain) NSString *lastName;
@property (nonatomic, retain) NSString *country;
@property (nonatomic, retain) NSString *state;

@property (nonatomic, retain) NSString *homepage;
@property (nonatomic, retain) NSString *email;
@property (nonatomic, retain) NSString *mobile;
@property (nonatomic, retain) NSString *office;
*/


@end

@interface VPContactList : NSObject {
    unsigned owner;
    NSMutableArray *contactsOnline;
    NSMutableArray *contactsOffline;
}

@property (nonatomic, retain) NSMutableArray *contactsOnline;
@property (nonatomic, retain) NSMutableArray *contactsOffline;

+ (VPContactList*) sharedInstance;

- (id) init;

- (NSMutableArray*) onlineContacts;
- (NSMutableArray*) offlineContacts;

- (void) loadFromSearchList:(NSString*)list;
- (void) loadFromContactList:(NSString*)list;

- (void) downloadAddressBook;
- (void) refreshOnlineStatus;

- (void)loadPhoto:(NSString*)number lastUpdate:(NSString*)update;

- (VPContact*) contactByNumber:(NSString*)number;
//- (VPContact*) contactByUsername:(NSString*)username;


//- (void) addOrReplaceContact:(VPContact*)contact;

@end

//
//  VPDb.h
//  Vphonet

#import <Foundation/Foundation.h>

@class VPDatabaseImpl;

@interface VPDatabase : NSObject {
    VPDatabaseImpl *impl;
}

+ (VPDatabase*) sharedInstance;

- (NSArray*) query:(NSString*)sql;

- (BOOL) exec:(NSString*)sql;

@end

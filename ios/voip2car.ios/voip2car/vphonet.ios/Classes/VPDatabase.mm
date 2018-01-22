//
//  VPDatabase.mm
//  Vphonet

#import "sqlite3.h"
#import "VPLogger.h"
#import "VPDatabase.h"

@interface VPDatabaseImpl : NSObject {
@public
    sqlite3* db;
}
@end;
@implementation VPDatabaseImpl
@end;


@implementation VPDatabase

static VPDatabase *vpDatabaseSharedInstance = nil;

#pragma mark ---- VPDatabase singleton object methods ----

+ (VPDatabase*)sharedInstance 
{
	@synchronized(self) 
	{
		if (vpDatabaseSharedInstance == nil) 
		{
			[[self alloc] init]; // assignment not done here
		}
	}
	return vpDatabaseSharedInstance;
}

+ (id)allocWithZone:(NSZone *)zone 
{
	@synchronized(self) 
	{
		if (vpDatabaseSharedInstance == nil) 
		{
			vpDatabaseSharedInstance= [super allocWithZone:zone];
			return vpDatabaseSharedInstance;  // assignment and return on first allocation
		}
	}
	return nil; // on subsequent allocation attempts return nil
}

- (id)copyWithZone:(NSZone *)zone
{
	return self;
}

- (id)retain 
{
	return self;
}

- (unsigned)retainCount 
{
	return UINT_MAX;  // denotes an object that cannot be released
}

- (void)release 
{
	//do nothing
}

- (id)autorelease 
{
	return self;
}

#pragma mark ---- VPDatabase object methods ----

-(id) init
{
    self = [super init];
    
    impl = [[VPDatabaseImpl alloc] init];

    char *error;      
    
    NSArray *documentPaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *dbPath = [NSString stringWithFormat:@"%@/db.sqlite",
                        [documentPaths objectAtIndex:0]];
    
    VP_LOG_MSG(VP_LOG_DEBUG, @"Using db: %@", dbPath); 
    
    int rc = sqlite3_open([dbPath cStringUsingEncoding:NSASCIIStringEncoding], &impl->db);
    if (rc != SQLITE_OK) 
    {
        VP_LOG_MSG(VP_LOG_ERROR, @"Could not open db: %d", rc);
        return nil;
    }
    
    sqlite3_busy_timeout(impl->db, 3000);
    
    NSString *sql = [[NSString alloc] initWithContentsOfFile:
                     [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"db.sql"]];
    

    if(sql == nil)
    {
        VP_LOG_MSG(VP_LOG_ERROR, @"Error reading db.sql file");
        return nil;
    }
    
    rc = sqlite3_exec(impl->db, [sql cStringUsingEncoding:NSASCIIStringEncoding], 
                      NULL, NULL, &error); 
    
    if (rc != SQLITE_OK) 
    { 
        VP_LOG_MSG(VP_LOG_ERROR, @"SQL error: %s (%d)", error?error:"", rc); 
        if (error) sqlite3_free(error); 
        return nil;
    }

    
    return self;
    
}

- (NSArray*) query:(NSString*)sql
{
    sqlite3_stmt* stmt;
    int i;
    int rc = sqlite3_prepare(impl->db, [sql cStringUsingEncoding:NSASCIIStringEncoding], 
                             -1, &stmt, 0);
    if (rc != SQLITE_OK) 
    {
        sqlite3_finalize(stmt);
        VP_LOG_MSG(VP_LOG_ERROR, @"Could not execute query: \"%@\"", sql);
        return nil;
    }   


    int columnCount = sqlite3_column_count(stmt);
/*    NSMutableArray *columnNames = [[NSMutableArray alloc] initWithCapacity:columnCount];
    for (i = 0; i < columnCount; i++)
    {
        
    }
  */  
    
    //    sqlite3_column_origin_name(sqlite3_stmt*,int);
  
    NSMutableArray *rows = [[NSMutableArray alloc] init];
    while (sqlite3_step(stmt) == SQLITE_ROW) 
    {
        NSMutableArray *columns = [[[NSMutableArray alloc] initWithCapacity:columnCount] autorelease];
        for (i = 0; i < columnCount; i++)
        {
            const char *val = (const char*)sqlite3_column_text(stmt, i);
            if (val && val[0])
            {
                [columns addObject:[NSString stringWithCString:val
                                             encoding:NSUTF8StringEncoding]];
            }
            else 
            {
                [columns addObject:@""];
            }

        }
        
        [rows addObject:columns];
    }

    sqlite3_finalize(stmt);

    return [rows autorelease];
}

- (BOOL) exec:(NSString*)sql
{
    char *error; 
    int rc;
    rc = sqlite3_exec(impl->db, [sql cStringUsingEncoding:NSASCIIStringEncoding], 
                      NULL, NULL, &error); 
    
    if (rc != SQLITE_OK) 
    { 
        VP_LOG_MSG(VP_LOG_ERROR, @"SQL error: %s (%d)", error?error:"", rc); 
        if (error) sqlite3_free(error); 
        return NO;
    }

    return YES;
}

@end

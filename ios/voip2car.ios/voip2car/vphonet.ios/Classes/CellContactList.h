//
//  CellContactList.h
//  Vphonet
//
//  Created by uncle on 24.05.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface CellContactList : UITableViewCell {

	UIImageView	*imagePhoto;
	UIImageView	*imageStatus;
	UILabel		*label;
    
}

@property (nonatomic, retain) IBOutlet UIImageView	*imagePhoto;
@property (nonatomic, retain) IBOutlet UIImageView	*imageStatus;
@property (nonatomic, retain) IBOutlet UILabel		*label;

@end

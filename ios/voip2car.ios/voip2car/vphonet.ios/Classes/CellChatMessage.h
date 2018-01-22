//
//  CellChatMessage.h
//  Vphonet
//
//  Created by uncle on 06.06.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface CellChatMessage : UITableViewCell {
 
	IBOutlet UILabel		*labelName;
	IBOutlet UILabel		*labelDate;
	IBOutlet UILabel		*labelStatus;
	IBOutlet UILabel		*labelMessage;
	IBOutlet UIButton		*background;
	IBOutlet UIImageView	*imageLeftBeak;	
	IBOutlet UIImageView	*imageRightBeak;	
}

@property (nonatomic, retain) IBOutlet UILabel		*labelName;
@property (nonatomic, retain) IBOutlet UILabel		*labelDate;
@property (nonatomic, retain) IBOutlet UILabel		*labelStatus;
@property (nonatomic, retain) IBOutlet UILabel		*labelMessage;
@property (nonatomic, retain) IBOutlet UIButton		*background;
@property (nonatomic, retain) IBOutlet UIImageView	*imageLeftBeak;
@property (nonatomic, retain) IBOutlet UIImageView	*imageRightBeak;

@end

//
//  CellProfileHeader.h
//  Vphonet
//
//  Created by uncle on 03.05.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface CellProfileHeader : UITableViewCell {

	IBOutlet UILabel		*labelName;
	IBOutlet UILabel		*labelEMail;
	IBOutlet UIButton		*buttonPhoto;
//	IBOutlet UIButton		*buttonPhoto;
	IBOutlet UIImageView	*statusImage;
	IBOutlet UILabel		*statusLabel;

	IBOutlet UIButton	*backgroundName;
	IBOutlet UIButton	*backgroundHeader;
}


@property (nonatomic, retain) IBOutlet UILabel		*labelName;
@property (nonatomic, retain) IBOutlet UILabel		*labelEMail;
@property (nonatomic, retain) IBOutlet UIButton		*buttonPhoto;
@property (nonatomic, retain) IBOutlet UIImageView	*statusImage;
@property (nonatomic, retain) IBOutlet UILabel		*statusLabel;

@property (nonatomic, retain) IBOutlet UIButton	*backgroundName;
@property (nonatomic, retain) IBOutlet UIButton	*backgroundHeader;


@end

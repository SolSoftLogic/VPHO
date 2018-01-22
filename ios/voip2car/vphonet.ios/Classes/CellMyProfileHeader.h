//
//  CellMyProfileHeader.h
//  Vphonet
//
//  Created by uncle on 26.05.11.
//  Copyright 2011 Lundlay. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface CellMyProfileHeader : UITableViewCell {
	IBOutlet UIView			*tableHeaderView;    
	IBOutlet UIButton		*photoView;
	IBOutlet UILabel		*labelName;
	IBOutlet UILabel		*labelStatus;
	IBOutlet UIImageView	*imageStatus;
	IBOutlet UIButton		*buttonAdd;
	
	
	IBOutlet UIButton		*backgroundHeader;
	IBOutlet UIButton		*backgroundName;
	IBOutlet UIButton		*backgroundStatus;
    
}

@property (retain, nonatomic) IBOutlet UIView *tableHeaderView;
@property (retain, nonatomic) IBOutlet UIButton *photoView;
@property (retain, nonatomic) IBOutlet UILabel *labelName;
@property (retain, nonatomic) IBOutlet UILabel *labelStatus;
@property (retain, nonatomic) IBOutlet UIImageView *imageStatus;
@property (retain, nonatomic) IBOutlet UIButton *buttonAdd;

@property (assign, nonatomic) IBOutlet UIButton		*backgroundHeader;
@property (assign, nonatomic) IBOutlet UIButton		*backgroundName;
@property (assign, nonatomic) IBOutlet UIButton		*backgroundStatus;


@end

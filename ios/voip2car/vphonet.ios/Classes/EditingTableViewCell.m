
#import "EditingTableViewCell.h"

@implementation EditingTableViewCell

@synthesize label, textField;


- (id)initWithStyle:(UITableViewCellStyle)style reuseIdentifier:(NSString *)reuseIdentifier
{
    self = [super initWithStyle:style reuseIdentifier:reuseIdentifier];
    if (self) {
        // Initialization code
		UIView *backView = [[[UIView alloc] initWithFrame:CGRectZero] autorelease];
		backView.backgroundColor = [UIColor clearColor];
		
		self.backgroundView = backView;
    }
    return self;
}

- (NSString *) reuseIdentifier {
	return @"EditingTableViewCell";
}
/*
- (id)initWithStyle:(UITableViewCellStyle)style reuseIdentifier:(NSString *)reuseIdentifier {
    if ((self = [super initWithStyle:style reuseIdentifier:reuseIdentifier])) {
        // Initialization code
    }
    return self;
}
*/ 


- (void)dealloc {
	[label release];
	[textField release];
	[super dealloc];
}

@end

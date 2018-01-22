

@interface EditingTableViewCell : UITableViewCell {
	UILabel *label;
	UITextField *textField;
}

@property (nonatomic, retain) IBOutlet UILabel *label;
@property (nonatomic, retain) IBOutlet UITextField *textField;

@end

package com.vphonenet;

import java.util.ArrayList;
import java.util.List;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.res.Resources;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.inputmethod.EditorInfo;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.Spinner;
import android.widget.SpinnerAdapter;
import android.widget.TextView;
import android.widget.TextView.OnEditorActionListener;
class Country
{
	private String 			stCountry_ 			= null;
	private String 			stPrefix_ 			= null;
	private String 			stCode_ 			= null;
	private String 			stPhoneCd_ 			= null;
	
	public Country(String pr, String ct, String cd, String ph)
	{
		stPrefix_ = pr;
		stCountry_ = ct;
		stCode_ = cd;
		stPhoneCd_ = ph;			
	}
	
	public Country(String pr[])
	{
		if (pr.length > 3)
		{
			stPrefix_ = pr[0];
			stCountry_ = pr[1];
			stCode_ = pr[2];
			stPhoneCd_ = pr[3];	
		}
	}	
		
	private 		Country()			{};
	public String 	getCountry()		{return stCountry_;};
	public String 	getPrefix()			{return stPrefix_;};
	public String 	getCode()			{return stCode_;};
	public String 	getPhoneCd()		{return stPhoneCd_;};
}

/*
 * 
 *   TO DO:  add DO (Dominicana Republic) flag  !!!!!!!!!!!!!
 * 
 * 
 */

public class Registrat2 extends Activity
{
	static private final int 		WRONG_USER_NAME					= 2;
	static private final int 		SILENT_TO_DIADN					= 3;
	private String					stDiagno_						= null;
	private EditText 				etPhoNo_ 						= null;
	private Button 					btDone_ 						= null;
	private Spinner					spCountries_					= null;
	private OrderAdapter 			adCountries_					= null;
	private ArrayList<Country> 		arCountry_						= null;
	private String 					arCountries_[][] 				= {
			{"AF", "Afghanistan", "197", "+93"},
			{"AX", "Aland Islands", "255", "+358"},
			{"AL", "Albania", "74", "+355"},
			{"DZ", "Algeria", "5", "+213"},
			{"AS", "American Samoa", "156", "+684"},
			{"AD", "Andorra", "221", "+376"},
			{"AO", "Angola", "32", "+244"},
			{"AI", "Anguilla", "172", "+1"},
			{"AQ", "Antarctica", "144", "+672"},
			{"AG", "Antigua and Barbuda", "222", "+1"},
			{"AR", "Argentina", "120", "+54"},
			{"AM", "Armenia", "83", "+374"},
			{"AW", "Aruba", "61", "+297"},
			{"", "Ascension", "", "+247"},
			{"AU", "Australia", "136", "+61"},
			{"", "Australian External Territories", "", "+672"},
			{"AT", "Austria", "99", "+43"},
			{"AZ", "Azerbaijan", "219", "+994"},
			{"BS", "Bahamas", "175", "+1"},
			{"BH", "Bahrain", "212", "+973"},
			{"BD", "Bangladesh", "191", "+880"},
			{"BB", "Barbados", "176", "+1"},
			{"BY", "Belarus", "84", "+375"},
			{"BE", "Belgium", "66", "+32"},
			{"BZ", "Belize", "107", "+501"},
			{"BJ", "Benin", "17", "+229"},
			{"BM", "Bermuda", "177", "+1"},
			{"BT", "Bhutan", "214", "+975"},
			{"BO", "Bolivia", "126", "+591"},
			{"BA", "Bosnia and Herzegovina", "90", "+387"},
			{"BW", "Botswana", "54", "+267"},
			{"BV", "Bouvet Islands", "257", ""},
			{"BR", "Brazil", "121", "+55"},
			{"IO", "British Indian Ocean Territory", "258", ""},
			{"BN", "Brunei Darussalm", "145", "+673"},
			{"BG", "Bulgaria", "78", "+359"},
			{"BF", "Burkina Faso (former Upper Volta)", "14", "+226"},
			{"BI", "Burundi", "45", "+257"},
			{"KH", "Cambodia (Kingdom of)", "223", "+855"},
			{"CM", "Cameroon", "25", "+237"},
			{"CA", "Canada", "2", "+1"},
			{"CV", "Cape Verde", "26", "+238"},
			{"KY", "Cayman Islands", "224", "+1"},
			{"CF", "Central African Republic", "24", "+236"},
			{"TD", "Chad", "23", "+235"},
			{"CL", "Chile", "122", "+56"},
			{"CN", "China", "187", "+86"},
			{"CX", "Christmas Island", "225", "+61"},
			{"CC", "Cocos (Keeling) Islands", "226", "+61"},
			{"CO", "Colombia", "123", "+57"},
			{"KM", "Comoros", "270", "+269"},
			{"CG", "Congo", "30", "+242"},
			{"CD", "Congo, The Democratic Republic of the", "31", "+243"},
			{"CK", "Cook Islands", "154", "+682"},
			{"CR", "Costa Rica", "112", "+506"},
			{"CI", "Cote dIvoire", "260", "+225"},
			{"HR", "Croatia", "89", "+385"},
			{"CU", "Cuba", "118", "+53"},
			{"CY", "Cyprus", "76", "+357"},
			{"CZ", "Czech Republic", "227", "+420"},
			{"DK", "Denmark", "101", "+45"},
			{"", "Diego Garcia", "", "+246"},
			{"DJ", "Djibouti", "41", "+253"},
			{"DM", "Dominica", "228", "+1"},
			{"DO", "Dominican Republic", "229", "+1"},
			{"EC", "Ecuador", "128", "+593"},
			{"EG", "Egypt", "3", "+20"},
			{"SV", "El Salvador", "109", "+503"},
			{"GQ", "Equatorial Guinea", "28", "+240"},
			{"ER", "Eritrea", "230", "+291"},
			{"EE", "Estonia", "231", "+372"},
			{"ET", "Ethiopia", "39", "+251"},
			{"FK", "Falkland Islands(Malvinas)", "106", "+500"},
			{"FO", "Faroe Islands", "62", "+298"},
			{"FJ", "Fiji", "151", "+679"},
			{"FI", "Finland", "77", "+358"},
			{"FR", "France", "67", "+33"},
			{"GF", "French Guiana", "129", "+594"},
			{"PF", "French Polynesia", "232", "+689"},
			{"TF", "French Southern Territories", "261", ""},
			{"GA", "Gabon", "29", "+241"},
			{"GM", "Gambia", "8", "+220"},
			{"GE", "Georgia", "220", "+995"},
			{"DE", "Germany", "105", "+49"},
			{"GH", "Ghana", "21", "+233"},
			{"GI", "Gibraltar", "69", "+350"},
			{"GR", "Greece", "64", "+30"},
			{"GL", "Greenland", "63", "+299"},
			{"GD", "Grenada", "233", "+1"},
			{"", "Group of countries, shared code", "", "+388"},
			{"GP", "Guadeloupe", "234", "+590"},
			{"GU", "Guam", "143", "+1"},
			{"GT", "Guatemala", "108", "+502"},
			{"GN", "Guinea", "12", "+224"},
			{"GW", "Guinea-Bissau", "33", "+245"},
			{"GY", "Guyana", "127", "+592"},
			{"HT", "Haiti", "115", "+509"},
			{"HM", "Heard Island And Mcdonald Islands", "262", ""},
			{"VA", "Holy See (Vatican City State)", "94", "+39"},
			{"", "Holy See (Vatican City State)", "", "+379"},
			{"HN", "Honduras", "110", "+504"},
			{"HK", "Hong Kong", "183", "+852"},
			{"HU", "Hungary", "79", "+36"},
			{"IS", "Iceland", "73", "+354"},
			{"IN", "India", "194", "+91"},
			{"ID", "Indonesia", "137", "+62"},
			{"", "Inmarsat (Atlantic Ocean-East)", "", "+871"},
			{"", "Inmarsat (Atlantic Ocean-West)", "", "+874"},
			{"", "Inmarsat (Indian Ocean)", "", "+873"},
			{"", "Inmarsat (Pacific Ocean)", "", "+872"},
			{"", "Inmarsat SNAC", "", "+870"},
			{"", "International Freephone Service", "", "+800"},
			{"", "International Mobile, shared code", "", "+881"},
			{"", "International Networks, shared code", "", "+882"},
			{"IR", "Iran, Islamic Republic Of", "217", "+98"},
			{"IQ", "Iraq", "204", "+964"},
			{"IE", "Ireland", "72", "+353"},
			{"IL", "Israel", "211", "+972"},
			{"IT", "Italy", "93", "+39"},
			{"JM", "Jamaica", "235", "+1"},
			{"JP", "Japan", "179", "+81"},
			{"JO", "Jordan", "202", "+962"},
			{"KZ", "Kazakhstan", "166", "+7"},
			{"KE", "Kenya", "42", "+254"},
			{"KI", "Kiribati", "158", "+686"},
			{"KP", "Korea, Democratic People\'s Republic Of", "182", "+850"},
			{"KR", "Korea, Republic of", "180", "+82"},
			{"KW", "Kuwait", "205", "+965"},
			{"KG", "Kyrgystan", "167", "+7"},
			{"LA", "Lao Peoples Democratic Republic", "236", "+856"},
			{"LV", "Latvia", "81", "+371"},
			{"LB", "Lebanon", "201", "+961"},
			{"LS", "Lesotho", "53", "+266"},
			{"LR", "Liberia", "19", "+231"},
			{"LY", "Libyan Arab Jamahiriya", "237", "+218"},
			{"LI", "Liechtenstein", "96", "+423"},
			{"LT", "Lithuania", "80", "+370"},
			{"LU", "Luxembourg", "71", "+352"},
			{"MO", "Macao", "184", "+853"},
			{"MK", "Macedonia, The Former Yugoslav Republic Of", "238", "+389"},
			{"MG", "Madagascar", "48", "+261"},
			{"MW", "Malawi", "52", "+265"},
			{"MY", "Malaysia", "135", "+60"},
			{"MV", "Maldives", "200", "+960"},
			{"ML", "Mali", "11", "+223"},
			{"MT", "Malta", "75", "+356"},
			{"MH", "Marshall Islands", "164", "+692"},
			{"MQ", "Martinique", "131", "+596"},
			{"MR", "Mauritania", "10", "+222"},
			{"MU", "Mauritius", "18", "+230"},
			{"YT", "Mayotte", "56", "+269"},
			{"MX", "Mexico", "117", "+52"},
			{"FM", "Micronesia, Federated States Of", "163", "+691"},
			{"MD", "Moldova, Republic Of", "82", "+373"},
			{"MC", "Monaco", "85", "+377"},
			{"MN", "Mongolia", "215", "+976"},
			{"MS", "Montserrat", "239", "+1"},
			{"MA", "Morocco", "4", "+212"},
			{"MZ", "Mozambique", "46", "+258"},
			{"MM", "Myanmar", "199", "+95"},
			{"NA", "Namibia", "51", "+264"},
			{"NR", "Nauru", "146", "+674"},
			{"NP", "Nepal", "216", "+977"},
			{"NL", "Netherlands", "65", "+31"},
			{"AN", "Netherlands Antilles", "134", "+599"},
			{"NC", "New Caledonia", "159", "+687"},
			{"NZ", "New Zealand", "139", "+64"},
			{"NI", "Nicaragua", "111", "+505"},
			{"NE", "Niger", "15", "+227"},
			{"NG", "Nigeria", "22", "+234"},
			{"NU", "Niue", "155", "+683"},
			{"NF", "Norfolk Island", "240", "+672"},
			{"MP", "North Mariana Islands", "142", "+1"},
			{"NO", "Norway", "103", "+47"},
			{"OM", "Oman", "208", "+968"},
			{"PK", "Pakistan", "196", "+92"},
			{"PW", "Palau", "152", "+680"},
			{"PS", "Palestinian Territory, occupied", "273", "+970"},
			{"PA", "Panama", "113", "+507"},
			{"PG", "Papua New Guinea", "147", "+675"},
			{"PY", "Paraguay", "130", "+595"},
			{"PE", "Peru", "116", "+51"},
			{"PH", "Philippines", "138", "+63"},
			{"PN", "Pitcairn", "271", ""},
			{"PL", "Poland", "104", "+48"},
			{"PT", "Portugal", "70", "+351"},
			{"PR", "Puerto Rico", "241", "+1"},
			{"QA", "Qatar", "213", "+974"},
			{"RE", "Reunion", "49", "+262"},
			{"RO", "Romania", "95", "+40"},
			{"RU", "Russia Federation", "168", "+7"},
			{"RW", "Rwanda", "38", "+250"},
			{"SH", "Saint Helena", "58", "+290"},
			{"KN", "Saint Kitts And Nevis", "242", "+1"},
			{"LC", "Saint Lucia", "243", "+1"},
			{"PM", "Saint Pierre And Miquelon", "114", "+508"},
			{"VC", "Saint Vincent And The Grenadies", "244", "+1"},
			{"WS", "Samoa", "245", "+685"},
			{"SM", "San Marino", "59", "+378"},
			{"ST", "Sao Tome and Principe", "27", "+239"},
			{"SA", "Saudi Arabia", "206", "+966"},
			{"SN", "Senegal", "9", "+221"},
			{"CS", "Serbia And Montenegro", "246", "+381"},
			{"SC", "Seychelles", "36", "+248"},
			{"SL", "Sierra Leone", "20", "+232"},
			{"SG", "Singapore", "140", "+65"},
			{"SK", "Slovakia", "250", "+421"},
			{"SI", "Slovenia", "272", "+386"},
			{"SB", "Solomon Islands", "149", "+677"},
			{"SO", "Somalia", "40", "+252"},
			{"ZA", "South Africa", "57", "+27"},
			{"GS", "South Georgia And The South Sandwich Islands", "274", "+995"},
			{"ES", "Spain", "68", "+34"},
			{"LK", "Sri Lanka", "198", "+94"},
			{"SD", "Sudan", "37", "+249"},
			{"SR", "Suriname", "132", "+597"},
			{"SJ", "Svalbard And Jan Mayen", "264", "+47"},
			{"SZ", "Swaziland", "55", "+268"},
			{"SE", "Sweden", "102", "+46"},
			{"CH", "Switzerland", "97", "+41"},
			{"SY", "Syrian Arab Republic (Syria)", "203", "+963"},
			{"TW", "Taiwan -Province Of China", "192", "+886"},
			{"TJ", "Tajikistan", "169", "+992"},
			{"TZ", "Tanzania, United Republic Of", "43", "+255"},
			{"TH", "Thailand", "141", "+66"},
			{"TL", "Timor-Leste", "253", "+670"},
			{"TG", "Togo", "16", "+228"},
			{"TK", "Tokelau", "162", "+690"},
			{"TO", "Tonga", "148", "+676"},
			{"TT", "Trinidad and Tobago", "60", "+1"},
			{"TN", "Tunisia", "6", "+216"},
			{"TR", "Turkey", "193", "+90"},
			{"TM", "Turkmenistan", "218", "+993"},
			{"TC", "Turks And Caicos Islands", "254", "+1"},
			{"TV", "Tuvalu (Ellice Islands)", "160", "+688"},
			{"UG", "Uganda", "44", "+256"},
			{"UA", "Ukraine", "87", "+380"},
			{"AE", "United Arab Emirates", "210", "+971"},
			{"GB", "United Kingdom", "100", "+44"},
			{"US", "United States", "1", "+1"},
			{"UM", "United States Minor Outlying Islands", "266", ""},
			{"UY", "Uruguay", "133", "+598"},
			{"UZ", "Uzbekistan", "170", "+998"},
			{"VU", "Vanuatu", "150", "+678"},
			{"VE", "Venezuela", "124", "+58"},
			{"VN", "Viet Nam", "181", "+84"},
			{"VG", "Virgin Islands British", "178", "+1"},
			{"VI", "Virgin Islands, U.S", "256", "+1"},
			{"WF", "Wallis and Futuna", "153", "+681"},
			{"EH", "Western Sahara", "267", "+212"},
			{"YE", "Yemen", "209", "+967"},
			{"ZM", "Zambia", "47", "+260"},
			{"ZW", "Zimbabwe", "50", "+263"}};
	
	public void onCreate(Bundle savedInstanceState) 
	{
		super.onCreate(savedInstanceState);
	    requestWindowFeature(Window.FEATURE_NO_TITLE);
		setContentView(R.layout.regist2);
		Context ct = getApplicationContext();
		Resources rc = ct.getResources();
		spCountries_ = (Spinner) findViewById(R.id.country);
		etPhoNo_ = (EditText) findViewById(R.id.phonumber);
		arCountry_ = new ArrayList<Country>();
		for (int i = 0; i < arCountries_.length; i++)
			arCountry_.add( new Country(arCountries_[i]));
		adCountries_ = new OrderAdapter( this, arCountry_);	
		spCountries_.setAdapter(adCountries_);
		spCountries_.setSelection(8);
		etPhoNo_.setOnEditorActionListener(
				new OnEditorActionListener() 
				{
					public boolean onEditorAction(TextView v, int actionId, KeyEvent event) 
					{
						if (actionId == EditorInfo.IME_ACTION_SEARCH ||
							actionId == EditorInfo.IME_ACTION_NEXT || 
							actionId == EditorInfo.IME_ACTION_DONE) 
							return !checkPhone(WRONG_USER_NAME);
						return true;
					}
				});
		btDone_ = (Button) findViewById(R.id.done);
		btDone_.setOnClickListener(new View.OnClickListener() 
	    {
	    	public void onClick(View v) 
	    	{
	    		stDiagno_ = "";	
				if ( !checkPhone(SILENT_TO_DIADN))
				{
					Resources rc = getApplicationContext().getResources();	
					stDiagno_ = rc.getString(R.string.folerda) + stDiagno_;
					showMyDialog(WRONG_USER_NAME, stDiagno_);
				}	
	    	}
	    });
	}
	
	@Override
	public boolean dispatchKeyEvent(KeyEvent event) 
	{
	    if (event.getAction() == KeyEvent.ACTION_DOWN) 
	    {
	    	int r = event.getKeyCode();
	    	switch (event.getKeyCode()) 
	        {
	        	case KeyEvent.KEYCODE_DPAD_DOWN:	        			
	        	case KeyEvent.KEYCODE_DPAD_UP:
	        		return !checkField(WRONG_USER_NAME);
            }
        }
    	return super.dispatchKeyEvent(event);
	}
	
	private  void showMyDialog(int iDial, String st)
	{
		Context ct = getApplicationContext();
		Resources rc = ct.getResources();
		stDiagno_ = "";
		AlertDialog.Builder dlg=new AlertDialog.Builder(this);
		String sta = null;
		switch (iDial)
		{
			case WRONG_USER_NAME:		
				dlg.setTitle(rc.getString(R.string.error));
                dlg.setMessage(st);
                dlg.show();	
			case SILENT_TO_DIADN:
				if (stDiagno_.length() > 2)
					stDiagno_ = stDiagno_ + '\n';
				stDiagno_ = stDiagno_ + st;
		}
	}

	private boolean checkField(int iMood)
	{
		boolean bOut = false;
		View vv = this.getCurrentFocus();
		Resources i = vv.getResources();
		switch (vv.getId())
		{
			case R.id.phonumber:
				bOut = checkPhone(iMood);
				break;

		}
		return bOut;
	}
	
	private boolean checkPhone(int iMood)
	{
		boolean bOut = true;	
		String st = etPhoNo_.getText().toString();
		Resources rc = getApplicationContext().getResources();
		String stText = rc.getString(R.string.phonum);
		if (st == null)
		{
			stText = stText + " "+ rc.getString(R.string.wrphon);
			showMyDialog(iMood, stText);	
			bOut = false;
		}
		else
			if (st.length() < 10)
			{
				stText = stText + " "+ rc.getString(R.string.wrphon);
				showMyDialog(iMood, stText);
				bOut = false;
			}
			else
			{
				for(int i = 0; i < st.length(); i++)
					if (!Character.isDigit(st.charAt(i)))
					{
						stText = stText + " "+ rc.getString(R.string.wrphon);
						showMyDialog(iMood, stText);	
						bOut = false;
						break;
					}
			}		
		return bOut;
	}
	
	private class OrderAdapter extends BaseAdapter implements SpinnerAdapter
	{
	    private List<Country> _items;

		public int getCount() {	return _items.size();}

		public Country getItem(int position) {return _items.get(position);}

		public long getItemId(int position) {return position;}		
		
	    public OrderAdapter(Activity context, ArrayList<Country> items) 
	    {
	    	super();
	    	this._items = items;
	    }
//	    @Override
	    public View getView(int position, View convertView, ViewGroup parent) 
	    {
	    	View v = convertView;
	    	if (v == null) 
	    	{
	    		LayoutInflater vi = (LayoutInflater)getSystemService(Context.LAYOUT_INFLATER_SERVICE);
	    		v = vi.inflate(R.layout.spinnerline, null);
	    	}
	    	Country f = _items.get(position);
	    	if (f != null) 
	    	{
	    		TextView ct = null;
				TextView ph = null;
				ImageView fl = null;
      			ph = (TextView) v.findViewById(R.id.stPhone);
      			ct = (TextView) v.findViewById(R.id.stCountry);
    			fl = (ImageView) v.findViewById(R.id.flag);
    			String st = f.getPrefix().toLowerCase();
    			int resID = getResources().getIdentifier(st, "drawable", "com.vphonenet");
	    		if (fl != null)
	    				fl.setImageResource(resID);
	    		if (ct != null)
	    			ct.setText(f.getCountry()); 
	    		if (ph != null)
	    			ph.setText(f.getPhoneCd());
	    	}
	    	return v;
	    }
	}
}

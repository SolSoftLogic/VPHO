package com.vphonenet;

import java.util.ArrayList;

import android.app.ListActivity;
import android.content.Context;
import android.content.res.Resources;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.ArrayAdapter;
import android.widget.RelativeLayout;
import android.widget.TextView;
class ChatMessInfo
{
	private String 		stUserName_  			= null;
	private String 		stMessage_  			= null;	
	private String 		stTimeStamp_  			= null;	
	private String		stFileName_				= null;
	private byte[] 		byFile_  				= null;

	public ChatMessInfo(){};
	public ChatMessInfo(String username, String message, String timestamp)
	{
		stUserName_  			= username;
		stMessage_  			= message;	
		stTimeStamp_			= timestamp;
	};
	public ChatMessInfo(String username, String filename, String timestamp, byte[] file)
	{
		stUserName_  			= username;
		stFileName_  			= filename;	
		stTimeStamp_			= timestamp;
		byFile_ 				= file;
	};	
	
	String getUserName(){return stUserName_;};
	String getMessage(){return stMessage_;};
	String getTimeStamp(){return stTimeStamp_;};
	byte[] getFile(){return byFile_;};
	String getFileName(){return stFileName_;};
}
public class Chatting extends ListActivity
{
	private String stCorespName_						= "";
	private final String[] CONTACTLIST = new String[] 
	                                            	{
	                                            		"Aaron",
	                                            		"Dian Caspar",
	                                            		"Aaron",
	                                            		"Dian Caspar",
	                                            	    "Aaron",
	                                            	    "Dian Caspar"
	                                            	};	// !!!!! delete this dummy list
	private final String[] MESSAGETLIST = new String[] 
	                                            	{
	                                            		"I'm at work",
	                                            		"not here",
	                                            	    "Hi i'm fine thankssfdddfsdddddddddddddddddddddddddddddddddddddddddddddddddddddddddffffffffffffffff",
	                                            	    "Whats up",
	                                            	    "Good night",
	                                            	    "I dont like it"
	                                            	};	// !!!!! delete this dummy list
	static private ArrayList<ChatMessInfo> _lsChatMsgList = new ArrayList<ChatMessInfo>();	
	public void onCreate(Bundle savedInstanceState) 
	{
		super.onCreate(savedInstanceState);
	    requestWindowFeature(Window.FEATURE_NO_TITLE);
		setContentView(R.layout.chatting);
		_lsChatMsgList.clear();                          
		for (int i = 0; i < CONTACTLIST.length; i++)// !!!!! delete this cycle

				_lsChatMsgList.add(new ChatMessInfo(CONTACTLIST[i], MESSAGETLIST[i],"Sep 3, 3.34 PM"));

		OrderAdapter aaCurLst = new OrderAdapter
				(this, R.layout.chatline, _lsChatMsgList);
		int y = aaCurLst.getCount();
		setListAdapter(aaCurLst); 
		TextView tv = (TextView) findViewById(R.id.title);
		if (tv != null)
		{
			Resources rc = getApplicationContext().getResources();	
			tv.setText(rc.getString(R.string.chatacti));
		}
		Bundle bundle = getIntent().getExtras();
		if (bundle != null)
		{
			/* you can replace next 5 lines with Native methods  */
			stCorespName_ = bundle.getString("CORRESPONDENT");
			TextView tvTitle = (TextView) findViewById(R.id.title);
			tvTitle.setText(stCorespName_);
		}
	
	
	}

	class OrderAdapter extends ArrayAdapter<ChatMessInfo> 
	{
		private ArrayList<ChatMessInfo> _items;
	    public OrderAdapter(Context context, int textViewResourceId, ArrayList<ChatMessInfo> items) 
	    {
	    	super(context, textViewResourceId, items);
	        this._items = items;
	    }
	        
	    @Override
	    public View getView(int position, View convertView, ViewGroup parent) 
	    {
	    	View v = convertView;
	        if (v == null) 
	        {
	        	LayoutInflater vi = (LayoutInflater)getSystemService(Context.LAYOUT_INFLATER_SERVICE);
	        	v = vi.inflate(R.layout.chatline, null);
	        }
	        ChatMessInfo f = _items.get(position);
	        if (f != null) 
	        {
	        	TextView tvna = null;
	        	TextView tvme = null;
	        	TextView tvnu = null;
    			tvna = (TextView) v.findViewById(R.id.username);
    			tvme = (TextView) v.findViewById(R.id.message);
    			tvnu = (TextView) v.findViewById(R.id.timestamp);
    			String user = f.getUserName();
        		tvna.setHorizontallyScrolling(true);
        		if (tvna != null) 
        			tvna.setText(f.getUserName() ); 
           		if (tvme != null) 
           			tvme.setText(f.getMessage() ); 
           		if (tvnu != null)
           			tvnu.setText(f.getTimeStamp());
           		RelativeLayout lo = (RelativeLayout)v.findViewById(R.id.speachballoon);
           		if (lo != null)
           			if (user == stCorespName_)
           			{
           				lo.setBackgroundResource(R.drawable.chatfromright);
           				tvnu.setPadding(0, 0, 10, 0);
          				tvna.setPadding(0, 0, 0, 0);
           				tvme.setPadding(0, 0, 0, 0);
           			}
           			else
           			{
           				lo.setBackgroundResource(R.drawable.chatfromleft);  
           				tvna.setPadding(10, 0, 0, 0);
           				tvme.setPadding(10, 0, 0, 0);
           			}
        	}
        	return v;
        }
    }
}

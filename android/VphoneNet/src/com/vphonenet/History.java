package com.vphonenet;

import java.util.ArrayList;

import com.vphonenet.Contacts.OrderAdapter;

import android.app.ListActivity;
import android.content.Context;
import android.content.res.Resources;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.TextView;
class EventInfo
{
	public static final	int RECEIVESUCCESS			= 0;
	public static final	int RECEIVEFAIL				= 1;
	public static final	int DIALESUCCESS			= 2;
	public static final	int DIALEFAIL				= 3;
	private String 			stUserName_  			= null;
	private String 			stUserID_  				= null;	
	private int 			iEventType_  			= -1;
	private byte[] 			byUserPhoto_  			= null;
	private String			stDate_					= null;
	public EventInfo(){};
	public EventInfo(String username, String userid, int event, byte[] photo, String date)
	{
		stUserName_  			= username;
		stUserID_  				= userid;	
		iEventType_  			= event;
		byUserPhoto_  			= photo;
		stDate_					= date;
	};
	String getUserName(){return stUserName_;};
	String getUserID(){return stUserID_;};
	String getDate(){return stDate_;};
	int getEventType(){return iEventType_;};
	byte[] getUserPhoto(){return byUserPhoto_;};
}
public class History extends ListActivity
{
	
	private CompoundAdapter sparceAdapter_			= null;;
	private final String[] CALLSFROM = new String[]
	{
    		"Sasson Dan",
       		"James Cooper",
       	    "Margaret",
       	    "Ariela",
      	    "Roy Levi",
       	    "Elan",
    		"Sasson Dan",
       		"James Cooper",
       	    "Margaret",
       	    "Ariela",
      	    "Roy Levi",
       	    "Elan",
    		"Sasson Dan",
       		"James Cooper",
       	    "Margaret",
       	    "Ariela",
      	    "Roy Levi",
       	    "Elan"
	};	// !!!!! delete this dummy list
	
	private final String[] CALLSDATE = new String[]
	{
		" 7:14 AM",
		" 8:32 AM",
		"10:30 AM",
		"10:45 AM",
		" 1:55 PM",
		" 2:15 PM",
		" 7:14 AM",
		" 8:32 AM",
		"10:30 AM",
		"10:45 AM",
		" 1:55 PM",
		" 2:15 PM",
		" 7:14 AM",
		" 8:32 AM",
		"10:30 AM",
		"10:45 AM",
		" 1:55 PM",
		" 2:15 PM",
	};	// !!!!! delete this dummy list
	private final int[] CALLSEVENT = new int[]
	{EventInfo.RECEIVESUCCESS, EventInfo.DIALESUCCESS, EventInfo.RECEIVEFAIL, EventInfo.RECEIVESUCCESS, EventInfo.DIALESUCCESS, EventInfo.RECEIVEFAIL, EventInfo.RECEIVEFAIL, EventInfo.DIALESUCCESS, EventInfo.DIALESUCCESS, EventInfo.RECEIVESUCCESS, EventInfo.DIALESUCCESS, EventInfo.DIALESUCCESS, EventInfo.DIALESUCCESS, EventInfo.RECEIVEFAIL, EventInfo.DIALESUCCESS, EventInfo.DIALESUCCESS, EventInfo.RECEIVESUCCESS, EventInfo.RECEIVEFAIL};	// !!!!! delete this dummy list	
	private ArrayList<ArrayList<EventInfo>> _lsHistoryList = new ArrayList<ArrayList<EventInfo>>();
	private ArrayList<OrderAdapter> _lsHistoryListAds = new ArrayList<OrderAdapter>();

	public void onCreate(Bundle savedInstanceState) 
	{
		super.onCreate(savedInstanceState);
	    requestWindowFeature(Window.FEATURE_NO_TITLE);
		setContentView(R.layout.history);
		// this is a test chunk
		int n = 0;
		int m = 0;
		for (int i = 0; i < CALLSFROM.length; i++, n++)// !!!!! delete this cycle
		{
			if (n==0)
			{
				_lsHistoryList.add( new ArrayList<EventInfo>());
				
			}
			_lsHistoryList.get(m).add(new EventInfo(CALLSFROM[i], "ID??????",CALLSEVENT[i], null, CALLSDATE[i]));
			if (n==5)
			{ 
				n = -1; 
				_lsHistoryListAds.add(new OrderAdapter(this, R.layout.historyline, _lsHistoryList.get(m++)));
			}
		} // the end of the test chunk
/*		OrderAdapter aaCurLst = new OrderAdapter
		(this, R.layout.historyline, _lsHistoryList.get(0));*
		setListAdapter(aaCurLst); 
		Resources rc = getApplicationContext().getResources();	
		TextView tv = (TextView) findViewById(R.id.title);
		tv.setText(rc.getString(R.string.history));	
*/		
		sparceAdapter_ = new CompoundAdapter(this,R.layout.historyheader);
		sparceAdapter_.addSection("Today", _lsHistoryListAds.get(0));
		sparceAdapter_.addSection("Yesterday", _lsHistoryListAds.get(1));
		sparceAdapter_.addSection("Day before yesterday", _lsHistoryListAds.get(2));
		setListAdapter(sparceAdapter_);
		
	}
	   class OrderAdapter extends ArrayAdapter<EventInfo> 
	    {
	        private ArrayList<EventInfo> _items;
	        public OrderAdapter(Context context, int textViewResourceId, ArrayList<EventInfo> items) 
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
	        		v = vi.inflate(R.layout.historyline, null);
	        	}
	        	EventInfo f = _items.get(position);
	        	if (f != null) 
	        	{
	        		TextView tvna = null;
	        		TextView tvda = null;
	    			ImageView iph = null;
	    			ImageView ist = null;
	    			tvna = (TextView) v.findViewById(R.id.username);
	    			tvda = (TextView) v.findViewById(R.id.timestamp);
	 //   			tvna.setHorizontallyScrolling(true);

	    			ist = (ImageView) v.findViewById(R.id.calltype);		
	        		if (ist != null)
	        			if (f.getEventType() == EventInfo.RECEIVESUCCESS) 
	        				ist.setImageResource(R.drawable.received);
	        			else
	        				if (f.getEventType() == EventInfo.DIALESUCCESS) 
	            				ist.setImageResource(R.drawable.outgoing);
	        				else 
	        					ist.setImageResource(R.drawable.missed);

	        		if (tvna != null) 
	        		{
	        			if (f.getEventType() == EventInfo.RECEIVEFAIL)
	        				tvna.setTextColor(Color.RED);
	        			else
	        				tvna.setTextColor(Color.BLACK);
	        			tvna.setText(f.getUserName() ); 
	        			
	        		}
	        		if (tvda != null) 
	        		{
	        			if (f.getEventType() == EventInfo.RECEIVEFAIL)
	        				tvda.setTextColor(Color.RED);
	        			else
	        				tvda.setTextColor(Color.BLACK);
	        				
	        			tvda.setText(f.getDate() ); 
	        		}
	    			iph = (ImageView) v.findViewById(R.id.arrow);
	        		if (iph != null)
	        		{
	        			if (f.getEventType() == EventInfo.RECEIVEFAIL)
	        			{
	        				iph.setImageResource(R.drawable.bluearrow);
//	        				iph.setPadding(5, 0, 5, 0);
	        			}
	        			else
	        			{
	        				iph.setImageResource(R.drawable.arrow);
//	        				iph.setPadding(14, 0, 14, 0);
	        			}

	        		}
	        	}
	        	return v;
	        }
	    }
}

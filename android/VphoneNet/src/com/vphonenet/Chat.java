package com.vphonenet;

import java.util.ArrayList;

import com.vphonenet.Contacts.OrderAdapter;

import android.app.ListActivity;
import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;
class ActivityInfo
{
	private String 		stUserName_  			= null;
	private String 		stMessage_  			= null;	
	private int 		iMessNumb_  			= -1;
	private byte[] 		byUserPhoto_  			= null;

	public ActivityInfo(){};
	public ActivityInfo(String username, String message, int number, byte[] photo)
	{
		stUserName_  			= username;
		stMessage_  			= message;	
		byUserPhoto_  			= photo;
		iMessNumb_				= number;
	};
	String getUserName(){return stUserName_;};
	String getMessage(){return stMessage_;};
	int getMessNum(){return iMessNumb_;};
	byte[] getUserPhoto(){return byUserPhoto_;};
}
public class Chat extends ListActivity
{
	private final String[] CONTACTLIST = new String[] 
	                                            	{
	                                            		"Aaron",
	                                            		"Dian Caspar",
	                                            	    "Julia Steven",
	                                            	    "Peter Smith",
	                                            	    "Sean Ladnier",
	                                            	    "John Cogan"
	                                            	};	// !!!!! delete this dummy list
	private final String[] MESSAGETLIST = new String[] 
	                                            	{
	                                            		"I'm at work",
	                                            		"not here",
	                                            	    "Hi i'm fine thanks",
	                                            	    "Whats up",
	                                            	    "Good night",
	                                            	    "I dont like it"
	                                            	};	// !!!!! delete this dummy list
	static private ArrayList<ActivityInfo> _lsActivityList = new ArrayList<ActivityInfo>();		
	public void onCreate(Bundle savedInstanceState) 
	{
		super.onCreate(savedInstanceState);
	    requestWindowFeature(Window.FEATURE_NO_TITLE);
		setContentView(R.layout.contacts);
		_lsActivityList.clear();                          
		for (int i = 0; i < CONTACTLIST.length; i++)// !!!!! delete this cycle
			if (i==3)
				_lsActivityList.add(new ActivityInfo(CONTACTLIST[i], MESSAGETLIST[i],0, null));
			else
				_lsActivityList.add(new ActivityInfo(CONTACTLIST[i], MESSAGETLIST[i],i, null));
		OrderAdapter aaCurLst = new OrderAdapter
				(this, R.layout.chatactiv, _lsActivityList);
		int y = aaCurLst.getCount();
		setListAdapter(aaCurLst); 
		TextView tv = (TextView) findViewById(R.id.title);
		Resources rc = getApplicationContext().getResources();	
//		tv.setText(rc.getString(R.string.chatacti));
	}
	
    @Override 
    protected void onListItemClick(ListView l, View v, int position, long id) 
    {// Called when user selects list screen list entry
    	 ActivityInfo ob = _lsActivityList.get(position);	
    	Intent intent = new Intent(getParent(), Chatting.class);
    	Bundle bundle = new Bundle();
    	bundle.putString("CORRESPONDENT",ob.getUserName());
    	intent.putExtras(bundle);    	
    	ActivityPack parentActivity = (ActivityPack)getParent();
 		parentActivity.startChildActivity("CHATTING", intent);
    }	
	
    class OrderAdapter extends ArrayAdapter<ActivityInfo> 
    {
        private ArrayList<ActivityInfo> _items;
        public OrderAdapter(Context context, int textViewResourceId, ArrayList<ActivityInfo> items) 
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
        		v = vi.inflate(R.layout.chatactiv, null);
        	}
        	ActivityInfo f = _items.get(position);
        	if (f != null) 
        	{
        		TextView tvna = null;
        		TextView tvme = null;
        		TextView tvnu = null;
    			ImageView iph = null;
    			ImageView ist = null;
    			tvna = (TextView) v.findViewById(R.id.username);
    			tvme = (TextView) v.findViewById(R.id.message);
    			tvnu = (TextView) v.findViewById(R.id.number);
    			int iNum = f.getMessNum();
    			if (iNum < 1) 
    				tvnu.setVisibility(View.GONE);
 
    			else
    				tvnu.setText(Integer.toString(iNum)); 
    			
    			iph = (ImageView) v.findViewById(R.id.spermatozoid);

        		byte bt[] = f.getUserPhoto();
        		if (iph != null)
        		{
        			byte b[] = f.getUserPhoto();
        			if (b != null)
        					iph.setImageBitmap(BitmapFactory.decodeByteArray(bt,0,bt.length));
        			else
        				iph.setImageResource(R.drawable.profilepicture); 
        		}
        		tvna.setHorizontallyScrolling(true);
        		if (tvna != null) 
        			tvna.setText(f.getUserName() ); 
           		if (tvme != null) 
           			tvme.setText(f.getMessage() ); 
 
        	}
        	return v;
        }
    }
}

package com.vphonenet;

import java.util.ArrayList;
import com.vphonenet.Contacts.OrderAdapter;

import android.app.ListActivity;
import android.content.Context;
import android.content.Intent;
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
class  SearchInfo
{
	private String 		stUserName_  			= null;
	private String 		stUserID_  				= null;	
	private int 		iUserStatus_  			= -1;
	private byte[] 		byUserPhoto_  			= null;
	private String		stUserMail_				= null;
	public SearchInfo(){};
	public SearchInfo(String username, String 	userid, int status, byte[] photo)
	{
		stUserName_  			= username;
		stUserID_  				= userid;	
		iUserStatus_  			= status;
		byUserPhoto_  			= photo;
	};
	String getUserName(){return stUserName_;};
	String getUserID(){return stUserID_;};
	int getStatus(){return iUserStatus_;};
	byte[] getUserPhoto(){return byUserPhoto_;};
}
public class ExternalSearch extends ListActivity
{
	private final String[] CONTACTLIST = new String[] 
	                                            	{
	                                            		"Jenny Magnoli",
	                                            		"Jenny.Magnoli",
	                                            	    "Jenny_Magnoli",
	                                            	    "Jenny_Magnoli.3D",
	                                            	    "JennyMagnoli1978",
	                                            	    "Jenny Magnoli.k"
	                                            	};	// !!!!! delete this dummy list
	private final String[] NICK = new String[] 
	                                            	{
	                                            		"JennyMagnoli",
	                                            		"Jenny.Magnolig",
	                                            	    "Jenny_Magnoli_",
	                                            	    "Jenny_Magnoli_3D",
	                                            	    "JennyMagnoli_1978",
	                                            	    "Jenny_Magnoli_k"
	                                            	};	// !!!!! delete this dummy list
	private final int[] photo = new int[] 
	{
			R.drawable.baba1,
			R.drawable.baba2,
			R.drawable.baba3,
			R.drawable.baba4,
			R.drawable.baba5,
			R.drawable.baba6,
	};	// !!!!! delete this dummy list
	static private ArrayList<SearchInfo> lsSearchList_ = new ArrayList<SearchInfo>();		
	public void onCreate(Bundle savedInstanceState) 
	{
		super.onCreate(savedInstanceState);
	    requestWindowFeature(Window.FEATURE_NO_TITLE);
		setContentView(R.layout.extsearch);
		lsSearchList_.clear();                          
		for (int i = 0; i < CONTACTLIST.length; i++)// !!!!! delete this cycle
			lsSearchList_.add(new SearchInfo(CONTACTLIST[i], NICK[i], photo[i], null));
		OrderAdapter aaCurLst = new OrderAdapter
		(this, R.layout.extsearchline, lsSearchList_);
		setListAdapter(aaCurLst); 
	}
	
	@Override 
	protected void onListItemClick(ListView l, View v, int position, long id) 
	{// Called when user selects list screen list entry
		SearchInfo ob = lsSearchList_.get(position);	
		Context ct = getParent();				
	    Intent intent = new Intent().setClass(this,  BrifProfile.class);
	    Bundle bundle = new Bundle();
	    bundle.putString("POTENTCONT",ob.getUserName());
	    intent.putExtras(bundle);    	
	    ActivityPack parentActivity = (ActivityPack)getParent();
	 	parentActivity.startChildActivity("BRIFPROFILE", intent);
	}	
	class OrderAdapter extends ArrayAdapter<SearchInfo> 
    {
        private ArrayList<SearchInfo> _items;
        public OrderAdapter(Context context, int textViewResourceId, ArrayList<SearchInfo> items) 
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
        		v = vi.inflate(R.layout.extsearchline, null);
        	}
        	SearchInfo f = _items.get(position);
        	if (f != null) 
        	{
        		TextView tvna = null;
        		TextView tvnb = null;
    			ImageView iph = null;

    			tvna = (TextView) v.findViewById(R.id.usernambold);
    			tvnb = (TextView) v.findViewById(R.id.username);
    			iph = (ImageView) v.findViewById(R.id.photo);
 
        		if (tvna != null) 
        			tvna.setText(f.getUserName() ); 
           		if (tvnb != null) 
        			tvnb.setText(f.getUserID() ); 
        		byte bt[] = f.getUserPhoto();
        		if (iph != null)
        		{
        			byte b[] = f.getUserPhoto();
        			if (b != null)
        					iph.setImageBitmap(BitmapFactory.decodeByteArray(bt,0,bt.length));
        			else
        				iph.setImageResource(f.getStatus()); 
        		}
        	}
        	return v;
        }
    }
}

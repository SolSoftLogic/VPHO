package com.vphonenet;
import java.util.ArrayList;

import android.app.ListActivity;
import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.TextView.OnEditorActionListener;

class ContactInfo
{
	private String 		stUserName_  			= null;
	private String 		stUserID_  				= null;	
	private int 		iUserStatus_  			= -1;
	private byte[] 		byUserPhoto_  			= null;
	private String		stUserMail_				= null;
	public ContactInfo(){};
	public ContactInfo(String username, String 	userid, int status, byte[] photo, String mail)
	{
		stUserName_  			= username;
		stUserID_  				= userid;	
		iUserStatus_  			= status;
		byUserPhoto_  			= photo;
		stUserMail_				= mail;
	};
	String getUserName(){return stUserName_;};
	String getUserID(){return stUserID_;};
	String getUserMail(){return stUserMail_;};
	int getStatus(){return iUserStatus_;};
	byte[] getUserPhoto(){return byUserPhoto_;};
}

public class Contacts extends ListActivity
{
	private final String[] CONTACTLIST = new String[] 
	{
		"Sasson Dan",
		"James Cooper",
	    "Margaret",
	    "Ariela",
	    "Roy Levi",
	    "Elan"
	};	// !!!!! delete this dummy list
	private Button		btExternalSearch_		= null;
	private Button		btInternalSearch_		= null;
	private boolean 	bExtSearch_				= false;	
	private EditText 	etSearch_				= null;
	static private ArrayList<ContactInfo> lsContactList_ = new ArrayList<ContactInfo>();		
	public void onCreate(Bundle savedInstanceState) 
	{
		super.onCreate(savedInstanceState);
	    requestWindowFeature(Window.FEATURE_NO_TITLE);
		setContentView(R.layout.contacts);
		lsContactList_.clear();                          
		for (int i = 0; i < CONTACTLIST.length; i++)// !!!!! delete this cycle
			lsContactList_.add(new ContactInfo(CONTACTLIST[i], "ID??????",1, null, "user@mail.changeit"));
		OrderAdapter aaCurLst = new OrderAdapter
		(this, R.layout.relativelo, lsContactList_);
		setListAdapter(aaCurLst); 

		etSearch_ = (EditText)findViewById(R.id.entertext);
		if (etSearch_ != null)
		{
			etSearch_.setImeOptions(EditorInfo.IME_ACTION_SEARCH);
			etSearch_.setOnEditorActionListener
				(
					new OnEditorActionListener() 
					{
						public boolean onEditorAction(TextView v, int actionId, KeyEvent event) 
						{				
							if (actionId == EditorInfo.IME_ACTION_SEARCH ||
									actionId == EditorInfo.IME_ACTION_NEXT || 
									actionId == EditorInfo.IME_ACTION_DONE) 
							{
								if (bExtSearch_)
									doExternalSearch();
								else
									doInternalSearch();
								bExtSearch_ = false;
								InputMethodManager imm = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
								imm.hideSoftInputFromWindow(etSearch_.getWindowToken(), 0);
								return true;
							}
							return false;			
						}
					}
				);
		}
		
		btExternalSearch_ = (Button) findViewById(R.id.btwithcross);
		if (btExternalSearch_ != null)
			btExternalSearch_.setOnClickListener
			(
				new View.OnClickListener() 
				{
					public void onClick(View v) 
					{	
						etSearch_.setImeOptions(EditorInfo.IME_ACTION_SEARCH);
						InputMethodManager mgr = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
						mgr.toggleSoftInput(0, 0);
						mgr.showSoftInput(etSearch_, 0);	
						bExtSearch_ = true;
					}
				}
			);

		btInternalSearch_ = (Button) findViewById(R.id.search);
		if (btInternalSearch_ != null)
			btInternalSearch_.setOnClickListener(new View.OnClickListener() 
			{
				public void onClick(View v) 
				{		
					bExtSearch_ = false;
		   	}
		});		
	}
	
    @Override
	public void onResume()
	{
		super.onResume();
		LinearLayout ll = (LinearLayout)findViewById(R.id.entertextlo);
		ll.setVisibility(View.VISIBLE);
	    InputMethodManager mgr = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
	    mgr.toggleSoftInput(0, 0);
	    TextView tvsearch = (TextView) findViewById(R.id.entertext);
	    if (mgr != null) 
	    	mgr.hideSoftInputFromWindow(tvsearch.getWindowToken(), 0);
	}
    
    @Override 
    protected void onListItemClick(ListView l, View v, int position, long id) 
    {// Called when user selects list screen list entry
    	ContactInfo ob = lsContactList_.get(position);	
    	Intent intent = new Intent(getParent(), WhatToDo.class);
    	Bundle bundle = new Bundle();
    	bundle.putString("USERNAME",ob.getUserName());
    	bundle.putString("USERID",ob.getUserID());
    	bundle.putString("USERMAIL",ob.getUserMail());
    	bundle.putInt("USERSTATUS",ob.getStatus());
    	bundle.putByteArray("USERPORTRITE",ob.getUserPhoto());
    	intent.putExtras(bundle);    	
    	ActivityPack parentActivity = (ActivityPack)getParent();
 		parentActivity.startChildActivity("WHATTODO", intent);
    }
    
    @Override
    public boolean dispatchKeyEvent(KeyEvent event) 
    {
    	int i = event.getKeyCode();
        if (event.getAction() == KeyEvent.ACTION_DOWN) 
        {
        	bExtSearch_ = false;
        	LinearLayout ll = (LinearLayout) findViewById(R.id.entertextlo);
        	if (ll != null)
        		if (ll.getVisibility() == View.GONE)
        		{
        			populate();
   		        	ll.setVisibility(View.VISIBLE);
        		        
   		        	ll = (LinearLayout) findViewById(R.id.titlelo);
        		    if (ll != null)
        		       	ll.setBackgroundResource(R.drawable.contactlist);
        		        
        		    Button bt = (Button) findViewById(R.id.btwithcross);
        		    if (bt != null)
        		    	bt.setVisibility(View.VISIBLE);
        		    return true;
        		}
        }
        return super.dispatchKeyEvent(event);
    }
    
    private void doExternalSearch()
    {
		Intent in = new Intent().setClass(this, ExternalSearch.class);
		ActivityPack parentActivity = (ActivityPack)getParent();
		parentActivity.startChildActivity("EXTERNALSEARCH", in);
//		startActivity(in);
    }
    
    private void doInternalSearch()
    {
    	populateSearch();  	
        LinearLayout ll = (LinearLayout) findViewById(R.id.entertextlo);
        if (ll != null)
        	ll.setVisibility(View.GONE);
        
        ll = (LinearLayout) findViewById(R.id.titlelo);
        if (ll != null)
        	ll.setBackgroundResource(R.drawable.searchdir);
        
    	Button bt = (Button) findViewById(R.id.btwithcross);
    	if (bt != null)
    		bt.setVisibility(View.GONE);
    }
    
    private void populateSearch()
    {
    	lsContactList_.clear();
    	OrderAdapter aaCurLst = (OrderAdapter)this.getListAdapter();
    	for (int i = 0; i < CONTACTLIST.length-4; i++)// !!!!! delete this cycle
			lsContactList_.add(new ContactInfo(CONTACTLIST[i], "ID??????",1, null, "user@mail.changehim"));
    	aaCurLst.notifyDataSetChanged();
    }
    
    private void populate()
    {
    	lsContactList_.clear();
    	OrderAdapter aaCurLst = (OrderAdapter)this.getListAdapter();
    	for (int i = 0; i < CONTACTLIST.length; i++)// !!!!! delete this cycle
			lsContactList_.add(new ContactInfo(CONTACTLIST[i], "ID??????",1, null, "user@mail.changehim"));
    	aaCurLst.notifyDataSetChanged();
    }
    
    class OrderAdapter extends ArrayAdapter<ContactInfo> 
    {
        private ArrayList<ContactInfo> _items;
        public OrderAdapter(Context context, int textViewResourceId, ArrayList<ContactInfo> items) 
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
        		v = vi.inflate(R.layout.relativelo, null);
        	}
        	ContactInfo f = _items.get(position);
        	if (f != null) 
        	{
        		TextView tvna = null;
    			ImageView iph = null;
    			ImageView ist = null;
    			tvna = (TextView) v.findViewById(R.id.username);
 //   			tvna.setHorizontallyScrolling(true);
    			iph = (ImageView) v.findViewById(R.id.prof);
    			ist = (ImageView) v.findViewById(R.id.status);		
        		if (ist != null)
        			if (f.getStatus()==0) 
        				ist.setImageResource(R.drawable.statusred);
        			else
        				if (f.getStatus()==1) 
            				ist.setImageResource(R.drawable.statusgreen);
        				else 
        					ist.setImageResource(R.drawable.statusyellow);

        		if (tvna != null) 
        			tvna.setText(f.getUserName() ); 
        		byte bt[] = f.getUserPhoto();
        		if (iph != null)
        		{
        			byte b[] = f.getUserPhoto();
        			if (b != null)
        					iph.setImageBitmap(BitmapFactory.decodeByteArray(bt,0,bt.length));
        			else
        				iph.setImageResource(R.drawable.profilepicture); 
        		}
        	}
        	return v;
        }
    }
}

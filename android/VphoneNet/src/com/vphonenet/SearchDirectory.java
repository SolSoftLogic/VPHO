package com.vphonenet;

import android.app.Activity;
import android.app.Dialog;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;

public class SearchDirectory extends Activity
{
	private final int 	INVITE_DIALOG 			= 3;
	private Button 		btSearch_ 				= null;
	private Button 		btInvite_ 				= null;
	public void onCreate(Bundle savedInstanceState) 
	{
		super.onCreate(savedInstanceState);
	    requestWindowFeature(Window.FEATURE_NO_TITLE);
	    setContentView(R.layout.searchdire);
		btSearch_ = (Button) findViewById(R.id.btsearch);
		String stSearcData = null;
		Bundle bundle = getIntent().getExtras();
		if (bundle != null)
			stSearcData = bundle.getString("SEARCHRNAME");
		if (stSearcData != null)
		{
			TextView tv = (TextView)findViewById(R.id.pleyouza);
			if (tv != null)
				tv.setVisibility(View.GONE);
			LinearLayout lv = (LinearLayout)findViewById(R.id.buttona);
			if (lv != null)
				lv.setVisibility(View.GONE);
		}
		if (btSearch_ != null)
			btSearch_.setOnClickListener
			(
				new View.OnClickListener() 
				{
					public void onClick(View v) 
					{	
					   	Intent intent = new Intent(getParent(), SearchDirLast.class);
					   	ActivityPack parentActivity = (ActivityPack)getParent();
				 		parentActivity.startChildActivity("SEARCDIRLAST", intent);
					}
					
				}
			);
		btInvite_ = (Button) findViewById(R.id.btnvite);
		if (btInvite_ != null)
			btInvite_.setOnClickListener
			(
				new View.OnClickListener() 
				{
					public void onClick(View v) 
					{
						revealDeal();	
					}
				}
			);
	}
	
	private void revealDeal()
	{
		Dialog dialog = new Dialog(this);
		dialog.setContentView(R.layout.invitedlg);
		dialog.setCancelable(true);
/*		Button button = (Button) dialog.findViewById(R.id.btsendinv);
		button.setOnClickListener
		(
			new OnClickListener() 
			{
				public void onClick(View v) 
				{
					finish();
				}
			}
		);*/
		dialog.show();		
	}

}

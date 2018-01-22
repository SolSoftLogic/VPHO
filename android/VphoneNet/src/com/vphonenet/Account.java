package com.vphonenet;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.View;
import android.view.Window;
import android.widget.LinearLayout;

public class Account extends Activity 
{
	LinearLayout ll_ = null;
	public void onCreate(Bundle savedInstanceState) 
	{
		super.onCreate(savedInstanceState);
	    requestWindowFeature(Window.FEATURE_NO_TITLE);
		setContentView(R.layout.account);

		ll_ = (LinearLayout) findViewById(R.id.editprofilel);
	
		ll_.setOnTouchListener
		(
			new View.OnTouchListener() 
			{
//				@Override
				public boolean onTouch(View v, MotionEvent event) 
				{
			    	Intent intent = new Intent(getParent(), BrifProfile.class);
				   	ActivityPack parentActivity = (ActivityPack)getParent();
			 		parentActivity.startChildActivity("WHATTODO", intent);

					return false;
	
				}
			}
		);
	}
}

package com.vphonenet;

import android.app.Activity;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.View;
import android.view.Window;
import android.view.inputmethod.EditorInfo;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.TextView.OnEditorActionListener;

public class WhatToDo extends Activity
{
	private Button 					btVoiceCall_ 						= null;
	private Button 					btVideoCall_ 						= null;
	private Button 					btChat_ 							= null;
	private Button 					btShareFiles_ 						= null;
	private Button 					btViewProf_ 						= null;
	public void onCreate(Bundle savedInstanceState) 
	{
		super.onCreate(savedInstanceState);
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		setContentView(R.layout.whattodo);
		Bundle bundle = getIntent().getExtras();
		String userName = null;
		String userID = null;
		byte[] userPhoto = null;
		String userMail = null;
		int userStatus = -1;
		if (bundle != null)
		{
			/* you can replace next 5 lines with Native methods  */
			userName = bundle.getString("USERNAME");
			userID = bundle.getString("USERID");
			userMail = bundle.getString("USERMAIL");
			userStatus = bundle.getInt("USERSTATUS");
			userPhoto = bundle.getByteArray("USERPORTRITE");
			TextView tvTitle = (TextView) findViewById(R.id.title);
			tvTitle.setText(userName);
			TextView tvName = (TextView) findViewById(R.id.username);
			tvName.setText(userName);
			TextView tvMail = (TextView) findViewById(R.id.usermail);
			tvMail.setText(userMail);
			TextView tvStatus = (TextView) findViewById(R.id.stausval);
			ImageView ivStatus = (ImageView) findViewById(R.id.status_image);
			ImageView ivPortrait = (ImageView) findViewById(R.id.portrait);
			if (userStatus == 0)
			{
				ivStatus.setImageResource(R.drawable.statusred);
				tvStatus.setText("offline");
			}
			else
				if (userStatus == 1)
				{
					ivStatus.setImageResource(R.drawable.statusgreen);
					tvStatus.setText("online");
				}
				else
				{
					ivStatus.setImageResource(R.drawable.statusyellow);
					tvStatus.setText("away");
				}
			if (userPhoto != null)
			{
				ivPortrait.setImageBitmap(BitmapFactory.decodeByteArray(userPhoto,0,userPhoto.length));
			}
/*			else
			{
				ivPortrait.setImageResource(R.drawable.profilepicture); 
			}*/
		}

		btVideoCall_ = (Button) findViewById(R.id.videocall);
		btVideoCall_.setOnClickListener(new View.OnClickListener() 
	    {
	    	public void onClick(View v) 
	    	{

	    	}
		});
		
		btVoiceCall_ = (Button) findViewById(R.id.voicecall);
		btVoiceCall_.setOnClickListener(new View.OnClickListener() 
	    {
	    	public void onClick(View v) 
	    	{
	    	
	    	}
		});
		
		btChat_ = (Button) findViewById(R.id.chat);
		btChat_.setOnClickListener(new View.OnClickListener() 
	    {
	    	public void onClick(View v) 
	    	{
	    		ActiveFraim.changeTab(1);
	    	}
		});
		
		btShareFiles_ = (Button) findViewById(R.id.sharefile);
		btShareFiles_.setOnClickListener(new View.OnClickListener() 
	    {
	    	public void onClick(View v) 
	    	{
	    	
	    	}
		});
		
		btViewProf_ = (Button) findViewById(R.id.viefullprofile);
		btViewProf_.setOnClickListener(new View.OnClickListener() 
	    {
	    	public void onClick(View v) 
	    	{
	    	
	    	}
		});
	}
}

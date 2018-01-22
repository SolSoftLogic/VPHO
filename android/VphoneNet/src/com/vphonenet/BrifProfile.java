package com.vphonenet;

import android.app.Activity;
import android.content.Intent;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
class BrifProfInfo
{
	private String 		stUserName_  			= null;
	private String 		stUserID_  				= null;	
	private int 		iUserStatus_  			= -1;
	private byte[] 		byUserPhoto_  			= null;
	private String		stUserMail_				= null;
	private String		stCountry_				= null;
	private String		stState_					= null;
	private String		stBirthday_				= null;
	private String		stGender_				= null;
	public BrifProfInfo(){};
	public BrifProfInfo(String username, 
						String 	userid, 
						int status, 
						byte[] photo, 
						String mail,
						String 	country, 
						String state, 
						String birthday, 
						String gender
						)
	{
		stUserName_  			= username;
		stUserID_  				= userid;	
		iUserStatus_  			= status;
		byUserPhoto_  			= photo;
		stUserMail_				= mail;
		stCountry_				= country;
		stState_				= state;
		stBirthday_				= birthday;
		stGender_				= gender;
		
	};
	String getUserName(){return stUserName_;};
	String getUserID(){return stUserID_;};
	String getUserMail(){return stUserMail_;};
	int getStatus(){return iUserStatus_;};
	byte[] getUserPhoto(){return byUserPhoto_;};
	String getCountry(){return stCountry_;}
	String getState(){return stState_;}
	String getBirthDay(){return stBirthday_;}
	String getGender(){return stGender_;}
}
public class BrifProfile extends Activity
{
	private String stCorespName_ = null;
	public void onCreate(Bundle savedInstanceState) 
	{
		super.onCreate(savedInstanceState);
	    requestWindowFeature(Window.FEATURE_NO_TITLE);
		setContentView(R.layout.brifprofile);
		Bundle bundle = getIntent().getExtras();
		if (bundle != null)
		{
			stCorespName_ = bundle.getString("POTENTCONT");
			TextView tvTitle = (TextView) findViewById(R.id.title);
			if (tvTitle != null)
				tvTitle.setText(stCorespName_);
			BrifProfInfo brpro = getBrifProfile(stCorespName_);
			
			TextView tvName = (TextView) findViewById(R.id.username);
			if (tvName != null)
				tvName.setText(stCorespName_);
			TextView tvMail = (TextView) findViewById(R.id.usermail);
			if (tvMail != null)
				tvMail.setText(brpro.getUserMail());
//			TextView tvStatus = (TextView) findViewById(R.id.stausval);
			
//			ImageView ivStatus = (ImageView) findViewById(R.id.status_image);
//			if (ivStatus != null)
//				ivStatus.setImageResource(R.drawable.statusred);
				
			ImageView ivPortrait = (ImageView) findViewById(R.id.portrait);	
			byte[] userPhoto = brpro.getUserPhoto();
			if (ivPortrait != null)	
				if (userPhoto != null)
					ivPortrait.setImageBitmap(BitmapFactory.decodeByteArray(userPhoto,0,userPhoto.length));
				else
					ivPortrait.setImageResource(R.drawable.baba4);
			
			tvName = (TextView) findViewById(R.id.country);
			if (tvName != null)
				tvName.setText(brpro.getCountry());
			
			tvName = (TextView) findViewById(R.id.state);
			if (tvName != null)
				tvName.setText(brpro.getState());
			
			tvName = (TextView) findViewById(R.id.Birthday);
			if (tvName != null)
				tvName.setText(brpro.getBirthDay());
			
			tvName = (TextView) findViewById(R.id.gender);
			if (tvName != null)
				tvName.setText(brpro.getGender());
			
			 Button btRequest_ = (Button) findViewById(R.id.addcontact);
			 if (btRequest_ != null)
				btRequest_.setOnClickListener
				(
					new View.OnClickListener() 
					{
						public void onClick(View v) 
						{	
							invoceRequest();
						}
					}
				);
		}
		
	}
	private BrifProfInfo getBrifProfile(String stUser)
	{
		return new BrifProfInfo("Jenny Magnoly",
				"JennyMagnolyID",
				1,
				null,
				"jennymagnoly@vphonet.com",
				"Germany",
				"Saarland",
				"17 June 1917",
				"Feemale");
	}
	private void invoceRequest()
	{
		Intent in = new Intent().setClass(this, SendRequest.class);
		ActivityPack parentActivity = (ActivityPack)getParent();
		parentActivity.startChildActivity("SENDREQUEST", in);
	}
}

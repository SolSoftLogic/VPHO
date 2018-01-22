package com.vphonenet;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.graphics.Color;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.View;
import android.view.Window;
import android.view.inputmethod.EditorInfo;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.TextView.OnEditorActionListener;


public class Registration extends Activity 
{
	static private final int 		WRONG_USER_NAME					= 2;
	static private final int 		SILENT_TO_DIADN					= 3;
	private EditText 				etUi_							= null;
	private EditText 				etPa_ 							= null;
	private EditText 				etEm_ 							= null;
	private EditText 				etCf_ 							= null;
	private EditText 				etFiNa_							= null;
	private EditText 				etLaNa_ 						= null;
	private Button 					btDone_ 						= null;
	private String					stFirstName_					= null;
	private String					stLastName_						= null;
	private String					stUserName_						= null;
	private String					stPassword_						= null;
	private String					stEMail_						= null;
	private String					stDiagno_						= null;

	public void onCreate(Bundle savedInstanceState) 
	{
		super.onCreate(savedInstanceState);
	    requestWindowFeature(Window.FEATURE_NO_TITLE);
		setContentView(R.layout.register);
		Context ct = getApplicationContext();
		Resources rc = ct.getResources();
		stDiagno_ = "";
		etUi_ = (EditText) findViewById(R.id.UserID);
		etPa_ = (EditText) findViewById(R.id.Password);	
		etCf_ = (EditText) findViewById(R.id.Confirmation);
		etFiNa_ = (EditText) findViewById(R.id.firstname);
		etLaNa_ = (EditText) findViewById(R.id.lastname);
		etEm_ = (EditText) findViewById(R.id.email);
		etUi_.setOnEditorActionListener(
				new OnEditorActionListener() 
				{
					public boolean onEditorAction(TextView v, int actionId, KeyEvent event) 
					{
						if (actionId == EditorInfo.IME_ACTION_SEARCH ||
							actionId == EditorInfo.IME_ACTION_NEXT || 
							actionId == EditorInfo.IME_ACTION_DONE) 
							return !checkUserId(WRONG_USER_NAME);
						return true;
					}
				});
		
		etPa_.setOnEditorActionListener(
				new OnEditorActionListener() 
				{
					public boolean onEditorAction(TextView v, int actionId, KeyEvent event) 
					{
						if (actionId == EditorInfo.IME_ACTION_SEARCH ||
								actionId == EditorInfo.IME_ACTION_NEXT || 
								actionId == EditorInfo.IME_ACTION_DONE ) 
								return !checkPassword(WRONG_USER_NAME);
						return true;
					}
				});
		etCf_.setOnEditorActionListener(
				new OnEditorActionListener() 
				{
					public boolean onEditorAction(TextView v, int actionId, KeyEvent event) 
					{
						if (actionId == EditorInfo.IME_ACTION_SEARCH ||
								actionId == EditorInfo.IME_ACTION_NEXT || 
								actionId == EditorInfo.IME_ACTION_DONE) 
								return !checkConfirm(WRONG_USER_NAME);
						return true;
					}
				});

		etFiNa_.setOnEditorActionListener(
				new OnEditorActionListener() 
				{
					public boolean onEditorAction(TextView v, int actionId, KeyEvent event) 
					{		
						if (actionId == EditorInfo.IME_ACTION_SEARCH ||
								actionId == EditorInfo.IME_ACTION_NEXT || 
								actionId == EditorInfo.IME_ACTION_DONE) 
								return !checkFirstNa(WRONG_USER_NAME);
						return true;
					}
				});
		
		etLaNa_.setOnEditorActionListener(
				new OnEditorActionListener() 
				{
					public boolean onEditorAction(TextView v, int actionId, KeyEvent event) 
					{				
						if (actionId == EditorInfo.IME_ACTION_SEARCH ||
								actionId == EditorInfo.IME_ACTION_NEXT || 
								actionId == EditorInfo.IME_ACTION_DONE) 
							return !checkLastNa(WRONG_USER_NAME);
						return true;			
					}
				});
		
		etEm_.setOnEditorActionListener(
				new OnEditorActionListener() 
				{
					public boolean onEditorAction(TextView v, int actionId, KeyEvent event) 
					{		
						if (actionId == EditorInfo.IME_ACTION_SEARCH ||
								actionId == EditorInfo.IME_ACTION_NEXT || 
								actionId == EditorInfo.IME_ACTION_DONE) 
								return !checkEmail(WRONG_USER_NAME);
							return true;
					}
				});

		btDone_ = (Button) findViewById(R.id.done);
		
		btDone_.setOnClickListener(new View.OnClickListener() 
	    {
	    	public void onClick(View v) 
	    	{	
	    		stDiagno_ = "";
				boolean a = checkUserId(SILENT_TO_DIADN);
				boolean b = checkPassword(SILENT_TO_DIADN);
				boolean c = checkConfirm(SILENT_TO_DIADN);
				boolean d = checkFirstNa(SILENT_TO_DIADN);
				boolean e = checkLastNa(SILENT_TO_DIADN);
				boolean f = checkEmail(SILENT_TO_DIADN);    		
				if ( !a || !b || !c || !d || !e || !f )
				{
					Resources rc = getApplicationContext().getResources();	
					stDiagno_ = rc.getString(R.string.folerda) + stDiagno_;
					showMyDialog(WRONG_USER_NAME, stDiagno_);
				}
				else
					changeScreen();
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
			case R.id.UserID:
				bOut = checkUserId(iMood);
				break;
			case R.id.Password:
				bOut = checkPassword(iMood);
				break;
			case R.id.Confirmation:
				bOut = checkConfirm(iMood);
				break;
			case R.id.firstname:
				bOut = checkFirstNa(iMood);
				break;
			case R.id.lastname:
				bOut = checkLastNa(iMood);
				break;
			case R.id.email:
				bOut = checkEmail(iMood);
				break;
		}
		return bOut;
	}
	
	private boolean checkUserId(int iMood)
	{
		boolean bOut = true;
		String st = etUi_.getText().toString();
		Resources rc = getApplicationContext().getResources();
		String stText = rc.getString(R.string.UserID);
		if (st == null)
		{
			stText = stText + " "+ rc.getString(R.string.paramit);
			stText = stText + " "+ rc.getString(R.string.and);
			stText = stText + " "+ rc.getString(R.string.muststar);
			showMyDialog(iMood, stText);	
			bOut = false;
		}
		else
			if (st.length() < 6)
			{
				stText = stText + " " + rc.getString(R.string.paramit);
				if (st.length() > 0)
					if (!Character.isLetter(st.charAt(0)))
					{
						stText = stText + " "+ rc.getString(R.string.and);
						stText = stText + " "+ rc.getString(R.string.muststar);
					}
				showMyDialog(iMood, stText);
				bOut = false;
			}
			else
			{
				boolean b = false;
				if (!Character.isLetter(st.charAt(0)))
				{
					stText = stText + " " + rc.getString(R.string.muststar);
					bOut = false;
					b = true;
				}
				for (int i = 1; i < st.length(); i++)
					if (!Character.isLetter(st.charAt(i)) && !Character.isDigit(st.charAt(i)) )
						if (st.charAt(i) != '-' && st.charAt(i) != '_' && st.charAt(i) != '.')
						{
							if (b)
								stText = stText + " "+ rc.getString(R.string.and);
							stText = stText + " "+ rc.getString(R.string.conletdi);
							bOut = false;
						}
				if (!bOut)
					showMyDialog(iMood, stText);	
			}
		
		TextView tv = (TextView) findViewById(R.id.tvUserID);
		if (bOut)
			tv.setTextColor(Color.BLACK);
		else
			tv.setTextColor(Color.RED);
		return bOut;
	}

	private boolean checkPassword(int iMood)
	{
		boolean bOut = true;	
		String st = etPa_.getText().toString();
		Resources rc = getApplicationContext().getResources();
		String stText = rc.getString(R.string.Password);
		if (st == null)
		{
			stText = stText + " "+ rc.getString(R.string.paramit);
			showMyDialog(iMood, stText);	
			bOut = false;
		}
		else
			if (st.length() < 6)
			{
				stText = stText + " "+ rc.getString(R.string.paramit);
				showMyDialog(iMood, stText);
				bOut = false;
			}
		
		TextView tv = (TextView) findViewById(R.id.tvPassword);
		if (bOut)
			tv.setTextColor(Color.BLACK);
		else
			tv.setTextColor(Color.RED);
		return bOut;
	}

	private boolean checkConfirm(int iMood)
	{
		boolean bOut = true;	
		String st = etCf_.getText().toString();
		Resources rc = getApplicationContext().getResources();
		String stText = rc.getString(R.string.confirm);
		if (st == null)
		{
			stText = stText + " "+ rc.getString(R.string.padoma);
			showMyDialog(iMood, stText);	
			bOut = false;
		}
		else
			if (st.length() < 6)
			{
				stText = stText + " "+ rc.getString(R.string.paramit);
				showMyDialog(iMood, stText);
				bOut = false;
			}
			else
			{
				String ct = etPa_.getText().toString();
				if (!st.equals(ct))
				{
					stText = stText + " "+ rc.getString(R.string.padoma);
					showMyDialog(iMood, stText);
					bOut = false;
				}
			}	
		TextView tv = (TextView) findViewById(R.id.tvConfirm);
		if (bOut)
			tv.setTextColor(Color.BLACK);
		else
			tv.setTextColor(Color.RED);
		return bOut;
	}
	
	private boolean checkEmail(int iMood)
	{
		boolean bOut = true;	
		String st = etEm_.getText().toString();
		Resources rc = getApplicationContext().getResources();
		String stText = rc.getString(R.string.email);
		if (st == null)
		{
			stText = stText + " "+ rc.getString(R.string.bemal);
			showMyDialog(iMood, stText);
			bOut = false;
		}
		else
		{
			st = st.trim();
			int iDot = st.indexOf('.');
			if (iDot < 0)
			{
					stText = stText + " "+ rc.getString(R.string.bemal);
					showMyDialog(iMood, stText);	
					bOut = false;
			}
			else
				if (iDot == 0 || iDot == st.length()-1 || st.charAt(st.length()-1) == '.')
				{
					stText = stText + " "+ rc.getString(R.string.bemal);
					showMyDialog(iMood, stText);
					bOut = false;
				}
			if (bOut)
			{
				int i = st.indexOf('@');
				if (i == 0 || i == st.length()-1 )
				{
					stText = stText + " "+ rc.getString(R.string.bemal);
					showMyDialog(iMood, stText);	
					bOut = false;
				}
			}
		}	
		TextView tv = (TextView) findViewById(R.id.tvEmail);
		if (bOut)
			tv.setTextColor(Color.BLACK);
		else
			tv.setTextColor(Color.RED);
		return bOut;
	}
	
	private boolean checkFirstNa(int iMood)
	{
		boolean bOut = true;	
		String st = etFiNa_.getText().toString();
		Resources rc = getApplicationContext().getResources();
		String stText = rc.getString(R.string.firstName);
		if (st == null)
		{
			stText = stText + " "+ rc.getString(R.string.wfna);
			showMyDialog(iMood, stText);	
			bOut = false;
		}
		else
			if (st.length() > 0)
			{
				for (int i = 0; i < st.length(); i++)
					if (!Character.isLetter(st.charAt(i)))
					{
						stText = stText + " "+ rc.getString(R.string.wfna);
						showMyDialog(iMood, stText);	
						bOut = false;
						break;
					}
			}
			else
			{
				stText = stText + " "+ rc.getString(R.string.wfna);
				showMyDialog(iMood, stText);	
				bOut = false;
			}
		TextView tv = (TextView) findViewById(R.id.tvFirstName);
		if (bOut)
			tv.setTextColor(Color.BLACK);
		else
			tv.setTextColor(Color.RED);				
		return bOut;
	}
	
	private boolean checkLastNa(int iMood)
	{
		boolean bOut = true;	
		String st = etLaNa_.getText().toString();
		Resources rc = getApplicationContext().getResources();
		String stText = rc.getString(R.string.lastName);
		if (st == null)
		{
			stText = stText + " "+ rc.getString(R.string.wrln);
			showMyDialog(iMood, stText);
			bOut = false;
		}
		else
			if (st.length() > 0)
			{
				for (int i = 0; i < st.length(); i++)
					if (!Character.isLetter(st.charAt(i)))
					{
						stText = stText + " "+ rc.getString(R.string.wrln);
						showMyDialog(iMood, stText);
						bOut = false;
						break;
					}
			}		
			else
			{
				stText = stText + " "+ rc.getString(R.string.wrln);
				showMyDialog(iMood, stText);
				bOut = false;
			}
		TextView tv = (TextView) findViewById(R.id.tvLastName);
		if (bOut)
			tv.setTextColor(Color.BLACK);
		else
			tv.setTextColor(Color.RED);		
		return bOut;
	}
	
	private void changeScreen()
	{
    	Intent in = new Intent().setClass(this, Registrat2.class);
		startActivity(in);
//		finish();
	}
	void makeRegistration()
	{
		if ( NativeLib.svpRegister(stUserName_) == 0) // check it !!!!!!!!!!!!!!!!!!!
		{
			Intent in = new Intent().setClass(this, Contacts.class);
			startActivity(in);
			finish();			
		}
	}
}

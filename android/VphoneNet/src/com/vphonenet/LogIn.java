package com.vphonenet;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.Resources;
import android.os.AsyncTask;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.text.Html;
import android.view.KeyEvent;
import android.view.View;
import android.view.Window;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.TextView.OnEditorActionListener;

public class LogIn  extends Activity
{
//	private BGThread 					bgThread_ 				= null;
	static private final int 			PROGRESS_DIALOG 		= 0;
	static private final int 			WRONG_USER 				= 1;
	static private final int 			WRONG_PASS 				= 2;
	private ProgressDialog 				pdLoad_					= null;
	private Button 						btOK_ 					= null;
	private Button 						btRegistr_ 				= null;
	private Button 						btRemember_ 			= null;
	private static Prefs 				prPrefs_ 				= null;
	private EditText 					etLogin_				= null;
	static Prefs 						getPrefds()				{return prPrefs_;};
	static String						stUserName_				= null;
	static String						stPassWord_				= null;

	public void onCreate(Bundle savedInstanceState) 
	{
		super.onCreate(savedInstanceState);
	    requestWindowFeature(Window.FEATURE_NO_TITLE);
	    setContentView(R.layout.passwordentry);
	    System.loadLibrary("vphonenet");  
	    prPrefs_ = new Prefs(PreferenceManager.getDefaultSharedPreferences(this));
	    btOK_ = (Button) findViewById(R.id.OK); 
	    btRegistr_ = (Button) findViewById(R.id.Redistration); 
	    btRemember_ = (Button) findViewById(R.id.remember); 
		Context ct = getApplicationContext();
		Resources rc = ct.getResources();
	    String st = rc.getString(R.string.cantreme) + "<br>" + "<u><font color=blue>" + rc.getString(R.string.clickhre) + "</font></u>";
	    btRemember_.setText(Html.fromHtml(st));
	    etLogin_ = (EditText)findViewById(R.id.UserID);
/*	    btOK_.setOnClickListener(new View.OnClickListener() 
		{
			public void onClick(View v) 
			{
			    InputMethodManager mgr = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
//			    mgr.toggleSoftInput(0, 0);
			    mgr.hideSoftInputFromWindow(etLogin_.getWindowToken(), 0);
//			    mgr.showSoftInput(etLogin_, 0);
				CheckEntry();
			}
		});*/
	    
	    etLogin_ = (EditText)findViewById(R.id.entertext);
		if (etLogin_ != null)
		{
//			etLogin_.setImeOptions(EditorInfo.IME_ACTION_SEARCH);
			etLogin_.setOnEditorActionListener
				(
					new OnEditorActionListener() 
					{
						public boolean onEditorAction(TextView v, int actionId, KeyEvent event) 
						{				
							if (actionId == EditorInfo.IME_ACTION_SEARCH ||
									actionId == EditorInfo.IME_ACTION_NEXT || 
									actionId == EditorInfo.IME_ACTION_DONE) 
							{

								InputMethodManager imm = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
								imm.hideSoftInputFromWindow(etLogin_.getWindowToken(), 0);
								CheckEntry();
								return true;
							}
							return false;			
						}
					}
				);
		}
		

	    
	    btRegistr_.setOnClickListener(new View.OnClickListener() 
		{
			public void onClick(View v) 
			{
				Registration();
			}
		});
	    
	    btRemember_.setOnClickListener(new View.OnClickListener() 
		{
			public void onClick(View v) 
			{
				 Recall();
			}
		});
	}
    @Override
	public void onResume()
	{
		super.onResume();
		//http://developer.android.com/intl/zh-TW/resources/articles/on-screen-inputs.html	
		//This code (3 next lines does not works here)
		// To initiate appearence of soft keyboard when activity starts its need to
		// request this action in the manifest for respective activity with following tag
		// android:windowSoftInputMode="stateVisible|adjustResize"
	    InputMethodManager mgr = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
	    mgr.toggleSoftInput(0, 0);
	    mgr.showSoftInput(etLogin_, 0);

	}
    
	private  void showMyDialog(String st)
	{
		Context ct = getApplicationContext();
		Resources rc = ct.getResources();
		AlertDialog.Builder dlg=new AlertDialog.Builder(this);
		String sta = null;

		dlg.setTitle(rc.getString(R.string.error));
        dlg.setMessage(st);
        dlg.show();		
	}
    
    private void completReg(int iResult)
    {
		Context ct = getApplicationContext();
		Resources rc = ct.getResources();
		if (iResult == 0) // проверь это !!!!!!!!!!!!!!!!!!!!!!!
			VPhoneNet();
		else
		{
			AlertDialog dialog = new AlertDialog.Builder(this).create();
			dialog.setMessage(rc.getText(R.string.usernotf));
			dialog.setButton(DialogInterface.BUTTON_POSITIVE, rc.getText(R.string.cancel),
					new DialogInterface.OnClickListener() 
					{
						public void onClick(DialogInterface dialog, int which) 
						{
//							start_SaveSettingsActivity();
						}
					});
			dialog.setButton(DialogInterface.BUTTON_NEGATIVE, rc.getText(R.string.Redistration),
			new DialogInterface.OnClickListener() 
			{
				public void onClick(DialogInterface dialog, int which) 
				{	
					Registration();
				}
			});                               
			dialog.show();			
		}
			
		Toast.makeText(getApplicationContext(), 
				rc.getText(R.string.wrongreg), 
				Toast.LENGTH_SHORT).show();		    	
    }
	private void CheckEntry()
	{
		EditText etPassword = (EditText)findViewById(R.id.Password);
		EditText etLogin = (EditText)findViewById(R.id.UserID); 
		String login = etLogin.getText().toString();
		String password = etPassword.getText().toString();
		Boolean bL = true;
		Boolean bP = true;
		if (login == null )
			bL = false;
		else
			if (login.length() < 1)
				bL = false;

		if (password == null )
			bP = false;
		else
			if (password.length() < 1)
					bP = false;
		Context ct = getApplicationContext();
		Resources rc = ct.getResources();
		
		if (bL && bP)	
		{
			stUserName_	= login;
			stPassWord_		= password;
			this.showDialog(PROGRESS_DIALOG);	
		}
		else
		{
			String st = (String)rc.getText(R.string.fill) + " ";
			int i = 0;
			if (!bL)
			{
				 st =  st + rc.getText(R.string.UserID);
				 i++;
			}
			if (!bP)
			{
				if (i > 0)
					st =  st + " " + rc.getText(R.string.and) + " ";
				 st =  st + rc.getText(R.string.Password);
				 i++;
			}		  
			if (i > 1)
				st = st + " " + rc.getText(R.string.fields);
			else
				st = st + " " + rc.getText(R.string.field);
			showMyDialog(st);
			Toast.makeText(getApplicationContext(), st, Toast.LENGTH_SHORT).show();
		}
	}
	
    private void VPhoneNet()
    {
    	if (prPrefs_ != null)
    		prPrefs_.savePrefs(PreferenceManager.getDefaultSharedPreferences(this));
		Intent in = new Intent().setClass(this, ActiveFraim.class);
		startActivity(in);
		finish();
    }
    
    private void Recall()
    {
    	Intent in = new Intent().setClass(this, ForgotPassword.class);
		startActivity(in);    	
    }
    
    private void Registration()
    {

    	Intent in = new Intent().setClass(this, Registration.class);
		startActivity(in);
//		finish();
    }
    
	protected Dialog onCreateDialog(int id) 
	{
		Context ct = getApplicationContext();
		Resources rc = ct.getResources();
		String st = rc.getString(R.string.contoser);
		switch(id) 
		{
			case PROGRESS_DIALOG:
				pdLoad_ = new ProgressDialog(this);
				pdLoad_.setProgressStyle(ProgressDialog.STYLE_SPINNER);
				pdLoad_.setMessage(st);
				return pdLoad_;
			default:
				return null;
		}
	}
	protected void onPrepareDialog(int id, Dialog dialog)
	{
		switch(id) 
		{
			case PROGRESS_DIALOG:
				new DownloadFilesTask().execute(2);
		}
	}
    
    private class DownloadFilesTask extends AsyncTask<Integer, Integer, Long> 
    {
    	 int bOut_ = -1;
    	protected Long doInBackground(Integer... urls) 
    	{
    		long totalSize = 0;
    		bOut_ = NativeLib.svpLogin(stUserName_, stPassWord_);
    		return totalSize;
    	}

    	protected void onProgressUpdate(Integer... progress) 
    	{

    	}

    	protected void onPostExecute(Long result) 
    	{
	   			dismissDialog(PROGRESS_DIALOG);
    			completReg(bOut_);   			
    	}
    }   

}

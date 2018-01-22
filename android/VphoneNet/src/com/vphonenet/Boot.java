package com.vphonenet;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Bundle;
import android.view.Window;
import android.view.inputmethod.InputMethodManager;

public class Boot extends Activity
{
	public void onCreate(Bundle savedInstanceState) 
	{
		super.onCreate(savedInstanceState);
	    requestWindowFeature(Window.FEATURE_NO_TITLE);
		setContentView(R.layout.boot);
		new DownloadFilesTask().execute(2);
	}
    @Override
	public void onResume()
	{
		super.onResume();
	}
    
    private void startApp()
    {
    	
    	Intent in = new Intent().setClass(this, LogIn.class);
		startActivity(in);
		finish();    	
    }
    
    private class DownloadFilesTask extends AsyncTask<Integer, Integer, Long> 
    {
    	boolean bOut_ = false;
    	protected Long doInBackground(Integer... urls) 
    	{
    		long totalSize = 0;
    		try
    		{
    			Thread.sleep(4000);
    		}
    		catch(Exception e)
    		{
    			
    		}		
    		return totalSize;
    	}

    	protected void onProgressUpdate(Integer... progress) 
    	{
//    		setProgressPercent(progress[0]);
    	}

    	protected void onPostExecute(Long result) 
    	{
    		startApp();  
    	}
    }
}

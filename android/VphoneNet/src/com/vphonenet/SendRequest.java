package com.vphonenet;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.view.Window;
import android.widget.Button;

public class SendRequest  extends Activity
{
	public void onCreate(Bundle savedInstanceState) 
	{
		super.onCreate(savedInstanceState);
	    requestWindowFeature(Window.FEATURE_NO_TITLE);
		setContentView(R.layout.sendrequest);

		Button btRequest_ = (Button) findViewById(R.id.addcontact);
		if (btRequest_ != null)
			btRequest_.setOnClickListener
			(
				new View.OnClickListener() 
				{
					public void onClick(View v) 
					{	
						sendRequest();
					}
				}
			);
	}
	
	private void sendRequest()
	{
		
	}
}

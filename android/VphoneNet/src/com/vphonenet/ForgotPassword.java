package com.vphonenet;

import com.vphonenet.R;
import android.app.Activity;
import android.content.Context;
import android.content.res.Resources;
import android.os.Bundle;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.EditText;

public class ForgotPassword extends Activity
{
	private Button 		btOK_ 			= null;
	public void onCreate(Bundle savedInstanceState) 
	{
		super.onCreate(savedInstanceState);
	    requestWindowFeature(Window.FEATURE_NO_TITLE);
	    setContentView(R.layout.forgotpass);
	    btOK_ = (Button) findViewById(R.id.OK); 
		Context ct = getApplicationContext();
		Resources rc = ct.getResources();

	    btOK_.setOnClickListener(new View.OnClickListener() 
		{
			public void onClick(View v) 
			{
				EditText et = (EditText) findViewById(R.id.userid);
				if (et != null)
				{
					String login = et.getText().toString();
					if (login != null)
						if (login.trim().length() > 0)
							NativeLib.svpForgotPassword(login);
				}
			}
		});


	}
}

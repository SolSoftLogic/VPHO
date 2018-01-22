package com.vphonenet;

import android.content.Intent;
import android.os.Bundle;


public class PrifilePack  extends ActivityPack
{
	private static PrifilePack 		mySelf = null;
	@Override
    public void onCreate(Bundle savedInstanceState) 
	{
        super.onCreate(savedInstanceState);
        if (mySelf == null)
        	mySelf = this;
        startChildActivity("Account", new Intent(this, Account.class));
    }
	public static void changeActivity(String lable, Intent activity)
	{
		if (mySelf != null)
			mySelf.startChildActivity(lable, activity);
		
	}
}

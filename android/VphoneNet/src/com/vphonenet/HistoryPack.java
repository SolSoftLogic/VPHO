package com.vphonenet;

import android.content.Intent;
import android.os.Bundle;

public class HistoryPack extends ActivityPack
{
	private static HistoryPack		mySelf = null;
	@Override
    public void onCreate(Bundle savedInstanceState) 
	{
        super.onCreate(savedInstanceState);
        if (mySelf == null)
        	mySelf = this;
        startChildActivity("History", new Intent(this, History.class));
    }
	public static void changeActivity(String lable, Intent activity)
	{
		if (mySelf != null)
			mySelf.startChildActivity(lable, activity);
		
	}
}

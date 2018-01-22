package com.vphonenet;

import android.content.Intent;
import android.os.Bundle;

public class SearchPack extends ActivityPack
{
	private static SearchPack		mySelf = null;
	@Override
    public void onCreate(Bundle savedInstanceState) 
	{
        super.onCreate(savedInstanceState);
        if (mySelf == null)
        	mySelf = this;
        startChildActivity("SearchDir", new Intent(this, SearchDirectory.class));
    }
	public static void changeActivity(String lable, Intent activity)
	{
		if (mySelf != null)
			mySelf.startChildActivity(lable, activity);
		
	}
}

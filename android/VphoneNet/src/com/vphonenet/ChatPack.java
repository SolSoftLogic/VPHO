package com.vphonenet;

import android.content.Intent;
import android.os.Bundle;

public class ChatPack extends ActivityPack
{
	private static ChatPack		mySelf = null;
	@Override
    public void onCreate(Bundle savedInstanceState) 
	{
        super.onCreate(savedInstanceState);
        if (mySelf == null)
        	mySelf = this;
        startChildActivity("Chat", new Intent(this, Chat.class));
    }
	public static void changeActivity(String lable, Intent activity)
	{
		if (mySelf != null)
			mySelf.startChildActivity(lable, activity);
		
	}
}

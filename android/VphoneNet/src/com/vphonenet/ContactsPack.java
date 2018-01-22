package com.vphonenet;

import android.content.Intent;
import android.os.Bundle;


public class ContactsPack extends ActivityPack
{
	@Override
    public void onCreate(Bundle savedInstanceState) 
	{
        super.onCreate(savedInstanceState);
        startChildActivity("Contacts", new Intent(this, Contacts.class));
    }
}

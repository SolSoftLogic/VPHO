package com.vphonenet;

import android.app.TabActivity;
import android.content.Intent;
import android.content.res.Resources;
import android.graphics.Color;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.Window;
import android.widget.ImageView;
import android.widget.TabHost;
import android.widget.TextView;

public class ActiveFraim extends TabActivity
{
	private static TabHost tabHost_ 			= null;
	public void onCreate(Bundle savedInstanceState) 
	{
		super.onCreate(savedInstanceState);
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		setContentView(R.layout.tabfraim);
		Resources res = getResources();
		View v = null;
		tabHost_ = getTabHost(); 
		TabHost.TabSpec spec; 
		Intent intent; 
		tabHost_.setBackgroundColor( Color.BLACK);
		intent = new Intent().setClass(this, ContactsPack.class);
		addTab("Contacts", intent, R.drawable.contactsdsc);
/*		spec = tabHost_.newTabSpec("contacts").setIndicator("Contacts",
		 res.getDrawable(R.drawable.contactsdsc))
		 .setContent(intent);
		tabHost_.addTab(spec);*/
		
		intent = new Intent().setClass(this, ChatPack.class);
		addTab("Chat", intent, R.drawable.chatdsc);
/*		spec = tabHost_.newTabSpec("chat").setIndicator("Chat",
		 res.getDrawable(R.drawable.chatdsc))
		 .setContent(intent);
		 tabHost_.addTab(spec);*/
		 
		intent = new Intent().setClass(this, Contacts.class);
		addTab("Dialpad", intent, R.drawable.dialdsc);
/*			spec = tabHost_.newTabSpec("dialpad").setIndicator("Dialpad",
			 res.getDrawable(R.drawable.dialdsc))
			 .setContent(intent);
			tabHost_.addTab(spec);*/
			
			intent = new Intent().setClass(this, Contacts.class);
			/*		spec = tabHost_.newTabSpec("profile").setIndicator("Profile",
									res.getDrawable(R.drawable.profiledsc))
					 .setContent(intent);
					tabHost_.addTab(spec);*/
					addTab("Profile", intent, R.drawable.profiledsc);
			
		intent = new Intent().setClass(this, HistoryPack.class);
		addTab("History", intent, R.drawable.historydsc);
/*			spec = tabHost_.newTabSpec("history").setIndicator("History",
			 res.getDrawable(R.drawable.historydsc))
			 .setContent(intent);
			tabHost_.addTab(spec);*/

		

		 tabHost_.setCurrentTab(0);
	}
	public static void changeTab(int i){if (i > -1 )tabHost_.setCurrentTab(i);};
	private void addTab(String label, Intent intent, int drawableId)
	{
		View tabView = LayoutInflater.from(this).inflate(R.layout.tabview, getTabWidget(), false);
		TextView title = (TextView) tabView.findViewById(R.id.tabText);
		title.setText(label);
		ImageView icon = (ImageView) tabView.findViewById(R.id.icon);
		icon.setImageResource(drawableId);
		TabHost.TabSpec spec; 
		spec = tabHost_.newTabSpec(label).setIndicator(tabView).setContent(intent);
		tabHost_.addTab(spec);
	}
}

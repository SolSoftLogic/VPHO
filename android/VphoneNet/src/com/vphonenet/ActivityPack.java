package com.vphonenet;

import java.util.ArrayList;

import android.app.Activity;
import android.app.ActivityGroup;
import android.app.LocalActivityManager;
import android.content.Intent;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.Window;

public class ActivityPack extends ActivityGroup
{
	private ArrayList<String> 		lsActivityID_ 		= null;

	
	@Override
	public void onCreate(Bundle savedInstanceState) 
	{
	       super.onCreate(savedInstanceState);       
	        if (lsActivityID_ == null) 
	        lsActivityID_ = new ArrayList<String>();
	}
    /**
     * This is called when a child activity of this one calls its finish method. 
     * This implementation calls {@link LocalActivityManager#destroyActivity} on the child activity
     * and starts the previous activity.
     * If the last child activity just called finish(),this activity (the parent),
     * calls finish to finish the entire group.
     */
  @Override
  public void finishFromChild(Activity child) 
  {
      LocalActivityManager manager = getLocalActivityManager();
      int index = lsActivityID_.size()-1;
      
      if (index < 1) 
      {
          finish();
          return;
      }
          
      manager.destroyActivity(lsActivityID_.get(index), true);
      lsActivityID_.remove(index); index--;
      String lastId = lsActivityID_.get(index);
      Intent lastIntent = manager.getActivity(lastId).getIntent();
      Window newWindow = manager.startActivity(lastId, lastIntent);
      setContentView(newWindow.getDecorView());
  }
  
  public void startChildActivity(String Id, Intent intent) 
  {     
	      Window window = getLocalActivityManager().startActivity(Id,intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP));
	      if (window != null) 
	      {
	    	  lsActivityID_.add(Id);
	    	  setContentView(window.getDecorView()); 
	      }    		  
  }
  
  @Override
  public boolean onKeyDown(int keyCode, KeyEvent event) 
  {
      if (keyCode == KeyEvent.KEYCODE_BACK)
           return true;
      return super.onKeyDown(keyCode, event);
  }
  
  @Override
  public boolean onKeyUp(int keyCode, KeyEvent event) 
  {
      if (keyCode == KeyEvent.KEYCODE_BACK) 
      {
          int length = lsActivityID_.size();
          if ( length > 1) 
          {
              Activity current = getLocalActivityManager().getActivity(lsActivityID_.get(length-1));
              current.finish();
          }  
          return true;
      }
      return super.onKeyUp(keyCode, event);
  }
	
}

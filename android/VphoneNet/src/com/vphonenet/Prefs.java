package com.vphonenet;


import android.content.SharedPreferences;


/*
 * This is a preferences data class.
 * It manages data with the help of SharedPreferences pointer,
 *  presented by caller 
 *  User can access preferences data with the help of setters and getters methods.
 */
public final class Prefs 
{
	private String 			stPassword_ 		= null; 	
	private String 			stLogin_ 			= null;			
	private Boolean 		bRemember_ 			= false;
	private Boolean 		bNeedUpdate_ 		= false;
	// this is class constructor. It reads preferences data from device file system
	public Prefs(SharedPreferences pf)
	{
		if (pf != null)
		{
			stPassword_ = pf.getString("Password", null); 
			stLogin_ = pf.getString("Login", null); 
			bRemember_ = pf.getBoolean("RememberMe", false); 
		}		
	}
	
	// this method saves changed preferences data
	private void update(SharedPreferences pf)
	{
		if (pf!=null)
		{
			SharedPreferences.Editor editor = pf.edit();
			if (bRemember_)
			{		
				editor.putString("Password", stPassword_);
				editor.putString("Login", stLogin_);
			}
			editor.putBoolean("RememberMe", bRemember_);
			editor.commit(); 
			
		}
	}
	
	public void savePrefs(SharedPreferences pf)
	{
		if (bNeedUpdate_)
		{
			update(pf);
			bNeedUpdate_ = false;
		}
	}
	
	//Getters methods
	public String getPassword(){return stPassword_;};
	public String getLogin(){return stLogin_;};
	public Boolean getREmemberMe(){return bRemember_;};

	//Setters methods
	public void setPassword(String st){ stPassword_ = st; bNeedUpdate_ = true;};
	public void setLogin(String st){stLogin_ = st; bNeedUpdate_ = true;};
	public void setLogin(Boolean st){bRemember_ = st; bNeedUpdate_ = true;};

}
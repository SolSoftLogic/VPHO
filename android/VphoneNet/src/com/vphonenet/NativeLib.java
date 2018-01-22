package com.vphonenet;

public class NativeLib 
{
	public static NativeLib			selPh_ = null; 
	public NativeLib()
	{
		if (selPh_ == null)
			selPh_ = this;
	}
	private static NativeLib getMe()
	{
		if (selPh_ == null)
			selPh_ = new NativeLib();
		return selPh_;
	}
	public static int svpInit()
	{
		return getMe().vpInit();
	}
	
	private int vpInit()
	{
		//Init vpstack
		return 0;
	}

	public static int svpLogin(String login, String password)
	{
		return getMe().vpLogin(login, password );
		//login

	}
	
	private int vpLogin(String login, String password)
	{
		//ntvpLogin(login, password );

		//login
		return 0;
	}

	public static int svpForgotPassword(String login)
	{
		//send email
		return getMe().vpForgotPassword(login);
	}
	
	private int vpForgotPassword(String login)
	{
		//send email
		return 0;
	}

	public static int svpRegister(String login)
	{
		return getMe().vpRegister(login);
	}
	
	private int vpRegister(String login)
	{
		//register new user
		return 0;
	}


}

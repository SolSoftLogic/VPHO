package com.vphonenet;
class UserLoginData 
{
	public String					frm_first_name					= null;
	public String					frm_last_name					= null;
	public String					frm_username					= null;
	public String					frm_password					= null;
	public String					frm_email						= null;
	public String 					frm_phone						= null;
	public String					frm_country						= null;
}

class UserAccountData 
{
	public String					stSettings_						= null;
	public String					stCredit_						= null;
	public int 						iStatus_						= -1;
}

class UserProfileData
{
	public String					stState_						= null;
	public String					stBirthDay_						= null;
	public String					stGender_						= null;
	public String					stLanguage_						= null;
	private byte[] 					byPhoto_						= null; 
}

public class ItisMe 
{
	private UserLoginData 		myLogData_		= null;
	private UserAccountData 	myAccountData_	= null;
	private UserProfileData		myProfData_		= null;
	private static ItisMe 		pSelf_ 			= null;

	
	private static ItisMe getMe()
	{
		if (pSelf_ == null)
			pSelf_ = new ItisMe();
		return pSelf_;
	}
	static public UserLoginData getMyLogData(){return getMe().myLogData_;};
	static public void setMyLogData(UserLoginData data){getMe().myLogData_ = data;}
	static public UserAccountData getMyAccountData(){return getMe().myAccountData_;};
	static public void setMyAccountData(UserAccountData data){getMe().myAccountData_ = data;}
	static public UserProfileData getMyProfData(){return getMe().myProfData_;};
	static public void setMyProfData(UserProfileData data){getMe().myProfData_ = data;}

}

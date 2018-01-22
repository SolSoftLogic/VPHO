package com.vphonenet;

public class Status 
{
	public static final int AOL_UNKNOWN = 0;
	public static final int AOL_OFFLINE = 1;
	public static final int AOL_ONLINE = 2;
	public static final int AOL_LIMITED = 4;
	public static final int AOL_WEBCAMPRESENT = 5;
	public static final int AOL_LIMITED_WEBCAMPRESENT = 6;
	public static final int AOL_CALLFORWARDING = 7;
	public static int getResource(int i)
	{
		int iOut = R.drawable.satusunknown;
		switch(i)
		{
			case AOL_UNKNOWN:
				iOut = R.drawable.satusunknown;
				break;
			case AOL_OFFLINE:
				iOut = R.drawable.statusred;
				break;
			case AOL_ONLINE:
				iOut = R.drawable.statusgreen;
				break;
			case AOL_LIMITED:
				iOut = R.drawable.statusyellow;
				break;
		}
		return iOut;
	}
	
	public static int getTextRes(int i)
	{
		int iOut = R.string.statusunknown;
		switch(i)
		{
			case AOL_UNKNOWN:
				iOut = R.string.statusunknown;
				break;
			case AOL_OFFLINE:
				iOut = R.string.statusoff;
				break;
			case AOL_ONLINE:
				iOut = R.string.statuson;	
				break;
			case AOL_LIMITED:
				iOut = R.string.statuslimited;
				break;
			case AOL_WEBCAMPRESENT:
				iOut = R.string.statuswebcampresent;
				break;
			case AOL_LIMITED_WEBCAMPRESENT:
				iOut = R.string.statuswebcampresentlimit;
				break;
			case AOL_CALLFORWARDING:
				iOut = R.string.statuscallforward;
				break;
		}
		return iOut;		
	}
}

package com.vphonenet;

import java.io.File;
import java.text.DateFormat;
import java.util.ArrayList;
import android.app.ListActivity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

public class ShareFile  extends ListActivity
{
	static String _stExtension = null; /*requested extension*/
	protected static final int REQUEST_PESENTED_OK = 1; /* successful exit code */
//	static boolean _bLayoutLinear = true; /* layout selector: linear or relative */
//	static boolean _bNeedAnswer = false; /* application need to replay with selected file path */ 
	private File _fCurPoint = null; /* current folder*/
	/* list of entry names. It's used for a screen presentation */
	private ArrayList<String> _lsListItems = new ArrayList<String>();
	/* list of File objects. It's used for a directory changing */	
	private ArrayList<File> _lsListFiles = new ArrayList<File>();		
	private Button _btRoot = null;/* root navigation button*/
	private Button _btParent = null;/* parent navigation button*/
	private TextView _tvDirNameHeader = null;/* folder path header*/
	SharedPreferences mPreferences = null; //Preference system object
	OrderAdapter mCurDirLst = null;
    @Override
    public void onCreate(Bundle savedInstanceState) 
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.filesurfer);
        Intent in = this.getIntent();
        Bundle bn = in.getExtras();
        /* create header: two navigation buttons and folder path name */
        _btRoot = (Button) findViewById(R.id.root_button);
        _btParent = (Button) findViewById(R.id.parent_button);
        _tvDirNameHeader = (TextView)findViewById(R.id.folder_name);
        /*set button imaged*/
        _btRoot.setBackgroundResource(R.drawable.bonsai);
        _btParent.setBackgroundResource(R.drawable.gunsen);
        /* set button listeners */
        _btRoot.setOnClickListener(new View.OnClickListener() 
        	{
        		public void onClick(View v) {fillCurDirList("/", _fCurPoint.toString());}
        	});
        _btParent.setOnClickListener(new View.OnClickListener() 
    	{
    		public void onClick(View v) 
    		{
    			String st = _fCurPoint.getParent();
    			if (st != null)
    				fillCurDirList(st, _fCurPoint.toString());
    		}
    	});       
        mPreferences = 	PreferenceManager.getDefaultSharedPreferences(this);
		mCurDirLst = new OrderAdapter(this, R.layout.filesurferline, _lsListFiles);
		setListAdapter(mCurDirLst);  
 
    }
    @Override
	public void onResume()
	{
		super.onResume();
        String stFile = mPreferences.getString("CurFile", "/"); 
    	File file = new File(stFile);
    	String st = "/";
    	if (file.exists())
    	{
    		st = file.getParent();
    		if (st == null)
    			st = "/";
    	}
	    fillCurDirList(st, stFile);
	}
    @Override	
    public void onStop()
    {
    	super.onStop();
        SharedPreferences.Editor editor = mPreferences.edit();       
        String st = _fCurPoint.toString();
        editor.putString("CurFile", st);
        editor.commit(); 
    }
    private boolean fillCurDirList(String stDirName, String stFile)
    {
    	boolean bOut = false;
    	if (stDirName == null)
    		return bOut;
    	/* create new File object for selected folder name */
    	File file = new File(stDirName);
		int iCurPos = 0;
    	if (file.exists())
    	{// if this is a real file system object
    		_fCurPoint = file;
    		if (file.isDirectory())
    		{// if this file system object is a folder
    			/* fix a current folder */
    			_tvDirNameHeader.setText(file.getPath());
	    		if (_stExtension != null)
	    			_tvDirNameHeader.append(" ("+_stExtension+")");
//    			_fCurPoint = file;
    			/* clear lists */
       			_lsListFiles.clear();
    			_lsListItems.clear(); 
    			/* create list of file entries for the current directory */
       			File fListItrms[] = file.listFiles();
       			if (fListItrms!=null)
       			{
       				String stServ = null;
       				int i = 0;
       				if (stFile!=null)
       				{
       					int ist = stFile.lastIndexOf("/");
       					if (ist > -1)
       						stFile = stFile.substring(ist+1);
       				}
       				for (File fl:fListItrms)
       				{// populate screen and instrumental lists
       					stServ = fl.getName();
      					if (fl.isDirectory() || 
            					_stExtension == null || 
            					(_stExtension != null && 
            							(
            									stServ.lastIndexOf(_stExtension.toLowerCase()) > 0 ||
            									stServ.lastIndexOf(_stExtension.toUpperCase()) > 0
            									)
            					)
            				)
       					{
       						_lsListItems.add(stServ);
       						if (stFile != null)
       							if (stFile.equals(stServ))
       								iCurPos = i;
       						_lsListFiles.add(fl);
       					}
      					i++;
      					mCurDirLst.notifyDataSetChanged();
       				}
       			}
           		if (stFile != null)
           			this.setSelection(iCurPos);
    		}
    		else
    			makeAnswer(file);
    		bOut = true;
    	}
    	return bOut;
    }
    void makeAnswer(File file)
	{
/*		Intent in = new Intent();
		Bundle bn = new Bundle();
		String stPath = file.getAbsolutePath();
		bn.putString("FilePath", stPath);
		in.putExtras(bn);
		this.setResult(REQUEST_PESENTED_OK,in);*/
		finish();
	}    
    @Override 
    protected void onListItemClick(ListView l, View v, int position, long id) 
    {// Called when user selects list screen list entry
    	/* get selected item position */
    	String stDirItem = getListView().getItemAtPosition(position).toString();
    	/* get string name of the selected item */
    	stDirItem = _lsListFiles.get(position).getPath();
    	/* change folder and create new file list */ 
    	fillCurDirList(stDirItem, null);
    }
    private class OrderAdapter extends ArrayAdapter<File> 
    {
    	private ArrayList<File> _items;
    	public OrderAdapter(Context context, int textViewResourceId, ArrayList<File> items) 
    	{
    		super(context, textViewResourceId, items);
    		this._items = items;
    	}
    	@Override
    	public View getView(int position, View convertView, ViewGroup parent) 
    	{

    		View v = convertView;
    		if (v == null) 
    		{
    			/* set layout for a list item data */
    			LayoutInflater vi = (LayoutInflater)getSystemService(Context.LAYOUT_INFLATER_SERVICE);
    			v = vi.inflate(R.layout.filesurferline, null);
    		}
    		File f = _items.get(position);/*  get current list item data */
    		if (f != null) 
    		{
    			TextView bt = null;
				TextView tt = null;
				ImageView iv = null;
    			/* get pointers to layout items */
 
       			bt = (TextView) v.findViewById(R.id.bottomline);
    			tt = (TextView) v.findViewById(R.id.topline);
    			iv = (ImageView) v.findViewById(R.id.icon);
 
 
    			if (iv != null)/* set image */
    				if (f.isDirectory()) /* current entry is a folder*/
    					iv.setImageResource(R.drawable.torii);
    				else /* current entry is a file */
    					iv.setImageResource(R.drawable.iching);
    			if (tt != null) /* set file name */
    				tt.setText(f.getName() ); 
    			/* set format for date and time string */
    			DateFormat df1 = DateFormat.getDateTimeInstance(DateFormat.MEDIUM, DateFormat.MEDIUM);
    			if(bt != null) /* show file date and time */
    				bt.setText(df1.format(f.lastModified()));
    		}
    		return v;
    	}
    }
}

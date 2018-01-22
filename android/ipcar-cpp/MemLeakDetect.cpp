/*************************************************************
 Author		: David A. Jones
 File Name	: MemLeakDetect.h
 Date		: July 30, 2004
 Synopsis		 
			A trace memory feature for source code to trace and
			find memory related bugs. 


****************************************************************/
// July 2009: tim.s.stevens@bt.com : Modified it to work with ANSI or UNICODE. Based on:
// http://www.codeproject.com/cpp/MemLeakDetect.asp

#ifdef _DEBUG1

int catchMemoryAllocHook(int	allocType, 
						 void	*userData, 
						 size_t size, 
						 int	blockType, 
						 long	requestNumber, 
		  const unsigned char	*filename, // Can't be UNICODE
						 int	lineNumber) ;




#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#ifdef UNICODE
	#ifndef _UNICODE
		#define _UNICODE
	#endif
#endif

#include <iostream>
#include <tchar.h>
using namespace std ;
// TODO: reference additional headers your program requires here
#include <vector>
#include <deque>

#include "MemLeakDetect.h"

static CMemLeakDetect*	g_pMemTrace			= NULL;
static _CRT_ALLOC_HOOK	pfnOldCrtAllocHook	= NULL;

static int MyTrace(const TCHAR * lpszFormat, ...) ;

int MyTrace(const TCHAR * lpszFormat, ...)
{
 	va_list args;
	va_start( args, lpszFormat);
#ifndef UNICODE
	char buffer[1024];
	vsprintf_s( buffer, lpszFormat, args );
	return _CrtDbgReport(_CRT_WARN,NULL,NULL,NULL,buffer);
#else
	TCHAR buffer[1024];
	vswprintf_s( buffer, lpszFormat, args );
	char fmtbuf[1024] ;
	WideCharToMultiByte(CP_ACP, 0, buffer, -1,
		fmtbuf, 1024, NULL, NULL ) ;

	return _CrtDbgReport(_CRT_WARN,NULL,NULL,NULL,fmtbuf);
#endif
}



int catchMemoryAllocHook(int	allocType, 
						 void	*userData, 
						 size_t size, 
						 int	blockType, 
						 long	requestNumber, 
		  const unsigned char	*filename, // Can't be UNICODE
						 int	lineNumber)
{
	_CrtMemBlockHeader *pCrtHead;
	long prevRequestNumber;
#ifdef UNICODE
	wchar_t Wname[1024] ;
	Wname[0] = '\0' ;
#endif
	// internal C library internal allocations
	if ( blockType == _CRT_BLOCK )
	{
		return( TRUE );
	}
	// check if someone has turned off mem tracing
	if  ((( _CRTDBG_ALLOC_MEM_DF & _crtDbgFlag) == 0) && 
		(( allocType			== _HOOK_ALLOC)		|| 
			( allocType			== _HOOK_REALLOC)))
	{
		if (pfnOldCrtAllocHook)
		{
			pfnOldCrtAllocHook(allocType, userData, size, blockType, requestNumber, filename, lineNumber);
		}
		return TRUE;
	}

	// protect if mem trace is not initialized
	if (g_pMemTrace == NULL)
	{
		if (pfnOldCrtAllocHook)
		{
			pfnOldCrtAllocHook(allocType, userData, size, blockType, requestNumber, filename, lineNumber);
		}
		return TRUE;
	}

	// protect internal mem trace allocs
	if (g_pMemTrace->isLocked)
	{
		if (pfnOldCrtAllocHook)
		{
			pfnOldCrtAllocHook(allocType, userData, size, blockType, requestNumber, filename, lineNumber);
		}
		return( TRUE);
	}
	// lock the function
	g_pMemTrace->isLocked = true;
	//
#ifdef UNICODE
	int len ;
	if (NULL != filename)
	{
		len = strlen((char *)filename) + 1 ;
		MultiByteToWideChar(CP_ACP, 0, (char *)filename, len, Wname, len) ;
	}
	else
		len = 0 ;
#else
	#define Wname (char*)filename
#endif
	if (allocType == _HOOK_ALLOC)
	{

		g_pMemTrace->addMemoryTrace((void *) requestNumber, size, Wname, lineNumber);
	}
	else
	if (allocType == _HOOK_REALLOC)
	{
		if (_CrtIsValidHeapPointer(userData))
		{
			pCrtHead = pHdr(userData);
			prevRequestNumber = pCrtHead->lRequest;
			//
			if (pCrtHead->nBlockUse == _IGNORE_BLOCK)
			{
				if (pfnOldCrtAllocHook)
				{
					pfnOldCrtAllocHook(allocType, userData, size, blockType, requestNumber, filename, lineNumber);
				}
				goto END;
			}
	   		g_pMemTrace->redoMemoryTrace((void *) requestNumber, (void *) prevRequestNumber, size, Wname, lineNumber);
		}
	}
	else
	if (allocType == _HOOK_FREE)
	{
		if (_CrtIsValidHeapPointer(userData))
		{
			pCrtHead = pHdr(userData);
			requestNumber = pCrtHead->lRequest;
			//
			if (pCrtHead->nBlockUse == _IGNORE_BLOCK)
			{
				if (pfnOldCrtAllocHook)
				{
					pfnOldCrtAllocHook(allocType, userData, size, blockType, requestNumber, filename, lineNumber);
				}
				goto END;
			}
	   		g_pMemTrace->removeMemoryTrace((void *) requestNumber, userData);
		}
	}
END:
	// unlock the function
	g_pMemTrace->isLocked = false;
	return TRUE;
}

void CMemLeakDetect::addMemoryTrace(void* addr,  DWORD asize,  TCHAR *fname, DWORD lnum)
{
	AllocBlockInfo ainfo;
	//
	if (m_AllocatedMemoryList.Lookup(addr, ainfo))
	{
		// already allocated
		AfxTrace(_T("ERROR!CMemLeakDetect::addMemoryTrace() Address(0x%08X) already allocated\n"), addr);
		return;
	}
	//
	ainfo.address		= addr;
	ainfo.lineNumber	= lnum;
	ainfo.size			= asize;
	ainfo.occurance		= memoccurance++;
	MLD_STACKWALKER(&ainfo.traceinfo[0]);
	//
	if (fname)
		_tcsncpy_s(&ainfo.fileName[0], MLD_MAX_NAME_LENGTH, fname, MLD_MAX_NAME_LENGTH);
	else
	  ainfo.fileName[0] = 0;
	//
	m_AllocatedMemoryList.SetAt(addr, ainfo);
};
void CMemLeakDetect::redoMemoryTrace(void* addr,  void* oldaddr, DWORD asize,  /*char*/TCHAR *fname, DWORD lnum)
{
	AllocBlockInfo ainfo;

	if (m_AllocatedMemoryList.Lookup(oldaddr,(AllocBlockInfo &) ainfo))
	{
		m_AllocatedMemoryList.RemoveKey(oldaddr);
	}
	else
	{
		AfxTrace(_T("ERROR!CMemLeakDetect::redoMemoryTrace() didnt find Address(0x%08X) to free\n"), oldaddr);
	}
	//
	ainfo.address		= addr;
	ainfo.lineNumber	= lnum;
	ainfo.size			= asize;
	ainfo.occurance		= memoccurance++;
	MLD_STACKWALKER(&ainfo.traceinfo[0]);
	//
	if (fname)
		_tcsncpy_s(&ainfo.fileName[0], MLD_MAX_NAME_LENGTH, fname, MLD_MAX_NAME_LENGTH);
	else
	  ainfo.fileName[0] = 0;

	m_AllocatedMemoryList.SetAt(addr, ainfo);
};
void CMemLeakDetect::removeMemoryTrace(void* addr, void* realdataptr)
{
	AllocBlockInfo ainfo;
	//
	if (m_AllocatedMemoryList.Lookup(addr,(AllocBlockInfo &) ainfo))
	{
		m_AllocatedMemoryList.RemoveKey(addr);
	}
	else
	{
	   //freeing unallocated memory
		AfxTrace(_T("ERROR!CMemLeakDetect::removeMemoryTrace() didnt find Address(0x%08X) to free\n"), addr);
	}
};
void CMemLeakDetect::cleanupMemoryTrace()
{
	m_AllocatedMemoryList.RemoveAll();
};

void CMemLeakDetect::dumpMemoryTrace()
{
	POSITION			pos;
	LPVOID				addr;
	AllocBlockInfo		ainfo;
	TCHAR				buf[MLD_MAX_NAME_LENGTH];
	TCHAR				symInfo[MLD_MAX_NAME_LENGTH];
	TCHAR				srcInfo[MLD_MAX_NAME_LENGTH];
	int					totalSize						= 0;
	int					numLeaks						= 0;
	STACKFRAMEENTRY*	p								= 0;

	//
	_tcscpy_s(symInfo, MLD_MAX_NAME_LENGTH, MLD_TRACEINFO_NOSYMBOL);
	_tcscpy_s(srcInfo, MLD_MAX_NAME_LENGTH, MLD_TRACEINFO_NOSYMBOL);
	//
	pos = m_AllocatedMemoryList.GetStartPosition();
	//

	while(pos != m_AllocatedMemoryList.end())
	{
		numLeaks++;
		_stprintf_s(buf, _T("Memory Leak(%d)------------------->\n"), numLeaks);
		AfxTrace(buf);
		//
		m_AllocatedMemoryList.GetNextAssoc(pos, (LPVOID &) addr, (AllocBlockInfo&) ainfo);
		if (ainfo.fileName[0] != NULL)
		{
			_stprintf_s(buf, _T("Memory Leak <0x%X> bytes(%d) occurance(%d) %s(%d)\n"), 
					ainfo.address, ainfo.size, ainfo.occurance, ainfo.fileName, ainfo.lineNumber);
		}
		else
		{
			_stprintf_s(buf, _T("Memory Leak <0x%X> bytes(%d) occurance(%d)\n"), 
					ainfo.address, ainfo.size, ainfo.occurance);
		}
		//
		AfxTrace(buf);
		//
		p = &ainfo.traceinfo[0];

		while(p[0].addrPC.Offset)
		{
			symFunctionInfoFromAddresses(p[0].addrPC.Offset, p[0].addrFrame.Offset, symInfo, MLD_MAX_NAME_LENGTH);
			symSourceInfoFromAddress(p[0].addrPC.Offset, srcInfo, MLD_MAX_NAME_LENGTH);
			AfxTrace(_T("%s->%s()\n"), srcInfo, symInfo);
			p++;
		}
		totalSize += ainfo.size;

	}

	_stprintf_s(buf, _T("\n-----------------------------------------------------------\n"));
	AfxTrace(buf);
	if(!totalSize) 
	{
		_stprintf_s(buf,_T("No Memory Leaks Detected for %d Allocations\n\n"), memoccurance);
		AfxTrace(buf);
	}
	else
	{
		_stprintf_s(buf, _T("Total %d Memory Leaks: %d bytes Total Alocations %d\n\n"), numLeaks, totalSize, memoccurance);
		AfxTrace(buf);
	}
}

void CMemLeakDetect::Init()
{
	  m_dwsymBufSize		= (MLD_MAX_NAME_LENGTH + sizeof(PIMAGEHLP_SYMBOL));
	  m_hProcess			= GetCurrentProcess();
	  m_pSymbol				= (PIMAGEHLP_SYMBOL)GlobalAlloc( GMEM_FIXED, m_dwsymBufSize);

	  m_AllocatedMemoryList.InitHashTable(10211, TRUE);
	  initSymInfo( NULL);
	  isLocked				= false;
	  g_pMemTrace			= this;
	  pfnOldCrtAllocHook	= _CrtSetAllocHook( catchMemoryAllocHook ); 
}
void CMemLeakDetect::End()
{
	isLocked				= true;
	_CrtSetAllocHook(pfnOldCrtAllocHook);
	dumpMemoryTrace();
	cleanupMemoryTrace();
	cleanupSymInfo();
	GlobalFree(m_pSymbol);
	g_pMemTrace				= NULL;
}
CMemLeakDetect::CMemLeakDetect()
{
	Init();
}

CMemLeakDetect::~CMemLeakDetect()
{
	End();
}

// PRIVATE STUFF
void CMemLeakDetect::symbolPaths( TCHAR* lpszSymbolPath, UINT BufSizeTCHARs)
{
	TCHAR lpszPath[MLD_MAX_NAME_LENGTH];

   // Creating the default path where the dgbhelp.dll is located
   // ".;%_NT_SYMBOL_PATH%;%_NT_ALTERNATE_SYMBOL_PATH%;%SYSTEMROOT%;%SYSTEMROOT%\System32;"
	_tcscpy_s(lpszSymbolPath, BufSizeTCHARs, _T(".;..\\;..\\..\\"));

	// environment variable _NT_SYMBOL_PATH
	if ( GetEnvironmentVariable(_T("_NT_SYMBOL_PATH"), lpszPath, MLD_MAX_NAME_LENGTH ))
	{
	    _tcscat_s( lpszSymbolPath, BufSizeTCHARs, _T(";"));
		_tcscat_s( lpszSymbolPath, BufSizeTCHARs, lpszPath );
	}

	// environment variable _NT_ALTERNATE_SYMBOL_PATH
	if ( GetEnvironmentVariable( _T("_NT_ALTERNATE_SYMBOL_PATH"), lpszPath, MLD_MAX_NAME_LENGTH ))
	{
		_tcscat_s( lpszSymbolPath, BufSizeTCHARs, _T(";"));
		_tcscat_s( lpszSymbolPath, BufSizeTCHARs, lpszPath );
	}

	// environment variable SYSTEMROOT
	if ( GetEnvironmentVariable( _T("SYSTEMROOT"), lpszPath, MLD_MAX_NAME_LENGTH ) )
	{
	    _tcscat_s( lpszSymbolPath, BufSizeTCHARs, _T(";"));
		_tcscat_s( lpszSymbolPath, BufSizeTCHARs, lpszPath);
		_tcscat_s( lpszSymbolPath, BufSizeTCHARs, _T(";"));

		// SYSTEMROOT\System32
		_tcscat_s( lpszSymbolPath, BufSizeTCHARs, lpszPath );
		_tcscat_s( lpszSymbolPath, BufSizeTCHARs, _T("\\System32"));
	}
}

BOOL CMemLeakDetect::cleanupSymInfo()
{
	return SymCleanup( GetCurrentProcess() );
}

// Initializes the symbol files
BOOL CMemLeakDetect::initSymInfo( TCHAR* lpszUserSymbolPath)
{
	TCHAR    lpszSymbolPath[MLD_MAX_NAME_LENGTH];
    DWORD   symOptions = SymGetOptions();

	symOptions |= SYMOPT_LOAD_LINES; 
	symOptions &= ~SYMOPT_UNDNAME;
	SymSetOptions( symOptions );

    // Get the search path for the symbol files
	symbolPaths( lpszSymbolPath, MLD_MAX_NAME_LENGTH);
	//
	if (lpszUserSymbolPath)
	{
		_tcscat_s(lpszSymbolPath, MLD_MAX_NAME_LENGTH, _T(";"));
		_tcscat_s(lpszSymbolPath, MLD_MAX_NAME_LENGTH, lpszUserSymbolPath);
	}

#ifdef UNICODE
	int len = _tcslen(lpszSymbolPath) + 1 ;
	char dest[1024] ;
	WideCharToMultiByte( CP_ACP, 0, lpszSymbolPath, -1, dest, len, NULL, NULL );
	BOOL bret = SymInitialize( GetCurrentProcess(), dest, TRUE);
#else
	BOOL bret = SymInitialize( GetCurrentProcess(), lpszSymbolPath, TRUE) ;
#endif

	return bret ;
}

void CMemLeakDetect::symStackTrace(STACKFRAMEENTRY* pStacktrace )
{
	STACKFRAME     callStack;
	BOOL           bResult;
	CONTEXT        context;
	HANDLE		   hThread  = GetCurrentThread();

	// get the context
	memset( &context, NULL, sizeof(context) );
	context.ContextFlags = CONTEXT_FULL;
	if ( !GetThreadContext( hThread, &context ) )
	{
       AfxTrace(_T("Call stack info(thread=0x%X) failed.\n"), hThread );
	   return;
	}
	//initialize the call stack
	memset( &callStack, NULL, sizeof(callStack) );
	callStack.AddrPC.Offset    = context.Eip;
	callStack.AddrStack.Offset = context.Esp;
	callStack.AddrFrame.Offset = context.Ebp;
	callStack.AddrPC.Mode      = AddrModeFlat;
	callStack.AddrStack.Mode   = AddrModeFlat;
	callStack.AddrFrame.Mode   = AddrModeFlat;
	//
	for( DWORD index = 0; index < MLD_MAX_TRACEINFO; index++ ) 
	{
		bResult = StackWalk(IMAGE_FILE_MACHINE_I386,
							m_hProcess,
							hThread,
							&callStack,
							NULL, 
							NULL,
							SymFunctionTableAccess,
							SymGetModuleBase,
							NULL);

		//if ( index == 0 )
		 //  continue;

		if( !bResult || callStack.AddrFrame.Offset == 0 ) 
			break;
		//
		pStacktrace[0].addrPC	 = callStack.AddrPC;
		pStacktrace[0].addrFrame = callStack.AddrFrame;
		pStacktrace++;
	}
	//clear the last entry
	memset(pStacktrace, NULL, sizeof(STACKFRAMEENTRY));
}

//
// This code is still under investigation
// I have to test this code and make sure it is compatible
// with the other stack walker!
//
void CMemLeakDetect::symStackTrace2(STACKFRAMEENTRY* pStacktrace )
{
	ADDR			FramePtr				= NULL;
	ADDR			InstructionPtr			= NULL;
	ADDR			OriFramePtr				= NULL;
	ADDR			PrevFramePtr			= NULL;
	long			StackIndex				= NULL;

	// Get frame pointer
	_asm mov DWORD PTR [OriFramePtr], ebp

	FramePtr = OriFramePtr;

	//
	while (FramePtr)
	{
		InstructionPtr = ((ADDR *)FramePtr)[1];

		pStacktrace[StackIndex].addrPC.Offset	= InstructionPtr;
		pStacktrace[StackIndex].addrPC.Segment	= NULL;
		pStacktrace[StackIndex].addrPC.Mode		= AddrModeFlat;
		//
		StackIndex++;
		PrevFramePtr			= FramePtr;
		FramePtr				= ((ADDR *)FramePtr)[0];
	}
}

BOOL CMemLeakDetect::symFunctionInfoFromAddresses(ULONG fnAddress, ULONG stackAddress, /*LPTSTR*/TCHAR * lpszSymbol, 
												  UINT BufSizeTCHARs)
{
	DWORD             dwDisp	= 0;

	::ZeroMemory(m_pSymbol, m_dwsymBufSize );
	m_pSymbol->SizeOfStruct		= m_dwsymBufSize;
	m_pSymbol->MaxNameLength	= m_dwsymBufSize - sizeof(IMAGEHLP_SYMBOL);
    // Set the default to unknown
	_tcscpy_s( lpszSymbol, BufSizeTCHARs, MLD_TRACEINFO_NOSYMBOL);

	// Get symbol info for IP
	if ( SymGetSymFromAddr( m_hProcess, (ULONG)fnAddress, &dwDisp, m_pSymbol ) )
	{
#ifdef UNICODE
		int len = strlen(m_pSymbol->Name) + 1 ;
		wchar_t dest[1024] ;
		MultiByteToWideChar(CP_ACP, 0, m_pSymbol->Name, len, dest, len );
		_tcscpy_s(lpszSymbol, BufSizeTCHARs, dest);
#else
		_tcscpy_s(lpszSymbol, BufSizeTCHARs, m_pSymbol->Name);
#endif
		return TRUE;
	}

	//create the symbol using the address because we have no symbol
	_stprintf_s(lpszSymbol, BufSizeTCHARs, _T("0x%08X"), fnAddress);
	return FALSE;
}

BOOL CMemLeakDetect::symSourceInfoFromAddress(UINT address, TCHAR* lpszSourceInfo, UINT BufSizeTCHARs)
{
	BOOL           ret = FALSE;
	IMAGEHLP_LINE  lineInfo;
	DWORD          dwDisp;
	TCHAR          lpModuleInfo[MLD_MAX_NAME_LENGTH] = MLD_TRACEINFO_EMPTY;

	_tcscpy_s(lpszSourceInfo, BufSizeTCHARs, MLD_TRACEINFO_NOSYMBOL);

	memset( &lineInfo, NULL, sizeof( IMAGEHLP_LINE ) );
	lineInfo.SizeOfStruct = sizeof( IMAGEHLP_LINE );

	if ( SymGetLineFromAddr( m_hProcess, address, &dwDisp, &lineInfo ) )
	{
	   // Using the "sourcefile(linenumber)" format
#ifdef UNICODE
		wchar_t dest[1024] ;
		int len = strlen((char *)lineInfo.FileName) + 1 ;
		MultiByteToWideChar(CP_ACP, 0, (char *)lineInfo.FileName, len, dest, len) ;
		_stprintf_s(lpszSourceInfo, BufSizeTCHARs, _T("%s(%d): 0x%08X"), dest, lineInfo.LineNumber, address );//	<--- Size of the char thing.
#else
		_stprintf_s(lpszSourceInfo, BufSizeTCHARs, _T("%s(%d): 0x%08X"), lineInfo.FileName, lineInfo.LineNumber, address );//	<--- Size of the char thing.
#endif
		ret = TRUE;
	}
	else
	{
        // Using the "modulename!address" format
	  	symModuleNameFromAddress( address, lpModuleInfo, MLD_MAX_NAME_LENGTH);

		if ( lpModuleInfo[0] == _T('?') || lpModuleInfo[0] == _T('\0'))
		{
			// Using the "address" format
			_stprintf_s(lpszSourceInfo, BufSizeTCHARs, _T("0x%08X"), lpModuleInfo, address );
		}
		else
		{
			_stprintf_s(lpszSourceInfo, BufSizeTCHARs, _T("%sdll! 0x%08X"), lpModuleInfo, address );
		}
		ret = FALSE;
	}
	//
	return ret;
}

BOOL CMemLeakDetect::symModuleNameFromAddress( UINT address, TCHAR* lpszModule, UINT BufSizeTCHARs)
{
	BOOL              ret = FALSE;
	IMAGEHLP_MODULE   moduleInfo;

	::ZeroMemory( &moduleInfo, sizeof(IMAGEHLP_MODULE) );
	moduleInfo.SizeOfStruct = sizeof(IMAGEHLP_MODULE);
	
	if ( SymGetModuleInfo( m_hProcess, (DWORD)address, &moduleInfo ) )
	{
		// Note. IMAGEHLP_MODULE::ModuleName seems to be hardcoded as 32 char/wchar_t (VS2008).
#ifdef UNICODE
		int len = _tcslen(lpszModule) + 1 ;
		char dest[1024] ;
		WideCharToMultiByte( CP_ACP, 0, lpszModule, -1, dest, len, NULL, NULL );

		strcpy_s(moduleInfo.ModuleName, 32, dest);	// bloody ANSI!
#else
		strcpy_s(moduleInfo.ModuleName, 32, lpszModule);
#endif
		ret = TRUE;
	}
	else
	{
		_tcscpy_s( lpszModule, BufSizeTCHARs, MLD_TRACEINFO_NOSYMBOL);
	}
	
	return ret;
}

#endif

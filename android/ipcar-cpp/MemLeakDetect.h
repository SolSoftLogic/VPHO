/*!
\file
Code to detect memory allocation leaks in your application. Only present in _DEBUG builds, & is stripped out for NDEBUG.
To catch most malloc/free or new/delete leaks, simply add this block of code at the application level:
Usage:

\code
#ifdef _DEBUG
	#ifdef MEMORY_LEAK_CHECK
		#include "../common/MemLeakDetect.h"
		static CMemLeakDetect memLeakDetect;
	#endif
#endif
\endcode

A typical leak might be:
\code
	int *pfoo = new int[1000];
\endcode
Then forgetting to do
\code
	delete [] pfoo;
\endcode

Then when running under a debugger, if there is a leak, you'll get this kind of output in the Output pane:

\code
Memory Leak(1)------------------->
Memory Leak <0xBC> bytes(86) occurance(0)
c:\code\ta2svn\sandbox\pjh\software\common\memleakdetect.cpp(201): 0x0044B7C3->CMemLeakDetect::addMemoryTrace()
c:\code\ta2svn\sandbox\pjh\software\common\memleakdetect.cpp(140): 0x0044B4B2->catchMemoryAllocHook()
0x0012D874->_malloc_dbg()
0x0012D874->_malloc_dbg()
0x0012D874->_malloc_dbg()
0x0012D874->malloc()
0x0012D874->??2@YAPAXI@Z()
f:\dd\vctools\crt_bld\self_x86\crt\src\newaop.cpp(7): 0x004B4D1E->operator new[]()
c:\code\ta2svn\sandbox\pjh\software\hw_app\hw_app.cpp(145): 0x00442276->wmain()
f:\dd\vctools\crt_bld\self_x86\crt\src\crtexe.c(579): 0x004B56C8->__tmainCRTStartup()
f:\dd\vctools\crt_bld\self_x86\crt\src\crtexe.c(399): 0x004B550F->wmainCRTStartup()
0x0012D874->RegisterWaitForInputIdle()

-----------------------------------------------------------
Total 1 Memory Leaks: 86 bytes Total Alocations 276

\endcode

You can then double-click in the Output pane on the leak ((145) in the example above) and be taken to the source line
which caused the leak.
*/

/*************************************************************
 Author		: David A. Jones
 File Name	: MemLeakDetect.h
 Date		: July 30, 2004
 Synopsis	:		 
			A trace memory feature for source code to trace and
			find memory related bugs. 

 Future		:
				1) Memory corruption
				2) Freeing memory without allocating
				3) Freeing memory twice
				4) Not Freeing memory at all
				5) over running memory boundardies


				July 2009: Mods by tim.s.stevens@bt.com (UNICODE/ANSI, more secure CRT with VS 2008).
****************************************************************/

#if !defined(MEMLEAKDETECT_H)
#define MEMLEAKDETECT_H
// This lot only works in _DEBUG builds, since the Windows APIs are debug-only.
#ifdef _DEBUG1

#define _CRTDBG_MAP_ALLOC
#include <map>
#define _CRTBLD
#include <windows.h>

#include <..\crt\src\dbgint.h>
#include <ImageHlp.h>
#include <crtdbg.h>	// Probably in "Microsoft Visual Studio 9.0\VC\crt\src"

#pragma comment( lib, "imagehlp.lib" )

using namespace std;

// if you want to use the custom stackwalker otherwise
// comment this line out
#define MLD_CUSTOMSTACKWALK			1
//
#define MLD_MAX_NAME_LENGTH			256
#define MLD_MAX_TRACEINFO			256
#define MLD_TRACEINFO_EMPTY			_T("")
#define MLD_TRACEINFO_NOSYMBOL		_T("?(?)")

#ifdef  MLD_CUSTOMSTACKWALK
#define MLD_STACKWALKER				symStackTrace2
#else
#define MLD_STACKWALKER				symStackTrace
#endif

#define AfxTrace MyTrace

typedef DWORD ADDR;

/*!
\class CMemLeakDetect
Memory leak catcher.
*/
class CMemLeakDetect
{
	public:

		typedef struct 	{
				ADDRESS				addrPC;
				ADDRESS				addrFrame;
			
		} STACKFRAMEENTRY;

		typedef struct { 
				void*				address;
				DWORD				size;
				TCHAR				fileName[MLD_MAX_NAME_LENGTH];
				DWORD				lineNumber;
				DWORD				occurance;
				STACKFRAMEENTRY		traceinfo[MLD_MAX_TRACEINFO];

		} AllocBlockInfo;

		//typedef int POSITION;
		typedef map<LPVOID, AllocBlockInfo>				KEYMAP;
		typedef map<LPVOID, AllocBlockInfo>::iterator	POSITION;
		typedef pair<LPVOID, AllocBlockInfo>			KEYVALUE;

		class CMapMem
		{
			public:

				KEYMAP			m_Map;
				POSITION		m_Pos;

				inline BOOL Lookup(LPVOID pAddr,  AllocBlockInfo& aInfo) { 

					m_Pos = m_Map.find(pAddr);
					//
					if (m_Pos == m_Map.end())
					{
						return FALSE;
					}
					//
					pAddr = m_Pos->first;
					aInfo = m_Pos->second;

					return TRUE;
				};

				inline POSITION end() { 

					return m_Map.end(); 
				};

				inline void RemoveKey(LPVOID pAddr) { 
					
					m_Map.erase(pAddr);
				};

				inline void RemoveAll() {
					m_Map.clear();
				};

				void SetAt(LPVOID pAddr, AllocBlockInfo& aInfo) {

					m_Map[pAddr] = aInfo;
				};

				inline POSITION GetStartPosition() { 
					POSITION pos = m_Map.begin(); 
					return pos;
				};

				inline void GetNextAssoc(POSITION& pos, LPVOID& rAddr, AllocBlockInfo& aInfo) {

					rAddr = pos->first;
					aInfo = pos->second;
					pos++;
				};

				void InitHashTable(int preAllocEntries, BOOL flag) 	{
					 preAllocEntries	= NULL;
					 flag				= NULL;
				};

		};

		CMemLeakDetect();
		~CMemLeakDetect();
		void Init();
		void End();
		void addMemoryTrace(void* addr,  DWORD asize,  TCHAR *fname, DWORD lnum);
		void redoMemoryTrace(void* addr,  void* oldaddr, DWORD asize,  TCHAR *fname, DWORD lnum);
		void removeMemoryTrace(void* addr, void* realdataptr);
		void cleanupMemoryTrace();
		void dumpMemoryTrace();
		//

		//CMap<LPVOID, LPVOID, AllocBlockInfo, AllocBlockInfo> m_AllocatedMemoryList;
		CMapMem			 m_AllocatedMemoryList;
	DWORD memoccurance;
	bool  isLocked;
	//
	private:

		BOOL initSymInfo(TCHAR* lpUserPath);
		BOOL cleanupSymInfo();
		void symbolPaths( TCHAR* lpszSymbolPaths, UINT BufSizeTCHARs);
		void symStackTrace(STACKFRAMEENTRY* pStacktrace);
		void symStackTrace2(STACKFRAMEENTRY* pStacktrace);
		BOOL symFunctionInfoFromAddresses(ULONG fnAddress, ULONG stackAddress, TCHAR* lpszSymbol, UINT BufSizeTCHARs);
		BOOL symSourceInfoFromAddress(UINT address, TCHAR* lpszSourceInfo, UINT BufSizeTCHARs);
		BOOL symModuleNameFromAddress(UINT address, TCHAR* lpszModule, UINT BufSizeTCHARs);

		HANDLE				m_hProcess;
		PIMAGEHLP_SYMBOL	m_pSymbol;
		DWORD				m_dwsymBufSize;
};
#endif
#endif

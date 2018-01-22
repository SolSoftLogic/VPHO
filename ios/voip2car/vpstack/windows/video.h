#ifndef _WIN32_WCE
#include <multimon.h>
#include <vfw.h>
#include <ddraw.h>
#else
#include <ddraw.h>
#endif

#define OUTMODE_VFW 0
#define OUTMODE_DD 1
#define OUTMODE_OVERLAY 2
#define OUTMODE_RGB24 0
#define OUTMODE_RGB16 0x10
#define OUTMODE_RGB15 0x20
#define OUTMODE_YUY2 0x30
#define OUTMODE_YV12 0x40
#define OUTMODE_UYVY 0x50
#define OUTMODE_FOURCCMASK 0xfffffff0

class VIDEOOUT {
public:
	VIDEOOUT();
	~VIDEOOUT();
	int Open(HWND hParent, TCHAR *wndname, int outmode, int x, int y, int w, int h);
	void FrameData(void *fd) { framedata = fd; }
	void Frame(void *data);
	void Close(bool closewnd = true);
	int SetFullScreen(int fs);

	HWND hWnd, hWndParent;
	int brightness, contrast;
	bool held, fullscreen, contextmenu, dontcorrectaspectratio;

protected:
	LPDIRECTDRAWSURFACE dds, dds_primary;
	LPDIRECTDRAWCLIPPER clp;
	LPDIRECTDRAW dd;
	int nmonitors;
	GUID mguids[4];
	RECT mrects[4];
#ifndef _WIN32_WCE
	HDRAWDIB hdd;
#endif
	int width, height;
	struct {
		BITMAPINFOHEADER bih;
		DWORD colors[3];
	} bi;
	int ddrc, usedmonitor, framesize;
	bool usecolorkey, clipperset, alwaysontop, framedatainit;
	int outmode;
	void *framedata;
	RECT wrect;
	bool hasoverlay, hasyuy2, hasyv12, hasuyvy, hasdestoverlay;

	int CheckDD();
	void CorrectAspectRatio(RECT *rect);
	int OpenDDWindow(HWND hParent, TCHAR *wndname, int outmode, int x, int y, int w, int h, bool openwnd);
	int DrawBitmapToDD(void *data);
	int OpenVFWWindow(HWND hParent, TCHAR *wndname, int x, int y, int w, int h);
	int DrawBitmapToVFW(void *data);
	LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	friend static LRESULT APIENTRY voProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	friend static BOOL WINAPI DDEnumCallbackEx(GUID FAR *lpGUID, LPSTR  lpDriverDescription, LPSTR  lpDriverName, LPVOID lpContext, HMONITOR hm);
};

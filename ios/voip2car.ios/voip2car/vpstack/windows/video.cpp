#include <windows.h>
#ifndef _WIN32_WCE
#define COMPILE_MULTIMON_STUBS
#include <multimon.h>
#endif
#include <stdio.h>
#include <math.h>
#include <tlhelp32.h>
#include "video.h"

#ifdef _WIN32_WCE
#define DDSCAPS_OFFSCREENPLAIN 0
#define DDBLT_WAIT 0
#define DDCKEYCAPS_DESTOVERLAY 0
#define DDOVER_DDFX 0
#define DDLOCK_NOSYSLOCK 0
#define DDLOCK_WAIT 0
#define DDBLT_ASYNC 0
#endif

HBITMAP hHoldBmp;

void yuv2rgb(unsigned char *yuv, unsigned char *rgb, int xres, int yres);
void yuv2rgbdownsample(unsigned char *yuv, unsigned char *rgb, int xres, int yres);
void yuy2torgb(unsigned char *yuy2, unsigned char *rgb, int xres, int yres);

#define KEYCOLOR RGB(10, 0, 10)

static LRESULT APIENTRY voProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	VIDEOOUT *videoout;

	if(uMsg == WM_CREATE)
		SetWindowLong(hWnd, GWL_USERDATA, (LONG)((LPCREATESTRUCT)lParam)->lpCreateParams);
	videoout = (VIDEOOUT *)GetWindowLong(hWnd, GWL_USERDATA);
	if(uMsg == WM_APP)
		DestroyWindow(hWnd);
	if(videoout)
		return videoout->WndProc(hWnd, uMsg, wParam, lParam);
	else return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

static HRESULT (WINAPI *DirectDrawCreate1)(GUID *lpGUID, LPDIRECTDRAW *lplpDD, IUnknown *pUnkOuter);
static HRESULT (WINAPI *DirectDrawEnumerateEx1)(LPDDENUMCALLBACKEX lpCallback, LPVOID lpContext, DWORD dwFlags);

//#include "printcaps.h"

static BOOL WINAPI DDEnumCallbackEx(GUID FAR *lpGUID, LPSTR  lpDriverDescription, LPSTR  lpDriverName, LPVOID lpContext, HMONITOR hm)
{
	MONITORINFO mi;
	VIDEOOUT *vo = (VIDEOOUT *)lpContext;

	if(lpGUID && hm && vo->nmonitors < 4)
	{
		vo->mguids[vo->nmonitors] = *lpGUID;
		mi.cbSize = sizeof(mi);
		GetMonitorInfo(hm, &mi);
		vo->mrects[vo->nmonitors] = mi.rcMonitor;
		vo->nmonitors++;
	}
	return DDENUMRET_OK;
}

static int InitDirectDraw()
{
	HMODULE hLib;
	static bool alreadycalled;

	if(alreadycalled)
		return DirectDrawCreate1 ? 0 : -1;
	alreadycalled = true;
	hLib = LoadLibrary(TEXT("DDRAW.DLL"));
	if(!hLib)
		return -1;
	if(!(DirectDrawCreate1 = (HRESULT (WINAPI *)(GUID *, LPDIRECTDRAW *, IUnknown *))GetProcAddress(hLib, TEXT("DirectDrawCreate"))))
		return -1;
#ifndef _WIN32_WCE
	if(!(DirectDrawEnumerateEx1 = (HRESULT (WINAPI *)(LPDDENUMCALLBACKEX, LPVOID, DWORD))GetProcAddress(hLib, TEXT("DirectDrawEnumerateExA"))))
		return -1;
#endif
	return 0;
}

int VIDEOOUT::CheckDD()
{
	struct {	// Windows CE 6.0 does not work properly here, it ignores dwSize and
				// overwrites other 16 bytes, as it's longer there than in WCE 5.2
		DDCAPS ddc;
		char dummy[16];
	} ddc;
	LPDIRECTDRAW dd;

	nmonitors = 0;
#ifndef _WIN32_WCE
	if(DirectDrawEnumerateEx1(DDEnumCallbackEx, this, DDENUM_ATTACHEDSECONDARYDEVICES))
		return -1;
#endif
	if(DirectDrawCreate1(0, &dd, 0))
		return -1;
    ZeroMemory(&ddc, sizeof(ddc));
    ddc.ddc.dwSize = sizeof(ddc.ddc);
	if(dd->GetCaps(&ddc.ddc, 0))
	{
		dd->Release();
		return -1;
	}
//	printcaps(&ddc.ddc);
	if(ddc.ddc.dwCKeyCaps & DDCKEYCAPS_DESTOVERLAY)
		hasdestoverlay = true;
#ifdef _WIN32_WCE
	if(ddc.ddc.dwOverlayCaps & DDOVERLAYCAPS_OVERLAYSUPPORT)
#else
	if(ddc.ddc.dwCaps & DDCAPS_OVERLAY)
#endif
		hasoverlay = true;
#ifndef _WIN32_WCE
	if(ddc.ddc.dwCaps & DDCAPS_BLTFOURCC && ddc.ddc.dwCaps & DDCAPS_BLTSTRETCH)
#endif
	{
		DWORD numcodes = 0, *codes, i;

		if(!dd->GetFourCCCodes(&numcodes, 0) && numcodes)
		{
			codes = (DWORD *)malloc(4 * numcodes);
			if(!dd->GetFourCCCodes(&numcodes, codes))
				for(i = 0; i < numcodes; i++)
				{
					if(codes[i] == mmioFOURCC('U', 'Y', 'V', 'Y'))
						hasuyvy = true;
					if(codes[i] == mmioFOURCC('Y', 'U', 'Y', '2'))
						hasyuy2 = true;
					if(codes[i] == mmioFOURCC('Y', 'V', '1', '2'))
						hasyv12 = true;
				}
			free(codes);
		}
	}
	dd->Release();
	return 0;
}

VIDEOOUT::VIDEOOUT()
{
	static bool wndregistered;

	dd = 0;
	dds = 0;
#ifndef _WIN32_WCE
	hdd = 0;
#endif
	dds_primary = 0;
	clp = 0;
	hWnd = 0;
	held = false;
	outmode = 0;
	fullscreen = false;
	dontcorrectaspectratio = false;
	contrast = brightness = 500;
	framedata = 0;
	framedatainit = false;
	contextmenu = true;
	nmonitors = 0;
	usedmonitor = -1;
	hasoverlay = hasyuy2 = hasyv12 = hasuyvy = hasdestoverlay = false;
	if(!InitDirectDraw())
		ddrc = CheckDD();
	else ddrc = -1;
	if(!wndregistered)
	{
		WNDCLASS wndclass;

		ZeroMemory(&wndclass, sizeof(wndclass));
		wndclass.lpszClassName = TEXT("VIDEOOUT");
		wndclass.lpfnWndProc = voProc;
		wndclass.style = CS_DBLCLKS;
		wndclass.hCursor = LoadCursor(0, IDC_ARROW);
		wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wndclass.hInstance = GetModuleHandle(0);
		RegisterClass(&wndclass);
		wndregistered = true;
	}
}

VIDEOOUT::~VIDEOOUT()
{
	Close();
	if(hWnd)
		SetWindowLong(hWnd, GWL_USERDATA, 0);
}

#ifndef _WIN32_WCE
int VIDEOOUT::OpenVFWWindow(HWND hParent, char *wndname, int x, int y, int w, int h)
{
	RECT rect;

	hdd = DrawDibOpen();
	if(!hdd)
		return -1;
	rect.top = rect.left = 0;
	rect.right = w;
	rect.bottom = h;
	AdjustWindowRect(&rect, WS_CHILD, FALSE);

	hWnd = CreateWindow("VIDEOOUT", wndname, WS_CHILD | WS_VISIBLE, x, y, rect.right - rect.left, rect.bottom - rect.top, hParent, 0, GetModuleHandle(0), this);
	return 0;
}

int VIDEOOUT::DrawBitmapToVFW(void *data)
{
	HDC hdc;
	RECT rect;
	int value, contrastmultiplier, brightnessoffset, i;

	if(!hdd || !hWnd)
		return -1;
	GetClientRect(hWnd, &rect);
	hdc = GetDC(hWnd);
	if((outmode & OUTMODE_FOURCCMASK) == OUTMODE_RGB24 && (brightness != 500 || contrast != 500))
	{
		brightnessoffset = (brightness - 500) / 5;
		contrastmultiplier = (int)(exp((contrast - 500) * 0.002) * 1000);
		for(i = 0; i < (int)bi.bih.biSizeImage; i++)
		{
			value = ((BYTE *)data)[i];
			value = value * contrastmultiplier / 1000 + brightnessoffset;
			if(value < 0)
				value = 0;
			if(value > 255)
				value = 255;
			((BYTE *)data)[i] = value;
		}
	}
	CorrectAspectRatio(&rect);
	DrawDibDraw(hdd, hdc, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, &bi.bih, data, 0, 0, width, height, 0);
	ReleaseDC(hWnd, hdc);
	return 0;
}
#endif

int VIDEOOUT::OpenDDWindow(HWND hParent, TCHAR *wndname, int outmode, int x, int y, int w, int h, bool createwnd)
{
	DDSURFACEDESC ddsd;
	DDPIXELFORMAT ddpf;
	RECT rect, rect2;
	DDBLTFX bltFX;
	int rc, i, highestarea, bestmonitor;
	HRESULT hr;

	if(ddrc || (outmode & OUTMODE_OVERLAY && !hasoverlay))
		return -1;

	// Find the monitor that contains the most of the window
	if(!createwnd && hWnd)
		GetWindowRect(hWnd, &rect);
	else {
		rect.left = x;
		rect.top = y;
		rect.right = x + w;
		rect.bottom = y + h;
		MapWindowPoints(hParent, 0, (POINT *)&rect, 2);
	}
	bestmonitor = -1;
	highestarea = 0;
	for(i = 0; i < nmonitors; i++)
	{
		IntersectRect(&rect2, &rect, &mrects[i]);
		if((rect2.right - rect2.left) * (rect2.bottom - rect2.top) > highestarea)
		{
			highestarea = (rect2.right - rect2.left) * (rect2.bottom - rect2.top);
			bestmonitor = i;
		}
	}
#ifdef _WIN32_WCE
	bestmonitor = -1;
#endif
	if(usedmonitor == bestmonitor && dds)
		return 0;
	usedmonitor = bestmonitor;
	if(dds)
		Close(false);
	if(bestmonitor >= 0)
	{
		if(DirectDrawCreate1(&mguids[bestmonitor], &dd, 0))
			return -1;
	} else if(DirectDrawCreate1(0, &dd, 0))
		return -1;

	if(hr = dd->SetCooperativeLevel(hWnd, DDSCL_NORMAL))
	{
		dd->Release();
		dd = 0;
		return -1;
	}
	ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	if(hr = dd->CreateSurface(&ddsd, &dds_primary, 0))
	{
		dd->Release();
		dd = 0;
		return -1;
	}
	rect.top = rect.left = 0;
	rect.right = w;
	rect.bottom = h;
	rect2 = rect;
	AdjustWindowRectEx(&rect, WS_CHILD, FALSE, 0);
	width = w;
	height = h;
	ZeroMemory(&ddpf, sizeof(ddpf));
	ddpf.dwSize = sizeof(ddpf);
	if((outmode & OUTMODE_FOURCCMASK) == OUTMODE_RGB24)
	{
		ddpf.dwFlags = DDPF_RGB;
		ddpf.dwFourCC = 0;
		ddpf.dwRGBBitCount = 24;
		ddpf.dwRBitMask = 0xff0000;
		ddpf.dwGBitMask = 0x00ff00;
		ddpf.dwBBitMask = 0x0000ff;
		ddpf.dwRGBAlphaBitMask = 0;
	} else if((outmode & OUTMODE_FOURCCMASK) == OUTMODE_RGB16)
	{
		ddpf.dwFlags = DDPF_RGB;
		ddpf.dwFourCC = 0;
		ddpf.dwRGBBitCount = 16;
		ddpf.dwRBitMask = 0xf800;
		ddpf.dwGBitMask = 0x07e0;
		ddpf.dwBBitMask = 0x001f;
		ddpf.dwRGBAlphaBitMask = 0;
	} else {
		ddpf.dwFlags = DDPF_FOURCC;
		if((outmode & OUTMODE_FOURCCMASK) == OUTMODE_YUY2)
			ddpf.dwFourCC = mmioFOURCC('Y', 'U', 'Y', '2');
		else if((outmode & OUTMODE_FOURCCMASK) == OUTMODE_YV12)
			ddpf.dwFourCC = mmioFOURCC('Y', 'V', '1', '2');
		else if((outmode & OUTMODE_FOURCCMASK) == OUTMODE_UYVY)
			ddpf.dwFourCC = mmioFOURCC('U', 'Y', 'V', 'Y');
	}
    ZeroMemory(&ddsd, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
	ddsd.ddsCaps.dwCaps = (outmode & OUTMODE_OVERLAY ? DDSCAPS_OVERLAY : DDSCAPS_OFFSCREENPLAIN) | DDSCAPS_VIDEOMEMORY;
    ddsd.dwBackBufferCount = 0;
    ddsd.dwWidth = w;
    ddsd.dwHeight = h;
    ddsd.ddpfPixelFormat = ddpf;
	if(rc = dd->CreateSurface(&ddsd, &dds, 0))
	{
		ddsd.dwFlags &= ~DDSD_CAPS;
		if((outmode & OUTMODE_OVERLAY) || (rc = dd->CreateSurface(&ddsd, &dds, 0)))
		{
			dds_primary->Release();
			dds_primary = 0;
			dd->Release();
			dd = 0;
			return -1;
		}
	}
	ZeroMemory(&bltFX, sizeof(bltFX));
	bltFX.dwSize = sizeof(bltFX);
	bltFX.dwFillColor = KEYCOLOR;
	dds->Blt(&rect2, 0, &rect2, DDBLT_COLORFILL | DDBLT_WAIT, &bltFX);
	dd->CreateClipper(0, &clp, 0);
	if(outmode & OUTMODE_OVERLAY && hasdestoverlay)
		usecolorkey = true;
	else usecolorkey = false;
	clipperset = true;
	if(createwnd)
		hWnd = CreateWindow(TEXT("VIDEOOUT"), wndname, WS_CHILD | WS_VISIBLE, x, y, rect.right - rect.left, rect.bottom - rect.top, hParent, 0, GetModuleHandle(0), this);
	if(clp)
	{
		clp->SetHWnd(0, hWnd);
		if(!(outmode & OUTMODE_OVERLAY) || !usecolorkey)
			dds_primary->SetClipper(clp);
		else clipperset = false;
	}
	return 0;
}

int VIDEOOUT::Open(HWND hParent, TCHAR *wndname, int outmode, int x, int y, int w, int h)
{
	if(!hasoverlay && outmode & OUTMODE_OVERLAY)
		return -1;
	if(!hasyuy2 && (outmode & OUTMODE_FOURCCMASK) == OUTMODE_YUY2)
		return -1;
	if(!hasyv12 && (outmode & OUTMODE_FOURCCMASK) == OUTMODE_YV12)
		return -1;
	if(!hasuyvy && (outmode & OUTMODE_FOURCCMASK) == OUTMODE_UYVY)
		return -1;
	hWndParent = hParent;
	width = w;
	height = h;

	ZeroMemory(&bi, sizeof(bi));
	bi.bih.biSize = sizeof(bi.bih);
	bi.bih.biPlanes = 1;
	framesize = 2*w*h;
	switch(outmode & OUTMODE_FOURCCMASK)
	{
	case OUTMODE_RGB24:
		bi.bih.biBitCount = 24;
		bi.bih.biCompression = BI_RGB;
		bi.bih.biSizeImage = 3 * w * h;
		framesize = 3*w*h;
		break;
	case OUTMODE_RGB16:
		bi.bih.biBitCount = 16;
		bi.bih.biCompression = BI_BITFIELDS;
		bi.bih.biSizeImage = 2 * w * h;
		bi.bih.biClrUsed = 3;
		bi.colors[0] = 0xf800;
		bi.colors[1] = 0x7e0;
		bi.colors[2] = 0x1f;
		break;
	case OUTMODE_RGB15:
		bi.bih.biBitCount = 16;
		bi.bih.biCompression = BI_RGB;
		bi.bih.biSizeImage = 2 * w * h;
		break;
	case OUTMODE_YUY2:
	case OUTMODE_UYVY:
		bi.bih.biBitCount = 24;
		bi.bih.biCompression = BI_RGB;
		bi.bih.biSizeImage = 3 * w * h;
		break;
	}
	if(framedata)
		free(framedata);
	framedata = malloc(framesize);
	bi.bih.biWidth = w;
	bi.bih.biHeight = h;
	
	this->outmode = outmode;
	if(outmode & OUTMODE_DD)
	{
		if(!OpenDDWindow(hParent, wndname, outmode, x, y, w, h, true))
		{
			if(hParent)
			{
				RECT rect;

				GetClientRect(hParent, &rect);
				SetWindowPos(hWnd, 0, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);
			}
			return 0;
		}
	}
#ifndef _WIN32_WCE
	else if(!(outmode & OUTMODE_OVERLAY) && (outmode & OUTMODE_FOURCCMASK) <= OUTMODE_YUY2 && (outmode & OUTMODE_FOURCCMASK) != OUTMODE_RGB16)
	{
		if(!OpenVFWWindow(hParent, wndname, x, y, w, h))
		{
			if(hParent)
			{
				RECT rect;

				GetClientRect(hParent, &rect);
				SetWindowPos(hWnd, 0, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);
			}
			return 0;
		}
	}
#endif
	return -1;
}

void VIDEOOUT::Frame(void *data)
{
	if(data != framedata && framedata)
	{
		memcpy(framedata, data, framesize);
		framedatainit = true;
	}
#ifndef _WIN32_WCE
	if(!(outmode & OUTMODE_DD))
	{
		if((outmode & OUTMODE_FOURCCMASK) < OUTMODE_YUY2)
			DrawBitmapToVFW(data);
		else if((outmode & OUTMODE_FOURCCMASK) == OUTMODE_YUY2)
		{
			unsigned char *buf = (unsigned char *)malloc(width*height*3);
			yuy2torgb((unsigned char *)data, buf, width, height);
			DrawBitmapToVFW(buf);
			free(buf);
		}
	} else
#endif		
		DrawBitmapToDD(data);
}

void VIDEOOUT::Close(bool closewnd)
{
	if(hWnd && closewnd)
	{
		PostMessage(hWnd, WM_APP, 0, 0);
		hWnd = 0;
	}
	if(dds)
	{
		dds->Release();
		dds = 0;
	}
	if(clp)
	{
		clp->Release();
		clp = 0;
	}
	if(dds_primary)
	{
		dds_primary->Release();
		dds_primary = 0;
	}
	if(dd)
	{
		dd->Release();
		dd = 0;
	}
	if(framedata)
	{
		free(framedata);
		framedata = 0;
	}
#ifndef _WIN32_WCE
	if(hdd)
	{
		DrawDibEnd(hdd);
		DrawDibClose(hdd);
		hdd = 0;
	}
#endif
}

static int getchildrect(HWND hWnd, RECT *rect)
{
	HWND hParent = GetParent(hWnd);

	if(!hParent)
		return 0;
	GetWindowRect(hWnd, rect);
	MapWindowPoints(0, hParent, (LPPOINT)rect, 2);
	return 1;
}

int VIDEOOUT::SetFullScreen(int fs)
{
	RECT rect;

	if(!hWnd)
		return -1;
	fullscreen = !!fs;
	if(fullscreen)
	{
		if(alwaysontop)
			SetWindowPos(GetParent(hWnd), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
 		getchildrect(hWnd, &wrect);
		if(usedmonitor >= 0)
			rect = mrects[usedmonitor];
		else {
			rect.left = 0;
			rect.top = 0;
			rect.right = GetSystemMetrics(SM_CXSCREEN);
			rect.bottom = GetSystemMetrics(SM_CYSCREEN);
		}
		ShowWindow(hWndParent, SW_HIDE);
		SetParent(hWnd, 0);
		SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) & ~WS_CHILD | WS_POPUP);
		MoveWindow(hWnd, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, TRUE);
	} else {
		SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) & ~WS_POPUP | WS_CHILD);
		SetParent(hWnd, hWndParent);
		ShowWindow(hWndParent, SW_SHOW);
		MoveWindow(hWnd, wrect.left, wrect.top, wrect.right - wrect.left, wrect.bottom - wrect.top, TRUE);
		if(alwaysontop)
			SetWindowPos(GetParent(hWnd), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
	return 0;
}

LRESULT VIDEOOUT::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RECT rect, srcrect;
	POINT pt;
	DDOVERLAYFX overlayFX;
	PAINTSTRUCT ps;
	HDC hdc, hdcmem;
	DDBLTFX bltFX;
	HMENU hmenu;
	HWND hWndPt;
	unsigned flags;
	static unsigned prevflags;

	switch(uMsg)
	{
	case WM_CREATE:
		alwaysontop = false;
#ifdef _WIN32_WCE
		SetTimer(hWnd, 1, 100, 0);
#endif
		break;
	case WM_TIMER:
		GetWindowRect(hWnd, &rect);
		hWndPt = WindowFromPoint(*(POINT *)&rect);
		flags = hWndPt == hWnd ? DDOVER_SHOW : DDOVER_HIDE;
		if(prevflags != flags)
		{
			prevflags = flags;
			SendMessage(hWnd, WM_SIZE, 0, 0);
		}
		break;
	case WM_SIZE:
	case WM_MOVE:
		if(outmode & OUTMODE_DD)
		{
			TCHAR wndname[300];

			GetClientRect(hWnd, &rect);
			GetWindowText(hWnd, wndname, sizeof(wndname) / sizeof(*wndname));
			OpenDDWindow(GetParent(hWnd), wndname, outmode, 0, 0, width, height, false);
		}
		if(outmode & OUTMODE_OVERLAY && dds)
		{
#ifdef _WIN32_WCE
			GetWindowRect(hWnd, &rect);
			hWndPt = WindowFromPoint(*(POINT *)&rect);
			flags = hWndPt == hWnd ? DDOVER_SHOW : DDOVER_HIDE;
#else
			flags = DDOVER_SHOW;
#endif
			GetClientRect(hWnd, &rect);
			pt.x = pt.y = 0;
			ClientToScreen(hWnd, &pt);
			if(usedmonitor >= 0)
			{
				pt.x -= mrects[usedmonitor].left;
				pt.y -= mrects[usedmonitor].top;
			}
			rect.left = pt.x;
			rect.top = pt.y;
			rect.right += pt.x;
			rect.bottom += pt.y;
			CorrectAspectRatio(&rect);
			srcrect.left = srcrect.top = 0;
			srcrect.right = width;
			srcrect.bottom = height;
			ZeroMemory(&overlayFX, sizeof(overlayFX));
			overlayFX.dwSize = sizeof(overlayFX);
			overlayFX.dckDestColorkey.dwColorSpaceLowValue = KEYCOLOR;
			overlayFX.dckDestColorkey.dwColorSpaceHighValue = KEYCOLOR;
			dds->UpdateOverlay(&srcrect, dds_primary, &rect, (usecolorkey ? DDOVER_DDFX : 0) | flags, &overlayFX);
#ifdef _WIN32_WCE	// UpdateOverlay does not work on Eanovo
			dds->SetOverlayPosition(pt.x, pt.y);
#endif
		}
		if(uMsg == WM_SIZE)
			InvalidateRect(hWnd, 0, TRUE);
		break;
	case WM_PAINT:
		GetClientRect(hWnd, &rect);
		hdc = BeginPaint(hWnd, &ps);
		if(held)
		{
			hdcmem = CreateCompatibleDC(hdc);
			SelectObject(hdcmem, hHoldBmp);
			StretchBlt(hdc, 0, 0, rect.right, rect.bottom, hdcmem, 0, 0, 176, 144, SRCCOPY);
			DeleteDC(hdcmem);
		} else if(outmode & OUTMODE_DD && dds)
		{
			if(!clipperset && clp)
				dds_primary->SetClipper(clp);
			pt.x = ps.rcPaint.left;
			pt.y = ps.rcPaint.top;
			ClientToScreen(hWnd, &pt);
			rect.left = pt.x;
			rect.top = pt.y;
			pt.x = ps.rcPaint.right;
			pt.y = ps.rcPaint.bottom;
			ClientToScreen(hWnd, &pt);
			rect.right = pt.x;
			rect.bottom = pt.y;
			ZeroMemory(&bltFX, sizeof(bltFX));
			bltFX.dwSize = sizeof(bltFX);
			bltFX.dwFillColor = KEYCOLOR;
			dds_primary->Blt(&rect, 0, &rect, DDBLT_COLORFILL | DDBLT_WAIT, &bltFX);
			if(!clipperset && clp)
				dds_primary->SetClipper(0);
		}
		if(framedatainit)
			Frame(framedata);
		EndPaint(hWnd, &ps);
		break;
	case WM_LBUTTONDBLCLK:
		SetFullScreen(!fullscreen);
		break;
	case WM_CONTEXTMENU:
		if(!contextmenu)
			break;
		hmenu = CreatePopupMenu();
		if(alwaysontop)
			AppendMenu(hmenu, 0, 101, TEXT("Unset always on top"));
		else AppendMenu(hmenu, 0, 100, TEXT("Always on top"));
		TrackPopupMenu(hmenu, 0, LOWORD(lParam), HIWORD(lParam), 0, hWnd, 0);
		DestroyMenu(hmenu);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case 100:
			SetWindowPos(GetParent(hWnd), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			alwaysontop = true;
			break;
		case 101:
			SetWindowPos(GetParent(hWnd), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			alwaysontop = false;
			break;
		}
		break;
	case WM_DESTROY:
		break;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

void VIDEOOUT::CorrectAspectRatio(RECT *rect)
{
	int w = width, h = height, ww;

	if(dontcorrectaspectratio)
		return;
	if(w == 176 && h == 144 || w == 352 && h == 288 || w == 720 && h == 576 || w == 704 && h == 576)	
	{
		w = 4;
		h = 3;
	}
	if((rect->right-rect->left) * h > w * (rect->bottom - rect->top))
	{
		ww = rect->right-rect->left;
		rect->left += (ww - (rect->bottom - rect->top) * w / h) / 2;
		rect->right = rect->left + (rect->bottom - rect->top) * w / h;
	} else if((rect->right-rect->left) * h < w * (rect->bottom - rect->top))
	{
		ww = rect->bottom-rect->top;
		rect->top += (ww - (rect->right - rect->left) * h / w) / 2;
		rect->bottom = rect->top + (rect->right - rect->left) * h / w;
	}
}

int VIDEOOUT::DrawBitmapToDD(void *data)
{
	DDSURFACEDESC ddsd;
	RECT rectsrc, rectdest, rectwnd;
	POINT pt;
	int value, contrastmultiplier, brightnessoffset, i, j, yoffs, uoffs, voffs;

	if(!dds || !hWnd)
		return -1;
	ddsd.dwSize = sizeof(ddsd);
	if((outmode & OUTMODE_FOURCCMASK) < OUTMODE_YUY2)
	{
		HDC hdc, hdcmem;
		HBITMAP hBmp;

		if((outmode & OUTMODE_FOURCCMASK) == OUTMODE_RGB24 && (brightness != 500 || contrast != 500))
		{
			brightnessoffset = (brightness - 500) / 5;
			contrastmultiplier = (int)(exp((contrast - 500) * 0.002) * 1000);
			for(i = 0; i < (int)bi.bih.biSizeImage; i++)
			{
				value = ((BYTE *)data)[i];
				value = value * contrastmultiplier / 1000 + brightnessoffset;
				if(value < 0)
					value = 0;
				if(value > 255)
					value = 255;
				((BYTE *)data)[i] = value;
			}
		}
		if(0)
		{
			int rc = dds->GetDC(&hdc);
	#ifdef _WIN32_WCE
			void *data2 = 0;

			hBmp = CreateDIBSection(hdc, (BITMAPINFO *)&bi, DIB_RGB_COLORS, &data2, 0, 0);
			if(hBmp && data2)
				memcpy(data2, data, bi.bih.biSizeImage);
	#else
			hBmp = CreateDIBitmap(hdc, &bi.bih, CBM_INIT, data, (BITMAPINFO *)&bi.bih, 0);
	#endif
			if(hBmp)
			{
				hdcmem = CreateCompatibleDC(hdc);
				SelectObject(hdcmem, hBmp);
				BitBlt(hdc, 0, 0, width, height, hdcmem, 0, 0, SRCCOPY);
				DeleteDC(hdcmem);
				DeleteObject(hBmp);
			}
			dds->ReleaseDC(hdc);
		}
		if(dds->Lock(0, &ddsd, DDLOCK_NOSYSLOCK | DDLOCK_WAIT, 0))
			return -1;
		if((outmode & OUTMODE_FOURCCMASK) == OUTMODE_RGB24)
		{
			for(i = 0; i < height; i++)
				memcpy((BYTE *)ddsd.lpSurface + i * ddsd.lPitch, (BYTE *)data + (height-1-i) * 3 * width, 3 * width);
		} else {
			 for(i = 0; i < height; i++)
				memcpy((BYTE *)ddsd.lpSurface + i * ddsd.lPitch, (BYTE *)data + (height-1-i) * 2 * width, 2 * width);
		}
		dds->Unlock(0);
	} else {
		if(dds->Lock(0, &ddsd, DDLOCK_NOSYSLOCK | DDLOCK_WAIT, 0))
			return -1;
		if(brightness != 500 || contrast != 500)
		{
			brightnessoffset = (brightness - 500) / 5;
			yoffs = (257 * brightnessoffset + 504 * brightnessoffset + 98 * brightnessoffset) / 1000;
			voffs = (439 * brightnessoffset - 368 * brightnessoffset - 71 * brightnessoffset) / 1000;
			uoffs = (-148 * brightnessoffset - 291 * brightnessoffset + 439 * brightnessoffset) / 1000;
			contrastmultiplier = (int)(exp((contrast - 500) * 0.002) * 1000);
			for(i = 0; i < height; i++)
				for(j = 0; j < width; j++)
				{
					value = ((BYTE *)data)[i * 2 * width + 2 * j] - 16;
					value = value * contrastmultiplier / 1000 + yoffs;
					if(value < 0)
						value = 0;
					if(value > 219)
						value = 219;
					((BYTE *)ddsd.lpSurface)[i * ddsd.lPitch + 2 * j] = value + 16;

					value = ((BYTE *)data)[i * 2 * width + 2 * j + 1] - 128;
					value = value * contrastmultiplier / 1000 + ((j % 2) ? voffs : uoffs);
					if(value < -112)
						value = -112;
					if(value > 111)
						value = 111;
					((BYTE *)ddsd.lpSurface)[i * ddsd.lPitch + 2 * j + 1] = value + 128;
				}
		} else if((outmode & OUTMODE_FOURCCMASK) == OUTMODE_YV12)
		{
			for(i = 0; i < height; i++)
				memcpy((BYTE *)ddsd.lpSurface + i * ddsd.lPitch, (BYTE *)data + i * width, width);
		} else for(i = 0; i < height; i++)
			memcpy((BYTE *)ddsd.lpSurface + i * ddsd.lPitch, (BYTE *)data + i * 2 * width, 2 * width);
		dds->Unlock(0);
	}
	if(!(outmode & OUTMODE_OVERLAY))
	{
		DDSURFACEDESC ddsdp;

		GetClientRect(hWnd, &rectwnd);
		pt.x = pt.y = 0;
		ClientToScreen(hWnd, &pt);
		rectdest = rectwnd;
		if(usedmonitor >= 0)
		{
			pt.x -= mrects[usedmonitor].left;
			pt.y -= mrects[usedmonitor].top;
		}
		rectdest.left = pt.x;
		rectdest.top = pt.y;
		rectdest.right += pt.x;
		rectdest.bottom += pt.y;

		CorrectAspectRatio(&rectdest);

		rectsrc.left = rectsrc.top = 0;
		rectsrc.right = ddsd.dwWidth;
		rectsrc.bottom = ddsd.dwHeight;
		ddsdp.dwSize = sizeof(ddsdp);
		dds_primary->GetSurfaceDesc(&ddsdp);
		if(rectdest.right - rectdest.left)
		{
			if(rectdest.right > (int)ddsdp.dwWidth)
			{
				rectsrc.right = (ddsdp.dwWidth - rectdest.left) * rectsrc.right / (rectdest.right - rectdest.left);
				rectdest.right = ddsdp.dwWidth;
			} else if(rectdest.left < 0)
			{
				rectsrc.left = -rectdest.left * rectsrc.right / (rectdest.right - rectdest.left);
				rectdest.left = 0;
			}
		}
		if(rectdest.bottom - rectdest.top)
		{
			if(rectdest.bottom > (int)ddsdp.dwHeight)
			{
				rectsrc.bottom = (ddsdp.dwHeight - rectdest.top) * rectsrc.bottom / (rectdest.bottom - rectdest.top);
				rectdest.bottom = ddsdp.dwHeight;
			} else if(rectdest.top < 0)
			{
				rectsrc.top = -rectdest.top * rectsrc.bottom / (rectdest.bottom - rectdest.top);
				rectdest.top = 0;
			}
		}
		dds_primary->Blt(&rectdest, dds, &rectsrc, DDBLT_ASYNC, 0);
	}
	return 0;
}

void yuy2torgb(unsigned char *yuy2, unsigned char *rgb, int xres, int yres)
{
	unsigned char *y1;
	unsigned char *rgbbase;
	int x, y, Y00, Y01, YUR, YUG, YUB, U, V, xres2=xres/2;
	register int tmp;

	for(y = 0; y < yres; y++)
		for(x = 0; x < xres2; x++)
		{
			y1 = yuy2 + 4*x+2*y*xres;
			Y00 = (y1[0] - 16) * 298 / 256;
			Y01 = (y1[2] - 16) * 298 / 256;
			U = y1[1] - 128;
			V = y1[3] - 128;

			rgbbase = rgb + xres * (yres-1) * 3 + x * 6 - y * xres * 3;

			// HDTV bis
//			YUR = 351 * V / 256;
//			YUG = (-179 * U - 86 * V) / 256;
//			YUB = 443 * U / 256;

			// HDTV
			YUR = 459 * V / 256;
			YUG = (-137 * U - 55 * V) / 256;
			YUB = 541 * U / 256;

			// SDTV
//			YUR = 409 * V / 256;
//			YUG = (-208 * U - 100 * V) / 256;
//			YUB = 517 * U / 256;


			tmp = Y00 + YUB;
			if(tmp < 0) tmp = 0;
			if(tmp > 255) tmp = 255;
			*rgbbase++ = (unsigned char)tmp;
			tmp = Y00 + YUG;
			if(tmp < 0) tmp = 0;
			if(tmp > 255) tmp = 255;
			*rgbbase++ = (unsigned char)tmp;
			tmp = Y00 + YUR;
			if(tmp < 0) tmp = 0;
			if(tmp > 255) tmp = 255;
			*rgbbase++ = (unsigned char)tmp;

			tmp = Y01 + YUB;
			if(tmp < 0) tmp = 0;
			if(tmp > 255) tmp = 255;
			*rgbbase++ = (unsigned char)tmp;
			tmp = Y01 + YUG;
			if(tmp < 0) tmp = 0;
			if(tmp > 255) tmp = 255;
			*rgbbase++ = (unsigned char)tmp;
			tmp = Y01 + YUR;
			if(tmp < 0) tmp = 0;
			if(tmp > 255) tmp = 255;
			*rgbbase = (unsigned char)tmp;
		}
}

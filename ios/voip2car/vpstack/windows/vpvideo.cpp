#include <stdio.h>
#include "../util.h"
#include "../vpstack.h"
#include "vpvideo.h"

//#define LOSSDEBUG

static VPVIDEO *localvideo;
static tchar videocapturedevice[100];

int VPVIDEO::ninstances;	
VIDEOCODEC *VPVIDEO::encoder;
IVPSTACK *VPVIDEO::vps_in;
VPVIDEO *VPVIDEO::instances[MAXVPVIDEO];
DXCAPTUREFRAME VPVIDEO::dcfb;
static DXCAPTURE *dxcapture;
static CRITICAL_SECTION cs;

struct STACKCALL
{
	STACKCALL(IVPSTACK *a, VPCALL b) { vps = a; vpcall = b; }
	IVPSTACK *vps;
	VPCALL vpcall;
};

void DisconnectSTACKCALL(void *sc)
{
	STACKCALL *s = (STACKCALL *)sc;

	s->vps->Disconnect(s->vpcall, REASON_NORMAL);
	delete s;
}

void NotifyWindowClose(IVPSTACK *vps, VPCALL vpcall)
{
	_beginthread(DisconnectSTACKCALL, 0, new STACKCALL(vps, vpcall));
}

static LRESULT WINAPI VideoWindowWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	VPVIDEO *vpv;

	switch(uMsg)
	{
	case WM_CREATE:
		break;
	case WM_MOVE:
		vpv = (VPVIDEO *)GetWindowLong(hWnd, GWL_USERDATA);
		if(!vpv)
			break;
		SendMessage(vpv->hWndVideoOut, WM_MOVE, 0, 0);
		break;
	case WM_SIZE:
		vpv = (VPVIDEO *)GetWindowLong(hWnd, GWL_USERDATA);
		if(!vpv)
			break;
		MoveWindow(vpv->hWndVideoOut, 0, 0, LOWORD(lParam), HIWORD(lParam), FALSE);
		break;
	case WM_CLOSE:
		vpv = (VPVIDEO *)GetWindowLong(hWnd, GWL_USERDATA);
		if(!vpv)
			break;
		if(vpv->vps)
			NotifyWindowClose(vpv->vps, vpv->vpcall);
		else if(vpv->NotifyRoutine)
			vpv->NotifyRoutine(vpv->notifyparam, VPMSG_WINDOWCLOSED, 0, 0);
		break;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

int VPVIDEO_Init()
{
	static bool init;
	
	if(init)
		return 0;
	init = true;
#ifdef INCLUDEVP3
	VP3CODEC_Init();
#endif
	InitializeCriticalSection(&cs);

	return 0;
}

static void RegisterVideoWindow()
{
	static WNDCLASS wc;

	if(wc.lpfnWndProc)
		return;
	wc.lpfnWndProc = VideoWindowWndProc;
	wc.hInstance = GetModuleHandle(0);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.lpszClassName = TEXT("VIDEOWINDOW");
	RegisterClass(&wc);
}

static void VideoThread(void *vpvd)
{
	((VPVIDEO *)vpvd)->VideoThread();
}

void VPVIDEO::VideoThread()
{
	MSG msg;
	int i;
	HWND hWndToClose;

	if(Create(vps, vpcall, fourccrx, xresrx, yresrx, fourcc, xres, yres, framerate, quality))
	{
		EnterCriticalSection(&cs);
		VPVIDEO::ninstances--;
		if(!VPVIDEO::ninstances)
		{
			if(dxcapture)
			{
				delete dxcapture;
				dxcapture = 0;
			}
			if(VPVIDEO::encoder)
			{
				delete VPVIDEO::encoder;
				VPVIDEO::encoder = 0;
			}
		}
		starting = false;
		initgood = false;
		LeaveCriticalSection(&cs);
		if(NotifyRoutine)
			NotifyRoutine(notifyparam, VPMSG_VIDEOSTARTERROR, (unsigned)vps, (unsigned)vpcall);
		return;
	}
	Lprintf("Video window opened");
	hWndVideoOut = out.hWnd;
	if(!vps)
		localvideo = this;
	starting = false;
	if(NotifyRoutine)
		NotifyRoutine(notifyparam, VPMSG_VIDEOSTARTOK, (unsigned)vps, (unsigned)vpcall);
	SetWindowLong(hWnd, GWL_USERDATA, (LONG)this);
	while(GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	SetWindowLong(hWnd, GWL_USERDATA, 0);
	decoder->End();
	VPVIDEO::ninstances--;
	free(frame);
	for(i = 0; i < MAXVPVIDEO; i++)
		if(VPVIDEO::instances[i] == this)
			VPVIDEO::instances[i] = 0;
	hWndToClose = hWnd;
	hWnd = 0;
	DestroyWindow(hWndToClose);	// This can block, so needs to be after hWnd = 0
	Lprintf("Video window closed");
	EnterCriticalSection(&cs);
	if(VPVIDEO::ninstances == 0)
	{
		Lprintf("Video acquisition closed");
		if(dxcapture)
		{
			delete dxcapture;
			dxcapture = 0;
		}
		if(VPVIDEO::encoder)
		{
			delete VPVIDEO::encoder;
			VPVIDEO::encoder = 0;
		}
	}
	LeaveCriticalSection(&cs);
}

VPVIDEO::VPVIDEO(IVPSTACK *vps, VPCALL vpcall, unsigned fourccrx, unsigned xresrx, unsigned yresrx, unsigned fourcc, unsigned xres, unsigned yres, unsigned framerate, unsigned quality)
{
	if(!vps && localvideo)
	{
		hWnd = 0;
		initgood = false;
		return;
	}
	if(vps)
		vps_in = vps;
	this->vps = vps;
	this->vpcall = vpcall;
	this->fourcc = fourcc;
	this->xres = xres;
	this->yres = yres;
	this->fourccrx = fourccrx;
	this->xresrx = xresrx;
	this->yresrx = yresrx;
	this->framerate = framerate;
	this->quality = quality;
	initgood = true;
	starting = false;
	hidden = false;
	memset(&vrect, 0, sizeof(vrect));
	hParent = 0;
	childid = 0;
	hWnd = 0;
	decoder = 0;
	NotifyRoutine = 0;
	Lprintf("New VPVIDEO, vpstack=%p, vpcall=%d, RX=(%x,%u,%u), TX=(%x,%u,%u,%u,%x)",
		vps, vpcall, fourccrx, xresrx, yresrx, fourcc, xres, yres, framerate, quality);
#ifdef INCLUDEVP3
	if(fourccrx == F_VP31)
		decoder = NewVP3();
#endif
#ifdef INCLUDEXVID
	if(fourccrx == F_XVID)
		decoder = NewXVID();
#endif
#ifdef INCLUDEAVC
	if(fourccrx == F_H264)
		decoder = NewAVC();
#endif
	if(!decoder)
		Lprintf("Error: VPVIDEO created with no decoder");
}

int VPVIDEO::Start()
{
	if(!initgood || starting)
	{
		Lprintf("Error starting VPVIDEO");
		return -1;
	}
	starting = true;
	_beginthread(::VideoThread, 0, this);
	if(vps)
	{
		while(starting)
			Sleep(1);
		if(!initgood)
		{
			Lprintf("Error starting VPVIDEO");
			return -1;
		}
		Lprintf("VPVIDEO started");
	}
	return 0;
}

VPVIDEODATA *VPVIDEOFACTORY::New(IVPSTACK *vps, VPCALL vpcall, unsigned fourccrx, unsigned xresrx, unsigned yresrx, unsigned fourcc, unsigned xres, unsigned yres, unsigned framerate, unsigned quality)
{
	VPVIDEO *vd = new VPVIDEO(vps, vpcall, fourccrx, xresrx, yresrx, fourcc, xres, yres, framerate, quality);
	vd->Start();
	return vd;
}

int VPVIDEO::Create(IVPSTACK *vps, VPCALL vpcall, unsigned fourccrx, unsigned xresrx, unsigned yresrx, unsigned fourcc, unsigned xres, unsigned yres, unsigned framerate, unsigned quality)
{
	int i, rc;
	char name[MAXNAMELEN+1], s[300];
	TCHAR tname[MAXNAMELEN+1];
	RECT rect;
	int mode;

	if(!decoder)
		return -1;
	EnterCriticalSection(&cs);
	ninstances++;
	if(ninstances == 1)
	{
		if(vps)
			VPVIDEO::vps = vps;
#ifdef INCLUDEVP3
		if(fourcc == F_VP31)
			encoder = NewVP3();
#endif
#ifdef INCLUDEXVID
		if(fourcc == F_XVID)
			encoder = NewXVID();
#endif
#ifdef INCLUDEAVC
		if(fourcc == F_H264)
			encoder = NewAVC();
#endif
		if(!encoder)
		{
			LeaveCriticalSection(&cs);
			Lprintf("Error: VPVIDEO created with no encoder");
			return -1;
		}
		dxcapture = new DXCAPTURE;
		dxcapture->Init(hParent ? hParent : GetDesktopWindow(), WM_APP);
		if(!*videocapturedevice)
		{
			i = 1;
			TCHAR device[1][100];

			if(!dxcapture->EnumCaptureDevices(device, &i, DXC_MODE_VIDEO) && i)
				_tcscpy(videocapturedevice, device[0]);
			else {
//				LeaveCriticalSection(&cs);
				Lprintf("Error: No video capture device found");
//				return -1;
			}
		}
		if(*videocapturedevice)
		{
//			static VIDEOFORMAT formats[100];

			WindowsStringToUTF8(videocapturedevice, s);
			Lprintf("Capture device is %s", s);
			dxcapture->SelectDevice(videocapturedevice, DXC_MODE_VIDEO);
//			dxcapture->EnumFormats(formats, 100);
			dxcapture->SetCallback(&dcfb);
			dxcapture->SetVideoFormat(xres, yres, framerate, F_I420);
			Lprintf("Trying to capture in I420");
			if((rc = encoder->StartEncode(xres, yres, F_I420, framerate, quality)) || dxcapture->StartCapture())
			{
				Lprintf("Error, %s, trying to capture in YV12", rc ? "cannot encode" : dxcapture->LastErrorPtr());
				dxcapture->SetVideoFormat(xres, yres, framerate, F_YV12);
				if((rc = encoder->StartEncode(xres, yres, F_YV12, framerate, quality)) || dxcapture->StartCapture())
				{
					Lprintf("Error, %s, trying to capture in YUY2", rc ? "cannot encode" : dxcapture->LastErrorPtr());
					dxcapture->SetVideoFormat(xres, yres, framerate, F_YUY2);
					if((rc = encoder->StartEncode(xres, yres, F_YUY2, framerate, quality)) || dxcapture->StartCapture())
					{
						Lprintf("Error, %s, trying to capture in RGB24", rc ? "cannot encode" : dxcapture->LastErrorPtr());
						dxcapture->SetVideoFormat(xres, yres, framerate, F_BGR24);
						if((rc = encoder->StartEncode(xres, yres, F_BGR24, framerate, quality)) || dxcapture->StartCapture())
						{
	//						LeaveCriticalSection(&cs);
							Lprintf("Error, %s, cannot capture", rc ? "cannot encode" : dxcapture->LastErrorPtr());
	//						return -1;
						}
					}
				}
			}
			Lprintf("Video acquisition opened");
		}
	}
	LeaveCriticalSection(&cs);
	frame = (BYTE *)malloc(3*xresrx*yresrx);
	if(vps)
		vps->GetCallRemoteName(vpcall, name);
	else strcpy(name, "(Local)");
	RegisterVideoWindow();
	if(vrect.right == vrect.left || vrect.bottom == vrect.top)
	{
		rect.left = rect.top = 0;
		if(xresrx == 176)
		{
			rect.right = 192;
			rect.bottom = 144;
		} else if(xresrx == 352)
		{
			rect.right = 384;
			rect.bottom = 288;
		} else if(xresrx == 720 || xresrx == 704)
		{
			rect.right = 768;
			rect.bottom = 576;
		} else {
			rect.right = xresrx;
			rect.bottom = yresrx;
		}
	} else rect = vrect;
	UTF8ToWindowsString(name, tname);
	if(hParent)
		hWnd = CreateWindow(TEXT("VIDEOWINDOW"), tname, WS_CHILD | (hidden ? 0 : WS_VISIBLE), rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, hParent, (HMENU)childid, GetModuleHandle(0), 0);
	else {
		if(rect.left == 0 && rect.top == 0)
		{
			AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, 0);
			hWnd = CreateWindow(TEXT("VIDEOWINDOW"), tname, WS_OVERLAPPEDWINDOW | (hidden ? 0 : WS_VISIBLE), CW_USEDEFAULT, 0, rect.right-rect.left, rect.bottom-rect.top, 0, 0, GetModuleHandle(0), 0);
		} else hWnd = CreateWindow(TEXT("VIDEOWINDOW"), tname, WS_OVERLAPPEDWINDOW | (hidden ? 0 : WS_VISIBLE), rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, 0, 0, GetModuleHandle(0), 0);
	}
	Lprintf("Opening YUY2 window");
	mode= F_YUY2;
#ifdef _WIN32_WCE
	if(out.Open(hWnd, tname, OUTMODE_DD | OUTMODE_YUY2 | OUTMODE_OVERLAY, 0, 0, xresrx, yresrx))
#else
	if(out.Open(hWnd, tname, OUTMODE_DD | OUTMODE_YUY2, 0, 0, xresrx, yresrx))
#endif
	{
		Lprintf("Failed to open YUY2 window, trying RGB24");
#if 0
		if(out.Open(hWnd, tname, OUTMODE_DD | OUTMODE_RGB16 | OUTMODE_OVERLAY, 0, 0, xresrx, yresrx))
			Lprintf("Error opening video out window");
		else mode = F_BGR16;
#else
		if(out.Open(hWnd, tname, OUTMODE_DD | OUTMODE_RGB24, 0, 0, xresrx, yresrx))
			Lprintf("Error opening video out window");
		else mode = F_BGR24;
#endif
	}
	if(decoder->StartDecode(xresrx, yresrx, mode))
		Lprintf("Error starting decoder (%d,%d,%d", xresrx, yresrx);
	for(i = 0; i < MAXVPVIDEO; i++)
		if(!instances[i])
			if(!InterlockedCompareExchangeLong((LONG *)&instances[i], (LONG)this, 0))
				break;
	if(!hidden)
	{
		ShowWindow(hWnd, SW_SHOW);
		if(!hParent)
			SetForegroundWindow(hWnd);
		SendMessage(out.hWnd, WM_SIZE, 0, 0);
	}
	return 0;
}

int VPVIDEO::SetFullScreen(int fs)
{
	return out.SetFullScreen(fs);
}

int VPVIDEO::SetVideoWindowData(HWINDOW hParent, int childid, const RECTANGLE *rect, bool hidden)
{
	if(hWnd)
	{
		if(!hParent && this->hParent)
		{
			::SetParent(hWnd, 0);
			SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) & ~WS_CHILD | WS_POPUP);
		} else if(hParent && !this->hParent)
		{
			SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) & ~WS_POPUP | WS_CHILD);
			if(childid)
				SetWindowLong(hWnd, GWL_ID, childid);
			::SetParent(hWnd, (HWND)hParent);
		} else {
			::SetParent(hWnd, (HWND)hParent);
			if(hParent && childid)
				SetWindowLong(hWnd, GWL_ID, childid);
		}
		if(rect)
			MoveWindow(hWnd, rect->left, rect->top, rect->right-rect->left, rect->bottom-rect->top, TRUE);
		ShowWindow(hWnd, hidden ? SW_HIDE : SW_SHOW);
	}
	this->hParent = (HWND)hParent;
	this->childid = childid;
	if(rect)
		vrect = *(RECT *)rect;
	this->hidden = hidden;
	return 0;
}

VPVIDEO::~VPVIDEO()
{
	while(starting)
		Sleep(10);
	if(hWnd)
		PostMessage(hWnd, WM_QUIT, 0, 0);
	while(hWnd)
		Sleep(10);
	delete decoder;
	if(this == localvideo)
		localvideo = 0;
	Lprintf("VPVIDEO destroyed");
}

void VPVIDEO::VideoFrame(WORD timestamp, const unsigned char *buf, int buflen, bool keyframe)
{
	int rc;
//	unsigned x, y, i;

	if(!decoder)
		return;
	rc = decoder->Decode(buf, buflen, frame);
	if(rc)
	{
		Lprintf("Error %d decoding video frame (ts=%u, buflen=%d, keyframe=%d)", rc, timestamp, buflen, keyframe);
		return;
	}
	/*if(zoomedframe)
	{
		if(mode == F_BGR16)
		for(y = 0; y < yresrx; y++)
			for(x = 0; x < xresrx; x++)
			{
				i = (4*xresrx+2*x)*2;
				zoomedframe[i+4*xresrx] = zoomedframe[i+4*xresrx+2] = zoomedframe[i] = zoomedframe[i+2] = frame[2*(xresrx*y+x)];
				zoomedframe[i+4*xresrx+1] = zoomedframe[i+4*xresrx+3] = zoomedframe[i+1] = zoomedframe[i+3] = frame[2*(xresrx*y+x)+1];
			}
		else for(y = 0; y < yresrx; y++)
			for(x = 0; x < xresrx; x++)
			{
				i = (4*xresrx+2*x)*3;
				zoomedframe[i+6*xresrx] = zoomedframe[i+6*xresrx+3] = zoomedframe[i] = zoomedframe[i+3] = frame[3*(xresrx*y+x)];
				zoomedframe[i+6*xresrx+1] = zoomedframe[i+6*xresrx+4] = zoomedframe[i+1] = zoomedframe[i+4] = frame[3*(xresrx*y+x)+1];
				zoomedframe[i+6*xresrx+2] = zoomedframe[i+6*xresrx+5] = zoomedframe[i+2] = zoomedframe[i+5] = frame[3*(xresrx*y+x)+2];
			}
		out.Frame(zoomedframe);
	} else*/ out.Frame(frame);
}

#ifdef UNICODE
int VPVIDEO_EnumerateCaptureDevices(char devices[][100], int *ndevices)
{
	int rc, i;
	TCHAR (*devices1)[100];

	if(*ndevices)
	{
		devices1 = (TCHAR (*)[100])malloc(*ndevices * sizeof(TCHAR) * 100);
	} else devices1 = 0;
	if(dxcapture)
		rc = dxcapture->EnumCaptureDevices(devices1, ndevices, DXC_MODE_VIDEO);
	else {
		DXCAPTURE dxcap;
		rc = dxcap.EnumCaptureDevices(devices1, ndevices, DXC_MODE_VIDEO);
	}
	if(!rc)
	{
		if(devices1)
		{
			for(i = 0; i < *ndevices; i++)
				WindowsStringToUTF8(devices1[i], devices[i]);
			free(devices1);
		}
	} else *ndevices = 0;
	return rc;
}
#endif

int VPVIDEO_EnumerateCaptureDevices(TCHAR devices[][100], int *ndevices)
{
	if(dxcapture)
		return dxcapture->EnumCaptureDevices(devices, ndevices, DXC_MODE_VIDEO);
	DXCAPTURE dxcap;
	return dxcap.EnumCaptureDevices(devices, ndevices, DXC_MODE_VIDEO);
}

int VPVIDEO::SetNotifyRoutine(void (*notify)(void *param, unsigned, unsigned, unsigned), void *param)
{
	NotifyRoutine = notify;
	notifyparam = param;
	return 0;
}

static void yuy2torgb(unsigned char *yuy2, unsigned char *rgb, int xres, int yres)
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

int VPVIDEO::CaptureImage(void **image, int *w, int *h)
{
	*image = malloc(3*xresrx*yresrx);
	yuy2torgb(frame, (BYTE *)*image, xresrx, yresrx);
	*w = xresrx;
	*h = yresrx;
	return 0;
}

void DXCAPTUREFRAME::VideoFrame(double SampleTime, BYTE *frame, unsigned bufferlen)
{
	VPCALL vpcalls[100];
	static BYTE compressed[100000];
	int iskey, ncalls, i;
	unsigned len;

#ifdef LOSSDEBUG
	FILE *fp;

	generatelog = 1;
	fp = fopen("\\tmp\\videoin.yuv", "a+b");
	if(fp)
	{
		fwrite(frame, 160*120*3/2, 1, fp);
		fclose(fp);
	}
#endif
	if(VPVIDEO::encoder->Encode(frame, compressed, &len, &iskey) || !len)
		return;
#ifdef LOSSDEBUG
	if(rand() < 5000)
		return;
	fp = fopen("\\tmp\\videoin.avc", "a+b");
	if(fp)
	{
		fwrite(&len, 4, 1, fp);
		fwrite(frame, len, 1, fp);
		fclose(fp);
	}
#endif
	if(localvideo)
		localvideo->VideoFrame(0, compressed, len, !!iskey);
	if(VPVIDEO::vps_in)
	{
		ncalls = VPVIDEO::vps_in->EnumCalls(vpcalls, 100, (1<<BC_VIDEO) | (1<<BC_AUDIOVIDEO));
		for(i = 0; i < ncalls; i++)
			VPVIDEO::vps_in->SendVideo(vpcalls[i], (unsigned short)(SampleTime * 1000), compressed, len, !!iskey);
	}
}

#ifdef UNICODE
int VPVIDEO_SetCaptureDevice(const char *device)
{
	UTF8ToWindowsString(device, videocapturedevice);
	return 0;
}
#endif

int VPVIDEO_SetCaptureDevice(const tchar *device)
{
	_tcsncpy(videocapturedevice, device, sizeof videocapturedevice - 1);
	return 0;
}

VPVIDEODATA *CreateVPVIDEO(IVPSTACK *vps, VPCALL vpcall, unsigned fourccrx, unsigned xresrx, unsigned yresrx, unsigned fourcc, unsigned xres, unsigned yres, unsigned framerate, unsigned quality)
{
	return new VPVIDEO(vps, vpcall, fourccrx, xresrx, yresrx, fourcc, xres, yres, framerate, quality);
}

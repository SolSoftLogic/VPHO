#define _WIN32_WINNT 0x400
#include <windows.h>
#include <dshow.h>
#include <atlbase.h>

#define XRES 176
#define YRES 144
#define FPS1 10
#define FOURCCCAPTURE mmioFOURCC('I','4','2','0')
//#define FOURCCCAPTURE mmioFOURCC('Y','U','Y','2')
#define VIDEOCAPTUREDEVICE 2

//typedef DWORD *DWORD_PTR;
//typedef LONG *LONG_PTR;

#include <qedit.h>
#include <stdio.h>
#include "dxcapture.h"
#include "../video.h"
#include "../../../vp3/vp3.h"

typedef struct {
	int rate, bytes, lasttm;
} ratecounter;

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow);
LONG APIENTRY MainWndProc(HWND hWnd, UINT uMsg, UINT wParam, LONG lParam);
void RateCount(ratecounter *xc, int len);

HINSTANCE hInst;
HWND MainWindow;
VIDEOOUT *vo;
DXCAPTURE *dxc;
VP3CODEC enc,dec;
ratecounter xc;

class DXCAPTUREFRAME : public DXCAPTUREFRAMEBASE {
	void VideoFrame(double SampleTime, BYTE *frame, unsigned bufferlen);
	void AudioFrame(double SampleTime, BYTE *frame, unsigned bufferlen);
} dxcf;

void DXCAPTUREFRAME::VideoFrame(double SampleTime, BYTE *frame, unsigned bufferlen)
{
	char s[300];
	
	sprintf(s, "%f %d video bytes\n", SampleTime, bufferlen);
	OutputDebugString(s);

	unsigned len;
	int iskey;
	char comp[XRES*YRES*2+4096];
	unsigned char len_h, len_l;
	static char decomp[XRES*YRES*3];
	static int n;
	
	enc.Encode(frame, comp, &len, &iskey);
	dec.Decode(comp, len, decomp);
	RateCount(&xc, len);
	/*if(n < 100)
	{
		FILE *fp = fopen("d:\\tmp\\a.vp3", "a+b");
		if(fp)
		{
			len_h = len>>8;
			len_l = len;
			fwrite(&len_h, 1, 1, fp);
			fwrite(&len_l, 1, 1, fp);
			fwrite(comp, len, 1, fp);
			fclose(fp);
		}
	}
	if(n < 100)
	{
		FILE *fp = fopen("d:\\tmp\\a.yuv", "a+b");
		if(fp)
		{
			fwrite(frame, bufferlen, 1, fp);
			fclose(fp);
		}
		n++;
	}*/
	if(0)
	{
		FILE *fp = fopen("c:\\tmp\\vpstat.txt", "a+");
		if(fp)
		{
			fprintf(fp, "%u\t%u\n", GetTickCount(), len);
			fclose(fp);
		}
	}

	if(vo)
		vo->Frame(decomp);
}

void DXCAPTUREFRAME::AudioFrame(double SampleTime, BYTE *frame, unsigned bufferlen)
{
	char s[300];
	
	if(0)
	{
		FILE *fp = fopen("d:\\tmp\\a.pcm", "a+b");
		if(fp)
		{
			fwrite(frame, bufferlen, 1, fp);
			fclose(fp);
		}
	}
	sprintf(s, "%f %d audio bytes\n", SampleTime, bufferlen);
	OutputDebugString(s);
}

void FlushMsgQueue(void)
{
    MSG msg;

    while(PeekMessage(&msg,NULL,0,0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void benchmarkvo()
{
	int i, w = 1024, h = 768, N = 1000;
	unsigned t;
	char s[300];
	HDC hdc;
	
	void *buf = malloc(2*w*h);
	
	memset(buf, 128, w*h*2);
	SetWindowPos(MainWindow, 0, 0, 0, w + 10, h + 40, SWP_NOZORDER);
	vo->Open(MainWindow, "Video", OUTMODE_DD | OUTMODE_YUY2, 0, 0, w, h);
	FlushMsgQueue();
	t = GetTickCount();
	for(i = 0; i < N; i++)
	{
		((BYTE *)buf)[2 * i + 8*w+1] = 0;
		vo->Frame(buf);
	}
	t = GetTickCount() - t;
	free(buf);
	vo->Close();
	FlushMsgQueue();
	sprintf(s, "%d ms, %d bytes, %d MB/s        ", t, N*2*w*h, (N*2*w*h) / t / 1000);
	hdc = GetDC(MainWindow);
	SetTextColor(hdc, PALETTERGB(0, 0, 0));
	TextOut(hdc, 10, 10, s, strlen(s));
	ReleaseDC(MainWindow, hdc);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	TCHAR devices[10][100];
	int ndevices;

	if(!InitInstance(hInstance, nCmdShow))
		return (FALSE);
	enc.StartEncode(XRES,YRES,FOURCCCAPTURE, FPS1, 0x36);
	dec.StartDecode(XRES,YRES,mmioFOURCC('Y','U','Y','2'));
//	dec.StartDecode(352,288,24);
	vo = new VIDEOOUT;
	dxc = new DXCAPTURE;
	dxc->Init(MainWindow, WM_APP);
	ndevices = 10;
	dxc->EnumCaptureDevices(devices, &ndevices, DXC_MODE_VIDEO);
	MessageBox(MainWindow, devices[VIDEOCAPTUREDEVICE], "Using video capture device", 0);
	dxc->SelectDevice(devices[VIDEOCAPTUREDEVICE], DXC_MODE_VIDEO);
	ndevices = 10;
	dxc->EnumCaptureDevices(devices, &ndevices, DXC_MODE_AUDIO);
	MessageBox(MainWindow, devices[0], "Using audio capture device", 0);
	dxc->SelectDevice(devices[0], DXC_MODE_AUDIO);
	dxc->SetCallback(&dxcf);
	dxc->SetVideoFormat(XRES, YRES, FPS1, FOURCCCAPTURE);
	vo->Open(MainWindow, "Video", OUTMODE_DD | OUTMODE_YUY2, 0, 0, XRES, YRES);
	if(dxc->StartCapture())
		MessageBox(MainWindow, dxc->LastErrorPtr(), "Cannot start capture", 0);
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	delete dxc;
	delete vo;
	return (msg.wParam);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	WNDCLASS wc;
	RECT rect;

	hInst = hInstance;
	wc.style = 0;
	wc.lpfnWndProc = (WNDPROC)MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = 0;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName =  0;
	wc.lpszClassName = "Class name";
	if(!RegisterClass(&wc))
		return FALSE;
	rect.left = rect.top = 0;
	rect.right = XRES;
	rect.bottom = YRES;
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
	MainWindow = CreateWindow(
	"Class name",
	"Window name",
	WS_OVERLAPPEDWINDOW,
	100,
	100,
	rect.right-rect.left,
	rect.bottom-rect.top,
	NULL,
	NULL,
	hInst,
	NULL
	);
	if(!MainWindow)
		return FALSE;
	ShowWindow(MainWindow, nCmdShow);
	UpdateWindow(MainWindow);
	return TRUE;
}

LONG APIENTRY MainWndProc(HWND hWnd, UINT uMsg, UINT wParam, LONG lParam)
{
	RECT rect;

	switch (uMsg)
	{
	case WM_CREATE:
		SetTimer(hWnd, 1, 100, 0);
		break;
	case WM_TIMER:
		{
			char s[300];
			HDC hdc = GetDC(hWnd);
			sprintf(s, "%d ", xc.rate / 125);
			TextOut(hdc, 10, 10, s, strlen(s));
			ReleaseDC(hWnd, hdc);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_MOVE:
	case WM_SIZE:
		GetClientRect(hWnd, &rect);
		if(vo && vo->hWnd)
		{
			MoveWindow(vo->hWnd, 0, 0, rect.right, rect.bottom, TRUE);
			SendMessage(vo->hWnd, WM_SIZE, 0, MAKELPARAM(rect.right, rect.bottom));
		}
		break;
	case WM_KEYDOWN:
		switch(wParam)
		{
		case '1':
			dxc->SettingsWindow(hWnd, 1);
			break;
		case '2':
			dxc->SettingsWindow(hWnd, 2);
			break;
		case '3':
			dxc->SettingsWindow(hWnd, 3);
			break;
		case '4':
			dxc->SettingsWindow(hWnd, 4);
			break;
		case '5':
			dxc->SettingsWindow(hWnd, 5);
			break;
		case '6':
			benchmarkvo();
			break;
		}
		break;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

#define XC_ALPHA 0.8f
#define XC_MULT (1.0f - XC_ALPHA) * 1000.0f / XC_INTERVAL
#define XC_INTERVAL 100

void RateCount(ratecounter *xc, int len)
{
	int t;

	t = GetTickCount();
	xc->bytes += len;
	if(!xc->lasttm)
		xc->lasttm = t;
	while(t > xc->lasttm)
	{
		xc->rate = (int)(xc->rate * XC_ALPHA) + xc->bytes;
		xc->lasttm += XC_INTERVAL;
		xc->bytes = 0;
	}
}

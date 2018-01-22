#include <windows.h>
#include "../../windows/video.h"
#include "../../windows/dxcapture/dxcapture.h"

#define XRES 320
#define YRES 240

VIDEOOUT *vo;

static void YV12ToYUY2(const BYTE *src, BYTE *dst, int xres, int yres)
{
	int x, y;

	for(y = 0; y < yres; y++)
		for(x = 0; x < xres; x++)
		{
			//dst[3*(y*xres+x)+1] = dst[3*(y*xres+x)+2] = dst[3*(y*xres+x)] = src[y*xres+x];
			dst[2*(y*xres+x)] = src[y*xres+x];
			dst[2*(y*xres+x)+1] = 128;
		}
}

class CAP : public DXCAPTUREFRAMEBASE
{
	void VideoFrame(double SampleTime, BYTE *frame, unsigned bufferlen)
	{
		WCHAR ws[300];
		static BYTE frame2[XRES*YRES*3];

		wsprintf(ws, TEXT("%f %d\r\n"), SampleTime, bufferlen);
		OutputDebugString(ws);
		YV12ToYUY2(frame, frame2, XRES, YRES);
		vo->Frame(frame2);
	}
} cb;

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow);
LONG APIENTRY MainWndProc(HWND hWnd, UINT uMsg, UINT wParam, LONG lParam);

HINSTANCE hInst;
HWND MainWindow;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpwstr, int nCmdShow)
{
	DXCAPTURE cap;
	TCHAR devices[10][100];
	VIDEOFORMAT formats[10];
	int n = 10;
	MSG msg;

	CoInitialize(0);
	vo = new VIDEOOUT;
	if(!InitInstance(hInstance, nCmdShow))
		return (FALSE);
	cap.Init(0, WM_APP);
	cap.EnumCaptureDevices(devices, &n, DXC_MODE_VIDEO);
	cap.SelectDevice(devices[0], DXC_MODE_VIDEO);
	n = cap.EnumFormats(formats, 10);
	cap.SetVideoFormat(XRES, YRES, 10, F_YV12);
	cap.SetCallback(&cb);
	vo->Open(MainWindow, TEXT("Video"), OUTMODE_DD | OUTMODE_OVERLAY | OUTMODE_YUY2, 0, 0, XRES, YRES);
	cap.StartCapture();
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	cap.StopCapture();
	delete vo;
	return 0;
}

#define WS_OVERLAPPEDWINDOW WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX 

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
	wc.lpszClassName = TEXT("Class name");
	if(!RegisterClass(&wc))
		return FALSE;
	rect.left = rect.top = 0;
	rect.right = XRES;
	rect.bottom = YRES;
	AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, 0);
	MainWindow = CreateWindow(
	TEXT("Class name"),
	TEXT("Window name"),
	WS_OVERLAPPEDWINDOW,
	30,
	50,
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
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

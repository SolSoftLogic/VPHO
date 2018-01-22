#ifndef _VPVIDEO_H_INCLUDED_
#define _VPVIDEO_H_INCLUDED_

#include "../../codecs/videocodecs.h"
#include "dxcapture/dxcapture.h"
#include "video.h"


#define MAXVPVIDEO 10

class VPVIDEO;

class DXCAPTUREFRAME : public DXCAPTUREFRAMEBASE
{
	void VideoFrame(double SampleTime, BYTE *frame, unsigned bufferlen);
	friend class VPVIDEO;
};

class VPVIDEO : public VPVIDEODATA
{
public:
	VPVIDEO(IVPSTACK *vps, VPCALL vpcall, unsigned fourccrx, unsigned xresrx, unsigned yresrx, unsigned fourcc, unsigned xres, unsigned yres, unsigned framerate, unsigned quality);
	~VPVIDEO();
	int Start();
	void VideoFrame(WORD timestamp, const unsigned char *buf, int buflen, bool keyframe);
	int Create(IVPSTACK *vps, VPCALL vpcall, unsigned fourccrx, unsigned xresrx, unsigned yresrx, unsigned fourcc, unsigned xres, unsigned yres, unsigned framerate, unsigned quality);
	int SetVideoWindowData(HWINDOW hParent, int childid, const RECTANGLE *rect, bool hidden);
	int SetNotifyRoutine(void (*notify)(void *param, unsigned, unsigned, unsigned), void *param);
	int CaptureImage(void **image, int *w, int *h);
	int SetFullScreen(int fs);

protected:

	static int ninstances;	
//	static VIDEOIN in;
	static VIDEOCODEC *encoder;
	static IVPSTACK *vps_in;
	static VPVIDEO *instances[MAXVPVIDEO];
	static DXCAPTUREFRAME dcfb;

	void (*NotifyRoutine)(void *param, unsigned message, unsigned param1, unsigned param2);
	void *notifyparam;
	VPCALL vpcall;
	IVPSTACK *vps;
	HWND hWndVideoOut;
	unsigned fourcc, xres, yres, framerate, quality, fourccrx, xresrx, yresrx;
	bool starting, initgood, hidden;

	VIDEOOUT out;
	VIDEOCODEC *decoder;
	HWND hWnd, hParent;
	RECT vrect;
	int childid;
	BYTE *frame;
	friend class DXCAPTUREFRAME;
	friend static void VideoThread(void *vpvd1);
	friend static LRESULT WINAPI VideoWindowWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void VideoThread();
	void Quit() { PostMessage(hWnd, WM_QUIT, 0, 0); }
};

#endif

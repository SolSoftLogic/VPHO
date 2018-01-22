#ifndef _DXCAPTURE_DEFINED_
#define _DXCAPTURE_DEFINED_

#include <windows.h>
#include <dshow.h>
#ifdef _WIN32_WCE
#define USEDSSINK
#else
#include <qedit.h>
#endif
#ifdef INCLUDE_DECKLINK
#include "/projects/qnet/studio/decklinkinterface.h"
#endif

struct AVPacket;
class DXCAPTUREFRAMEBASE {
public:
	virtual void VideoFrame(double SampleTime, BYTE *frame, unsigned bufferlen) {};
	virtual void AudioFrame(double SampleTime, BYTE *frame, unsigned bufferlen) {};
	virtual void EncodedFrame(AVPacket *pkt) {};
};

#define DXC_MODE_VIDEO 1
#define DXC_MODE_AUDIO 2
#ifdef INCLUDE_DECKLINK
#define DXC_MODE_TIMECODE 3
#endif

#define FMTSUPPORT_RGB24 0x1
#define FMTSUPPORT_RGB32 0x2
#define FMTSUPPORT_RGB8 0x4
#define FMTSUPPORT_RGB555 0x8
#define FMTSUPPORT_RGB565 0x10
#define FMTSUPPORT_I420 0x100
#define FMTSUPPORT_YV12 0x200
#define FMTSUPPORT_UYVY 0x400
#define FMTSUPPORT_YUY2 0x800
#define FMTSUPPORT_YVU9 0x1000
#define FMTSUPPORT_MJPG 0x2000

struct VIDEOFORMAT {
	int xres, yres;
	DWORD fourcc;
};

class DXCAPTURE;

#ifndef USEDSSINK
class MyISampleGrabberCB : public ISampleGrabberCB
{
public:
	ULONG WINAPI AddRef() { return instances++; }
	ULONG WINAPI Release() { return --instances; }
	STDMETHODIMP QueryInterface(REFIID iid, void **ppvObject) { *ppvObject = this; return S_OK; }
    MyISampleGrabberCB() {	instances = 0; }
	STDMETHODIMP SampleCB(double SampleTime, IMediaSample *pSample)	{	return E_NOTIMPL;	}
	STDMETHODIMP BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen);

protected:
	DXCAPTURE *capture;
	int mode;
	friend DXCAPTURE;
	unsigned instances;
};
#endif

class BDAGraph;

class DXCAPTURE {
public:
	DXCAPTURE();
	~DXCAPTURE();
	int Init(HWND hWnd, UINT uMsg);
	int EnumCaptureDevices(TCHAR devices[][100], int *ndevices, int mode);
	int SelectDevice(const TCHAR *device, int mode);
	int SetCallback(DXCAPTUREFRAMEBASE *dxcfb) { fcb = dxcfb; return 0; }
	int SettingsWindow(HWND hWnd, int what);
	int SetVideoFormat(int xres, int yres, double fps, DWORD fourcc);
	int GetVideoFormat(int *xres, int *yres, double *fps, DWORD *fourcc);
	int SetAudioFormat(int channels, int sfreq);
	int GetAudioFormat(int *channels, int *sfreq);
	int StartCapture();
	int StopCapture();
	int GetDroppedFramesInfo(int *notdropped, int *dropped);
	int SetChannel(int ch);
	int IsBDA();
	int GetCaptureFilter(IBaseFilter **filter);
	int GetVcrTimecode(char *text, unsigned *frames, double *framerate);
	int GetFormats();
	int EnumFormats(VIDEOFORMAT *formats, int max);
	const char *LastErrorPtr() { return lasterrorstr; }
#ifdef USEBDA
	int ProgramsList(char *list, int size);
	int SetProgramId(int id);
#endif
protected:
	int OpenError(const char *error, HRESULT hr);

	CRITICAL_SECTION cs;
	IGraphBuilder *pGB;
//	ICaptureGraphBuilder2 *pBuilder;
	IMediaControl *pMC;
	IBaseFilter *pNullVideoRenderer, *pNullAudioRenderer;
	IBaseFilter *pVideoCaptureSource, *pAudioCaptureSource;
	IBaseFilter *pMjpegDec;
	IAMCrossbar *pCrossbar;
	IAMTVTuner *pTVTuner;
	IAMDroppedFrames *pDroppedFrames;
#ifndef USEDSSINK
	IBaseFilter *pVideoGrabber, *pAudioGrabber;
	ISampleGrabber *pIVideoGrabber, *pIAudioGrabber;
	MyISampleGrabberCB AudioGrabberCB, VideoGrabberCB;
#endif
	ICaptureGraphBuilder2 *pCGB;
	IAMTimecodeReader *pTimecodeReader;
	int xres, yres, frameduration;
	int channels, sfreq;
	DWORD fourcc;
	WAVEFORMATEX wf;
	DXCAPTUREFRAMEBASE *fcb;

	int notdroppedframes, droppedframes;
	double audiotm, videotm;
	bool running;
	int status;	// 0 = init, 1 = running, 2 = closing
#ifdef USEBDA
	bool usebda;
	BDAGraph *bda;
	int bdafrequency, bdaprogramid;
	char bdadevice[100];
#endif
//	BYTE audiobuffer[AUDIOBUFFERSIZE];
	unsigned abhead, abtail;
	LONG nthreads;
	HWND AppWindow;
	UINT NotifyMsg;
	struct {
		int counter;
		int source;
		int streamvalid;
		TIMECODE tc;
	} timecode;
	char lasterrorstr[100];

#ifdef USEDSSINK
	friend void WINAPI DSCallback(void *param, IMediaSample *ms);
#else
	friend MyISampleGrabberCB;
#endif

#ifdef INCLUDE_DECKLINK
	void TimecodeFrame(double SampleTime, BYTE *frame, unsigned bufferlen);

	IDecklinkIOControl *pDecklinkIOControl;
	IBaseFilter *pTimecodeCaptureSource, *pNullTimecodeRenderer, *pTimecodeGrabber;
	ISampleGrabber *pITimecodeGrabber;
	MyISampleGrabberCB TimecodeGrabberCB;
#endif
};

#endif

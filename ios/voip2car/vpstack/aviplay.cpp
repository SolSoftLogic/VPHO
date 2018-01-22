#include <stdio.h>
#include "portability.h"
#include "windows/video.h"
#include "windows/winaudio.h"
#include "../codecs/videocodecs.h"
#include "audiocodecs.h"
#include "util.h"
#include "avifile.h"
#include "vpstack.h"

class AVIPLAYER : public IAVIPLAYER
{
public:
	AVIPLAYER();
	~AVIPLAYER();
	int PlayAVIFile(const char *path);
	int Pause(int pause) { this->pause = pause; return 0; }
	int PlayWAVFile(const char *path, int loop);
	int Stop();
	int Progress();	// In thousands
	int WaitEnd();
	int SetWindowData(HWINDOW hWnd, const RECTANGLE *rect);
	int SetVideoControl(int control, int value);
	int SetAudioDevice(const char *dev);

protected:
	void PlayAVIFileThread(void *param);

	HWND hWnd;
	bool playing, stopplay;
	int codec, pause;
	RECT rect;
	void *havi;
	void *hwav;
	int loop;
	unsigned audiotm, videotm;
	char audiodev[100];
	VIDEOCODEC *vdec;
	AUDIODECODER *adec;
	VIDEOOUT *video;
	AUDIOOUT *audio;
	AVIDATA avidata;
};

IAVIPLAYER *CreateAVIPLAYER()
{
	return new AVIPLAYER();
}

AVIPLAYER::AVIPLAYER()
{
	hWnd = 0;
	playing = stopplay = false;
	codec = 0;
	memset(&rect, 0, sizeof(rect));
	havi = hwav = 0;
	loop = false;
	audiotm = videotm = 0;
	memset(audiodev, 0, sizeof(audiodev));
	vdec = 0;
	adec = 0;
	video = 0;
	audio = 0;
	pause = 0;
	memset(&avidata, 0, sizeof(avidata));
}

AVIPLAYER::~AVIPLAYER()
{
	Stop();
}

int AVIPLAYER::SetWindowData(HWINDOW hWnd, const RECTANGLE *rect)
{
	this->hWnd = (HWND)hWnd;
	this->rect = *(RECT *)rect;
	return 0;
}

int AVIPLAYER::PlayAVIFile(const char *path)
{
	tchar taudiodev[100];
	bool usergb24 = false;

	Lprintf("AVIOpen %s", path);
	havi = 	AVIOpen(path, &avidata);
	if(!havi)
		return -1;
	codec = WF2Codec((WAVEFMT *)avidata.wf);
	if(codec == -1)
	{
		Lprintf("AVIPLAYER: Cannot decode file (format tag=%d)", ((WAVEFORMATEX *)avidata.wf)->wFormatTag);
		AVIClose(havi);
		return -1;
	}
#ifdef INCLUDEVP3
	if(avidata.fourcc == F_VP31)
		vdec = NewVP3();
#endif
#ifdef INCLUDEXVID
	if(avidata.fourcc == F_XVID)
		vdec = NewXVID();
#endif
#ifdef INCLUDEAVC
	if(avidata.fourcc == F_H264)
		vdec = NewAVC();
#endif
	if(!vdec)
	{
		Lprintf("AVIPLAYER: Cannot decode file (fourcc=%x)", avidata.fourcc);
		AVIClose(havi);
		havi = 0;
		return -1;
	}
	video = new VIDEOOUT;
#ifdef _WIN32_WCE
	if(video->Open(hWnd, TEXT(""), OUTMODE_DD | OUTMODE_YUY2 | OUTMODE_OVERLAY, 0, 0, avidata.width, avidata.height))
#else
	if(video->Open(hWnd, TEXT(""), OUTMODE_DD | OUTMODE_YUY2, 0, 0, avidata.width, avidata.height))
#endif
	{
		Lprintf("Failed to open YUY2 window, trying RGB24");
		if(video->Open(hWnd, TEXT(""), OUTMODE_DD | OUTMODE_RGB24, 0, 0, avidata.width, avidata.height))
		{
			Lprintf("AVIPLAYER: Error opening video out window");
			delete video;
			return -1;
		} else usergb24 = true;
	}
	if(vdec->StartDecode(avidata.width, avidata.height, usergb24 ? F_BGR24 : F_YUY2))
	{
		Lprintf("AVIPLAYER: Error starting decoder (%d,%d)", avidata.width, avidata.height);
		delete video;
		return -1;
	}

	MoveWindow(video->hWnd, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, TRUE);
	adec = new AUDIODECODER;
	audio = new AUDIOOUT;
	if(*audiodev)
	{
		UTF8ToWindowsString(audiodev, taudiodev);
		audio->SetAudioDevice(taudiodev);
	}
	playing = true;
	beginclassthread((ClassThreadStart)&AVIPLAYER::PlayAVIFileThread, this, 0);
	return 0;
}

int AVIPLAYER::SetAudioDevice(const char *dev)
{
	strcpy(audiodev, dev);
	return 0;
}

void AVIPLAYER::PlayAVIFileThread(void *param)
{
	static BYTE buf[200000];
	static short pcm[10000];
	static BYTE frame[640*480*4];
	unsigned len, vlen;
	int rc;

	audiotm = videotm = 0;
	while(!stopplay)
	{
		if(havi)
			rc = AVIReadAudio(havi, buf, &len);
		else if(hwav && adec)
			rc = WAVRead(hwav, buf, &len);
		else if(hwav && !adec)
			rc = WAVRead(hwav, pcm, &len);
		else break;
		if(!rc)
		{
			if(adec)
				len = adec->Decode(codec, buf, len, pcm);
			if(len == -1)
				break;
			while(audio->PutData(avidata.channels, avidata.sfreq, pcm, len) == -2 || (!audiotm && !videotm && pause))
			{
				if(havi && audiotm > videotm + audio->GetCurrentDelay() || (!audiotm && !videotm && pause))
				{
					if(AVIReadVideo(havi, buf, &vlen))
					{
						stopplay = true;
						break;
					}
					vdec->Decode(buf, vlen, frame);
					video->Frame(frame);
					videotm += avidata.framelen / 1000;
				}
				Sleep(10);
			}
			audiotm += 1000 * len / (2 * avidata.channels * avidata.sfreq);
		} else if(hwav && loop)
		{
			WAVSeek(hwav, 0);
			Sleep(10);
		} else break;
		while(pause && !stopplay)
			Sleep(10);
	}
	while(!stopplay)
	{
		if(audio->GetCurrentDelay() == -1)
			break;
		if(havi && audiotm > videotm + audio->GetCurrentDelay())
		{
			if(AVIReadVideo(havi, buf, &vlen))
			{
				stopplay = true;
				break;
			}
			vdec->Decode(buf, vlen, frame);
			video->Frame(frame);
			videotm += avidata.framelen / 1000;
		}
		Sleep(10);
	}
	if(hwav)
		WAVClose(hwav);
	if(havi)
		AVIClose(havi);
	if(adec)
		delete adec;
	if(vdec)
		delete vdec;
	if(video)
		delete video;
	if(audio)
		delete audio;
	adec = 0;
	vdec = 0;
	video = 0;
	audio = 0;
	havi = 0;
	hwav = 0;
	playing = false;
}

int AVIPLAYER::WaitEnd()
{
	while(playing)
		Sleep(10);
	if(audio)
		audio->Stop();
	return 0;
}

int AVIPLAYER::Stop()
{
	stopplay = true;
	while(playing)
		Sleep(10);
	stopplay = false;
	return 0;
}

int AVIPLAYER::SetVideoControl(int control, int value)
{
	if(!video)
		return -1;
	if(control == 0)
		video->brightness = value;
	else if(control == 1)
		video->contrast = value;
	else return -1;
	return 0;
}

int AVIPLAYER::Progress()
{
	int a = avidata.lenms;

	if(!playing)
		return -1;
	if(a)
		return MulDiv(1000, audiotm, a);
	else return 0;
}

int AVIPLAYER::PlayWAVFile(const char *file, int loop)
{
	tchar taudiodev[100];
	
	Stop();
	hwav = 	WAVOpen(file, (WAVEFMT *)avidata.wf);
	if(!hwav)
		return -1;
	avidata.sfreq = ((WAVEFORMATEX *)avidata.wf)->nSamplesPerSec;
	avidata.channels = ((WAVEFORMATEX *)avidata.wf)->nChannels;
	this->loop = loop;
	if(((WAVEFORMATEX *)avidata.wf)->wFormatTag != 1)
	{
		codec = WF2Codec((WAVEFMT *)avidata.wf);
		if(codec == -1)
		{
			Lprintf("WAVPLAYER: Cannot decode file (format tag=%d)", ((WAVEFORMATEX *)avidata.wf)->wFormatTag);
			WAVClose(hwav);
			return -1;
		}
		adec = new AUDIODECODER;
	}
	audio = new AUDIOOUT;
	if(*audiodev)
	{
		UTF8ToWindowsString(audiodev, taudiodev);
		audio->SetAudioDevice(taudiodev);
	}
	playing = true;
	beginclassthread((ClassThreadStart)&AVIPLAYER::PlayAVIFileThread, this, 0);
	return 0;
}

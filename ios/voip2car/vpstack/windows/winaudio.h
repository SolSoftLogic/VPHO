#ifndef _WINAUDIO_H_DEFINED_
#define _WINAUDIO_H_DEFINED_

#include <windows.h>
#include <mmreg.h>
#include <msacm.h>
#include <dsound.h>

#ifdef DSAUDIO
#define AUDIOOUT tAudioOutDS
#else
#define AUDIOOUT tAudioOut
#endif

#define AUDIOIN tAudioIn
#define AUDIOPLAYER tAudioPlayer

#define AUDIOERR_FATAL -3

#define BUFFERS 100
#define DEFINEDCODECS 16

class tAudioOut {
public:
	tAudioOut() { memset(this, 0, sizeof(tAudioOut)); gmaxdelay = 500; volume = 0xffffffff; playdev = WAVE_MAPPER; InitializeCriticalSection(&cs); }
	~tAudioOut() { Stop(1); DeleteCriticalSection(&cs); }
	int SetAudioDevice(TCHAR *sdev);
	int CheckAudioDevice(TCHAR *sdev);
	int PutData(int channels, int sfreq, short *buf, int len);	// sfreq in Hz, len in bytes
	int Stop();

	int SetVolume(int thousands);
	unsigned GetVolume() { return volume; }
	int SetInitialDelay(int delay);	// in milliseconds
	unsigned GetInitialDelay() { return gdelay; }
	int SetBufferLength(int delay);	// in milliseconds
	unsigned GetBufferLength() { return gmaxdelay; }

	int VuMeter(void);			// in millibell
	int GetCurrentDelay();

	int ForceStart();
	int Beep(int sfreqmultiplier);

	int outgain;

protected:
	static friend void WaveOutPeriodical(void *audio);
	void Periodical();
	int Start(int channels, int sfreq);
	int Stop(int docs);
	HWAVEOUT hwo;
	WAVEHDR wh[BUFFERS];

	int buftail, bufhead, playdev;
	int vumeter, playbufs, playing;
	LONGLONG sentsamples, playedsamples;
	int gsfreq, gdelay, gmaxdelay, underrun;
	unsigned volume;
	int periodicalactive, stop;
	CRITICAL_SECTION cs;
	bool terminating;
};

class tAudioIn {
public:
	tAudioIn() { memset(this, 0, sizeof(tAudioIn)); threshold = -50; reclevel = 500; micboost = true; recorddev = WAVE_MAPPER; InitializeCriticalSection(&cs); }
	~tAudioIn() { Stop(); DeleteCriticalSection(&cs); }
	int SetAudioDevice(TCHAR *sdev);
	int CheckAudioDevice(TCHAR *sdev);
	int Start(int channels, int sfreq, int framelen, int buflength);	// in Hz, samples, milliseconds
	int Stop();
	int GetData(short *data);
	int GetDataTm() { return tm; }
	int GetRecLevel() { return reclevel; }
	int SetRecLevel(int thousands);
	int SetAudioLine(TCHAR *linename);
	int FillCBWithInputLines(HWND hWnd, int defcomponenttype);
	int EnumInputLines(TCHAR inputlines[][100], unsigned *ninputlines);
	int VuMeter();				// in millibell
	void SetThreshold(int dB) { threshold = dB;}
	int GetThreshold() { return threshold; }
	void EchoSuppression(short *buf, int len);
	void SetEchoSuppressionRefLevel(int level) { vu_level = level; }
	void SetEchoSuppressionMode(int high) { highechosuppression = high; }
	int micboost, echosuppression, autothreshold;

protected:
	int vu_level, highechosuppression;
	float es_inputenergy, es_refenergy;
	int gframelen, gsfreq, grecordbufs, vumeter, bufhead, reclevel, gbuflength, rframes, threshold,
		silencedetected, recorddev, ingain, noiselevel;
	int hpfinmem[2];
	double hpfoutmem[2];
	unsigned tm;
	TCHAR recline[100];
	WAVEHDR wh[BUFFERS];
	HWAVEIN hwi;
	CRITICAL_SECTION cs;
};

#ifdef DSAUDIO
class tAudioOutDS : public tAudioOut
{
public:
	tAudioOutDS();
	~tAudioOutDS();
	int SetAudioDevice(TCHAR *sdev);
	int CheckAudioDevice(TCHAR *sdev);
	int PutData(int channels, unsigned sfreq, short *buf, unsigned len);

	int SetVolume(int thousands);
	unsigned GetVolume() { return 0; }
	int SetInitialDelay(unsigned ms) { return 0; }
	int GetInitialDelay() { return 0; }
	int SetBufferLength(unsigned ms);
	unsigned GetBufferLength() { return maxdelay; }
	int Stop();

	int VuMeter();
	int GetCurrentDelay();

	int ForceStart() { return 0; }
	int Beep(int sfreqmultiplier);

	int outgain;

protected:
	void StartPlaying();
	void thread();
	int Start(int channels, unsigned sfreq);
	static friend void dsthread(void *object);

	unsigned sfreq, maxdelay, idev;
	int imixer;
	DWORD writepos, zeroedpos, prevwritecursor, buffersize;
	char playing;
	unsigned short *vumeter;
	bool stopping;
	IDirectSound *dsound;
	IDirectSoundBuffer *dsbuf;
};
#endif

class tAudioEncoder {
public:
	tAudioEncoder() {hAcm = 0;}
	~tAudioEncoder();
	int Encode(int codec, short *in, unsigned inbytes, BYTE *out);
protected:
	HACMSTREAM hAcm;
	ACMSTREAMHEADER ash;
	int codec;
};

class tAudioDecoder {
public:
	tAudioDecoder() {hAcm = 0;}
	~tAudioDecoder();
	int Decode(int codec, const BYTE *in, unsigned inbytes, short *out);
protected:
	HACMSTREAM hAcm;
	ACMSTREAMHEADER ash;
	int codec;
};

class tAudioPlayer {
public:
	tAudioPlayer() { playing = false; f = 0; buffer = 0; }
	~tAudioPlayer() { Stop(); }
	int PlayFile(TCHAR *path, int loop);
	int PlayBuffer(char *buffer, int bufferlen, int loop);
	int Stop();
	int Pause(int op);
	int Progress();
	int WaitEnd();
	
	AUDIOOUT audioout;
protected:
	void PlayAudioFileThread(void *param);

	bool playing, stopplay, pause;
	int filelen, progress, loop;
	int f;
	char *buffer;
	int bufferlen;

};

void InitializeDS();
int ds_USBDeviceName(TCHAR *devicename);
int wavein_GetDeviceName(int n, TCHAR *devicename);
int waveout_GetDeviceName(int n, TCHAR *devicename);
int AuFillCBWithInputLines(HWAVEIN hwi, HWND hWnd, int defcomponenttype);
int EnumInputLines(HWAVEIN hwi, TCHAR inputlines[][100], unsigned *ninputlines);
int AUDIO_EnumerateDevices(TCHAR devices[][100], int *ndevices, int recording);
const char *GetCodecName(int codec);
TCHAR *ds_GetDeviceName(int i);
extern HWND ds_AppWindow;
extern unsigned codecsfreq[DEFINEDCODECS];
extern int codecuframelen[DEFINEDCODECS];
extern int codeccframelen[DEFINEDCODECS];
extern struct ACM_WFC {
	WAVEFORMATEX wf;
	unsigned char extra[114];
} acm_wfc[DEFINEDCODECS];

#endif

#ifndef _AUDIO_H_INCLUDED
#define _AUDIO_H_INCLUDED

#define AUDIOERR_FATAL (-3)

#include "../portability.h"
#include "../util.h"

struct ALSAAUDIO;

class AUDIOOUT {
public:
	AUDIOOUT();
	~AUDIOOUT();
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
	int au;
	int cursfreq, curchannels;
	int volume, gdelay, gmaxdelay;
	CRITICAL_SECTION cs;
	ALSAAUDIO *alsa;
};

class AUDIOIN {
public:
	AUDIOIN();
	~AUDIOIN();
	int EnumInputLines(TCHAR inputlines[][100], unsigned *ninputlines);
	int SetAudioDevice(TCHAR *sdev);
	int CheckAudioDevice(TCHAR *sdev);
	int Start(int channels, int sfreq, int framelen, int buflength);	// in Hz, bytes, milliseconds
	int Stop();
	int GetData(short *data);
	int GetDataTm() { return tm; }
	int GetRecLevel() { return reclevel; }
	int SetRecLevel(int thousands);
	int SetAudioLine(TCHAR *linename);
//	int FillCBWithInputLines(HWND hWnd, int defcomponenttype);
	int VuMeter();				// in millibell
	void SetThreshold(int dB) { threshold = dB;}
	int GetThreshold() { return threshold; }
	void EchoSuppression(short *buf, int len);
	void SetEchoSuppressionRefLevel(int level) { vu_level = level; }
	void SetEchoSuppressionMode(int high) { highechosuppression = high; }
	int micboost, echosuppression, autothreshold;

protected:
	int au;
	int cursfreq, curchannels, curframelen;
	int vu_level, highechosuppression, tm, threshold, reclevel;
	CRITICAL_SECTION cs;
	ALSAAUDIO *alsa;
};

int AUDIO_EnumerateDevices(char devices[][100], int *ndevices, int recording);

#endif

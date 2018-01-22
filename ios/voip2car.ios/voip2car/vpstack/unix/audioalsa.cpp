#include <unistd.h>
#include <sys/soundcard.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include "alsa.h"
#include "audio.h"

extern "C" void lprintf(int type, const char *fmt, ...)
{
	va_list ap;
	char s[300];

	va_start(ap, fmt);
	vsnprintf(s, sizeof s, fmt, ap);	
	va_end(ap);
	s[strlen(s)-1] = 0;
	Lprintf("%s", s);
}

AUDIOOUT::AUDIOOUT()
{
	Zero(*this);
	InitializeCriticalSection(&cs);
	alsa = new ALSAAUDIO;
	alsa_init(alsa);
}

AUDIOOUT::~AUDIOOUT()
{
	Stop();
	delete alsa;
	DeleteCriticalSection(&cs);
}

AUDIOIN::AUDIOIN()
{
	Zero(*this);
	InitializeCriticalSection(&cs);
	alsa = new ALSAAUDIO;
	alsa_init(alsa);
}

AUDIOIN::~AUDIOIN()
{
	Stop();
	delete alsa;
	DeleteCriticalSection(&cs);
}

int AUDIOOUT::PutData(int channels, int sfreq, short *buf, int len)	// sfreq in Hz, len in bytes
{
	int first = 0, i;

	EnterCriticalSection(&cs);
	if(sfreq != cursfreq || channels != curchannels)
	{
		alsa_close_play(alsa);
		cursfreq = 0;
		curchannels = 0;
		if(alsa_open_play(alsa, sfreq, channels, len, 1))
		{
			LeaveCriticalSection(&cs);
			return -1;
		}
		first = 1;
		cursfreq = sfreq;
		curchannels = channels;
	}
	if(first)
	{
		// Put some initial delay
		for(i = 0; i < 1; i++)
			if(alsa_put(alsa, buf,len))
			{
				LeaveCriticalSection(&cs);
				return -1;
			}
	}
	if(alsa_put(alsa, buf, len))
	{
		LeaveCriticalSection(&cs);
		return -1;
	}
	LeaveCriticalSection(&cs);
	return 0;
}

int AUDIOOUT::Stop()
{
	EnterCriticalSection(&cs);
	if(!cursfreq)
	{
		LeaveCriticalSection(&cs);
		return -1;
	}
	alsa_close_play(alsa);
	cursfreq = 0;
	curchannels = 0;
	LeaveCriticalSection(&cs);
	return 0;
}

int AUDIOOUT::GetCurrentDelay()
{
	EnterCriticalSection(&cs);
	if(!cursfreq)
	{
		LeaveCriticalSection(&cs);
		return -1;
	}
	int n = alsa_play_delay(alsa);
	if(n == -1 || !cursfreq || !curchannels)
	{
		LeaveCriticalSection(&cs);
		return -1;
	}
	n = n * 500 / (curchannels * cursfreq);
	LeaveCriticalSection(&cs);
	return n;
}

int AUDIOOUT::SetVolume(int thousands)
{
	return 0;
}

int AUDIOOUT::VuMeter()
{
	if(!cursfreq)
		return -1;
	if(alsa->vumeter[0] > alsa->vumeter[1])
		return alsa->vumeter[0];
	return alsa->vumeter[1];
}

int AUDIOIN::Start(int channels, int sfreq, int framelen, int buflength)	// in Hz, bytes, milliseconds
{
	EnterCriticalSection(&cs);
	if(sfreq != cursfreq || channels != curchannels)
	{
		alsa_close_record(alsa);
		cursfreq = 0;
		curchannels = 0;
		if(alsa_open_record(alsa, sfreq, channels, framelen*2, 1))
		{
			LeaveCriticalSection(&cs);
			return -1;
		}
		cursfreq = sfreq;
		curchannels = channels;
	}
	LeaveCriticalSection(&cs);
	return 0;
}

int AUDIOIN::GetData(short *data)
{
	int rc;

	EnterCriticalSection(&cs);
	if(!cursfreq)
	{
		LeaveCriticalSection(&cs);
		return -1;
	}
	rc = alsa_get(alsa, data);
	LeaveCriticalSection(&cs);
	return rc;
}

int AUDIOIN::Stop()
{
	EnterCriticalSection(&cs);
	if(!cursfreq)
	{
		LeaveCriticalSection(&cs);
		return -1;
	}
	alsa_close_record(alsa);
	cursfreq = 0;
	curchannels = 0;
	LeaveCriticalSection(&cs);
	return 0;
}

int AUDIOIN::SetRecLevel(int thousands)
{
	return 0;
}

int AUDIOIN::SetAudioLine(TCHAR *linename)
{
	return 0;
}

int AUDIOIN::VuMeter()
{
	if(!cursfreq)
		return -1;
	if(alsa->vumeter[0] > alsa->vumeter[1])
		return alsa->vumeter[0];
	return alsa->vumeter[1];
}

int AUDIOIN::SetAudioDevice(TCHAR *sdev)
{
	return -1;
}

int AUDIOIN::EnumInputLines(TCHAR inputlines[][100], unsigned *ninputlines)
{
	return -1;
}

int AUDIOOUT::SetAudioDevice(TCHAR *sdev)
{
	return -1;
}

int AUDIO_EnumerateDevices(char devices[][100], int *ndevices, int recording)
{
	if(*ndevices)
	{
		strcpy(devices[0], "Audio device");
		*ndevices = 1;
	}
	return 0;
}

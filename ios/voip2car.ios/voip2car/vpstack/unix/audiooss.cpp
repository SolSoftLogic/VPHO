#include <unistd.h>
#include <sys/soundcard.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include "audio.h"

AUDIOOUT::AUDIOOUT()
{
	Zero(*this);
}

AUDIOOUT::~AUDIOOUT()
{
	Stop();
}

int AUDIOOUT::PutData(int channels, int sfreq, short *buf, int len)	// sfreq in Hz, len in bytes
{
	int first = 0, i;

	if(sfreq != cursfreq || channels != curchannels || !au)
	{
		int stereo = channels - 1, format = AFMT_S16_LE;
		
		if(au)
		{
			close(au);
			au = 0;
		}
		au = open("/dev/dsp", O_WRONLY);
		if(au == -1)
		{
			au = 0;
			return -1;
		}
		ioctl(au, SNDCTL_DSP_SPEED, &sfreq);
		ioctl(au, SNDCTL_DSP_STEREO, &stereo);
		ioctl(au, SNDCTL_DSP_SETFMT, &format);
		cursfreq = sfreq;
		curchannels = channels;
		first = 1;
	}
	if(first)
	{
		// Put some initial delay
		for(i = 0; i < 30; i++)
			if(write(au, buf, len) != len)
				return -1;
	}
	if(write(au, buf, len) != len)
		return -1;
	return 0;
}

int AUDIOOUT::Stop()
{
	if(au)
		close(au);
	au = 0;
	return 0;
}

int AUDIOOUT::GetCurrentDelay()
{
	int odelay;
	
	if(!au)
		return -1;
	ioctl(au, SNDCTL_DSP_GETODELAY, &odelay);
	return odelay * 500 / cursfreq;
}

int AUDIOOUT::SetVolume(int thousands)
{
	return 0;
}

int AUDIOOUT::VuMeter()
{
	return 0;
}

AUDIOIN::AUDIOIN()
{
	Zero(*this);
}

AUDIOIN::~AUDIOIN()
{
	Stop();
}

int AUDIOIN::Start(int channels, int sfreq, int framelen, int buflength)	// in Hz, bytes, milliseconds
{
	if(sfreq != cursfreq || channels != curchannels || !au)
	{
		int stereo = channels - 1, format = AFMT_S16_LE;
		
		if(au)
		{
			close(au);
			au = 0;
		}
		au = open("/dev/dsp", O_RDONLY);
		if(au == -1)
		{
			au = 0;
			return -1;
		}
		ioctl(au, SNDCTL_DSP_SPEED, &sfreq);
		ioctl(au, SNDCTL_DSP_STEREO, &stereo);
		ioctl(au, SNDCTL_DSP_SETFMT, &format);
		cursfreq = sfreq;
		curchannels = channels;
		curframelen = framelen;
	}
	return 0;
}

int AUDIOIN::GetData(short *data)
{
	if(!au)
		return -1;
	return read(au, data, curframelen*2);
}

int AUDIOIN::Stop()
{
	if(au)
		close(au);
	au = 0;
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
	return 0;
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

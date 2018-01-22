#include <stdio.h>
#include "portability.h"
#include "vpstack.h"
#include "vpaudio.h"
#include "audiocodecs.h"
#include "simpleaudio.h"

static int ninstances;	
static int mute;
static AUDIOIN gin, *in = &gin;
static AUDIOENCODER gencoder, *encoder = &gencoder;
static int codec;
static IVPSTACK *vps;
static int vumeterin = -10000;
static int vumeterout = -10000;
static VPSIMPLEAUDIO *instances[MAXSIMPLEAUDIO];
TCHAR VPSIMPLEAUDIO_audiodev[2][100];

int SilenceFrame(short *buf, int nsamples, int *peak)
{
	int i, threshold;

	for(i = 0; i < nsamples; i++)
		if(buf[i] < -*peak)
			*peak = -buf[i];
		else if(buf[i] > *peak)
			*peak = buf[i];
	threshold = *peak / 20;
	for(i = 0; i < nsamples; i++)
		if(buf[i] < -threshold || buf[i] > threshold)
			return 0;
	return 1;
}

static void StartAudioIn()
{
	int recorderframelen;

	if(codec == CODEC_SPEEXD)
		recorderframelen = 640;
	else if(codec == CODEC_SPEEXC || codec == CODEC_SPEEXE)
		recorderframelen = 1280;
	else if(codec == CODEC_ILBC30)
		recorderframelen = 240;
	else recorderframelen = 320;
	if(*VPSIMPLEAUDIO_audiodev[1])
		in->SetAudioDevice(VPSIMPLEAUDIO_audiodev[1]);
	in->Start(1, codecsfreq[codec], recorderframelen, 500);
	_beginthread(VPSIMPLEAUDIO_AudioAcquisitionThread, 0, 0);
}

VPSIMPLEAUDIO::VPSIMPLEAUDIO(IVPSTACK *vps, VPCALL vpcall, int codec)
{
	int i;

#ifdef DSAUDIO
	InitializeDS();
#endif
	decoder = new AUDIODECODER;
	out = new AUDIOOUT;
	peak = 0;
	ninstances++;
	::vps = vps;
	if(ninstances == 1)
	{
		::codec = codec;
		if(!mute)
			StartAudioIn();
	}
	for(i = 0; i < MAXSIMPLEAUDIO; i++)
		if(!instances[i])
			if(!InterlockedCompareExchangeLong((LONG *)&instances[i], (LONG)this, 0))
				break;
}

VPSIMPLEAUDIO::~VPSIMPLEAUDIO()
{
	int i;

	ninstances--;
	for(i = 0; i < MAXSIMPLEAUDIO; i++)
		if(instances[i] == this)
			instances[i] = 0;
	delete decoder;
	delete out;
}

void VPSIMPLEAUDIO::AudioFrame(int codec, unsigned timestamp, const unsigned char *buf, int buflen)
{
	short audiobuffer[8*1280];	// = 8 max frames * 2560 bytes for longest frame
	int rc, nsamples, nsamplesinbuffer;

	rc = decoder->Decode(codec, buf, buflen, audiobuffer);
	if(rc > 0)
	{
		nsamples = rc / 2;
		if(*VPSIMPLEAUDIO_audiodev[0])
			out->SetAudioDevice(VPSIMPLEAUDIO_audiodev[0]);
		nsamplesinbuffer = out->GetCurrentDelay() * 8;
#ifdef WIN32
		if(nsamplesinbuffer <= nsamples)
		{
			static short nullbuffer[4*8*1280];

			if(nsamples <= 1280)
				out->PutData(1, codecsfreq[codec], audiobuffer, rc);	// Put some initial delay, if the packet is short, repeat it instead of putting zeros
			else out->PutData(1, codecsfreq[codec], nullbuffer, rc);
		}
		if(nsamplesinbuffer <= 1600 || nsamplesinbuffer <= 2*nsamples || !SilenceFrame(audiobuffer, nsamples, &peak))
#endif
			out->PutData(1, codecsfreq[codec], audiobuffer, rc);
		vumeterout = out->VuMeter();
	}
}

void VPSIMPLEAUDIO_AudioAcquisitionThread(void *dummy)
{
	short audiobuffer[1280];
	BYTE buf[1500];
	VPCALL vpcalls[100];
	unsigned short tm;
	int rc;
	unsigned i, ncalls;

	while(ninstances && !mute)
	{
		rc = in->GetData(audiobuffer);
		if(rc == AUDIOERR_FATAL)
			break;
		if(rc > 0)
		{
			tm = in->GetDataTm();
			vumeterin = in->VuMeter();
			rc = encoder->Encode(codec, audiobuffer, rc, buf);
			if(rc > 0)
			{
				ncalls = vps->EnumCalls(vpcalls, 100, (1<<BC_VOICE) | (1<<BC_AUDIOVIDEO));
				for(i = 0; i < ncalls; i++)
					vps->SendAudio(vpcalls[i], codec, in->GetDataTm(), buf, rc);
			}
		}
		Sleep(10);
	}
	in->Stop();
}

int VPSIMPLEAUDIO_GetVuMeters(int *in, int *out)
{
	*in = vumeterin;
	*out = vumeterout;
	return 0;
}

int VPSIMPLEAUDIO_SetMicBoost(int on)
{
	if(on == -1)
	{
		in->micboost = !in->micboost;
		in->SetRecLevel(in->GetRecLevel());
	} else if(!!in->micboost != !!on)
	{
		in->micboost = !!on;
		in->SetRecLevel(in->GetRecLevel());
	}
	return 0;
}

int VPSIMPLEAUDIO_SetLevel(int recording, int level)
{
	if(recording == 2)
		in->SetAudioLine((TCHAR *)level);
	else if(recording)
		in->SetRecLevel(level);
	else {
		int i;

		for(i = 0; i < MAXSIMPLEAUDIO; i++)
			if(instances[i])
				instances[i]->out->SetVolume(level);
	}
	return 0;
}

int VPSIMPLEAUDIO_EnumerateDevices(TCHAR devices[][100], int *ndevices, int recording)
{
#ifdef _WIN32
	int i;

	if(recording == 2)
	{
		in->SetAudioDevice(VPSIMPLEAUDIO_audiodev[1]);
		return in->EnumInputLines(devices, (unsigned *)ndevices);
	} else if(recording)
	{
		if(waveInGetNumDevs() < (unsigned)*ndevices)
			*ndevices = waveInGetNumDevs();
		for(i = 0; i < *ndevices; i++)
		{
			*devices[i] = 0;
			wavein_GetDeviceName(i, devices[i]);
		}
	} else {
#ifdef DSAUDIO
		InitializeDS();
		for(i = 0; ds_GetDeviceName(i); i++)
			strcpy(devices[i], ds_GetDeviceName(i));
		*ndevices = i;
#else
		if(waveOutGetNumDevs() < (unsigned)*ndevices)
			*ndevices = waveOutGetNumDevs();
		for(i = 0; i < *ndevices; i++)
		{
			*devices[i] = 0;
			waveout_GetDeviceName(i, devices[i]);
		}
#endif
	}
#else
	// Simple default
	if(*ndevices)
	{
		_tcscpy(devices[0], TEXT("Audio device"));
		*ndevices = 1;
	}
#endif
	return 0;
}

int VPSIMPLEAUDIO_SetAutoThreshold(int thre)
{
	in->autothreshold = thre;
	in->SetThreshold(-50);
	return 0;
}

int VPSIMPLEAUDIO_Mute(int m)
{
	if(mute == m)
		return 0;
	mute = m;
	if(!mute && in->VuMeter() == -10000)
		StartAudioIn();
	else if(mute)
		vumeterin = -10000;
	return 0;
}

VPAUDIODATA *CreateVPSIMPLEAUDIO(IVPSTACK *vps, VPCALL vpcall, int codec)
{
	return new VPSIMPLEAUDIO(vps, vpcall, codec);
}

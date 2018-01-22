#include <stdio.h>
#include <math.h>
#include "portability.h"
#include "vpstack.h"
#include "vpaudio.h"
#include "audiocodecs.h"
#include "util.h"
#include "vpstack.h"
#include "mixingaudio.h"
#include "audioutil.h"
#include "avifile.h"

#ifdef INCLUDE_AEC
#include "speex/speex_echo.h"
#include "speex/speex_preprocess.h"
#endif

#ifdef _WIN32_WCE
#define MAXSFREQ 8000
#else
#define MAXSFREQ 32000
#endif
#define MAXFRAMELEN (8*1280)	// 8 frames * 1280 samples

static void StartAudioIn();
void VPMIXINGAUDIO_AudioAcquisitionThread(void *dummy);

#ifdef INCLUDE_AEC
static int aec = AEC_NONE, aec_change;
#endif

static int ninstances;	
static int mute, aes;
static void *hwav;
static int vumeterin = -10000, vumeterout = -10000;
static VPMIXINGAUDIO *instances[MAXMIXINGAUDIO];
static CRITICAL_SECTION cs;

static AUDIOIN gin, *in = &gin;
static AUDIOOUT gout, *out = &gout;
static AUDIOBUFFER grecsource, *recsource = &grecsource;

static tchar audiodev[2][100];
static char savepath[MAX_PATH];
static char triggerrestart;

static int SilenceFrame(short *buf, int nsamples, int *peak)
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

#define EXP_THRESHOLD 1.0f

static void EchoSuppression(short *buf, int samples, int vuout_mB, int highechosuppression)
{
	static const float rodown = 0.0005f, roup = 0.005f;
	static const double em = log(10.0)/20.0;
	static float es_refenergy, es_inputenergy;
	int i;
	float o;
	double ro2, newen;
	int refenergy, ref_first, ref_last;

	ref_first = (int)es_refenergy;
	newen = exp(vuout_mB * em / 100.0) * 32767.0;
	if(newen > es_refenergy)
		ro2 = pow(1.00 - roup, samples);
	else ro2 = pow(1.00 - rodown, samples);
	es_refenergy = (float)(ro2 * es_refenergy + (1 - ro2) * newen);
	ref_last = (int)es_refenergy;
	for(i = 0; i < samples; i++)
	{
		refenergy = ref_first * (samples - i) / samples + ref_last * i / samples;
		o = buf[i];
//		printf("%d %f", refenergy, es_inputenergy);
		if(highechosuppression)
		{
			if(es_inputenergy < refenergy * 4.0f)
				o *= es_inputenergy / (refenergy * 4.0f) * es_inputenergy / (refenergy * 4.0f);
		} else if(es_inputenergy * EXP_THRESHOLD < refenergy)
		{
			o *= (es_inputenergy * EXP_THRESHOLD) / refenergy;	// 1:2 expansion on low level signals
//			printf(", expanding %f", (es_inputenergy * EXP_THRESHOLD) / refenergy);
		}
//		printf("\n");
		if(abs(buf[i]) > es_inputenergy)
			es_inputenergy = (1 - rodown) * es_inputenergy + roup * abs(buf[i]);
		else es_inputenergy = (1 - rodown) * es_inputenergy + roup * abs(buf[i]);
		if(o > 32767.0)
			buf[i] = 32767;
		else if(o < -32768.0)
			buf[i] = -32768;
		else buf[i] = (short)o;
	}
}

VPMIXINGAUDIO::VPMIXINGAUDIO(IVPSTACK *vps, VPCALL vpcall, int codec)
{
	int i;

	decoder = new AUDIODECODER;
	encoder = new AUDIOENCODER;
	decresampler = new RESAMPLER;
	outresampler = new RESAMPLER;
	netsource = new AUDIOBUFFER;
	EnterCriticalSection(&cs);
	ninstances++;
	LeaveCriticalSection(&cs);
	this->vps = vps;
	this->vpcall = vpcall;
	this->codec = codec;
	cursample = 0;
	peak = 0;
	hwav = 0;
	save = false;
	framesperpacket = 1;
	if(ninstances == 1)
	{
		if(!mute)
			StartAudioIn();
	}
	for(i = 0; i < MAXMIXINGAUDIO; i++)
		if(!instances[i])
			if(!InterlockedCompareExchangeLong((LONG *)&instances[i], (LONG)this, 0))
				break;
}

VPMIXINGAUDIO::~VPMIXINGAUDIO()
{
	int i;

	EnterCriticalSection(&cs);
	ninstances--;
	for(i = 0; i < MAXMIXINGAUDIO; i++)
		if(instances[i] == this)
			instances[i] = 0;
	LeaveCriticalSection(&cs);
	delete netsource;
	delete decresampler;
	delete outresampler;
	delete decoder;
	if(hwav)
		WAVClose(hwav);
}

static void StartAudioIn()
{
	if(*audiodev[1])
		in->SetAudioDevice(audiodev[1]);
	in->Start(1, MAXSFREQ, MAXSFREQ / 100, 500);	// Start @ 32 kHz, 10 ms frame length
	_beginthread(VPMIXINGAUDIO_AudioAcquisitionThread, 0, 0);
}

void VPMIXINGAUDIO::AudioFrame(int codec, unsigned timestamp, const unsigned char *buf, int buflen)
{
	short audiobuffer[MAXFRAMELEN];
	short audiobufferresampled[MAXSFREQ/8000*MAXFRAMELEN];
	int rc, nsamples, nsamplesinbuffer;

	rc = decoder->Decode(codec, buf, buflen, audiobuffer);
	if(rc > 0)
	{
		if(save)
		{
			BYTE wf[1000];

			save = false;
			if(!Codec2WF(codec, (WAVEFMT *)wf))
			{
				((WAVEFORMATEX *)wf)->cbSize = 0;
				((WAVEFORMATEX *)wf)->wBitsPerSample = 16;
				((WAVEFORMATEX *)wf)->wFormatTag = 1;
				((WAVEFORMATEX *)wf)->nBlockAlign = 2;
				((WAVEFORMATEX *)wf)->nAvgBytesPerSec = ((WAVEFORMATEX *)wf)->nSamplesPerSec * 2;
				hwav = WAVCreate(savepath, (WAVEFMT *)wf);
			}
		}
		if(hwav)
			WAVWrite(hwav, audiobuffer, rc);
		nsamples = rc / 2;
		if(*audiodev[0] && (triggerrestart & 1))
		{
			out->SetAudioDevice(audiodev[0]);
			triggerrestart &= ~1;
		}
		nsamplesinbuffer = netsource->SamplesInBuffer();
		nsamples = decresampler->Resample(codecsfreq[codec], MAXSFREQ, audiobuffer, nsamples, audiobufferresampled);
		if(nsamplesinbuffer <= nsamples)
		{
			static short nullbuffer[MAXSFREQ/8000*MAXFRAMELEN];

			if(nsamples <= 1280)
				netsource->PutData(audiobufferresampled, nsamples);	// Put some initial delay, if the packet is short, repeat it instead of putting zeroes
			else netsource->PutData(nullbuffer, nsamples);
		}
		if(nsamplesinbuffer <= 1600 || nsamplesinbuffer <= 2*nsamples || !SilenceFrame(audiobufferresampled, nsamples, &peak))
			netsource->PutData(audiobufferresampled, nsamples);
	}
}

int VPMIXINGAUDIO::Save(const char *path)
{
	strcpy(savepath, path);
	save = true;
	return 0;
}

/******************************************************************************

  Audio streams flow, three connected users example

In receiver thread:
-------------------
receiver -> decoder -> resampler_to32kHz -> netsource1
receiver -> decoder -> resampler_to32kHz -> netsource2
receiver -> decoder -> resampler_to32kHz -> netsource3

In transmitter thread:
----------------------

recorder_32kHz -> recsource

recsource  -> |
netsource2 -> | -> mixer -> resampler_from32kHz -> encoder -> transmitter1
netsource3 -> |

recsource  -> |
netsource1 -> | -> mixer -> resampler_from32kHz -> encoder -> transmitter2
netsource3 -> |

recsource  -> |
netsource1 -> | -> mixer -> resampler_from32kHz -> encoder -> transmitter3
netsource2 -> |

netsource1 -> |
netsource2 -> | -> mixer -> player_32kHz
netsource3 -> |

The player and recorder have the same frame length (10 ms), so they go in sync
Transmitters have different frame lengths, which are multiples of 10 ms and
they transmit in different moments. netsource and recsource (tAudioBuffer) need
to keep the samples until the latest transmitter takes them.

******************************************************************************/


void VPMIXINGAUDIO_AudioAcquisitionThread(void *dummy)
{
	static short audiobuffermic_orig[MAXSFREQ / 100];	// 10 ms
	static short audiobuffermic_aec[MAXSFREQ / 100];	// 10 ms
	static short audiobuffermix[MAXSFREQ/8000*MAXFRAMELEN];
	static short audiobufferresampled[MAXSFREQ/8000*MAXFRAMELEN];
	static short audiobuffersave[MAXFRAMELEN][2];
	const int recorderframelen = MAXSFREQ / 100;	// 10 ms
	static BYTE buf[1500];
	unsigned short tm;
	short *audiobuffermic;
	int rc, i, j, n;
	int currentsample, nextsample;
	bool conf[MAXMIXINGAUDIO], held[MAXMIXINGAUDIO];
#ifdef INCLUDE_AEC
   SpeexEchoState *st;
   SpeexPreprocessState *den;
   int sampleRate = MAXSFREQ;

   if(aec)
   {
		st = speex_echo_state_init(recorderframelen, aec * sampleRate / 8000);
		den = speex_preprocess_state_init(recorderframelen, sampleRate);
		speex_echo_ctl(st, SPEEX_ECHO_SET_SAMPLING_RATE, &sampleRate);
		speex_preprocess_ctl(den, SPEEX_PREPROCESS_SET_ECHO_STATE, st);
		aec_change = false;
   } else st = 0;
#endif

	vumeterin = vumeterout = -10000;
	recsource->Reset();
	currentsample = 0;
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
	while(ninstances && !mute)
	{
		if(*audiodev[1] && (triggerrestart & 2))
		{
			in->Stop();
			in->SetAudioDevice(audiodev[1]);
			in->Start(1, MAXSFREQ, recorderframelen, 500);	// Start @ 32 kHz, 10 ms frame length
			triggerrestart &= ~2;
		}
		rc = in->GetData(audiobuffermic_orig);
		if(rc == AUDIOERR_FATAL)
			break;
		if(rc > 0)
		{
			tm = in->GetDataTm();
			vumeterin = in->VuMeter();
			memset(audiobuffermix, 0, recorderframelen * 2);
			memset(held, 0, sizeof(held));
			memset(conf, 0, sizeof(conf));
			EnterCriticalSection(&cs);
			for(i = 0; i < MAXMIXINGAUDIO; i++)
			if(instances[i])
			{
				IVPSTACK *vps = instances[i]->vps;
				VPCALL vpcall = instances[i]->vpcall;
				instances[i]->netsource->PeekDataMix(currentsample, audiobuffermix, recorderframelen);
				LeaveCriticalSection(&cs);

				if(vps->IsHeld(vpcall) > 0)
					held[i] = true;
				rc = vps->IsConferenced(vpcall);
				if(rc > 0 && rc & 1)
					conf[i] = true;
				EnterCriticalSection(&cs);
			}
			LeaveCriticalSection(&cs);
			audiobuffermic = audiobuffermic_orig;
#ifdef INCLUDE_AEC
			if(aec_change)
			{
				if(st)
				{
					speex_echo_state_destroy(st);
					speex_preprocess_state_destroy(den);
					st = 0;
				}
				if(aec)
				{
					st = speex_echo_state_init(recorderframelen, aec * sampleRate / 8000);
					den = speex_preprocess_state_init(recorderframelen, sampleRate);
					speex_echo_ctl(st, SPEEX_ECHO_SET_SAMPLING_RATE, &sampleRate);
					speex_preprocess_ctl(den, SPEEX_PREPROCESS_SET_ECHO_STATE, st);
				}
				aec_change = 0;
			}
			if(st)
			{
				speex_echo_cancellation(st, (spx_int16_t *)audiobuffermic_orig, (spx_int16_t *)audiobuffermix, (spx_int16_t *)audiobuffermic_aec);
				speex_preprocess_run(den, (spx_int16_t *)audiobuffermic_aec);
				audiobuffermic = audiobuffermic_aec;
			}
#endif
			if(aes == AES_LOW)
				EchoSuppression(audiobuffermic, recorderframelen, vumeterout, 0);
			else if(aes == AES_HIGH)
				EchoSuppression(audiobuffermic, recorderframelen, vumeterout, 1);
			recsource->PutData(audiobuffermic, recorderframelen);
			out->PutData(1, MAXSFREQ, audiobuffermix, 2 * recorderframelen);
			if(!hwav && *savepath)
			{
				WAVEFMT wf;

				wf.cbSize = 0;
				wf.nBlockAlign = 4;
				wf.nChannels = 2;
				wf.nSamplesPerSec = MAXSFREQ;
				wf.nAvgBytesPerSec = 4*MAXSFREQ;
				wf.wBitsPerSample = 16;
				wf.wFormatTag = 1;
				hwav = WAVCreate(savepath, &wf);
				if(!hwav)
					*savepath = 0;
			} else if(hwav && !*savepath)
			{
				WAVClose(hwav);
				hwav = 0;
			}
			if(hwav)
			{
				for(i = 0; i < recorderframelen; i++)
				{
					audiobuffersave[i][0] = audiobuffermic_orig[i];
					audiobuffersave[i][1] = audiobuffermix[i];
				}
				WAVWrite(hwav, audiobuffersave, recorderframelen*4);
			}
			vumeterout = out->VuMeter();
			currentsample += recorderframelen;
			nextsample = currentsample;
			EnterCriticalSection(&cs);
			for(i = 0; i < MAXMIXINGAUDIO; i++)
			if(instances[i] && !held[i])
			{
				// n contains the number of samples to pick
				n = codecuframelen[instances[i]->codec] / 2 *	// samples/frame
					MAXSFREQ / codecsfreq[instances[i]->codec] *	// correct with sampling frequency
					instances[i]->framesperpacket;				// multiply with the number of frames
				if(!recsource->PeekData(&instances[i]->cursample, audiobuffermix, n))
				{
					if(conf[i])
					for(j = 0; j < MAXMIXINGAUDIO; j++)
						if(i != j && conf[j] && instances[j]->netsource)
							instances[j]->netsource->PeekDataMix(instances[i]->cursample, audiobuffermix, n);
					instances[i]->cursample += n;
					rc = instances[i]->outresampler->Resample(MAXSFREQ, codecsfreq[instances[i]->codec], audiobuffermix, n, audiobufferresampled);
					rc = instances[i]->encoder->Encode(instances[i]->codec, audiobufferresampled, rc * 2, (BYTE *)buf);
					if(rc > 0)
					{
						IVPSTACK *vps = instances[i]->vps;
						VPCALL vpcall = instances[i]->vpcall;
						int codec = instances[i]->codec;

						LeaveCriticalSection(&cs);
						vps->SendAudio(vpcall, codec, tm, buf, rc);
						EnterCriticalSection(&cs);
					}
				}
				if(instances[i] && instances[i]->cursample - nextsample >= 0x80000000)
					nextsample = instances[i]->cursample;
			}
			for(i = 0; i < MAXMIXINGAUDIO; i++)
			if(instances[i])
				instances[i]->netsource->Advance(nextsample);
			LeaveCriticalSection(&cs);
			recsource->Advance(nextsample);
		} else Sleep(10);
	}
	in->Stop();
	out->Stop();
	vumeterin = vumeterout = -10000;
#ifdef INCLUDE_AEC
	if(st)
	{
		speex_echo_state_destroy(st);
		speex_preprocess_state_destroy(den);
	}
#endif
}

int VPMIXINGAUDIO_Init()
{
	static bool init;
	
	if(init)
		return 0;
	init = true;
#ifdef DSAUDIO
	InitializeDS();
#endif
#ifdef TARGET_OS_IPHONE
    AudioSessionInitialize(NULL, NULL, NULL, NULL);
#endif
	InitializeCriticalSection(&cs);
	return 0;
}

int VPMIXINGAUDIO_GetVuMeters(int *in, int *out)
{
	*in = vumeterin;
	*out = vumeterout;
	return 0;
}

int VPMIXINGAUDIO_SetMicBoost(int on)
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

int VPMIXINGAUDIO_SetLevel(int recording, int level)
{
	if(recording == 2)
		in->SetAudioLine((TCHAR *)level);
	else if(recording)
		in->SetRecLevel(level);
	else out->SetVolume(level);
	return 0;
}

#ifdef UNICODE
int VPMIXINGAUDIO_EnumerateDevices(char devices[][100], int *ndevices, int recording)
{
	int rc, i;
	TCHAR (*devices1)[100];

	if(*ndevices)
	{
		devices1 = (TCHAR (*)[100])malloc(*ndevices * sizeof(TCHAR) * 100);
	} else devices1 = 0;
	if(recording == 2)
	{
		in->SetAudioDevice(audiodev[1]);
		rc = in->EnumInputLines(devices1, (unsigned *)ndevices);
	} else rc = AUDIO_EnumerateDevices(devices1, ndevices, recording);
	if(!rc)
	{
		if(devices1)
		{
			for(i = 0; i < *ndevices; i++)
				WindowsStringToUTF8(devices1[i], devices[i]);
			free(devices1);
		}
	} else *ndevices = 0;
	return rc;
}
#endif

int VPMIXINGAUDIO_EnumerateDevices(TCHAR devices[][100], int *ndevices, int recording)
{
	if(recording == 2)
	{
		in->SetAudioDevice(audiodev[1]);
		return in->EnumInputLines(devices, (unsigned *)ndevices);
	} else return AUDIO_EnumerateDevices(devices, ndevices, recording);
}

int VPMIXINGAUDIO_SetAutoThreshold(int thre)
{
	in->autothreshold = thre;
	in->SetThreshold(-50);
	return 0;
}

int VPMIXINGAUDIO_Mute(int m)
{
#ifdef TARGET_OS_IPHONE
	bool param = m == 0 ? false : true;
	in->SetMute(param);
	return 0;
#else	
	if(mute == m)
		return 0;
	mute = m;
	if(!mute && in->VuMeter() == -10000)
		StartAudioIn();
	else if(mute)
		vumeterin = -10000;
	return 0;
#endif	
}

#ifdef TARGET_OS_IPHONE                                                                                                     
bool VPMIXINGAUDIO_GetMute()
{
	return in->IsMute();
}

void VPMIXINGAUDIO_SetMute(bool AMute)
{
	return in->SetMute(AMute);
}

bool VPMIXINGAUDIO_GetSpeaker()
{
	return out->IsSpeaker();
}

void VPMIXINGAUDIO_SetSpeaker(bool ASpeaker)
{
	return out->SetSpeaker(ASpeaker);
}

void VPMIXINGAUDIO_Stop()
{
//	in->Stop();
	out->Stop();
}
#endif   


#ifdef UNICODE
int VPMIXINGAUDIO_SetAudioDevice(int recording, const char *device)
{
	if(recording < 0 || recording > 1)
		return -1;
	UTF8ToWindowsString(device, audiodev[recording]);
	triggerrestart = 3;
	return 0;
}
#endif

int VPMIXINGAUDIO_SetAudioDevice(int recording, const tchar *device)
{
	if(recording < 0 || recording > 1)
		return -1;
	_tcsncpy(audiodev[recording], device, sizeof audiodev[0]);
	triggerrestart = 3;
	return 0;
}

VPAUDIODATA *CreateVPMIXINGAUDIO(IVPSTACK *vps, VPCALL vpcall, int codec)
{
	return new VPMIXINGAUDIO(vps, vpcall, codec);
}

int VPMIXINGAUDIO_SetAcousticEchoSuppression(int type)
{
	aes = type;
	return 0;
}

int VPMIXINGAUDIO_SetAcousticEchoCancellation(int type)
{
#ifdef INCLUDE_AEC
	aec = type;
	aec_change = true;
	return 0;
#else
	return VPERR_UNSUPPORTEDCALL;
#endif
}

int VPMIXINGAUDIO_RecordToFile(const char *path)
{
	if(path)
		strcpy(savepath, path);
	else {
		*savepath = 0;
		if((!ninstances || mute) && hwav)
		{
			WAVClose(hwav);
			hwav = 0;
		}
	}
	return 0;
}

#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include "../portability.h"
#include "winaudio.h"

static void wipespaces(TCHAR *s)
{
	TCHAR *p;

	p = s + _tcslen(s) - 1;
	while(p > s && *p == ' ')
		*p-- = 0;
	for(p = s; *p == ' '; p++);
	if(p > s)
		memmove(s, p, (_tcslen(p) + 1) * sizeof(TCHAR));
}

static void DS_EnumerateDevices();

static void WaveOutPeriodical(void *audio)
{
	((tAudioOut *)audio)->Periodical();
}

int tAudioOut::SetVolume(int thousands)
{
	volume = thousands * 65535 / 1000;
	if(hwo)
		waveOutSetVolume(hwo, (volume << 16) | volume);
	return 0;
}

int tAudioOut::Start(int channels, int sfreq)
{
	WAVEFORMATEX wf;
	HWAVEOUT h;
	int i;

	stop = buftail = bufhead = playing = underrun = 0;
	playedsamples = sentsamples = 0;
	vumeter = -9000;
	gsfreq = sfreq;
	wf.wBitsPerSample = 16;
	wf.wFormatTag = WAVE_FORMAT_PCM;
	wf.nChannels = channels;
	wf.nSamplesPerSec = sfreq;
	wf.nAvgBytesPerSec = 2 * channels * sfreq;
	wf.nBlockAlign = 2 * channels;
	wf.cbSize = 0;
	if(waveOutOpen(&h, playdev, &wf, 0, 0, 0))
		return -1;
	waveOutPause(h);
	if(volume != 0xffffffff)
		waveOutSetVolume(h, (volume << 16) | volume);
	for(i = 0; i < BUFFERS; i++)
		wh[i].dwUser = 0;
	playbufs = BUFFERS;
	hwo = h;
	_beginthread(WaveOutPeriodical, 0, this);
	return 0;
}

int tAudioOut::Stop()
{
	Stop(1);
	return 0;
}

int tAudioOut::Stop(int docs)
{
	int i;

	stop = 1;
	while(periodicalactive)
		Sleep(10);
	if(docs)
		EnterCriticalSection(&cs);
	if(!hwo)
	{
		if(docs)
			LeaveCriticalSection(&cs);
		return 0;
	}
	playing = 0;
	waveOutReset(hwo);
	for(i = 0; i < playbufs; i++)
	{
		if(wh[i].dwUser)
		{
			waveOutUnprepareHeader(hwo, &wh[i], sizeof(WAVEHDR));
			free(wh[i].lpData);
			wh[i].dwFlags = 0;
			wh[i].dwUser = 0;
		}
	}
	waveOutClose(hwo);
	hwo = 0;
	if(docs)
		LeaveCriticalSection(&cs);
	return 0;
}

int tAudioOut::SetAudioDevice(TCHAR *sdev)
{
	int i, n = waveOutGetNumDevs();

	playdev = WAVE_MAPPER;
	for(i = 0; i < n; i++)
	{
		WAVEOUTCAPS woc;

		waveOutGetDevCaps(i, &woc, sizeof(woc));
		wipespaces(woc.szPname);
		if(!_tcscmp(woc.szPname, sdev))
		{
			playdev = i;
			return 0;
		}
	}
	return -1;
}

int tAudioOut::CheckAudioDevice(TCHAR *sdev)
{
	int i, n = waveOutGetNumDevs();
	WAVEOUTCAPS woc;

	for(i = 0; i < n; i++)
	{
		waveOutGetDevCaps(i, &woc, sizeof(woc));
		wipespaces(woc.szPname);
		if(!_tcscmp(woc.szPname, sdev))
			return 0;
	}
	if(n)
	{
		waveOutGetDevCaps(0, &woc, sizeof(woc));
		wipespaces(woc.szPname);
		_tcscpy(sdev, woc.szPname);
		return -1;
	} else {
		*sdev = 0;
		return -1;
	}
}

int tAudioOut::PutData(int channels, int sfreq, short *buf, int len)
{
	short *p;
	int x, n;

	EnterCriticalSection(&cs);
	if(sfreq != gsfreq)
		Stop(0);
	if(!hwo)
		if(Start(channels, sfreq))
		{
			LeaveCriticalSection(&cs);
			return -1;
		}
	if(buftail == playbufs - 1 && bufhead == 1 || buftail == playbufs - 2 && bufhead == 0 ||
		buftail < playbufs - 2 && buftail + 2 == bufhead)
	{
		LeaveCriticalSection(&cs);
		return -1;	// Overrun
	}
	if(sentsamples + len / 2 > playedsamples + gmaxdelay * gsfreq / 1000)
	{
		// Delay exceeded: silently drop frame
		LeaveCriticalSection(&cs);
		return -2;
	}
	wh[buftail].lpData = (char *)malloc(len);
	wh[buftail].dwBufferLength = len;
	wh[buftail].dwFlags = 0;
	wh[buftail].dwUser = 1;
	waveOutPrepareHeader(hwo, &wh[buftail], sizeof(WAVEHDR));
	p = (short *)wh[buftail].lpData;
	memcpy(p, buf, len);
	len /= 2;
	if(outgain)
	{
		for(n = 0; n < len; n++)
		{
			x = p[n] * 3;
			if(x > 32767)
				x = 32767;
			else if(x < -32768)
				x = -32768;
			p[n] = (short)x;
		}
	}
	waveOutWrite(hwo, &wh[buftail], sizeof(WAVEHDR));
	buftail++;
	if(buftail == playbufs)
		buftail = 0;
	sentsamples += len;
	gdelay = 3 * len * 1000 / gsfreq;
	if(sentsamples * 1000 >= gdelay * gsfreq && !playing)
	{
		playing = 1;
		waveOutRestart(hwo);
	}
	LeaveCriticalSection(&cs);
	return 0;
}

int tAudioOut::ForceStart()
{
	if(hwo && !playing)
	{
		playing = 1;
		waveOutRestart(hwo);
	}
	return 0;
}

int tAudioOut::SetInitialDelay(int delay)
{
	gdelay = delay;
	return 0;
}

int tAudioOut::SetBufferLength(int delay)
{
	if(delay < 50)
		delay = 50;
	gmaxdelay = delay;
	return 0;
}

int tAudioOut::Beep(int sfreqmultiplier)
{
	short frame[800];
	int i;

	for(i = 0; i < 400 * sfreqmultiplier; i++)
		frame[i] = (short)(10000 * sin(i / (2.0 * sfreqmultiplier)));
	PutData(1, 8000 * sfreqmultiplier, frame, 800 * sfreqmultiplier);
	ForceStart();
	return 0;
}

void tAudioOut::Periodical()
{
	short *p;
	int max, x, n, len;

	periodicalactive = 1;
	while(!stop)
	{
		EnterCriticalSection(&cs);
		while(wh[bufhead].dwFlags & WHDR_DONE)
		{
			playedsamples += wh[bufhead].dwBufferLength / 2;
			waveOutUnprepareHeader(hwo, &wh[bufhead], sizeof(WAVEHDR));
			free(wh[bufhead].lpData);
			wh[bufhead].dwFlags = 0;
			wh[bufhead].dwUser = 0;
			bufhead++;
			if(bufhead == playbufs)
				bufhead = 0;
		}
		max = 1;
		if(wh[bufhead].dwUser)
		{
			p = (short *)wh[bufhead].lpData;
			len = wh[bufhead].dwBufferLength / 2;
			for(n = 0; n < len; n++)
			{
				x = p[n] * p[n];
				if(x > max)
					max = x;
			}
		}
		vumeter = (int)(1000.0 * log10(max / 1073741824.0));
		if(buftail == bufhead && playing)
		{
			periodicalactive = 0;
			Stop(0);
			LeaveCriticalSection(&cs);
			return;
		}
		LeaveCriticalSection(&cs);
		Sleep(10);
	}
	periodicalactive = 0;
}

int tAudioOut::VuMeter(void)
{
	if(!playing)
		return -10000;
	return vumeter;
}

int tAudioOut::GetCurrentDelay()
{
	if(playing && gsfreq)
		return (int)(1000 * (sentsamples - playedsamples) / gsfreq);
	else return -1;
}

int tAudioIn::SetAudioDevice(TCHAR *sdev)
{
	int i, n = waveInGetNumDevs();

	recorddev = WAVE_MAPPER;
	for(i = 0; i < n; i++)
	{
		WAVEINCAPS wic;

		waveInGetDevCaps(i, &wic, sizeof(wic));
		wipespaces(wic.szPname);
		if(!_tcscmp(wic.szPname, sdev))
		{
			if(i != recorddev)
			{
				recorddev = i;
				if(hwi)
					Start(1, gsfreq, gframelen, gbuflength);
			}
			return 0;
		}
	}
	return -1;
}

int tAudioIn::CheckAudioDevice(TCHAR *sdev)
{
	int i, n = waveInGetNumDevs();
	WAVEINCAPS wic;

	for(i = 0; i < n; i++)
	{
		waveInGetDevCaps(i, &wic, sizeof(wic));
		wipespaces(wic.szPname);
		if(!_tcscmp(wic.szPname, sdev))
			return 0;
	}
	if(n)
	{
		waveInGetDevCaps(0, &wic, sizeof(wic));
		wipespaces(wic.szPname);
		_tcscpy(sdev, wic.szPname);
		return -1;
	} else {
		*sdev = 0;
		return -1;
	}
}

int tAudioIn::Start(int channels, int sfreq, int framelen, int buflength)
{
	WAVEFORMATEX wf;
	HWAVEIN h;
	int i;

	EnterCriticalSection(&cs);
	if(hwi)
		Stop();
	silencedetected = 0;
	rframes = 0;
	hwi = 0;
	bufhead = 0;
	gbuflength = buflength;
	gframelen = framelen;
	gsfreq = sfreq;
	wf.wBitsPerSample = 16;
	wf.wFormatTag = WAVE_FORMAT_PCM;
	wf.nChannels = 1;
	wf.nSamplesPerSec = sfreq;
	wf.nAvgBytesPerSec = 2 * sfreq;
	wf.nBlockAlign = 2;
	wf.cbSize = 0;
	if(waveInOpen(&h, recorddev, &wf, 0, 0, 0))
	{
		LeaveCriticalSection(&cs);
		return -1;
	}
	grecordbufs = sfreq / framelen * buflength / 1000;
	if(grecordbufs < 6)
		grecordbufs = 6;
	if(grecordbufs > BUFFERS)
		grecordbufs = BUFFERS;
	for(i = 0; i < grecordbufs; i++)
	{
		wh[i].lpData = (char *)malloc(2 * framelen);
		wh[i].dwBufferLength = 2 * framelen;
		wh[i].dwFlags = 0;
		waveInPrepareHeader(h, &wh[i], sizeof(WAVEHDR));
		waveInAddBuffer(h, &wh[i], sizeof(WAVEHDR));
	}
	hpfoutmem[0] = hpfoutmem[1] = hpfinmem[0] = hpfinmem[1] = 0;
	waveInStart(h);
	tm = GetTickCount();
	hwi = h;
	SetRecLevel(reclevel);
	es_refenergy = 0;
	es_inputenergy = 0;
	vu_level = -100;
	LeaveCriticalSection(&cs);
	return 0;
}

int tAudioIn::Stop()
{
	int i;

	EnterCriticalSection(&cs);
	if(!hwi)
	{
		LeaveCriticalSection(&cs);
		return 0;
	}
	waveInReset(hwi);
	for(i = 0; i < grecordbufs; i++)
	{
		waveInUnprepareHeader(hwi, &wh[i], sizeof(WAVEHDR));
		free(wh[i].lpData);
		wh[i].dwFlags = 0;
	}
	waveInClose(hwi);
	hwi = 0;
	LeaveCriticalSection(&cs);
	return 0;
}

static void HPFilter(short *in, short *out, int len, int *inbuf, double *outbuf)
{
	// butter(2,0.0002,'high');
	static const double coefB[3] = {0.99955581038761, -1.99911162077522, 0.99955581038761};
	static const double coefA[2] = {-1.99911142347080, 0.99911181807964};
	int i, yi;
	double y;

	y = coefB[0] * in[0] + coefB[1] * inbuf[0] + coefB[2] * inbuf[1] - coefA[0] * outbuf[0] - coefA[1] * outbuf[1];
	yi = (int)y;
	out[0] = yi > 32767 ? 32767 : yi < -32678 ? -32768 : yi;
	outbuf[1] = outbuf[0];
	outbuf[0] = y;
	y = coefB[0] * in[1] + coefB[1] * in[0] + coefB[2] * inbuf[0] - coefA[0] * outbuf[0] - coefA[1] * outbuf[1];
	yi = (int)y;
	out[1] = yi > 32767 ? 32767 : yi < -32678 ? -32768 : yi;
	outbuf[1] = outbuf[0];
	outbuf[0] = y;
	for(i = 2; i < len; i++)
	{
		y = coefB[0] * in[i] + coefB[1] * in[i - 1] + coefB[2] * in[i - 2] - coefA[0] * outbuf[0] - coefA[1] * outbuf[1];
		yi = (int)y;
		out[i] = yi > 32767 ? 32767 : yi < -32678 ? -32768 : yi;
		outbuf[1] = outbuf[0];
		outbuf[0] = y;
	}
	inbuf[1] = in[len - 2];
	inbuf[0] = in[len - 1];
}

int tAudioIn::GetData(short *data)
{
	int len, n, x, max = 1;

	EnterCriticalSection(&cs);
	if(!hwi)
	{
		LeaveCriticalSection(&cs);
		return AUDIOERR_FATAL;
	}
	if(!(wh[bufhead].dwFlags & WHDR_DONE))
	{
		LeaveCriticalSection(&cs);
		return -1;
	}
	HPFilter((short *)wh[bufhead].lpData, data, wh[bufhead].dwBufferLength / 2, hpfinmem, hpfoutmem);
	len = wh[bufhead].dwBufferLength / 2;
	if(ingain)
	{
		for(n = 0; n < len; n++)
		{
			x = data[n] * 10;
			if(x > 32767)
				x = 32767;
			else if(x < -32768)
				x = -32768;
			data[n] = (short)x;
		}
	}
	waveInAddBuffer(hwi, &wh[bufhead], sizeof(WAVEHDR));
	bufhead++;
	rframes++;
	if(bufhead == grecordbufs)
		bufhead = 0;

	EchoSuppression(data, len);
	for(n = 0; n < len; n++)
	{
		x = data[n] * data[n];
		if(x > max)
			max = x;
	}
	vumeter = (int)(1000.0 * log10(max / 1073741824.0));
	/*{
		char s[300];
		sprintf(s, "vu=%d, n=%d, n2=%d*%d/%d = %d\r\n",
			vumeter, noiselevel, noiselevel, 10*2*gsfreq-gframelen, 10*2*gsfreq, noiselevel * (10*2*gsfreq - gframelen) / (10*2*gsfreq));
		OutputDebugString(s);
	}*/
	noiselevel = noiselevel * (10*2*gsfreq - gframelen) / (10*2*gsfreq);
	if(vumeter < noiselevel)
		noiselevel = vumeter;
	if(autothreshold)
		threshold = noiselevel / 100 + autothreshold;
	if(threshold > -50 && vumeter < threshold * 100)
	{
		if(silencedetected == 10)
		{
			LeaveCriticalSection(&cs);
			return -2;
		}
		silencedetected++;
	} else silencedetected = 0;
	tm += len * 1000 / gsfreq;
	LeaveCriticalSection(&cs);
	return 2 * len;
}

int tAudioIn::VuMeter()
{
	if(!hwi)
		return -10000;
	return vumeter;
}

#define EXP_THRESHOLD 1.0f

void tAudioIn::EchoSuppression(short *buf, int samples)
{
	int i;
	float o;
	static const float rodown = 0.0005f, roup = 0.005f;
	double ro2, newen;
	static const double em = log(10.0)/20.0;
	int refenergy, ref_first, ref_last;

	ref_first = (int)es_refenergy;
	newen = exp(vu_level * em) * 32767.0;
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

int tAudioIn::SetAudioLine(TCHAR *linename)
{
	_tcscpy(recline, linename);
	SetRecLevel(reclevel);
	return 0;
}

static TCHAR *stristr(TCHAR *string, TCHAR *substring)
{
	int len = _tcslen(substring);
	
	while(*string)
	{
		if(!memicmp(string, substring, sizeof(TCHAR) * len))
			return string;
		string++;
	}
	return 0;
}

int tAudioIn::SetRecLevel(int thousands)
{
	HMIXER hmx;
	MIXERLINE mxl;
	MIXERLINECONTROLS mxlc;
	MIXERCONTROL mxc;
	MIXERCONTROLDETAILS mxcd;
	MIXERCONTROLDETAILS_UNSIGNED volume[2];
	MIXERCONTROLDETAILS_BOOLEAN mcmute[2];
	MIXERCAPS mxcaps;
	int left = thousands * 65535 / 1000, right = left, mute = true;
	TCHAR *devname = recline;
	bool has_micboost;
	int rc, connections[2] = {0,0}, canselect[2] = {0, 0}, reccontrolid[2], recmultipleitems[2], i, j,
		componentid[2] = {-1,-1}, componentdest[2] = {-1,-1}, controls[2];

	reclevel = thousands;
	if(mixerGetNumDevs() < 1)
		return -1;
	if(!hwi)
		return 0;
	if(mixerOpen(&hmx, (UINT)hwi, 0, 0, MIXER_OBJECTF_HWAVEIN) != MMSYSERR_NOERROR)
		return -1;
	if(mixerGetDevCaps((UINT)hmx, &mxcaps, sizeof(mxcaps)) != MMSYSERR_NOERROR)
	{
		mixerClose(hmx);
		return -1;
	}
	mxl.cbStruct = sizeof(mxl);
	for(mxl.dwDestination = 0; mxl.dwDestination < mxcaps.cDestinations; mxl.dwDestination++)
	{
		if(mixerGetLineInfo((HMIXEROBJ)hmx, &mxl, MIXER_GETLINEINFOF_DESTINATION | MIXER_OBJECTF_HMIXER) != MMSYSERR_NOERROR)
		{
			mixerClose(hmx);
			return -1;
		}
		if(mxl.dwComponentType == MIXERLINE_COMPONENTTYPE_DST_WAVEIN)
		{
			componentid[0] = mxl.dwLineID;
			componentdest[0] = mxl.dwDestination;
			connections[0] = mxl.cConnections;
			controls[0] = mxl.cControls;
		} else if(mxl.dwComponentType == MIXERLINE_COMPONENTTYPE_DST_SPEAKERS)
		{
			componentid[1] = mxl.dwLineID;
			componentdest[1] = mxl.dwDestination;
			connections[1] = mxl.cConnections;
			controls[1] = mxl.cControls;
		}
	}
	if(devname && *devname)
	for(i = 0; i < 2 && componentid[i] != -1; i++)
	{
		mxlc.cbStruct = sizeof(mxlc);
		mxlc.dwLineID = componentid[i];
		mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_MUX;
		mxlc.cControls = 1;
		mxlc.cbmxctrl = sizeof(mxc);
		mxlc.pamxctrl = &mxc;
		if((rc = mixerGetLineControls((HMIXEROBJ)hmx, &mxlc, MIXER_GETLINECONTROLSF_ONEBYTYPE | MIXER_OBJECTF_HMIXER)) == MMSYSERR_NOERROR)
		{
			reccontrolid[i] = mxc.dwControlID;
			recmultipleitems[i] = mxc.cMultipleItems;
			canselect[i] = 1;
		} else {
			mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_MIXER;
			if((rc = mixerGetLineControls((HMIXEROBJ)hmx, &mxlc, MIXER_GETLINECONTROLSF_ONEBYTYPE | MIXER_OBJECTF_HMIXER)) == MMSYSERR_NOERROR)
			{
				reccontrolid[i] = mxc.dwControlID;
				recmultipleitems[i] = mxc.cMultipleItems;
				canselect[i] = 1;
			} else {
				mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_MULTIPLESELECT;
				if((rc = mixerGetLineControls((HMIXEROBJ)hmx, &mxlc, MIXER_GETLINECONTROLSF_ONEBYTYPE | MIXER_OBJECTF_HMIXER)) == MMSYSERR_NOERROR)
				{
					reccontrolid[i] = mxc.dwControlID;
					recmultipleitems[i] = mxc.cMultipleItems;
					canselect[i] = 2;
				}
			}
		}
	}
	for(i = 0; i < 1 && componentdest[i] != -1; i++)	// Change to 2 instead of 1 if you want to change mute
	for(mxl.dwSource = 0; mxl.dwSource < (unsigned)connections[i]; mxl.dwSource++)
	{
		mxl.cbStruct = sizeof(mxl);
		mxl.dwDestination = componentdest[i];
		if(mixerGetLineInfo((HMIXEROBJ)hmx, &mxl, MIXER_GETLINEINFOF_SOURCE | MIXER_OBJECTF_HMIXER) != MMSYSERR_NOERROR)
		{
			mixerClose(hmx);
			return -1;
		}
		if(devname && *devname && _tcsicmp(mxl.szName, devname))
			continue;
		for(j = 0; j < controls[i]; j++)
		{
			mxlc.cbStruct = sizeof(mxlc);
			mxlc.dwLineID = componentid[i];
			if(j == 0)
				mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
			else mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;
			mxlc.cControls = 1;
			mxlc.cbmxctrl = sizeof(mxc);
			mxlc.pamxctrl = &mxc;
			if((rc = mixerGetLineControls((HMIXEROBJ)hmx, &mxlc, MIXER_GETLINECONTROLSF_ONEBYTYPE | MIXER_OBJECTF_HMIXER)) != MMSYSERR_NOERROR)
				continue;
			mxcd.cbStruct = sizeof(mxcd);
			mxcd.dwControlID = mxc.dwControlID;
			mxcd.cChannels = 1;
			mxcd.cMultipleItems = 0;
			if(j == 0)
			{
				mxcd.cbDetails = sizeof(volume);
				mxcd.paDetails = volume;
				volume[0].dwValue = (mxc.Bounds.dwMaximum - mxc.Bounds.dwMinimum) * left / 65535 + mxc.Bounds.dwMinimum;
				volume[1].dwValue = (mxc.Bounds.dwMaximum - mxc.Bounds.dwMinimum) * right / 65535 + mxc.Bounds.dwMinimum;
			} else {
				mxcd.cbDetails = sizeof(mcmute);
				mxcd.paDetails = mcmute;
				mcmute[0].fValue = mute;
				mcmute[1].fValue = mute;
			}
			if((rc = mixerSetControlDetails((HMIXEROBJ)hmx, &mxcd, MIXER_OBJECTF_HMIXER)) != MMSYSERR_NOERROR)
			{
				mixerClose(hmx);
				return -1;
			}
			break;	// Take away if you want to change mute
		}
		if(mxl.cControls == 0)
			continue;
		mxlc.cbStruct = sizeof(mxlc);
		mxlc.dwLineID = mxl.dwLineID;
		if(i == 0)
			mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
		else mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;
		mxlc.cControls = 1;
		mxlc.cbmxctrl = sizeof(mxc);
		mxlc.pamxctrl = &mxc;
		if((rc = mixerGetLineControls((HMIXEROBJ)hmx, &mxlc, MIXER_GETLINECONTROLSF_ONEBYTYPE | MIXER_OBJECTF_HMIXER)) != MMSYSERR_NOERROR)
			continue;
		mxcd.cbStruct = sizeof(mxcd);
		mxcd.dwControlID = mxc.dwControlID;
		mxcd.cChannels = 1;
		mxcd.cMultipleItems = 0;
		if(i == 0)
		{
			mxcd.cbDetails = sizeof(volume);
			mxcd.paDetails = volume;
			volume[0].dwValue = (mxc.Bounds.dwMaximum - mxc.Bounds.dwMinimum) * left / 65535 + mxc.Bounds.dwMinimum;
			volume[1].dwValue = (mxc.Bounds.dwMaximum - mxc.Bounds.dwMinimum) * right / 65535 + mxc.Bounds.dwMinimum;
		} else {
			mxcd.cbDetails = sizeof(mcmute);
			mxcd.paDetails = mcmute;
			mcmute[0].fValue = mute;
			mcmute[1].fValue = mute;
		}
		if((rc = mixerSetControlDetails((HMIXEROBJ)hmx, &mxcd, MIXER_OBJECTF_HMIXER)) != MMSYSERR_NOERROR)
		{
			mixerClose(hmx);
			return -1;
		}
	}
	i = 0;
	if(canselect[i])
	{
		MIXERCONTROLDETAILS_BOOLEAN recsource[20];
		MIXERCONTROLDETAILS_LISTTEXT recsourcetext[20];
		int selectposition, j;

		mxcd.cbStruct = sizeof(mxcd);
		mxcd.dwControlID = reccontrolid[i];
		mxcd.cChannels = 1;
		mxcd.cMultipleItems = recmultipleitems[i];
		mxcd.cbDetails = sizeof(recsourcetext[0]);
		mxcd.paDetails = recsourcetext;
		if((rc = mixerGetControlDetails((HMIXEROBJ)hmx, &mxcd, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_LISTTEXT)) != MMSYSERR_NOERROR)
		{
			mixerClose(hmx);
			return -1;
		}
		selectposition = -1;
		for(j = 0; j < recmultipleitems[i]; j++)
			if(!_tcscmp(recsourcetext[j].szName, devname))
			{
				selectposition = j;
				break;
			}
		if(selectposition != -1)
		{
			mxcd.cbStruct = sizeof(mxcd);
			mxcd.dwControlID = reccontrolid[i];
			mxcd.cChannels = 1;
			mxcd.cMultipleItems = recmultipleitems[i];
			mxcd.cbDetails = sizeof(recsource[0]);
			mxcd.paDetails = recsource;
			if(canselect[i] == 1)
			{
				memset(recsource, 0, sizeof(recsource));
				recsource[selectposition].fValue = 1;
			} else {
				for(i = 0; i < recmultipleitems[i]; i++)
					recsource[i].fValue = 1;
				recsource[selectposition].fValue = 0;
			}
			if((rc = mixerSetControlDetails((HMIXEROBJ)hmx, &mxcd, MIXER_OBJECTF_HMIXER)) != MMSYSERR_NOERROR)
			{
				mixerClose(hmx);
				return -1;
			}
		}
	}
	// Set Mic boost
	has_micboost = false;
	for(mxl.dwSource = 0; mxl.dwSource < (unsigned)connections[0]; mxl.dwSource++)
	{
		MIXERCONTROL mxc[20];

		mxl.cbStruct = sizeof(mxl);
		mxl.dwDestination = componentdest[0];
		if(mixerGetLineInfo((HMIXEROBJ)hmx, &mxl, MIXER_GETLINEINFOF_SOURCE | MIXER_OBJECTF_HMIXER) != MMSYSERR_NOERROR)
		{
			mixerClose(hmx);
			return -1;
		}
		if(devname && *devname)
		{
			if(_tcsicmp(mxl.szName, devname))
				continue;
		} else if(mxl.dwComponentType != MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE)
			continue;
		if(*recline && _tcscmp(mxl.szName, recline))
			break;
		mxlc.cbStruct = sizeof(mxlc);
		mxlc.dwLineID = mxl.dwLineID;
		mxlc.dwControlType = 0;
		mxlc.cControls = 10;
		mxlc.cbmxctrl = sizeof(mxc[0]);
		mxlc.pamxctrl = mxc;
		if((rc = mixerGetLineControls((HMIXEROBJ)hmx, &mxlc, MIXER_GETLINECONTROLSF_ALL | MIXER_OBJECTF_HMIXER)) != MMSYSERR_NOERROR)
			continue;
		for(i = 0; i < (int)mxlc.cControls; i++)
		{
			if((mxc[i].dwControlType & MIXERCONTROL_CT_UNITS_MASK) != MIXERCONTROL_CT_UNITS_BOOLEAN)
				continue;
			if(stristr(mxc[i].szName, TEXT("gain")) || stristr(mxc[i].szName, TEXT("boost")) || stristr(mxc[i].szName, TEXT("20")))
			{
				MIXERCONTROLDETAILS_BOOLEAN mcd_micboost;

				mcd_micboost.fValue = micboost;
				mxcd.cbStruct = sizeof(mxcd);
				mxcd.dwControlID = mxc[i].dwControlID;
				mxcd.cChannels = 1;
				mxcd.cMultipleItems = 0;
				mxcd.cbDetails = sizeof(mcd_micboost);
				mxcd.paDetails = &mcd_micboost;
				if((rc = mixerSetControlDetails((HMIXEROBJ)hmx, &mxcd, MIXER_OBJECTF_HMIXER)) != MMSYSERR_NOERROR)
				{
					mixerClose(hmx);
					return -1;
				}
				has_micboost = true;
				break;
			}
		}
		break;
	}
	if(!has_micboost)
		ingain = micboost;
	else ingain = 0;
	noiselevel = 0;

	mixerClose(hmx);
	return 0;
}

int tAudioIn::FillCBWithInputLines(HWND hWnd, int defcomponenttype)
{
	WAVEFORMATEX fmt;
	HWAVEIN hwitmp;
	WAVEINCAPS wic;
	int rc;

	if(hwi)
		return AuFillCBWithInputLines(hwi, hWnd, defcomponenttype);

	fmt.wFormatTag = WAVE_FORMAT_PCM;
	fmt.nChannels = 1;
	fmt.nSamplesPerSec = 8000;
	fmt.nAvgBytesPerSec = 2 * 8000;
	fmt.nBlockAlign = 2;
	fmt.wBitsPerSample = 16;
	fmt.cbSize = sizeof(WAVEFORMATEX);
	waveInGetDevCaps(recorddev, &wic, sizeof(wic));
	if(rc = waveInOpen(&hwitmp, recorddev, &fmt, 0, 0, 0))
		return -1;
	rc = AuFillCBWithInputLines(hwitmp, hWnd, defcomponenttype);
	waveInClose(hwitmp);
	return rc;
}

int tAudioIn::EnumInputLines(TCHAR inputlines[][100], unsigned *ninputlines)
{
	WAVEFORMATEX fmt;
	HWAVEIN hwitmp;
	WAVEINCAPS wic;
	int rc;

	if(hwi)
		return ::EnumInputLines(hwi, inputlines, ninputlines);

	fmt.wFormatTag = WAVE_FORMAT_PCM;
	fmt.nChannels = 1;
	fmt.nSamplesPerSec = 8000;
	fmt.nAvgBytesPerSec = 2 * 8000;
	fmt.nBlockAlign = 2;
	fmt.wBitsPerSample = 16;
	fmt.cbSize = sizeof(WAVEFORMATEX);
	waveInGetDevCaps(recorddev, &wic, sizeof(wic));
	if(rc = waveInOpen(&hwitmp, recorddev, &fmt, 0, 0, 0))
		return -1;
	rc = ::EnumInputLines(hwitmp, inputlines, ninputlines);
	waveInClose(hwitmp);
	return rc;
}

int AuFillCBWithInputLines(HWAVEIN hwi, HWND hWnd, int defcomponenttype)
{
	int connections;
	HMIXER hmx;
	MIXERLINE mxl;
	MIXERCAPS mxcaps;
	TCHAR s[300];

	if(mixerGetNumDevs() < 1)
		return -1;
	if(mixerOpen(&hmx, (UINT)hwi, 0, 0, MIXER_OBJECTF_HWAVEIN) != MMSYSERR_NOERROR)
		return -1;
	if(mixerGetDevCaps((UINT)hmx, &mxcaps, sizeof(mxcaps)) != MMSYSERR_NOERROR)
	{
		mixerClose(hmx);
		return -1;
	}
	mxl.cbStruct = sizeof(mxl);
	for(mxl.dwDestination = 0; mxl.dwDestination < mxcaps.cDestinations; mxl.dwDestination++)
	{
		if(mixerGetLineInfo((HMIXEROBJ)hmx, &mxl, MIXER_GETLINEINFOF_DESTINATION | MIXER_OBJECTF_HMIXER) != MMSYSERR_NOERROR)
		{
			mixerClose(hmx);
			return -1;
		}
		if(mxl.dwComponentType == MIXERLINE_COMPONENTTYPE_DST_WAVEIN)
			break;
	}
	connections = mxl.cConnections;
	*s = 0;
	SendMessage(hWnd, CB_RESETCONTENT, 0, 0);
	for(mxl.dwSource = 0; mxl.dwSource < (unsigned)connections; mxl.dwSource++)
	{
		mxl.cbStruct = sizeof(mxl);
		if(mixerGetLineInfo((HMIXEROBJ)hmx, &mxl, MIXER_GETLINEINFOF_SOURCE | MIXER_OBJECTF_HMIXER) != MMSYSERR_NOERROR)
		{
			mixerClose(hmx);
			return -1;
		}
		if(mxl.dwComponentType == (unsigned)defcomponenttype)
			_tcscpy(s, mxl.szName);
		SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)mxl.szName);
	}
	mixerClose(hmx);
	if(*s)
		SendMessage(hWnd, CB_SELECTSTRING, -1, (LPARAM)s);
	return 0;
}

int EnumInputLines(HWAVEIN hwi, TCHAR inputlines[][100], unsigned *ninputlines)
{
	int connections;
	HMIXER hmx;
	MIXERLINE mxl;
	MIXERCAPS mxcaps;
	TCHAR s[300];
	unsigned n = 0;

	if(mixerGetNumDevs() < 1)
		return -1;
	if(mixerOpen(&hmx, (UINT)hwi, 0, 0, MIXER_OBJECTF_HWAVEIN) != MMSYSERR_NOERROR)
		return -1;
	if(mixerGetDevCaps((UINT)hmx, &mxcaps, sizeof(mxcaps)) != MMSYSERR_NOERROR)
	{
		mixerClose(hmx);
		return -1;
	}
	mxl.cbStruct = sizeof(mxl);
	for(mxl.dwDestination = 0; mxl.dwDestination < mxcaps.cDestinations; mxl.dwDestination++)
	{
		if(mixerGetLineInfo((HMIXEROBJ)hmx, &mxl, MIXER_GETLINEINFOF_DESTINATION | MIXER_OBJECTF_HMIXER) != MMSYSERR_NOERROR)
		{
			mixerClose(hmx);
			return -1;
		}
		if(mxl.dwComponentType == MIXERLINE_COMPONENTTYPE_DST_WAVEIN)
			break;
	}
	connections = mxl.cConnections;
	*s = 0;
	for(mxl.dwSource = 0; mxl.dwSource < (unsigned)connections; mxl.dwSource++)
	{
		mxl.cbStruct = sizeof(mxl);
		if(mixerGetLineInfo((HMIXEROBJ)hmx, &mxl, MIXER_GETLINEINFOF_SOURCE | MIXER_OBJECTF_HMIXER) != MMSYSERR_NOERROR)
		{
			mixerClose(hmx);
			return -1;
		}
		if(n < *ninputlines)
			_tcscpy(inputlines[n++], mxl.szName);
	}
	mixerClose(hmx);
	*ninputlines = n;
	return 0;
}

int wavein_GetDeviceName(int n, TCHAR *devicename)
{
	if(n < (int)waveInGetNumDevs())
	{
		WAVEINCAPS wic;

		if(waveInGetDevCaps(n, &wic, sizeof(wic)) == MMSYSERR_NOERROR)
		{
			wipespaces(wic.szPname);
			_tcscpy(devicename, wic.szPname);
			return 0;
		}
	}
	return -1;
}

int waveout_GetDeviceName(int n, TCHAR *devicename)
{
	if(n < (int)waveOutGetNumDevs())
	{
		WAVEOUTCAPS woc;

		if(waveOutGetDevCaps(n, &woc, sizeof(woc)) == MMSYSERR_NOERROR)
		{
			wipespaces(woc.szPname);
			_tcscpy(devicename, woc.szPname);
			return 0;
		}
	}
	return -1;
}

int tAudioPlayer::WaitEnd()
{
	while(playing)
		Sleep(10);
	audioout.Stop();
	return 0;
}

int tAudioPlayer::Stop()
{
	stopplay = true;
	audioout.Stop();
	while(playing)
		Sleep(10);
	stopplay = false;
	return 0;
}

int tAudioPlayer::PlayBuffer(char *buffer, int bufferlen, int loop)
{
	Stop();

	if(!buffer || bufferlen < 160)
		return -1;
	f = 0;
	this->buffer = (char *)malloc(bufferlen);
	memcpy(this->buffer, buffer, bufferlen);
	this->bufferlen = bufferlen;
	this->loop = loop;
	playing = true;
	beginclassthread((ClassThreadStart)&tAudioPlayer::PlayAudioFileThread, this, 0);
	return 0;
}

int tAudioPlayer::PlayFile(TCHAR *file, int loop)
{
	Stop();  

	f = topenfile(file, O_RDONLY | O_BINARY);
	if(f == -1)
		return -1;
	this->loop = loop;
	buffer = 0;
	playing = true;
	beginclassthread((ClassThreadStart)&tAudioPlayer::PlayAudioFileThread, this, 0);
	return 0;
}

void tAudioPlayer::PlayAudioFileThread(void *param)
{
	int codec = -1, rc, looppos;
	char buf[160], *bufp;
	HACMSTREAM hacm = 0;
	WAVEFORMATEX wfo, *wfi;
	ACMSTREAMHEADER ash;

	pause = 0;
	if(f)
	{
		if(readfile(f, buf, 160) < 160)
			goto endplaythread;
	} else memcpy(buf, buffer, 160);
	if(!memcmp(buf, "RIFF", 4) && !memcmp(buf + 8, "WAVEfmt ", 8))
	{
		wfi = (WAVEFORMATEX *)(buf + 20);
		if(BuildDWord(buf + 16) == 16)
			wfi->cbSize = 0;
		memset(&wfo, 0, sizeof(wfo));
		wfo.wFormatTag = WAVE_FORMAT_PCM;
		wfo.cbSize = 0;
		wfo.nChannels = wfi->nChannels;
		wfo.wBitsPerSample = 16;
		wfo.nBlockAlign = 2 * wfi->nChannels;
		wfo.nSamplesPerSec = wfi->nSamplesPerSec;
		wfo.nAvgBytesPerSec = wfo.nSamplesPerSec * 2 * wfi->nChannels;
		if(!acmStreamOpen(&hacm, 0, wfi, &wfo, 0, 0, 0, ACM_STREAMOPENF_NONREALTIME))
		{
			if(wfi->nBlockAlign <= 4 && wfi->wFormatTag == WAVE_FORMAT_PCM)
				wfi->nBlockAlign = 800;
			memset(&ash, 0, sizeof(ACMSTREAMHEADER));
			ash.cbStruct = sizeof(ACMSTREAMHEADER);
			ash.cbSrcLength = wfi->nBlockAlign;
			ash.cbDstLength = (wfi->nBlockAlign * wfo.nAvgBytesPerSec + wfi->nAvgBytesPerSec - 1) / wfi->nAvgBytesPerSec / 2 * 2;
			ash.pbSrc = (BYTE *)malloc(ash.cbSrcLength);
			ash.pbDst = (BYTE *)malloc(ash.cbDstLength);
			acmStreamPrepareHeader(hacm, &ash, 0);
		} else goto endplaythread;
		if(f)
			seekfile(f, 20 + BuildDWord(buf + 16), SEEK_SET);
		else {
			bufp = buffer + 20 + BuildDWord(buf + 16);
			if(bufp > buffer + bufferlen)
				goto endplaythread;
		}
		for(;;)
		{
			if(f)
			{
				rc = readfile(f, buf, 8);
				if(rc < 8)
					goto endplaythread;
			} else {
				memcpy(buf, bufp, 8);
				bufp += 8;
			}
			if(!memcmp(buf, "data", 4))
			{
				progress = 0;
				filelen = (BYTE)buf[4] | ((BYTE)buf[5] << 8) | ((BYTE)buf[6] << 16) | ((BYTE)buf[7] << 24);
				if(f || bufp + filelen <= buffer + bufferlen)
				{
					if(f)
						looppos = seekfile(f, 0, SEEK_CUR);
					else looppos = bufp - buffer;

					while(!stopplay)
					{
						while(!stopplay && pause)
							Sleep(10);
						if(f)
						{
							if(readfile(f, ash.pbSrc, wfi->nBlockAlign) != wfi->nBlockAlign)
							{
								if(loop)
								{
									seekfile(f, looppos, SEEK_SET);
									if(readfile(f, ash.pbSrc, wfi->nBlockAlign) != wfi->nBlockAlign)
										break;
									progress = 0;
								} else break;
							}
						} else {
							if(bufp + wfi->nBlockAlign > buffer + bufferlen)
							{
								if(loop)
								{
									bufp = buffer + looppos;
									memcpy(ash.pbSrc, bufp, wfi->nBlockAlign);
									bufp += wfi->nBlockAlign;
									progress = 0;
								} else break;
							} else {
								memcpy(ash.pbSrc, bufp, wfi->nBlockAlign);
								bufp += wfi->nBlockAlign;
							}
						}
						progress += wfi->nBlockAlign;
						while(audioout.GetCurrentDelay() > (int)audioout.GetBufferLength() - 100)
							Sleep(10);
						acmStreamConvert(hacm, &ash, ACM_STREAMCONVERTF_BLOCKALIGN);
						rc = audioout.PutData(wfi->nChannels, wfi->nSamplesPerSec, (short *)ash.pbDst, ash.cbDstLength);
					}
					while(!stopplay && audioout.GetCurrentDelay() > 0)
						Sleep(1);
					filelen = 0;
				}
				goto endplaythread;
			} else if(f)
				seekfile(f, BuildDWord(buf + 4), SEEK_CUR);
			else {
				bufp += BuildDWord(buf + 4);
				if(bufp > buffer + bufferlen)
					goto endplaythread;
			}
		}
	}
endplaythread:
	if(hacm)
	{
		acmStreamConvert(hacm, &ash, ACM_STREAMCONVERTF_BLOCKALIGN | ACM_STREAMCONVERTF_END);
		acmStreamUnprepareHeader(hacm, &ash, 0);
		free(ash.pbSrc);
		free(ash.pbDst);
		acmStreamClose(hacm, 0);
	}
	if(f)
		closefile(f);
	if(buffer)
		free(buffer);
	audioout.Stop();
	playing = false;
}

int tAudioPlayer::Pause(int op)
{
	if(!playing)
		return -1;
	switch(op)
	{
	case 0:
		pause = false;
		break; 
	case 1:
		pause = true;
		break;
	case -1:
		pause = !pause;
		break;
	}
	return pause;
}

int tAudioPlayer::Progress()
{
	int a = filelen;

	if(!playing)
		return -1;
	if(a)
		return MulDiv(10000, progress, a);
	else return 0;
}

int AUDIO_EnumerateDevices(TCHAR devices[][100], int *ndevices, int recording)
{
	int i;

	if(recording)
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
	return 0;
}

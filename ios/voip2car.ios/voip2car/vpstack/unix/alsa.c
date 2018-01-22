#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <alsa/asoundlib.h>
#include "alsa.h"

enum {LOG_ERR, LOG_NORM, LOG_VERBOSE};
void lprintf(int type, const char *fmt, ...);

#ifdef HASMAIN
int verbosity = LOG_VERBOSE;

void lprintf(int type, const char *fmt, ...)
{
	va_list ap;

	if(type > verbosity)
		return;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);	
	va_end(ap);
}
#endif

static int snd_params(snd_pcm_t *handle, unsigned nbytes, unsigned sfreq, unsigned channels, int lowdelay)
{
	snd_pcm_hw_params_t *hw_params;
	snd_pcm_sw_params_t *sw_params;
	int err;

	snd_pcm_hw_params_malloc(&hw_params);
	snd_pcm_hw_params_any(handle, hw_params);
	snd_pcm_hw_params_set_access(handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(handle, hw_params, SND_PCM_FORMAT_S16_LE);
	snd_pcm_hw_params_set_rate_near(handle, hw_params, &sfreq, 0);
	snd_pcm_hw_params_set_channels(handle, hw_params, channels);
	snd_pcm_hw_params_set_period_size(handle, hw_params, nbytes / (2*channels), 0);
	snd_pcm_hw_params_set_buffer_time(handle, hw_params, lowdelay ? 100000 : 500000, 0);
	if((err = snd_pcm_hw_params(handle, hw_params)) < 0)
	{
		snd_pcm_hw_params_free (hw_params);
		lprintf (LOG_ERR, "Cannot set audio parameters: %s\n",	 snd_strerror (err));
		return -1;
	}
	snd_pcm_hw_params_free(hw_params);

	snd_pcm_sw_params_malloc(&sw_params);
	snd_pcm_sw_params_current (handle, sw_params);
	snd_pcm_sw_params_set_avail_min (handle, sw_params, nbytes / (2*channels));
	snd_pcm_sw_params_set_start_threshold (handle, sw_params, 0);
//	snd_pcm_sw_params_set_period_event(handle, sw_params, 1);
	if((err = snd_pcm_sw_params (handle, sw_params)) < 0)
	{
		snd_pcm_sw_params_free(sw_params);
		lprintf (LOG_ERR, "Cannot set software parameters: %s\n", snd_strerror (err));
		return -1;
	}
	snd_pcm_sw_params_free(sw_params);
	return 0;
}

static void calcvumeter(const short *buf, int nsamples, int nchannels, int *vumeter)
{
	int n, x, max, max2;
	
	if(nchannels == 1)
	{
		max = 0;
		for(n = 0; n < nsamples; n++)
		{
			x = buf[n] * buf[n];
			if(x > max)
				max = x;
		}
		vumeter[0] = vumeter[1] = (int)(10.0 * log10(max / 1073741824.0));
	} else {
		max = max2 = 0;
		for(n = 0; n < nsamples; n++)
		{
			x = buf[n] * buf[n];
			if(x > max)
				max = x;
			n++;
			x = buf[n] * buf[n];
			if(x > max2)
				max2 = x;
		}
		vumeter[0] = (int)(10.0 * log10(max / 1073741824.0));
		vumeter[1] = (int)(10.0 * log10(max2 / 1073741824.0));
	}
}

int alsa_init(ALSAAUDIO *audio)
{
	memset(audio, 0, sizeof *audio);
	audio->vumeter[1] = -100;
	audio->vumeter[1] = -100;
	return 0;
}

int alsa_open_play(ALSAAUDIO *audioout, unsigned sfreq, unsigned channels, unsigned nbytes, int lowdelay)
{
	int err;
	snd_pcm_t *handle;
	
	if(audioout->handle)
		return -1;
	if((err = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0)
	{
		lprintf(LOG_ERR, "Playback open error: %s\n", snd_strerror(err));
		return -1;
	}
	if(snd_params(handle, nbytes, sfreq, channels, lowdelay))
	{
		snd_pcm_close(handle);
		return -1;
	}
	audioout->vumeter[0] = audioout->vumeter[1] = -100;
	audioout->sfreq = sfreq;
	audioout->bufsize = nbytes;
	audioout->channels = channels;
	audioout->handle = handle;
	audioout->lowdelay = lowdelay;
	lprintf(LOG_VERBOSE, "Playback opened: sfreq=%u, channels=%u, bufsize=%u, lowdelay=%d\n", sfreq, channels, nbytes, lowdelay);
	return 0;
}

int alsa_close_play(ALSAAUDIO *audioout);

int alsa_put(ALSAAUDIO *audioout, void *buf, unsigned nbytes)
{
	int err;
	
	if(!audioout->handle)
		return -1;
	err = snd_pcm_writei(audioout->handle, buf, nbytes / (2*audioout->channels));
	if(err < 0)
	{
		if(err == -EPIPE)
		{
			lprintf(LOG_ERR, "Audio playback underrun\n");
			alsa_close_play(audioout);
			alsa_open_play(audioout, audioout->sfreq, audioout->channels, nbytes, audioout->lowdelay);
			snd_pcm_writei(audioout->handle, buf, nbytes / (2*audioout->channels));
		} else lprintf(LOG_VERBOSE, "Playback error: %s\n", snd_strerror(err));
	} else calcvumeter((short *)buf, audioout->bufsize / (2*audioout->channels), audioout->channels, audioout->vumeter);
	return err;
}

int alsa_play_delay(ALSAAUDIO *audioout)
{
	snd_pcm_sframes_t delay;
	int err;
	
	if(!audioout->handle)
		return -1;
	if(snd_pcm_state(audioout->handle) != SND_PCM_STATE_RUNNING)
		return -1;
	if((err = snd_pcm_delay(audioout->handle, &delay)) < 0)
	{
		lprintf(LOG_VERBOSE, "Error obtaining delay from audio device: %s\n", snd_strerror(err));
		return -1;
	}
	return delay * 2 * audioout->channels;
}

int alsa_close_play(ALSAAUDIO *audioout)
{
	if(!audioout->handle)
		return -1;
	snd_pcm_close(audioout->handle);
	audioout->handle = 0;
	audioout->vumeter[0] = audioout->vumeter[1] = -100;
	lprintf(LOG_VERBOSE, "Playback closed\n");
	return 0;
}

int alsa_open_record(ALSAAUDIO *audioin, unsigned sfreq, unsigned channels, unsigned nbytes, int lowdelay)
{
	int err;
	snd_pcm_t *handle;
	
	if(audioin->handle)
		return -1;
	if((err = snd_pcm_open(&handle, "default", SND_PCM_STREAM_CAPTURE, 0)) < 0)
	{
		lprintf(LOG_ERR, "Record open error: %s\n", snd_strerror(err));
		return -1;
	}
	if(snd_params(handle, nbytes, sfreq, channels, lowdelay))
	{
		snd_pcm_close(handle);
		return -1;
	}
	audioin->vumeter[0] = audioin->vumeter[1] = -100;
	audioin->sfreq = sfreq;
	audioin->bufsize = nbytes;
	audioin->channels = channels;
	audioin->handle = handle;
	memset(audioin->hpfin, 0, sizeof(audioin->hpfin));
	memset(audioin->hpfout, 0, sizeof(audioin->hpfout));
	lprintf(LOG_VERBOSE, "Recording opened: sfreq=%u, channels=%u, bufsize=%u, lowdelay=%d\n", sfreq, channels, nbytes, lowdelay);
	return 0;
}

unsigned getusecs();

static void HPFilterStereo(short *in, short *out, int len, int *inmem, double *outmem)
{
	// butter(2,0.0002,'high');
	static const double coefB[3] = {0.99955581038761, -1.99911162077522, 0.99955581038761};
	static const double coefA[2] = {-1.99911142347080, 0.99911181807964};
	int i, ch, yi;
	double y;

	for(ch = 0; ch < 2; ch++)
	{
		y = coefB[0] * in[ch] + coefB[1] * inmem[ch] + coefB[2] * inmem[2+ch] - coefA[0] * outmem[ch] - coefA[1] * outmem[2+ch];
		yi = (int)y;
		out[ch] = yi > 32767 ? 32767 : yi < -32678 ? -32768 : yi;
		outmem[2+ch] = outmem[ch];
		outmem[ch] = y;
		y = coefB[0] * in[2+ch] + coefB[1] * in[ch] + coefB[2] * inmem[ch] - coefA[0] * outmem[ch] - coefA[1] * outmem[2+ch];
		yi = (int)y;
		out[2+ch] = yi > 32767 ? 32767 : yi < -32678 ? -32768 : yi;
		outmem[2+ch] = outmem[ch];
		outmem[ch] = y;
		for(i = 2; i < len; i++)
		{
			y = coefB[0] * in[2*i+ch] + coefB[1] * in[2*(i - 1)+ch] + coefB[2] * in[2*(i - 2)+ch] - coefA[0] * outmem[ch] - coefA[1] * outmem[2+ch];
			yi = (int)y;
			out[2*i+ch] = yi > 32767 ? 32767 : yi < -32678 ? -32768 : yi;
			outmem[2+ch] = outmem[ch];
			outmem[ch] = y;
		}
		inmem[2+ch] = in[2*(len - 2)+ch];
		inmem[ch] = in[2*(len - 1)+ch];
	}
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


int alsa_get(ALSAAUDIO *audioin, void *buf)
{
	int err;
	short frame[1152*2];
	//snd_pcm_sframes_t delay;
	
	if(!audioin->handle)
		return -1;
	err = snd_pcm_readi(audioin->handle, frame, audioin->bufsize / (2*audioin->channels));
	if(err < 0)
	{
		if(err == -EPIPE)
		{
			snd_pcm_prepare(audioin->handle);
			lprintf(LOG_ERR, "Audio recording overrun\n");
		} else {
			lprintf(LOG_ERR, "Recording error: %s\n", snd_strerror(err));
			usleep(100000);
		}
	}
	if(audioin->channels == 2)
		HPFilterStereo(frame, buf, audioin->bufsize / 4, audioin->hpfin, audioin->hpfout);
	else HPFilter(frame, buf, audioin->bufsize / 2, audioin->hpfin, audioin->hpfout);
	calcvumeter((short *)buf, audioin->bufsize / (2*audioin->channels), audioin->channels, audioin->vumeter);
	
/*	if((err = snd_pcm_delay(audioin->handle, &delay)) < 0)
	{
		lprintf(LOG_VERBOSE, "Error obtaining delay from audio device: %s\n", snd_strerror(err));
		return -1;
	}
	lprintf(LOG_VERBOSE, "Delay=%f\n", (double)delay/audioin->sfreq);*/
	return err;
}

int alsa_close_record(ALSAAUDIO *audioin)
{
	if(!audioin->handle)
		return -1;
	snd_pcm_close(audioin->handle);
	audioin->handle = 0;
	audioin->vumeter[0] = audioin->vumeter[1] = -100;
	lprintf(LOG_VERBOSE, "Recording closed\n");
	return 0;
}

#ifdef HASMAIN
#define NSAMPLES 32
int main()
{
	short buf[NSAMPLES];
	int i;
	ALSAAUDIO in, out;
	
	alsa_init(&in);
	alsa_init(&out);
	alsa_open_record(&in, 44100, 2, NSAMPLES*2);
	alsa_open_play(&out, 44100, 2, NSAMPLES*2);
	alsa_put(&out, buf, NSAMPLES*2);
	alsa_put(&out, buf, NSAMPLES*2);
	alsa_put(&out, buf, NSAMPLES*2);
	for(i = 0; i < 10000; i++)
	{
		alsa_get(&in, buf);
		alsa_put(&out, buf, NSAMPLES*2);
		alsa_play_delay();
	}
	alsa_close_record(&in);
	alsa_close_play(&out);
	return 0;
}
#endif

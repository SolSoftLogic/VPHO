#ifndef _ALSA_H_INCLUDED_
#define _ALSA_H_INCLUDED_

struct snd_pcm_t;

typedef struct ALSAAUDIO
{
	snd_pcm_t *handle;
	int sfreq, channels, bufsize, lowdelay;
	int vumeter[2];
	int hpfin[4];
	double hpfout[4];
} ALSAAUDIO;

#ifdef __cplusplus
extern "C" {
#endif

int alsa_init(ALSAAUDIO *obj);
int alsa_open_play(ALSAAUDIO *obj, unsigned sfreq, unsigned channels, unsigned nbytes, int lowdelay);
int alsa_put(ALSAAUDIO *obj, void *buf, unsigned nbytes);
int alsa_close_play(ALSAAUDIO *obj);
int alsa_play_delay(ALSAAUDIO *obj);
int alsa_open_record(ALSAAUDIO *obj, unsigned sfreq, unsigned channels, unsigned nbytes, int lowdelay);
int alsa_get(ALSAAUDIO *obj, void *buf);
int alsa_close_record(ALSAAUDIO *obj);
int alsa_getvumeters(ALSAAUDIO *obj, int *vumeters);

#ifdef __cplusplus
}
#endif

#endif

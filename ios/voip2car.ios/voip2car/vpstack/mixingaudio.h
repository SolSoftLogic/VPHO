#ifndef _SIMPLEAUDIO_H_INCLUDED_
#define _SIMPLEAUDIO_H_INCLUDED_

class AUDIOOUT;
class AUDIOIN;
class AUDIOENCODER;
class AUDIODECODER;
class RESAMPLER;
class AUDIOBUFFER;

#define MAXMIXINGAUDIO 10

class VPMIXINGAUDIO : public VPAUDIODATA
{
public:
	VPMIXINGAUDIO(IVPSTACK *vps, VPCALL vpcall, int codec);
	~VPMIXINGAUDIO();
	void AudioFrame(int codec, unsigned timestamp, const unsigned char *buf, int buflen);
	int Save(const char *path);

	friend void VPMIXINGAUDIO_AudioAcquisitionThread(void *dummy);
protected:
	IVPSTACK *vps;
	VPCALL vpcall;
	AUDIODECODER *decoder;
	AUDIOENCODER *encoder;
	RESAMPLER *decresampler, *outresampler;
	AUDIOBUFFER *netsource;
	int peak, framesperpacket, codec;
	unsigned cursample;
	void *hwav;
	char savepath[MAX_PATH];
	bool save;
};

#endif

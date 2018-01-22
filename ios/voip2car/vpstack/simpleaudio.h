#ifndef _SIMPLEAUDIO_H_INCLUDED_
#define _SIMPLEAUDIO_H_INCLUDED_

class AUDIOOUT;
class AUDIOIN;
class AUDIOENCODER;
class AUDIODECODER;

#define MAXSIMPLEAUDIO 10

class VPSIMPLEAUDIO : public VPAUDIODATA
{
public:
	VPSIMPLEAUDIO(IVPSTACK *vps, VPCALL vpcall, int codec);
	~VPSIMPLEAUDIO();
	void AudioFrame(int codec, unsigned timestamp, const unsigned char *buf, int buflen);
	friend void VPSIMPLEAUDIO_AudioAcquisitionThread(void *dummy);
	friend int VPSIMPLEAUDIO_SetLevel(int recording, int level);

protected:
	AUDIOOUT *out;
	AUDIODECODER *decoder;
	int peak;
};

#endif

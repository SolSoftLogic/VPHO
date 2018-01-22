#ifndef _AUDIOCODECS_H_INCLUDED
#define _AUDIOCODECS_H_INCLUDED

#define DEFINEDCODECS 16

#ifndef CODEC_GSM
#define CODEC_GSM 0
#define CODEC_MSGSM 0
#define CODEC_TRUESPEECH 1
#define CODEC_G723 2
#define CODEC_SPEEXA 3
#define CODEC_SPEEXB 4
#define CODEC_SPEEXC 5
#define CODEC_SPEEXD 6
#define CODEC_SPEEXE 7
#define CODEC_GSMFR 8
#define CODEC_G729 9
#define CODEC_G711A 0xa
#define CODEC_G711U 0xb
#define CODEC_ILBC30 0xc
#define CODEC_ILBC20 0xd
#define CODEC_LINEARBE 0xe
#define CODEC_LINEARLE 0xf
#define CODEC_LAST CODEC_G711U
#endif

extern int codecuframelen[DEFINEDCODECS];
extern int codeccframelen[DEFINEDCODECS];
extern unsigned codecsfreq[DEFINEDCODECS];

class AUDIOENCODER {
public:
	AUDIOENCODER() { hCodec = 0; }
	~AUDIOENCODER() { Close(); }
	int Encode(int codec, short *in, unsigned inbytes, unsigned char *out);

protected:
	int Open(int codec);
	void Close();

	void *hCodec;
	int codec;
};

class AUDIODECODER {
public:
	AUDIODECODER() { hCodec = 0; }
	~AUDIODECODER() { Close(); }
	int Decode(int codec, const unsigned char *in, unsigned inbytes, short *out);

protected:
	int Open(int codec);
	void Close();

	void *hCodec;
	int codec;
};

#endif

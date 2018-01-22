extern "C" {
#include "../gsmlib/gsm.h"
};

#include <string.h>
#include "audiocodecs.h"

// Uncompressed frame lengths in bytes
int codecuframelen[DEFINEDCODECS] = {640, 480, 480, 320, 640, 2560, 1280, 2560, 320, 160, 320, 320, 480, 320, 320, 320};
// Compressed frames lengths in bytes
int codeccframelen[DEFINEDCODECS] = {65, 32, 24, 20, 32, 93, 90, 119, 33, 10, 160, 160, 50, 38, 320, 320};
// Sampling frequencies
unsigned codecsfreq[DEFINEDCODECS] = {8000, 8000, 8000, 8000, 16000, 32000, 32000, 8000, 8000, 8000, 8000, 8000, 8000, 8000, 8000, 8000};
// Codec names
static const char *codecnames[DEFINEDCODECS] = {"MS-GSM 6.10 (13kbps)", "Truespeech (8.5kbps)", "MS G.723.1 (6.4kbps)",
"Speex (8kbps)", "Speex (12.8kbps, 16kHz)", "Speex (18.2kbps, 32kHz)", "Speex (36kbps, 32kHz)", "Speex (5.9kbps)", "GSM 6.10 (13.2kbps)",
"G.729 (8kbps)", "G.711 A-law (64kbps)", "G.711 mu-law (64kbps)", "iLBC30", "iLBC20", "Linear BE", "Linear LE"};

void gsm_fromms(unsigned char *in, unsigned char *out);
void gsm_toms(unsigned char *in, unsigned char *out);

extern "C" {
unsigned char linear2mulaw(int pcm_val);
int mulaw2linear(unsigned char u_val);
unsigned char linear2alaw(int pcm_val);
int alaw2linear(unsigned char a_val);
};

#ifdef INCLUDESPEEX
void *speex_create(int decoder, int type);	// Type from 0 to 4
void speex_destroy(void *hcodec);
int speex_encode(void *hcodec, const short *frame, unsigned char *compressed);
int speex_decode(void *hcodec, const unsigned char *compressed, short *frame);
#endif

#ifdef INCLUDEILBC
#include "../ilbc/ilbclib.h"
#endif

const char *GetCodecName(int codec)
{
	if(codec >= 0 && codec < DEFINEDCODECS)
		return codecnames[codec];
	return 0;
}

int AUDIOENCODER::Open(int codec)
{
	this->codec = codec;
	switch(codec)
	{
	case CODEC_MSGSM:
	{
		int opt = 1;
		
		hCodec = gsm_create();
		gsm_option((gsm)hCodec, GSM_OPT_WAV49, &opt);
	}
		break;
	case CODEC_GSMFR:
		hCodec = gsm_create();
		break;
	case CODEC_G711A:
	case CODEC_G711U:
		break;
#ifdef INCLUDESPEEX
	case CODEC_SPEEXA:
	case CODEC_SPEEXB:
	case CODEC_SPEEXC:
	case CODEC_SPEEXD:
	case CODEC_SPEEXE:
		hCodec = speex_create(0, codec - CODEC_SPEEXA);
		break;
#endif
#ifdef INCLUDEILBC
	case CODEC_ILBC20:
		hCodec = ilbc_createencoder(0);
		break;
	case CODEC_ILBC30:
		hCodec = ilbc_createencoder(1);
		break;
#endif
	case CODEC_LINEARBE:
	case CODEC_LINEARLE:
		break;
	default:
		return -1;
	}
	return 0;
}

void AUDIOENCODER::Close()
{
	if(!hCodec)
		return;
	switch(codec)
	{
	case CODEC_MSGSM:
	case CODEC_GSMFR:
		gsm_destroy((gsm)hCodec);
		break;
	case CODEC_G711A:
	case CODEC_G711U:
		break;
#ifdef INCLUDESPEEX
	case CODEC_SPEEXA:
	case CODEC_SPEEXB:
	case CODEC_SPEEXC:
	case CODEC_SPEEXD:
	case CODEC_SPEEXE:
		speex_destroy(hCodec);
		break;
#endif
#ifdef INCLUDEILBC
	case CODEC_ILBC20:
	case CODEC_ILBC30:
		ilbc_close(hCodec);
		break;
#endif
	case CODEC_LINEARBE:
	case CODEC_LINEARLE:
		break;
	}
}

int AUDIOENCODER::Encode(int codec, short *in, unsigned inbytes, unsigned char *out)
{
	unsigned i;

	if(!hCodec || this->codec != codec)
	{
		Close();
		i = Open(codec);
		if(i)
			return -1;
	}
	switch(codec)
	{
	case CODEC_MSGSM:
		for(i = 0; i < inbytes; i += 640)
		{
			gsm_encode((gsm)hCodec, in, out);
			gsm_encode((gsm)hCodec, in + 160, out + 32);
			in += 320;
			out += 65;
		}
		return inbytes / 640 * 65;
	case CODEC_GSMFR:
		for(i = 0; i < inbytes; i += 320)
		{
			gsm_encode((gsm)hCodec, in, out);
			in += 160;
			out += 33;
		}
		return inbytes / 320 * 33;
	case CODEC_G711A:
		inbytes /= 2;
		for(i = 0; i < inbytes; i++)
			out[i] = linear2alaw(in[i]);
		return inbytes;
	case CODEC_G711U:
		inbytes /= 2;
		for(i = 0; i < inbytes; i++)
			out[i] = linear2mulaw(in[i]);

		return inbytes;
#ifdef INCLUDESPEEX
	case CODEC_SPEEXA:
	case CODEC_SPEEXB:
	case CODEC_SPEEXC:
	case CODEC_SPEEXD:
	case CODEC_SPEEXE:
		for(i = 0; i < inbytes; i += codecuframelen[codec])
		{
			speex_encode(hCodec, in, out);
			in += codecuframelen[codec] / 2;
			out += codeccframelen[codec];
		}
		return inbytes / codecuframelen[codec] * codeccframelen[codec];
#endif
#ifdef INCLUDEILBC
	case CODEC_ILBC20:
	case CODEC_ILBC30:
		for(i = 0; i < inbytes; i += codecuframelen[codec])
		{
			ilbc_encode(hCodec, in, out);
			in += codecuframelen[codec] / 2;
			out += codeccframelen[codec];
		}
		return inbytes / codecuframelen[codec] * codeccframelen[codec];
#endif
	case CODEC_LINEARBE:
		for(i = 0; i < inbytes/2; i++)
			((short *)out)[i] = ((unsigned short)in[i] << 8) | ((unsigned short)in[i] >> 8);
		return inbytes;
	case CODEC_LINEARLE:
		memcpy(out, in, inbytes);
		return inbytes;
	default:
		return 0;
	}
}

int AUDIODECODER::Open(int codec)
{
	this->codec = codec;
	switch(codec)
	{
	case CODEC_MSGSM:
	{
		int opt = 1;
		
		hCodec = gsm_create();
		gsm_option((gsm)hCodec, GSM_OPT_WAV49, &opt);
	}
		break;
	case CODEC_GSMFR:
		hCodec = gsm_create();
		break;
	case CODEC_G711A:
	case CODEC_G711U:
		break;
#ifdef INCLUDESPEEX
	case CODEC_SPEEXA:
	case CODEC_SPEEXB:
	case CODEC_SPEEXC:
	case CODEC_SPEEXD:
	case CODEC_SPEEXE:
		hCodec = speex_create(1, codec - CODEC_SPEEXA);
		break;
#endif
#ifdef INCLUDEILBC
	case CODEC_ILBC20:
		hCodec = ilbc_createdecoder(0);
		break;
	case CODEC_ILBC30:
		hCodec = ilbc_createdecoder(1);
		break;
#endif
	case CODEC_LINEARBE:
	case CODEC_LINEARLE:
		break;
	default:
		return -1;
	}
	return 0;
}

void AUDIODECODER::Close()
{
	if(!hCodec)
		return;
	switch(codec)
	{
	case CODEC_MSGSM:
	case CODEC_GSMFR:
		gsm_destroy((gsm)hCodec);
		break;
	case CODEC_G711A:
	case CODEC_G711U:
		break;
#ifdef INCLUDESPEEX
	case CODEC_SPEEXA:
	case CODEC_SPEEXB:
	case CODEC_SPEEXC:
	case CODEC_SPEEXD:
	case CODEC_SPEEXE:
		speex_destroy(hCodec);
		break;
#endif
#ifdef INCLUDEILBC
	case CODEC_ILBC20:
	case CODEC_ILBC30:
		ilbc_close(hCodec);
		break;
#endif
	case CODEC_LINEARBE:
	case CODEC_LINEARLE:
		break;
	}
}

int AUDIODECODER::Decode(int codec, const unsigned char *in, unsigned inbytes, short *out)
{
	unsigned i;

	if(!hCodec || this->codec != codec)
	{
		Close();
		i = Open(codec);
		if(i)
			return -1;
	}
	switch(codec)
	{
	case CODEC_MSGSM:
		for(i = 0; i < inbytes; i += 65)
		{
			gsm_decode((gsm)hCodec, (unsigned char *)in, out);
			gsm_decode((gsm)hCodec, (unsigned char *)in + 33, out + 160);
			in += 65;
			out += 320;
		}
		return inbytes / 65 * 640;
	case CODEC_GSMFR:
		for(i = 0; i < inbytes; i += 33)
		{
			gsm_decode((gsm)hCodec, (unsigned char *)in, out);
			in += 33;
			out += 160;
		}
		return inbytes / 33 * 320;
	case CODEC_G711A:
		for(i = 0; i < inbytes; i++)
			out[i] = alaw2linear(in[i]);
		return inbytes * 2;
	case CODEC_G711U:
		for(i = 0; i < inbytes; i++)
			out[i] = mulaw2linear(in[i]);
		return inbytes * 2;
#ifdef INCLUDESPEEX
	case CODEC_SPEEXA:
	case CODEC_SPEEXB:
	case CODEC_SPEEXC:
	case CODEC_SPEEXD:
	case CODEC_SPEEXE:
		for(i = 0; i < inbytes; i += codeccframelen[codec])
		{
			speex_decode(hCodec, in, out);
			in += codeccframelen[codec];
			out += codecuframelen[codec] / 2;
		}
		return inbytes / codeccframelen[codec] * codecuframelen[codec];
#endif
#ifdef INCLUDEILBC
	case CODEC_ILBC20:
	case CODEC_ILBC30:
		for(i = 0; i < inbytes; i += codeccframelen[codec])
		{
			ilbc_decode(hCodec, in, out);
			in += codeccframelen[codec];
			out += codecuframelen[codec] / 2;
		}
		return inbytes / codeccframelen[codec] * codecuframelen[codec];
#endif
	case CODEC_LINEARBE:
		for(i = 0; i < inbytes/2; i++)
			out[i] = (((unsigned short *)in)[i] << 8) | (((unsigned short *)in)[i] >> 8);
		return inbytes;
	case CODEC_LINEARLE:
		memcpy(out, in, inbytes);
		return inbytes;
	default:
		return 0;
	}
}


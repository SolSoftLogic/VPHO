#include <windows.h>
#include <mmreg.h>
#include <msacm.h>
#include <math.h>
#include <stdio.h>
#include "winaudio.h"
#include "../gsm.h"
#include "../vprotocol.h"

#define INCLUDESPEEX

#ifdef INCLUDESPEEX
void *speex_create(int decoder, int type);	// Type from 0 to 4
void speex_destroy(void *hcodec);
int speex_encode(void *hcodec, const short *frame, unsigned char *compressed);
int speex_decode(void *hcodec, const unsigned char *compressed, short *frame);
#endif

struct ACM_WFC acm_wfc[DEFINEDCODECS] =
	{{{0x31, 1, 8000, 1625, 65, 0, 2}, {0x40, 1}},
	{{0x22, 1, 8000, 1067, 32, 1, 32}, {1, 0, 240}},
	{{0x42, 1, 8000, 800, 24, 0, 10}, {2, 0, 0xce, 0x9a, 0x32, 0xf7, 0xa2, 0xae, 0xde, 0xac}},

{{0xa109, 1, 8000, 1000, 20, 4, 114}, {
0x01, 0x00, 0x53, 0x70, 0x65, 0x65, 0x78, 0x20, 0x20, 0x20, 0x73, 0x70, 0x65, 0x65, 0x78, 0x2d,
0x31, 0x2e, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
0x00, 0x00, 0x50, 0x00, 0x00, 0x00, 0x40, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00,
0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x09, 0x43, 0x6f, 0x64, 0x65, 0x63, 0x20, 0x68, 0x6f, 0x6d, 0x65, 0x70, 0x61, 0x67,
0x65, 0x20, 0x77, 0x77, 0x77, 0x2e, 0x6f, 0x70, 0x65, 0x6e, 0x61, 0x63, 0x6d, 0x2e, 0x6f, 0x72,
0x67, 0x20}},

{{0xa109, 1, 16000, 1600, 32, 4, 114}, {
0x01, 0x00, 0x53, 0x70, 0x65, 0x65, 0x78, 0x20, 0x20, 0x20, 0x73, 0x70, 0x65, 0x65, 0x78, 0x2d,
0x31, 0x2e, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
0x00, 0x00, 0x50, 0x00, 0x00, 0x00, 0x80, 0x3e, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00,
0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x40, 0x01, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x09, 0x43, 0x6f, 0x64, 0x65, 0x63, 0x20, 0x68, 0x6f, 0x6d, 0x65, 0x70, 0x61, 0x67,
0x65, 0x20, 0x77, 0x77, 0x77, 0x2e, 0x6f, 0x70, 0x65, 0x6e, 0x61, 0x63, 0x6d, 0x2e, 0x6f, 0x72,
0x67, 0x20}},

{{0xa109, 1, 32000, 2325, 93, 5, 114}, {
0x01, 0x00, 0x53, 0x70, 0x65, 0x65, 0x78, 0x20, 0x20, 0x20, 0x73, 0x70, 0x65, 0x65, 0x78, 0x2d,
0x31, 0x2e, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
0x00, 0x00, 0x50, 0x00, 0x00, 0x00, 0x00, 0x7d, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x04, 0x00,
0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x80, 0x02, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x09, 0x43, 0x6f, 0x64, 0x65, 0x63, 0x20, 0x68, 0x6f, 0x6d, 0x65, 0x70, 0x61, 0x67,
0x65, 0x20, 0x77, 0x77, 0x77, 0x2e, 0x6f, 0x70, 0x65, 0x6e, 0x61, 0x63, 0x6d, 0x2e, 0x6f, 0x72,
0x67, 0x20}},

{{0xa109, 1, 32000, 4500, 90, 9, 114}, {
0x01, 0x00, 0x53, 0x70, 0x65, 0x65, 0x78, 0x20, 0x20, 0x20, 0x73, 0x70, 0x65, 0x65, 0x78, 0x2d,
0x31, 0x2e, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
0x00, 0x00, 0x50, 0x00, 0x00, 0x00, 0x00, 0x7d, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x04, 0x00,
0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x80, 0x02, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x09, 0x43, 0x6f, 0x64, 0x65, 0x63, 0x20, 0x68, 0x6f, 0x6d, 0x65, 0x70, 0x61, 0x67,
0x65, 0x20, 0x77, 0x77, 0x77, 0x2e, 0x6f, 0x70, 0x65, 0x6e, 0x61, 0x63, 0x6d, 0x2e, 0x6f, 0x72,
0x67, 0x20}},

{{0xa109, 1, 8000, 744, 119, 2, 114}, {
0x01, 0x00, 0x53, 0x70, 0x65, 0x65, 0x78, 0x20, 0x20, 0x20, 0x73, 0x70, 0x65, 0x65, 0x78, 0x2d,
0x31, 0x2e, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
0x00, 0x00, 0x50, 0x00, 0x00, 0x00, 0x40, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00,
0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x09, 0x43, 0x6f, 0x64, 0x65, 0x63, 0x20, 0x68, 0x6f, 0x6d, 0x65, 0x70, 0x61, 0x67,
0x65, 0x20, 0x77, 0x77, 0x77, 0x2e, 0x6f, 0x70, 0x65, 0x6e, 0x61, 0x63, 0x6d, 0x2e, 0x6f, 0x72,
0x67, 0x20}},
{{0x0, 1, 8000, 1650, 33, 0, 0}},
{{0x0, 1, 8000, 1625, 10, 0, 0}},
{{0x6, 1, 8000, 8000, 160, 8, 0}},
{{0x7, 1, 8000, 8000, 160, 8, 0}}
};

static WAVEFORMATEX wfu[DEFINEDCODECS] = {
{1, 1, 8000, 16000, 2, 16, 0},
{1, 1, 8000, 16000, 2, 16, 0},
{1, 1, 8000, 16000, 2, 16, 0},
{1, 1, 8000, 16000, 2, 16, 0},
{1, 1, 16000, 32000, 2, 16, 0},
{1, 1, 32000, 64000, 2, 16, 0},
{1, 1, 32000, 64000, 2, 16, 0},
{1, 1, 8000, 16000, 2, 16, 0},
{1, 1, 8000, 16000, 2, 16, 0},
{1, 1, 8000, 16000, 2, 16, 0},
{1, 1, 8000, 16000, 2, 16, 0},
{1, 1, 8000, 16000, 2, 16, 0}};

int codecuframelen[DEFINEDCODECS] = {640, 480, 480, 320, 640, 2560, 1280, 2560, 320, 160, 320, 320};
int codeccframelen[DEFINEDCODECS] = {65, 32, 24, 20, 32, 93, 90, 119, 33, 10, 160, 160};
unsigned codecsfreq[DEFINEDCODECS] = {8000, 8000, 8000, 8000, 16000, 32000, 32000, 8000, 8000, 8000, 8000, 8000};
static const char *codecnames[DEFINEDCODECS] = {"MS-GSM 6.10 (13kbps)", "Truespeech (8.5kbps)", "MS G.723.1 (6.4kbps)",
"Speex (8kbps)", "Speex (12.8kbps)", "Speex (18.2kbps)", "Speex (36kbps)", "Speex (5.9kbps)", "GSM 6.10 (13.2kbps)",
"G.729 (8kbps)", "G.711 A-law (64kbps)", "G.711 mu-law (64kbps)"};

void gsm_fromms(unsigned char *in, unsigned char *out);
void gsm_toms(unsigned char *in, unsigned char *out);

extern "C" {
unsigned char linear2mulaw(int pcm_val);
int mulaw2linear(unsigned char u_val);
unsigned char linear2alaw(int pcm_val);
int alaw2linear(unsigned char a_val);
};

const char *GetCodecName(int codec)
{
	if(codec >= 0 && codec < DEFINEDCODECS)
		return codecnames[codec];
	return 0;
}

BOOL CALLBACK acmFormatEnumCallback(HACMDRIVERID hadid, LPACMFORMATDETAILS pafd, DWORD dwInstance, DWORD fdwSupport)
{
	int i;

	if(pafd->pwfx->wFormatTag == 41225 && pafd->pwfx->nChannels == 1 &&
		(pafd->pwfx->nAvgBytesPerSec == 1000 && pafd->pwfx->wBitsPerSample == 4 ||
		pafd->pwfx->nAvgBytesPerSec == 1600 ||
		pafd->pwfx->nAvgBytesPerSec == 2325 ||
		pafd->pwfx->nAvgBytesPerSec == 4500 || pafd->pwfx->nSamplesPerSec == 8000))
	{
		printf("{{0xa109, 1, %d, %d, %d, %d, %d}, {\n", pafd->pwfx->nSamplesPerSec, pafd->pwfx->nAvgBytesPerSec, pafd->pwfx->nBlockAlign,
			pafd->pwfx->wBitsPerSample, pafd->pwfx->cbSize);
		for(i = 0; i < pafd->pwfx->cbSize; i++)
		{
			printf("0x%02x", acm_wfc[0].extra[i]);
			if(i == pafd->pwfx->cbSize - 1)
				printf("}}\n\n");
			else if(i % 16 == 15)
				printf(",\n");
			else printf(", ");
		}
	}
	return TRUE;
}

void FindCodecs()
{
	ACMFORMATDETAILS afd;
	int rc;

	memset(&afd, 0, sizeof(afd));
	afd.cbStruct = sizeof(afd);
	afd.pwfx = &acm_wfc[0].wf;
	afd.cbwfx = sizeof(acm_wfc[0]);
	rc = acmFormatEnum(0, &afd, acmFormatEnumCallback, 0, 0);
}

HACMSTREAM ACMEncoderOpen(int codec, ACMSTREAMHEADER *ash)
{
	HACMSTREAM hacm;

//	int rc;
//	ACMFORMATCHOOSE afc = { sizeof(ACMFORMATCHOOSE), 0, 0, &acm_wfc[0].wf, sizeof(acm_wfc[0]), "Select format", 0,0,0,0,0,0,0,0,0,0};
//	rc = acmFormatChoose(&afc);
//	if(!codec)
//		FindCodecs();
	if(codec < CODEC_GSM || codec > CODEC_LAST)
		return 0;
	memset(ash, 0, sizeof(ACMSTREAMHEADER));
	ash->cbStruct = sizeof(ACMSTREAMHEADER);
	ash->dwUser = codec;
	ash->cbSrcLength = codecuframelen[codec];
	ash->cbDstLength = codeccframelen[codec];
	if(codec == CODEC_GSMFR)
		return (HACMSTREAM)gsm_create();
	if(codec == CODEC_GSM)
	{
		int opt = 1;
		hacm = (HACMSTREAM)gsm_create();
		gsm_option((gsm)hacm, GSM_OPT_WAV49, &opt);
		return hacm;
	}
	if(codec == CODEC_G711A || codec == CODEC_G711U)
		return (HACMSTREAM)1;
#ifdef INCLUDESPEEX
	if(codec >= CODEC_SPEEXA && codec <= CODEC_SPEEXE)
		return (HACMSTREAM)speex_create(0, codec - CODEC_SPEEXA);
#endif
	if(!acmStreamOpen(&hacm, 0, &wfu[codec], &acm_wfc[codec].wf, 0, 0, 0, ACM_STREAMOPENF_NONREALTIME))
	{
		ash->pbSrc = (BYTE *)malloc(codecuframelen[codec]);
		ash->pbDst = (BYTE *)malloc(codeccframelen[codec] + 1);
		acmStreamPrepareHeader(hacm, ash, 0);
		return hacm;
	} else return 0;
}

int ACMEncode(HACMSTREAM hacm, ACMSTREAMHEADER *ash, short *frame, unsigned char *compressed, int codec)
{
	int i;

	if(codec == CODEC_GSM)
	{
		gsm_encode((gsm)hacm, frame, compressed);
		gsm_encode((gsm)hacm, frame + 160, compressed + 32);
	} else if(codec == CODEC_GSMFR)
		gsm_encode((gsm)hacm, frame, compressed);
	else if(codec == CODEC_G711A)
	{
		for(i = 0; i < 160; i++)
			compressed[i] = linear2alaw(frame[i]);
	} else if(codec == CODEC_G711U)
	{
		for(i = 0; i < 160; i++)
			compressed[i] = linear2mulaw(frame[i]);
	}
#ifdef INCLUDESPEEX
	else if(codec >= CODEC_SPEEXA && codec <= CODEC_SPEEXE)
		speex_encode(hacm, frame, compressed);
#endif
	else {
		memcpy(ash->pbSrc, frame, codecuframelen[codec]);
		acmStreamConvert(hacm, ash, ACM_STREAMCONVERTF_BLOCKALIGN);
		memcpy(compressed, ash->pbDst, codeccframelen[codec]);
	}
	return codeccframelen[codec];
}

int ACMEncoderClose(HACMSTREAM hacm, ACMSTREAMHEADER *ash)
{
	if(ash->dwUser == CODEC_GSM || ash->dwUser == CODEC_GSMFR)
		gsm_destroy((gsm)hacm);
	else if(ash->dwUser == CODEC_G711A || ash->dwUser == CODEC_G711U)
		;
#ifdef INCLUDESPEEX
	else if(ash->dwUser >= CODEC_SPEEXA && ash->dwUser <= CODEC_SPEEXE)
		speex_destroy(hacm);
#endif
	else {
		acmStreamConvert(hacm, ash, ACM_STREAMCONVERTF_BLOCKALIGN | ACM_STREAMCONVERTF_END);
		acmStreamUnprepareHeader(hacm, ash, 0);
		acmStreamClose(hacm, 0);
		free(ash->pbSrc);
		free(ash->pbDst);
	}
	return 0;
}

HACMSTREAM ACMDecoderOpen(int codec, ACMSTREAMHEADER *ash)
{
	HACMSTREAM hacm;

	if(codec < CODEC_GSM || codec > CODEC_LAST)
		return 0;
	memset(ash, 0, sizeof(ACMSTREAMHEADER));
	ash->cbStruct = sizeof(ACMSTREAMHEADER);
	ash->dwUser = codec;
	ash->cbSrcLength = codecuframelen[codec];
	ash->cbDstLength = codeccframelen[codec];
	if(codec == CODEC_GSMFR)
		return (HACMSTREAM)gsm_create();
	if(codec == CODEC_GSM)
	{
		int opt = 1;
		hacm = (HACMSTREAM)gsm_create();
		gsm_option((gsm)hacm, GSM_OPT_WAV49, &opt);
		return hacm;
	}
	if(codec == CODEC_G711A || codec == CODEC_G711U)
		return (HACMSTREAM)1;
#ifdef INCLUDESPEEX
	if(codec >= CODEC_SPEEXA && codec <= CODEC_SPEEXE)
		return (HACMSTREAM)speex_create(1, codec - CODEC_SPEEXA);
#endif
	if(!acmStreamOpen(&hacm, 0, &acm_wfc[codec].wf, &wfu[codec], 0, 0, 0, ACM_STREAMOPENF_NONREALTIME))
	{
		memset(ash, 0, sizeof(ACMSTREAMHEADER));
		ash->cbStruct = sizeof(ACMSTREAMHEADER);
		ash->dwUser = codec;
		ash->cbSrcLength = codeccframelen[codec];
		ash->cbDstLength = codecuframelen[codec];
		ash->pbSrc = (BYTE *)malloc(codeccframelen[codec]);
		ash->pbDst = (BYTE *)malloc(codecuframelen[codec]);
		acmStreamPrepareHeader(hacm, ash, 0);
		return hacm;
	} else return 0;
}

int ACMDecode(HACMSTREAM hacm, ACMSTREAMHEADER *ash, const unsigned char *compressed, short *frame, int len, int codec)
{
	int rc, i;

	if(codec == CODEC_GSM)
	{
		for(i = 0; i < len; i += 65)
		{
			gsm_decode((gsm)hacm, (BYTE *)compressed + 65 * i, (short *)frame + 320 * i);
			gsm_decode((gsm)hacm, (BYTE *)compressed + 65 * i + 33, (short *)frame + 320 * i + 160);
		}
		return 0;
	}
	if(codec == CODEC_GSMFR)
	{
		for(i = 0; i < len; i += 33)
			gsm_decode((gsm)hacm, (BYTE *)compressed + 33 * i, (short *)frame + 320 * i);
		return 0;
	}
	if(codec == CODEC_G711A)
	{
		for(i = 0; i < len; i++)
			frame[i] = alaw2linear(compressed[i]);
		return 0;
	}
	if(codec == CODEC_G711U)
	{
		for(i = 0; i < len; i++)
			frame[i] = mulaw2linear(compressed[i]);
		return 0;
	}
#ifdef INCLUDESPEEX
	else if(codec >= CODEC_SPEEXA && codec <= CODEC_SPEEXE)
	{
		for(i = 0; i < len; i += codeccframelen[codec])
			speex_decode(hacm, compressed + codeccframelen[codec] * i, frame + codecuframelen[codec] * i);
	}
#endif
	else for(i = 0; i < len; i += codeccframelen[codec])
	{
		memcpy(ash->pbSrc, compressed, codeccframelen[codec]);
		rc = acmStreamConvert(hacm, ash, ACM_STREAMCONVERTF_BLOCKALIGN);
		memcpy(frame + i * codecuframelen[codec], ash->pbDst, codecuframelen[codec]);
	}
	return rc;
}

int ACMDecoderClose(HACMSTREAM hacm, ACMSTREAMHEADER *ash)
{
	if(ash->dwUser == CODEC_GSMFR)
//	if(ash->dwUser == CODEC_GSM || ash->dwUser == CODEC_GSMFR)
		gsm_destroy((gsm)hacm);
	else if(ash->dwUser == CODEC_G711A || ash->dwUser == CODEC_G711U)
		;
#ifdef INCLUDESPEEX
	else if(ash->dwUser >= CODEC_SPEEXA && ash->dwUser <= CODEC_SPEEXE)
		speex_destroy(hacm);
#endif
	else {
		acmStreamConvert(hacm, ash, ACM_STREAMCONVERTF_BLOCKALIGN | ACM_STREAMCONVERTF_END);
		acmStreamUnprepareHeader(hacm, ash, 0);
		acmStreamClose(hacm, 0);
		free(ash->pbSrc);
		free(ash->pbDst);
	}
	return 0;
}

tAudioEncoder::~tAudioEncoder()
{
	if(hAcm)
		ACMEncoderClose(hAcm, &ash);
}

int tAudioEncoder::Encode(int codec, short *in, unsigned inbytes, BYTE *out)
{
	unsigned i, cycles;
	int outbytes = 0;

	if(!hAcm || this->codec != codec)
	{
		this->codec = codec;
		if(hAcm)
			ACMEncoderClose(hAcm, &ash);
		hAcm = ACMEncoderOpen(codec, &ash);
		if(!hAcm)
			return 0;
	}
	for(i = cycles = 0; cycles < 8 && i < inbytes; i += codecuframelen[codec], cycles++)
	{
		ACMEncode(hAcm, &ash, in + i / 2, out, codec);
		out += codeccframelen[codec];
		outbytes += codeccframelen[codec];
	}
	return outbytes;
}

tAudioDecoder::~tAudioDecoder()
{
	if(hAcm)
		ACMDecoderClose(hAcm, &ash);
}

int tAudioDecoder::Decode(int codec, const BYTE *in, unsigned inbytes, short *out)
{
	int outbytes = 0;
	unsigned i, cycles;

	if(!hAcm || this->codec != codec)
	{
		this->codec = codec;
		if(hAcm)
			ACMDecoderClose(hAcm, &ash);
		hAcm = ACMDecoderOpen(codec, &ash);
		if(!hAcm)
			return 0;
	}
	for(i = cycles = 0; cycles < 8 && i < inbytes; i += codeccframelen[codec], cycles++)
	{
		ACMDecode(hAcm, &ash, in + i, out, codeccframelen[codec], codec);
		out += codecuframelen[codec] / 2;
		outbytes += codecuframelen[codec];
	}
	return outbytes;
}

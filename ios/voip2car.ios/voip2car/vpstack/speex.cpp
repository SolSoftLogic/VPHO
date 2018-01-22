#include <string.h>
#include <stdlib.h>

#include "speex/speex.h"
#include "speex/speex_header.h"
#include "speex/speex_stereo.h"
#include "speex/speex_callbacks.h"

#define UINT unsigned int
#define SHORT short
#define BYTE unsigned char
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#define min(a,b)            (((a) < (b)) ? (a) : (b))

struct SPEEX {
	SPEEX() { memset(this, 0, sizeof(SPEEX)); }
	char type;

    SpeexBits bits;
    const SpeexMode *speex_mode;
	SpeexHeader speex_header;
	int frame_size;
	int bits_per_frame;
    void *enc_state;
    void *dec_state;
	SpeexCallback speex_callback;
	int block;
	int samples;
	float fp_tmp[2*640];
};

static int rates[5] = {8000, 16000, 32000, 32000, 8000};
static int qualities[5] = {4,4,5,9,2};
static struct {
	UINT nBitsPerFrame;
	UINT nFrameSize;
	UINT nFramesPerBlock;
	UINT nEffectiveBitrate;
} QualityInfo[5]	= {
	  160, 160,	1,	8000,	//	8000 1 4
	  256, 320,	1, 12800,	// 16000 1 4
	  372, 640,	2, 18600,	// 32000 1 5
	  720, 640,	1, 36000,	// 32000 1 9
	  119, 160,	8,	5950,	//	8000 1 2
};

void *speex_create(int decoder, int type)
{
	if(type < 0 || type > 4)
		return 0;
	int tmp;

	SPEEX *psi = new SPEEX;

	psi->type = (char)type;
	psi->speex_mode = rates[type] == 8000 ? &speex_nb_mode : rates[type] == 16000 ? &speex_wb_mode : &speex_uwb_mode;
	speex_mode_query(psi->speex_mode, SPEEX_MODE_FRAME_SIZE, &psi->frame_size);
	speex_init_header(&psi->speex_header, rates[type], 1, psi->speex_mode);

	psi->bits_per_frame = QualityInfo[type].nBitsPerFrame;
	psi->speex_header.frames_per_packet = QualityInfo[type].nFramesPerBlock;
	psi->speex_header.vbr = 0;
	psi->speex_header.nb_channels = 1;

	if(decoder)
		psi->dec_state = speex_decoder_init(psi->speex_mode);
	else {
		psi->enc_state = speex_encoder_init(psi->speex_mode);
		tmp	= qualities[type];
		speex_encoder_ctl(psi->enc_state, SPEEX_SET_QUALITY, &tmp);
	}
	speex_bits_init(&psi->bits);
	psi->block = 0;
	psi->samples = 0;
	
	return psi;
}

void speex_destroy(void *hcodec)
{
	SPEEX *psi = (SPEEX *)hcodec;

	if (psi->enc_state)
		speex_encoder_destroy(psi->enc_state);
	if (psi->dec_state)
		speex_decoder_destroy(psi->dec_state);
	speex_bits_destroy(&psi->bits);
	free(hcodec);
}

unsigned short le_short(unsigned short s)
{
   unsigned short ret=s;
#ifdef WORDS_BIGENDIAN
   ret =  s>>8;
   ret += s<<8;
#endif
   return ret;
}

int speex_encode(void *hcodec, const short *frame, unsigned char *compressed)
{
	SPEEX *psi = (SPEEX *)hcodec;
	int remaining = QualityInfo[psi->type].nFrameSize * QualityInfo[psi->type].nFramesPerBlock;
	int frame_size = psi->frame_size;
	int frames_per_block = psi->speex_header.frames_per_packet;
	const SHORT *pSample = frame;

	while(remaining > 0)
	{
		int i;

		// can we fill up the sample buffer entirely?
		if (remaining >= (frame_size - psi->samples))
		{
			// fill up the sample buffer entirely
			for (i=psi->samples; i<frame_size; i++) psi->fp_tmp[i] = (SHORT)le_short(*pSample++); 
			remaining -= (frame_size - psi->samples);
			psi->samples = frame_size;
		}
		else
		{
			// continue filling the sample buffer with all we got
			for (i=psi->samples; i<remaining; i++) psi->fp_tmp[i] = (SHORT)le_short(*pSample++);
			psi->samples += remaining;
			remaining = 0;
		}

		// is the samplebuffer full?
		if (psi->samples == frame_size)
		{
			speex_encode(psi->enc_state, psi->fp_tmp, &psi->bits);

			psi->samples = 0;

			psi->block++;

			// write out the data we've collected if we filled the
			// block with frames or if this is the end of stream
			if (psi->block == frames_per_block)
			{
				compressed += speex_bits_write(&psi->bits, (char *)compressed, 65536);
				speex_bits_reset(&psi->bits);

				psi->block = 0;
			}
		}
	}
	return 0;
}

int speex_decode(void *hcodec, const unsigned char *compressed, short *frame)
{
	SPEEX *psi = (SPEEX *)hcodec;
    const BYTE *            pbDstStart;
	SHORT *pSample;

	int frame_size = psi->frame_size;
	int frames_per_block = psi->speex_header.frames_per_packet;
	int remaining, nBlockAlignment;

	pbDstStart = compressed;

	nBlockAlignment = QualityInfo[psi->type].nBitsPerFrame;
	remaining = frames_per_block * nBlockAlignment / 8;
	pSample = frame;

	while (remaining > 0 || speex_bits_remaining(&psi->bits) >= psi->bits_per_frame)
	{
		if (psi->block % frames_per_block == 0)
		{
			// skip padding bits, forward to next full byte
			if (psi->bits.bitPtr != 0)
			{
				psi->bits.bitPtr = 0;
				psi->bits.charPtr++;
			}
		}

		if (remaining >= nBlockAlignment)
		{
			speex_bits_read_whole_bytes(&psi->bits, (char*)compressed, nBlockAlignment);
			compressed += nBlockAlignment;
			remaining -= nBlockAlignment;
		}
		else
		{
			speex_bits_read_whole_bytes(&psi->bits, (char *)compressed, remaining);
			compressed += remaining;
			remaining = 0;
		}

		while (speex_bits_remaining(&psi->bits) >= psi->bits_per_frame) 
		{
			int ret;
			int i;

			ret = speex_decode(psi->dec_state, &psi->bits, psi->fp_tmp);

			if (ret < 0 || 
                speex_bits_remaining(&psi->bits)<0)
			{
				speex_bits_reset(&psi->bits);
				psi->block = 0;
				break;
			}

			for (i=0; i<frame_size; i++) {
				*pSample++ = (SHORT)le_short((SHORT)max(min(psi->fp_tmp[i],32767.0),-32768.0));
			};

			psi->block++;
			psi->block = psi->block % frames_per_block;

			if (psi->block == 0) break;
		}
	}

	return 0;
}

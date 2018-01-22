#include "videocodecs.h"
#include "../avc_h264/enc/src/avcenc_api.h"
#include "../avc_h264/dec/include/avcdec_api.h"
#include "../avc_h264/enc/include/pvavcencoder.h"
#include "../avc_h264/enc/include/pvavcencoder_factory.h"
#include "../avc_h264/dec/include/pvavcdecoder.h"
#include "../avc_h264/dec/include/pvavcdecoder_factory.h"
#include "../avc_h264/colorconvert/include/ccyuv420toyuv422.h"
#include <string.h>

class AVCCODEC : public VIDEOCODEC
{
public:
	AVCCODEC();
	~AVCCODEC();
	int StartEncode(int width, int height, unsigned fourcc, unsigned framerate, unsigned quality);
	int StartDecode(int width, int height, unsigned fourcc);
	int Encode(const void *in, void *out, unsigned *len, int *iskey);
	int Decode(const void *in, int len, void *out);
	int End();

protected:
	// Decoder related
	friend int avcDPBAlloc(void *userData, uint frame_size_in_mbs, uint num_buffers);
	friend void avcFrameUnbind(void *userData, int indx);
	friend int avcFrameBind(void *userData, int indx, uint8 **yuv);
	int DPBAlloc(uint frame_size_in_mbs, uint num_buffers);
	void FrameUnbind(int indx);
	int FrameBind(int indx, uint8 **yuv);
	AVCHandle *dec;
	uint8 *frames;
	bool *frames_avail;
	unsigned nframes, frame_size;
	bool decinit;

	// Encoder related
	PVAVCEncoderInterface *enc;
	uint8 *tmpframe;
	// Common
	int width, height;
	unsigned ts, framerate, fourcc;
};

VIDEOCODEC *NewAVC() { return new AVCCODEC(); }

AVCCODEC::AVCCODEC()
{
	width = height = 0;
	enc = 0;
	dec = 0;
	frames = 0;
	tmpframe = 0;
	frames_avail = 0;
}

AVCCODEC::~AVCCODEC()
{
	End();
}

int AVCCODEC::StartEncode(int width, int height, unsigned fourcc, unsigned framerate, unsigned quality)
{
	TAVCEI_RETVAL rc;
	TAVCEIInputFormat informat;
	TAVCEIEncodeParam encparam;

	switch(fourcc)
	{
	case F_I420:
		informat.iFrameOrientation = -1;
		informat.iVideoFormat = EAVCEI_VDOFMT_YUV420;
		break;
	case F_YV12:
		informat.iFrameOrientation = -1;
		informat.iVideoFormat = EAVCEI_VDOFMT_YVU420;
		break;
	case F_UYVY:
		informat.iFrameOrientation = -1;
		informat.iVideoFormat = EAVCEI_VDOFMT_YUV420;
		break;
	case F_YUY2:
		informat.iFrameOrientation = -1;
		informat.iVideoFormat = EAVCEI_VDOFMT_YUV420;
		break;
	case F_BGR12:
		informat.iFrameOrientation = -1;
		informat.iVideoFormat = EAVCEI_VDOFMT_RGB12;
		break;
	case F_BGR24:
		informat.iFrameOrientation = 1;
		informat.iVideoFormat = EAVCEI_VDOFMT_RGB24;
		break;
	default:
		return -1;
	}
	if(enc)
		PVAVCEncoderFactory::DeletePVAVCEncoder(enc);
	enc = PVAVCEncoderFactory::CreatePVAVCEncoder();
	this->width = width;
	this->height = height;
	this->framerate = framerate;
	this->fourcc = fourcc;
	informat.iFrameWidth = width;
	informat.iFrameHeight = height;
	informat.iFrameRate = (float)framerate;
	memset(&encparam, 0, sizeof encparam);
	encparam.iEncodeID = 0;
	encparam.iProfile = EAVCEI_PROFILE_BASELINE;
	encparam.iLevel = EAVCEI_LEVEL_AUTODETECT;
	encparam.iNumLayer = 1;
	encparam.iFrameWidth[0] = width;
	encparam.iFrameHeight[0] = height;
	encparam.iBitRate[0] = width * height * framerate * (quality & 0xf) / 12;
	encparam.iFrameRate[0] = (float)framerate;
	encparam.iEncMode = EAVCEI_ENCMODE_RECORDER;
	encparam.iOutOfBandParamSet = false;
	encparam.iOutputFormat = EAVCEI_OUTPUT_ANNEXB;
	encparam.iPacketSize = 0;
	if(quality & 0x100)
		encparam.iRateControlType = EAVCEI_RC_CONSTANT_Q;
	else encparam.iRateControlType = EAVCEI_RC_CBR_1;
	encparam.iBufferDelay = 2.0;
	encparam.iIquant[0] = 40 - (2*(quality & 0xf));
	encparam.iSceneDetection = true;
	encparam.iIFrameInterval = (quality>>4) & 0xf;
	encparam.iNumIntraMBRefresh = 0;
	encparam.iClipDuration = 0;
	rc = enc->Initialize(&informat, &encparam);
	if(rc)
		goto fail;
	if(fourcc == F_UYVY || fourcc == F_YUY2)
		tmpframe = (uint8 *)malloc(width*height*3/2);
	ts = 0;
	return 0;
fail:
	PVAVCEncoderFactory::DeletePVAVCEncoder(enc);
	return -1;
}

static int avcmalloc(void *userData, int32 size, int attribute)
{
	return (int)malloc(size);
}

static void avcfree(void *userData, int mem)
{
	free((void *)mem);
}

static void avcdebug(uint32 *userData, AVCLogType type, char *string1, int val1, int val2)
{
}

int avcDPBAlloc(void *userData, uint frame_size_in_mbs, uint num_buffers)
{
	return ((AVCCODEC *)userData)->DPBAlloc(frame_size_in_mbs, num_buffers);
}

void avcFrameUnbind(void *userData, int indx)
{
    ((AVCCODEC *)userData)->FrameUnbind(indx);
}

int avcFrameBind(void *userData, int indx, uint8 **yuv)
{
    return ((AVCCODEC *)userData)->FrameBind(indx, yuv);
}

int AVCCODEC::DPBAlloc(uint frame_size_in_mbs, uint num_buffers)
{
	if(frames)
		free(frames);
	if(frames_avail)
		free(frames_avail);
	frames = (uint8 *)malloc(frame_size_in_mbs * 384 * num_buffers);
	frames_avail = (bool *)malloc(num_buffers * sizeof(bool));
	memset(frames_avail, 0, num_buffers * sizeof(bool));
	nframes = num_buffers;
	frame_size = frame_size_in_mbs * 384;
	return 1;
}

void AVCCODEC::FrameUnbind(int indx)
{
	if(indx >= 0 && indx < (int)nframes)
		frames_avail[indx] = false;
}

void Lprintf(const char *fmt, ...);

int AVCCODEC::FrameBind(int indx, uint8 **yuv)
{
	if(indx < 0 || indx >= (int)nframes || frames_avail[indx])
	{
		//Lprintf("NOMEM indx=%d, nframes=%d", indx, nframes);
		return 0;
	}
	frames_avail[indx] = true;
	*yuv = frames + frame_size * indx;
	return 1;
}

int AVCCODEC::StartDecode(int width, int height, unsigned fourcc)
{
	if(fourcc != F_I420 && fourcc != F_YV12 && fourcc != F_YUY2 && fourcc != F_UYVY && fourcc != F_BGR24)
		return -1;
	if(dec)
	{
		PVAVCCleanUpDecoder(dec);
		delete dec;
	}
	dec = new AVCHandle();
	this->fourcc = fourcc;
	memset(dec, 0, sizeof *dec);
	dec->userData = this;
	dec->CBAVC_Malloc = avcmalloc;
	dec->CBAVC_Free = avcfree;
	dec->CBAVC_DebugLog = avcdebug;
	dec->CBAVC_DPBAlloc = avcDPBAlloc;
	dec->CBAVC_FrameBind = avcFrameBind;
	dec->CBAVC_FrameUnbind = avcFrameUnbind;
	decinit = false;
	this->width = width;
	this->height = height;
	return 0;
}

void yuy2toyuv(unsigned char *yuy2, unsigned char **yuv, int xres, int yres)
{
	int x, y, xres2;
	
	xres2 = xres / 2;
	for(y = 0; y < yres; y++)
		for(x = 0; x < xres2; x++)
		{
			yuv[0][y*xres+2*x] = *yuy2++;
			yuv[1][y/2*xres2+x] = *yuy2++;
			yuv[0][y*xres+2*x+1] = *yuy2++;
			yuv[2][y/2*xres2+x] = *yuy2++;
		}
}

void uyvytoyuv(unsigned char *yuy2, unsigned char **yuv, int xres, int yres)
{
	int x, y, xres2;
	
	xres2 = xres / 2;
	for(y = 0; y < yres; y++)
		for(x = 0; x < xres2; x++)
		{
			yuv[1][y/2*xres2+x] = *yuy2++;
			yuv[0][y*xres+2*x] = *yuy2++;
			yuv[2][y/2*xres2+x] = *yuy2++;
			yuv[0][y*xres+2*x+1] = *yuy2++;
		}
}

int AVCCODEC::Encode(const void *in, void *out, unsigned *len, int *iskey)
{
	TAVCEIInputData indata;
	TAVCEIOutputData outdata;
	TAVCEI_RETVAL rc;
	int rem, SPSlen = 0, PPSlen = 0;

	if(!enc)
		return -1;
	if(fourcc == F_YUY2 || fourcc == F_UYVY)
	{
		unsigned char *yuv[3];

		yuv[0] = tmpframe;
		yuv[1] = tmpframe + width*height;
		yuv[2] = tmpframe + width*height * 5 / 4;
		if(fourcc == F_YUY2)
			yuy2toyuv((unsigned char *)in, yuv, width, height);
		else uyvytoyuv((unsigned char *)in, yuv, width, height);
		indata.iSource = tmpframe;
	} else indata.iSource = (uint8 *)in;
	indata.iTimeStamp = ts;
	ts += 1000 / framerate;
	rc = enc->Encode(&indata);
	if(rc == EAVCEI_FRAME_DROP)
	{
		*len = 0;
		*iskey = 0;
		return 0;
	}
	if(rc)
		return -1;
	*((uint8 *)out) = 0;	// First byte is 0 by default (no SPS and no PPS)
	outdata.iBitstream = (uint8 *)out + 1;
	outdata.iBitstreamSize = 1000000;
	rc = enc->GetOutput(&outdata, &rem);
	if(rc == EAVCEI_MORE_NAL)
	{
		SPSlen = outdata.iBitstreamSize;
		outdata.iBitstream = (uint8 *)out + SPSlen + 1;
		outdata.iBitstreamSize = 1000000;
		rc = enc->GetOutput(&outdata, &rem);
		if(rc == EAVCEI_MORE_NAL)
		{
			PPSlen = outdata.iBitstreamSize;
			// First byte encodes the lengths of SPS and PPS, then SPS follows, then PPS
			*((uint8 *)out) = SPSlen<<4 | PPSlen;
			outdata.iBitstream = (uint8 *)out + SPSlen + PPSlen + 1;
			outdata.iBitstreamSize = 1000000;
			rc = enc->GetOutput(&outdata, &rem);
		}
	}
	if(rc == EAVCEI_FRAME_DROP)
	{
		*len = 0;
		*iskey = 0;
		return 0;
	}
	if(rc)
		return -1;
	*iskey = outdata.iKeyFrame;
	*len = outdata.iBitstreamSize + SPSlen + PPSlen + 1;
	return 0;
}

void yuvtoyuy2(unsigned char **yuv, unsigned char *yuy2, int xres, int yres)
{
	int x, y, xres2;
	
	xres2 = xres / 2;
	for(y = 0; y < yres; y++)
		for(x = 0; x < xres2; x++)
		{
			*yuy2++ = yuv[0][y*xres+2*x];
			*yuy2++ = yuv[1][y/2*xres2+x];
			*yuy2++ = yuv[0][y*xres+2*x+1];
			*yuy2++ = yuv[2][y/2*xres2+x];
		}
}

void yuvtouyvy(unsigned char **yuv, unsigned char *yuy2, int xres, int yres)
{
	int x, y, xres2;
	
	xres2 = xres / 2;
	for(y = 0; y < yres; y++)
		for(x = 0; x < xres2; x++)
		{
			*yuy2++ = yuv[1][y/2*xres2+x];
			*yuy2++ = yuv[0][y*xres+2*x];
			*yuy2++ = yuv[2][y/2*xres2+x];
			*yuy2++ = yuv[0][y*xres+2*x+1];
		}
}

void yuvtorgb(unsigned char **yuv, unsigned char *rgb, int xres, int yres)
{
	int x, y, Y, U, V, YUR, YUG, YUB, xres2 = xres / 2;

#ifndef TARGET_OS_IPHONE    
	rgb = rgb + 3*xres*(yres-1);
#endif   
    unsigned char *rgb_ptr = rgb;
	for(y = 0; y < yres; y++)
	{
		for(x = 0; x < xres; x++)
		{
			Y = (yuv[0][y*xres+x]-16) * 298 / 256;
			U = yuv[1][y/2*xres2+x/2] - 128;
			V = yuv[2][y/2*xres2+x/2] - 128;

			YUR = Y + 459 * V / 256;
			YUG = Y + (-137 * U - 55 * V) / 256;
			YUB = Y + 541 * U / 256;
			if(YUR < 0)
				YUR = 0;
			if(YUR > 255)
				YUR = 255;
			if(YUG < 0)
				YUG = 0;
			if(YUG > 255)
				YUG = 255;
			if(YUB < 0)
				YUB = 0;
			if(YUB > 255)
				YUB = 255;
#ifdef TARGET_OS_IPHONE                                                                                                                  
                        *rgb_ptr++ = (unsigned char)YUR;                                                                                      
                        *rgb_ptr++ = (unsigned char)YUG;                                                                                      
                        *rgb_ptr++ = (unsigned char)YUB;                                                                                      
#else           
			*rgb++ = (unsigned char)YUB;
			*rgb++ = (unsigned char)YUG;
			*rgb++ = (unsigned char)YUR;
#endif
		}
#ifndef TARGET_OS_IPHONE         
		rgb -= 6*xres;
#endif        
	}
}

int AVCCODEC::Decode(const void *in, int len, void *out)
{
	AVCDec_Status rc;
	AVCFrameIO outdata;
	int indx, release_flag, SPSlen, PPSlen;

	if(!dec)
		return -100;
	if(*((uint8 *)in))
	{
		SPSlen = *((uint8 *)in) >> 4;
		PPSlen = *((uint8 *)in) & 0xf;
		rc = PVAVCDecSeqParamSet(dec, (uint8 *)in + 1, SPSlen);
		if(rc != AVCDEC_SUCCESS)
			return rc - 200;
		rc = PVAVCDecPicParamSet(dec, (uint8 *)in + 1 + SPSlen, PPSlen);
		if(rc != AVCDEC_SUCCESS)
			return rc - 300;
		len -= SPSlen + PPSlen;
		in = (uint8 *)in + SPSlen + PPSlen;
		decinit = true;
	} else if(!decinit)
		return -101;
	in = (uint8 *)in + 1;
	len--;
	rc = PVAVCDecodeSlice(dec, (uint8 *)in, len);
	if(rc != AVCDEC_PICTURE_READY)
		return rc - 400;
	rc = PVAVCDecGetOutput(dec, &indx, &release_flag, &outdata);
	if(rc != AVCDEC_SUCCESS)
		return rc - 500;
	switch(fourcc)
	{
	case F_I420:
		memcpy((char *)out, outdata.YCbCr[0], width*height);
		memcpy((char *)out+width*height, outdata.YCbCr[1], width*height/4);
		memcpy((char *)out+width*height*5/4, outdata.YCbCr[2], width*height*4);
		break;
	case F_YV12:
		memcpy((char *)out, outdata.YCbCr[0], width*height);
		memcpy((char *)out+width*height, outdata.YCbCr[2], width*height/4);
		memcpy((char *)out+width*height*5/4, outdata.YCbCr[1], width*height*4);
		break;
	case F_YUY2:
		yuvtoyuy2(outdata.YCbCr, (unsigned char *)out, width, height);
		break;
	case F_UYVY:
		yuvtouyvy(outdata.YCbCr, (unsigned char *)out, width, height);
		break;
	case F_BGR24:
		yuvtorgb(outdata.YCbCr, (unsigned char *)out, width, height);
		break;
	}
	return 0;
}

int AVCCODEC::End()
{
	if(enc)
	{
		PVAVCEncoderFactory::DeletePVAVCEncoder(enc);
		enc = 0;
	}
	if(dec)
	{
		PVAVCCleanUpDecoder(dec);
		delete dec;
		dec = 0;
	}
	if(frames)
	{
		free(frames);
		frames = 0;
	}
	if(frames_avail)
	{
		free(frames_avail);
		frames_avail = 0;
	}
	if(tmpframe)
	{
		free(tmpframe);
		tmpframe = 0;
	}
	return 0;
}


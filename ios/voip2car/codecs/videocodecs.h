#ifndef _VIDEOCDEOC_H_INCLUDED_
#define _VIDEOCDEOC_H_INCLUDED_

class VIDEOCODEC {
public:
	virtual ~VIDEOCODEC() {}
	virtual int StartEncode(int width, int height, unsigned fourcc, unsigned framerate, unsigned quality) = 0;
	virtual int StartDecode(int width, int height, unsigned fourcc) = 0;
	virtual int Encode(const void *in, void *out, unsigned *len, int *iskey) = 0;
	virtual int Decode(const void *in, int len, void *out) = 0;
	virtual int End() = 0;
};

#ifndef mmioFOURCC
#define mmioFOURCC(a,b,c,d) ((a) | (b)<<8 | (c)<<16 | (d)<<24)
#endif

#define F_VP31 mmioFOURCC('V','P','3','1')
#define F_XVID mmioFOURCC('X','V','I','D')
#define F_H264 mmioFOURCC('H','2','6','4')
#define F_I420 mmioFOURCC('I','4','2','0')
#define F_YV12 mmioFOURCC('Y','V','1','2')
#define F_YUY2 mmioFOURCC('Y','U','Y','2')
#define F_UYVY mmioFOURCC('U','Y','V','Y')
#define F_BGRA32 32
#define F_BGR24 24
#define F_BGR16 16
#define F_BGR15 15
#define F_BGR12 12

VIDEOCODEC *NewVP3();
VIDEOCODEC *NewXVID();
VIDEOCODEC *NewAVC();

int VP3CODEC_Init();
void VP3CODEC_Finish();

// Video encoding quality encoding:
// VP3
// 0xKb, key frame every K seconds, bitrate = width*height*framerate*b/76.8
// XVID
// 0xKb, key frame every K seconds, zone quantizer increment = b * 100
// H.264
// 0x1Kb, key frame every K seconds, bitrate = width*height*framerate*b/12
// 0x0Kb, key frame every K seconds, Q = 40-2*b

#endif

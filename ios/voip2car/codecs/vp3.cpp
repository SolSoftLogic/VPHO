#include <windows.h>
#include <stdio.h>
#include "cclib.h"
#include "videocodecs.h"

extern "C"
{
#include "vp31/TYPE_ALIASES.h"
#include "vp31/vfw_comp_interface.h"
#include "vp31/vfw_pb_interface.h"
#include "duck_dxl.h"

int vp31_Init(void);
int vp31_Exit(void);
}

static unsigned count;
int VP3CODEC_Init();
void VP3CODEC_Finish();

class VP3CODEC : public VIDEOCODEC
{
public:
	VP3CODEC();
	~VP3CODEC();
	int StartEncode(int width, int height, unsigned fourcc, unsigned framerate, unsigned quality);
	int StartDecode(int width, int height, unsigned fourcc);
	int Encode(const void *in, void *out, unsigned *len, int *iskey);
	int Decode(const void *in, int len, void *out);
	int End();

protected:
	void DefaultSettings(COMP_CONFIG &compConfig);

	int mode;
	unsigned fourcc;
	int width, height;
	unsigned sizeimage;
	BITDEPTH bitdepth;

	// Compressor
	xCP_INST cpi;
	COMP_CONFIG compConfig;
	YUV_INPUT_BUFFER_CONFIG  yuvConfig;
	unsigned char *yuvBuffer;

	// Decompressor
    int showWhiteDots;
    int postProcessingLevel;
    int cpuFree;
    DXL_XIMAGE_HANDLE xim;
    DXL_VSCREEN_HANDLE vsc;
};

VIDEOCODEC *NewVP3() { return new VP3CODEC(); }

int VP3CODEC_Init()
{
	if(!count)
	{
		InitCCLib(SpecialProc);
		VPEInitLibrary();
		DXL_InitVideoEx(64,64);
		vp31_Init();
	}
	count++;
	return 0;
}

void VP3CODEC_Finish()
{
	count--;
	if(!count)
	{
		VPEDeInitLibrary();
	    DXL_ExitVideo(); 
		vp31_Exit();
		DeInitCCLib();
	}
}

VP3CODEC::VP3CODEC()
{
	showWhiteDots = 0;
	mode = 0;
	xim = 0;
	vsc = 0;
	cpi = 0;
	yuvBuffer = 0;
	cpuFree = 70;
	postProcessingLevel = 6;
	DefaultSettings(compConfig);
}

VP3CODEC::~VP3CODEC()
{
	End();
}

void VP3CODEC::DefaultSettings(COMP_CONFIG &compConfig)
{ 
	compConfig.FrameSize						= 0;
	compConfig.TargetBitRate					= 300;
	compConfig.FrameRate 						= 25;
	compConfig.KeyFrameFrequency 				= 120;
	compConfig.KeyFrameDataTarget 				= 110;      
	compConfig.Quality 							= 32;
	compConfig.AllowDF 							= FALSE;
	compConfig.QuickCompress					= FALSE;    
	compConfig.AutoKeyFrameEnabled				= TRUE;    
	compConfig.AutoKeyFrameThreshold			= 90;
	compConfig.MinimumDistanceToKeyFrame		= 8;
	compConfig.ForceKeyFrameEvery 				= 120;
    compConfig.NoiseSensitivity					= 2;        
    compConfig.Sharpness						= 1;        
}


// Higher nibble: key frame every x seconds (x * framerate frames)
// Lower nibble: quality
int VP3CODEC::StartEncode(int width, int height, unsigned fourcc, unsigned framerate, unsigned quality)
{
	if(fourcc != F_I420 && fourcc != F_YUY2 &&
		fourcc != F_BGR24 && fourcc != F_BGRA32 || width & 15 || height & 15)
		return -1;

	if(mode)
		End();
	yuvBuffer = new unsigned char [width * height * 3 / 2];

	// Set the frame size component
	compConfig.FrameSize = static_cast <DWORD>((width << 16) + height);
	compConfig.KeyFrameDataTarget = static_cast <int> (2*(width + height) / 10) * (quality&0xf) / 10;
	compConfig.FrameRate = framerate;
	compConfig.KeyFrameFrequency = compConfig.ForceKeyFrameEvery = framerate * (quality>>4 & 0xf);
	compConfig.TargetBitRate = width*height*framerate*(quality&0xf)/76800;
	compConfig.Quality = 56-3*(quality&0xf);
	yuvConfig.UVStride = width/2;
	yuvConfig.UVWidth = width/2;
	yuvConfig.UVHeight = height /2;
	yuvConfig.YStride = width;
	yuvConfig.YWidth = width;
	yuvConfig.YHeight = height;	
	yuvConfig.YBuffer = (char *)yuvBuffer;
	yuvConfig.UBuffer = (char *)yuvBuffer + yuvConfig.YWidth * yuvConfig.YHeight;
	yuvConfig.VBuffer = (char *)yuvConfig.UBuffer + yuvConfig.UVWidth * yuvConfig.UVHeight;
	
	if(fourcc == F_I420)
	{
		yuvConfig.YStride *= -1;
		yuvConfig.UVStride *= -1;
	}

    if(compConfig.KeyFrameFrequency==0)
        compConfig.KeyFrameFrequency = compConfig.ForceKeyFrameEvery;

    if(!StartEncoder(&cpi, &compConfig))
	{
		delete yuvBuffer;
		return -1;
	}
	this->width = width;
	this->height = height;
	this->fourcc = fourcc;
	mode = 1;
	return 0;
}

int VP3CODEC::StartDecode(int width, int height, unsigned fourcc)
{
	if(mode)
		End();
	if(fourcc != F_YUY2 && fourcc != F_YV12 && fourcc != F_BGR15 && fourcc != F_BGR16 &&
		fourcc != F_BGR24 && fourcc != F_BGRA32 || width & 15 || height & 15)
		return -1;
	this->width = width;
	this->height = height;
	this->fourcc = fourcc;
	mode = 2;
	switch(fourcc)
	{
	case F_YV12:
		sizeimage = width * height * 3 / 2;
		bitdepth = DXYV12;
		break;
	case 15:
		sizeimage = width * height * 2;
		bitdepth = DXRGB16_555;
		break;
	case F_BGR16:
		sizeimage = width * height * 2;
		bitdepth = DXRGB16_565;
		break;
	case F_YUY2:
		sizeimage = width * height * 2;
		bitdepth = DXYUY2;
		break;
	case F_BGR24:
		sizeimage = width * height * 3;
		bitdepth = DXRGB24;
		break;
	case F_BGRA32:
		sizeimage = width * height * 4;
		bitdepth = DXRGB32;
		break;
	}
	return 0;
}

int VP3CODEC::Encode(const void *in, void *out, unsigned *len, int *iskey)
{
	if(mode != 1)
		return -1;
	*iskey = 0;

	if(fourcc == F_I420)
	{
		// our input parameters are flipped from what we want and U and V are 
		// reversed from what we normally expect! Correct this.
		yuvConfig.YBuffer = (char *)in + yuvConfig.YWidth * (-1+yuvConfig.YHeight);
		yuvConfig.UBuffer = (char *)in + yuvConfig.YWidth * yuvConfig.YHeight + yuvConfig.UVWidth * (-1+yuvConfig.UVHeight);
		yuvConfig.VBuffer = (char *)yuvConfig.UBuffer + yuvConfig.UVWidth * yuvConfig.UVHeight;
	} else if(fourcc == F_YUY2)
		YUY2toYV12((unsigned char *)in+yuvConfig.YWidth*2*(yuvConfig.YHeight-1), yuvConfig.YWidth, yuvConfig.YHeight,
                    (unsigned char *)yuvConfig.YBuffer, (unsigned char *)yuvConfig.UBuffer, (unsigned char *)yuvConfig.VBuffer,
					-yuvConfig.YWidth * 2, yuvConfig.YWidth);
	else if(fourcc == 32)
		CC_RGB32toYV12((unsigned char *)in, yuvConfig.YWidth, yuvConfig.YHeight,
                    (unsigned char *)yuvConfig.YBuffer, (unsigned char *)yuvConfig.UBuffer, (unsigned char *)yuvConfig.VBuffer);
	else CC_RGB24toYV12((unsigned char *)in, yuvConfig.YWidth, yuvConfig.YHeight,
					(unsigned char *)yuvConfig.YBuffer, (unsigned char *)yuvConfig.UBuffer, (unsigned char *)yuvConfig.VBuffer);
	*len =  EncodeFrameYuv(cpi, &yuvConfig, (unsigned char *)out, (unsigned *)iskey);
	return 0;
}

int VP3CODEC::Decode(const void *in, int len, void *out)
{
	int bytes;

	if(mode != 2)
		return -1;
    if(!xim) 
	{
	    xim = DXL_CreateXImageOfType((BYTE *)in, F_VP31);
		if(!xim) 
			return -1;
		xim = DXL_AlterXImage(xim, (BYTE *)in, F_VP31, DXRGBNULL, width, height);
		if(!xim) 
			return -1;
		DXL_AlterXImageData(xim, (BYTE *)in);
		if(cpuFree)
			vp31_SetParameter(xim, 1, cpuFree);
		else vp31_SetParameter(xim, 0, postProcessingLevel);
	} else DXL_AlterXImageData(xim, (BYTE *)in);

	DXL_SetXImageCSize(xim, sizeimage);
	bytes = sizeimage / (width * height);
 
	if(!vsc) 
	{
		if(bitdepth == DXRGB24 || bitdepth == DXRGB32 || bitdepth == DXRGB16_555 || bitdepth == DXRGB16_565)
			vsc = DXL_CreateVScreen((BYTE *)out + (height-1)*width*bytes, bitdepth, -width * bytes, height);	
		else vsc = DXL_CreateVScreen((BYTE *)out, bitdepth, width * bytes, height);
		if(!vsc) 
			return -1;
		DXL_SetVScreenBlitQuality(vsc, DXBLIT_SAME);
		DXL_AlterVScreenView(vsc, 0, 0, width, height);
	} else {
		if(bitdepth == DXRGB24 || bitdepth == DXRGB32 || bitdepth == DXRGB16_555 || bitdepth == DXRGB16_565)
			DXL_AlterVScreen(vsc, (BYTE *)out + (height-1)*width*bytes, bitdepth, -width * bytes, height);
		else DXL_AlterVScreen(vsc, (BYTE *)out, bitdepth, width * bytes, height);
		DXL_SetVScreenBlitQuality(vsc, DXBLIT_SAME);
	}
	if(vsc && xim)
	{
		DXL_VScreenSetInfoDotsFlag(vsc, showWhiteDots);	
		DXL_dxImageToVScreen(xim, vsc);
	}
	return 0;
}

int VP3CODEC::End()
{
	if(mode == 1)	// If encoding
	{
		StopEncoder(&cpi);
		delete [] yuvBuffer;
	}
	if(mode == 2)
	{
		if(xim)
			DXL_DestroyXImage(xim);
		if(vsc) 
			DXL_DestroyVScreen( vsc);        
	}
	mode = 0;
	xim = 0;
	vsc = 0;
	cpi = 0;
	yuvBuffer = 0;
	return 0;
}

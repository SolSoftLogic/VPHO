#include <windows.h>
#include <stdio.h>
#include "videocodecs.h"
#include "xvid.h"

class XVIDCODEC : public VIDEOCODEC
{
public:
	XVIDCODEC();
	~XVIDCODEC();
	int StartEncode(int width, int height, unsigned fourcc, unsigned framerate, unsigned quality);
	int StartDecode(int width, int height, unsigned fourcc);
	int Encode(const void *in, void *out, unsigned *len, int *iskey);
	int Decode(const void *in, int len, void *out);
	int End();

protected:
	void *enc_handle, *dec_handle;
	void *tmpbuf;
	int tmpbuflen;
	int width, height, csp, bpp;
};

VIDEOCODEC *NewXVID() { return new XVIDCODEC(); }

XVIDCODEC::XVIDCODEC()
{
	enc_handle = dec_handle = 0;
	tmpbuf = 0;
}

XVIDCODEC::~XVIDCODEC()
{
	End();
}

int XVIDCODEC::StartEncode(int width, int height, unsigned fourcc, unsigned framerate, unsigned quality)
{
	xvid_enc_create_t xvid_enc_create;
	xvid_plugin_single_t single;
	xvid_gbl_init_t xvid_gbl_init;
	xvid_enc_plugin_t plugins[1];
	xvid_enc_zone_t zone;
	int xerr;
	static int ARG_QUANTS[6] = {2, 31, 2, 31, 2, 31};

	if(enc_handle)
	{
		xvid_encore(enc_handle, XVID_ENC_DESTROY, NULL, NULL);
		enc_handle = 0;
	}
	switch(fourcc)
	{
	case F_I420:
		csp = XVID_CSP_I420;
		bpp = 1;
		break;
	case F_YV12:
		csp = XVID_CSP_YV12;
		bpp = 1;
		break;
	case F_UYVY:
		csp = XVID_CSP_UYVY;
		bpp = 2;
		break;
	case F_YUY2:
		csp = XVID_CSP_YUY2;
		bpp = 2;
		break;
	case F_BGR24:
		csp = XVID_CSP_BGR;
		bpp = 3;
		break;
	default:
		return -1;
	}
	this->width = width;
	this->height = height;
	memset(&xvid_gbl_init, 0, sizeof(xvid_gbl_init));
	xvid_gbl_init.version = XVID_VERSION;
    xvid_gbl_init.debug = 0;
	xvid_global(NULL, XVID_GBL_INIT, &xvid_gbl_init, NULL);

	memset(&xvid_enc_create, 0, sizeof(xvid_enc_create));
	xvid_enc_create.version = XVID_VERSION;

	/* Width and Height of input frames */
	xvid_enc_create.width = width;
	xvid_enc_create.height = height;
	xvid_enc_create.profile = 0xf5; /* Unrestricted */

	xvid_enc_create.plugins = plugins;
	xvid_enc_create.num_plugins = 0;

	memset(&single, 0, sizeof(xvid_plugin_single_t));
	single.version = XVID_VERSION;
	single.bitrate = 0;
	single.reaction_delay_factor = 16;
	single.averaging_period = 100;
	single.buffer = 100;

	plugins[xvid_enc_create.num_plugins].func = xvid_plugin_single;
	plugins[xvid_enc_create.num_plugins].param = &single;
	xvid_enc_create.num_plugins++;

	zone.frame = 0;
	zone.base = 100;
	zone.mode = XVID_ZONE_QUANT;
	zone.increment = (quality & 0xf) * 100;
	xvid_enc_create.zones = &zone;
	xvid_enc_create.num_zones = 1;

	xvid_enc_create.fincr = 1;
	xvid_enc_create.fbase = framerate;
    xvid_enc_create.max_key_interval = framerate * ((quality>>4) & 0xf);

	xvid_enc_create.min_quant[0]=ARG_QUANTS[0];
	xvid_enc_create.min_quant[1]=ARG_QUANTS[2];
	xvid_enc_create.min_quant[2]=ARG_QUANTS[4];
	xvid_enc_create.max_quant[0]=ARG_QUANTS[1];
	xvid_enc_create.max_quant[1]=ARG_QUANTS[3];
	xvid_enc_create.max_quant[2]=ARG_QUANTS[5];

	xvid_enc_create.max_bframes = 2;
	xvid_enc_create.bquant_ratio = 150;
	xvid_enc_create.bquant_offset = 100;

	xvid_enc_create.global |= XVID_GLOBAL_PACKED;
	xvid_enc_create.global |= XVID_GLOBAL_CLOSED_GOP;
	xerr = xvid_encore(NULL, XVID_ENC_CREATE, &xvid_enc_create, NULL);

	enc_handle = xvid_enc_create.handle;

	return xerr;
}

int XVIDCODEC::StartDecode(int width, int height, unsigned fourcc)
{
	int ret;

	if(dec_handle)
	{
		xvid_decore(dec_handle, XVID_DEC_DESTROY, NULL, NULL);
		dec_handle = 0;
	}
	switch(fourcc)
	{
	case F_BGR15:
	case F_BGR16:
		csp = XVID_CSP_RGB555;
		bpp = 2;
		break;
	case F_BGR24:
		csp = XVID_CSP_BGR;
		bpp = 3;
		break;
	case F_BGRA32:
		csp = XVID_CSP_BGRA;
		bpp = 4;
		break;
	case F_YUY2:
		csp = XVID_CSP_YUY2;
		bpp = 2;
		break;
	case F_UYVY:
		csp = XVID_CSP_UYVY;
		bpp = 2;
		break;
	case F_I420:
		csp = XVID_CSP_I420;
		bpp = 1;
		break;
	case F_YV12:
		csp = XVID_CSP_YV12;
		bpp = 1;
		break;
	default:
		return -1;
	}
	this->width = width;
	this->height = height;
	xvid_gbl_init_t   xvid_gbl_init;
	xvid_dec_create_t xvid_dec_create;

	memset(&xvid_gbl_init, 0, sizeof(xvid_gbl_init_t));
	memset(&xvid_dec_create, 0, sizeof(xvid_dec_create_t));
	xvid_gbl_init.version = XVID_VERSION;
	xvid_gbl_init.debug = 0;
	xvid_global(NULL, 0, &xvid_gbl_init, NULL);

	xvid_dec_create.version = XVID_VERSION;
	xvid_dec_create.width = width;
	xvid_dec_create.height = height;
	ret = xvid_decore(NULL, XVID_DEC_CREATE, &xvid_dec_create, NULL);
	dec_handle = xvid_dec_create.handle;

	return ret;
}

int XVIDCODEC::Encode(const void *in, void *out, unsigned *len, int *iskey)
{
	int ret;

	if(!enc_handle)
		return -1;
	xvid_enc_frame_t xvid_enc_frame;
	xvid_enc_stats_t xvid_enc_stats;

	memset(&xvid_enc_frame, 0, sizeof(xvid_enc_frame));
	xvid_enc_frame.version = XVID_VERSION;

	memset(&xvid_enc_stats, 0, sizeof(xvid_enc_stats));
	xvid_enc_stats.version = XVID_VERSION;

	xvid_enc_frame.bitstream = out;
	xvid_enc_frame.length = -1;

	xvid_enc_frame.input.plane[0] = (void *)in;

	xvid_enc_frame.input.csp = csp;
	xvid_enc_frame.input.stride[0] = width * bpp;

	xvid_enc_frame.par = 1;

	// Various flags
	//xvid_enc_frame.type = XVID_TYPE_IVOP;
	//xvid_enc_frame.vol_flags |= XVID_VOL_QUARTERPEL;
	//xvid_enc_frame.motion |= XVID_ME_QUARTERPELREFINE16 | XVID_ME_QUARTERPELREFINE8;
	//xvid_enc_frame.vol_flags |= XVID_VOL_GMC;
	//xvid_enc_frame.motion |= XVID_ME_GME_REFINE;
	//xvid_enc_frame.vop_flags = XVID_VOP_INTER4V;;
	//xvid_enc_frame.vop_flags |= XVID_VOP_RD_BVOP;
	xvid_enc_frame.vop_flags |= XVID_VOP_HALFPEL;
	xvid_enc_frame.vop_flags |= XVID_VOP_HQACPRED;
	xvid_enc_frame.vop_flags |= XVID_VOP_TRELLISQUANT;
	xvid_enc_frame.motion |= XVID_ME_CHROMA_PVOP + XVID_ME_CHROMA_BVOP;
	//xvid_enc_frame.vol_flags |= XVID_VOL_INTERLACING;
	//xvid_enc_frame.vop_flags |= XVID_VOP_TOPFIELDFIRST;
	xvid_enc_frame.motion |= XVID_ME_FASTREFINE16 | XVID_ME_FASTREFINE8 | 
							 XVID_ME_SKIP_DELTASEARCH | XVID_ME_FAST_MODEINTERPOLATE | 
							 XVID_ME_BFRAME_EARLYSTOP;

	switch (1)
	{
	case 1: /* VHQ_MODE_DECISION */
		xvid_enc_frame.vop_flags |= XVID_VOP_MODEDECISION_RD;
		break;

	case 2: /* VHQ_LIMITED_SEARCH */
		xvid_enc_frame.vop_flags |= XVID_VOP_MODEDECISION_RD;
		xvid_enc_frame.motion |= XVID_ME_HALFPELREFINE16_RD;
		xvid_enc_frame.motion |= XVID_ME_QUARTERPELREFINE16_RD;
		break;

	case 3: /* VHQ_MEDIUM_SEARCH */
		xvid_enc_frame.vop_flags |= XVID_VOP_MODEDECISION_RD;
		xvid_enc_frame.motion |= XVID_ME_HALFPELREFINE16_RD;
		xvid_enc_frame.motion |= XVID_ME_HALFPELREFINE8_RD;
		xvid_enc_frame.motion |= XVID_ME_QUARTERPELREFINE16_RD;
		xvid_enc_frame.motion |= XVID_ME_QUARTERPELREFINE8_RD;
		xvid_enc_frame.motion |= XVID_ME_CHECKPREDICTION_RD;
		break;

	case 4: /* VHQ_WIDE_SEARCH */
		xvid_enc_frame.vop_flags |= XVID_VOP_MODEDECISION_RD;
		xvid_enc_frame.motion |= XVID_ME_HALFPELREFINE16_RD;
		xvid_enc_frame.motion |= XVID_ME_HALFPELREFINE8_RD;
		xvid_enc_frame.motion |= XVID_ME_QUARTERPELREFINE16_RD;
		xvid_enc_frame.motion |= XVID_ME_QUARTERPELREFINE8_RD;
		xvid_enc_frame.motion |= XVID_ME_CHECKPREDICTION_RD;
		xvid_enc_frame.motion |= XVID_ME_EXTSEARCH_RD;
		break;
	default :
		break;
	}


	/* Encode the frame */
	ret = xvid_encore(enc_handle, XVID_ENC_ENCODE, &xvid_enc_frame, &xvid_enc_stats);
	*iskey = !!(xvid_enc_frame.out_flags & XVID_KEYFRAME);
	*len = ret;
	//printf("ENC %d %d\n", ret, xvid_enc_stats.type);
	return ret < 0;
}

int XVIDCODEC::Decode(const void *in, int len, void *out)
{
	int ret;

	if(!dec_handle)
		return -1;
	//printf("DECIN %d\n", len);
	xvid_dec_frame_t xvid_dec_frame;
	xvid_dec_stats_t xvid_dec_stats;

	do {
		memset(&xvid_dec_frame, 0, sizeof(xvid_dec_frame_t));
		memset(&xvid_dec_stats, 0, sizeof(xvid_dec_stats_t));
		xvid_dec_frame.version = XVID_VERSION;
		xvid_dec_stats.version = XVID_VERSION;
		if(tmpbuf)
		{
			xvid_dec_frame.bitstream = tmpbuf;
			xvid_dec_frame.length = tmpbuflen;
		} else {
			xvid_dec_frame.bitstream = (void *)in;
			xvid_dec_frame.length = len;
		}
		xvid_dec_frame.output.plane[0]  = out;
		xvid_dec_frame.output.stride[0] = width * bpp;
		xvid_dec_frame.output.csp = csp;
		ret = xvid_decore(dec_handle, XVID_DEC_DECODE, &xvid_dec_frame, &xvid_dec_stats);
		if(tmpbuf)
		{
			//printf("DECTMP %d,%d,%d\n", tmpbuflen, ret, xvid_dec_stats.type);
			free(tmpbuf);
			tmpbuf = 0;
			if(ret == 0 && xvid_dec_stats.type <= 0 && len >= 0)
				ret = 1;
		} else if(ret > 0)
		{
			//printf("DEC %d,%d,%d\n", len, ret, xvid_dec_stats.type);
			in = (void *)((char *)in + ret);
			len -= ret;
		}// else printf("DEC %d,%d,%d\n", len, ret, xvid_dec_stats.type);
	} while(ret > 0 && xvid_dec_stats.type <= 0 && len > 0);

	if(len > 0)
	{
		tmpbuf = malloc(len);
		memcpy(tmpbuf, in, len);
		tmpbuflen = len;
	}
	//if(xvid_dec_stats.type <= 0)
	//	printf("************************ ALERT ******************************\n");
	//printf("RET %d\n", ret);
	return ret > 0 && xvid_dec_stats.type > 0 ? 0 : -1;
}

int XVIDCODEC::End()
{
	int xerr = 0;

	if(enc_handle)
		xerr |= xvid_encore(enc_handle, XVID_ENC_DESTROY, NULL, NULL);
	if(dec_handle)
		xerr |= xvid_decore(dec_handle, XVID_DEC_DESTROY, NULL, NULL);
	if(tmpbuf)
		free(tmpbuf);
	enc_handle = dec_handle = tmpbuf = 0;
	return xerr;
}


class THEORACODEC
{
public:
	THEORACODEC();
	~THEORACODEC();
	int StartEncode(int width, int height, unsigned fourcc, unsigned framerate, unsigned quality);
	int StartDecode(int width, int height, unsigned fourcc);
	int Encode(const void *in, void *out, unsigned *len, int *iskey);
	int Decode(const void *in, int len, void *out);
	int End();
protected:
	th_enc_ctx *td;
	th_info ti;

};

THEORACODEC::THEORACODEC()
{
}

int THEORACODEC::StartEncode(int width, int height, unsigned fourcc, unsigned framerate, unsigned quality)
{
	th_info_init(&ti);    
	ti.frame_width = ti.pic_width = width;
	ti.frame_height = ti.pic_height = height;
	ti.pic_x = ti.pic_y = 0;
	ti.fps_numerator = framerate;
	ti.fps_denominator = 1;
	ti.colorspace = TH_CS_UNSPECIFIED;
	ti.pixel_fmt = TH_PF_420;
	ti.quality = quality;
}

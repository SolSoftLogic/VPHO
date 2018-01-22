// cl /MT /DINCLUDEXVID /O2 codecstest.cpp /I\src\xvidcore\src \src\xvidcore\build\win32\bin\libxvidcore.lib
// cl /MT /O2 codecstest.cpp ..\avc_h264\win32\release\avc.lib

#include <stdio.h>
#include "../vpstack/portability.h"
#include "videocodecs.h"
#ifdef INCLUDEXVID
#include "xvid.cpp"
#endif

#ifdef _WIN32_WCE
extern FILE *fplog;
#else
#define fplog stdout
#endif

int main(int argc, char **argv)
{
#ifdef INCLUDEXVID
	VIDEOCODEC *codec = NewXVID();
#else
	VIDEOCODEC *codec = NewAVC();
#endif
	int x = 176, y = 144, fourcc = F_I420, bpp = 12, rc, quality = 0x35;
	unsigned len;
	int iskey;
	FILE *fpi, *fpo;
	static unsigned char buf1[640*480*3], buf2[1000000];

	if(argc != 4 && argc != 5 && argc != 7 && argc != 8)
	{
		fprintf(fplog, "Syntax: codecstest e/d <input> <output> [fmt [xres yres [quality-hex]]]\n");
		fprintf(fplog, "<fmt> can be I420,YV12,YUY2,UYVY,BGR24 or BGR16\n");
		fprintf(fplog, "Resolution is by default 176x144, <fmt> I420\n");
		return -1;
	}
	if(argc == 8)
		quality = strtoul(argv[7], 0, 16);
	if(argc == 7)
	{
		x = atoi(argv[5]);
		y = atoi(argv[6]);
	}
	if(argc > 4)
	{
		if(!stricmp(argv[4], "I420"))
		{
			fourcc = F_I420;
			bpp = 12;
		} else if(!stricmp(argv[4], "YV12"))
		{
			fourcc = F_YV12;
			bpp = 12;
		} else if(!stricmp(argv[4], "YUY2"))
		{
			fourcc = F_YUY2;
			bpp = 16;
		} else if(!stricmp(argv[4], "UYVY"))
		{
			fourcc = F_UYVY;
			bpp = 16;
		} else if(!stricmp(argv[4], "BGR24"))
		{
			fourcc = F_BGR24;
			bpp = 24;
		} else if(!stricmp(argv[4], "BGR16"))
		{
			fourcc = F_BGR16;
			bpp = 16;
		} else {
			fprintf(fplog, "Unsupported format\n");
			return -1;
		}
	}
	fpi = fopen(argv[2], "rb");
	fpo = fopen(argv[3], "wb");
	if(fpi && fpo)
	{
		if(argv[1][0] == 'e')
		{
			if(codec->StartEncode(x, y, fourcc, 10, quality))
			{
				fprintf(fplog, "StartEncode failed\n");
				return -1;
			}
			while(fread(buf1, x*y*bpp/8, 1, fpi))
			{
				if(codec->Encode(buf1, buf2, &len, &iskey))
				{
					fprintf(fplog, "Encode failed\n");
					return -1;
				}
				//fprintf(fpo, "%d %d\n", iskey, len);
				if(len)
				{
					fwrite(&len, 4, 1, fpo);
					fwrite(buf2, len, 1, fpo);
				}
			}
		}
		if(argv[1][0] == 'd')
		{
			if(codec->StartDecode(x, y, fourcc))
			{
				fprintf(fplog, "StartDecode failed\n");
				return -1;
			}
			while(fread(&len, 4, 1, fpi) && fread(buf1, len, 1, fpi))
			{
				rc = codec->Decode(buf1, len, buf2);
				if(rc)
				{
					fprintf(fplog, "Decode failed, rc=%d\n", rc);
					return -1;
				}
				fwrite(buf2, x*y*bpp/8, 1, fpo);
			}
		}
	}
	if(fpi)
		fclose(fpi);
	if(fpo)
		fclose(fpo);
	delete codec;
	return 0;
}

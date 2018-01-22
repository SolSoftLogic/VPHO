#include <string.h>
#include "portability.h"
#include "avifile.h"
#include "audiocodecs.h"

struct MainAVIHeader {
    DWORD  dwMicroSecPerFrame;
    DWORD  dwMaxBytesPerSec;
    DWORD  dwPaddingGranularity;
    DWORD  dwFlags;
    DWORD  dwTotalFrames;
    DWORD  dwInitialFrames;
    DWORD  dwStreams;
    DWORD  dwSuggestedBufferSize;
    DWORD  dwWidth;
    DWORD  dwHeight;
    DWORD  dwScale;
    DWORD  dwRate;
    DWORD  dwStart;
    DWORD  dwLength;
};

struct AVIStreamHeader {
    DWORD  fccType;
    DWORD  fccHandler;
    DWORD  dwFlags;
    DWORD  dwReserved1;
    DWORD  dwInitialFrames;
    DWORD  dwScale;
    DWORD  dwRate;
    DWORD  dwStart;
    DWORD  dwLength;
    DWORD  dwSuggestedBufferSize;
    DWORD  dwQuality;
    DWORD  dwSampleSize;
	WORD   left;
	WORD   top;
	WORD   right;
	WORD   bottom;
};

struct IDX {
	unsigned ckid, flags, offset, length;
} *idx;

struct LIST {
	unsigned fourcc, length, id;
};

struct CHUNK {
	unsigned fourcc, length;
};

struct AVIH {
	AVIH() { memset(this, 0, sizeof(*this)); }
	LIST avi;
	LIST hdrl;
	CHUNK avih;
	MainAVIHeader h;
};

struct VLIST {
	VLIST() { memset(this, 0, sizeof(*this)); }
	LIST list;
	CHUNK strh;
	AVIStreamHeader h;
	CHUNK strf;
	BITMAPINFOHEADER bih;
	CHUNK strn;
	char name[16];
};

struct ALIST {
	ALIST() { memset(this, 0, sizeof(*this)); }
	LIST list;
	CHUNK strh;
	AVIStreamHeader h;
	CHUNK strf;
	WAVEFORMATEX wf;
	BYTE filler[1500];
};

struct AVIFILE
{
	AVIFILE() { memset(this, 0, sizeof(*this)); }
	AVIDATA d;
	int f;
	unsigned vframes, aframes;
	unsigned maxvbuf, maxabuf;
	unsigned movilen, movipos;

	unsigned curapos, curvpos;
	struct IDX *idx;
	unsigned idxlen, idxalloc;
	bool write;
};

static int WriteHeader(AVIFILE *af)
{
	AVIH avih;
	VLIST vlist;
	ALIST alist;
	LIST movi;
	AVIDATA *ad = &af->d;
	int alistlen = sizeof(alist.list) + sizeof(alist.strh) + sizeof(alist.h) + sizeof(alist.strf) + ad->wflen;

	avih.avi.fourcc = mmioFOURCC('R','I','F','F');
	avih.avi.length = sizeof(avih) - 8 + sizeof(vlist) + alistlen + sizeof(movi) + af->movilen + sizeof(CHUNK) + af->idxlen * 16;
	avih.avi.id = mmioFOURCC('A','V','I',' ');

	avih.hdrl.fourcc = mmioFOURCC('L','I','S','T');
	avih.hdrl.length = sizeof(avih.avih) + sizeof(avih.h) + 4 + sizeof(vlist) + alistlen;
	avih.hdrl.id = mmioFOURCC('h','d','r','l');
	avih.avih.fourcc = mmioFOURCC('a','v','i','h');
	avih.avih.length = sizeof(avih.h);

	avih.h.dwMicroSecPerFrame = ad->framelen;
	avih.h.dwFlags = 0x810;	// AVIF_TRUSTCKTYPE | AVIF_HASINDEX
	avih.h.dwStreams = 2;
	avih.h.dwWidth = ad->width;
	avih.h.dwHeight = ad->height;
	avih.h.dwTotalFrames = af->vframes;

	avih.h.dwMaxBytesPerSec = af->maxvbuf * (1000000 / af->d.framelen) + af->maxabuf * (af->d.sfreq / af->d.blocksamples);
	avih.h.dwSuggestedBufferSize = af->maxvbuf > af->maxabuf ? af->maxvbuf : af->maxabuf;

	vlist.list.fourcc = mmioFOURCC('L','I','S','T');
	vlist.list.length = sizeof(vlist) - 8;
	vlist.list.id = mmioFOURCC('s','t','r','l');
	vlist.strh.fourcc = mmioFOURCC('s','t','r','h');
	vlist.strh.length = sizeof(vlist.h);
	vlist.h.fccType = mmioFOURCC('v','i','d','s');
	vlist.h.fccHandler = ad->fourcc;
	vlist.h.dwScale = 1;
	if(ad->framelen)
		vlist.h.dwRate = 1000000 / ad->framelen;
	else vlist.h.dwRate = 10;
	vlist.h.dwQuality = -1;
	vlist.h.dwLength = af->vframes;
	vlist.h.dwSuggestedBufferSize = af->maxvbuf;
	vlist.h.right = ad->width;
	vlist.h.bottom = ad->height;
	vlist.strf.fourcc = mmioFOURCC('s','t','r','f');
	vlist.strf.length = sizeof(vlist.bih);
	vlist.bih.biSize = sizeof(vlist.bih);
	vlist.bih.biWidth = ad->width;
	vlist.bih.biHeight = ad->height;
	vlist.bih.biPlanes = 1;
	vlist.bih.biBitCount = 0;
	vlist.bih.biCompression = ad->fourcc;
	vlist.strn.fourcc = mmioFOURCC('s','t','r','n');
	vlist.strn.length = 16;
	strcpy(vlist.name, "VP Videomessage");

	alist.list.fourcc = mmioFOURCC('L','I','S','T');
	alist.list.length = 2*8+4 + sizeof(alist.h) + ad->wflen;
	alist.list.id = mmioFOURCC('s','t','r','l');
	alist.strh.fourcc = mmioFOURCC('s','t','r','h');
	alist.strh.length = sizeof(alist.h);
	alist.h.fccType = mmioFOURCC('a','u','d','s');
	alist.h.dwScale = ad->blocksamples;
	alist.h.dwRate = ad->sfreq;
	alist.h.dwSampleSize = ad->blocklength;
	alist.h.dwSuggestedBufferSize = af->maxabuf;
	alist.h.dwLength = af->aframes;
	alist.strf.fourcc = mmioFOURCC('s','t','r','f');
	alist.strf.length = ad->wflen;
	memcpy(&alist.wf, ad->wf, ad->wflen);

	movi.fourcc = mmioFOURCC('L','I','S','T');
	movi.length = af->movilen + 4;
	movi.id = mmioFOURCC('m','o','v','i');

	writefile(af->f, &avih, sizeof(avih));
	writefile(af->f, &vlist, sizeof(vlist));
	writefile(af->f, &alist, alistlen);
	writefile(af->f, &movi, sizeof(movi));
	return 0;
}

void *AVICreate(const char *path, const AVIDATA *ad)
{
	int f;
	AVIFILE *af;

	if(!ad->framelen || !ad->blocklength || !ad->blocksamples)
		return 0;
	f = openfile(path, O_WRONLY | O_TRUNC | O_CREAT);
	if(f == -1)
		return 0;
	af = new AVIFILE;
	af->d = *ad;
	af->f = f;
	af->idxalloc = 1000;
	af->idx = (IDX *)malloc(af->idxalloc*16);
	af->write = true;
	WriteHeader(af);

	return af;
}

int AVIClose(void *h)
{
	AVIFILE *af = (AVIFILE *)h;
	CHUNK idx1;

	if(af->write)
	{
		if(af->vframes)
			af->d.framelen = (unsigned)(1000000.0 * af->aframes * af->d.blocksamples / af->d.sfreq / af->vframes);
		seekfile(af->f, 0, SEEK_SET);
		WriteHeader(af);
		seekfile(af->f, 0, SEEK_END);
		idx1.fourcc = mmioFOURCC('i','d','x','1');
		idx1.length = af->idxlen * 16;
		writefile(af->f, &idx1, 8);
		writefile(af->f, af->idx, af->idxlen * 16);
	}
	if(af->idx)
		free(af->idx);
	closefile(af->f);
	delete af;
	return 0;
}

int AVIWriteVideo(void *h, const void *buf, int buflen, int keyframe)
{
	AVIFILE *af = (AVIFILE *)h;
	unsigned hd[2];
	
	hd[0] = mmioFOURCC('0','0','d','c');
	hd[1] = buflen;
	af->idx[af->idxlen].ckid = hd[0];
	af->idx[af->idxlen].flags = keyframe ? 0x10: 0;
	af->idx[af->idxlen].offset = af->movilen + 4;
	af->idx[af->idxlen].length = buflen;
	buflen = (buflen+1)/2*2;
	writefile(af->f, hd, 8);
	writefile(af->f, buf, buflen);
	af->vframes++;
	if(buflen + 8 > (int)af->maxvbuf)
		af->maxvbuf = buflen + 8;
	af->idxlen++;
	af->movilen += 8 + buflen;
	if(af->idxlen == af->idxalloc)
	{
		af->idxalloc += 1000;
		af->idx = (IDX *)malloc(af->idxalloc * 16);
	}
	return 0;
}

int AVIWriteAudio(void *h, const void *buf, int buflen)
{
	AVIFILE *af = (AVIFILE *)h;
	unsigned hd[2];

	hd[0] = mmioFOURCC('0','1','w','b');
	hd[1] = buflen;
	af->idx[af->idxlen].ckid = hd[0];
	af->idx[af->idxlen].flags = 0x10;
	af->idx[af->idxlen].offset = af->movilen + 4;
	af->idx[af->idxlen].length = buflen;
	buflen = (buflen+1)/2*2;
	writefile(af->f, hd, 8);
	writefile(af->f, buf, buflen);
	if(buflen + 8 > (int)af->maxabuf)
		af->maxabuf = buflen + 8;
	af->aframes += buflen / af->d.blocklength;
	af->idxlen++;
	af->movilen += 8 + buflen;
	if(af->idxlen == af->idxalloc)
	{
		af->idxalloc += 1000;
		af->idx = (IDX *)malloc(af->idxalloc * 16);
	}
	return 0;
}

void *AVIOpen(const char *path, AVIDATA *ad)
{
	int f, br;
	BYTE buf[1500];;
	AVIFILE *af;
	AVIH avih;
	CHUNK chunk;
	AVIStreamHeader ash;

	f = openfile(path, O_RDONLY);
	if(f == -1)
		return 0;
	af = new AVIFILE;
	af->f = f;
	if(readfile(f, &avih, sizeof(avih)) < (int)sizeof(avih))
	{
		closefile(f);
		delete af;
		return 0;
	}
	if(avih.avi.fourcc != mmioFOURCC('R','I','F','F') || avih.avi.id != mmioFOURCC('A','V','I',' ') ||
		avih.hdrl.fourcc != mmioFOURCC('L','I','S','T') || avih.hdrl.id != mmioFOURCC('h','d','r','l') ||
		avih.avih.fourcc != mmioFOURCC('a','v','i','h'))
	{
		closefile(f);
		delete af;
		return 0;
	}
	br = sizeof(avih.avih) + avih.avih.length + 4;
	while(br < (int)avih.hdrl.length)
	{
		if(readfile(f, &chunk, sizeof(chunk)) < (int)sizeof(chunk))
		{
			closefile(f);
			delete af;
			return 0;
		}
		br += sizeof(chunk);
		if(chunk.fourcc == mmioFOURCC('L','I','S','T'))
		{
			unsigned br = 4, length = chunk.length;

			if(readfile(f, &chunk, 4) < 4)
			{
				closefile(f);
				delete af;
				return 0;
			}
			if(chunk.fourcc == mmioFOURCC('s','t','r','l'))
			{
				CHUNK chunk;
				bool ashread = false;

				while(br < length)
				{
					if(readfile(f, &chunk, sizeof(chunk)) < (int)sizeof(chunk))
					{
						closefile(f);
						delete af;
						return 0;
					}
					br += sizeof(chunk);
					if(chunk.fourcc == mmioFOURCC('s','t','r','h'))
					{
						if(readfile(f, buf, chunk.length) != (int)chunk.length)
						{
							closefile(f);
							delete af;
							return 0;
						}
						ashread = true;
						memcpy(&ash, buf, sizeof(ash));
						if(ash.dwRate)
							ad->lenms = MulDiv(ash.dwScale * 1000, ash.dwLength, ash.dwRate);
					} else if(chunk.fourcc == mmioFOURCC('s','t','r','f') && ashread)
					{
						if(readfile(f, buf, chunk.length) != (int)chunk.length)
						{
							closefile(f);
							delete af;
							return 0;
						}
						if(ash.fccType == mmioFOURCC('v','i','d','s'))
						{
							BITMAPINFOHEADER *bih = (BITMAPINFOHEADER *)buf;
							ad->fourcc = ash.fccHandler;
							ad->width = bih->biWidth;
							ad->height = bih->biHeight;
							ad->framelen = (unsigned)(1000000.0 * ash.dwScale / ash.dwRate);
						} else if(ash.fccType == mmioFOURCC('a','u','d','s'))
						{
							WAVEFORMATEX *wf = (WAVEFORMATEX *)buf;
							ad->channels = wf->nChannels;
							ad->sfreq = wf->nSamplesPerSec;
							ad->wflen = chunk.length;
							memcpy(ad->wf, buf, ad->wflen);
						}
					} else seekfile(f, (chunk.length+1)/2*2, SEEK_CUR);
					br += (chunk.length + 1) / 2 * 2;
				}
			} else seekfile(af->f, length - 4, SEEK_CUR);
		} else seekfile(f, (chunk.length+1)/2*2, SEEK_CUR);
		br += (chunk.length + 1) / 2 * 2;
	}
	br += 12;
	while(br < (int)avih.avi.length)
	{
		if(readfile(f, &chunk, sizeof(chunk)) < (int)sizeof(chunk))
		{
			closefile(f);
			delete af;
			return 0;
		}
		br += sizeof(chunk);
		if(chunk.fourcc == mmioFOURCC('L','I','S','T'))
		{
			if(readfile(f, &chunk, 4) < 4)
			{
				closefile(f);
				delete af;
				return 0;
			}
			if(chunk.fourcc == mmioFOURCC('m','o','v','i'))
			{
				af->movipos = seekfile(af->f, 0, SEEK_CUR);
				af->movilen = chunk.length - 4;
				seekfile(af->f, chunk.length - 4, SEEK_CUR);
			}
		} else if(chunk.fourcc == mmioFOURCC('i','d','x','1'))
		{
			af->idxlen = af->idxalloc = chunk.length / 16;
			af->idx = (IDX *)malloc(af->idxlen * 16);
			if(readfile(af->f, af->idx, af->idxlen * 16) != (int)af->idxlen * 16)
			{
				closefile(f);
				delete af;
				return 0;
			}
		} else seekfile(f, (chunk.length+1)/2*2, SEEK_CUR);
		br += (chunk.length+1)/2*2;
	}
	af->d = *ad;
	return af;
}

int AVIReadVideo(void *h, void *buf, unsigned *buflen)
{
	AVIFILE *af = (AVIFILE *)h;
	CHUNK chunk;
	int len;

	if(af->curvpos >= af->movilen)
		return -1;
	if(seekfile(af->f, af->curvpos + af->movipos, SEEK_SET) == -1)
		return -1;
	while(af->curvpos < af->movilen)
	{
		if(readfile(af->f, &chunk, 8) != 8)
			return -1;
		len = (chunk.length + 1) / 2 * 2;
		af->curvpos += 8;
		if(chunk.fourcc == mmioFOURCC('0','0','d','c') || chunk.fourcc == mmioFOURCC('0','0','d','b'))
		{
			if(readfile(af->f, buf, len) != len)
			{
				af->curvpos = af->movilen;
				return -1;
			}
			*buflen = chunk.length;
			af->curvpos += len;
			return 0;
		} else {
			seekfile(af->f, len, SEEK_CUR);
			af->curvpos += len;
		}
	}
	return -1;
}

int AVIReadAudio(void *h, void *buf, unsigned *buflen)
{
	AVIFILE *af = (AVIFILE *)h;
	CHUNK chunk;
	int len;

	if(af->curapos >= af->movilen)
		return -1;
	if(seekfile(af->f, af->curapos + af->movipos, SEEK_SET) == -1)
		return -1;
	while(af->curapos < af->movilen)
	{
		if(readfile(af->f, &chunk, 8) != 8)
			return -1;
		af->curapos += 8;
		len = (chunk.length + 1) / 2 * 2;
		if(chunk.fourcc == mmioFOURCC('0','1','w','b'))
		{
			if(readfile(af->f, buf, len) != len)
			{
				af->curapos = af->movilen;
				return -1;
			}
			*buflen = chunk.length;
			af->curapos += len;
			return 0;
		} else {
			seekfile(af->f, len, SEEK_CUR);
			af->curapos += len;
		}
	}
	return -1;
}

/*void TestAVI()
{
	AVIDATA ad;
	void *h;
	BYTE buf[30000];
	unsigned buflen;

	h = AVIOpen("C:\\Documents and Settings\\vitez\\My Documents\\Vphonet\\Received video messages\\2010-04-27 _ 17-33-53 _ 2521822701.avi", &ad);
	if(h)
	{
		AVIReadAudio(h, buf, &buflen);
		AVIReadAudio(h, buf, &buflen);
		AVIReadAudio(h, buf, &buflen);
		AVIReadAudio(h, buf, &buflen);
		AVIReadVideo(h, buf, &buflen);
		AVIReadVideo(h, buf, &buflen);
		AVIReadVideo(h, buf, &buflen);
		AVIReadVideo(h, buf, &buflen);
		AVIClose(h);
	}
}*/

struct ACM_WFC {
	WAVEFORMATEX wf;
	unsigned char extra[114];
} acm_wfc[12] =
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

int Codec2AD(int codec, AVIDATA *ad)
{
	if(codec < 0 || codec > 11)
		return -1;
	ad->channels = 1;
	ad->sfreq = codecsfreq[codec];
	ad->blocklength = codeccframelen[codec];
	ad->blocksamples = codecuframelen[codec] / 2;
	ad->wflen = sizeof(WAVEFORMATEX) + ((WAVEFORMATEX *)&acm_wfc[codec])->cbSize;
	memcpy(ad->wf, &acm_wfc[codec], ad->wflen);
	return 0;
}

int Codec2WF(int codec, WAVEFMT *wf)
{
	if(codec < 0 || codec > 11)
		return -1;
	memcpy(wf, &acm_wfc[codec], sizeof(WAVEFORMATEX) + ((WAVEFORMATEX *)&acm_wfc[codec])->cbSize);
	return 0;
}

int WF2Codec(const WAVEFMT *wf)
{
	int i;

	for(i = 0; i < 12; i++)
		if(!memcmp(wf, &acm_wfc[i], sizeof(WAVEFORMATEX)))
			return i;
	return -1;
}

struct WAVHDR
{
	LIST wavhdr;
	CHUNK fmt;
	WAVEFORMATEX wf;
	BYTE extra[256];
};

struct WAVFILE
{
	WAVHDR hdr;
	unsigned length, n;
	int f;
	bool write;
};

void *WAVCreate(const char *path, const WAVEFMT *wf)
{
	WAVHDR hdr;
	CHUNK dc;
	WAVFILE *w;
	int f;
	
	f = openfile(path, O_WRONLY | O_CREAT | O_TRUNC);
	if(f == -1)
		return 0;
	hdr.wavhdr.fourcc = mmioFOURCC('R','I','F','F');
	hdr.wavhdr.length = 0;
	hdr.wavhdr.id = mmioFOURCC('W','A','V','E');
	hdr.fmt.fourcc = mmioFOURCC('f','m','t',' ');
	hdr.fmt.length = sizeof(*wf) + wf->cbSize;
	memcpy(&hdr.wf, wf, sizeof(*wf) + wf->cbSize);
	dc.fourcc = mmioFOURCC('d','a','t','a');
	dc.length = 0;
	writefile(f, &hdr, sizeof(LIST)+sizeof(CHUNK)+hdr.fmt.length);
	writefile(f, &dc, sizeof(dc));
	w = new WAVFILE;
	w->f = f;
	w->hdr = hdr;
	w->n = 0;
	w->write = true;
	return (void *)w;
}

void *WAVOpen(const char *path, WAVEFMT *wf)
{
	WAVHDR hdr;
	CHUNK dc;
	WAVFILE *w;

	int f = openfile(path, O_RDONLY);
	if(f == -1)
		return 0;
	if(readfile(f, &hdr, sizeof(LIST)+sizeof(CHUNK)) != sizeof(LIST)+sizeof(CHUNK))
	{
		closefile(f);
		return 0;
	}
	if(hdr.wavhdr.fourcc != mmioFOURCC('R','I','F','F') || hdr.wavhdr.id != mmioFOURCC('W','A','V','E') ||
		hdr.fmt.fourcc != mmioFOURCC('f','m','t',' '))
	{
		closefile(f);
		return 0;
	}
	if(hdr.fmt.length > sizeof(WAVEFORMATEX) + 256)
	{
		closefile(f);
		return 0;
	}
	if(readfile(f, &hdr.wf, hdr.fmt.length) != (int)hdr.fmt.length)
	{
		closefile(f);
		return 0;
	}
	memcpy(wf, &hdr.wf, hdr.fmt.length);
	if(readfile(f, &dc, sizeof(dc)) != sizeof(dc))
	{
		closefile(f);
		return 0;
	}
	if(dc.fourcc != mmioFOURCC('d','a','t','a'))
	{
		closefile(f);
		return 0;
	}
	w = new WAVFILE;
	w->f = f;
	w->hdr = hdr;
	w->write = false;
	w->n = 0;
	w->length = dc.length;
	return w;
}

int WAVRead(void *h, void *buf, unsigned *buflen)
{
	WAVFILE *w = (WAVFILE *)h;
	int rc;
	
	if(w->hdr.wf.wFormatTag == 1)
		*buflen = w->hdr.wf.nSamplesPerSec / 25 * 2 * w->hdr.wf.nChannels;
	else *buflen = w->hdr.wf.nBlockAlign;
	if(*buflen + w->n > w->length)
		return -1;
	rc = readfile(((WAVFILE *)h)->f, buf, *buflen);
	if(rc != (int)*buflen)
		return -1;
	((WAVFILE *)h)->n += rc;
	return 0;
}

int WAVWrite(void *h, const void *buf, unsigned buflen)
{
	if(writefile(((WAVFILE *)h)->f, buf, buflen) != (int)buflen)
		return -1;
	((WAVFILE *)h)->n += buflen;
	return 0;
}

int WAVClose(void *h)
{
	WAVFILE *w = (WAVFILE *)h;
	if(w->write)
	{
		seekfile(w->f, 0, SEEK_SET);
		w->hdr.wavhdr.length = 4 + sizeof(w->hdr.fmt) + w->hdr.fmt.length + 8 + w->n;
		writefile(w->f, &w->hdr, sizeof(w->hdr.wavhdr));
		seekfile(w->f, sizeof(w->hdr.wavhdr) + sizeof(w->hdr.fmt) + w->hdr.fmt.length + 4, SEEK_SET);
		writefile(w->f, &w->n, 4);
	}
	closefile(w->f);
	return 0;
}

int WAVSeek(void *h, unsigned pos)
{
	WAVFILE *w = (WAVFILE *)h;

	if(w->write)
		return -1;
	w->n = pos;
	return seekfile(w->f, sizeof(w->hdr.wavhdr) + sizeof(w->hdr.fmt) + w->hdr.fmt.length + 8 + pos, SEEK_SET);
}

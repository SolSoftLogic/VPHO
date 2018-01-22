#ifndef _AVIFILE_H_INCLUDED
#define _AVIFILE_H_INCLUDED

#include "vpstack.h"

struct AVIDATA
{
	// Video
	unsigned fourcc, framelen, width, height;
	// Audio
	int channels, sfreq, blocklength, blocksamples, wflen;
	unsigned char wf[1500];
	unsigned lenms;	// Only used in AVIOpen
};

struct WAVEFMT
{
    unsigned short        wFormatTag;         /* format type */
    unsigned short        nChannels;          /* number of channels (i.e. mono, stereo...) */
    unsigned long         nSamplesPerSec;     /* sample rate */
    unsigned long         nAvgBytesPerSec;    /* for buffer estimation */
    unsigned short        nBlockAlign;        /* block size of data */
    unsigned short        wBitsPerSample;     /* number of bits per sample of mono data */
    unsigned short        cbSize;             /* the count in bytes of the size of */
    /* extra information (after cbSize) */
};

void *AVICreate(const char *path, const AVIDATA *ad);
int AVIClose(void *h);
int AVIWriteVideo(void *h, const void *buf, int buflen, int keyframe);
int AVIWriteAudio(void *h, const void *buf, int buflen);
void *AVIOpen(const char *path, AVIDATA *ad);
int AVIReadVideo(void *h, void *buf, unsigned *buflen);
int AVIReadAudio(void *h, void *buf, unsigned *buflen);
int Codec2AD(int codec, AVIDATA *ad);
int Codec2WF(int codec, WAVEFMT *wf);
int WF2Codec(const WAVEFMT *wf);

void *WAVCreate(const char *path, const WAVEFMT *wf);
void *WAVOpen(const char *path, WAVEFMT *wf);
int WAVRead(void *h, void *buf, unsigned *buflen);
int WAVWrite(void *h, const void *buf, unsigned buflen);
int WAVSeek(void *h, unsigned pos);
int WAVClose(void *h);

#endif

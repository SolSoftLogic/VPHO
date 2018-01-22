#ifndef _AUDIOUTIL_H_INCLUDED_
#define _AUDIOUTIL_H_INCLUDED_

class RESAMPLER {
public:
	RESAMPLER() { Init(2); srcsf = 8000; destsf = 16000; }
	int Init(int factor);
	int UpSample(short *in, int nsamples, short *out);
	int DownSample(short *in, int nsamples, short *out);
	int Resample(int from, int to, short *in, int nsamples, short *out);	// returns the number of output samples
protected:
	short buf[12000];
	short *filter;
	int factor, filtlen;
	int srcsf, destsf;
};

class AUDIOBUFFER {
public:
	AUDIOBUFFER();
	~AUDIOBUFFER();
	void Set(int nsamples);
	void Reset();
	unsigned SamplesInBuffer();
	int PutData( short *buf, unsigned nsamples);
	int PeekData(unsigned *fromsample, short *buf, unsigned nsamples);	// fromsample = 0 means from first available; fromsample will point to the true fromsample
	int PeekDataMix(unsigned fromsample, short *buf, unsigned nsamples);	// fromsample = 0 means from first available
	unsigned Advance(unsigned sample);	// advance to sample
protected:
	CRITICAL_SECTION cs;
	short *buffer;
	unsigned tail, head, length;
	unsigned startsample;
};

#endif
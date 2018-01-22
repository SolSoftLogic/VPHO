#include "portability.h"
#include "audioutil.h"
#include "filters.h"

int RESAMPLER::Init(int factor)
{
	memset(buf, 0, sizeof(buf));
	this->factor = factor;
	filter = filters[factor / 2 - 1];
	filtlen = filterslen[factor / 2 - 1];
	return 0;
}

int RESAMPLER::UpSample(short *in, int nsamples, short *out)
{
	int i, filtj, signalj, len, sample, margin;

	margin = (filtlen + factor - 1) / factor;
	memcpy(buf + margin, in, 2 * nsamples);
	len = nsamples * factor;
	for(i = 0; i < len; i++)
	{
		sample = 0;
		for(filtj = i % factor, signalj = i / factor + margin; filtj < filtlen; signalj--, filtj += factor)
			sample += filter[filtj] * buf[signalj] / 512;
		sample = (int)(sample / 66);
		if(sample < -32768)
			sample = -32768;
		else if(sample > 32767)
			sample = 32767;
		out[i] = (short)sample;
	}
	memmove(buf, buf + nsamples, margin * 2);
	return 0;
}

int RESAMPLER::DownSample(short *in, int nsamples, short *out)
{
	int len, sample, i, j;

	memcpy(buf + filtlen - 1, in, nsamples * 2);
	len = nsamples / factor;
	for(i = 0; i < len; i++)
	{
		sample = 0;
		for(j = 0; j < filtlen; j++)
			sample += filter[j] * buf[factor * i + filtlen - 1 - j] / 512;
		sample = (int)(sample / (factor * 65));
		if(sample < -32768)
			sample = -32768;
		else if(sample > 32767)
			sample = 32767;
		out[i] = (short)sample;
	}
	memmove(buf, buf + nsamples, (filtlen - 1) * 2);
	return 0;
}

int RESAMPLER::Resample(int from, int to, short *in, int nsamples, short *out)
{
	if(from == to)
	{
		memcpy(out, in, 2 * nsamples);
		return nsamples;
	}
	if(srcsf != from || destsf != to)
	{
		srcsf = from;
		destsf = to;
		if(srcsf > destsf)
			Init(srcsf / destsf);
		else Init(destsf / srcsf);
	}
	if(srcsf > destsf)
	{
		DownSample(in, nsamples, out);
		return nsamples / (srcsf / destsf);
	} else {
		UpSample(in, nsamples, out);
		return nsamples * (destsf / srcsf);
	}
}

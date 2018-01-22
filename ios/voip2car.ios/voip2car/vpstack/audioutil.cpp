#include "portability.h"
#include "audioutil.h"

AUDIOBUFFER::AUDIOBUFFER()
{
	InitializeCriticalSection(&cs);
	Reset();
	buffer = 0;
	Set(32000);
}

void AUDIOBUFFER::Set(int nsamples)
{
	if(buffer)
		delete[] buffer;
	buffer = new short [nsamples];
	length = nsamples;
	head = tail = startsample = 0;
}

AUDIOBUFFER::~AUDIOBUFFER()
{
	delete[] buffer;
	DeleteCriticalSection(&cs);
}

void AUDIOBUFFER::Reset()
{
	EnterCriticalSection(&cs);
	tail = head = startsample = 0;
	LeaveCriticalSection(&cs);
}

unsigned AUDIOBUFFER::SamplesInBuffer()
{
	return (tail - head + length) % length;
}

int AUDIOBUFFER::PutData(short *buf, unsigned nsamples)
{
	EnterCriticalSection(&cs);
	if((tail + 1) % nsamples == head)
	{
		LeaveCriticalSection(&cs);
		return -1;
	}
	if(head != tail && (head - tail + length) % length <= nsamples)
		nsamples = (head - tail - 1 + length) % length;
	if(tail + nsamples > length)
	{
		memcpy(buffer + tail, buf, (length - tail) * 2);
		memcpy(buffer, buf + length - tail, (nsamples - (length - tail)) * 2);
	} else memcpy(buffer + tail, buf, nsamples * 2);
	tail = (tail + nsamples) % length;
	LeaveCriticalSection(&cs);
	return 0;
}

int AUDIOBUFFER::PeekData(unsigned *fromsample, short *buf, unsigned nsamples)
{
	unsigned offs;
	unsigned head2;

	EnterCriticalSection(&cs);
	if(!*fromsample)
		*fromsample = startsample;
	if(*fromsample - startsample >= 0x80000000)	// Wants to take before start
		*fromsample = startsample;
	offs = *fromsample - startsample;
	if((tail - head + length) % length < offs + nsamples)	// Not enough data in buffer
	{
		LeaveCriticalSection(&cs);
		return -1;
	}
	head2 = (head + offs) % length;
	if(head2 + nsamples > length)
	{
		memcpy(buf, buffer + head2, (length - head2) * 2);
		memcpy(buf + (length - head2), buffer, (nsamples - (length - head2)) * 2);
	} else memcpy(buf, buffer + head2, nsamples * 2);
	LeaveCriticalSection(&cs);
	return 0;
}

int AUDIOBUFFER::PeekDataMix(unsigned fromsample, short *buf, unsigned nsamples)
{
	unsigned ps = nsamples, i, offs, head2;
	int sum, ptr;

	EnterCriticalSection(&cs);
	if(!fromsample)
		fromsample = startsample;
	if(fromsample - startsample >= 0x80000000)	// Wants to take before start
		fromsample = startsample;
	offs = fromsample - startsample;
//	char s[300]; sprintf(s, "fromsample=%u startsample=%u ps=%u tail=%u head=%u diff=%d len=%d\n", fromsample, startsample, ps, tail, head, fromsample - startsample, (tail - head + length) % length); OutputDebugString(s);
	if((tail - head + length) % length <= offs)	// Offset behind buffer
	{
//		OutputDebugString("fail\n");
		LeaveCriticalSection(&cs);
		return -1;
	}
	head2 = (head + offs) % length;
	if((tail - head2 + length) % length < ps)
		ps = (tail - head2 + length) % length;
	ptr = head2;
	for(i = 0; i < ps; i++)
	{
		sum = buf[i] + buffer[ptr];
		if(sum > 32767)
			buf[i] = 32767;
		else if(sum < -32768)
			buf[i] = -32768;
		else buf[i] = sum;
		ptr = (ptr + 1) % length;
	}
	LeaveCriticalSection(&cs);
	return 0;
}

unsigned AUDIOBUFFER::Advance(unsigned sample)
{
	unsigned ps;

	EnterCriticalSection(&cs);
	ps = sample - startsample;
	if((tail - head + length) % length < ps)
		ps = (tail - head + length) % length;
	head = (head + ps) % length;
	startsample = sample;	// Always advances to sample, to be keep the buffers synchronized
	LeaveCriticalSection(&cs);
	return sample;
}

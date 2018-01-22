/*
 *  audio.cpp
 *  vpstack
 *
 *  Created by uncle on 22.01.11.
 *  Copyright 2011. All rights reserved.
 *
 */
#include "AudioUnit/AudioUnit.h"

#include <iostream>

#include "iosportability.h"
#include "../util.h"
#include "vpaudio.h"
#include "tone_generator.h"


// Determine the size, in bytes, of a buffer necessary to represent the supplied number
// of seconds of audio data.
static int compute_record_buffer_bytes_size(const AudioStreamBasicDescription *format, AudioQueueRef queue, float seconds)
{
    int packets, frames, bytes;
    
    frames = (int)ceil(seconds * format->mSampleRate);
    
    if (format->mBytesPerFrame > 0)
        bytes = frames * format->mBytesPerFrame;
    else {
        UInt32 maxPacketSize;
        if (format->mBytesPerPacket > 0)
            maxPacketSize = format->mBytesPerPacket;    // constant packet size
        else {
            UInt32 propertySize = sizeof(maxPacketSize); 
            AudioQueueGetProperty(queue, kAudioConverterPropertyMaximumOutputPacketSize, &maxPacketSize, &propertySize);
        }
        if (format->mFramesPerPacket > 0)
            packets = frames / format->mFramesPerPacket;
        else
            packets = frames;   // worst-case scenario: 1 frame in a packet
        if (packets == 0)       // sanity check
            packets = 1;
        bytes = packets * maxPacketSize;
    }
    return bytes;
}


// Callback function from 
void __handle_output_buffer(void *userData, AudioQueueRef AQueue, AudioQueueBufferRef ABuffer) 
{
	AUDIOOUT* player = static_cast<AUDIOOUT*>(userData);

	if (player)
	{
		player->AddPlayedSamples(ABuffer->mAudioDataByteSize / 2);
	}
}

AUDIOOUT::AUDIOOUT() :	m_bPlaying(false), 
						m_MaxBufferSize(0), 
						m_PlayedSamples(0), 
						m_RecievedSamples(0), 
						m_bSpeaker(false)
{ 
	pthread_mutex_init(&mutex, NULL); 
};

AUDIOOUT::~AUDIOOUT() 
{ 
	Stop();  
	pthread_mutex_destroy(&mutex);
};


void AUDIOOUT::AddPlayedSamples(UInt32 ALen)
{
	pthread_mutex_lock(&mutex);
	m_PlayedSamples += ALen;
	pthread_mutex_unlock(&mutex);
}



bool AUDIOOUT::Start(int channels, int freq)
{
    OSStatus err = noErr;
	
	if (m_bPlaying) {
		return true;
	}
	
	// Set audio category
	UInt32 category = kAudioSessionCategory_PlayAndRecord; 
	err = AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(category), &category);
	if (err) 
	{
		//NSLog(@"ERROR:kAudioSessionProperty_AudioCategory: %@", err);
		return false;
	}
	
    // Create the audio stream description
    m_AudioDesc.mSampleRate		= freq;
    m_AudioDesc.mFormatID			= kAudioFormatLinearPCM;
    m_AudioDesc.mFormatFlags		= kAudioFormatFlagIsSignedInteger | kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked; 
    m_AudioDesc.mChannelsPerFrame = channels;
    m_AudioDesc.mFramesPerPacket	= 1;
    m_AudioDesc.mBitsPerChannel	= 16;
    m_AudioDesc.mBytesPerPacket	= m_AudioDesc.mBitsPerChannel / 8 * m_AudioDesc.mChannelsPerFrame;
    m_AudioDesc.mBytesPerFrame	= m_AudioDesc.mBytesPerPacket * m_AudioDesc.mFramesPerPacket;
    m_AudioDesc.mReserved			= 0;
	
    // Create the playback audio queue
    err = AudioQueueNewOutput(&m_AudioDesc,
                              __handle_output_buffer,
                              this,
                              NULL, 
                              NULL,
                              0,
                              &m_AudioQueue);
	
	
	m_MaxBufferSize = compute_record_buffer_bytes_size(&m_AudioDesc, m_AudioQueue, 0.01 /* 10 ms */);

    for(int i = 0; i < MAXPLAYBUFFERS; i++) {
        // Create the buffer for the queue
        err = AudioQueueAllocateBuffer(m_AudioQueue, m_MaxBufferSize, &m_AudioBuffers[i]);
        if (err) 
		{
			//DLog("Can't allocate audio player buffer");
            return false;
		}
        
        // Clear the data
		memset(m_AudioBuffers[i]->mAudioData, 0, m_MaxBufferSize);
        m_AudioBuffers[i]->mAudioDataByteSize = m_MaxBufferSize;
    }
	
	m_CurrentBuffer = 0;
	m_RecievedSamples = m_PlayedSamples = 0;
	
	m_bPlaying = true;	
    err = AudioQueueStart(m_AudioQueue, NULL);
	if (err)
	{
		//DLog("Can't  AudioQueueStart");	
		return false;
	}
	
	 
	return m_bPlaying;
	
}

void AUDIOOUT::SetSpeaker(bool ASpeaker)
{
	
	m_bSpeaker = ASpeaker;
	UInt32 audioRouteOverride =  ASpeaker ? kAudioSessionOverrideAudioRoute_Speaker : kAudioSessionOverrideAudioRoute_None; 
	AudioSessionSetProperty(kAudioSessionProperty_OverrideAudioRoute,                         
							sizeof (audioRouteOverride),                                      
							&audioRouteOverride);
}

bool AUDIOOUT::IsSpeaker()
{
	return m_bSpeaker;
}

int AUDIOOUT::PutData(int channels, int sfreq, short *buf, int len)	
{
	OSStatus err = noErr;

	if (Start(channels,sfreq))
	{
		memcpy(m_AudioBuffers[m_CurrentBuffer]->mAudioData, (char*) buf, MIN(m_AudioBuffers[m_CurrentBuffer]->mAudioDataByteSize, len));
		m_AudioBuffers[m_CurrentBuffer]->mAudioDataByteSize = MIN(m_AudioBuffers[m_CurrentBuffer]->mAudioDataByteSize, len);
		
		err = AudioQueueEnqueueBuffer(m_AudioQueue, m_AudioBuffers[m_CurrentBuffer], 0, NULL);
		
		m_CurrentBuffer++;
		if(m_CurrentBuffer == MAXPLAYBUFFERS)
			m_CurrentBuffer = 0;
		
		m_RecievedSamples += len / 2;
	}

	return 0;
}

int AUDIOOUT::Stop()
{
	//DLog("Can't player AudioQueueStop");
	OSStatus ret = 0;
	
	if (m_bPlaying)
	{
		ret = AudioQueueStop(m_AudioQueue, false);
		if (ret)
		{
	//		DLog("Can't player AudioQueueStop");
		}
		
        if (m_AudioQueue) 
		{
			for(int i = 0; i < MAXPLAYBUFFERS; i++)
			{
				AudioQueueFreeBuffer(m_AudioQueue, m_AudioBuffers[i]);
				m_AudioBuffers[i] = NULL;
			}
			
            AudioQueueDispose(m_AudioQueue, true);
			m_AudioQueue = NULL;
        }
	
	}
	
	m_bPlaying = false;
	return 0;
}

int AUDIOOUT::GetCurrentDelay()
{
	return -1;
}

int AUDIOOUT::SetVolume(int thousands)
{
	return 0;
}

int AUDIOOUT::VuMeter()
{
	return 0;
}

int AUDIOOUT::SetAudioDevice(TCHAR *sdev)
{
	return -1;
}



/*************************** I N P U T ****************************************/
// Callback audion caprure
void __handle_input_buffer (void *userData, AudioQueueRef queue, AudioQueueBufferRef ABuffer, const AudioTimeStamp *start_time, UInt32 number_packet_descriptions, const AudioStreamPacketDescription *packet_descriptions ) {
	
		//OSStatus err = noErr;
	AUDIOIN* recorder = static_cast<AUDIOIN*>(userData);
	
	if (recorder) {
		recorder->MakeBufferReady(ABuffer);
	}
}

AUDIOIN::AUDIOIN() : m_bRecording(false), m_TimeStamp(0), vumeter(0), rframes(0), m_CurrentBuffer(0), noiselevel(0), threshold(0), silencedetected(0),
					m_bMute(false)

{ 
	pthread_mutex_init(&mutex, NULL); 
	pthread_mutex_init(&mutexSound, NULL); 
} 

AUDIOIN::~AUDIOIN() 
{ 
	Stop();
	pthread_mutex_destroy(&mutex);
	pthread_mutex_destroy(&mutexSound); 
	
}

void AUDIOIN::MakeBufferReady(AudioQueueBufferRef ABuffer)
{
	pthread_mutex_lock(&mutex);
	ABuffer->mUserData = (void*)1; // buffer is ready
	if (IsMute())
	  memset(ABuffer->mAudioData, 0, ABuffer->mAudioDataByteSize);
	pthread_mutex_unlock(&mutex);
}

bool AUDIOIN::IsMute()
{
	bool result = false;
	pthread_mutex_lock(&mutexSound);
	result = m_bMute;
	pthread_mutex_unlock(&mutexSound);
	return result;
}

void AUDIOIN::SetMute(bool AMute)
{
	pthread_mutex_lock(&mutexSound);
	m_bMute = AMute;
	pthread_mutex_unlock(&mutexSound);
}

int AUDIOIN::Start(int channels, int sfreq, int framelen, int buflength)	// in Hz, bytes, milliseconds
{
	OSStatus ret = 0;
	
	if (m_bRecording) {
		return true;
	}
	
	// Set audio category
	UInt32 category = kAudioSessionCategory_PlayAndRecord;
	AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(category), &category);
	
	
    // Initialize the audio stream description
    m_AudioDesc.mSampleRate		= sfreq;//sfreq;
    m_AudioDesc.mFormatID			= kAudioFormatLinearPCM;
    m_AudioDesc.mFormatFlags		= kAudioFormatFlagIsSignedInteger | kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked;
    m_AudioDesc.mChannelsPerFrame = 1;//channels;
    m_AudioDesc.mFramesPerPacket	= 1;
    m_AudioDesc.mBitsPerChannel	= 16;
    m_AudioDesc.mBytesPerPacket	= m_AudioDesc.mBitsPerChannel / 8 * m_AudioDesc.mChannelsPerFrame;
    m_AudioDesc.mBytesPerFrame	= m_AudioDesc.mBytesPerPacket;
    m_AudioDesc.mReserved = 0;
	
    // Create the record audio queue
    ret = AudioQueueNewInput(&m_AudioDesc,
							 __handle_input_buffer,
							 this,
							 NULL, 
							 kCFRunLoopCommonModes,
							 0,
							 &m_AudioQueue);
	
	
    m_MaxBufferSize = compute_record_buffer_bytes_size(&m_AudioDesc, m_AudioQueue, 0.01 /* ms */);		// Dublicate

	if (ret) 
	{
		//DLog("Create AudioQueueNewInput false");	
		return false;
	}
	
    
    for(int i = 0; i < MAXRECORDBUFFERS; i++) 
	{
        // Create the buffer for the queue
        ret = AudioQueueAllocateBuffer(m_AudioQueue, m_MaxBufferSize, &m_AudioBuffers[i]);
        if (ret) 
		{
			//DLog("Can't allocate audio recorder buffer");
			return false;
		}
        
        // Enqueue the buffer
		ret = AudioQueueEnqueueBuffer(m_AudioQueue, m_AudioBuffers[i], 0, NULL);
        if (ret) 
		{
			//DLog("Can't enqueue the recorder buffer");	
			return false;
		}
    }
	m_CurrentBuffer = 0;
	m_TimeStamp = GetTickCount();
	
	hpfoutmem[0] = hpfoutmem[1] = hpfinmem[0] = hpfinmem[1] = 0;
	rframes = 0;
	es_refenergy = 0;
	es_inputenergy = 0;
	vu_level = -100;
	
	
	m_bRecording = true;
	ret = AudioQueueStart(m_AudioQueue, NULL);
	if (ret)
	{
		//DLog("Can't recorder AudioQueueStart");
		return false;
	}
	
	return m_bRecording;
}

int AUDIOIN::Stop()
{
	OSStatus ret = 0;
	m_bRecording = false;
	
	if (m_bRecording)
	{
		ret = AudioQueueStop(m_AudioQueue, false);
		if (ret)
		{
			//DLog("AUDIOIN:: Can't AudioQueueStop");
		}
		
        if (m_AudioQueue) 
		{
			for(int i = 0; i < MAXRECORDBUFFERS; i++)
			{
				AudioQueueFreeBuffer(m_AudioQueue, m_AudioBuffers[i]);
				m_AudioBuffers[i] = NULL;
			}
			
            AudioQueueDispose(m_AudioQueue, true);
			m_AudioQueue = NULL;
        }
	}
	m_bRecording = false;
	return 0;
}



static void HPFilter(short *in, short *out, int len, int *inbuf, double *outbuf)
{
	// butter(2,0.0002,'high');
	static const double coefB[3] = {0.99955581038761, -1.99911162077522, 0.99955581038761};
	static const double coefA[2] = {-1.99911142347080, 0.99911181807964};
	int i, yi;
	double y;
	
	y = coefB[0] * in[0] + coefB[1] * inbuf[0] + coefB[2] * inbuf[1] - coefA[0] * outbuf[0] - coefA[1] * outbuf[1];
	yi = (int)y;
	out[0] = yi > 32767 ? 32767 : yi < -32678 ? -32768 : yi;
	outbuf[1] = outbuf[0];
	outbuf[0] = y;
	y = coefB[0] * in[1] + coefB[1] * in[0] + coefB[2] * inbuf[0] - coefA[0] * outbuf[0] - coefA[1] * outbuf[1];
	yi = (int)y;
	out[1] = yi > 32767 ? 32767 : yi < -32678 ? -32768 : yi;
	outbuf[1] = outbuf[0];
	outbuf[0] = y;
	for(i = 2; i < len; i++)
	{
		y = coefB[0] * in[i] + coefB[1] * in[i - 1] + coefB[2] * in[i - 2] - coefA[0] * outbuf[0] - coefA[1] * outbuf[1];
		yi = (int)y;
		out[i] = yi > 32767 ? 32767 : yi < -32678 ? -32768 : yi;
		outbuf[1] = outbuf[0];
		outbuf[0] = y;
	}
	inbuf[1] = in[len - 2];
	inbuf[0] = in[len - 1];
}

#define swap(a) (a << 8) + (a >> 8);

int AUDIOIN::GetData(short* data)
{
	OSStatus err = noErr;

	if (!m_bRecording)
		return AUDIOERR_FATAL;
	
	
	pthread_mutex_lock(&mutex);
	
	if (!m_AudioBuffers[m_CurrentBuffer]->mUserData)
	{
		pthread_mutex_unlock(&mutex);
		return -1;
	}
	
	UInt32 ioBytes = m_AudioBuffers[m_CurrentBuffer]->mAudioDataByteSize; 
	
	memcpy(data, m_AudioBuffers[m_CurrentBuffer]->mAudioData, m_AudioBuffers[m_CurrentBuffer]->mAudioDataByteSize);
	//HPFilter((short *)wh[m_CurrentBuffer].lpData, data, wh[m_CurrentBuffer].dwBufferLength / 2, hpfinmem, hpfoutmem);
	
	m_AudioBuffers[m_CurrentBuffer]->mUserData = NULL;

	int len = ioBytes / 2;
	
	/*
	if(ingain)
	{
		for(int n = 0; n < len; n++)
		{
			x = data[n] * 10;
			if(x > 32767)
				x = 32767;
			else if(x < -32768)
				
				x = -32768;
			data[n] = (short)x;
		}
	}
	*/
	
	err = AudioQueueEnqueueBuffer(m_AudioQueue, m_AudioBuffers[m_CurrentBuffer], 0, NULL);

	pthread_mutex_unlock(&mutex);
	
	m_CurrentBuffer++;
	rframes++;
	
	if(m_CurrentBuffer == MAXRECORDBUFFERS)
		m_CurrentBuffer = 0;
	
	int max = 0;
	int x = 0;
	for(int n = 0; n < len; n++)
	{
		x = data[n] * data[n];
		if(x > max)
			max = x;
	}
	
	vumeter = (int)(1000.0 * log10(max / 1073741824.0));

	noiselevel = noiselevel * (10 * 2 * m_AudioDesc.mSampleRate - 320) / (10 * 2 * m_AudioDesc.mSampleRate);
	
	if(vumeter < noiselevel)
		noiselevel = vumeter;
	
	if(autothreshold)
		threshold = noiselevel / 100 + autothreshold;
	
	/*
	TODO: now damps sound	
	if(threshold > -50 && vumeter < threshold * 100)
	{
		if(silencedetected == 10)
		{
			//LeaveCriticalSection(&cs);
			return -2;
		}
		silencedetected++;
	} else 
		silencedetected = 0;
	*/
	
	m_TimeStamp += len * 1000 / m_AudioDesc.mSampleRate; 
	
	return ioBytes;
}

#define EXP_THRESHOLD 1.0f

void AUDIOIN::EchoSuppression(short *buf, int samples)
{
	int i;
	float o;
	static const float rodown = 0.0005f, roup = 0.005f;
	double ro2, newen;
	static const double em = log(10.0)/20.0;
	int refenergy, ref_first, ref_last;
	
	ref_first = (int)es_refenergy;
	newen = exp(vu_level * em) * 32767.0;
	if(newen > es_refenergy)
		ro2 = pow(1.00 - roup, samples);
	else ro2 = pow(1.00 - rodown, samples);
	es_refenergy = (float)(ro2 * es_refenergy + (1 - ro2) * newen);
	ref_last = (int)es_refenergy;
	for(i = 0; i < samples; i++)
	{
		refenergy = ref_first * (samples - i) / samples + ref_last * i / samples;
		o = buf[i];
		//		printf("%d %f", refenergy, es_inputenergy);
		if(highechosuppression)
		{
			if(es_inputenergy < refenergy * 4.0f)
				o *= es_inputenergy / (refenergy * 4.0f) * es_inputenergy / (refenergy * 4.0f);
		} else if(es_inputenergy * EXP_THRESHOLD < refenergy)
		{
			o *= (es_inputenergy * EXP_THRESHOLD) / refenergy;	// 1:2 expansion on low level signals
																//			printf(", expanding %f", (es_inputenergy * EXP_THRESHOLD) / refenergy);
		}
		//		printf("\n");
		if(abs(buf[i]) > es_inputenergy)
			es_inputenergy = (1 - rodown) * es_inputenergy + roup * abs(buf[i]);
		else es_inputenergy = (1 - rodown) * es_inputenergy + roup * abs(buf[i]);
		if(o > 32767.0)
			buf[i] = 32767;
		else if(o < -32768.0)
			buf[i] = -32768;
		else buf[i] = (short)o;
	}
}


int AUDIOIN::SetRecLevel(int thousands)
{
	// iPhone OS uses a fixed recording input level.
	// http://developer.apple.com/library/ios/#codinghowtos/AudioAndVideo/index.html
	return 0;
}

int AUDIOIN::SetAudioLine(TCHAR *linename)
{
	//DLog("");
	return 0;
}

int AUDIOIN::VuMeter()
{
	///DLog("");
	return 0;
}

int AUDIOIN::SetAudioDevice(TCHAR *sdev)
{
	//DLog(sdev);
	return -1;
}


int AUDIOIN::EnumInputLines(TCHAR inputlines[][100], unsigned *ninputlines)
{
	return 0;
}


int AUDIO_EnumerateDevices(TCHAR devices[][100], int *ndevices, int recording)
{
	return 0;
}

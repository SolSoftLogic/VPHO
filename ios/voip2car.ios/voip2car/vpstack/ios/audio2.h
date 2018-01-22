/*
 *  audio2.h
 *  vpstack
 *
 *  Created by uncle on 26.01.11.
 *  Copyright 2011 Lundlay Ukraine. All rights reserved.
 *
 */
#ifndef AUDIO2_H
#define AUDIO2_H

#include <CoreAudio/CoreAudioTypes.h>
#include <AudioToolbox/AudioToolbox.h>;
#include <AudioUnit/AudioUnit.h>


#include "circular_buffer.h"
//#include "audio2.h"
// workaround to remove warnings, when including OSServiceProviders
//#define __OPENTRANSPORTPROVIDERS__

// needed by lists.h of pwlib, unfortunately also defined in previous
// includes from Apple
//#undef nil


//#define CA_DUMMY_DEVICE_NAME "Null"
#define kAudioDeviceDummy kAudioDeviceUnknown


//class CircularBuffer;

class PSoundChannelCoreAudio //: public PSoundChannel
{
public:
	enum State{
		init_,
		open_,
		setformat_,
		setbuffer_,
		running_,
		mute_,
		destroy_
	};
	
	enum Directions {
		Recorder,
		Player
    };

	
	static void Init();
	PSoundChannelCoreAudio();
	
	//PSoundChannelCoreAudio(const PString &device,
	//					   PSoundChannel::Directions dir,
	//					   unsigned numChannels,
	//					   unsigned sampleRate,
	//					   unsigned bitsPerSample);
	~PSoundChannelCoreAudio();
	
	virtual bool SetFormat(unsigned numChannels,
							   unsigned sampleRate,
							   unsigned bitsPerSample);
	virtual unsigned GetChannels() const;
	virtual unsigned GetSampleRate() const;
	virtual unsigned GetSampleSize() const;
	virtual bool SetBuffers(unsigned int size, unsigned int count);
	virtual bool GetBuffers(unsigned int & size, unsigned int & count);
	virtual bool SetVolume(unsigned volume);
	virtual bool GetVolume(unsigned & volume);
	
	/* Open functions */
//	virtual bool Open(const PString & device,
//						  Directions dir,
//						  unsigned numChannels,
//						  unsigned sampleRate,
//						  unsigned bitsPerSample);
	/* gets never called, see sound.h:
	 * baseChannel->PChannel::IsOpen();  */
	virtual bool IsOpen() const;
	/* gets never called, see sound.h:
	 * baseChannel->PChannel::GetHandle();*/
	virtual int GetHandle() const;
	
	virtual bool Abort();
//	PSoundChannel *CreateOpenedChannel(const PString & driverName,
//									   const PString & deviceName,
//									   const PSoundChannel::Directions,
//									   unsigned numChannels,
//									   unsigned sampleRate,
//									   unsigned bitsPerSample);
	
//	static PString GetDefaultDevice(Directions dir);
//	static PStringList GetDeviceNames(Directions dir);
	
	virtual bool Write(const void *buf, unsigned int len);
//	virtual bool PlaySound(const PSound & sound, bool wait);
//	virtual bool PlayFile(const PFilePath & file, bool wait);
	virtual bool HasPlayCompleted();
	virtual bool WaitForPlayCompletion();
	
	virtual bool Read(void *buf, unsigned int len);
//	virtual bool RecordSound(PSound & sound);
//	virtual bool RecordFile(const PFilePath & file);
	virtual bool StartRecording();
	virtual bool isRecordBufferFull();
	virtual bool AreAllRecordBuffersFull();
	virtual bool WaitForRecordBufferFull();
	virtual bool WaitForAllRecordBuffersFull();
	
protected:
	/**
	 * Common steps for all constructors 
	 */
	void CommonConstruct(){ 
//		os_handle = -1; //  == channel closed.
		// set to a non negative value so IsOpen() returns true
	}  
	
	//OSStatus SetupInputUnit(AudioDeviceID in);
	//OSStatus EnableIO();
	//OSStatus SetupOutputUnit(AudioDeviceID in);
	//OSStatus SetDeviceAsCurrent(AudioDeviceID in);
	
	/**
	 * Based on the desired format, try to configure the AUHAL
	 * Units to match this format as close as possible
	 * e.g. 32bit float -> 16int, stereo -> mono */
	OSStatus MatchHALInputFormat();
	OSStatus MatchHALOutputFormat();
	
	
	OSStatus CallbackSetup();
	
	/**
	 * Pull/Callback function to pass data to AudioConverter
	 */
	static OSStatus ComplexBufferFillPlayback(OpaqueAudioConverter*, 
											  UInt32*, 
											  AudioBufferList*, 
											  AudioStreamPacketDescription**, 
											  void*);
	static OSStatus ComplexBufferFillRecord(OpaqueAudioConverter*, 
											UInt32*, 
											AudioBufferList*, 
											AudioStreamPacketDescription**, 
											void*);
	
	/**
	 * Callback for the AudioUnit to pull/notify more data 
	 */
	static OSStatus PlayRenderProc(
								   void *inRefCon,
								   AudioUnitRenderActionFlags *ioActionFlags,
								   const struct AudioTimeStamp *TimeStamp,
								   UInt32 inBusNumber,
								   UInt32 inNumberFrames,
								   struct AudioBufferList * ioData);
	
	
	static OSStatus RecordProc(
							   void *inRefCon, 
							   AudioUnitRenderActionFlags *ioActionFlags,
							   const AudioTimeStamp *inTimeStamp,
							   UInt32 inBusNumber,
							   UInt32 inNumberFrames,
							   AudioBufferList * ioData);
	
	static OSStatus VolumeChangePropertyListener(
//												 AudioDeviceID id, 
												 UInt32 chan,
												 bool isInput,
//												 AudioDevicePropertyID propID, 
												 void* inUserData
												 );
	
	
	/**
	 * Recording needs a couple more buffers, which are setup by this 
	 * function
	 */
	OSStatus SetupAdditionalRecordBuffers();
	
	/**
	 * Player or Recorder ? 
	 */
	Directions direction;
	State state;
	
	static pthread_mutex_t& GetReadMuteMutex();
	static pthread_mutex_t& GetWriteMuteMutex();
	static bool& GetReadMute();
	static bool& GetWriteMute();
	
	/* These functions just return the right mutex/variable depending whehter
	 * the channel is recorder/player
	 */
	pthread_mutex_t& GetIsMuteMutex();
	bool & isMute();
	
	/** 
	 * Devices
	 */
	AudioUnit mAudioUnit;
//	AudioDeviceID mDeviceID;
	AudioStreamBasicDescription hwASBD, pwlibASBD;
	
	/** Sample rate converter part of AudioToolbox Framework */
	AudioConverterRef converter;     
	CircularBuffer *mCircularBuffer;
	
	/** sample rate of the  AudioUnit as a mutliple of pwlib sample rate */
	Float64 rateTimes8kHz;
	
	/** number and size of internal buffers 
	 * see also SetBuffers */
	unsigned int bufferSizeBytes;
	unsigned int bufferCount;
	
	
	
	/*
	 * Buffer to hold data that are passed to the converter.
	 * Separate means independant of the circular_buffer
	 */
	char* converter_buffer;
	UInt32 converter_buffer_size;
	
	
	/* ==========================================================
	 * Variables used only by the Recorder to circumvent
	 * the inappropriaty control flow of the pull model
	 */
	
	/** Buffers to capture raw data from the microphone */
	CircularBuffer* mInputCircularBuffer;
   AudioBufferList* mInputBufferList;
   UInt32 mRecordInputBufferSize;

#define MIN_INPUT_FILL 20


   /** buffer list to catch the output of the AudioConverter */
   AudioBufferList* mOutputBufferList;
   UInt32 mRecordOutputBufferSize;

};

#endif


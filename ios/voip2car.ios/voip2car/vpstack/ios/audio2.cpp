/*
 *  audio2.cpp
 *  vpstack
 *
 *  Created by uncle on 26.01.11.
 *  Copyright 2011 Lundlay Ukraine. All rights reserved.
 *
 */

#include "audio2.h"
#include "circular_buffer.h"

#include <iostream>  // used for Volume Listener
#include <CoreServices/CoreServices.h>


/************** util *******************/


#define checkStatus( err )// \
if(err) {\
OSStatus error = static_cast<OSStatus>(err);\
cout << "CoreAudio Error " << __func__ << " "  \
<<  error   << "("  << (char*)&err <<  ")" << endl;  \
}         



//#include "maccoreaudio/circular_buffer.inl"

/***** PSoundChannel implementation *****/

PSoundChannelCoreAudio::PSoundChannelCoreAudio() 
: state(init_), mCircularBuffer(NULL), converter_buffer(NULL),
mInputCircularBuffer(NULL), mInputBufferList(NULL), mOutputBufferList(NULL)
{
	CommonConstruct();
	
}

/*
PSoundChannelCoreAudio::PSoundChannelCoreAudio(const PString & device,
											   Directions dir,
											   unsigned numChannels,
											   unsigned sampleRate,
											   unsigned bitsPerSample)
: state(init_), mCircularBuffer(NULL), converter_buffer(NULL),
mInputCircularBuffer(NULL), mInputBufferList(NULL), mOutputBufferList(NULL)
{
	CommonConstruct();
	Open(device, dir, numChannels, sampleRate, bitsPerSample);
}
*/


PSoundChannelCoreAudio::~PSoundChannelCoreAudio()
{
	OSStatus err = noErr;
	State curr(state);
	state = destroy_;
	usleep(1000*20); // about the intervall between two callbacks
	// ensures that current callback has terminated 
	
	/* OutputUnit also for input device,
	 * Stop everything before deallocating buffers */
	switch(curr) {
		case running_:
			err = AudioOutputUnitStop(mAudioUnit);
			checkStatus(err);
			DLog(/*direction <<*/ " AudioUnit stopped" );
			usleep(1000*20); // ensure that all callbacks terminated
			/* fall through */
		case mute_:
		case setbuffer_:
			/* check for all buffers unconditionally */
			err = AudioUnitUninitialize(mAudioUnit);
			checkStatus(err);
			/* fall through */
		case setformat_:
			err = AudioConverterDispose(converter);
			checkStatus(err);
			/* fall through */
		case open_:
			//err = CloseComponent(mAudioUnit);
			checkStatus(err);
//			err = AudioDeviceRemovePropertyListener(mDeviceID,  1, 	
//													kAudioPropertyWildcardSection, kAudioDevicePropertyVolumeScalar, 
//													VolumeChangePropertyListener);
			checkStatus(err);
			/* fall through */
		case init_:
		case destroy_:
			/* nop */;
	}
	
	/* now free all buffers */
	if(this->converter_buffer != NULL){
		free(this->converter_buffer);
		this->converter_buffer = NULL;
	}
	if(this->mCircularBuffer != NULL){
		delete mCircularBuffer;
		this->mCircularBuffer = NULL;
	}
	if(this->mInputCircularBuffer !=NULL) {
		delete this->mInputCircularBuffer;
		this->mInputCircularBuffer = NULL;
	}
	if(this->mInputBufferList != NULL){
		free(this->mInputBufferList);
		this->mInputBufferList = NULL;
	}
	if(this->mOutputBufferList != NULL){
		free(this->mOutputBufferList);
		this->mOutputBufferList = NULL;
	}
	
	// tell PChannel::IsOpen() that the channel is closed.
//	os_handle = -1;  
}


unsigned PSoundChannelCoreAudio::GetChannels() const
{
	if(state >= setformat_)
		return pwlibASBD.mChannelsPerFrame;
	else 
		return 0;
}

unsigned PSoundChannelCoreAudio::GetSampleRate() const
{
	if(state >= setformat_)
		return (unsigned)(pwlibASBD.mSampleRate);
	else 
		return 0;
}

unsigned PSoundChannelCoreAudio::GetSampleSize() const
{
	if(state >= setformat_)
		return (pwlibASBD.mBitsPerChannel);
	else 
		return 0;
}


/*
 * Functions for retrieving AudioDevice list
 */
//#include "maccoreaudio/maccoreaudio_devices.inl"

/*
PString PSoundChannelCoreAudio::GetDefaultDevice(Directions dir)
{
	OSStatus err = noErr;
	UInt32 theSize;
	AudioDeviceID theID;
	
	theSize = sizeof(AudioDeviceID);
	
	if (dir == Player) {
		err = AudioHardwareGetProperty(
									   kAudioHardwarePropertyDefaultOutputDevice,
									   &theSize, &theID);
	}
	else {
		err =  AudioHardwareGetProperty(
										kAudioHardwarePropertyDefaultInputDevice,
										&theSize, &theID);
	}
	
	if (err == 0) {
		return CADeviceName(theID);
	} else {
		return CA_DUMMY_DEVICE_NAME;
	}
}
*/
 
/*
PStringList PSoundChannelCoreAudio::GetDeviceNames(Directions dir)
{
	PStringList devices;
	
	int numDevices;
	AudioDeviceID *deviceList;
	bool isInput = (dir == Recorder);
	
	numDevices = CADeviceList(&deviceList);
	
	for (int i = 0; i < numDevices; i++) {
		PString s = CADeviceName(deviceList[i]);
		if (CADeviceSupportDirection(deviceList[i], isInput) > 0) {
			devices.AppendString(s);
		}
	}
	devices.AppendString(CA_DUMMY_DEVICE_NAME);
	
	if(deviceList != NULL) {
		free(deviceList);
		deviceList = NULL;
	}
	
	return devices;
}
 */

/*
 * Functions responsible for converting 8kHz to 44k1Hz. It works like this. 
 * The AudioHardware is abstracted by an AudioUnit(AUHAL). Each time the device
 * has more data available or needs new data a callback function is called.
 * These are PlayRenderProc and RecordProc. 
 *
 * The user data is stored in a format a set by ::SetFormat. Usually 8kHz, mono,
 * 16bit unsigned. The AudioUnit usually provides 44,1kHz, stereo, 32bit float.
 * So conversion is needed. The conversion is done by the AUConverter from 
 * the AudioToolbox framework.
 *
 * Currently inside the callback functions from the AUHAL, we pass the request
 * to the Converter, that in turn uses another callback function to grab some
 * of data in the input format. All this is done on-the-fly, which means inside
 * the thread managing the AudioHardware. The callback functions of the 
 * converter are ComplexBufferFillPlayback, ComplexbufferFillRecord.
 *
 * The problem we have that 44,1kHz is not a multiple of 8kHz, so we can never 
 * be sure about how many data the converter is going to ask exactly, 
 * sometimes it might be 102, 106.. This is especially true in case of the 
 * first request, where it might ask some additional data depending on 
 * PrimeMethod that garantuees smoth resampling at the border.
 *
 * To summarize, when the AudioUnit device is ready to handle more data. It 
 * calls its callback function, within these functions the data is processed or 
 * prepared by pulling through AUConverter. The converter in turn calls its 
 * callback function to request more input data. Depending on whether we talk 
 * about Read or Write, this includes more or less complex buffering. 
 */



/*
 *  Callback function called by the converter to request more data for 
 *  playback.
 *  
 *  outDataPacketDesc is unused, because all our packets have the same
 *  format and do not need individual description
 */
OSStatus PSoundChannelCoreAudio::ComplexBufferFillPlayback( 
														   AudioConverterRef            inAudioConverter,
														   UInt32                   *ioNumberDataPackets,
														   AudioBufferList           *ioData,
														   AudioStreamPacketDescription **outDataPacketDesc,
														   void             *inUserData)
{
	OSStatus err = noErr;
	PSoundChannelCoreAudio *This = 
	static_cast<PSoundChannelCoreAudio*>(inUserData);
	AudioStreamBasicDescription pwlibASBD = This->pwlibASBD;
	CircularBuffer* circBuf = This->mCircularBuffer;
	
	// output might stop in case there is a complete buffer underrun!!!
	UInt32 minPackets = VPMIN(*ioNumberDataPackets,
							circBuf->size() / pwlibASBD.mBytesPerPacket );
	UInt32 outBytes = minPackets* pwlibASBD.mBytesPerPacket;
	
	//PTRACE(2, __func__ << " requested " << *ioNumberDataPackets << 
	//      " packets, " <<  " fetching " << minPackets << " packets");
	
	if(outBytes > This->converter_buffer_size){
		//PTRACE(1, This->direction << " Converter buffer too small");
		
		// doesn't matter converter will ask right again for remaining data
		// converter buffer multiple of packet size
		outBytes = This->converter_buffer_size;
	}
	
	// dequeue data from circular buffer, without locking(false)
	outBytes = circBuf->Drain(This->converter_buffer, outBytes, false);
	
	UInt32 reqBytes = *ioNumberDataPackets * pwlibASBD.mBytesPerPacket;
	if(outBytes < reqBytes && outBytes < This->converter_buffer_size) {
		reqBytes = VPMIN(reqBytes, This->converter_buffer_size);
		//PTRACE(1, "Buffer underrun, filling up with silence " 
		//	   << (reqBytes - outBytes) << " bytes ");
		
		bzero(This->converter_buffer + outBytes, reqBytes - outBytes );
		outBytes = reqBytes;
	}
	
	// fill structure that gets returned to converter
	ioData->mBuffers[0].mData = (char*)This->converter_buffer;
	ioData->mBuffers[0].mDataByteSize = outBytes;
	
	*ioNumberDataPackets = outBytes / pwlibASBD.mBytesPerPacket;
	
	return err;
}



/*
 * CoreAudio Player callback function
 */
OSStatus PSoundChannelCoreAudio::PlayRenderProc(
												void*                         inRefCon,
												AudioUnitRenderActionFlags*   ioActionFlags,
												const struct AudioTimeStamp*  TimeStamp,
												UInt32                        inBusNumber,
												UInt32                        inNumberFrames,
												struct AudioBufferList*       ioData)
{  
	OSStatus err = noErr;
	PSoundChannelCoreAudio *This = 
	static_cast<PSoundChannelCoreAudio *>(inRefCon);
	
	if( This->state != running_  || This->mCircularBuffer->Empty() ) {
		//   PTRACE(1, __func__ << " terminating");
		return noErr;
	}
	
	//PTRACE(4, __func__ << ", frames " << inNumberFrames);
	
	err = AudioConverterFillComplexBuffer(This->converter,
										  PSoundChannelCoreAudio::ComplexBufferFillPlayback, 
										  This, 
										  &inNumberFrames, // should be packets
										  ioData,
										  NULL /*outPacketDescription*/);
	checkStatus(err);
	
	
	/* now that cpu intensive work is done, make stereo from mono
	 * assume non-interleaved ==> 1 buffer per channel */
	UInt32 len = ioData->mBuffers[0].mDataByteSize;
	if(len > 0 && This->state == running_){
		unsigned i = 1;
		while(i < ioData->mNumberBuffers) {
			memcpy(ioData->mBuffers[i].mData, ioData->mBuffers[0].mData, len);  
			ioData->mBuffers[i].mDataByteSize = len;
			i++;
		}
	}
	
	return err;
}




OSStatus PSoundChannelCoreAudio::RecordProc(
											void*                        inRefCon,
											AudioUnitRenderActionFlags*  ioActionFlags,
											const AudioTimeStamp*        inTimeStamp,
											UInt32                       inBusNumber,
											UInt32                       inNumberFrames,
											AudioBufferList *            ioData)
{
	//PTRACE(2,  __func__ << ", frames  " << inNumberFrames );
	
	OSStatus err = noErr;
	PSoundChannelCoreAudio *This =
	static_cast<PSoundChannelCoreAudio *>(inRefCon);
	CircularBuffer* inCircBuf   = This->mInputCircularBuffer;
	AudioStreamBasicDescription asbd = This->hwASBD;
	
	if(This->state != running_){
		return noErr;
	}
	
	if( This->mRecordInputBufferSize < inNumberFrames * asbd.mFramesPerPacket){
		DLog("Allocated ABL RecordBuffer is too small ");
		inNumberFrames = This->mRecordInputBufferSize / asbd.mFramesPerPacket;
	}
	
	/* fetch the data from the microphone or other input device */
	AudioBufferList*  inputData =  This->mInputBufferList;
	err= AudioUnitRender(This->mAudioUnit,
						 ioActionFlags,
						 inTimeStamp, 
						 inBusNumber,
						 inNumberFrames, //# of frames  requested
						 inputData);// Audio Buffer List to hold data    
	checkStatus(err);
	
	/* in any case reduce to mono by taking only the first buffer */
	AudioBuffer *audio_buf = &inputData->mBuffers[0];
	inCircBuf->Fill((char *)audio_buf->mData, audio_buf->mDataByteSize, 
					false, true); // do not wait, overwrite oldest frames 
	
	/*
	 * Sample Rate Conversion(SRC)
	 */
	unsigned int frames = inCircBuf->size() / This->hwASBD.mBytesPerFrame;
	
	
	/* given the number of Microphone frames how many 8kHz frames are
	 * to expect, keeping a minimum buffer fill of MIN_INPUT_FILL frames to 
	 * have some data handy in case the converter requests more Data */
	if(frames > MIN_INPUT_FILL){
		UInt32 pullFrames = int(float(frames-MIN_INPUT_FILL)/This->rateTimes8kHz);
		UInt32 pullBytes = MIN( This->converter_buffer_size,
							   pullFrames * This->pwlibASBD.mBytesPerFrame);
		
		UInt32 pullPackets = pullBytes / This->pwlibASBD.mBytesPerPacket;
		
		DLog(" going to pull %d packets", pullPackets);
		
		/* now pull the frames through the converter */
		AudioBufferList* outputData = This->mOutputBufferList;
		err = AudioConverterFillComplexBuffer(This->converter,
											  PSoundChannelCoreAudio::ComplexBufferFillRecord, 
											  This, 
											  &pullPackets, 
											  outputData, 
											  NULL /*outPacketDescription*/);
		checkStatus(err);
		
		/* put the converted data into the main CircularBuffer for later 
		 * fetching by the public Read function */
		audio_buf = &outputData->mBuffers[0];
		This->mCircularBuffer->Fill((char*)audio_buf->mData, 
									audio_buf->mDataByteSize, 
									false, true); // do not wait, overwrite oldest frames
	}
	
	return err;
}

/** 
 * Callback function called by the converter to fetch more date 
 */
OSStatus PSoundChannelCoreAudio::ComplexBufferFillRecord( 
														 AudioConverterRef            inAudioConverter,
														 UInt32                   *ioNumberDataPackets,
														 AudioBufferList           *ioData,
														 AudioStreamPacketDescription **outDataPacketDesc,
														 void             *inUserData)
{
	
	OSStatus err = noErr;
	PSoundChannelCoreAudio *This = 
	static_cast<PSoundChannelCoreAudio *>(inUserData);
	CircularBuffer* inCircBuf   = This->mInputCircularBuffer;
	AudioStreamBasicDescription& hwASBD = This->hwASBD;
	
	
	// make sure it's always a multiple of packets
	UInt32 minPackets = MIN(*ioNumberDataPackets,
							inCircBuf->size() / hwASBD.mBytesPerPacket );
	UInt32 ioBytes = minPackets * hwASBD.mBytesPerPacket;
	
	
	//PTRACE(5, __func__ << " " << *ioNumberDataPackets << " requested " 
	//     << " fetching " << minPackets << " packets");
	
	if(ioBytes > This->converter_buffer_size){
		DLog("converter_buffer too small %d requested but only %d fit in", ioBytes, This->converter_buffer_size);
		ioBytes = This->converter_buffer_size;
	}
	
	ioBytes = inCircBuf->Drain((char*)This->converter_buffer, ioBytes, false);
	
	if(ioBytes  != minPackets * hwASBD.mBytesPerPacket) {
		// no more a multiple of packet problably !!!
		DLog("Failed to fetch the computed number of packets");
	}
	
	ioData->mBuffers[0].mData = This->converter_buffer;
	ioData->mBuffers[0].mDataByteSize = ioBytes;
	
	// assuming non-interleaved or mono 
	*ioNumberDataPackets = ioBytes / hwASBD.mBytesPerPacket;
	
	return err;
	
}


OSStatus PSoundChannelCoreAudio::CallbackSetup(){
	OSStatus err = noErr;
	AURenderCallbackStruct callback;
	
	callback.inputProcRefCon = this;
	if (direction == Recorder) 
	{
		callback.inputProc = RecordProc;
		/* kAudioOutputUnit stands for both Microphone/Speaker */
		err = AudioUnitSetProperty(mAudioUnit,
								   kAudioOutputUnitProperty_SetInputCallback,
								   kAudioUnitScope_Global,
								   0,
								   &callback,
								   sizeof(callback));
		
	}
	else {
		callback.inputProc = PlayRenderProc;
		err = AudioUnitSetProperty(mAudioUnit, 
								   kAudioUnitProperty_SetRenderCallback,
								   kAudioUnitScope_Input,
								   0,
								   &callback,
								   sizeof(callback));
	}
	checkStatus(err);
	return err;
}


/********* Function for configuring & initialization of audio units *********/

/**
 * Functions to open an AUHAL component and assign it the device indicated 
 * by deviceID. Conigures the unit for match user desired format  as close as
 * possible while not assuming special hardware. (able to change sampling rate)
 */
/*
OSStatus PSoundChannelCoreAudio::SetupInputUnit(AudioDeviceID in)
{  
	OSStatus err = noErr;
	
	Component comp;            
	ComponentDescription desc;
	
	//There are several different types of Audio Units.
	//Some audio units serve as Outputs, Mixers, or DSP
	//units. See AUComponent.h for listing
	desc.componentType = kAudioUnitType_Output;
	
	//Every Component has a subType, which will give a clearer picture
	//of what this components function will be.
	desc.componentSubType = kAudioUnitSubType_HALOutput;
	
	//all Audio Units in AUComponent.h must use 
	//"kAudioUnitManufacturer_Apple" as the Manufacturer
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	desc.componentFlags = 0;
	desc.componentFlagsMask = 0;
	
	//Finds a component that meets the desc spec's
	comp = FindNextComponent(NULL, &desc);
	if (comp == NULL) return kAudioCodecUnspecifiedError;
	
	//gains access to the services provided by the component
	err = OpenAComponent(comp, &mAudioUnit);
	checkStatus(err);
	
	err = EnableIO();
	checkStatus(err);
	
	err= SetDeviceAsCurrent(in);
	checkStatus(err);
	
	return err;
}
*/
/**
 * By default all units are configured for output. If we want to use a 
 * unit for input we must configure it, before assigning the corresponding
 * device to it. This to make sure that it asks the device driver for the ASBD
 * of the input direction.
 */
/*
OSStatus PSoundChannelCoreAudio::EnableIO()
{
	OSStatus err = noErr;
	UInt32 enableIO;
	
	///////////////
	//ENABLE IO (INPUT)
	//You must enable the Audio Unit (AUHAL) for input and disable output 
	//BEFORE setting the AUHAL's current device.
	
	//Enable input on the AUHAL
	enableIO = 1;
	err =  AudioUnitSetProperty(mAudioUnit,
								kAudioOutputUnitProperty_EnableIO,
								kAudioUnitScope_Input,
								1, // input element
								&enableIO,
								sizeof(enableIO));
	checkStatus(err);
	
	//disable Output on the AUHAL
	enableIO = 0;
	err = AudioUnitSetProperty(mAudioUnit,
							   kAudioOutputUnitProperty_EnableIO,
							   kAudioUnitScope_Output,
							   0,   //output element
							   &enableIO,
							   sizeof(enableIO));
	return err;
}
*/

/*
 * Functions to open an AUHAL component and assign it the device indicated 
 * by deviceID. The builtin converter is configured to accept non-interleaved
 * data.
 */
/*
OSStatus PSoundChannelCoreAudio::SetupOutputUnit(AudioDeviceID out){
	OSStatus err;
	
	//An Audio Unit is a OS component
	//The component description must be setup, then used to 
	//initialize an AudioUnit
	ComponentDescription desc;  
	
	desc.componentType = kAudioUnitType_Output;
	desc.componentSubType = kAudioUnitSubType_HALOutput;
	//desc.componentSubType = kAudioUnitSubType_DefaultOutput;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	desc.componentFlags = 0;
	desc.componentFlagsMask = 0;
	
	//Finds an component that meets the desc spec's 
	Component comp = FindNextComponent(NULL, &desc);  
	if (comp == NULL) return kAudioCodecUnspecifiedError;
    
	//gains access to the services provided by the component
	err = OpenAComponent(comp, &mAudioUnit);  
	checkStatus(err);
	
	//enableIO not needed, because output is default
	
	err = SetDeviceAsCurrent(out);
	return err;
}
*/

/*

OSStatus PSoundChannelCoreAudio::SetDeviceAsCurrent(AudioDeviceID id)
{                       
	UInt32 size = sizeof(AudioDeviceID);
	OSStatus err = noErr;
	
	//get the default device if the device id not specified // bogus 
	if(id == kAudioDeviceUnknown) 
	{  
		if(direction == Recorder) {
			err = AudioHardwareGetProperty(
										   kAudioHardwarePropertyDefaultOutputDevice, &size, &id);
		} else {
			err = AudioHardwareGetProperty(
										   kAudioHardwarePropertyDefaultInputDevice, &size, &id);
		}
		checkStatus(err);   
	}                   
	
	mDeviceID = id;
	
	// Set the Current Device to the AUHAL.
	// this should be done only after IO has been enabled on the AUHAL.
	// This means the direction selected, to make sure the ASBD for the proper
	// direction is requested
	err = AudioUnitSetProperty(mAudioUnit,
							   kAudioOutputUnitProperty_CurrentDevice,
							   kAudioUnitScope_Global,
							   0,  
							   &mDeviceID,
							   sizeof(mDeviceID));
	checkStatus(err);
	
	return err;
}  
*/


/*
 * The major task of Open() is to find the matching device ID.
 *
 */

//bool PSoundChannelCoreAudio::Open(const PString & deviceName,
//									  Directions dir,
//									  unsigned numChannels,
//									  unsigned sampleRate,
//									  unsigned bitsPerSample)
//{
//	OSStatus err;
//	
//	/* Save whether this is a Player or Recorder */
//	this->direction = dir;
//	
//	/*
//	 * Init the AudioUnit and assign it to the requested AudioDevice
//	 */
//	if (strcmp(deviceName, CA_DUMMY_DEVICE_NAME) == 0) {
//		/* Dummy device */
//		PTRACE(6, "Dummy device " << direction);
//		mDeviceID = kAudioDeviceUnknown;
//	} else {
//		
//		AudioDeviceID deviceID = GetDeviceID(deviceName, direction == Recorder);
//		if(direction == Player)
//			err = SetupOutputUnit(deviceID);
//		else 
//			err = SetupInputUnit(deviceID);
//		checkStatus(err);
//	}
	
	
	/*
	 * Add a listener to print current volume setting in case it is changed
	 */
//	err = AudioDeviceAddPropertyListener(mDeviceID,  1, 	
//										 kAudioPropertyWildcardSection, kAudioDevicePropertyVolumeScalar, 
//										 VolumeChangePropertyListener, (void *)this);
//	checkStatus(err);
	
	
	//os_handle = mDeviceID;  // tell PChanne::IsOpen() that the channel is open.
//	os_handle = 8;  // tell PChannel::IsOpen() that the channel is open.
//	state = open_;
//	return SetFormat(numChannels, sampleRate, bitsPerSample);
//}

/* 
 * Audio Unit for the Hardware Abstraction Layer(AUHAL) have builtin 
 * converters. It would be nice if we could configure it to spit out/consume 
 * the data in the format the data are passed by Read/Write function calls.
 *
 * Unfortunately this is not possible for the microphone, because this 
 * converter does not have a buffer inside, so it cannot do any Sample
 * Rate Conversion(SRC). We would have to set the device nominal sample
 * rate itself to 8kHz. Unfortunately not all microphones can do that,
 * so this is not an option. Maybe there will be some change in the future
 * by Apple, so we leave it here. 
 *
 * For the output we have the problem that we do not know currently how
 * to configure the channel map so that a mono input channel gets copied 
 * to all output channels, so we still have to do the conversion ourselves
 * to copy the result onto all output channels.
 *
 * Still the builtin converters can be used for something useful, such as 
 * converting from interleaved -> non-interleaved and to reduce the number of 
 * bits per sample to save space and time while copying 
 */

/* 
 * Configure the builtin AudioConverter to accept non-interleaved data.
 * Turn off SRC by setting the same sample rate at both ends.
 * See also general notes above
 */ 
OSStatus PSoundChannelCoreAudio::MatchHALOutputFormat()
{
	OSStatus err = noErr;
	//AudioStreamBasicDescription& asbd = hwASBD;
	UInt32 size = sizeof (AudioStreamBasicDescription);
	
	memset(&hwASBD, 0, size);
	
	/*
	 err = AudioDeviceGetProperty(mDeviceID, 
	 0,     // channel
	 //true,  // isInput
	 false,  // isInput
	 kAudioDevicePropertyStreamFormat,
	 &size, &hwASBD);
	 checkStatus(err);
	 */
	
	//Get the current stream format of the output
	err = AudioUnitGetProperty (mAudioUnit,
								kAudioUnitProperty_StreamFormat,
								kAudioUnitScope_Output,
								0,  // output bus 
								&hwASBD,
								&size);
	checkStatus(err);  
	
	// make sure it is non-interleaved
	bool isInterleaved = 
	!(hwASBD.mFormatFlags & kAudioFormatFlagIsNonInterleaved);
	
	hwASBD.mFormatFlags |= kAudioFormatFlagIsNonInterleaved; 
	if(isInterleaved){
		// so its only one buffer containing all data, according to 
		// list.apple.com: You only multiply out by mChannelsPerFrame 
		// if you are doing interleaved.
		hwASBD.mBytesPerPacket /= hwASBD.mChannelsPerFrame;
		hwASBD.mBytesPerFrame  /= hwASBD.mChannelsPerFrame;
	}
	
	//Set the stream format of the output to match the input
	err = AudioUnitSetProperty (mAudioUnit,
								kAudioUnitProperty_StreamFormat,
								kAudioUnitScope_Input,
								0,
								&hwASBD,
								size);
	
	
	// make sure we really know the current format
	size = sizeof (AudioStreamBasicDescription);
	err = AudioUnitGetProperty (mAudioUnit,
								kAudioUnitProperty_StreamFormat,
								kAudioUnitScope_Input,
								0,  // input bus
								&hwASBD,
								&size);
	
	return err;
}


/* 
 * Configure the builtin AudioConverter to provide data in non-interleaved 
 * format. Turn off SRC by setting the same sample rate at both ends.
 * See also general notes above
 */ 
OSStatus PSoundChannelCoreAudio::MatchHALInputFormat()
{
	OSStatus err = noErr;
	AudioStreamBasicDescription& asbd = hwASBD;
	UInt32 size = sizeof (AudioStreamBasicDescription);
	
	memset(&asbd, 0, size);
	
	/*
	 err = AudioDeviceGetProperty(mDeviceID, 
	 0,     // channel
	 true,  // isInput
	 kAudioDevicePropertyStreamFormat,
	 &size, 
	 &asbd);
	 checkStatus(err);
	 */
	
	/* This code asks for the supported sample rates of the microphone
	 UInt32 count, numRanges;
	 err = AudioDeviceGetPropertyInfo ( mDeviceID, 
	 0, true,
	 kAudioDevicePropertyAvailableNominalSampleRates, 
	 &count, NULL );
	 
	 numRanges = count / sizeof(AudioValueRange);
	 AudioValueRange* rangeArray = (AudioValueRange*)malloc ( count );
	 
	 err = AudioDeviceGetProperty ( mDeviceID, 
	 0, true, 
	 kAudioDevicePropertyAvailableNominalSampleRates, 
	 &count, (void*)rangeArray );
	 checkStatus(err);
	 */
	
	//Get the current stream format of the output
	err = AudioUnitGetProperty (mAudioUnit,
								kAudioUnitProperty_StreamFormat,
								kAudioUnitScope_Input,
								1,  // input bus/
								&asbd,
								&size);
	
	/*
	 * make it one-channel, non-interleaved, keeping same sample rate 
	 */
	bool isInterleaved = 
	!(asbd.mFormatFlags & kAudioFormatFlagIsNonInterleaved); 
	
	if (isInterleaved)
		DLog("channels are interleaved ");
	
	// mFormatID -> assume lpcm !!!
	asbd.mFormatFlags |= kAudioFormatFlagIsNonInterleaved;   
	if(isInterleaved){
		// so it's only one buffer containing all channels, according to 
		//list.apple.com: You only multiple out by mChannelsPerFrame 
		//if you are doing interleaved.
		asbd.mBytesPerPacket /= asbd.mChannelsPerFrame;
		asbd.mBytesPerFrame  /= asbd.mChannelsPerFrame;
	}
	asbd.mChannelsPerFrame = 1;
	
	// Set it to output side of input bus
	size = sizeof (AudioStreamBasicDescription);
	err = AudioUnitSetProperty (mAudioUnit,
								kAudioUnitProperty_StreamFormat,
								kAudioUnitScope_Output,
								1,  // input bus
								&asbd,
								size);
	checkStatus(err);
	
	// make sure we really know the current format
	size = sizeof (AudioStreamBasicDescription);
	err = AudioUnitGetProperty (mAudioUnit,
								kAudioUnitProperty_StreamFormat,
								kAudioUnitScope_Output,
								1,  // input bus
								&hwASBD,
								&size);
	
	return err;
}



bool PSoundChannelCoreAudio::SetFormat(unsigned numChannels,
										   unsigned sampleRate,
										   unsigned bitsPerSample)
{
	// making some assumptions about input format for now
	//PAssert(sampleRate == 8000 && numChannels == 1 && bitsPerSample == 16, PUnsupportedFeature);
	
	if(state != open_){
		DLog("Please select a device first");
		return false;
	}
	
	/*
	 * Setup the pwlibASBD
	 */
	memset((void *)&pwlibASBD, 0, sizeof(AudioStreamBasicDescription)); 
	
	/* pwlibASBD->mReserved */
	pwlibASBD.mFormatID          = kAudioFormatLinearPCM;
	pwlibASBD.mFormatFlags       = kLinearPCMFormatFlagIsSignedInteger;
	pwlibASBD.mFormatFlags      |= kLinearPCMFormatFlagIsNonInterleaved; 
//#if PBYTE_ORDER == PBIG_ENDIAN
//	pwlibASBD.mFormatFlags      |= kLinearPCMFormatFlagIsBigEndian;
//#endif
	pwlibASBD.mSampleRate        = sampleRate;
	pwlibASBD.mChannelsPerFrame  = numChannels;
	pwlibASBD.mBitsPerChannel    = bitsPerSample;
	pwlibASBD.mBytesPerFrame     = bitsPerSample / 8;
	pwlibASBD.mFramesPerPacket   = 1;
	pwlibASBD.mBytesPerPacket    = pwlibASBD.mBytesPerFrame;
	
	
//	if(mDeviceID == kAudioDeviceDummy){
//		DLog(1, "Dummy device");
//		return true;
//	}
	
	OSStatus err;
	if(direction == Player)
		err = MatchHALOutputFormat();  
	else 
		err = MatchHALInputFormat();
	checkStatus(err);
	
	/*
	 * Sample Rate Conversion (SRC)
	 * Create AudioConverters, input/output buffers, compute conversion rate 
	 */
	
	//PTRACE(3, "ASBD PwLib Format of "    << direction << endl << pwlibASBD);
	//PTRACE(3, "ASBD Hardware Format of " << direction << endl << hwASBD);
	
	
	// how many samples has the output device compared to pwlib sample rate?
	rateTimes8kHz  = hwASBD.mSampleRate / pwlibASBD.mSampleRate;
	
	
	/*
	 * Create Converter for Sample Rate conversion
	 */
	if (direction == Player) 
		err = AudioConverterNew(&pwlibASBD, &hwASBD, &converter);
	else 
		err = AudioConverterNew(&hwASBD, &pwlibASBD, &converter);
	checkStatus(err);
	
	UInt32 quality = kAudioConverterQuality_Max;
	err = AudioConverterSetProperty(converter,
									kAudioConverterSampleRateConverterQuality,
									sizeof(UInt32),
									&quality);
	checkStatus(err);
	
	//if(direction == Recorder){
	// trying compute number of requested data more predictably also 
	// for the first request
	UInt32 primeMethod = kConverterPrimeMethod_None;
	err = AudioConverterSetProperty(converter,
									kAudioConverterPrimeMethod,
									sizeof(UInt32),
									&primeMethod);
	checkStatus(err);
	//}
	
	state = setformat_;
	return true;
}


/* gets never called, see sound.h:
 * baseChannel->PChannel::GetHandle(); 
 */
bool PSoundChannelCoreAudio::IsOpen() const
{
	//return (os_handle != -1);
	return (state != init_ || state != destroy_);
}


/* gets never called, see sound.h:
 * baseChannel->PChannel::GetHandle(); 
 */
int PSoundChannelCoreAudio::GetHandle() const
{
	DLog("GetHandle");
	//return os_handle; 
	return -1;
}

bool PSoundChannelCoreAudio::Abort()
{
	DLog("Abort");
	//PAssert(0, PUnimplementedFunction);
	return false;
}




/**
 * SetBuffers is used to create the circular buffer as requested by the caller 
 * plus all the hidden buffers used for Sample-Rate-Conversion(SRC)
 *
 * A device can not be used after calling Open(), SetBuffers() must
 * also be called before it can start working.
 *
 * size:    Size of each buffer
 * count:   Number of buffers
 *
 */


bool PSoundChannelCoreAudio::SetBuffers(unsigned int bufferSize,
											unsigned int bufferCount)
{
	OSStatus err = noErr;
	
	if(state != setformat_){
		// use GetError
		DLog("Please specify a format first");
		return false;
	}
	
	//PTRACE(3, __func__ << direction << " : "
	//	   << bufferSize << " BufferSize "<< bufferCount << " BufferCount");
	
//	PAssert(bufferSize > 0 && bufferCount > 0 && bufferCount < 65536, \
//			PInvalidParameter);
	
	this->bufferSizeBytes = bufferSize;
	this->bufferCount = bufferCount;
	
//	if(mDeviceID == kAudioDeviceDummy){
//		// abort here
//		PTRACE(1, "Dummy device");
//		return true;
//	}
	
	mCircularBuffer = new CircularBuffer(bufferSize * bufferCount );
	
	
	/** Register callback function */
	err = CallbackSetup();
	
	
	/** 
	 * Tune the buffer size of the underlying audio device.
	 * The aim is to make the device request half of the buffer size on
	 * each callback.
	 *
	 * Not possible, because  buffer size for device input/output is not
	 * independant of each other. Creates havoc in case SetBuffer is called with 
	 * different buffer size for Player/Recorder Channel
	 */
	/*
	 UInt32 targetSizeBytes = bufferSizeBytes / 2;
	 UInt32 size = sizeof(UInt32);
	 if (direction == Player) {
     err = AudioConverterGetProperty(converter,
	 kAudioConverterPropertyCalculateOutputBufferSize,
	 &size, &targetSizeBytes);
	 } else {
     err = AudioConverterGetProperty(converter,
	 kAudioConverterPropertyCalculateInputBufferSize,
	 &size, &targetSizeBytes);
	 }
	 checkStatus(err);
	 if (err) {
     return false;
	 }
	 
	 PTRACE(2, __func__ <<  " AudioDevice buffer size set to " 
	 << targetSizeBytes);
	 
	 UInt32 targetSizeFrames = targetSizeBytes / hwASBD.mBytesPerFrame;
	 if (direction == Player) {
	 err = AudioDeviceSetProperty( mDeviceID,
	 0, //&ts, timestruct 
	 0, // output channel 
	 true, // isInput 
	 // kAudioDevicePropertyBufferSize, 
	 kAudioDevicePropertyBufferFrameSize,
	 sizeof(UInt32),
	 &targetSizeFrames);
	 } else {
	 err = AudioDeviceSetProperty( mDeviceID,
	 0, //&ts, timestruct 
	 1, // input channel
	 false, // isInput 
	 kAudioDevicePropertyBufferFrameSize,
	 sizeof(UInt32),
	 &targetSizeFrames);
	 }
	 checkStatus(err);
	 */
	
	
	/** 
	 * Allocate byte array passed as input to the converter 
	 */
	UInt32 bufferSizeFrames, bufferSizeBytes;
	UInt32 propertySize = sizeof(UInt32);
	//err = AudioDeviceGetProperty( mDeviceID,
	//							 0,  // output channel,  
	//							 true,  // isInput 
	//							 kAudioDevicePropertyBufferFrameSize,
	//							 &propertySize,
	//							 &bufferSizeFrames);
	checkStatus(err);
	bufferSizeBytes = bufferSizeFrames * hwASBD.mBytesPerFrame;
	//UInt32 bufferSizeBytes = targetSizeBytes;
	
	if (direction == Player) {
		UInt32 propertySize = sizeof(UInt32);
		err = AudioConverterGetProperty(converter,
										kAudioConverterPropertyCalculateInputBufferSize,
										&propertySize,
										&bufferSizeBytes);
		checkStatus(err);
		converter_buffer_size = bufferSizeBytes;
	} else {
		// on each turn the device spits out bufferSizeBytes bytes
		// the input ringbuffer has at most MIN_INPUT_FILL frames in it 
		// all other frames were converter during the last callback
		converter_buffer_size = bufferSizeBytes + 
		2 * MIN_INPUT_FILL * hwASBD.mBytesPerFrame;
	}
	converter_buffer = (char*)malloc(converter_buffer_size);
//	if(converter_buffer == NULL)
		//DLog("Failed to allocate converter_buffer");
//	else
		//DLog("Allocated converter_buffer of size %d", converter_buffer_size );
	
	
	/** In case of Recording we need a couple of buffers more */
	if(direction == Recorder){
		SetupAdditionalRecordBuffers();
	}
	
	/*
	 * AU Setup, allocates necessary buffers... 
	 */
	err = AudioUnitInitialize(mAudioUnit);
	
	//(err);
	
	state = setbuffer_;
	
	return true;
	
}

OSStatus PSoundChannelCoreAudio::SetupAdditionalRecordBuffers()
{
	OSStatus err = noErr;
	UInt32 bufferSizeFrames, bufferSizeBytes;
	
	/** 
	 * build buffer list to take over the data from the microphone 
	 */
	UInt32 propertySize = sizeof(UInt32);
//	err = AudioDeviceGetProperty( mDeviceID,
//								 0,  // channel, probably all  
//								 true,  // isInput 
//								 //false,  // isInput ()
//								 kAudioDevicePropertyBufferFrameSize,
//								 &propertySize,
//								 &bufferSizeFrames);
	checkStatus(err);
	bufferSizeBytes = bufferSizeFrames * hwASBD.mBytesPerFrame;
	bufferSizeBytes += bufferSizeBytes / 10; // +10%
	
	//calculate size of ABL given the last field, assum non-interleaved 
	UInt32 mChannelsPerFrame = hwASBD.mChannelsPerFrame;
	//UInt32 propsize = (UInt32) &(((AudioBufferList *)0)->mBuffers[mChannelsPerFrame]);
	UInt32 propsize = sizeof(AudioBuffer) * mChannelsPerFrame + sizeof(AudioBufferList);
	
	//malloc buffer lists
	mInputBufferList = (AudioBufferList *)malloc(propsize);
	mInputBufferList->mNumberBuffers = hwASBD.mChannelsPerFrame;
	
	//pre-malloc buffers for AudioBufferLists
	for(UInt32 i =0; i< mInputBufferList->mNumberBuffers ; i++) {
		mInputBufferList->mBuffers[i].mNumberChannels = 1;
		mInputBufferList->mBuffers[i].mDataByteSize = bufferSizeBytes;
		mInputBufferList->mBuffers[i].mData = malloc(bufferSizeBytes);
	}
	mRecordInputBufferSize = bufferSizeBytes;
	
	/** allocate ringbuffer to cache data before passing them to the converter */
	// take only one buffer -> mono, use double buffering
	mInputCircularBuffer = new CircularBuffer(bufferSizeBytes * 2);
	
	
	/** 
	 * Build buffer list that is passed to the Converter to be filled with 
	 * the converted frames.
	 */
	// given the number of input bytes how many bytes to expect at the output?
	bufferSizeBytes += MIN_INPUT_FILL * hwASBD.mBytesPerFrame;
	propertySize = sizeof(UInt32);
	err = AudioConverterGetProperty(converter,
									kAudioConverterPropertyCalculateOutputBufferSize,
									&propertySize,
									&bufferSizeBytes);
	checkStatus(err);
	
	
	//calculate number of buffers from channels
	mChannelsPerFrame = pwlibASBD.mChannelsPerFrame;
	//propsize = (UInt32) &(((AudioBufferList *)0)->mBuffers[mChannelsPerFrame]);
	propsize = sizeof(AudioBuffer) * mChannelsPerFrame + sizeof(AudioBufferList);
	
	//malloc buffer lists
	mOutputBufferList = (AudioBufferList *)malloc(propsize);
	mOutputBufferList->mNumberBuffers = pwlibASBD.mChannelsPerFrame;
	
	//pre-malloc buffers for AudioBufferLists
	for(UInt32 i =0; i< mOutputBufferList->mNumberBuffers ; i++) {
		mOutputBufferList->mBuffers[i].mNumberChannels = 1;
		mOutputBufferList->mBuffers[i].mDataByteSize = bufferSizeBytes;
		mOutputBufferList->mBuffers[i].mData = malloc(bufferSizeBytes);
	}
	mRecordOutputBufferSize = bufferSizeBytes;
	
	return err;
}


bool PSoundChannelCoreAudio::GetBuffers(unsigned int & size,
											unsigned int & count)
{
	size = bufferSizeBytes;
	count = bufferCount;
	return true;
}



//OSStatus PSoundChannelCoreAudio::VolumeChangePropertyListener(AudioDeviceID id, 
//															  UInt32 chan, Boolean isInput, AudioDevicePropertyID propID, 
//															  void *user_data)
//{
//	PSoundChannelCoreAudio *This = 
//	static_cast<PSoundChannelCoreAudio*>(user_data);
//	OSStatus err = noErr;
//	UInt32 theSize = sizeof(Float32);
//	Float32 volume;
	
	/*
	 * Function similar to GetVolume, but we are free to ask the volume 	
	 * for the intput/output direction 
	 */
	
	// not all devices have a master channel
//	err = AudioDeviceGetProperty(This->mDeviceID, 0, isInput,
//								 kAudioDevicePropertyVolumeScalar,
//								 &theSize, &volume);
//	if(err != kAudioHardwareNoError) {
//		// take the value of first channel to be the volume
//		theSize = sizeof(volume);
//		err = AudioDeviceGetProperty(This->mDeviceID, 1, isInput,
//									 kAudioDevicePropertyVolumeScalar,
//									 &theSize, &volume);
//	}
	
//	std::cout << (isInput?"Recorder":"Player")
//	<< " volume updated " << unsigned(100*volume) << std::endl;
	
//	return noErr;
//}

//#include "maccoreaudio/mute_hack.inl"

/**
 * Also check out this to see the difference between streams and channels.
 * http://lists.apple.com/archives/coreaudio-api/2001/Nov/msg00155.html
 * In short a stream contains several channels, e. g. 2 in case of stereo.
 */

bool PSoundChannelCoreAudio::GetVolume(unsigned & volume)
{
/*	
	OSStatus err = noErr;
	UInt32 theSize;
	Float32 theValue;
	bool isInput = (direction == Player ? false : true);
	
	if(mDeviceID == kAudioDeviceDummy){
		//in the case of a dummy device, we simply return 0 in all cases
		PTRACE(1, "Dummy device");
		volume = 0;
		return true;
	}
	
	
	theSize = sizeof(theValue);
	// not all devices have a master channel
	err = AudioDeviceGetProperty(mDeviceID, 0, isInput,
								 kAudioDevicePropertyVolumeScalar,
								 &theSize, &theValue);
	if(err != kAudioHardwareNoError) {
		// take the value of first channel to be the volume
		theSize = sizeof(theValue);
		err = AudioDeviceGetProperty(mDeviceID, 1, isInput,
									 kAudioDevicePropertyVolumeScalar,
									 &theSize, &theValue);
	}
	
	
	if (err == kAudioHardwareNoError) {
		// volume is between 0 and 100? 
		volume = (unsigned) (theValue * 100);
		return true;
	} else {
		volume = 0;
		return false;
	}
*/ 
}


bool PSoundChannelCoreAudio::Write(const void *buf,unsigned int len)
{
	PTRACE(5, "Write called with len " << len);
	
	if(state < setbuffer_){
		PTRACE(1, __func__ << " Please initialize device first");
		return false;
	}
	
	pthread_mutex_lock(&GetIsMuteMutex());
	if(isMute() && state != mute_){
		PTRACE(3, __func__ << "muting the " << direction << " device");
		state = mute_;
		OSStatus err = AudioOutputUnitStop(mAudioUnit); 
		checkStatus(err);
		/* isMute() => state==mute */   
	}
	
	
	if (mDeviceID == kAudioDeviceDummy || isMute() && state == mute_ ) {
		lastWriteCount =  len; 
		
		// safe to assume non-interleaved or mono
		UInt32 nr_samples = len / pwlibASBD.mBytesPerFrame; 
		usleep(UInt32(nr_samples/pwlibASBD.mSampleRate * 1000000)); // 10E-6 [s]
		pthread_mutex_unlock(&GetIsMuteMutex());
		return true;  
	}
	
	// Start the device before putting datA into the buffer
	// Otherwise the thread could be locked in case the buffer is full
	// and the device is not running and draining the buffer
	if(state == setbuffer_ || (state == mute_ && !isMute())){
		state = running_;
		PTRACE(2, "Starting " << direction << " device.");
		OSStatus err = AudioOutputUnitStart(mAudioUnit);
		checkStatus(err);
	} 
	pthread_mutex_unlock(&GetIsMuteMutex());
	
	// Write to circular buffer with locking 
	lastWriteCount = mCircularBuffer->Fill((const char*)buf, len, true);
	
	return (true);
}


bool PSoundChannelCoreAudio::PlaySound(const PSound & sound,
										   bool wait)
{
	if (!Write((const BYTE *)sound, sound.GetSize()))
		return false;
	
	if (wait)
		return WaitForPlayCompletion();
	
	return true;
}

bool PSoundChannelCoreAudio::PlayFile(const PFilePath & file,
										  bool wait)
{
	PTRACE(1, __func__ );
	PAssert(0, PUnimplementedFunction);
	
	return true; 
}

bool PSoundChannelCoreAudio::HasPlayCompleted()
{
	PTRACE(1, __func__ );
	PAssert(0, PUnimplementedFunction);
	return false;
}

bool PSoundChannelCoreAudio::WaitForPlayCompletion()
{
	PTRACE(1, __func__ );
	PAssert(0, PUnimplementedFunction);
	return false;
}

bool PSoundChannelCoreAudio::Read(void *buf,
									  unsigned int len)
{
	PTRACE(5, "Read called with len " << len);
	
	if(state < setbuffer_){
		PTRACE(1, __func__ << " Please initialize device first");
		return false;
	}
	
	pthread_mutex_lock(&GetIsMuteMutex());
	if(isMute() && state != mute_){
		PTRACE(2, __func__ << "muting the " << direction << " device");
		state = mute_;
		OSStatus err = AudioOutputUnitStop(mAudioUnit); 
		checkStatus(err);
		/* isMute() => state==mute */   
	}
	
	if (mDeviceID == kAudioDeviceDummy || isMute()) {
		lastReadCount =  len; 
		bzero(buf, len);
		
		// we are working with non-interleaved or mono
		UInt32 nr_samples = len / pwlibASBD.mBytesPerFrame; 
		usleep(UInt32(nr_samples/pwlibASBD.mSampleRate * 1000000)); // 10E-6 [s]
		pthread_mutex_unlock(&GetIsMuteMutex());
		return true;  
	}
	
	// Start the device before draining data or the thread might be locked 
	// on an empty buffer and never wake up, because no device is filling
	// with data
	if(state == setbuffer_ || (state == mute_ && !isMute())){
		state = running_;
		PTRACE(2, "Starting " << direction << " device.");
		OSStatus err = AudioOutputUnitStart(mAudioUnit);
		checkStatus(err);
	}
	pthread_mutex_unlock(&GetIsMuteMutex());
	
	lastReadCount = mCircularBuffer->Drain((char*)buf, len, true);
	return (true);
}

/*
bool PSoundChannelCoreAudio::RecordSound(PSound & sound)
{
	PTRACE(1, __func__ );
	PAssert(0, PUnimplementedFunction);
	return false;
}

bool PSoundChannelCoreAudio::RecordFile(const PFilePath & file)
{
	PTRACE(1, __func__ );
	PAssert(0, PUnimplementedFunction);
	return false;
}

bool PSoundChannelCoreAudio::StartRecording()
{
	if(state != setbuffer_){
		PTRACE(1, __func__ << " Initialize the device first");
		return false;
	}
	
	pthread_mutex_lock(&GetIsMuteMutex());
	if(state == setbuffer_ || (state == mute_ && !isMute())  ){
		state = running_;
		PTRACE(2,__func__ <<  "Starting " << direction << " device.");
		OSStatus err = AudioOutputUnitStart(mAudioUnit);
		checkStatus(err);
	}
	pthread_mutex_unlock(&GetIsMuteMutex());
	return false;
}

bool PSoundChannelCoreAudio::isRecordBufferFull()
{
	PAssert(direction == Recorder, PInvalidParameter);
	if(state != setbuffer_){
		PTRACE(1, __func__ << " Initialize the device first");
		return false;
	}
	
	return (mCircularBuffer->size() > bufferSizeBytes);
}

bool PSoundChannelCoreAudio::AreAllRecordBuffersFull()
{
	PAssert(direction == Recorder, PInvalidParameter);
	if(state != setbuffer_){
		PTRACE(1, __func__ << " Initialize the device first");
		return false;
	}
	
	return (mCircularBuffer->Full());
}

bool PSoundChannelCoreAudio::WaitForRecordBufferFull()
{
	PTRACE(1, __func__ );
	PAssert(0, PUnimplementedFunction);
	if (os_handle < 0) {
		return false;
	}
	
	return PXSetIOBlock(PXReadBlock, readTimeout);
}

bool PSoundChannelCoreAudio::WaitForAllRecordBuffersFull()
{
	PTRACE(1, __func__ );
	PAssert(0, PUnimplementedFunction);
	return false;
}
*/

// End of fiunsigned intle

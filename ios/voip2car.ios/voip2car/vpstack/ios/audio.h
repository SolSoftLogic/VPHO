
#ifndef _AUDIO_H_INCLUDED
#define _AUDIO_H_INCLUDED

#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioServices.h>
#include <AudioToolbox/AudioToolbox.h>
#include <mach/mach_time.h>
#include <sys/time.h>

#include "iosportability.h"

//#define _MAX_BUFFERS 100

// For iPhone 2.x and earlier
//#if __IPHONE_OS_VERSION_MIN_REQUIRED <= __IPHONE_2_2
//# define kAudioUnitSubType_VoiceProcessingIO kAudioUnitSubType_RemoteIO
//# define kAudioSessionProperty_OverrideCategoryEnableBluetoothInput -1
//#endif

#define AUDIOERR_FATAL (-3)

//#define AUDIOOUT CCoreAudioOut
#define _NameOutDevice	"iPhone OS Out Audio Driver"
#define _NameInDevice	"iPhone OS Input Audio Driver"


#define MAXRECORDBUFFERS 8											///< Max recorder buffers
#define MAXPLAYBUFFERS 50											///< Max count play buffers

/**
 * Audio device play 
 */
class AUDIOOUT {
public:
	AUDIOOUT();
	~AUDIOOUT(); 

	int SetAudioDevice(TCHAR *sdev);								///< Not implemented
	int PutData(int channels, int sfreq, short *buf, int len);		///< Recived data for playing
	int Stop();														///< Stop playing
	int GetCurrentDelay();											///< TODO: ????
	int SetVolume(int thousands);									///< Not implemented
	int VuMeter();													///< TODO: ????
	
	void AddPlayedSamples(UInt32 ALen);
	bool IsSpeaker();
	void SetSpeaker(bool ASpeaker);
	
protected:
	bool Start(int chanels, int freq);

	pthread_mutex_t				mutex;
	pthread_mutex_t				mutexSpeaker;							///< Block mutex for Speaker
	
	volatile long long			m_RecievedSamples;					///< Recived samples from network
	volatile long long			m_PlayedSamples;					///< Palyed samples 
	unsigned					m_CurrentBuffer;					///< Current audio buffer for playing
	bool						m_bPlaying;							///< Device is playing mode
	unsigned int				m_MaxBufferSize;					///< Size of buffer audio sample
	AudioStreamBasicDescription m_AudioDesc;						///< Audio stream format
    AudioQueueRef				m_AudioQueue;						///< Queue for audio output
    AudioQueueBufferRef			m_AudioBuffers[MAXPLAYBUFFERS];		///< Buffers for Queue
	bool						m_bSpeaker;							///< Play sound over main speaker

};



/**
 * Audio device recorder 32000
 */
class AUDIOIN {
public:
	AUDIOIN();
	~AUDIOIN(); 
	int SetAudioDevice(TCHAR *sdev);
	int CheckAudioDevice(TCHAR *sdev);
	int Start(int AChannles, int ASampleRate, int AFrameLen, int ABufLength);	// in Hz, bytes, milliseconds
	int Stop();
	int GetData(short *AData);
	int GetDataTm() { return m_TimeStamp; }												
	int GetRecLevel() { return reclevel; }
	int SetRecLevel(int thousands);
	int SetAudioLine(TCHAR *linename);
//	int FillCBWithInputLines(HWND hWnd, int defcomponenttype);
	int VuMeter();				// in millibell
	void SetThreshold(int dB) { threshold = dB;}
	int GetThreshold() { return threshold; }
	
	void EchoSuppression(short *buf, int len);
	void SetEchoSuppressionRefLevel(int level) { vu_level = level; }
	void SetEchoSuppressionMode(int high) { highechosuppression = high; }
	int	 EnumInputLines(TCHAR inputlines[][100], unsigned *ninputlines);
	
	int micboost, echosuppression, autothreshold;

	void MakeBufferReady(AudioQueueBufferRef ABuffer);
	bool IsMute();
	void SetMute(bool AMute);
	
protected:
	int cursfreq, curchannels, curframelen;
	int highechosuppression, reclevel;
	int hpfinmem[2];
	double hpfoutmem[2];
	
	int vu_level;
	int			vumeter;											///< Sound amplitude
	unsigned	rframes;											///< Recieved audio frames from network source
	int			noiselevel;											///< Noise level
	int			threshold;
	int			silencedetected;
	
	// for echo canselation
	float		es_inputenergy;
	float		es_refenergy;

	pthread_mutex_t				mutex;								///< Block mutex for AudioBuffers
	pthread_mutex_t				mutexSound;							///< Block mutex for Mute and Speaker
	unsigned					m_TimeStamp;						///< Time stamp in milliseconds
	unsigned int				m_CurrentBuffer;					///< Current audio buffer for sent
	bool						m_bRecording;						///< Flag if device in recording state
	AudioStreamBasicDescription m_AudioDesc;						///< iOS audio stream Description
    AudioQueueRef				m_AudioQueue;						///< iOS reference audio queue
    AudioQueueBufferRef			m_AudioBuffers[MAXRECORDBUFFERS];	///< iOS array references for audio queue buffers
	unsigned int				m_MaxBufferSize;					///< Calculated buffers size for audio frame
	bool						m_bMute;								///< Mute microphone input
};

int AUDIO_EnumerateDevices(TCHAR devices[][100], int *ndevices, int recording); // TODO:


#endif

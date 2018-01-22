/*
 *  tone_generator.h
 *  vpstack
 *
 *  Created by uncle on 26.01.11.
 *  Copyright 2011. All rights reserved.
 *
 */
#ifndef TONE_GENERATOR_H
#define TONE_GENERATOR_H

#include <string>

class Tones : public std::basic_string<short>
{
	
public:
    enum {
		MaxVolume = 100,
		DefaultSampleRate = 8000,
		MinFrequency = 30,
		MinModulation = 5,
		SineScale = 1000
    };
	
    /** Create an empty tone buffer. Tones added will use the specified master volume. */
    Tones(unsigned AMasterVolume = MaxVolume, unsigned ASampleRate = DefaultSampleRate);

    /** Generate a tone using the specified values.
	 The operation parameter may be '+', 'x', '-' or ' ' for summing, modulation, pure tone or silence resepctively.
	 */
    bool Generate(char AOperation, unsigned AFrequency1, unsigned AFrequency2,				  
				  unsigned ADurationMilliseconds, unsigned AVolume = MaxVolume);
    bool Silence  (unsigned milliseconds);
	
protected:
    void Construct();
	
    bool Juxtapose(unsigned frequency1, unsigned frequency2, unsigned milliseconds, unsigned volume);
    bool Modulate (unsigned frequency, unsigned modulate, unsigned milliseconds, unsigned volume);
    bool PureTone (unsigned frequency, unsigned milliseconds, unsigned volume);
    //bool Silence  (unsigned milliseconds);
	
    unsigned CalcSamples(unsigned milliseconds, unsigned frequency1, unsigned frequency2 = 0);
	
    void AddSample(int sample, unsigned volume);
	
    unsigned m_sampleRate;
    unsigned m_maxFrequency;
    unsigned m_masterVolume;
    char     m_lastOperation;
    unsigned m_lastFrequency1, m_lastFrequency2;
    int      m_angle1, m_angle2;

	
};

#endif


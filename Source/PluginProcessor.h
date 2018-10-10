/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"


#define CLIPCAP_BUFFER_SIZE 3.0  // size of the record circular buffer in seconds
#define NUM_CLIPCAP_VOICES 32  // polyphony
#define NUM_SAMPLE_BUFFERS 128  // one for each midi note
#define CLIPCAP_MINIMUM_SAMPLE_TIME 0.2  // minimum sample length in seconds
#define CLIPCAP_LEVEL_THRESHOLD 0.01  // the slow envelope must be greater than this to trigger
#define CLIPCAP_TRIGGER_RATIO 1.14  // the ratio of fast envelope to slow to trigger a recording
#define CLIPCAP_SAMPLE_STOP_THRESHOLD 0.03  // the ratio of fast envelope to slow to end a recording

class ClipCapVoice
{
public:
	ClipCapVoice();
	~ClipCapVoice();

	// Trigger a new sample
	void Play(AudioBuffer<float> *sample, int preRoll);

	// Render output to the buffer if a note is playing
	void Process(AudioBuffer<float> &buffer);

	// True if this voice is playing a sample, false if it's available
	int IsPlaying(void) { return m_index >= 0; }

protected:
	AudioBuffer<float> *m_sample;
	int m_index;
	int m_preRoll;
};


//==============================================================================
/**
*/
class ClipCapAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    ClipCapAudioProcessor();
    ~ClipCapAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;

	void reset();

    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

	float SinglePoleCoeff(float cutoff, double sampleRate);
	float getEnvSlow() { return m_envSlow; }
	float getEnvFast() { return m_envFast; }
	float getEnvRecord() { return m_envRecord; }
	int getSampleStartIndex() { return m_sampleStartIndex; }
	int nextNote();

private:

	double m_sampleRate;
	AudioBuffer<float> recordBufferFloat;
	int m_recordIndex;

	AudioBuffer<float> m_sampleBuffers[NUM_SAMPLE_BUFFERS];
	int m_sampleBufferIndex;


	float m_envSlow;
	float m_envFast;
	float m_envSlowCoeff;
	float m_envFastCoeff;
	float m_envRecord;
	float m_envRecordCoeff;
	int m_sampleStartIndex;


	ClipCapVoice m_voices[NUM_CLIPCAP_VOICES];



    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClipCapAudioProcessor)
};

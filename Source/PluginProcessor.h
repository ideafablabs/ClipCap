/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"


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

private:

	double m_sampleRate;
	AudioBuffer<float> recordBufferFloat;
	uint32 m_recordIndex;

	float m_envSlow;
	float m_envFast;
	float m_envSlowCoeff;
	float m_envFastCoeff;
	uint32 m_sampleStartIndex;



    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClipCapAudioProcessor)
};

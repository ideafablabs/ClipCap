/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
ClipCapAudioProcessor::ClipCapAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

ClipCapAudioProcessor::~ClipCapAudioProcessor()
{
}

//==============================================================================
const String ClipCapAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ClipCapAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ClipCapAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ClipCapAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ClipCapAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ClipCapAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ClipCapAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ClipCapAudioProcessor::setCurrentProgram (int index)
{
}

const String ClipCapAudioProcessor::getProgramName (int index)
{
    return {};
}

void ClipCapAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void ClipCapAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

	/////samplePlayer.setCurrentPlaybackSampleRate(newSampleRate);
	/////keyboardState.reset();

	m_sampleRate = sampleRate;

	float recordBufferSeconds = 4.0;
	recordBufferFloat.setSize(2, (int)(sampleRate * recordBufferSeconds));
	reset();
}

float ClipCapAudioProcessor::SinglePoleCoeff(float cutoff, double sampleRate)
{
	return expf(-2.0 * MathConstants<float>::pi * (cutoff / sampleRate));
}

void ClipCapAudioProcessor::reset()
{
	// Use this method as the place to clear any delay lines, buffers, etc, as it
	// means there's been a break in the audio's continuity.
	recordBufferFloat.clear();
	m_recordIndex = 0;

	m_envFast = 0.0;
	m_envSlow = 0.0;
	m_envFastCoeff = SinglePoleCoeff(10, m_sampleRate);
	m_envSlowCoeff = SinglePoleCoeff(0.5, m_sampleRate);
}

void ClipCapAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ClipCapAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void ClipCapAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Only 1 channel here, and will always only be one
    auto *channelData = buffer.getWritePointer(0);
	auto *recordData = recordBufferFloat.getWritePointer(0);

	for (uint32 i = 0; i < buffer.getNumSamples(); i++)
	{
		float sample = channelData[i];
		recordData[m_recordIndex] = sample;

		float rms = sqrtf(sample * sample);
		m_envFast = (m_envFast * m_envFastCoeff) + (rms * (1.0 - m_envFastCoeff));
		m_envSlow = (m_envSlow * m_envSlowCoeff) + (rms * (1.0 - m_envSlowCoeff));

		if (++m_recordIndex > recordBufferFloat.getNumSamples())
			m_recordIndex = 0;
	}
}

//==============================================================================
bool ClipCapAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* ClipCapAudioProcessor::createEditor()
{
    return new ClipCapEditor (*this);
}

//==============================================================================
void ClipCapAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ClipCapAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ClipCapAudioProcessor();
}

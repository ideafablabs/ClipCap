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

	float recordBufferSeconds = CLIPCAP_BUFFER_SIZE;
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
	m_sampleStartIndex = -1;

	m_sampleBufferIndex = 0;
	for (int i = 0; i < NUM_SAMPLE_BUFFERS; i++)
		m_sampleBuffers[i].setSize(1, 1);

	m_envFast = 0.0;
	m_envSlow = 0.0;
	m_envFastCoeff = SinglePoleCoeff(10, m_sampleRate);
	m_envSlowCoeff = SinglePoleCoeff(0.5, m_sampleRate);
	m_envRecord = 0.0;
	m_envRecordCoeff = SinglePoleCoeff(0.5, m_sampleRate);
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

	// Only 1 channel here, and will always only be one
	jassert(totalNumInputChannels == 1 && totalNumOutputChannels == 1);

    auto *channelData = buffer.getWritePointer(0);
	auto *recordData = recordBufferFloat.getWritePointer(0);

	// Process the input and save clips to the sample pool
	int i;
	for (i = 0; i < buffer.getNumSamples(); i++)
	{
		float sample = channelData[i];
		recordData[m_recordIndex] = sample;

		float rms = sqrtf(sample * sample);
		m_envFast = (m_envFast * m_envFastCoeff) + (rms * (1.0 - m_envFastCoeff));
		m_envSlow = (m_envSlow * m_envSlowCoeff) + (rms * (1.0 - m_envSlowCoeff));

		if ((m_envSlow > CLIPCAP_LEVEL_THRESHOLD) && (m_envFast / m_envSlow > CLIPCAP_TRIGGER_RATIO))
		{
			m_envRecord = 1.0;  // recent triggers reset to 1.0
			if (m_sampleStartIndex == -1)
			{
				m_sampleStartIndex = m_recordIndex - (int)(m_sampleRate * 0.25); // set the start position back a bit to be less abrupt
				                                             //  ^^^  this delay should be something musical, like a quarter note, or a measure.  FIXME 
				if (m_sampleStartIndex < 0)
					m_sampleStartIndex += recordBufferFloat.getNumSamples(); // wrap
			}
		}
		else
		{
			m_envRecord *= m_envRecordCoeff;  // decay when not triggered
			if (m_sampleStartIndex > 0)
			{
				if (m_envRecord < CLIPCAP_SAMPLE_STOP_THRESHOLD || m_recordIndex == m_sampleStartIndex)
				{
					// store the sample
					uint32 sampleIdx = m_sampleBufferIndex++;
					if (m_sampleBufferIndex >= NUM_SAMPLE_BUFFERS)
						m_sampleBufferIndex = 0;
					int sampleSize = m_recordIndex - m_sampleStartIndex;
					if (sampleSize < 0)
						sampleSize += recordBufferFloat.getNumSamples(); // wrap
					else if (sampleSize == 0)
						sampleSize = recordBufferFloat.getNumSamples(); // full buffer
					if (sampleSize >= m_sampleRate * CLIPCAP_MINIMUM_SAMPLE_TIME)
					m_sampleBuffers[sampleIdx].setSize(1, sampleSize);

					if (m_recordIndex > m_sampleStartIndex) // no wrap
					{
						m_sampleBuffers[sampleIdx].copyFrom(0, 0, recordBufferFloat.getReadPointer(0, m_recordIndex), sampleSize);
					}
					else  // wrap
					{
						m_sampleBuffers[sampleIdx].copyFrom(0, 0, recordBufferFloat.getReadPointer(0, m_recordIndex), recordBufferFloat.getNumSamples() - m_sampleStartIndex);
						m_sampleBuffers[sampleIdx].copyFrom(0, recordBufferFloat.getNumSamples() - m_sampleStartIndex, recordBufferFloat.getReadPointer(0), m_recordIndex);
					}

					m_sampleStartIndex = -1;
				}
			}
		}

		if (++m_recordIndex > recordBufferFloat.getNumSamples())
			m_recordIndex = 0;
	}

	// Process the MIDI commands and trigger new samples
	MidiBuffer::Iterator midiIterator(midiMessages);
	MidiMessage currentMessage;
	int samplePosition = -1;
	while (midiIterator.getNextEvent(currentMessage, samplePosition))
	{
		if (currentMessage.isNoteOn())
		{
			int note_id = nextNote();
			if (note_id >= 0)
			{
				m_voices[note_id].Play(&m_sampleBuffers[(m_sampleBufferIndex - currentMessage.getNoteNumber() - 1) & 0x7F], samplePosition); // 0x7F to limit to MIDI's 7-bit 0-127 range
			}
		}
	}

	// Render any playing samples to the output
	buffer.clear();
	for (i = 0; i < NUM_CLIPCAP_VOICES; i++)
		m_voices[i].Process(buffer);

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

int ClipCapAudioProcessor::nextNote()
{
	for (int i = 0; i < NUM_CLIPCAP_VOICES; i++)
	{
		if (!m_voices[i].IsPlaying())
			return i;
	}
	return -1;
}



ClipCapVoice::ClipCapVoice()
{
	m_index = -1;
}
ClipCapVoice::~ClipCapVoice()
{
}

void ClipCapVoice::Play(AudioBuffer<float> *sample, int preRoll)
{
	m_sample = sample;
	m_index = 0;
	m_preRoll = preRoll;
}

void ClipCapVoice::Process(AudioBuffer<float> &buffer)
{
	if (m_index < 0)
		return;
	if (m_preRoll > 0 && m_preRoll > buffer.getNumSamples())
	{
		m_preRoll -= buffer.getNumSamples();
		return;
	}
	float *bufferPtr = buffer.getWritePointer(0);
	const float *sample = m_sample->getReadPointer(0);
	for (int i = m_preRoll; i < buffer.getNumSamples() && m_index < m_sample->getNumSamples(); i++, m_index++)
		bufferPtr[i] += sample[m_index];
	if (m_index >= m_sample->getNumSamples())
		m_index = -1;  // Done!
	m_preRoll = 0;
}
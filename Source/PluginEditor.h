/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"


//==============================================================================
/**
*/
class ClipCapEditor  : public AudioProcessorEditor, private Timer
{
public:
    ClipCapEditor (ClipCapAudioProcessor&);
    ~ClipCapEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
	void timerCallback() override;

	float convertMeterScale(float value);

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
	class DisplaySlider;
	ScopedPointer<DisplaySlider> m_envSlowSlider, m_envFastSlider;

    ClipCapAudioProcessor& processor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClipCapEditor)
};

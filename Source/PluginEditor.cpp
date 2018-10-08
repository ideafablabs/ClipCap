/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"



class ClipCapEditor::DisplaySlider : public Slider
{
public:
	DisplaySlider(ClipCapAudioProcessor& p) : m_audioProc(p), Slider("DispalySlider")
	{
		setRange(0.0, 1.0, 0.0);
	}
private:
	ClipCapAudioProcessor& m_audioProc;
};


//==============================================================================
ClipCapEditor::ClipCapEditor (ClipCapAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
	addAndMakeVisible(m_envSlowSlider = new DisplaySlider(p));
	m_envSlowSlider->setSliderStyle(Slider::LinearVertical);
	m_envSlowSlider->setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
	addAndMakeVisible(m_envFastSlider = new DisplaySlider(p));
	m_envFastSlider->setSliderStyle(Slider::LinearVertical);
	m_envFastSlider->setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
	addAndMakeVisible(m_recordSlider = new DisplaySlider(p));
	m_recordSlider->setSliderStyle(Slider::LinearVertical);
	m_recordSlider->setTextBoxStyle(Slider::NoTextBox, true, 0, 0);

	setSize(200, 200);

	startTimerHz(30);
}

ClipCapEditor::~ClipCapEditor()
{
}

//==============================================================================
void ClipCapEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

    g.setColour (Colours::white);
    g.setFont (15.0f);
    //g.drawFittedText ("Hello World!", getLocalBounds(), Justification::centred, 1);
}

void ClipCapEditor::resized()
{
    // Update component layout here.
	Rectangle<int> r(getLocalBounds().reduced(8));

	//Rectangle<int> sliderArea(r.removeFromTop(60));
	//m_displaySlider->setBounds(sliderArea.removeFromLeft(jmin(180, sliderArea.getWidth())));
	m_envSlowSlider->setBounds(r.removeFromLeft(jmin(60, r.getWidth())));
	m_envFastSlider->setBounds(r.removeFromLeft(jmin(60, r.getWidth())));
	m_recordSlider->setBounds(r.removeFromLeft(jmin(60, r.getWidth())));

}

float ClipCapEditor::convertMeterScale(float value)
{
	// return a value between 0 and 1 that maps to -120dB to 0dB
	float db = 20.0*logf(value);
	value = (120.0 + db) / 120.0;
	if (value < 0)
		return 0;
	return value;
}

void ClipCapEditor::timerCallback()
{
	m_envSlowSlider->setValue(convertMeterScale(processor.getEnvSlow()));
	m_envFastSlider->setValue(convertMeterScale(processor.getEnvFast()));
	m_recordSlider->setValue(convertMeterScale(processor.getEnvRecord()));
}
/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TutorialPluginAudioProcessorEditor::TutorialPluginAudioProcessorEditor (TutorialPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    // 
    // CONSTRUCTOR OF AudioProcessorEditor

    setSize (200, 300);
    // these define the parameters of our slider object
    midiVolume.setSliderStyle(juce::Slider::LinearBarVertical);
    midiVolume.setRange(0.0, 127.0, 1.0);
    midiVolume.setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 0);
    midiVolume.setPopupDisplayEnabled(true, false, this);
    midiVolume.setTextValueSuffix(" Volume");
    midiVolume.setValue(1.0);

    // this function adds the slider to the editor
    addAndMakeVisible(&midiVolume);
    // add the listener to the slider
    midiVolume.addListener(this);
    // In JUCE, buttons, sliders, and many other types of controls that may need 
    // to inform other objects about a change in their state are a type of 
    // broadcaster object.In order to respond to changes in a broadcaster object,
    // other classes need to be a listener for that specific type of broadcaster
}

void TutorialPluginAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    audioProcessor.noteOnVel = midiVolume.getValue();
}

TutorialPluginAudioProcessorEditor::~TutorialPluginAudioProcessorEditor()
{
    // DECONSTRUCTOR
}

//==============================================================================
void TutorialPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Midi Volume",0,0, getWidth(),30, juce::Justification::centred, 1); 

    // getLocalBounds() returns x,y, width and height parameters, returns a type rectangle 
}

void TutorialPluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    // sets the position and size of the slider with arguments (x, y, width, height)

    midiVolume.setBounds(40, 30, 20, getHeight() - 60);
}

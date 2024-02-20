/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ClassicBiquadFiltersAudioProcessorEditor::ClassicBiquadFiltersAudioProcessorEditor(ClassicBiquadFiltersAudioProcessor& p,
    juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor(&p), audioProcessor(p), valueTreeState(vts)
{
    setSize(480, 300);
}

ClassicBiquadFiltersAudioProcessorEditor::~ClassicBiquadFiltersAudioProcessorEditor()
{
}

//==============================================================================
void ClassicBiquadFiltersAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void ClassicBiquadFiltersAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    int widthTri = 120;
    int heightTri = 120;

    cutOffLabel.setBounds(0, 0, widthTri, 30);
    qLabel.setBounds(widthTri, 0, widthTri, 30);
    dryWetLabel.setBounds(2 * widthTri, 0, widthTri, 30);

    cutOffSlider.setBounds(0, 30, widthTri, 90);
    qSlider.setBounds(widthTri, 30, widthTri, 90);
    dryWetSlider.setBounds(2 * widthTri, 30, widthTri, 90);

    dBLabel.setBounds(0, heightTri, widthTri, 30);
    dBSlider.setBounds(widthTri, heightTri, 250 + dBSlider.getTextBoxWidth(), 30);


    LPF1textButton.setBounds(3 * widthTri + 10, 10, 100, 20);
    LPF2textButton.setBounds(3 * widthTri + 10, 10 + 1 * (25), 100, 20);
    HPF1textButton.setBounds(3 * widthTri + 10, 10 + 2 * (25), 100, 20);
    HPF2textButton.setBounds(3 * widthTri + 10, 10 + 3 * (25), 100, 20);
}

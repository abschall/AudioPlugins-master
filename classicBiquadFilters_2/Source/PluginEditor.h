/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "abschallLookAndFeel_Sliders.h"

class ClassicBiquadFilters_2AudioProcessorEditor  : public juce::AudioProcessorEditor
{    
public:
    void ClassicBiquadFilters_2AudioProcessorEditor::filterStyleSelection();
    typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;
    typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;

    ClassicBiquadFilters_2AudioProcessorEditor (ClassicBiquadFilters_2AudioProcessor& p, juce::AudioProcessorValueTreeState& vts);
    ~ClassicBiquadFilters_2AudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:

    juce::AudioProcessorValueTreeState& valueTreeState;
    ClassicBiquadFilters_2AudioProcessor& audioProcessor;

    // GUI elements
    rotaryPot cutOffPot;
    rotaryPot qPot;
    rotaryPot dryWetPot;
    vector<rotaryPot*> pots = { &cutOffPot, &qPot, &dryWetPot };

    juce::Label cutOffLabel;
    juce::Label qLabel;
    juce::Label dryWetLabel;
    vector<juce::Label*> titleLabels = { &cutOffLabel ,&qLabel, &dryWetLabel };

    std::unique_ptr<SliderAttachment> cutOffAttachment;
    std::unique_ptr<SliderAttachment> qAttachment;
    std::unique_ptr<SliderAttachment> dryWetAttachment;

    juce::ComboBox filterTypeChoice;
    std::unique_ptr<ComboBoxAttachment> filterTypeChoiceAttachment;

    // components sizes
    int wPot = 120, heightPot = 90;
    int wLabel = 120, heightLabel = 30;
    int widthBox = wPot;
    int heightBox = heightPot + heightLabel;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClassicBiquadFilters_2AudioProcessorEditor)
};

/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class DecibelSlider : public juce::Slider
{
public:
    DecibelSlider() {}
    double getValueFromText(const juce::String& text) override
    {
        // receives text instring format
        // Juce indicated "INF" for values ofdecibel below -100.0 dB

        auto minusInfinitydB = -100.0;
        auto decibelText = text.upToFirstOccurrenceOf("dB", false, false).trim();    // [1]
        return decibelText.equalsIgnoreCase("INF") ? minusInfinitydB : decibelText.getDoubleValue(); // [2]
        // compares if the trimmed Text is equal to "INF", if yes return double -100, else
        // return the value in double of decibelText
    }

    juce::String getTextFromValue(double value) override
    {
        return juce::Decibels::toString(value);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DecibelSlider)
};


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
    juce::Slider cutOffSlider;
    std::unique_ptr<SliderAttachment> cutOffAttachment;
    juce::Label cutOffLabel;

    juce::Slider qSlider;
    std::unique_ptr<SliderAttachment> qAttachment;
    juce::Label qLabel;

    juce::Slider dryWetSlider;
    std::unique_ptr<SliderAttachment> dryWetAttachment;
    juce::Label dryWetLabel;

    juce::ComboBox filterTypeChoice;
    std::unique_ptr<ComboBoxAttachment> filterTypeChoiceAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClassicBiquadFilters_2AudioProcessorEditor)
};

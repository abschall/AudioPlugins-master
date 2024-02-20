/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

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

//==============================================================================

class ClassicBiquadFiltersAudioProcessorEditor  : public juce::AudioProcessorEditor
{

public:
    typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachement;
    typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachement;

    ClassicBiquadFiltersAudioProcessorEditor(ClassicBiquadFiltersAudioProcessor&,
        juce::AudioProcessorValueTreeState&);
    ~ClassicBiquadFiltersAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::AudioProcessorValueTreeState& valueTreeState;
    ClassicBiquadFiltersAudioProcessor& audioProcessor;

    // GUI elements
    DecibelSlider dBSlider;
    juce::Label dBLabel;


    juce::Slider cutOffSlider;

    juce::Label cutOffLabel;

    juce::Slider qSlider;

    juce::Label qLabel;

    juce::Slider dryWetSlider;
    juce::Label dryWetLabel;

    juce::TextButton LPF1textButton;
    juce::TextButton HPF1textButton;
    juce::TextButton LPF2textButton;
    juce::TextButton HPF2textButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClassicBiquadFiltersAudioProcessorEditor)
};

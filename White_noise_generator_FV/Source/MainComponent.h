#pragma once

#include <JuceHeader.h>
#include <vector>
using std::vector;
// #include "biquad.h"
#include "dsp_fv/biquad.h"
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



class MainComponent  : public juce::AudioAppComponent,
    public Biquad, public juce::Button::Listener
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    //==============================================================================
    void updateFilterParameters();
    void buttonClicked(juce::Button* button) override;

private:
    //==============================================================================
    // Your private member variables go here...
    juce::Random random;


    float level = 0.0f;
    Biquad biquadFilter;
    float currentSampleRate; 

    ClassicFilters filter;
    float fc = 10.0f, Q = 1.0f, K = 1.0f;

    //==============================================================================
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::AudioAppComponent
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

    void updateAngleDelta();
    void updateLevelDelta();

private:
    //==============================================================================
    // Your private member variables go here...
    double currentSampleRate = 0.0, currentAngle = 0.0, angleDelta = 0.0; // [1]
    juce::Slider freqSlider;
    juce::Label freqLabel;
    juce::Slider levelSlider;
    juce::Label levelLabel;
    double currentLevel = 1.0f, targetLevel = 1.0f;
    double level = 0.125f; // rename variable to absoluteLevel
    double currentFrequency = 500.0, targetFrequency = 500.0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};



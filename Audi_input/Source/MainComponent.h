#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::AudioAppComponent,
                       public juce::ChangeListener
{
public:
    //==============================================================================
    MainComponent::MainComponent() :// hide advanced options
        audioSetupComp(deviceManager,
            0,     // minimum input channels
            256,   // maximum input channels
            0,     // minimum output channels
            256,   // maximum output channels
            false, // ability to select midi inputs
            false, // ability to select midi output device
            false, // treat channels as stereo pairs
            false)
        // see how to add this function in the s ource file instead of header file (see C++ Primer)
    {
        // Make sure you set the size of the component after
        // you add any child components.
        setSize(800, 600);
        setAudioChannels(2, 2);

        addAndMakeVisible(audioSetupComp);
        deviceManager.addChangeListener(this);

        //Defining juce::components
        addAndMakeVisible(NoiseLevelSlider);
        NoiseLevelSlider.setRange(0.0, 1.0);
        NoiseLevelSlider.setValue(0.0);
        NoiseLevelSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 160, NoiseLevelSlider.getTextBoxHeight());
        NoiseLevelSlider.onValueChange = [this]() {
            targetNoiseLevel = NoiseLevelSlider.getValue();
            };

        addAndMakeVisible(NoiseLevelLabel);
        NoiseLevelLabel.attachToComponent(&NoiseLevelSlider, true);
        NoiseLevelLabel.setText("Noise Level", juce::dontSendNotification);
    }

    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    void changeListenerCallback(juce::ChangeBroadcaster*) override
    {

    }
    //==============================================================================
    // Your private member variables go here...
    juce::Slider NoiseLevelSlider;
    juce::Label NoiseLevelLabel;
    juce::Random random;
    double targetNoiseLevel = 0.0f, currentNoiseLevel = 0.0f;
    double level = 1.0f;
    juce::AudioDeviceSelectorComponent audioSetupComp;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

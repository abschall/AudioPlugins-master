#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::Component, 
    public juce::Slider::Listener,
    public juce::Button::Listener
    // public juce::Label::Listener
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void sliderValueChanged(juce::Slider*) override;
    void buttonClicked(juce::Button*) override;

private:
    //==============================================================================
    // Your private member variables go here...
    juce::Slider Freq_slider;
    juce::Slider Duration_slider;
    juce::Label Freq_label;
    juce::Label Duration_label;
    
    juce::TextButton Reset_button;
    juce::Label Title_label;



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

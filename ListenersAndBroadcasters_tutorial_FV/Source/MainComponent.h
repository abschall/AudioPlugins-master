#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::Component,    
                       public juce::Button::Listener  // [1]
                        // in order to use a Button:Lister  they need 
                        // to be added as a sub-class to the MainComponent Class.
{
public:
    //==============================================================================
    MainComponent();     // can never be virtual 
    ~MainComponent() override; // the juce::Component  destructor is overrident by the MainComponent destructor()

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    // added method cheking if the button has been clicked 
    void buttonClicked(juce::Button* button) override;  // I dont know why the buttonCLicked function should specify override

private:
    //==============================================================================
    // Your private member variables go here...
    
    // 2 child components
    juce::TextButton currentDateAndTime_button; // can display a button containing some specific text
    juce::Label currentDateAndTime_label; //  can display a piece of text
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

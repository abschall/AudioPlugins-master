#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    addAndMakeVisible(Title_label);
    Title_label.setFont(juce::Font(16.0f, juce::Font::bold)); 
    Title_label.setColour(juce::Label::textColourId, juce::Colours::white);
    Title_label.setText("Setting some frequencies", juce::dontSendNotification);
    Title_label.setJustificationType(juce::Justification::centred);


    addAndMakeVisible(Freq_slider);
    Freq_slider.addListener(this); // adds a listener to the Freq_slider 
    Freq_slider.setRange(50, 20000); // sets the range of the slider 
    Freq_slider.setTextValueSuffix("Hz"); // appends the suffix at the end of the slider's text box number 
    Freq_slider.setValue(500.0f); // sets the initial value of the slider, which will in turn set the value if the duration slider 
    Freq_slider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 160, Freq_slider.getTextBoxHeight());
    Freq_slider.setSkewFactorFromMidPoint(500); // skewing the value at midpoint, sets the value at midpoint to 500 Hz
    // no notification 

    addAndMakeVisible(Freq_label);
    Freq_label.setText("Frequency", juce::dontSendNotification);
    Freq_label.attachToComponent(&Freq_slider, true); // attach the label to the slider 
    // the Freq_Label does not need to be positionned using the resize() function, because it is bound to Freq_slider 

    addAndMakeVisible(Reset_button);
    Reset_button.setButtonText("Reset Frequency");
    Reset_button.addListener(this);

    addAndMakeVisible(Duration_slider);
    Duration_slider.addListener(this);
    Duration_slider.setRange(1 / Freq_slider.getMaximum(), 1 / Freq_slider.getMinimum());
    Duration_slider.setTextValueSuffix("s");
    Duration_slider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 160, Duration_slider.getTextBoxHeight());
    Duration_slider.setSkewFactorFromMidPoint(0.002);
    
    addAndMakeVisible(Duration_label);
    Duration_label.setText("Duration", juce::dontSendNotification);
    Duration_label.attachToComponent(&Duration_slider, true);
    setSize (600, 100);
}

MainComponent::~MainComponent()
{
    // Removing listeners appropriately
    Freq_slider.removeListener(this);
    Duration_slider.removeListener(this);
    
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    /*
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setFont (juce::Font (16.0f));
    g.setColour (juce::Colours::white);
    g.drawText ("Hello World!", getLocalBounds(), juce::Justification::centred, true);*/
}

void MainComponent::resized() 
{
    int sliderLeft = 120;
    // see documentation for proper understanding of the rectangle sub class 
    Freq_slider.    setBounds(sliderLeft, 20, getWidth() - sliderLeft - 10, 20);
    Duration_slider.setBounds(sliderLeft, 50, getWidth() - sliderLeft - 10, 20);
    Reset_button.   setBounds(sliderLeft, 80, getWidth() - sliderLeft - 10, 20);
    Title_label.    setBounds(0, 0, getWidth(), 0);
    // setBounds( int x, int y, int width, int height)
        
}
void MainComponent::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &Freq_slider) // update the value of the other slider according to the value of the first slider 

    {
        Duration_slider.setValue(1.0 / Freq_slider.getValue(), juce::dontSendNotification);
    }
    else if (slider == &Duration_slider)
    {
        Freq_slider.setValue(1.0 / Duration_slider.getValue(), juce::dontSendNotification);
    }
    // both value are changed within the statement of the other one
}

void MainComponent::buttonClicked(juce::Button* button)
{
    if (button == &Reset_button) {        
        Freq_slider.setValue(666, juce::dontSendNotification);
        sliderValueChanged(&Freq_slider);
    }


}
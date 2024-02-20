#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    // Adding Button and Label to the window and displaying
    addAndMakeVisible(currentDateAndTime_button); // use this function to add any component to the display window
    currentDateAndTime_button.setButtonText("Checking current Time and Date");

    // In order to receive the messages that are broadcast, we need to register our listener
    // object with one or more broadcaster objects. In this case, we need to register with 
    // the TextButton object.
    currentDateAndTime_button.addListener(this); // adds a listener to the specfied button 

    // Many broadcaster objects in JUCE contain a nested Listener class, from which we can inherit, 
    // in order to become a listener for that type of broadcaster.


    addAndMakeVisible(currentDateAndTime_label);
    currentDateAndTime_label.setColour(juce::Label::backgroundColourId, juce::Colours::black);
    currentDateAndTime_label.setColour(juce::Label::textColourId, juce::Colours::white);
    currentDateAndTime_label.setJustificationType(juce::Justification::centred);

    setSize (600, 110);
}

MainComponent::~MainComponent()
{
    currentDateAndTime_button.removeListener(this);
    // Removing listeners appropriately is important if you set up more complex broadcaster-listener systems.
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{

    // (Our component is opaque, so we must completely fill the background with a solid colour)
    /*
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    
    g.setFont (juce::Font (16.0f));
    g.setColour (juce::Colours::white);
    g.drawText ("Hello World!", getLocalBounds(), juce::Justification::centred, true);
    */
}

void MainComponent::resized()
{
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update (but also define their positions with respect to the main page.
    currentDateAndTime_button.setBounds(10, 10, getWidth() - 20, 40);
    currentDateAndTime_label.setBounds(10, 60, getWidth() - 20, 40);
}

void MainComponent::buttonClicked(juce::Button* button) 
{
    if (button == &currentDateAndTime_button)                                  // [3]
    {
        auto currentTime = juce::Time::getCurrentTime();                // [4]

        auto includeDate = true;
        auto includeTime = true;
        auto currentTimeString = currentTime.toString(includeDate, includeTime);           // [5]

        currentDateAndTime_label.setText(currentTimeString, juce::dontSendNotification);      // [6]

        /*
        [3] compare if the clicked button corresponds to the currentDateAndTime_button,
        we compare if the address of the pointer passed to the function corresponds to the clicked
        Button's (currentDateAndTime_button) address, if that is the case we execute the if statement
        [4] get the current Date from the juce::Time class, which gets it from the OS
         get the Time  from the OS
        [5] change the date and Time to a string which we then pass to the Label [6] the text displayed by
        the label is updated 
        */
    }
}

#include "MainComponent.h"

//==============================================================================


MainComponent::MainComponent()
{
    setSize (800, 200);
    setAudioChannels (0, 2); // 2 output channels

    addAndMakeVisible(freqSlider);
    freqSlider.setRange(50, 5000);
    freqSlider.setValue(currentFrequency, juce::dontSendNotification);
    freqSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 120, freqSlider.getTextBoxHeight());
    freqSlider.setSize(300, freqSlider.getTextBoxHeight());
    freqSlider.setTextValueSuffix("Hz"); 
    freqSlider.setSkewFactorFromMidPoint(500.0f);
    
    // Obsolete funtion 
    /*freqSlider.onValueChange = [this]() // main component this, because we are inside the scope of MainComponent
        {
            // onValueChange : function callback object
            if (currentSampleRate > 0.0)
                updateAngleDelta();
                //this function is used when the frequency is not updated smoothly 
        };
        */
    freqSlider.onValueChange = [this]() { 
        targetFrequency = freqSlider.getValue();
        };
   
    addAndMakeVisible(freqLabel);
    freqLabel.attachToComponent(&freqSlider, true);
    freqLabel.setText("Frequency", juce::dontSendNotification);

    addAndMakeVisible(levelSlider);
    levelSlider.setRange(0, 1);
    levelSlider.setValue(currentLevel, juce::dontSendNotification);
    levelSlider.setSize(300, levelSlider.getTextBoxHeight());
    levelSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 120, freqSlider.getTextBoxHeight());
    levelSlider.onValueChange = [this]() {targetLevel = levelSlider.getValue();};
   
    addAndMakeVisible(levelLabel);
    levelLabel.attachToComponent(&levelSlider,true);
    levelLabel.setText("Level", juce::dontSendNotification);
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

void MainComponent::updateAngleDelta()
{
    auto cyclesPerSample = freqSlider.getValue() / currentSampleRate;      // [2]
    angleDelta = cyclesPerSample * 2.0 * juce::MathConstants<double>::pi;  // [3]

    // [2] : First we calculate the number of cycles that will need to complete for each output sample.
    // [3] : Then this is multiplied by the length of a whole sine wave cycle, which is 2pi radians.

}
//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.
    currentSampleRate = sampleRate;
    updateAngleDelta();
}
/*
void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    // this method of addressing the L andR buffers does not work any more,
    // both buffers L and R need to be addressed at the same time,
    // other zise every buffer will be active only have the time and during the time that the other buffer is processed
    // the last sample is repeated until the for loop passes on to processing that buffer 

    for (auto channel = 0; channel != bufferToFill.buffer->getNumChannels(); ++channel)
    {
    // buffers filled sequencially L R L R L R .... (would workf if sample to sample processing 
    // would be performed rather than AudioBlock processing / filling 
  
        auto* buffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);
        for (auto sample = bufferToFill.startSample;sample != bufferToFill.numSamples; ++sample)
        {
            auto currentSample = std::sin(currentAngle);
            currentAngle += angleDelta;
            buffer[sample] = level * currentSample;
        }
    }

}*/

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    // buffers have to be adressed in parallel during the process of an Audio Block

    auto* bufferL = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
    auto* bufferR = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);

    auto localTargetFrequency = targetFrequency; //[6]
    auto localTargetLevel = targetLevel; 
    if (!juce::approximatelyEqual(localTargetFrequency, currentFrequency)) // [7]
    {
        auto frequencyIncrement = (localTargetFrequency - currentFrequency) / bufferToFill.numSamples; // [8]

        for (auto sample = bufferToFill.startSample;sample != bufferToFill.numSamples; ++sample)
        {
            
            auto currentSample = (float) std::sin(currentAngle);
            currentFrequency += frequencyIncrement;  // [9]
            updateAngleDelta(); // [10]
            currentAngle += angleDelta;
            bufferL[sample] = currentLevel * level * currentSample;
            bufferR[sample] = currentLevel * level * currentSample;
        }
        currentFrequency = localTargetFrequency;
    }
    else    // [11]
    {
        if (!juce::approximatelyEqual(localTargetLevel, currentLevel))
            // the added if case is only added to the case when the current frequency is equa to the target frequency
            // we make the assumption thaat the user only uses the mouse to change the sliders, so only one slier can
            // be changed at a time, if a midi controler were to be used then an additional case woul have to be added 

        {
            auto levelIncrement = (localTargetLevel - currentLevel) / bufferToFill.numSamples;
            for (auto sample = bufferToFill.startSample;sample != bufferToFill.numSamples; ++sample)
            {
                auto currentSample = (float)std::sin(currentAngle);
                currentLevel += levelIncrement;
                currentAngle += angleDelta;
                bufferL[sample] = currentLevel * level * currentSample;
                bufferR[sample] = currentLevel * level * currentSample;
            }
            currentLevel = localTargetLevel;
        }
        else {
            for (auto sample = bufferToFill.startSample;sample != bufferToFill.numSamples; ++sample)
                {
                
                    auto currentSample = (float) std::sin(currentAngle);
                    currentAngle += angleDelta;
                    bufferL[sample] = currentLevel * level * currentSample;
                    bufferR[sample] = currentLevel * level * currentSample;
                }
        }

    }
}

/*
[6] create a local copy of the target frequency variable just in case, the value changes duing a thread 
[7] check if the current frequency is equal the target frequency
[8] if not : create a variable called frequency increment, which corresponds to the difference between
target frequency and current frequency.
[9]the current frequency is incremented by the frequency increment and the phase is recalculated ([10])

[11] if taret frequency and curret frequency are exactly equal, the sine wzve is just normally output, 
no need to change the freqency 
*/

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // You can add your drawing code here!
}

void MainComponent::resized()
{
    freqSlider. setBounds(120, 20, getWidth() - 120 - 20, 20);
    levelSlider.setBounds(120, 50, getWidth() - 120 - 20, 20);
}

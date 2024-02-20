#include "MainComponent.h"

//==============================================================================


MainComponent::~MainComponent()
{
    deviceManager.removeChangeListener(this);
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{ }

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo & bufferToFill)
{
        // getting the  active Input and Output buffers 
        auto* device = deviceManager.getCurrentAudioDevice();
        auto activeInputChannels = device->getActiveInputChannels();
        auto activeOutputChannels = device->getActiveOutputChannels();

        // get number of actve Input and Output channels
        auto maxInputChannels = activeInputChannels.getHighestBit() + 1;
        auto maxOutputChannels = activeOutputChannels.getHighestBit() + 1;

        auto localTargetLevel = targetNoiseLevel;      
        for (auto channel = 0; channel < maxOutputChannels; ++channel)
        {
            if ((!activeOutputChannels[channel]) || (maxInputChannels == 0))
                // checking whether inpuot or output channels are active or not
            {
                bufferToFill.buffer->clear(channel, bufferToFill.startSample, bufferToFill.numSamples);
            }
            else
            {
                auto actualInputChannels = channel % maxOutputChannels; // [1]
                // checking if there are more active outout channels than inout channels 
                if (!activeInputChannels[channel])  // [2]
                {
                    // checking whether input channel is active or not 
                    bufferToFill.buffer->clear(channel, bufferToFill.startSample, bufferToFill.numSamples);
                }
                else
                {   // [3]
                    if (!juce::approximatelyEqual(localTargetLevel, currentNoiseLevel))
                    {
                        auto levelIncrement = (targetNoiseLevel - localTargetLevel)/bufferToFill.numSamples;
                        for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
                        {
                            auto* inBuffer = bufferToFill.buffer->getReadPointer(actualInputChannels, bufferToFill.startSample);
                            auto* outBuffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);

                            auto noise = random.nextFloat() * 2.0f - 1.0f;
                            currentNoiseLevel += levelIncrement;
                            outBuffer[sample] = inBuffer[sample] * currentNoiseLevel * level;
                        }
                        currentNoiseLevel = localTargetLevel;
                    }
                    else
                    {
                        for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
                        {
                            auto* inBuffer = bufferToFill.buffer->getReadPointer(actualInputChannels, bufferToFill.startSample);
                            auto* outBuffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);
                            auto noise = random.nextFloat() * 2.0f - 1.0f;
                            // outBuffer[sample] = inBuffer[sample] + (inBuffer[sample] * level * noise);
                            outBuffer[sample] = inBuffer[sample] * currentNoiseLevel * level;
                        }
                    }
                }

            }
        }
                /*
                [1]: We may have requested more output channels than input channels,
                     so our app needs to make a decision about what to do about these extra outputs.In this example
                     we simply repeat the input channels if we have more outputs than inputs.
                     (To do this we use the modulo operator to "wrap" our access to the input channels based on
                     the number of input channels available.) In other applications it may be more appropriate to
                     output silence for higher numbered channels where there are more output channels than input channels.
                [2]: Individual input channels may be inactive so we output silence in this case too.
                [3]: This final block actually does the processing!Here we get pointers to the input and output buffer
                     samples and add some noise to the input samples.
                */        
}
void MainComponent::releaseResources()
{
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    auto rect = getLocalBounds();

    audioSetupComp.setBounds(rect.removeFromLeft(proportionOfWidth(0.6f)));
    rect.reduce(10, 10);
    int sliderLeft = 120;
    NoiseLevelSlider.setBounds(sliderLeft, 20, getWidth() - sliderLeft - 20, 20);

}




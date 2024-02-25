/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../dsp_fv/biquad.h"
#include "../../dsp_fv/circularBuffer.h"
#include "MultiTapDelay.h"

//==============================================================================

class AnalogMultiTapDelayAudioProcessor  : public juce::AudioProcessor,
    private juce::Timer, public FilteredNoise 
{
public:
    //==============================================================================
    AnalogMultiTapDelayAudioProcessor();
    ~AnalogMultiTapDelayAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    //==============================================================================
    void timerCallback() override
    {
        stopTimer();
        delayBufferFilled = true;

    }
    //==============================================================================
    // necessary variables

    double currentSampleRate;
    juce::AudioProcessorValueTreeState parameters;
    float maxDelayTime = 5001.0;
    bool delayBufferFilled = false;
    
    // Overall members
    std::atomic<float>* mix = nullptr;
    std::atomic<float>* inputLevel = nullptr;
    std::atomic<float>* delay = nullptr;
    std::atomic<float>* width = nullptr;
    std::atomic<float>* feedback = nullptr;

    // Taps members
    std::atomic < float>* timeRatio = nullptr;
    vector< std::atomic<bool>*> tapSelector;
    vector< std::atomic<float>*> tapLevels;
    const unsigned int numberOfTaps = 4;

    // Color members
    std::atomic<float>* noiseLevel = nullptr;
    std::atomic<float>* saturation = nullptr;

    // Filter members
    std::atomic<float>* lowPass = nullptr;
    std::atomic<float>* highPass = nullptr;

    // the delay algorithm
    MultiTapDelay delayAlgorithm; // change with delay Algorithm

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalogMultiTapDelayAudioProcessor)
};

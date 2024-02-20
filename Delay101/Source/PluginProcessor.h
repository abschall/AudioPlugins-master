/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../dsp_fv/biquad.h"
#include "../../dsp_fv/circularBuffer.h"

//==============================================================================
/**
*/

class combFilterWithFB : private CircularBuffer<float>
{
public:   
    void setParameters( double pCurrentSampleRate,
                        double pDelayTimeMs,             
                        float pDry,
                        float pWet,
                        float pFeedbackGain)
    {
        currentSampleRate = pCurrentSampleRate;
        //auto delayTimeMs = pDelayTimeMs;
        //samplePerMsec = currentSampleRate / 1000.0;
        //delayTimeInSamples = delayTimeMs * samplePerMsec;
        setDelayTimeInMs(pDelayTimeMs);
        setDryWetLevels(pDry, pWet);
        setFeedbackGain(pFeedbackGain);
    }
    void createDelayBuffer(double pSampleRate, double pBufferLengthMsec)
    {
        bufferLengthMsec = pBufferLengthMsec;
        currentSampleRate = pSampleRate;
        samplePerMsec = currentSampleRate / 1000.0;
        bufferLength = (unsigned int) (bufferLengthMsec * samplePerMsec) + 1;
        delayBuffer.createBuffer(bufferLength);

    }
    void setDryWetLevels(float pDry, float pWet)
    {
        dry = pDry;
        wet = pWet;
    }
    void setFeedbackGain(float feedback)
    {
        feedbackGain = feedback;
    }
    void setDelayTimeInSamples(unsigned int pDelayTimeInSamples)
    {
        delayTimeInSamples = pDelayTimeInSamples;
    }
    void setDelayTimeInMs(unsigned int pDelayTimeInMs)
    {
        delayTimeInSamples = pDelayTimeInMs * samplePerMsec;
    }
    float processAudioSample(float inputXn)
    {
        auto ynD = delayBuffer.readBuffer(delayTimeInSamples);
        auto ynFullWet = inputXn + feedbackGain * ynD;
        delayBuffer.writeBuffer(ynFullWet);
        auto yn = dry * inputXn + wet * ynD;
        return yn;
    }

private:
    CircularBuffer delayBuffer;
    double currentSampleRate;
    double samplePerMsec;
    double bufferLengthMsec;
    unsigned int bufferLength; // in samples

    unsigned int delayTimeInSamples;
    
    float dry;
    float wet;
    float feedbackGain;
};

class Delay101AudioProcessor  : public juce::AudioProcessor, private CircularBuffer<float>
{
public:
    //==============================================================================
    Delay101AudioProcessor();
    ~Delay101AudioProcessor() override;

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
    CircularBuffer delayLine;
    juce::AudioParameterFloat* wetDry;
    juce::AudioParameterFloat* feedbackGain;
    juce::AudioParameterFloat* delayTime; // in ms
    combFilterWithFB delay;
    //unsigned int delaySamples;
    double currentSampleRate;
    float level = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Delay101AudioProcessor)
};

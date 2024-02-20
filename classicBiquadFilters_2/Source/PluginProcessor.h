/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "dsp_fv/biquad.h"
using std::vector;
//==============================================================================
/**
*/
class ClassicBiquadFilters_2AudioProcessor  : public juce::AudioProcessor, public Biquad
{
public:
    //==============================================================================
    ClassicBiquadFilters_2AudioProcessor();
    ~ClassicBiquadFilters_2AudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* ClassicBiquadFilters_2AudioProcessor::createEditor() override;
    // juce::AudioProcessorEditor* createEditor() override; // { return new juce::GenericAudioProcessorEditor(*this); }
    bool hasEditor() const override;// { return true; }
    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    // Custom methods 
    //void updateFilterParameters();
    //void buttonClicked(juce::Button* button) override;


    //==============================================================================
    // Program
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    // State Information
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

private:
    //==============================================================================
    float level = 0.0f;
    Biquad biquadFilter;
    float currentSampleRate;

    ClassicFilters filter;
    //juce::AudioParameterFloat* fc, * Q, * K;
    std::atomic<float>* fcParameter = nullptr;
    std::atomic<float>* QParameter = nullptr;
    std::atomic<float>* dryWetParameter = nullptr;

    juce::AudioProcessorValueTreeState parameters;

    //float fc = 10.0f, Q = 1.0f, K = 1.0f;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClassicBiquadFilters_2AudioProcessor)
};
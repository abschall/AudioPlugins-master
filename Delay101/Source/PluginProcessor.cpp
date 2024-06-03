/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Delay101AudioProcessor::Delay101AudioProcessor(): AudioProcessor(BusesProperties() .withInput("Input", juce::AudioChannelSet::stereo())
    .withOutput("Output", juce::AudioChannelSet::stereo()))
{
    addParameter(wetDry = new juce::AudioParameterFloat(
        "wetDry",
        "wet Dry",
        juce::NormalisableRange<float>(0.0f, 1.0f),
        0.0f));

    addParameter(feedbackGain = new juce::AudioParameterFloat(
        "feedback",
        "FeedBack Gain",
        juce::NormalisableRange<float>(0.0f, 1.0f),
        0.0f));

    addParameter(delayTime = new juce::AudioParameterFloat(
        "delayTime",
        "Delay Time (ms)",
        10.0f,
        1999.0f,
        0.0f));
}

Delay101AudioProcessor::~Delay101AudioProcessor()
{
}
//==============================================================================
void Delay101AudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate =  sampleRate;
    auto delayTimeMsecCopy = delayTime->get();
    auto wetDryCopy = wetDry->get();
    auto feedbackGainCopy = feedbackGain->get();
    
    delay.setParameters(currentSampleRate, delayTimeMsecCopy, 1 - wetDryCopy, wetDryCopy, feedbackGainCopy);
    delay.createDelayBuffer(currentSampleRate, 500.0);
   
}

void Delay101AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Delay101AudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void Delay101AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    auto mainInputOutput = getBusBuffer(buffer, true, 0);
    auto delayTimeMsecCopy = delayTime->get();
    auto wetDryCopy = wetDry->get();
    auto feedbackGainCopy = feedbackGain->get();

    delay.setParameters(currentSampleRate, delayTimeMsecCopy, 1 - wetDryCopy, wetDryCopy, feedbackGainCopy);

    for (auto sample = 0;sample < buffer.getNumSamples(); ++sample)
    {
        if (stereo == true)
        {
            auto yn = delay.processAudioSample(0, *mainInputOutput.getReadPointer(1, sample));
            *mainInputOutput.getWritePointer(0, sample) = yn[0];
            *mainInputOutput.getWritePointer(1, sample) = yn[1];
        }
        else
        {
            auto yn = delay.processAudioSample(*mainInputOutput.getReadPointer(0, sample), *mainInputOutput.getReadPointer(1, sample));
            *mainInputOutput.getWritePointer(0, sample) = yn[0];
            *mainInputOutput.getWritePointer(1, sample) = yn[1];
        }
    }
}

//==============================================================================
const juce::String Delay101AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Delay101AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Delay101AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Delay101AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Delay101AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Delay101AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Delay101AudioProcessor::getCurrentProgram()
{
    return 0;
}

void Delay101AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Delay101AudioProcessor::getProgramName (int index)
{
    return {};
}

void Delay101AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}


//==============================================================================
bool Delay101AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Delay101AudioProcessor::createEditor()
{
    //return new Delay101AudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void Delay101AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Delay101AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Delay101AudioProcessor();
}

/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
AnalogMultiTapDelayAudioProcessor::AnalogMultiTapDelayAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    parameters(*this, nullptr, juce::Identifier::Identifier("AnalogMultiTapDelayVTS"),
        {
        std::make_unique<juce::AudioParameterFloat>(
            "mix", "Mix", juce::NormalisableRange<float>(0.0f, 1.0f),1.0f),
        std::make_unique<juce::AudioParameterFloat>(
            "inputLevel", "inputLevel", juce::NormalisableRange<float>(0.0f, 1.0f),1.0f),
        std::make_unique<juce::AudioParameterFloat>(
            "delay", "Delay", juce::NormalisableRange<float>(0.01f, 1.0f),0.01f),
        std::make_unique<juce::AudioParameterFloat>(
            "width", "Width", juce::NormalisableRange<float>(0.0f, 1.0f),0.5f),
        std::make_unique<juce::AudioParameterFloat>(
            "feedback", "Feedback", juce::NormalisableRange<float>(0.0f, 1.0f),0.3f),
        std::make_unique<juce::AudioParameterFloat>(
            "timeRatio", "Time Ratio",1.00f,2.00f,1.618f),

        std::make_unique<juce::AudioParameterFloat>(
            "tapLevel_1", "Tap Level 1", juce::NormalisableRange<float>(0.0f, 1.0f),0.f),
        std::make_unique<juce::AudioParameterFloat>(
            "tapLevel_2", "Tap Level 2", juce::NormalisableRange<float>(0.0f, 1.0f),0.f),
        std::make_unique<juce::AudioParameterFloat>(
            "tapLevel_3", "Tap Level 3", juce::NormalisableRange<float>(0.0f, 1.0f),0.f),
        std::make_unique<juce::AudioParameterFloat>(
            "tapLevel_4", "Tap Level 4", juce::NormalisableRange<float>(0.0f, 1.0f),0.f),

        std::make_unique<juce::AudioParameterBool>(
            "tapSelect_1", "Tap 1 Select",true),
        std::make_unique<juce::AudioParameterBool>(
            "tapSelect_2", "Tap 2 Select",false),
        std::make_unique<juce::AudioParameterBool>(
            "tapSelect_3", "Tap 3 Select",false),
        std::make_unique<juce::AudioParameterBool>(
            "tapSelect_4", "Tap 4 Select",false),

        std::make_unique<juce::AudioParameterFloat>(
            "noiseLevel", "Noise", juce::NormalisableRange<float>(0.0f, 1.0f),1.0f),
        std::make_unique<juce::AudioParameterFloat>(
            "saturation", "Saturation", juce::NormalisableRange<float>(0.0f, 1.0f),0.0f),

        std::make_unique<juce::AudioParameterFloat>(
            "lowPass", "Low-Pass", 20.0f,15000.0f,15000.0f),
        std::make_unique<juce::AudioParameterFloat>(
            "highPass", "High-Pass", 20.0f,15000.0f,20.0f)
        })

#endif
{ }

AnalogMultiTapDelayAudioProcessor::~AnalogMultiTapDelayAudioProcessor()
{
}

//==============================================================================
void AnalogMultiTapDelayAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{

    currentSampleRate = sampleRate;

    mix =       parameters.getRawParameterValue("mix");
    delay =     parameters.getRawParameterValue("delay");
    feedback=   parameters.getRawParameterValue("feedback");
    timeRatio = parameters.getRawParameterValue("timeRatio");
    noiseLevel= parameters.getRawParameterValue("noiseLevel");
    lowPass =   parameters.getRawParameterValue("lowPass");
    highPass =  parameters.getRawParameterValue("highPass");

    vector<float> tapLevelsCopy;
    for (auto tap = 0; tap < numberOfTaps ;++tap)
    {
        tapLevels.push_back( parameters.getRawParameterValue("tapLevel_" + to_string(tap+1)));
        tapLevelsCopy.push_back(tapLevels[tap]->load());
    }

    auto mixCopy = mix->load();
    auto delayCopy = delay->load()*maxDelayTime;
    auto feedbackCopy = feedback->load();
    auto timeRatioCopy = timeRatio->load()*2.0f;
    auto noiseLevelCopy = noiseLevel->load();
    auto lowPassCopy = lowPass->load();
    auto highPassCopy = highPass->load();

    // initialising delayAlgorithm
    delayAlgorithm.setParameters(currentSampleRate,
        delayCopy, timeRatioCopy, numberOfTaps,
        1.0f - mixCopy, mixCopy, feedbackCopy);
    delayAlgorithm.instantiateTaps();
    delayAlgorithm.createDelayBuffer((float)currentSampleRate, maxDelayTime);
    delayAlgorithm.createNoise(0.0);
    delayAlgorithm.setFiltersParameters(lowPassCopy, highPassCopy);

    // starting a timer which wills top when the delay buffer is filled for the first time 
    startTimer(maxDelayTime*1.2);
}

void AnalogMultiTapDelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AnalogMultiTapDelayAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
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

void AnalogMultiTapDelayAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    auto mainInputOutput = getBusBuffer(buffer, true, 0);

    // getting paramters value from valueTreeState parameters
    auto mixCopy = mix->load();
    auto delayCopy = delay->load()*maxDelayTime;
    auto feedbackCopy = feedback->load();
    auto timeRatioCopy = timeRatio->load()*2;
    auto noiseLevelCopy = noiseLevel->load();
    auto lowPassCopy = lowPass->load();
    auto highPassCopy = highPass->load();
    vector<float> tapLevelsCopy;
    for (auto tap = 0; tap < numberOfTaps; ++tap)
    {
        tapLevels[tap] = parameters.getRawParameterValue("tapLevel_"+ to_string(tap + 1));
        tapLevelsCopy.push_back(tapLevels[tap]->load());
    }

    // Updating DelayAlgorithm components
    delayAlgorithm.setTapLevels(tapLevelsCopy);
    delayAlgorithm.setParameters(currentSampleRate,
        delayCopy, timeRatioCopy, numberOfTaps,
        1.0f - mixCopy, mixCopy, feedbackCopy);
    delayAlgorithm.setTapsDelayTime();
    delayAlgorithm.setNoiseLevel(noiseLevelCopy);
    delayAlgorithm.updateFiltersParameters(lowPassCopy, highPassCopy);

    // Processing audio 
    if (delayBufferFilled == false)
        // waiting for the buffer to fill up
        for (auto sample = 0;sample < buffer.getNumSamples(); ++sample)
        {
            for (auto i = 0; i < mainInputOutput.getNumChannels(); ++i)
            {
                // calling the delay to processto fill the buffer, but not putting any sound out from the delay buffer
                delayAlgorithm.processAudioSample(*mainInputOutput.getReadPointer(i, sample));
                *mainInputOutput.getWritePointer(i, sample) = *mainInputOutput.getReadPointer(i, sample);
            }
        }

    else
        // once buffer has been filled initially proceed to routine audio processing
        for (auto sample = 0;sample < buffer.getNumSamples(); ++sample)
        {
            //for (auto i = 0; i < mainInputOutput.getNumChannels(); ++i)
            // stereo to mono output 
            *mainInputOutput.getWritePointer(0, sample) = delayAlgorithm.processAudioSample(*mainInputOutput.getReadPointer(1, sample)); 
            *mainInputOutput.getWritePointer(1, sample) = delayAlgorithm.processAudioSample(*mainInputOutput.getReadPointer(1, sample));
        }
}

//==============================================================================
bool AnalogMultiTapDelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AnalogMultiTapDelayAudioProcessor::createEditor()
{
    return new AnalogMultiTapDelayAudioProcessorEditor(*this,parameters); 
    // UNCOMMENT IF USING A CUSTOM EDITOR from the PluginEditor class

    //return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
const juce::String AnalogMultiTapDelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AnalogMultiTapDelayAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AnalogMultiTapDelayAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AnalogMultiTapDelayAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AnalogMultiTapDelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AnalogMultiTapDelayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AnalogMultiTapDelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AnalogMultiTapDelayAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String AnalogMultiTapDelayAudioProcessor::getProgramName (int index)
{
    return {};
}

void AnalogMultiTapDelayAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void AnalogMultiTapDelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void AnalogMultiTapDelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AnalogMultiTapDelayAudioProcessor();
}

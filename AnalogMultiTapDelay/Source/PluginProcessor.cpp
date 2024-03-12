/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
AnalogMultiTapDelayAudioProcessor::AnalogMultiTapDelayAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)

    ),
    parameters(*this, nullptr, juce::Identifier::Identifier("AnalogMultiTapDelayVTS"),
        {
        std::make_unique<juce::AudioParameterFloat>(
            "mix", "Mix", juce::NormalisableRange<float>(0.0f, 100.0f),0.5f),
        std::make_unique<juce::AudioParameterFloat>(
            "inputLevel", "inputLevel", juce::NormalisableRange<float>(0.0f, 1.0f),1.0f),
        std::make_unique<juce::AudioParameterFloat>(
            "delay", "Delay", 10.f, 5000.0f,1000.0f),
        std::make_unique<juce::AudioParameterFloat>(
            "width", "Width", juce::NormalisableRange<float>(0.0f, 1.0f),0.0f),
        std::make_unique<juce::AudioParameterFloat>(
            "feedback", "Feedback", juce::NormalisableRange<float>(0.0f, 100.0f),0.3f),
        std::make_unique<juce::AudioParameterFloat>(
            "timeRatio", "Time Ratio",1.00f,2.00f,1.618f),

        std::make_unique<juce::AudioParameterFloat>(
            "tapLevel_1", "Tap Level 1", juce::NormalisableRange<float>(0.0f, 100.0f),1.f),
        std::make_unique<juce::AudioParameterFloat>(
            "tapLevel_2", "Tap Level 2", juce::NormalisableRange<float>(0.0f, 100.0f),0.f),
        std::make_unique<juce::AudioParameterFloat>(
            "tapLevel_3", "Tap Level 3", juce::NormalisableRange<float>(0.0f, 100.0f),0.f),
        std::make_unique<juce::AudioParameterFloat>(
            "tapLevel_4", "Tap Level 4", juce::NormalisableRange<float>(0.0f, 100.0f),0.f),

        std::make_unique<juce::AudioParameterBool>(
            "tapSelect_1", "Tap 1 Select",true),
        std::make_unique<juce::AudioParameterBool>(
            "tapSelect_2", "Tap 2 Select",false),
        std::make_unique<juce::AudioParameterBool>(
            "tapSelect_3", "Tap 3 Select",false),
        std::make_unique<juce::AudioParameterBool>(
            "tapSelect_4", "Tap 4 Select",false),

        std::make_unique<juce::AudioParameterFloat>(
            "noiseLevel", "Noise", juce::NormalisableRange<float>(0.0f, 1.0f),0.0f),
        std::make_unique<juce::AudioParameterFloat>(
            "saturation", "Saturation", juce::NormalisableRange<float>(0.0f, 1.0f),0.0f),

        std::make_unique<juce::AudioParameterFloat>(
            "lowPass", "Low-Pass", 20.0f,15000.0f,15000.0f),
        std::make_unique<juce::AudioParameterFloat>(
            "highPass", "High-Pass", 20.0f,15000.0f,20.0f)
        })
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
    width = parameters.getRawParameterValue("width");

    vector<float> tapLevelsCopy;
    for (auto tap = 0; tap < numberOfTaps ;++tap)
    {
        tapLevels.push_back( parameters.getRawParameterValue("tapLevel_" + to_string(tap+1)));
        tapLevelsCopy.push_back(tapLevels[tap]->load()/100);
    }

    auto mixCopy = mix->load() / 100;
    auto delayCopy = delay->load()*maxDelayTime;
    auto feedbackCopy = feedback->load() / 100;
    auto timeRatioCopy = timeRatio->load();
    auto noiseLevelCopy = noiseLevel->load();
    auto lowPassCopy = lowPass->load();
    auto highPassCopy = highPass->load();
    auto widthCopy = width->load();

    // initialising delayAlgorithm
    delayAlgorithm.setParameters(currentSampleRate,
        delayCopy, timeRatioCopy, numberOfTaps,
        1.0f - mixCopy, mixCopy, feedbackCopy,widthCopy);
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
    auto BufferIn_L = mainInputOutput.getReadPointer(0);
    auto BufferIn_R = mainInputOutput.getReadPointer(1);
    auto BufferOut_L = mainInputOutput.getWritePointer(0);
    auto BufferOut_R = mainInputOutput.getWritePointer(1);

    // getting paramters value from valueTreeState parameters
    auto mixCopy = mix->load() / 100;
    auto delayCopy = delay->load();
    auto feedbackCopy = feedback->load() / 100;
    auto timeRatioCopy = timeRatio->load();
    auto noiseLevelCopy = noiseLevel->load();
    auto lowPassCopy = lowPass->load();
    auto highPassCopy = highPass->load();
    auto widthCopy = width->load();

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
        1.0f - mixCopy, mixCopy, feedbackCopy, widthCopy);
    delayAlgorithm.setTapsDelayTime();
    delayAlgorithm.setNoiseLevel(noiseLevelCopy);
    delayAlgorithm.updateFiltersParameters(lowPassCopy, highPassCopy);

    float rightIn = 0.0;
    float leftIn = 0.0;

    // Processing audio 
    //if (delayBufferFilled == false)
    //    // waiting for the buffer to fill up
    //    for (auto sample = 0;sample < buffer.getNumSamples(); ++sample)
    //    {
    //            // calling the delay to processto fill the buffer, but not putting any sound out from the delay buffer
    //            delayAlgorithm.processAudioSample(BufferIn_L[sample], BufferIn_R[sample]);
    //            BufferOut_L[sample] = 0;
    //            BufferOut_R[sample] = 0;
    //    }
    //else
        // once buffer has been filled, proceed to routine audio processing
        for (auto sample = 0;sample < buffer.getNumSamples(); ++sample)
        {
            rightIn = BufferIn_L[sample];
            leftIn = BufferIn_R[sample];
            auto yn = delayAlgorithm.processAudioSample(leftIn,rightIn);
            BufferOut_L[sample] = yn[0];
            BufferOut_R[sample] = yn[1];
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

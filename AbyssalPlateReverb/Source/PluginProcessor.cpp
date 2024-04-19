/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AbyssalPlateReverbAudioProcessor::AbyssalPlateReverbAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
    parameters(*this, nullptr, juce::Identifier::Identifier("AbyssalReverbVTS"),
        {
            std::make_unique<juce::AudioParameterFloat>(
            "mix", "Mix", juce::NormalisableRange<float>(0.0f, 1.0f),0.0f),
            std::make_unique<juce::AudioParameterFloat>(
            "absorption", "Absorption", juce::NormalisableRange<float>(0.0f, 20000.0f),20000.0f),
            std::make_unique<juce::AudioParameterFloat>(
            "earlyReflexions", "Early Reflexions Amount", juce::NormalisableRange<float>(0.0f, 1.0f),0.0f),
            std::make_unique<juce::AudioParameterFloat>(
            "decay", "Decay", juce::NormalisableRange<float>(0.0f, 1.0f),0.0f),
            std::make_unique<juce::AudioParameterFloat>(
            "damping", "Damping", juce::NormalisableRange<float>(0.0f, 0.9999f),0.5f),
            std::make_unique<juce::AudioParameterFloat>(
            "modRate", "Modulation Rate", juce::NormalisableRange<float>(0.01f, 10.0f),1.0f),
            std::make_unique<juce::AudioParameterFloat>(
            "modDepth", "Modulation Depth", juce::NormalisableRange<float>(0.0f, 1.0),0.0f)
        }
    )
#endif
{
}

AbyssalPlateReverbAudioProcessor::~AbyssalPlateReverbAudioProcessor()
{
}

//==============================================================================
void AbyssalPlateReverbAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    absorption = parameters.getRawParameterValue("absorption");
    damping = parameters.getRawParameterValue("damping");
    decay = parameters.getRawParameterValue("decay");
    earlyReflexions = parameters.getRawParameterValue("earlyReflexions");
    mix = parameters.getRawParameterValue("mix");
    modDepth = parameters.getRawParameterValue("modDepth");
    modRate = parameters.getRawParameterValue("modRate");

    controlParameters.absorption = absorption->load();
    controlParameters.damping = damping->load();
    controlParameters.decay = decay->load();
    controlParameters.earlyReflexions = earlyReflexions->load();
    controlParameters.mix = mix->load();
    controlParameters.modDepth = modDepth->load();
    controlParameters.modRate = modRate->load();

    reverbAlgorithm.reset(sampleRate);
    reverbAlgorithm.setParameters(controlParameters);
}

void AbyssalPlateReverbAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AbyssalPlateReverbAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
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

void AbyssalPlateReverbAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    auto mainInputOutput = getBusBuffer(buffer, true, 0);
    auto BufferIn_L = mainInputOutput.getReadPointer(0);
    auto BufferIn_R = mainInputOutput.getReadPointer(1);
    auto BufferOut_L = mainInputOutput.getWritePointer(0);
    auto BufferOut_R = mainInputOutput.getWritePointer(1);

    // get plugin parameters values
    controlParameters.absorption = absorption->load();
    controlParameters.damping = damping->load();
    controlParameters.decay = decay->load();
    controlParameters.earlyReflexions = earlyReflexions->load();
    controlParameters.mix = mix->load();
    controlParameters.modDepth = modDepth->load();
    controlParameters.modRate = modRate->load();

    // update Reverb Algorithm parameters
    reverbAlgorithm.updateParameters(controlParameters);

    vector<float> input = { 0.0, 0.0};

    for (auto sample = 0;sample < buffer.getNumSamples(); ++sample)
    {
        input[0] = BufferIn_L[sample];
        input[1] = BufferIn_R[sample];
        auto yn = reverbAlgorithm.processAudioSample(input);
        BufferOut_L[sample] = yn[0];
        BufferOut_R[sample] = yn[1];
    }
}

//==============================================================================
bool AbyssalPlateReverbAudioProcessor::hasEditor() const
{
    return false; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AbyssalPlateReverbAudioProcessor::createEditor()
{
    return new AbyssalPlateReverbAudioProcessorEditor(*this);
}

//==============================================================================
const juce::String AbyssalPlateReverbAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AbyssalPlateReverbAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AbyssalPlateReverbAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AbyssalPlateReverbAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AbyssalPlateReverbAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AbyssalPlateReverbAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AbyssalPlateReverbAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AbyssalPlateReverbAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String AbyssalPlateReverbAudioProcessor::getProgramName (int index)
{
    return {};
}

void AbyssalPlateReverbAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void AbyssalPlateReverbAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void AbyssalPlateReverbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AbyssalPlateReverbAudioProcessor();
}

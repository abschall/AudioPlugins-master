/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DattorroReverbAudioProcessor::DattorroReverbAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
    parameters(*this, nullptr, juce::Identifier::Identifier("SchroederReverbVTS"),
        { 
            std::make_unique<juce::AudioParameterFloat>(
            "mix", "Mix", juce::NormalisableRange<float>(0.0f, 1.0f),0.0f),
            std::make_unique<juce::AudioParameterFloat>(
            "predelay", "Predelay", juce::NormalisableRange<float>(0.0f, 500.0f),0.0f),
            std::make_unique<juce::AudioParameterFloat>(
            "inputDiffusion1", "Input Diffusion 1", juce::NormalisableRange<float>(0.0f, 0.9999999f),0.75f),
            std::make_unique<juce::AudioParameterFloat>(
            "inputDiffusion2", "Input Diffusion 2", juce::NormalisableRange<float>(0.0f, 0.9999999f),0.625f),
            std::make_unique<juce::AudioParameterFloat>(
            "decayDiffusion1", "Decay Diffusion 1", juce::NormalisableRange<float>(0.0f, 0.9999999f),0.7f),
            std::make_unique<juce::AudioParameterFloat>(
            "decayDiffusion2", "Decay Diffusion 2", juce::NormalisableRange<float>(0.25f, 0.5f),0.5f),
            std::make_unique<juce::AudioParameterFloat>(
            "decay", "Decay", juce::NormalisableRange<float>(0.0f,  0.9999999f),0.5f),
            std::make_unique<juce::AudioParameterFloat>(
            "damping", "Damping", juce::NormalisableRange<float>(0.0f, 20000.0f),10.0f),
            std::make_unique<juce::AudioParameterFloat>(
            "bandwidth", "Bandwidth", juce::NormalisableRange<float>(0.0f, 20000.0f),20000.0f)
        }
    )
#endif
{
}

DattorroReverbAudioProcessor::~DattorroReverbAudioProcessor()
{
}

//==============================================================================
void DattorroReverbAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    mix = parameters.getRawParameterValue("mix");
    predelay = parameters.getRawParameterValue("predelay");
    inputDiffusion1 = parameters.getRawParameterValue("inputDiffusion1");
    inputDiffusion2 = parameters.getRawParameterValue("inputDiffusion2");
    decayDiffusion1 = parameters.getRawParameterValue("decayDiffusion1");
    decayDiffusion2 = parameters.getRawParameterValue("decayDiffusion2");
    decay = parameters.getRawParameterValue("decay");
    damping = parameters.getRawParameterValue("damping");
    bandwidth = parameters.getRawParameterValue("bandwidth");


    controlParameters.mix = mix->load();
    controlParameters.predelay = predelay->load();
    controlParameters.inputDiffusion1 = inputDiffusion1->load();
    controlParameters.inputDiffusion2 = inputDiffusion2->load();
    controlParameters.decayDiffusion1 = decayDiffusion1->load();
    controlParameters.decayDiffusion2 = decayDiffusion2->load();
    controlParameters.damping = damping->load();
    controlParameters.bandwidth = bandwidth->load();
    controlParameters.decay = decay->load();

    
    reverbAlgorithm.reset(sampleRate);
    reverbAlgorithm.setParameters(controlParameters);
}

void DattorroReverbAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DattorroReverbAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
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

void DattorroReverbAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    auto mainInputOutput = getBusBuffer(buffer, true, 0);
    auto BufferIn_L = mainInputOutput.getReadPointer(0);
    auto BufferIn_R = mainInputOutput.getReadPointer(1);
    auto BufferOut_L = mainInputOutput.getWritePointer(0);
    auto BufferOut_R = mainInputOutput.getWritePointer(1);

    // get plugin parameters values
    controlParameters.mix = mix->load();
    controlParameters.predelay = predelay->load();
    controlParameters.inputDiffusion1 = inputDiffusion1->load();
    controlParameters.inputDiffusion2 = inputDiffusion2->load();
    controlParameters.decayDiffusion1 = decayDiffusion1->load();
    controlParameters.decayDiffusion2 = decayDiffusion2->load();
    controlParameters.damping = damping->load();
    controlParameters.bandwidth = bandwidth->load();
    controlParameters.decay = decay->load();

    // update Reverb Algorithm parameters
    reverbAlgorithm.updateParameters(controlParameters);


    vector<float> input = { 0.0,0.0 };

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
bool DattorroReverbAudioProcessor::hasEditor() const
{
    return false; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DattorroReverbAudioProcessor::createEditor()
{
    return new DattorroReverbAudioProcessorEditor (*this);
}

//==============================================================================
const juce::String DattorroReverbAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DattorroReverbAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool DattorroReverbAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool DattorroReverbAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double DattorroReverbAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DattorroReverbAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int DattorroReverbAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DattorroReverbAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String DattorroReverbAudioProcessor::getProgramName(int index)
{
    return {};
}

void DattorroReverbAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void DattorroReverbAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void DattorroReverbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DattorroReverbAudioProcessor();
}

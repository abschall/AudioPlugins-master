/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CouteauSuisseAudioProcessor::CouteauSuisseAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
   parameters(*this, nullptr, juce::Identifier::Identifier("CouteauSuisseVTS"),createParameterLayout())
#endif
{
}

CouteauSuisseAudioProcessor::~CouteauSuisseAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout CouteauSuisseAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterFloat>("level", "Level", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterInt>("select", "Select", 0, 2, 0));

    return layout;

}


//==============================================================================
const juce::String CouteauSuisseAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CouteauSuisseAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CouteauSuisseAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CouteauSuisseAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CouteauSuisseAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CouteauSuisseAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CouteauSuisseAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CouteauSuisseAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String CouteauSuisseAudioProcessor::getProgramName (int index)
{
    return {};
}

void CouteauSuisseAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void CouteauSuisseAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void CouteauSuisseAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CouteauSuisseAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void CouteauSuisseAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    auto mainInputOutput = getBusBuffer(buffer, true, 0);
    auto BufferIn_L = mainInputOutput.getReadPointer(0);
    auto BufferIn_R = mainInputOutput.getReadPointer(1);
    auto BufferOut_L = mainInputOutput.getWritePointer(0);
    auto BufferOut_R = mainInputOutput.getWritePointer(1);
    
    float noiseSelect = parameters.getRawParameterValue("select")->load();
    float levelSliderValue = parameters.getRawParameterValue("level")->load();

    float noiseSample;
    for (auto sample = 0;sample < buffer.getNumSamples(); ++sample)
    {
;
        if (noiseSelect == 0)
        {
            noiseSample = random.nextFloat();
        }
        if (noiseSelect == 1)
        {
            noiseSample = nP.generate();
        }
        if (noiseSelect == 2)
        {
            noiseSample = nB.generate();
        }
        BufferOut_L[sample] = noiseSample * levelSliderValue;
        BufferOut_R[sample] = noiseSample * levelSliderValue;
    }
}

//==============================================================================
bool CouteauSuisseAudioProcessor::hasEditor() const
{
    return false; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* CouteauSuisseAudioProcessor::createEditor()
{
    return new CouteauSuisseAudioProcessorEditor (*this);
}

//==============================================================================
void CouteauSuisseAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void CouteauSuisseAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CouteauSuisseAudioProcessor();
}

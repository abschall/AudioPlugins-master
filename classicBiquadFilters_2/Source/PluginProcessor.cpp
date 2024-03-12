/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"



//==============================================================================
ClassicBiquadFilters_2AudioProcessor::ClassicBiquadFilters_2AudioProcessor():AudioProcessor(BusesProperties() 
    .withInput("Input", juce::AudioChannelSet::stereo())
    .withOutput("Output", juce::AudioChannelSet::stereo())),
    parameters(*this,
        nullptr,
        juce::Identifier::Identifier("ClassicBiquadFiltersVTS"),
        {
            
        std::make_unique<juce::AudioParameterFloat>(
            "dryWet",
            "Dry Wet",
            juce::NormalisableRange<float>(0.0f, 1.0f),
            1.0f),
        std::make_unique<juce::AudioParameterFloat>(
            "qfactor",
            "Q Factor",
            juce::NormalisableRange<float>(0.001f, 20.0f),
            0.1f),
        std::make_unique<juce::AudioParameterFloat>(
            "fc",
            "Cut-off Freq",
            juce::NormalisableRange<float>(10.0f, 20000.0f,1.0f,.3f), // applying a skew factor to cut-offFreq
            0.1f),
        std::make_unique<juce::AudioParameterFloat>(
            "filtertype",
            "Filter Type",
            juce::NormalisableRange<float>(0.0f, 0.75f),
            0.0f)
        }
    )
{   
    fcParameter = parameters.getRawParameterValue("fc");
    QParameter = parameters.getRawParameterValue("qfactor");
    dryWetParameter = parameters.getRawParameterValue("dryWet");
    filterTypeParameter = parameters.getRawParameterValue("filtertype");

    //filterTypeParameter = parameters.getParameter("filterChoice");
    //filterTypeNum = (int)filterTypeParameter->getValue();
}


ClassicBiquadFilters_2AudioProcessor::~ClassicBiquadFilters_2AudioProcessor()
{
}


//==============================================================================
void ClassicBiquadFilters_2AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    auto fcCopy = fcParameter->load();
    auto QCopy = QParameter->load();
    auto KCopy = dryWetParameter->load();
    auto filterTypeCopy = filterTypeParameter->load();

    currentSampleRate = sampleRate;
    setFilterType((int) 4* filterTypeCopy);
    filter.setFilterGain(KCopy);
    filter.setCoefficients(fcCopy, QCopy, currentSampleRate);

}

void ClassicBiquadFilters_2AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void ClassicBiquadFilters_2AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    auto mainInputOutput = getBusBuffer(buffer, true, 0);

    auto fcCopy = fcParameter->load();
    auto QCopy = QParameter->load();
    auto KCopy = dryWetParameter->load();
    auto filterTypeCopy = filterTypeParameter->load();

    setFilterType((int) 4 * filterTypeCopy);
    filter.setFilterGain(KCopy);
    filter.setCoefficients(fcCopy, QCopy, currentSampleRate);

    for (auto sample = 0;sample < buffer.getNumSamples(); ++sample)
    {
        for (auto i = 0; i < mainInputOutput.getNumChannels(); ++i)
        {
            *mainInputOutput.getWritePointer(i, sample) = filter.processAudioSample(*mainInputOutput.getReadPointer(i, sample));
        }
    }
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ClassicBiquadFilters_2AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

//==============================================================================

const juce::String ClassicBiquadFilters_2AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ClassicBiquadFilters_2AudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool ClassicBiquadFilters_2AudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool ClassicBiquadFilters_2AudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double ClassicBiquadFilters_2AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ClassicBiquadFilters_2AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int ClassicBiquadFilters_2AudioProcessor::getCurrentProgram()
{
    return 0;
}

void ClassicBiquadFilters_2AudioProcessor::setCurrentProgram(int index)
{
}

const juce::String ClassicBiquadFilters_2AudioProcessor::getProgramName(int index)
{
    return {};
}

void ClassicBiquadFilters_2AudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
bool ClassicBiquadFilters_2AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ClassicBiquadFilters_2AudioProcessor::createEditor()
{
    return new ClassicBiquadFilters_2AudioProcessorEditor(*this, parameters);
}
//==============================================================================
void ClassicBiquadFilters_2AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ClassicBiquadFilters_2AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}
//==============================================================================
//Helper function
void ClassicBiquadFilters_2AudioProcessor::setFilterType(int filterNum)
{
    switch (filterNum) {
        case 0:
            filter.setFilterType("LPF1");
            break;
        case 1:
            filter.setFilterType("LPF2");
            break;
        case 2 :
            filter.setFilterType("HPF1");
            break;
        case 3:
            filter.setFilterType("HPF2");
            break;
        default :
            filter.setFilterType("LPF1");
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ClassicBiquadFilters_2AudioProcessor();
}

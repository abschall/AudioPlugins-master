#include "MainComponent.h"

// [Main Component CLASS]
//==============================================================================
MainComponent::MainComponent() : 
    synthAudioSource(keyboardState),
    keyboardComponent(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard),
    startTime(juce::Time::getMillisecondCounterHiRes() * 0.001)
{
    // [Main Window SETUP]
    setSize (800, 600);
    setOpaque(true);

    // [Initializing the external Midi Source]
    addAndMakeVisible(midiInputList); // Here the midiInputList is empty, it is an empty dropdown menu
    midiInputList.setTextWhenNoChoicesAvailable("No Available Midi Device");

    auto midiInputs = juce::MidiInput::getAvailableDevices(); // get the Available devices from MidiInput class
    juce::StringArray midiInputNames; // create anarray of the names

    for (auto input : midiInputs) // fill the array with the names 
        midiInputNames.add(input.name);

    midiInputList.addItemList(midiInputNames, 1); // add all the names to the dropdown menu

    midiInputList.onChange = [this] { midiInputList.getSelectedItemIndex();}; // from the users clicked item set it as the new midiSource

    for (auto inputs : midiInputs) // lookfor the first one and use it as default midi device
    {
        if (deviceManager.isMidiInputDeviceEnabled(inputs.identifier))
        {
            setMidiInput(midiInputs.indexOf(inputs)); // set the midi Input
            break;
        }
    }

    if (midiInputList.getSelectedId() == 0) // if no device was found then set the first one 
        setMidiInput(0);

    addAndMakeVisible(keyboardComponent);
    keyboardState.addListener(this);

    // [Initializing the external Midi Source]
    // [Initializing the Synthesizer]
    setAudioChannels(0, 2);
    startTimer (400);

    // [Initializing the Synthesizer]
}
/*
MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}*/

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    synthAudioSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    synthAudioSource.getNextAudioBlock(bufferToFill);
}

void MainComponent::releaseResources()
{
    synthAudioSource.releaseResources();
    keyboardState.removeListener(this);
    deviceManager.removeMidiInputDeviceCallback(juce::MidiInput::getAvailableDevices()[midiInputList.getSelectedItemIndex()].identifier, this);
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // You can add your drawing code here!
}

void MainComponent::resized()
{
    auto area = getLocalBounds();

    // [MIDI components position]
    midiInputList.setBounds(area.removeFromTop(36).removeFromRight(getWidth() - 150).reduced(8));
    keyboardComponent.setBounds(area.removeFromTop(80).reduced(8));
    midiMessagesBox.setBounds(area.reduced(8));
}

// [SynthAudioSource CLASS]

void SynthAudioSource::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    synth.setCurrentPlaybackSampleRate(sampleRate); // [3]
    midiCollector.reset(sampleRate);
}
void SynthAudioSource::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();

    juce::MidiBuffer incomingMidi;
    midiCollector.removeNextBlockOfMessages(incomingMidi, bufferToFill.numSamples);
    keyboardState.processNextMidiBuffer(incomingMidi, bufferToFill.startSample, bufferToFill.numSamples, true);       // [4]

    synth.renderNextBlock(*bufferToFill.buffer, incomingMidi, bufferToFill.startSample, bufferToFill.numSamples); // [5]
}
juce::MidiMessageCollector* SynthAudioSource::getMidiCollector()
{
    return &midiCollector;
}

void SynthAudioSource::releaseResources(){ }
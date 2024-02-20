#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent : public juce::AudioAppComponent,
                      private juce::MidiInputCallback, 
                      private juce::MidiKeyboardStateListener
{
public:

    MainComponent() :
        keyboardComponent(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard),
        startTime(juce::Time::getMillisecondCounterHiRes() * 0.001)
        //We must pass a MidiKeyboardState object to initialise the MidiKeyboardComponent object.And, since these are statically
        //allocated objects the MidiKeyboardState must be listed first in our member variables.
    {
        setOpaque(true);

        //==============================================================================
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

        // ================================================================================
        // [keyboardComponent and MidiKeyboardState]
        addAndMakeVisible(keyboardComponent);
        keyboardState.addListener(this);

        addAndMakeVisible(midiMessagesBox);
        setSize(600, 400);
    }
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override
    {
        keyboardState.removeListener(this);
        deviceManager.removeMidiInputDeviceCallback(juce::MidiInput::getAvailableDevices()[midiInputList.getSelectedItemIndex()].identifier, this);
    }

    //==============================================================================
    void paint(juce::Graphics& g) override;
    void resized() override
    {
        auto area = getLocalBounds();

        midiInputList.setBounds(area.removeFromTop(36).removeFromRight(getWidth() - 150).reduced(8));
        keyboardComponent.setBounds(area.removeFromTop(80).reduced(8));
        midiMessagesBox.setBounds(area.reduced(8));
    }

private:
    class IncomingMessageCallback : public juce::CallbackMessage
    {
    public:
        IncomingMessageCallback(MainComponent* o, const juce::MidiMessage& m, const juce::String& s)
            : owner(o), message(m), source(s)
        {}

        void messageCallback() override
        {
            if (owner != nullptr)
                owner->addMessageToList(message);
        }

        Component::SafePointer<MainComponent> owner;
        juce::MidiMessage message;
        juce::String source;
    };
    void setMidiInput(int index)
    {
        auto list = juce::MidiInput::getAvailableDevices();

        deviceManager.removeMidiInputDeviceCallback(list[lastInputIndex].identifier, this);

        auto newInput = list[index];

        if (!deviceManager.isMidiInputDeviceEnabled(newInput.identifier))
            deviceManager.setMidiInputDeviceEnabled(newInput.identifier, true);

        deviceManager.addMidiInputDeviceCallback(newInput.identifier, this);
        midiInputList.setSelectedId(index + 1, juce::dontSendNotification);

        lastInputIndex = index;
    }
    void handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message) override
    {
        const juce::ScopedValueSetter<bool> scopedInputFlag(isAddingFromMidiInput, true);
        keyboardState.processNextMidiEvent(message);
        postMessageToList(message, source->getName());
    }
    // This is used to dispach an incoming message to the message thread

    void postMessageToList(const juce::MidiMessage& m, const juce::String& source)
    {
        (new IncomingMessageCallback(this, m, source))->post();
    }
    void handleNoteOff(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float /*velocity*/) override
    {
        if (!isAddingFromMidiInput)
        {
            auto m = juce::MidiMessage::noteOff(midiChannel, midiNoteNumber);
            m.addToTimeStamp(juce::Time::getMillisecondCounterHiRes() * 0.001);
            postMessageToList(m, "On-screen Keyboard");
        }
    }
    void handleNoteOn(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override
    {
        if (!isAddingFromMidiInput)
        {
            auto m = juce::MidiMessage::noteOn(midiChannel, midiNoteNumber, velocity);
            m.addToTimeStamp(juce::Time::getMillisecondCounterHiRes() * 0.001);
            postMessageToList(m, "On-screen Keyboard");
        }
    }
    void logMessage(const juce::String& m)
    {
        // see logMessagetutorial and class for additionnal information on these objects
        // and methods

        midiMessagesBox.moveCaretToEnd();
        midiMessagesBox.insertTextAtCaret(m + juce::newLine);
    }
    void addMessageToList(juce::MidiMessage& message)
    {
        auto time = message.getTimeStamp();

        auto hours = ((int)(time / 3600)) % 24;
        auto minutes = ((int)(time / 60)) % 60;
        auto seconds = ((int)time) % 60;
        auto millis = ((int)(time * 1000.0)) % 1000;

        // creating a timecode from the message's timestamp
        auto timecode = juce::String::formatted("%02d:%02d:%02d.%03d",
            hours,
            minutes,
            seconds,
            millis);

        logMessage(timecode + "  -  " + getMidiMessageDescription(message)); // logging the message to the TextEditor
    }

    static juce::String getMidiMessageDescription(const juce::MidiMessage& m)
    {
        // taken from Midi Data tutorial from juce
        if (m.isNoteOn())           return "Note on " + juce::MidiMessage::getMidiNoteName(m.getNoteNumber(), true, true, 3);
        if (m.isNoteOff())          return "Note off " + juce::MidiMessage::getMidiNoteName(m.getNoteNumber(), true, true, 3);
        if (m.isProgramChange())    return "Program change " + juce::String(m.getProgramChangeNumber());
        if (m.isPitchWheel())       return "Pitch wheel " + juce::String(m.getPitchWheelValue());
        if (m.isAftertouch())       return "After touch " + juce::MidiMessage::getMidiNoteName(m.getNoteNumber(), true, true, 3) + ": " + juce::String(m.getAfterTouchValue());
        if (m.isChannelPressure())  return "Channel pressure " + juce::String(m.getChannelPressureValue());
        if (m.isAllNotesOff())      return "All notes off";
        if (m.isAllSoundOff())      return "All sound off";
        if (m.isMetaEvent())        return "Meta event";

        if (m.isController())
        {
            juce::String name(juce::MidiMessage::getControllerName(m.getControllerNumber()));

            if (name.isEmpty())
                name = "[" + juce::String(m.getControllerNumber()) + "]";

            return "Controller " + name + ": " + juce::String(m.getControllerValue());
        }

        return juce::String::toHexString(m.getRawData(), m.getRawDataSize());
    }

    // variable members
    juce::ComboBox midiInputList;
    int lastInputIndex = 0;
    juce::AudioDeviceManager deviceManager;
    juce::MidiKeyboardState keyboardState; // keeps track of which midikeyboard keys are pressed down
    juce::MidiKeyboardComponent keyboardComponent;
    
    
    bool isAddingFromMidiInput = false; //This flag is used to indicate that MIDI data is arriving 
    //from an external source, rather than mouse-clicks on the on-screen keyboard.

    juce::TextEditor midiMessagesBox;

    double startTime;
};
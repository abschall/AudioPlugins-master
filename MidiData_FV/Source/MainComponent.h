#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::AudioAppComponent, public juce::Timer
{
public:
    //==============================================================================
    MainComponent() : startTime(juce::Time::getMillisecondCounterHiRes() * 0.001) 
    {
        setSize(600, 400);
        
        addAndMakeVisible(snare); //[1]
        snare.setButtonText("Snare"); //[2]
        snare.onClick = [this] { setNoteNumber(36); }; //[3]

        //! [1] same as using setVisible(true) and addChildComponent
        //! [2] setting the text of the button
        //! [3] adding a lambda function to the onClick Callback method, the lambda function calls the setNoteNumberFunction
        
        addAndMakeVisible(bassdrum);
        bassdrum.setButtonText("Bass Drum");
        bassdrum.onClick = [this] { setNoteNumber(38); };

        addAndMakeVisible(midiMessagesBox);
        midiMessagesBox.setMultiLine(true);
        //see textEditor class for additional information on the following methods
        /*midiMessagesBox.setReturnKeyStartsNewLine(true);
        midiMessagesBox.setReadOnly(true);
        midiMessagesBox.setScrollbarsShown(true);
        midiMessagesBox.setCaretVisible(false);
        midiMessagesBox.setPopupMenuEnabled(true);
        */
        startTimer(1);

    }
    
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override
    {
        int widthButtons = 60;
        int heightButtons = 30;
        int sepHeight = 10;
        int halfWidth = getWidth() / 2;
        snare.      setBounds(10, 10                                , getWidth() - widthButtons-halfWidth, heightButtons);
        bassdrum.   setBounds(10, 10 + 1*(sepHeight + heightButtons), getWidth() - widthButtons-halfWidth, heightButtons);
        midiMessagesBox.setBounds(getLocalBounds().withWidth(halfWidth).withX(halfWidth).reduced(10));
    }

private:
    void timerCallback() override
    {
        auto currentTime = juce::Time::getMillisecondCounterHiRes() * 0.001 - startTime;
        auto currentSampleNumber = currentTime * sampleRate;

        for (auto metadata : midiBuffer) // iterating trought the midiBuffer
        {
            if (metadata.samplePosition > currentSampleNumber) 
            // If the timestamp for the MIDI message most recently retrieved from the MidiBuffer object
            //  is in the future, then we have finished processing and we exit the while() loop.
                break;
            auto message = metadata.getMessage(); // get the midiMessage
            message.setTimeStamp(metadata.samplePosition / sampleRate); // mark it with the timestamp
            addMessageToList(message); // add the message to the list of MidiMessages
        }
        midiBuffer.clear(previousSampleNumber, currentSampleNumber - previousSampleNumber); //clear the buffer from the messages we have just processed
        previousSampleNumber = currentSampleNumber; // update the sampleNumber 

    }

    void logMessage(const juce::String& m)
    {
        // see logMessagetutorial and class for additionnal information on these objects
        // and methods

        midiMessagesBox.moveCaretToEnd();
        midiMessagesBox.insertTextAtCaret(m + juce::newLine);
    }
    void setNoteNumber(int NoteNumber)
    {
        auto message = juce::MidiMessage::noteOn(midiChannel, NoteNumber, (juce::uint8)100); //[1]
        message.setTimeStamp(juce::Time::getMillisecondCounterHiRes()*0.001 - startTime); //[2]
        addMessageToBuffer(message);
        //addMessageToList(message); 
        //! [1] create a key perssed down midi message : param : midiCHannel, the set NoteNumber, the velocity
        //! [2] sets a timeStamp to the midiMessage
        
        //creating noteOff messae
        auto messageOff = juce::MidiMessage::noteOff(message.getChannel(), message.getNoteNumber());
        messageOff.setTimeStamp(message.getTimeStamp() + 0.1);
        addMessageToBuffer(messageOff);

    }
    void addMessageToBuffer(juce::MidiMessage& message)
    {
        // a midiBuffer holds a sequence of timestamped midi events

        auto timestamp = message.getTimeStamp();
        auto sampleNumber = (int)(timestamp * sampleRate);
        midiBuffer.addEvent(message, sampleNumber);
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

    //==============================================================================
    // Your private member variables go here...

    juce::TextButton snare;
    juce::TextButton bassdrum;
    juce::TextEditor midiMessagesBox;

    juce::MidiBuffer midiBuffer;
    int previousSampleNumber = 0;
    double sampleRate = 44100.0;

    int midiChannel = 1;
    double startTime;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

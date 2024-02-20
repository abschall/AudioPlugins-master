#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
struct SineWaveSound : public juce::SynthesiserSound
{
    // the SineWave Soundclass is very simple here, it only needs to report whether
    //  this sound should play on particular MIDI channels and specific notes or note ranges on that channel
    // it is here that we would add the data for a particuliar sound, such as a wavetable
    SineWaveSound() {}

    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};

struct SineWaveVoice : public juce::SynthesiserVoice
{
    SineWaveVoice() {}

    bool canPlaySound(juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<SineWaveSound*> (sound) != nullptr;
    }

    void startNote(int midiNoteNumber, float velocity,
        juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override
    {
        currentAngle = 0.0;
        level = velocity * 0.15;
        tailOff = 0.0;

        auto cyclesPerSecond = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
        auto cyclesPerSample = cyclesPerSecond / getSampleRate();

        angleDelta = cyclesPerSample * 2.0 * juce::MathConstants<double>::pi;
    }

    void stopNote(float /*velocity*/, bool allowTailOff) override
    {
        if (allowTailOff)
        {
            if (tailOff == 0.0)
                tailOff = 1.0;
        }
        else
        {
            clearCurrentNote();
            angleDelta = 0.0;
        }
    }

    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}

    void renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override
    {
        // generates the actual audio of the synthVoice !
        if (angleDelta != 0.0)
        {
            if (tailOff > 0.0) // [7]
            {
                while (--numSamples >= 0)
                {
                    auto currentSample = (float)(std::sin(currentAngle) * level * tailOff);

                    for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                        outputBuffer.addSample(i, startSample, currentSample);

                    currentAngle += angleDelta;
                    ++startSample;

                    tailOff *= 0.99; // [8]

                    if (tailOff <= 0.005)
                    {
                        clearCurrentNote(); // [9]

                        angleDelta = 0.0;
                        break;
                    }
                }
            }
            else
            {
                while (--numSamples >= 0) // [6]
                {
                    auto currentSample = (float)(std::sin(currentAngle) * level);

                    for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                        outputBuffer.addSample(i, startSample, currentSample);

                    currentAngle += angleDelta;
                    ++startSample;
                }
            }
        }
    }

private:
    double currentAngle = 0.0, angleDelta = 0.0, level = 0.0, tailOff = 0.0;
};

class SynthAudioSource : public juce::AudioSource
{
public:
    SynthAudioSource(juce::MidiKeyboardState& keyState) : keyboardState(keyState)
    {
        for (auto i = 0; i < 4; ++i)  // [1]    
        {
            synth.addVoice(new SineWaveVoice());
        }
        synth.addSound(new SineWaveSound());       // [2]
    }

    void setUsingSineWaveSound()
    {
        synth.clearSounds();
    }
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    juce::MidiMessageCollector* SynthAudioSource::getMidiCollector();
private:
    juce::MidiKeyboardState& keyboardState;
    juce::Synthesiser synth;
    juce::MidiMessageCollector midiCollector;
    
};

class MainComponent : public juce::AudioAppComponent,
    private juce::MidiInputCallback,
    private juce::MidiKeyboardStateListener,
    private juce::Timer
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent()
    {
        shutdownAudio();
    }

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;


private:
    void timerCallback() override
    {
        keyboardComponent.grabKeyboardFocus();
        stopTimer();
    }
    // [MIDI events handle methods]
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

        deviceManager.removeMidiInputDeviceCallback(list[lastInputIndex].identifier, synthAudioSource.getMidiCollector());

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
    // [MIDI events handle methods]

    //==============================================================================
    // Your private member variables go here...
    
    // [MIDI variables]
    juce::ComboBox midiInputList;
    int lastInputIndex = 0;
    juce::AudioDeviceManager deviceManager;
    juce::MidiKeyboardState keyboardState; 
    juce::MidiKeyboardComponent keyboardComponent;
    bool isAddingFromMidiInput = false; 
    juce::TextEditor midiMessagesBox;

    // [Time and Timer variables]
    double startTime;
    //[Synthesizer variables]
    SynthAudioSource synthAudioSource;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

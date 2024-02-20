#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
//! [SineWaveVoice class]
struct SineWaveSound : public juce::SynthesiserSound
{
    // appliesToNote , appliesToChannel  are pure virtual functions : override and define  
    SineWaveSound(){}
    bool appliesToNote      (int) override { return true;  }
    bool appliesToChannel   (int) override { return true;  }

};

//! [SineWaveVoice class]
struct SineWaveVoice : public juce::SynthesiserVoice
{
    SineWaveVoice(juce::Slider &attack, juce::Slider &release): attackSlider(attack), 
                                                                releaseSlider(release)
    { }
    // controllerMoved, pitchWheelMoved, canPlaySound, startNote,  stopNote, renderNextBlock are pure virtual functions 
    // and have to be overriden and have to be defined

    void controllerMoved(int,int)   override {}
    void pitchWheelMoved(int)       override {}

    bool canPlaySound(juce::SynthesiserSound*) override { return true; }
    // just return true if the synthesiser has only one voice and one sound
    // otherwise dynamic casting should be performed

    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int /*currentPitchWheelPosition*/) override
    {

        currentAngle = 0.0;
        tailOff = 0.0; 
        attackLvl = 0.05;
        level = velocity * 0.15;
        auto cyclesPerSecond = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
        auto cyclesPerSample = cyclesPerSecond / getSampleRate();
        angleDelta = cyclesPerSample * 2.0 * juce::MathConstants<double>::pi;

        // linear attack and release times 
        auto attackDuration = attackSlider.getValue();
        auto releaseDuration = releaseSlider.getValue();
        attackDelta = 1/((attackDuration/1000 )* getSampleRate()); // incrementing a linear delta value, at each sample 
        releaseDelta = 1 / ((releaseDuration / 1000) * getSampleRate());
    }

    void stopNote(float velocity, bool allowTailOff) override 
    {
        // !NO TAIL FOR THE TIME BEING 
        if (allowTailOff)
        {
            if (tailOff == 0.0)
            {
                tailOff = attackLvl; // if the key is released before the end of the attack time, then the tailOff value takes the last attackLvl value
            }
        }
        else
        {
            clearCurrentNote();
            angleDelta = 0.0;
        }
    }

    void renderNextBlock(juce::AudioBuffer< float >& outputBuffer, int startSample, int numSamples) override
    {
            // write the audio code here
        if (tailOff > 0.0)
        {
            while (--numSamples >= 0)
            {
                auto currentSample = (float)(std::sin(currentAngle) * level * tailOff);

                for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                {
                    outputBuffer.addSample(i, startSample, currentSample);
                }
                currentAngle += angleDelta;
                ++startSample;
                tailOff -= releaseDelta;

                if (tailOff <= 0.005)
                {
                    clearCurrentNote();
                    angleDelta = 0.0;
                    break;
                }
            }
        }
        else
        {
            if (attackLvl >= 1.0)
            {
                // this is the normal state : key pressed down (no release), attack Lvl reached final level in time, note is put out indefinitely 
                while (--numSamples >= 0)
                {
                    auto currentSample = (float)(std::sin(currentAngle) * level);

                    for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                    {
                        outputBuffer.addSample(i, startSample, currentSample);
                    }
                    currentAngle += angleDelta;
                    ++startSample;
                }
            }
            else
            {
                // if the attackLvl has not reached one, i.e if the attackTime has not completed then increment the attack Lvl 
                // level defines the overall max value when the key is pressed down. 
                while (--numSamples >= 0)
                {
                    auto currentSample = (float)(std::sin(currentAngle) * level * attackLvl);

                    for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                    {
                        outputBuffer.addSample(i, startSample, currentSample);
                    }
                    currentAngle += angleDelta;
                    ++startSample;

                    attackLvl += attackDelta;

                }
            }
        }
    }

private:
    double angleDelta = 0.0, currentAngle = 0.0;
    double tailOff = 0.0, level = 0.0;
    double attackLvl = 0.0, releaseLvl = 0.0;
    double attackDelta = 0.0, releaseDelta = 0.0;

    juce::Slider &attackSlider, &releaseSlider;    
};

//! [SynthAudioSource class]
class SynthAudioSource : public juce::AudioSource
{
    // the class instantiates the whole synth object : sample rate, audio buffers, midi buffers ,midisource, etc.
public:
    //==============================================================================
    SynthAudioSource(juce::MidiKeyboardState& keyState, juce::Slider &attack, juce::Slider &release) :
        keyboardState(keyState),
        attackSlider(attack),
        releaseSlider(release)
    {
        for (int i = 0; i < 4 ; ++i)
        {
            synth.addVoice(new SineWaveVoice(attackSlider,releaseSlider)); // adding some voices to the synth
        }
        synth.addSound(new SineWaveSound()); // defining which sound to play, here all the voices will play the same SineWave Sound
    }


    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
    {
        /* samplesPerBlockExpected not used here */
        synth.setCurrentPlaybackSampleRate(sampleRate);
        // The synthesiser needs to know the sample rate of the audio output.
        
    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override 
    {
        bufferToFill.clearActiveBufferRegion();
        juce::MidiBuffer incomingMidi;
        keyboardState.processNextMidiBuffer(       incomingMidi, bufferToFill.startSample, bufferToFill.numSamples, true);
        // we pull buffers of MIDI data from the MidiKeyboardState object.
        synth.renderNextBlock(*bufferToFill.buffer,incomingMidi, bufferToFill.startSample, bufferToFill.numSamples);
        // These buffers of MIDI are passed to the synthesiser which will be used to render the voices using the timestamps
        // of the note-on and note-off messages (and other MIDI channel voice messages).
    }

    void releaseResources() override{ 
    // nothing to do here 
    }

private:
    juce::Synthesiser synth; // the synthesizer is compromised of multiples voices and sounds
    juce::MidiKeyboardState& keyboardState;
    juce::Slider &attackSlider, &releaseSlider;

};

//! [MainComponent Class]
class MainComponent  : public juce::AudioAppComponent,
    private juce::MidiKeyboardStateListener
{
public:
    //==============================================================================
    MainComponent() : synthAudioSource(keyboardState,attackSlider,releaseSlider), 
        keyboardComponent(keyboardState,juce::KeyboardComponentBase::horizontalKeyboard)
    {
        addAndMakeVisible(keyboardComponent);
        setAudioChannels(0, 2);

        keyboardState.addListener(this);

        setSize(800, 600);

        // Sliders and Labels
        addAndMakeVisible(attackSlider);
        attackSlider.setRange(1.0, 5000.0);
        attackSlider.setTextValueSuffix("ms");
        attackSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 160, attackSlider.getTextBoxHeight());
        attackSlider.setSkewFactorFromMidPoint(500);

        addAndMakeVisible(releaseSlider);
        releaseSlider.setRange(1.0, 5000.0);
        releaseSlider.setTextValueSuffix("ms");
        releaseSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 160, releaseSlider.getTextBoxHeight());
        releaseSlider.setSkewFactorFromMidPoint(500);

        addAndMakeVisible(attackLabel);
        attackLabel.attachToComponent(&attackSlider,false);
        attackLabel.setText("Attack", juce::dontSendNotification);

        addAndMakeVisible(releaseLabel);
        releaseLabel.attachToComponent(&releaseSlider, false);
        releaseLabel.setText("Release", juce::dontSendNotification);
    }


    ~MainComponent() override {
        shutdownAudio();
    }

    //==============================================================================
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override {
        synthAudioSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    }
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override {
        bufferToFill.clearActiveBufferRegion();
        synthAudioSource.getNextAudioBlock(bufferToFill);
        
    }
    void releaseResources() override {
        synthAudioSource.releaseResources();
        keyboardState.removeListener(this);
    }

    //==============================================================================
    void paint(juce::Graphics& g) override {
        g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    }
    void resized() override {
        auto area = getLocalBounds();
        keyboardComponent.setBounds(10, 40, getWidth() - 20, 100);
        attackSlider.setBounds(10, keyboardComponent.getBottom() + 30, getWidth() - 20, 20);
        releaseSlider.setBounds(10, keyboardComponent.getBottom() + 80, getWidth() - 20, 20);
    }

private:
    void handleNoteOff(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float /*velocity*/) override {    }
    void handleNoteOn(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override {}
    juce::MidiKeyboardState keyboardState;
    juce::MidiKeyboardComponent keyboardComponent;
    SynthAudioSource synthAudioSource;

    juce::Slider attackSlider;
    juce::Slider releaseSlider;
    juce::Label attackLabel;
    juce::Label releaseLabel;
    double attack = 0.0, release = 0.0;

    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

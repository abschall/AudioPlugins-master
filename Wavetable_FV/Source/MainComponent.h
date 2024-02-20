#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class SineOscillator
{
public:
    SineOscillator()    {}
    ~SineOscillator()   {}

    forcedinline float getNextSample() noexcept
    {
        auto currentSample = std::sin(currentAngle);
        updateAngle();
        return currentSample;
    }

    void setFrequency(float frequency, float sampleRate)
    {
        auto cyclesPerSample = frequency / sampleRate;
        angleDelta = cyclesPerSample * 2 * juce::MathConstants<float>::pi;

    }
    forcedinline void updateAngle() noexcept
    {
        currentAngle += angleDelta;
    }

private:

    double angleDelta = 0.0, currentAngle = 0.0;

};


class WavetableOscillator
{
public:
    WavetableOscillator(const juce::AudioSampleBuffer& waveTableToUse) : wavetable(waveTableToUse),
        tableSize(waveTableToUse.getNumSamples()-1)
    {
        jassert (wavetable.getNumChannels() == 1);
    }
    ~WavetableOscillator() {}

    // ==========================================================================================
    void setFrequency(float frequency, float sampleRate)
    {
        auto samplesPerSampleRate = (float) wavetable.getNumSamples()/ sampleRate;
        tableDelta = samplesPerSampleRate*frequency;

    }

    forcedinline float getNextSample() noexcept
    {
        //auto tableSize = (unsigned int) wavetable.getNumSamples();
        auto index0 = (unsigned int) currentIndex;
        auto index1 =  index0 + 1;

        // auto* samples = wavetable.getReadPointer(0);
        // auto value0 = samples[index0];
        // auto value1 = samples[index1];
        auto value0 = wavetable.getSample(0,index0);
        auto value1 = wavetable.getSample(0, index1);
        

        auto interpolatedValue = value0 + (value1 - value0)* (currentIndex- (float) index0);
        
        if ((currentIndex += tableDelta) > (float) tableSize)
            currentIndex -= (float) tableSize;
        return interpolatedValue;


    }

    // ==========================================================================================
private:
    const juce::AudioSampleBuffer& wavetable;
    const int tableSize;
    float tableDelta = 0.0f;
    float currentIndex = 0.0f;
};

class MainComponent  : public juce::AudioAppComponent
{
public:    
    //==============================================================================
    // Wavetable functions

    //==============================================================================
    MainComponent()
    {        
        // createWavetableSine(); // FIRST THING TO DO !!!!!
        createWavetableSineHarmonics();
        setSize(600, 800);
        setAudioChannels(0, 2);
    }

    ~MainComponent() override
    {
        // This shuts down the audio device and clears the audio source.
        shutdownAudio();
    }

    //==============================================================================
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
    {
        auto numberOfOscillators = 1;

        for (auto i = 0; i < numberOfOscillators; ++i)
        {
            auto wtOscillator = new WavetableOscillator(SineHarmonicsTable);

            auto midiNote = juce::Random::getSystemRandom().nextDouble() * 36.0 + 48.0;
            auto frequency = 440.0 * pow(2.0, (midiNote - 69.0) / 12.0);

            wtOscillator->setFrequency((float)frequency, (float)sampleRate);
            wtOscillators.add(wtOscillator);
        }
        /*
        * // SineOscillator version
        for (auto i = 0; i < numberOfOscillators; ++i)
        {
            auto oscillator = new SineOscillator();
            auto midiNote = juce::Random::getSystemRandom().nextDouble() * 36.0 + 48.0; // [3]
            auto frequency = 440.0 * pow(2.0, (midiNote - 69.0) / 12.0);

            oscillator->setFrequency(frequency, sampleRate);
            oscillators.add(oscillator);
        }        */
        level = 0.25 / numberOfOscillators;
    }
        

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        
        auto bufferL = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
        auto bufferR = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);

        // getWritePointer retrns a writable pointer to one of the audioBuffers 
        bufferToFill.clearActiveBufferRegion();

        for (auto oscillatorIndex = 0; oscillatorIndex < wtOscillators.size(); ++oscillatorIndex)
        {
            //auto oscillator = oscillators.getUnchecked(oscillatorIndex);
            auto* oscillator = wtOscillators.getUnchecked(oscillatorIndex);
            for (auto sample = bufferToFill.startSample; sample < bufferToFill.numSamples; ++sample)
            {
                auto currentSample = oscillator->getNextSample();
                bufferL[sample] += currentSample * level;
                bufferR[sample] += currentSample * level;
            }
        }

    }

    void releaseResources() override
    {

    }

    //==============================================================================
    void paint(juce::Graphics& g) override
    {
        g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    }
    void resized() override
    {

    }

private:
    void createWavetableSine()
    {
        SineTable.setSize(1, (int)tableSize + 1);
        auto angleDelta = juce::MathConstants<float>::twoPi / (double)(tableSize - 1);
        auto currentAngle = 0.0;
        auto* samples = SineTable.getWritePointer(0);

        for (unsigned int i = 0; i < tableSize; ++i)
        {
            auto sample = (float)(std::sin(currentAngle));
            samples[i] = sample;
            //SineTable.setSample(0, i, sample); // seems to be identical 
            currentAngle += angleDelta;
        }
        samples[tableSize] = samples[0];
    }

    void createWavetableSineHarmonics()
    {
        SineHarmonicsTable.setSize(1, (int)tableSize + 1);
        SineHarmonicsTable.clear();
        // int harmonics[] = { 1,3,5,7,9,11,13,15}; // odd
        // int harmonics[] = { 0,2,4,6,8,10,12,14}; //even
        int harmonics[] = { 0,1,2,3,4,5,6,7 }; // odd and even 
        float harmonicsWeight[] = { 0.5f, 0.1f, 0.05f, 0.125f, 0.09f, 0.005f, 0.002f, 0.001f };

        auto* samples = SineHarmonicsTable.getWritePointer(0);
        int iter = 0;
        for (auto selectedHarmonic : harmonics)
        {
            auto angleDelta = juce::MathConstants<float>::twoPi * selectedHarmonic / (double)(tableSize - 1);
            auto currentAngle = 0.0;
            for (unsigned int i = 0; i < tableSize; ++i)
            {
                auto sample = (float)(std::sin(currentAngle));
                samples[i] += (float) sample *harmonicsWeight[iter];
                //SineTable.setSample(0, i, sample); // seems to be identical 
                currentAngle += angleDelta;
            }
            samples[tableSize] = samples[0];
            iter += 1;
        }
    }
        



    float level = 0.0;
    juce::OwnedArray<SineOscillator> oscillators; // sineOscillator synthesis
    juce::AudioSampleBuffer SineTable; //creating an AuioBuffer of 32 floating point and of size 1 << 7 ( 128)
    juce::AudioSampleBuffer SineHarmonicsTable;
    const unsigned int tableSize = 1 << 7; // equivalent to 128
    juce::OwnedArray<WavetableOscillator> wtOscillators;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

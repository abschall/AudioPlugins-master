#pragma once

#include "../../dsp_fv/combFilterWithFB.h"
#include <vector>
#include "../../dsp_fv/classicFilters.h"

using std::vector;

class FilteredNoise : private ClassicFilters
{
public:
    void createFilteredNoise(float pSampleRate)
    {
        lopFilter.setFilterType("LPF1");
        hipFilter.setFilterType("HPF1");

        lopFilter.setCoefficients(890, 1, pSampleRate);
        hipFilter.setCoefficients(6600, 1, pSampleRate);
        lopFilter.setFilterGain(1.0f);
        hipFilter.setFilterGain(1.0f);
    }

    float sound()
    {
        auto noiseSample = random.nextFloat() * 0.125;
        auto noiseSampleProcessed = level * lopFilter.processAudioSample(hipFilter.processAudioSample(noiseSample));
        return noiseSampleProcessed;
    }
private:    

    ClassicFilters lopFilter;
    ClassicFilters hipFilter;
    float level = 0.25f;
    juce::Random random;
};

class MultiTapDelay : public CombFilterWithFB
{
public:
    void setParameters(double pCurrentSampleRate, double pDelayTimeMs, double pRatioBetweenTaps,
        unsigned int pNumberOfTaps, float pDry, float pWet, float pFeedbackGain)
    {
        currentSampleRate = pCurrentSampleRate;
        timeRatio = pRatioBetweenTaps;
        setNumberOfTaps(pNumberOfTaps);
        setDelayTimeInMs(pDelayTimeMs);
        setDryWetLevels(pDry, pWet);
        setFeedbackGain(pFeedbackGain);
    }

    void createNoise(float pInitialLevel)
    {
        noiseSource.createFilteredNoise(currentSampleRate);
        noiseLevel = pInitialLevel;
    }

    void setNumberOfTaps(unsigned int N)
     {
         numberOfTaps = N;
     }
    //===============================================================
    // Methods to control the taps 
    void instantiateTaps()
     {
         for (int tap = 0; tap < numberOfTaps; ++tap)
         {
             if (tap == 0)
             {
                 tapDelayTimesInSamples.push_back(delayTimeInSamples);
                 tapLevels.push_back(1.0f);
             }
             else
             {
                 tapDelayTimesInSamples.push_back(tapDelayTimesInSamples[tap - 1] * timeRatio);
                 tapLevels.push_back(0.0f);
             }
         }
     }

    void setTapsDelayTime()
    {
        for (unsigned int tap = 0; tap < numberOfTaps; ++tap)
        {
            if (tap == 0)
            {
                tapDelayTimesInSamples[tap] = delayTimeInSamples;
            }
            else
                tapDelayTimesInSamples[tap] = tapDelayTimesInSamples[tap - 1] * timeRatio;
        }
    }

    void setTapLevels(vector<float> pTapLevels)
    {
        for (auto tap = 0; tap < numberOfTaps; ++tap)
        {
            tapLevels[tap] = pTapLevels[tap];
        }
    }
    //===============================================================
    // Methods to control the delays color (Noise and saturation)
    void setNoiseLevel(float pNoiseLevel)
    {
        noiseLevel = pNoiseLevel;
    }

    //===============================================================
    // Methods to set the low pass and highpass filter 
    void setFiltersParameters(float lowpassFrequency, float highpassFrequency)
    {
        lopFilter.setFilterType("LPF1");
        hipFilter.setFilterType("HPF1");
        lopFilter.setCoefficients(lowpassFrequency , 1, currentSampleRate);
        hipFilter.setCoefficients(highpassFrequency, 1, currentSampleRate);
    }

    void updateFiltersParameters(float lowpassFrequency, float highpassFrequency)
    {
        lopFilter.setCoefficients(lowpassFrequency, 1, currentSampleRate);
        hipFilter.setCoefficients(highpassFrequency, 1, currentSampleRate);
    }

    float processAudioSample(float inputXn) override
    {
        float ynD = 0.0;
        float sum = 0.0;

        for (auto tap = 0; tap < numberOfTaps; ++tap)
        {
            ynD += tapLevels[tap] * delayBuffer.readBuffer(tapDelayTimesInSamples[tap]);
            sum += tapLevels[tap];
        }

        if (sum < 1.0f)
            sum = 1.0f;

        ynD = ynD / sum; // normalise level of taps to 1, to avoid excessive feedback
        ynD += noiseLevel * noiseSource.sound(); // adding noise to the FB path signal
        auto ynDFiltered = hipFilter.processAudioSample(lopFilter.processAudioSample(ynD)); // filtering the signal 

        auto ynFullWet = inputXn + feedbackGain *ynDFiltered ;
        delayBuffer.writeBuffer(ynFullWet);
        auto yn = dry * inputXn + wet * ynDFiltered;

        return yn;
    }

private:
    FilteredNoise noiseSource;
    float noiseLevel;
	double timeRatio;
    vector<float> tapLevels;
    vector<float> tapDelayTimesInSamples;
    unsigned int numberOfTaps = 4;
    ClassicFilters lopFilter;
    ClassicFilters hipFilter;
};


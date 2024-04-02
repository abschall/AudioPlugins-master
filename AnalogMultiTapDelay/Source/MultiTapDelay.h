#pragma once

#include "../../dsp_fv/combFilterWithFB.h"
#include <vector>
#include "../../dsp_fv/classicFilters.h"

using std::vector;

class FilteredNoise : private ClassicFilters
{
public:
    /// <summary>
    /// Initializes the filtered noise generator with specific filter settings.
    /// </summary>
    /// <param name="pSampleRate">The sample rate at which the filters will operate.</param>
    void createFilteredNoise(float pSampleRate)
    {
        // Setting up the low-pass filter with a cutoff frequency of 890 Hz
        lopFilter.setFilterType("LPF1");
        // Setting up the high-pass filter with a cutoff frequency of 6600 Hz
        hipFilter.setFilterType("HPF1");

        // Configuring the coefficients for both filters
        lopFilter.setCoefficients(890, 1, pSampleRate);
        hipFilter.setCoefficients(6600, 1, pSampleRate);

        // Setting the gain for both filters to unity gain
        lopFilter.setFilterGain(1.0f);
        hipFilter.setFilterGain(1.0f);
    }

    /// <summary>
    /// Generates a filtered noise sample.
    /// </summary>
    /// <returns>A float representing the filtered noise sample.</returns>
    float sound()
    {
        // Generating a random noise sample and scaling it
        auto noiseSample = random.nextFloat() * 0.125;
        // Processing the noise sample through the high-pass and then low-pass filter
        auto noiseSampleProcessed = level * lopFilter.processAudioSample(hipFilter.processAudioSample(noiseSample));
        return noiseSampleProcessed;
    }
private:

    ClassicFilters lopFilter;
    ClassicFilters hipFilter;
    float level = 0.25f; // The level of the noise
    juce::Random random; // Random number generator for noise generation
};
/// <summary>
/// The MultiTapDelay algorithm
/// </summary>
class MultiTapDelay : public CombFilterWithFB_stereo
{
public:
    void setParameters(double pCurrentSampleRate, double pDelayTimeMs, double pRatioBetweenTaps,
        unsigned int pNumberOfTaps, float pDry, float pWet, float pFeedbackGain, float pStereoWidth)
    {
        currentSampleRate = pCurrentSampleRate;
        timeRatio = pRatioBetweenTaps;
        setNumberOfTaps(pNumberOfTaps);
        setDelayTimeInMs(pDelayTimeMs);
        setDryWetLevels(pDry, pWet);
        setFeedbackGain(pFeedbackGain);
        stereoWidth = pStereoWidth;
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

    /// <summary>
    /// Instantitates the Taps, called at prepareToPlay()
    /// </summary>
    void instantiateTaps()
     {
         for (int tap = 0; tap < numberOfTaps; ++tap)
         {
             if (tap == 0)
             {
                 tapDelayTimesInSamplesL.push_back(delayTimeInSamples);
                 tapDelayTimesInSamplesR.push_back(delayTimeInSamples );
                 tapLevels.push_back(1.0f);
             }
             else
             {
                 tapDelayTimesInSamplesL.push_back(tapDelayTimesInSamplesL[tap - 1] * timeRatio);
                 tapDelayTimesInSamplesR.push_back(tapDelayTimesInSamplesR[tap - 1] * timeRatio);
                 tapLevels.push_back(0.0f);
             }
         }
     }

    /// <summary>
    /// Sets initial delay times
    /// </summary>
    void setTapsDelayTime()
    {
        for (unsigned int tap = 0; tap < numberOfTaps; ++tap)
        {
            if (tap == 0)
            {
                tapDelayTimesInSamplesL[tap] = delayTimeInSamples;
                tapDelayTimesInSamplesR[tap] = delayTimeInSamples;
            }
            else
            {
                tapDelayTimesInSamplesL[tap] = tapDelayTimesInSamplesL[tap - 1] * timeRatio;
                tapDelayTimesInSamplesR[tap] = tapDelayTimesInSamplesR[tap - 1] * timeRatio;
            }

        }
    }
    /// <summary>
    /// Sets initial tap Levels
    /// </summary>
    /// <param name="pTapLevels"></param>
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
        // setting Left Side filters 
        lopFilterL.setFilterType("LPF1");
        hipFilterL.setFilterType("HPF1");
        lopFilterL.setCoefficients(lowpassFrequency , 1, currentSampleRate);
        hipFilterL.setCoefficients(highpassFrequency, 1, currentSampleRate);

        // setting Right Side filters 
        lopFilterR.setFilterType("LPF1");
        hipFilterR.setFilterType("HPF1");
        lopFilterR.setCoefficients(lowpassFrequency, 1, currentSampleRate);
        hipFilterR.setCoefficients(highpassFrequency, 1, currentSampleRate);
    }

    void updateFiltersParameters(float lowpassFrequency, float highpassFrequency)
    {
        //update Left Side Filter
        lopFilterL.setCoefficients(lowpassFrequency,  1, currentSampleRate);
        hipFilterL.setCoefficients(highpassFrequency, 1, currentSampleRate);

        //update Right Side Filter
        lopFilterR.setCoefficients(lowpassFrequency,  1, currentSampleRate);
        hipFilterR.setCoefficients(highpassFrequency, 1, currentSampleRate);
    }

    /// <summary>
    /// Processes left and right input samples 
    /// </summary>
    /// <param name="inputXnL"></param>
    /// <param name="inputXnR"></param>
    /// <returns> returns the output sample</returns>
    vector<float> processAudioSample(float inputXnL, float inputXnR) override
    {
        auto ynDL = 0.0f, ynDR = 0.0f, sum = 0.0f;
        auto noise = noiseLevel * noiseSource.sound();
        for (auto tap = 0; tap < numberOfTaps; ++tap)
        {
            // reading taps
            ynDL += tapLevels[tap] * delayBufferL.readBuffer(tapDelayTimesInSamplesL[tap]);
            ynDR += tapLevels[tap] * delayBufferR.readBuffer(tapDelayTimesInSamplesR[tap]);

            sum += tapLevels[tap];
        }

        if (sum < 1.0f)
            sum = 1.0f;

        ynDL = ynDL / sum; // normalise level of taps to 1, to avoid excessive feedback
        ynDR = ynDR / sum;
        ynDL = ynDL + noise;
        ynDR = ynDR + noise;

        // passes the audio through the filters 
        auto ynFullWetL = hipFilterL.processAudioSample(lopFilterL.processAudioSample(inputXnL + feedbackGain * ynDL));
        auto ynFullWetR = hipFilterL.processAudioSample(lopFilterR.processAudioSample(inputXnR + feedbackGain * ynDR));

        // writes samples to delay buffers
        delayBufferL.writeBuffer(ynFullWetR);
        delayBufferR.writeBuffer(ynFullWetL);

        // generates output samples
        vector<float> yn = { stereoWidth * (dry * inputXnL + wet * ynDL) + (1 - stereoWidth) * (dry * inputXnR + wet * ynDR),
                             stereoWidth * (dry * inputXnR + wet * ynDR) + (1 - stereoWidth) * (dry * inputXnL + wet * ynDL) };
        return yn;
    }

private:
    FilteredNoise noiseSource;
    float stereoWidth;
    float noiseLevel;
	double timeRatio;
    vector<float> tapLevels;
    vector<float> tapDelayTimesInSamplesL;
    vector<float> tapDelayTimesInSamplesR;
    unsigned int numberOfTaps = 4;

    // Each channel requires its own signal processing  chain, one filter / channel, one saturation / channel, etc..
    // except the noise which is just an added signal, input signal does not pass through it processing block

    ClassicFilters lopFilterL, lopFilterR;
    ClassicFilters hipFilterL, hipFilterR;
};


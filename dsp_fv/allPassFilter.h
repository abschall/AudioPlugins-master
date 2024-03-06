#pragma once

#include "circularBuffer.h"
/// <summary>
/// AllPass Filter structure as described by MR Schroeder in the 1961 paper
/// "Colorless" Artificial Reverberation, 1961
/// </summary>
class allPassFilter : public CircularBuffer<float>
{
public:

    void setParameters(double pCurrentSampleRate, double pDelayTimeMs, float pFeedbackGain, bool pEnableAPF)
    {
        currentSampleRate = pCurrentSampleRate;
        setDelayTimeInMs(pDelayTimeMs);
        setFeedbackGain(pFeedbackGain);
        enableAPF = pEnableAPF;
    }
    /// <summary>
/// Delay time in the circular buffer is set in samples. 
/// This method sets the delay time in number of samples directly
/// </summary>
/// <param name="pDelayTimeInSamples"></param>
    virtual void setDelayTimeInSamples(unsigned int pDelayTimeInSamples)
    {
        delayTimeInSamples = pDelayTimeInSamples;

    }
    /// <summary>
    /// sets the feedback gain of the APF
    /// </summary>
    /// <param name="feedback"></param>
    void setFeedbackGain(float feedback)
    {
        feedbackGain = feedback;
    }
    /// <summary>
    /// Delay time in the circular buffer is set in samples. This function converts the delay time given in ms, to
    /// number of samples using the sample rate.
    /// </summary>
    /// <param name="pDelayTimeInMs"></param>
    void setDelayTimeInMs(unsigned int pDelayTimeInMs)
    {
        delayTimeInSamples = pDelayTimeInMs * samplePerMsec;
    }
    /// <summary>
    /// intantiates a delay buffer. the length is calculated from the given 
    /// time in ms and converted to number of samples
    /// </summary>
    /// <param name="pSampleRate"></param>
    /// <param name="pBufferLengthMsec"></param>
    void createDelayBuffer(double pSampleRate, double pBufferLengthMsec)
    {
        bufferLengthMsec = pBufferLengthMsec;
        auto samplePerMsec = currentSampleRate / 1000.0;
        auto bufferLength = (unsigned int)(bufferLengthMsec * samplePerMsec) + 1;
        delayBuffer.createBuffer(bufferLength);
    }
    /// <summary>
    /// process the audio sample by an APF , mono input, mono output
    /// </summary>
    /// <param name="inputXn"></param>
    /// <returns></returns>
    virtual float processAudioSample(float inputXn)
    {
        auto ynD = delayBuffer.readBuffer(delayTimeInSamples);
        delayBuffer.writeBuffer(inputXn + ynD * feedbackGain);
        auto yn = ynD * (1 - feedbackGain * feedbackGain) + inputXn * (-feedbackGain);

        return yn;
    }

protected:
    double bufferLengthMsec;
    double currentSampleRate;
    unsigned int delayTimeInSamples;
    float feedbackGain;
    CircularBuffer delayBuffer;

    bool enableAPF;
};
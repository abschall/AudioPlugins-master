#pragma once

#include "circularBuffer.h"
/// <summary>
/// Comb Filter (one channel) with Feedback, inherits from the CircularBuffer class
/// Public members are : dry, wet (mix) and feedback ((gain) ratio of output signal added to input)
///  in the feedback path 
/// </summary>
class CombFilterWithFB : public CircularBuffer<float>
{
public:
    /// <summary>
    /// sets all the comb filters parameters:  mix, feedback, delay time.
    /// sample Rate is necessary
    /// </summary>
    /// <param name="pCurrentSampleRate"></param>
    /// <param name="pDelayTimeMs"></param>
    /// <param name="pDry"></param>
    /// <param name="pWet"></param>
    /// <param name="pFeedbackGain"></param>
    virtual void setParameters(double pCurrentSampleRate, double pDelayTimeMs,
        float pDry, float pWet,float pFeedbackGain)
    {
        currentSampleRate = pCurrentSampleRate;
        setDelayTimeInMs(pDelayTimeMs);
        setDryWetLevels(pDry, pWet);
        setFeedbackGain(pFeedbackGain);
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
    /// Delay time in the circular buffer is set in samples. This function converts the delay time given in ms, to
    /// number of samples using the sample rate.
    /// </summary>
    /// <param name="pDelayTimeInMs"></param>
    void setDelayTimeInMs(unsigned int pDelayTimeInMs)
    {
        delayTimeInSamples = pDelayTimeInMs * samplePerMsec;
    }

    /// <summary>
    /// process the audio sample, outputs the sum of input (dry) signal and processed (wet) signal
    /// </summary>
    /// <param name="inputXn"></param>
    /// <returns></returns>
    //virtual vector<float> processAudioSample(float inputXnL,float inputXnR)
    //{
    //    auto ynD = delayBufferL.readBuffer(delayTimeInSamples);
    //    auto ynFullWet = inputXnL + feedbackGain * ynD;
    //    delayBufferL.writeBuffer(ynFullWet);
    //    vector<float> yn = { dry * inputXnL + wet * ynD };
    //    return yn;
    //}
    virtual float processAudioSample(float inputXn)
    {
        auto ynD = delayBuffer.readBuffer(delayTimeInSamples);
        auto ynFullWet = inputXn + feedbackGain * ynD;
        delayBuffer.writeBuffer(ynFullWet);
        float yn =  dry * inputXn + wet * ynD;
        return yn;
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
        currentSampleRate = pSampleRate;
        samplePerMsec = currentSampleRate / 1000.0;
        bufferLength = (unsigned int)(bufferLengthMsec * samplePerMsec) + 1;
        delayBuffer.createBuffer(bufferLength);
        delayBuffer.flush();
    }

    /// <summary>
    /// set dry and wet parameters( usually both are correlated) : dry = 1 - wet
    /// </summary>
    /// <param name="pDry"></param>
    /// <param name="pWet"></param>
    void setDryWetLevels(float pDry, float pWet)
    {
        dry = pDry;
        wet = pWet;
    }
    /// <summary>
    /// set the feedback ratio (float number between 0 and 1)
    /// </summary>
    /// <param name="feedback"></param>
    void setFeedbackGain(float feedback)
    {
        feedbackGain = feedback;
    }

    // Public member variables
    float dry;
    float wet;
    float feedbackGain;

protected:
    double bufferLengthMsec;
    unsigned int bufferLength; // in samples
    unsigned int delayTimeInSamples;
    //unsigned int delayTimeInSamples;

    CircularBuffer delayBuffer;
    double currentSampleRate;
    double samplePerMsec;
private:

};

/// <summary>
/// Stereo Comb Filter (R and L channels) with Feedback, inherits from the CircularBuffer class
/// Public members are : dry, wet (mix) and feedback ((gain)ratio of output signal added to input)
///
/// </summary>
class CombFilterWithFB_stereo : public CircularBuffer<float>
{
public:
    /// <summary>
    /// sets all the comb filters parameters:  mix, feedback, delay time.
    /// sample Rate is necessary
    /// </summary>
    /// <param name="pCurrentSampleRate"></param>
    /// <param name="pDelayTimeMs"></param>
    /// <param name="pDry"></param>
    /// <param name="pWet"></param>
    /// <param name="pFeedbackGain"></param>
    virtual void setParameters(double pCurrentSampleRate, double pDelayTimeMs,
        float pDry, float pWet, float pFeedbackGain)
    {
        currentSampleRate = pCurrentSampleRate;
        setDelayTimeInMs(pDelayTimeMs);
        setDryWetLevels(pDry, pWet);
        setFeedbackGain(pFeedbackGain);
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
    /// Delay time in the circular buffer is set in samples. This function converts the delay time given in ms, to
    /// number of samples using the sample rate.
    /// </summary>
    /// <param name="pDelayTimeInMs"></param>
    void setDelayTimeInMs(unsigned int pDelayTimeInMs)
    {
        delayTimeInSamples = pDelayTimeInMs * samplePerMsec;
    }

    /// <summary>
    /// process the audio sample, outputs the sum of input (dry) signal and processed (wet) signal
    /// </summary>
    /// <param name="inputXn"></param>
    /// <returns></returns>
    //virtual vector<float> processAudioSample(float inputXnL,float inputXnR)
    //{
    //    auto ynD = delayBufferL.readBuffer(delayTimeInSamples);
    //    auto ynFullWet = inputXnL + feedbackGain * ynD;
    //    delayBufferL.writeBuffer(ynFullWet);
    //    vector<float> yn = { dry * inputXnL + wet * ynD };
    //    return yn;
    //}
    virtual vector<float> processAudioSample(float inputXnL, float inputXnR)
    {
        auto ynDL = delayBufferL.readBuffer(delayTimeInSamples);
        auto ynDR = delayBufferR.readBuffer(delayTimeInSamples);
        auto ynFullWetL = inputXnL + feedbackGain * ynDL;
        auto ynFullWetR = inputXnR + feedbackGain * ynDR;
        delayBufferL.writeBuffer(ynFullWetR);
        delayBufferR.writeBuffer(ynFullWetL);
        vector<float> yn = { dry * inputXnL + wet * ynDL,dry * inputXnR + wet * ynDR };
        return yn;
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
        currentSampleRate = pSampleRate;
        samplePerMsec = currentSampleRate / 1000.0;
        bufferLength = (unsigned int)(bufferLengthMsec * samplePerMsec) + 1;
        delayBufferL.createBuffer(bufferLength);
        delayBufferR.createBuffer(bufferLength);
        delayBufferL.flush();
        delayBufferR.flush();
    }

    /// <summary>
    /// set dry and wet parameters( usually both are correlated) : dry = 1 - wet
    /// </summary>
    /// <param name="pDry"></param>
    /// <param name="pWet"></param>
    void setDryWetLevels(float pDry, float pWet)
    {
        dry = pDry;
        wet = pWet;
    }
    /// <summary>
    /// set the feedback ratio (float number between 0 and 1)
    /// </summary>
    /// <param name="feedback"></param>
    void setFeedbackGain(float feedback)
    {
        feedbackGain = feedback;
    }

    // Public member variables
    float dry;
    float wet;
    float feedbackGain;

protected:
    double bufferLengthMsec;
    unsigned int bufferLength; // in samples
    unsigned int delayTimeInSamples;
    //unsigned int delayTimeInSamples;

    CircularBuffer delayBufferR;
    CircularBuffer delayBufferL;
    double currentSampleRate;
    double samplePerMsec;
private:

};




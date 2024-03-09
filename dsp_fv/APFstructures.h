#pragma once

#include "circularBuffer.h"

struct CombFilterParameters
{
	double delayTimeInMs = 0.0;
	float feedbackGain = 0.0;
	bool enableComb = false;
	double delayTimeInSamples = 0.0;
};

/// <summary>
/// simple recirculating Comb Filter class, the comb filters have fixed delay times and buffer length
/// !!! This should be modified to a common max buffer length ! The class as is (3/08/2024) does not 
/// lend itself to delay time modulation 
/// </summary>
class CombFilter : private CircularBuffer<float>
{
public:
	/// <summary>
	/// sets the delay time in ms, feedback loop gain, and enables/ disables  the comb filter 
	/// </summary>
	/// <param name="pParameters"></param>
	void setParameters(CombFilterParameters pParameters)
	{
		parameters.delayTimeInMs = pParameters.delayTimeInMs;
		parameters.feedbackGain = pParameters.feedbackGain;
		parameters.enableComb = pParameters.enableComb;
	}

	/// <summary>
	/// creates the Comb Filter's Delay Buffer (bufferLength = delay time),
	/// also sets the delay time in number of samples according to the sample rate 
	/// </summary>
	/// <param name="pSampleRate"></param>
	void createDelayBuffer(double pSampleRate)
	{
		currentSampleRate = pSampleRate;
		auto samplePerMsec = currentSampleRate / 1000.0;
		auto bufferLength = (unsigned int)(parameters.delayTimeInMs * samplePerMsec) + 1;
		parameters.delayTimeInSamples = parameters.delayTimeInMs * samplePerMsec;
		delayBuffer.createBuffer(bufferLength);
	}
	/// <summary>
	/// processes the incoming audio sample, output is full wet 
	/// </summary>
	/// <param name="inputXn"></param>
	/// <returns></returns>
	virtual float processAudioSample(float inputXn)
	{
		// full wet signal processing 

		if (parameters.enableComb == true)
		{

			auto ynD = delayBuffer.readBuffer(parameters.delayTimeInSamples);
			auto ynFullWet = inputXn + parameters.feedbackGain * ynD;
			delayBuffer.writeBuffer(ynFullWet);

			return ynD;
		}
		else
			return inputXn;

	}
protected:
	CombFilterParameters parameters;
	double currentSampleRate;
	CircularBuffer delayBuffer;
};

struct APFParameters
{
	double delayTimeInMs = 0.0;
	float feedbackGain = 0.0;
	bool enableAPF = false;
	double delayTimeInSamples;
};

/// <summary>
/// APF class,as described in the MR Schroeder 1961 and 1962 reverberation papers
/// </summary>
class allPassFilter : private CircularBuffer<float>
{
public:
	/// <summary>
	/// sets the delay time in ms, feedback loop gain, and enables/ disables  the comb filter 
	/// </summary>
	/// <param name="pParameters"></param>
	void setParameters(APFParameters pParameters)
	{
		parameters.delayTimeInMs = pParameters.delayTimeInMs;
		parameters.feedbackGain = pParameters.feedbackGain;
		parameters.enableAPF = pParameters.enableAPF;
	}
	/// <summary>
	/// creates the Comb Filter's Delay Buffer (bufferLength = delay time),
	/// also sets the delay time in number of samples according to the sample rate 
	/// </summary>
	/// <param name="pSampleRate"></param>
	void createDelayBuffer(double pSampleRate)
	{
		currentSampleRate = pSampleRate;
		auto samplePerMsec = currentSampleRate / 1000.0;
		auto bufferLength = (unsigned int)(parameters.delayTimeInMs * samplePerMsec) + 1;
		parameters.delayTimeInSamples = parameters.delayTimeInMs * samplePerMsec;
		delayBuffer.createBuffer(bufferLength);
	}
	/// <summary>
	/// processes the incoming audio sample, output is full wet 
	/// </summary>
	/// <param name="inputXn"></param>
	/// <returns></returns>
	virtual float processAudioSample(float inputXn)
	{
		auto ynD = 0.0;
		if (parameters.enableAPF == true)
		{
			ynD = delayBuffer.readBuffer(parameters.delayTimeInSamples);
			delayBuffer.writeBuffer(inputXn + ynD * parameters.feedbackGain);
			auto yn = (1 - parameters.feedbackGain * parameters.feedbackGain) * ynD + inputXn * (-parameters.feedbackGain);

			return yn;
		}
		else
		{
			return inputXn;
		}

	}
protected:
	APFParameters parameters;
	double currentSampleRate;
	CircularBuffer delayBuffer;
};

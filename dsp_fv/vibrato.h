#pragma once

#include "circularBuffer.h"
#include "lfo.h"	

//From W. Pirkle Modulation functions 
inline void boundValue(double& value, double minValue, double maxValue)
{
	value = fmin(value, maxValue);
	value = fmax(value, minValue);
}

//From W. Pirkle Modulation functions 
inline double doBipolarModulation(double bipolarModulatorValue, double minValue, double maxValue)
{
	// --- BIPOLAR bound
	boundValue(bipolarModulatorValue, -1.0, 1.0);

	// --- calculate range and midpoint
	double halfRange = (maxValue - minValue) / 2.0;
	double midpoint = halfRange + minValue;

	return bipolarModulatorValue * (halfRange)+midpoint;
}

struct vibratoParameters
{
	double depth;
	double rate_Hz;
	bool enableVibrato;

};

class Vibrato : private CircularBuffer<float>
{
public:

	/// <summary>
	/// Sets the delay time in ms and enables/ disables  the comb filter 
	/// </summary>
	/// <param name="pParameters"></param>
	void setParameters(vibratoParameters pParameters)
	{		
		parameters.rate_Hz = pParameters.rate_Hz;
		parameters.depth = pParameters.depth;
		parameters.enableVibrato = pParameters.enableVibrato;

		OscillatorParameters lfoParams = { generatorWaveform::kTriangle, parameters.rate_Hz };
		osc.setParameters(lfoParams);

	}
	/// <summary>
	/// Creates a delay buffer based on the sample rate and modulation parameters.
	/// </summary>
	/// <param name="pSampleRate">The sample rate.</param>
	void createDelayBuffer(double pSampleRate, double pBufferLength_ms)
	{
		currentSampleRate = pSampleRate;
		//samplesPerMsec = currentSampleRate / 1000.0;

		auto bufferLength = (unsigned int)(pBufferLength_ms * samplesPerMsec) + 1;
		delayBuffer.createBuffer(bufferLength);
		delayBuffer.flush();
	}
	/// <summary>
	/// Resets the filter with a new sample rate and initializes the LFO.
	/// </summary>
	/// <param name="pSampleRate">The new sample rate.</param>
	void reset(double pSampleRate)
	{
		currentSampleRate = pSampleRate;
		samplesPerMsec = currentSampleRate / 1000.0;
		osc.reset(currentSampleRate);
	}

	/// <summary>
	/// Processes an audio sample, applying modulation if enabled.
	/// </summary>
	/// <param name="inputXn">The input audio sample to process.</param>
	/// <returns>The processed audio sample.</returns>
	float processAudioSample(float inputXn)
	{
		if (parameters.enableVibrato == true)
		{
			delayBuffer.writeBuffer(inputXn);
			auto oscSample = osc.renderAudioOuput();

			float minDelay_ms = 0.0;
			float maxDelay_ms = 7.0;

			float modValue = (float)oscSample.normalOutput * parameters.depth;
			auto delayVal = doBipolarModulation(modValue, minDelay_ms, minDelay_ms + maxDelay_ms);

			return delayBuffer.readBuffer(delayVal*samplesPerMsec);
		}

		else
		{
			return inputXn;
		}
	}

private:
	double currentSampleRate;
	double samplesPerMsec;
	CircularBuffer delayBuffer;
	vibratoParameters parameters;
	LFO osc; // Low-frequency oscillator for modulation
};

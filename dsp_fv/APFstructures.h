#pragma once

#include "circularBuffer.h"
#include "lfo.h"		
struct delayLineParameters
{
	double delayTime_ms = 0.0;
	bool enableDelay = false;
	double delayTime_samples = 0.0;
};

/// <summary>
/// simple delay Line, Z^(-D) 
/// </summary>
class delayLine : private CircularBuffer<float>
{
public:
	/// <summary>
/// sets the delay time in ms and enables/ disables  the comb filter 
/// </summary>
/// <param name="pParameters"></param>
	void setParameters(delayLineParameters pParameters)
	{
		parameters.delayTime_ms = pParameters.delayTime_ms;
		parameters.enableDelay = pParameters.enableDelay;
		parameters.delayTime_samples = (unsigned int) parameters.delayTime_ms * samplesPerMsec + 1;
	}
	/// <summary>
	/// creates the Delay Line's Delay Buffer (bufferLength = delay time),
	/// also sets the delay time in number of samples according to the sample rate 
	/// </summary>
	/// <param name="pSampleRate"></param>
	void createDelayBuffer(double pSampleRate) 
	{
		currentSampleRate = pSampleRate;
		samplesPerMsec = currentSampleRate / 1000.0;
		auto bufferLength = (unsigned int)(parameters.delayTime_ms * samplesPerMsec) + 1;
		parameters.delayTime_samples = bufferLength;
		delayBuffer.createBuffer(bufferLength);
		delayBuffer.flush();
	}
	/// <summary>
	/// processes the incoming audio sample, output is full wet 
	/// </summary>
	/// <param name="inputXn"></param>
	/// <returns></returns>
	virtual float processAudioSample(float inputXn)
	{
		// full wet signal processing 

		if (parameters.enableDelay == true)
		{
			delayBuffer.writeBuffer(inputXn);
			return  delayBuffer.readBuffer(parameters.delayTime_samples);
		}
		else
		{
			return inputXn;
		}
	}
	/// <summary>
	/// reads the delay Line at a specific sample time. Delay TIme should be given in samples,
	/// in order to avoid superfluous ms to sample conversion 
	/// </summary>
	/// <param name="pDelayTime_samples"></param>
	/// <returns></returns>
	float readDelayLine(unsigned int pDelayTime_samples)
	{
		return delayBuffer.readBuffer((unsigned int)pDelayTime_samples);
	}

private:
	delayLineParameters parameters;
	double currentSampleRate;
	CircularBuffer delayBuffer;
	double samplesPerMsec;
};


struct CombFilterParameters
{
	double delayTime_ms = 0.0;
	double feedbackGain = 0.0;
	bool enableComb = false;
	double delayTime_samples = 0.0;
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
		parameters.delayTime_ms = pParameters.delayTime_ms;
		parameters.feedbackGain = pParameters.feedbackGain;
		parameters.enableComb = pParameters.enableComb;
		parameters.delayTime_samples = (unsigned int)parameters.delayTime_ms * samplesPerMsec + 1;
	}

	/// <summary>
	/// creates the Comb Filter's Delay Buffer (bufferLength = delay time),
	/// also sets the delay time in number of samples according to the sample rate 
	/// </summary>
	/// <param name="pSampleRate"></param>
	void createDelayBuffer(double pSampleRate)
	{
		currentSampleRate = pSampleRate;
		samplesPerMsec = currentSampleRate / 1000.0;
		auto bufferLength = (unsigned int)(parameters.delayTime_ms * samplesPerMsec) + 1;
		parameters.delayTime_samples = (unsigned int) parameters.delayTime_ms * samplesPerMsec + 1;
		delayBuffer.createBuffer(bufferLength);
		delayBuffer.flush();
	}

	/// <summary>
	/// reads the delay Line at a specific sample time. Delay TIme should be given in samples,
	/// in order to avoid superfluous ms to sample conversion 
	/// </summary>
	/// <param name="pDelayTime_samples"></param>
	/// <returns></returns>
	float readDelayLine(unsigned int pDelayTime_samples)
	{
		return delayBuffer.readBuffer((unsigned int) pDelayTime_samples);
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
			auto ynD = delayBuffer.readBuffer(parameters.delayTime_samples, true);
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
	double samplesPerMsec;
	CircularBuffer delayBuffer;
};

struct APFParameters
{
	double delayTime_ms = 0.0;
	double feedbackGain = 0.0;
	bool enableAPF = false;
	double delayTime_samples;
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
		parameters.delayTime_ms = pParameters.delayTime_ms;
		parameters.feedbackGain = pParameters.feedbackGain;
		parameters.enableAPF = pParameters.enableAPF;
		parameters.delayTime_samples = (unsigned int)parameters.delayTime_ms * samplesPerMsec + 1;
	}
	/// <summary>
	/// creates the Comb Filter's Delay Buffer (bufferLength = delay time),
	/// also sets the delay time in number of samples according to the sample rate 
	/// </summary>
	/// <param name="pSampleRate"></param>
	virtual void createDelayBuffer(double pSampleRate)
	{
		currentSampleRate = pSampleRate;
		samplesPerMsec = currentSampleRate / 1000.0;
		auto bufferLength = (unsigned int)(parameters.delayTime_ms * samplesPerMsec) + 1;
		parameters.delayTime_samples = (unsigned int)parameters.delayTime_ms * samplesPerMsec + 1;
		delayBuffer.createBuffer(bufferLength);
		delayBuffer.flush();
	}
	/// <summary>
	/// reads the delay Line at a specific sample time. Delay TIme should be given in samples,
	/// in order to avoid superfluous ms to sample conversion 
	/// </summary>
	/// <param name="pDelayTime_samples"></param>
	/// <returns></returns>
	float readDelayLine(unsigned int pDelayTime_samples)
	{
		return delayBuffer.readBuffer((unsigned int)pDelayTime_samples);
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
			ynD = delayBuffer.readBuffer(parameters.delayTime_samples);
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
	APFParameters parameters; //change parameters by apfParameters 
	double currentSampleRate;
	double samplesPerMsec;
	CircularBuffer delayBuffer;
};

/// <summary>
/// lattice APF structure / alternate APF as described by Jon Dattorro and Will Pirkle 
/// </summary>
class alternateAllPassFilter : public allPassFilter
{
public:
	float processAudioSample(float inputXn) override
	{
		if (parameters.enableAPF == true)
		{
			auto ynD = delayBuffer.readBuffer(parameters.delayTime_samples, true);
			auto temp = inputXn + ynD * parameters.feedbackGain;
			delayBuffer.writeBuffer(temp);
			auto yn = -parameters.feedbackGain * temp + ynD;

			return yn;
		}
		else
		{
			return inputXn;
		}
	}
};

struct APF_modulationParameters
{
	double LFORate_Hz = 0.0;
	double excursion_ms = 0.0;
	bool enableLFO = true;
	double excursion_samples;
};

/// <summary>
/// Class for an alternate all-pass filter with modulation.
/// </summary>
class alternateAllPassFilter_modulated : public allPassFilter
{
public:
	/// <summary>
	/// Resets the filter with a new sample rate and initializes the LFO.
	/// </summary>
	/// <param name="pSampleRate">The new sample rate.</param>
	void reset(double pSampleRate)
	{
		currentSampleRate = pSampleRate;
		lfo.reset(currentSampleRate);
	}

	/// <summary>
	/// Sets parameters for the all-pass filter and its modulation.
	/// </summary>
	/// <param name="pAPFparameters">Parameters for the all-pass filter.</param>
	/// <param name="pApfModParameters">Modulation parameters for the filter.</param>
	void setParameters(APFParameters pAPFparameters, APF_modulationParameters pApfModParameters)
	{
		allPassFilter::setParameters(pAPFparameters);
		setModulationParameters(pApfModParameters);

		OscillatorParameters lfoParams = { generatorWaveform::kSin, apfModParameters.LFORate_Hz };
		lfo.setParameters(lfoParams);
	}

	/// <summary>
	/// Creates a delay buffer based on the sample rate and modulation parameters.
	/// </summary>
	/// <param name="pSampleRate">The sample rate.</param>
	void createDelayBuffer(double pSampleRate) override
	{
		currentSampleRate = pSampleRate;
		samplesPerMsec = currentSampleRate / 1000.0;
		auto bufferLength = (unsigned int)((parameters.delayTime_ms + apfModParameters.excursion_ms) * samplesPerMsec + 1);
		parameters.delayTime_samples = parameters.delayTime_ms * samplesPerMsec;
		apfModParameters.excursion_samples = apfModParameters.excursion_ms * samplesPerMsec;
		delayBuffer.createBuffer(bufferLength);
		delayBuffer.flush();
	}

	/// <summary>
	/// Processes an audio sample, applying modulation if enabled.
	/// </summary>
	/// <param name="inputXn">The input audio sample to process.</param>
	/// <returns>The processed audio sample.</returns>
	float processAudioSample(float inputXn) override
	{
		if (parameters.enableAPF == true)
		{
			double modValue = 0.0;
			if (apfModParameters.enableLFO == true)
			{
				auto outLfo = lfo.renderAudioOuput();
				auto modValue = outLfo.normalOutput * apfModParameters.excursion_samples;
			}

			auto ynD = delayBuffer.readBuffer(parameters.delayTime_samples + modValue, true);
			auto temp = inputXn + parameters.feedbackGain * ynD;
			delayBuffer.writeBuffer(temp);
			auto yn = -parameters.feedbackGain * temp + ynD;
			return yn;
		}
		else
		{
			return inputXn;
		}
	}

protected:
	/// <summary>
	/// Sets the modulation parameters for the filter.
	/// </summary>
	/// <param name="pApfModParameters">Modulation parameters to set.</param>
	void setModulationParameters(APF_modulationParameters pApfModParameters)
	{
		apfModParameters.enableLFO = pApfModParameters.enableLFO;
		apfModParameters.excursion_ms = pApfModParameters.excursion_ms;
		apfModParameters.LFORate_Hz = pApfModParameters.LFORate_Hz;
	}

private:
	APF_modulationParameters apfModParameters; // Modulation parameters
	LFO lfo; // Low-frequency oscillator for modulation
};

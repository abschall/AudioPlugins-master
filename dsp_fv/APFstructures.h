#pragma once

#include "circularBuffer.h"
#include "lfo.h"	
#include "biquad.h"

struct delayLineParameters
{
	double delayTime_ms = 0.0;
	bool enableDelay = false;
	double delayTime_samples = 0.0;
};
/// <summary>
/// parameters of the modulation LFO
/// </summary>
struct DelayLine_modulationParameters
{
	double LFORate_Hz = 0.0;
	double excursion_ms = 0.0;
	bool enableLFO = true;
	double excursion_samples;
};

/// <summary>
/// Simple delay Line, Z^(-D) 
/// </summary>
class delayLine : private CircularBuffer<float>
{
public:
	/// <summary>
/// Sets the delay time in ms and enables/ disables  the comb filter 
/// </summary>
/// <param name="pParameters"></param>
	void setParameters(delayLineParameters pParameters)
	{
		parameters.delayTime_ms = pParameters.delayTime_ms;
		parameters.enableDelay = pParameters.enableDelay;
		parameters.delayTime_samples = (unsigned int) parameters.delayTime_ms * samplesPerMsec;
	}
	/// <summary>
	/// Creates the Delay Line's Delay Buffer (bufferLength = delay time),
	/// also sets the delay time in number of samples according to the sample rate 
	/// </summary>
	/// <param name="pSampleRate"></param>
	void createDelayBuffer(double pSampleRate) 
	{
		currentSampleRate = pSampleRate;
		samplesPerMsec = currentSampleRate / 1000.0;
		auto bufferLength = (unsigned int)(parameters.delayTime_ms * samplesPerMsec) + 1;
		parameters.delayTime_samples = parameters.delayTime_ms * samplesPerMsec;
		delayBuffer.createBuffer(bufferLength);

		// flushes the delayBuffer before any read or write.
		delayBuffer.flush();
	}
	/// <summary>
	/// Processes the incoming audio sample, output is full wet 
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
	/// Writes to delayLine and increments writePointer by 1
	/// </summary>
	/// <param name="x"></param>
	void writeDelayLine(float x)
	{
		delayBuffer.writeBuffer(x);
	}
	/// <summary>
	/// Reads the delay Line at a specific sample time. Delay TIme should be given in samples,
	/// in order to avoid superfluous ms to sample conversion 
	/// </summary>
	/// <param name="pDelayTime_samples"></param>
	/// <returns></returns>
	float readDelayLine(float pDelayTime_samples)
	{
		return delayBuffer.readBuffer(pDelayTime_samples);
	}

private:
	delayLineParameters parameters;
	double currentSampleRate;
	CircularBuffer delayBuffer;
	double samplesPerMsec;
};

/// <summary>
/// UNTESTED !!
/// A sinewave modulated delay Line
/// </summary>
class DelayLine_modulated : private CircularBuffer<float>
{
public:
	/// <summary>
	/// Resets the delay Line with a new sample rate and initializes the LFO
	/// </summary>
	/// <param name="pSampleRate">The new sample rate.</param>
	void reset(double pSampleRate)
	{
		currentSampleRate = pSampleRate;
		lfo.reset(currentSampleRate);
	}

	/// <summary>
	/// Sets parameters for the delay Line and its modulation LFO
	/// </summary>
	/// <param name="pAPFparameters">Parameters for the all-pass filter.</param>
	/// <param name="pApfModParameters">Modulation parameters for the filter.</param>
	void setParameters(delayLineParameters pParameters, DelayLine_modulationParameters pDelayModParameters)
	{
		parameters = pParameters;
		delayModParameters = pDelayModParameters;
		OscillatorParameters lfoParams = { generatorWaveform::kSin, delayModParameters.LFORate_Hz };
		lfo.setParameters(lfoParams);
	}

	/// <summary>
	/// Creates a delay buffer based on the sample rate and modulation parameters.
	/// </summary>
	/// <param name="pSampleRate">The sample rate.</param>
	void createDelayBuffer(double pSampleRate)
	{
		currentSampleRate = pSampleRate;
		samplesPerMsec = currentSampleRate / 1000.0;

		// Buffer length needs to take the excursion time (in samples) into account.
		auto bufferLength = (unsigned int)((parameters.delayTime_ms + delayModParameters.excursion_ms) * samplesPerMsec + 1);
		parameters.delayTime_samples = parameters.delayTime_ms * samplesPerMsec;
		delayModParameters.excursion_samples = delayModParameters.excursion_ms * samplesPerMsec;
		delayBuffer.createBuffer(bufferLength);
		//flush the delay line
		delayBuffer.flush();
	}
	/// <summary>
	/// Processes an audio sample, applying modulation if enabled.
	/// </summary>
	/// <param name="inputXn">The input audio sample to process.</param>
	/// <returns>The processed audio sample.</returns>
	float processAudioSample(float inputXn)
	{
		if (parameters.enableDelay == true)
		{
			double modValue = 0.0;
			if (delayModParameters.enableLFO == true)
			{
				auto outLfo = lfo.renderAudioOuput();
				auto modValue = outLfo.normalOutput * delayModParameters.excursion_samples;
			}

			delayBuffer.writeBuffer(inputXn);
			return  delayBuffer.readBuffer(parameters.delayTime_samples + modValue);
		}
		else
		{
			return inputXn;
		}
	}
private:
	DelayLine_modulationParameters delayModParameters;
	delayLineParameters parameters;
	LFO lfo;
	double currentSampleRate;
	double samplesPerMsec;
	CircularBuffer delayBuffer;
};

struct CombFilterParameters
{
	double delayTime_ms = 0.0;
	double feedbackGain = 0.0;
	bool enableComb = false;
	double delayTime_samples = 0.0;
};

/// <summary>
/// Simple recirculating Comb Filter class, the comb filters have fixed delay times and buffer length
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

struct FCombFilterParameters
{
	double delayTime_ms = 0.0;
	double g = 0.0;
	bool enableComb = false;

	double delayTime_samples = 0.0;
	double g1 = 0.0;
};

class FCombFilter : private CircularBuffer<float>
{
public:
	/// <summary>
	/// sets the delay time in ms, feedback loop gain, and enables/ disables  the comb filter 
	/// </summary>
	/// <param name="pParameters"></param>
	void setParameters(FCombFilterParameters pParameters)
	{
		parameters.delayTime_ms = pParameters.delayTime_ms;
		parameters.g = pParameters.g; //damping parameter
		parameters.g1 = sqrt(parameters.delayTime_ms / 250); // low pass filter coefficient 
		parameters.enableComb = pParameters.enableComb;
		parameters.delayTime_samples = (unsigned int)parameters.delayTime_ms * samplesPerMsec + 1;
		rpole.setType("direct");

		rpole.updateParameters({ 1.0, 0, 0 }, {0, (-1) * (float) parameters.g1, 0}); // LowPass 1-pole filter 
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
		// full wet signal processing 

		if (parameters.enableComb == true)
		{
			auto ynD = delayBuffer.readBuffer(parameters.delayTime_samples, true);
			ynD = rpole.processAudioSample(ynD);
			auto g2 = (1 - parameters.g1) * parameters.g ;// (1 - g1) * g // eventually to be updated quickliy somewhere
			auto ynFullWet = inputXn + g2 * ynD;
			delayBuffer.writeBuffer(ynFullWet);

			return ynD;
		}
		else
			return inputXn;
	}

protected:
	FCombFilterParameters parameters;
	double currentSampleRate;
	double samplesPerMsec;
	CircularBuffer delayBuffer;
	Biquad rpole;
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
	/// Sets the delay time in ms, feedback loop gain, and enables/ disables  the comb filter 
	/// </summary>
	/// <param name="pParameters"></param>
	virtual void setParameters(APFParameters pParameters) 
	{
		parameters.delayTime_ms = pParameters.delayTime_ms;
		parameters.feedbackGain = pParameters.feedbackGain;
		parameters.enableAPF = pParameters.enableAPF;
		parameters.delayTime_samples = (unsigned int)parameters.delayTime_ms * samplesPerMsec + 1;
	}
	/// <summary>
	/// Creates the delay buffer (bufferLength = delay time),
	/// also sets the delay time in number of samples according to the sample rate 
	/// </summary>
	/// <param name="pSampleRate"></param>
	virtual void createDelayBuffer(double pSampleRate)
	{
		currentSampleRate = pSampleRate;
		samplesPerMsec = currentSampleRate / 1000.0;
		auto bufferLength = (unsigned int)(parameters.delayTime_ms * samplesPerMsec) + 1;
		parameters.delayTime_samples = parameters.delayTime_ms * samplesPerMsec;
		delayBuffer.createBuffer(bufferLength);
	}

	/// <summary>
	/// reads the delay Line at a specific sample time. Delay Time should be given in samples,
	/// in order to avoid superfluous ms to sample conversion 
	/// </summary>
	/// <param name="pDelayTime_samples"></param>
	/// <returns></returns>
	float readDelayLine(unsigned int pDelayTime_samples)
	{
		return delayBuffer.readBuffer((unsigned int)pDelayTime_samples);
	}

	/// <summary>
	/// Processes the incoming audio sample, output is full wet 
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
/// Lattice APF structure / alternate APF as described by Jon Dattorro and Will Pirkle 
/// </summary>
class alternateAllPassFilter : public allPassFilter
{
public:
	/// <summary>
	/// Process the incoming Audio sample
	/// </summary>
	/// <param name="inputXn"></param>
	/// <returns> The processed Audio sample</returns>
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

/// <summary>
/// parameters of the modulation LFO
/// </summary>
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
		
		// Buffer length needs to take the excursion time (in samples) into account.
		auto bufferLength = (unsigned int)((parameters.delayTime_ms + apfModParameters.excursion_ms) * samplesPerMsec + 1);
		parameters.delayTime_samples = parameters.delayTime_ms * samplesPerMsec;
		apfModParameters.excursion_samples = apfModParameters.excursion_ms * samplesPerMsec;
		delayBuffer.createBuffer(bufferLength);
		//flush the delay line
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

/// <summary>
/// Shared Parameters structure between alternateAPF_1 and stretchedAPF_2
/// </summary>
struct  alternateAPF_1Parameters
{
	double feedbackGain = 0.0;
	bool enableAPF = false;
};

/// <summary>
/// First-Order IIR Allpass
/// </summary>
class alternateAPF_1
{
public:
	alternateAPF_1() {
		aCoeffVector = { 1,0,0 };
		bCoeffVector = { 1,0,0 };
		form = "direct";
	}

	alternateAPF_1(const string f) :form(f)
	{
		aCoeffVector = { 1,0,0 };
		bCoeffVector = { 1,0,0 };
	}
	/// <summary>
	/// Constructs a Biquad filter with a specified form but with default coefficients.
	/// </summary>
	/// <param name="f">The form of the biquad filter (e.g., "direct", "canonical").</param>
	alternateAPF_1(vector<double> aCoeff, vector<double> bCoeff, string f = "direct") :
		aCoeffVector(aCoeff), bCoeffVector(bCoeff), form(f) { }

	alternateAPF_1(double feedback, string f = "direct") 
	{
		aCoeffVector = { feedback,1,0 };
		bCoeffVector = { 1,feedback,0 };
		form = "direct";
	}
	alternateAPF_1(alternateAPF_1Parameters pParameters) : parameters(pParameters)
	{
		updateCoefficient(parameters.feedbackGain);
		form = "direct";
	}

	/// <summary>
	/// Updates the alternate allpass filter's feedback coefficient
	/// </summary>
	/// <param name="pFeedback"></param>
	virtual void updateCoefficient(double pFeedback)
	{
		aCoeffVector = { pFeedback,1,0 };
		bCoeffVector = { 1,pFeedback,0 };

		parameters.feedbackGain = pFeedback;
	}
	/// <summary>
	/// Returns all the parameters at once 
	/// </summary>
	/// <returns></returns>
	alternateAPF_1Parameters getParameters()
	{
		return parameters;
	}

	/// <summary>
	/// Processes the incoming Audio sample
	/// </summary>
	/// <param name="inputXn"></param>
	/// <returns> The processed Audio sample</returns>
	double processAudioSample(float xn) 
	{
		if (form == "direct")
		{
			float yn = aCoeffVector[0] * xn +
				aCoeffVector[1] * xStateVector[0] +
				aCoeffVector[2] * xStateVector[1] -
				bCoeffVector[1] * yStateVector[0] -
				bCoeffVector[2] * yStateVector[1];

			xStateVector[1] = xStateVector[0];
			xStateVector[0] = xn;

			yStateVector[1] = yStateVector[0];
			yStateVector[0] = yn;

			return yn;
		}
	}

private:
	alternateAPF_1Parameters parameters;
	vector<double> aCoeffVector;
	vector<double> bCoeffVector;
	vector<double> xStateVector{ 0.0f, 0.0f };
	vector<double> yStateVector{ 0.0f, 0.0f };
	vector<double> wStateVector{ 0.0f, 0.0f };
	string form = "direct";
};

class stretchedAPF_2
{
public:
	stretchedAPF_2() {
		aCoeffVector = { 1,0,0 };
		bCoeffVector = { 1,0,0 };
		form = "direct";
	}

	stretchedAPF_2(const string f) :form(f)
	{
		aCoeffVector = { 1,0,0 };
		bCoeffVector = { 1,0,0 };
	}
	/// <summary>
	/// Constructs a Biquad filter with a specified form but with default coefficients.
	/// </summary>
	/// <param name="f">The form of the biquad filter (e.g., "direct", "canonical").</param>
	stretchedAPF_2(vector<double> aCoeff, vector<double> bCoeff, string f = "direct") :

		aCoeffVector(aCoeff), bCoeffVector(bCoeff), form(f) { }

	stretchedAPF_2(double feedback, string f = "direct")
	{
		aCoeffVector = { feedback,0,1 };
		bCoeffVector = { 1,0 ,feedback };
		form = "direct";
	}
	stretchedAPF_2(alternateAPF_1Parameters pParameters) : parameters(pParameters)
	{
		updateCoefficient(parameters.feedbackGain);
		form = "direct";
	}

	/// <summary>
	/// Updates the alternate allpass filter's feedback coefficient
	/// </summary>
	/// <param name="pFeedback"></param>
	void updateCoefficient(double pFeedback)
	{
		aCoeffVector = { pFeedback,0,1 };
		bCoeffVector = { 1,0,pFeedback };

		parameters.feedbackGain = pFeedback;
	}

	/// <summary>
	/// Processes the incoming Audio sample
	/// </summary>
	/// <param name="inputXn"></param>
	/// <returns> The processed Audio sample</returns>
	double processAudioSample(float xn)
	{
		if (form == "direct")
		{
			float yn = aCoeffVector[0] * xn +
				aCoeffVector[1] * xStateVector[0] +
				aCoeffVector[2] * xStateVector[1] -
				bCoeffVector[1] * yStateVector[0] -
				bCoeffVector[2] * yStateVector[1];

			xStateVector[1] = xStateVector[0];
			xStateVector[0] = xn;

			yStateVector[1] = yStateVector[0];
			yStateVector[0] = yn;

			return yn;
		}
	}
private:
	alternateAPF_1Parameters parameters;
	vector<double> aCoeffVector;
	vector<double> bCoeffVector;
	vector<double> xStateVector{ 0.0f, 0.0f };
	vector<double> yStateVector{ 0.0f, 0.0f };
	vector<double> wStateVector{ 0.0f, 0.0f };
	string form = "direct";
};
struct nestedAPFParameters
{
	double delayTime_ms = 0.0;
	double feedbackGain_external = 0.0;
	double feedbackGain_nested = 0.0;
	bool enableAPF = false;
	double delayTime_samples;
};

class nestedAPF : public allPassFilter
{
public:
	/// <summary>
	/// Resets the filter with a new sample and set the internal nested APF's feedback coefficient 
	/// </summary>
	/// <param name="pSampleRate">The new sample rate.</param>
	void reset(double pSampleRate)
	{
		currentSampleRate = pSampleRate;
		samplesPerMsec = currentSampleRate / 1000;
		internalAPF.updateCoefficient(parameters.feedbackGain_nested);
	}

	/// <summary>
	/// Sets parameters for the all-pass filter and its modulation.
	/// </summary>
	/// <param name="pAPFparameters">Parameters for the all-pass filter.</param>
	/// <param name="pApfModParameters">Modulation parameters for the filter.</param>
	void setParameters(nestedAPFParameters pNestedAPFParameters)
	{
		parameters = pNestedAPFParameters;
		parameters.delayTime_samples = (parameters.delayTime_ms * samplesPerMsec);
		APFParameters pAPFparameters = { parameters.delayTime_ms,parameters.feedbackGain_external, parameters.enableAPF, parameters.delayTime_samples };
		allPassFilter::setParameters(pAPFparameters);
		internalAPF.updateCoefficient(parameters.feedbackGain_nested);
	}

	/// <summary>
	/// Processes an audio sample
	/// </summary>
	/// <param name="inputXn">The input audio sample to process.</param>
	/// <returns>The processed audio sample.</returns>
	float processAudioSample(float inputXn) override
	{
		if (parameters.enableAPF == true)
		{
			auto ynD = delayBuffer.readBuffer(parameters.delayTime_samples, true);
			auto wn = -parameters.feedbackGain_external * ynD + inputXn;
			auto wnD = internalAPF.processAudioSample(wn);

			delayBuffer.writeBuffer(wnD);

			return parameters.feedbackGain_external * wn + ynD;
		}
		else
		{
			return inputXn;
		}
	}
private:
	nestedAPFParameters parameters;
	alternateAPF_1 internalAPF;
};
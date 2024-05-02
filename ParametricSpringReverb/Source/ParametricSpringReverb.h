#pragma once
#include "../../dsp_fv/APFstructures.h"
#include "../../dsp_fv/classicFilters.h"


/// <summary>
/// reverb Control parameters, linked to sliders
/// </summary>
struct ReverbControlParameters
{
	double mix;
};

/// <summary>
/// Spring Reverb Model structure internal parameters 
/// </summary>
struct ReverbStructureParameters
{
	const double fC = 4200;			// Transition frequency
	const unsigned int Mlow = 100;		// Number of cascaded APF structures below fC
	const unsigned int Mhigh = 200;	// Number of cascaded APF structures above fC

	struct SpringModelParameters
	{
		double timeDelay_ms = 56;	// Delay Time in ms 
		float Nripple = 0.5;
		double ghf = -0.77;			// high - frequency chirps feedback gain
		double glf = -0.8;			// low - frequency chirps feedback gain
		double gmod_high = 0.0;		// Chf delay line modulation depth
		double gmod_low = 0.0;			// Clf delay line modulation depth

		double gripple = 0.1;		// Ripple filter feedforward coefficient
		double gecho = 0.2;			// Pre - echo delay Line feedforward coefficient

		// Coupling coefficients
		double	gdry = 0.0;
		double	ghigh = 0.005;
		double	glow = 1.0;

		double a1 = 0.62;			// First-order all pass filter coefficent 
	};

	// Derived variables 
	SpringModelParameters springModelParam;
	double defaultSampleRate = 44100; 
	double fN = defaultSampleRate / 2;
	double K = fN / fC;
	int K1 = (int)(round(K) - 1);
	double d = K - K1;
	double a2 = (1 - d) / (1 + d);

	nestedAPFParameters ClfCascadedAPFParam = { K1 / defaultSampleRate * 1000, springModelParam.a1, a2, true };
};

/// <summary>
/// Parametric Spring Reverberation Effect (2010) by V. Välimäki and J. D . Parker 
/// </summary>
class ParametricSpringReverb
{
public:
	/// <summary>
	/// set control parameters
	/// </summary>
	/// <param name="pControlParameters"></param>
	void setParameters(ReverbControlParameters pControlParameters)
	{
		controlParameters = pControlParameters;
	}

	/// <summary>
	/// Updates reverb parameters, by taking the values read from the control (sliders)
	/// </summary>
	/// <param name="pControlParameters"></param>
	void updateParameters(ReverbControlParameters pControlParameters)
	{
		setParameters(pControlParameters);
	}

	/// <summary>
	/// Resets reverb parameters 
	/// </summary>
	/// <param name="pSampleRate"></param>
	void reset(double pSampleRate)
	{
		sampleRate = pSampleRate;

		nestedAPF rNestedAPF;
		structureParameters.fN = sampleRate / 2;
		for (auto i = 0; i < structureParameters.Mlow; ++i)
		{
			rNestedAPF.reset(sampleRate);
			rNestedAPF.setParameters(structureParameters.ClfCascadedAPFParam);
			rNestedAPF.createDelayBuffer(sampleRate);
			Clf_cascadedAPF.push_back(std::move(rNestedAPF));
		}

	}

	/// <summary>
	/// Process the incoming  L and R input signals
	/// </summary>
	/// <param name="inputXn"></param>
	/// <returns> returns the processed audio samples </returns>
	vector<float> processAudioSample(vector<float> inputXn)
	{

		float input = 0.5f * (float)(inputXn[0] + inputXn[1]);
		float output = 0.0f;
		vector<float> outputYn = { 0.0, 0.0 };
		for (auto i = 0; i < structureParameters.Mlow; ++i)
		{
			if (i == 0)
				output = Clf_cascadedAPF[0].processAudioSample(input);
			else
				output = Clf_cascadedAPF[i].processAudioSample(output);
		}
		outputYn[0] = (1 - controlParameters.mix) * inputXn[0] + (controlParameters.mix) * output;
		outputYn[1] = (1 - controlParameters.mix) * inputXn[1] + (controlParameters.mix) * output;
		return outputYn;
	}

private:
	double sampleRate;
	ReverbControlParameters controlParameters;
	ReverbStructureParameters structureParameters;
	vector<nestedAPF> Clf_cascadedAPF; 
};
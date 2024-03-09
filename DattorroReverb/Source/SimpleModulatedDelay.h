#pragma once
#include "../../dsp_fv/APFstructures.h"

struct DelayControlParameters
{
	double mix;
};

struct ModDelayStructureParameters
{
	APFParameters apfParameters = { 30.0, 0.85, true };
	APF_modulationParameters apfModParameters = {1.0, 10.0,true};

};


class SimpleModulatedDelay
{
public:
	/// <summary>
	/// sets the delay's CONTROL parameters (outside controls)
	/// </summary>
	/// <param name="pControlParameters"></param>
	void setParameters(DelayControlParameters pControlParameters)
	{
		controlParameters.mix = pControlParameters.mix;
		// add additionnal control parameters here 
	}

	/// <summary>
	/// sets the inner (sub) components to a predefined state
	/// </summary>
	/// <param name="pSampleRate"></param>
	void reset(double pSampleRate)
	{
		sampleRate = pSampleRate;

		apf_modulated.reset(sampleRate); // resets the modulation LFO 
		apf_modulated.setParameters(strutureParameters.apfParameters, strutureParameters.apfModParameters);
		apf_modulated.createDelayBuffer(sampleRate);

	}
	/// <summary>
	/// processes the incoming audio sample by the modulated Delay algorithm
	/// </summary>
	/// <param name="inputXn"></param>
	/// <returns></returns>
	virtual float processAudioSample(float inputXn)
	{
		float yn = 0.0f;

		yn = apf_modulated.processAudioSample(inputXn);
		auto output = (1 - controlParameters.mix) * inputXn + (controlParameters.mix) * yn;

		return output;
	}
private:
	double sampleRate;
	DelayControlParameters controlParameters;
	ModDelayStructureParameters strutureParameters;
	alternateAllPassFilter_modulated apf_modulated;
};

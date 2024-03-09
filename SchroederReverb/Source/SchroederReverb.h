#pragma once
#include "../../dsp_fv/APFstructures.h"

/// <summary>
/// Outsidecontrol parameters (linked to Plugin parameters)
/// </summary>
struct ReverbControlParameters
{
	double mix;
};

/// <summary>
/// sets the Schroeder Reverb 1962,comb and allpass filters' parameters
/// The reverb is composed of 4 parallel comb filters, which feed into 2 short comb APF
/// creatng an echo density of ~1000echoes/s
/// </summary>
struct SchroederReverbStructureParameters
{
	// Inside parameters,a pre fixed and may not be changed by user outside (therefore build a class here)
	CombFilterParameters comb1 = { 45.0, 0.85, true };
	CombFilterParameters comb2 = { 38.12, 0.87, true };
	CombFilterParameters comb3 = { 33.25, 0.89, true };
	CombFilterParameters comb4 = { 30.0, 0.9, true };
	CombFilterParameters combFilterParameters[4] =  {comb1, comb2, comb3, comb4 };
	unsigned int numberOfCombFilters = sizeof(combFilterParameters)/sizeof(combFilterParameters[0]);

	APFParameters apf1 = { 1.0, 0.63, true};
	APFParameters apf2 = { 5.0, 0.63, true};
	APFParameters apfParameters[2] = {apf1, apf2};
	unsigned int numberOfAPF = sizeof(apfParameters)/ sizeof(apfParameters[0]);


};

/// <summary>
/// Schroeder Reverb (1962 Natural sounding artifical reverbaration) algorithm class
/// composed of 4 parallel comb filters, which feed into 2 short comb APF.
/// 
/// My note: I do not like the sound.
/// </summary>
class SchroederReverb
{
public:
	/// <summary>
	/// sets the reverb CONTROL parameters (outside controls)
	/// </summary>
	/// <param name="pControlParameters"></param>
	void setParameters(ReverbControlParameters pControlParameters)
	{
		controlParameters.mix = pControlParameters.mix;
		// add additionnal control parameters here 
	}

	/// <summary>
	/// sets the inner (sub) components to a predefined state, given by the SchroederReverbStructureParameters struct
	/// </summary>
	/// <param name="pSampleRate"></param>
	virtual void reset(double pSampleRate)
	{
		sampleRate = pSampleRate;
		for (auto numComb = 0; numComb < structureParameters.numberOfCombFilters; ++numComb)
		{
			combFilters[numComb].setParameters(structureParameters.combFilterParameters[numComb]);
			combFilters[numComb].createDelayBuffer(sampleRate);
		}
		for (auto numbAPF = 0; numbAPF < structureParameters.numberOfAPF; ++numbAPF)
		{
			APF[numbAPF].setParameters(structureParameters.apfParameters[numbAPF]);
			APF[numbAPF].createDelayBuffer(sampleRate);
		}
	}
	/// <summary>
	/// processes the incoming audio sample by the reverb algorithm
	/// </summary>
	/// <param name="inputXn"></param>
	/// <returns></returns>
	virtual float processAudioSample(float inputXn)
	{
		float yn = 0.0f;

		for (auto numComb = 0; numComb < structureParameters.numberOfCombFilters; ++numComb)
		{
			yn += combFilters[numComb].processAudioSample(inputXn);
		}

		for (auto numbAPF = 0; numbAPF < structureParameters.numberOfAPF; ++numbAPF)
		{
			yn = APF[numbAPF].processAudioSample(yn);
		}
		auto output = (1 - controlParameters.mix) * inputXn + (controlParameters.mix) * yn;
		return output;


	}

protected:
	double sampleRate;
	ReverbControlParameters controlParameters;
private:
	SchroederReverbStructureParameters structureParameters;
	CombFilter combFilters[4];
	allPassFilter APF[2];

};

/// <summary>
/// sets the Schroeder Reverb 1961, series allpass filters' parameters
/// </summary>
struct SchroederSeriesStructureParameters
{
	// Inside parameters, are fixed and may not be changed by user outside (therefore build a class here)

	// based on the serial APF reverb structure suggested in the 1961 paper 
	APFParameters apf1 = { 100.0, 0.7, true };
	APFParameters apf2 = { 68.0, -0.7, true };
	APFParameters apf3 = { 60.0, 0.7, true };
	APFParameters apf4 = { 19.7, 0.7, true };
	APFParameters apf5 = { 5.85, 0.7, true };

	APFParameters apfParameters[5] = { apf1, apf2, apf3, apf4, apf5 };
	unsigned int numberOfAPF = sizeof(apfParameters)/ sizeof(apfParameters[0]);
};

/// <summary>
/// Schroeder "serial" Reverb (1961 "colorless" artifical reverberation ) algorithm class
/// composed of 5 series APF.
/// 
/// My note: I like the sound of this simple reverb.
/// </summary>
class SchroederReverbSeries : public SchroederReverb
{
public:
	/// <summary>
	///  sets the inner (sub) components to a predefined state, given by the SchroederSeriesStructureParameters struct
	/// </summary>
	/// <param name="pSampleRate"></param>
	void reset(double pSampleRate) override
	{
		sampleRate = pSampleRate;

		for (auto numbAPF = 0; numbAPF < seriesStructureParameters.numberOfAPF; ++numbAPF)
		{
			APF[numbAPF].setParameters(seriesStructureParameters.apfParameters[numbAPF]);
			APF[numbAPF].createDelayBuffer(sampleRate);
		}
	}
	/// <summary>
	/// processes the incoming audio sample by the reverb algorithm
	/// </summary>
	/// <param name="inputXn"></param>
	/// <returns></returns>
	float processAudioSample(float inputXn) override
	{
		auto yn = APF[0].processAudioSample(inputXn);
		for (auto numbAPF = 1; numbAPF < seriesStructureParameters.numberOfAPF; ++numbAPF)
		{
			yn = APF[numbAPF].processAudioSample(yn);
		}

		auto output = (1 - controlParameters.mix) * inputXn + (controlParameters.mix) * yn;
		return output;
	}
protected:

private:
	SchroederSeriesStructureParameters seriesStructureParameters;
	allPassFilter APF[5];
};
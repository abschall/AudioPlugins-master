#pragma once

#include "../../dsp_fv/APFstructures.h"
#include "../../dsp_fv/classicFilters.h"

/// <summary>
/// reverb Control parameters, linked to sliders
/// </summary>
struct ReverbControlParameters
{
	double mix;
	double absorption;
	double earlyReflexions;
	double decay;
	double damping;
	double modRate;
	double modDepth;

	// add additional control parameters here 
};

/// <summary>
/// delaLines, APF, modulated APF and filter parameters 
/// </summary>
struct ReverbStructureParameters
{
	// predelay
	//delayLineParameters predelayParam = { 500.0, true };

	// early Reflexion alternate APFs
	APFParameters earlyReflexAPF1Param = { 5.647, -0.53, true };
	APFParameters earlyReflexAPF2Param = { 6.38, - 0.53, true };
	APFParameters earlyReflexAPF3Param = { 15.67, 0.45, true };
	APFParameters earlyReflexAPF4Param = { 16.75, 0.45, true };

	// early Reflexion low-pass comb filters
	FCombFilterParameters earlyReflexFcomb1Param = { 61.54, 0.35, true };
	FCombFilterParameters earlyReflexFcomb2Param = { 74.69, 0.35, true };

	// simple delay lines
	//delayLineParameters delayLine1Param = { 149.4,true };
	//delayLineParameters delayLine2Param = { 124.8,true };
	//delayLineParameters delayLine3Param = { 141.5,true };
	//delayLineParameters delayLine4Param = { 105.2,true };

	//// simple alternate APF
	//APFParameters alternateAPF5Param = { 60.4, 0.50, true };
	//APFParameters alternateAPF6Param = { 89.12, 0.50, true };

};


class EarlyReflexions
{
public:
	/// <summary>
	/// set internal delayLines and filter parameters
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
		// update bandwidth and damping 
		for (auto i = 0; i < absorptionFilter.size(); ++i)
			absorptionFilter[i].setCoefficients(controlParameters.absorption, 1.0, sampleRate);
	}
	void reset(double pSampleRate)
	{
		sampleRate = pSampleRate;

		// DelayLines and APF structures :  set parameters
		earlyReflexAPF1.setParameters(structureParameters.earlyReflexAPF1Param);
		earlyReflexAPF2.setParameters(structureParameters.earlyReflexAPF2Param);
		earlyReflexAPF3.setParameters(structureParameters.earlyReflexAPF3Param);
		earlyReflexAPF4.setParameters(structureParameters.earlyReflexAPF4Param);
		
		earlyReflexFcomb1.setParameters(structureParameters.earlyReflexFcomb1Param);
		earlyReflexFcomb2.setParameters(structureParameters.earlyReflexFcomb2Param);

		// Set absorption low pass filter type and initial cutoff frequency
		ClassicFilters filter;
		for (auto i = 0; i < 4; ++i)
		{
			filter.setFilterType("LPF1");
			filter.setCoefficients(20000, 1.0, sampleRate);
			absorptionFilter.push_back(filter);
		}

		// Create delay buffers
		earlyReflexAPF1.createDelayBuffer(sampleRate);
		earlyReflexAPF2.createDelayBuffer(sampleRate);
		earlyReflexAPF3.createDelayBuffer(sampleRate);
		earlyReflexAPF4.createDelayBuffer(sampleRate);
		earlyReflexFcomb1.createDelayBuffer(sampleRate);
		earlyReflexFcomb2.createDelayBuffer(sampleRate);
	}

	vector<float> processAudioSample(vector<float> inputXn)
	{

		float input = 0.5f * (float)(inputXn[0] + inputXn[1]);
		static float branch1 = 0.0, branch2 = 0.0, branch3 = 0.0, branch4 = 0.0;
		float decayER = 0.25;
		float lateReflexAmount = 0.15;

		branch1 = absorptionFilter[0].processAudioSample(input + decayER * branch4);
		branch1 = earlyReflexAPF1.processAudioSample(branch1);

		branch2 = absorptionFilter[1].processAudioSample(input + decayER * branch1);
		branch2 = earlyReflexAPF2.processAudioSample(branch2);

		branch3 = absorptionFilter[2].processAudioSample(input + decayER * branch2);
		branch3 = earlyReflexAPF3.processAudioSample(branch3);

		branch4 = absorptionFilter[3].processAudioSample(input + decayER * branch3);
		branch4 = earlyReflexAPF4.processAudioSample(branch4);

		float sumBranches = branch1 + branch2 + branch3 + branch4;
		
		auto lateReflex = lateReflexAmount * (earlyReflexFcomb1.processAudioSample(sumBranches) + earlyReflexFcomb2.processAudioSample(sumBranches));
		
		vector<float> output = { lateReflex + sumBranches,lateReflex + sumBranches };

		return output;
	}
private:
	double sampleRate;
	ReverbControlParameters controlParameters;
	ReverbStructureParameters structureParameters;

	vector<ClassicFilters> absorptionFilter;
	alternateAllPassFilter earlyReflexAPF1, earlyReflexAPF2, earlyReflexAPF3, earlyReflexAPF4;
	FCombFilter earlyReflexFcomb1, earlyReflexFcomb2;
};
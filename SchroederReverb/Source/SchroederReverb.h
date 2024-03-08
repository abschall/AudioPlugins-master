#pragma once

//#include "../../dsp_fv/allPassFilter.h"
//#include "../../dsp_fv/combFilterWithFB.h"
#include "../../dsp_fv/circularBuffer.h"

const unsigned int NUMBER_COMB_FILTERS = 4; // constant: not the most elegant solution here,should be specified inside the reverbStruct class
const unsigned int NUMBER_OUTPUT_APF = 2;

struct APFParameters
{
	double delayTimeInMs = 0.0;
	float feedbackGain = 0.0;
	bool enableAPF = false;
	double delayTimeInSamples;
};

class allPassFilter : private CircularBuffer<float>
{
public:
	void setParameters(APFParameters pParameters)
	{
		parameters.delayTimeInMs = pParameters.delayTimeInMs;
		parameters.feedbackGain = pParameters.feedbackGain;
		parameters.enableAPF = pParameters.enableAPF;
	}
	void createDelayBuffer(double pSampleRate)
	{
		currentSampleRate = pSampleRate;
		auto samplePerMsec = currentSampleRate / 1000.0;
		auto bufferLength = (unsigned int)(parameters.delayTimeInMs * samplePerMsec) + 1;
		parameters.delayTimeInSamples = parameters.delayTimeInMs * samplePerMsec;
		delayBuffer.createBuffer(bufferLength);
	}
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

struct CombFilterParameters
{
	double delayTimeInMs = 0.0;
	float feedbackGain = 0.0;
	bool enableComb= false;
	double delayTimeInSamples;
};

class CombFilter : private CircularBuffer<float>
{
public:
	void setParameters(CombFilterParameters pParameters)
	{
		parameters.delayTimeInMs = pParameters.delayTimeInMs;
		parameters.feedbackGain = pParameters.feedbackGain;
		parameters.enableComb = pParameters.enableComb;
	}
	void createDelayBuffer(double pSampleRate)
	{
		currentSampleRate = pSampleRate;
		auto samplePerMsec = currentSampleRate / 1000.0;
		auto bufferLength = (unsigned int)(parameters.delayTimeInMs * samplePerMsec) + 1;
		parameters.delayTimeInSamples = parameters.delayTimeInMs * samplePerMsec;
		delayBuffer.createBuffer(bufferLength);
	}
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

struct ReverbStructureParameters
{
	// Inside parameters,a re fixed and may not be changed by user outside (therefore build a class here)

	CombFilterParameters comb1 = { 45.0, 0.85, true };
	CombFilterParameters comb2 = { 38.12, 0.87, true };
	CombFilterParameters comb3 = { 33.25, 0.89, true };
	CombFilterParameters comb4 = { 30.0, 0.91, true };
	CombFilterParameters combFilterParameters[NUMBER_COMB_FILTERS] =  {comb1, comb2, comb3, comb4 };

	APFParameters apf1 = { 1.0, 0.63, true};
	APFParameters apf2 = { 5.0, 0.63, true};
	APFParameters apfParameters[NUMBER_OUTPUT_APF] = {apf1, apf2};


};

struct ReverbControlParameters
{
	// Outside control parameters
	double mix; // single outside control parameter for now
};


class SchroederReverb
{
public:
	// sets the reverb CONTROL parameters (outside controls)
	void setParameters(ReverbControlParameters pControlParameters)
	{
		controlParameters.mix = pControlParameters.mix;
		// add additionnal control parameters here 
	}
	// resets all the inner components to a common state
	virtual void reset(double pSampleRate)
	{
		sampleRate = pSampleRate;
		for (auto numComb = 0; numComb < NUMBER_COMB_FILTERS; ++numComb)
		{
			combFilters[numComb].setParameters(structureParameters.combFilterParameters[numComb]);
			combFilters[numComb].createDelayBuffer(sampleRate);
		}
		for (auto numbAPF = 0; numbAPF < NUMBER_OUTPUT_APF; ++numbAPF)
		{
			APF[numbAPF].setParameters(structureParameters.apfParameters[numbAPF]);
			APF[numbAPF].createDelayBuffer(sampleRate);
		}
	}

	virtual float processAudioSample(float inputXn)
	{
		float yn = 0.0f;

		for (auto numComb = 0; numComb < NUMBER_COMB_FILTERS; ++numComb)
		{
			yn += combFilters[numComb].processAudioSample(inputXn) * 0.125;
		}
		for (auto numbAPF = 0; numbAPF < NUMBER_OUTPUT_APF; ++numbAPF)
		{
			yn = APF[numbAPF].processAudioSample(yn);
		}
		
		return ((1 - controlParameters.mix) * inputXn + (controlParameters.mix) * yn);


	}

protected:
	double sampleRate;
	ReverbControlParameters controlParameters;
private:
	ReverbStructureParameters structureParameters;

	CombFilter combFilters[4];
	allPassFilter APF[2];

};

struct SchroederSerialStructureParameters
{
	// Inside parameters,a fixed and may not be changed by user outside (therefore build a class here)

	// based onthe serial APF reverb structure suggested in the 1961 paper 
	APFParameters apf1 = { 100.0, 0.7, true };
	APFParameters apf2 = { 68.0, -0.7, true };
	APFParameters apf3 = { 60.0, 0.7, true };
	APFParameters apf4 = { 19.7, 0.7, true };
	APFParameters apf5 = { 5.85, 0.7, true };

	APFParameters apfParameters[5] = { apf1, apf2, apf3, apf4, apf5 };
};

class SchroederReverbSeries 
{
public:
	// sets the reverb CONTROL parameters (outside controls)
	void setParameters(ReverbControlParameters pControlParameters)
	{
		controlParameters.mix = pControlParameters.mix;
		// add additionnal control parameters here 
	}
	void reset(double pSampleRate)
	{
		sampleRate = pSampleRate;

		for (auto numbAPF = 0; numbAPF < 5; ++numbAPF)
		{
			APF2[numbAPF].setParameters(serialStructureParameters.apfParameters[numbAPF]);
			APF2[numbAPF].createDelayBuffer(sampleRate);
		}
	}

	float processAudioSample(float inputXn)
	{
		auto yn = APF2[0].processAudioSample(inputXn);
		for (auto numbAPF = 1; numbAPF < 5; ++numbAPF)
		{
			yn = APF2[numbAPF].processAudioSample(yn);
		}

		auto output = (1 - controlParameters.mix) * inputXn + (controlParameters.mix) * yn;
		return output;
	}
protected:

	double sampleRate;
	ReverbControlParameters controlParameters;
private:
	SchroederSerialStructureParameters serialStructureParameters;
	allPassFilter APF2[5];
};
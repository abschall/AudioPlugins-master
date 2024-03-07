#pragma once

//#include "../../dsp_fv/allPassFilter.h"
#include "../../dsp_fv/combFilterWithFB.h"
#include "../../dsp_fv/circularBuffer.h"

struct APFParameters
{
	double delayTimeInMs;
	float feedbackGain;
	bool enableAPF;
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
		auto samplePerMsec = currentSampleRate / 1000.0;
		auto bufferLength = (unsigned int)(delayTimeInMs * samplePerMsec) + 1;
		delayBuffer.createBuffer(bufferLength);
	}
	virtual float processAudioSample(float inputXn)
	{
		auto ynD = delayBuffer.readBuffer(delayTimeInMs);
		delayBuffer.writeBuffer(inputXn + ynD * feedbackGain); 
		auto yn = ynD * (1 - feedbackGain * feedbackGain) + inputXn * (-feedbackGain);

		return yn;
	}
protected:
	APFParameters parameters;
	double currentSampleRate;
	CircularBuffer delayBuffer;
};

struct CombFilterParameters
{
	double delayTimeInMs;
	float feedbackGain;
	bool enableComb;
};

class CombFilter : private CircularBuffer<float>
{
public:
	void setParameters(CombFilterParameters pParameters)
	{
		parameters.delayTimeInMs = pParameters.delayTimeInMs;
		parameters.feedbackGain = pParameters.feedbackGain;
		parameters.enableAPF = pParameters.enableAPF;
	}
	void createDelayBuffer(double pSampleRate)
	{
		auto samplePerMsec = currentSampleRate / 1000.0;
		auto bufferLength = (unsigned int)(delayTimeInMs * samplePerMsec) + 1;
		delayBuffer.createBuffer(bufferLength);
	}
	virtual float processAudioSample(float inputXn)
	{
		// full wet signal processing 
		auto ynD = delayBuffer.readBuffer(delayTimeInSamples);
		auto ynFullWet = inputXn + feedbackGain * ynD;
		delayBuffer.writeBuffer(ynFullWet);
		
		return ynD;
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
	vector<CombFilterParameters> combFilterParameters = { comb1, comb2,	comb3, comb4 };

	APFParameters apf1 = { 1.0, 0.5, true };
	APFParameters apf2 = { 5.0, 0.707, true };
	vector<APFParameters> apfParameters = { apf1, apf2 };

	unsigned int numberCombFilters = 4;
	unsigned int numberOutputAPF = 2;
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
	void reset(double pSampleRate)
	{
		sampleRate = pSampleRate;
		for (auto numComb = 0; numComb < structureParameters.numberCombFilters; ++numComb)
		{
			combFilters[numComb].setParameters(structureParameters.combFilterParameters[numComb]);
			combFilters[numComb].createDelayBuffer(sampleRate);
		}
		for (auto numbAPF = 0; numbAPF < structureParameters.numberOutputAPF; ++numbAPF)
		{
			APF[numbAPF].setParameters(structureParameters.apfParameters[numbAPF]);
			APF[numbAPF].createDelayBuffer(sampleRate);
		}
	}

	float processAudioSample(float inputXn)
	{
		float yn = 0.0;

		for (auto numComb = 0; numComb < structureParameters.numberCombFilters; ++numComb)
		{
			yn += combFilters[numComb].processAudioSample(inputXn);
		}
		for (auto numbAPF = 0; numbAPF < structureParameters.numberOutputAPF; ++numbAPF)
		{
			yn = APF[numbAPF].processAudioSample(yn);
		}
		return ((1 - controlParameters.mix) * inputXn + (controlParameters.mix) * yn);


	}
protected:

private:
	ReverbStructureParameters structureParameters;
	ReverbControlParameters controlParameters;
	vector<CombFilter> combFilters;
	vector<allPassFilter> APF;
	double sampleRate;
};
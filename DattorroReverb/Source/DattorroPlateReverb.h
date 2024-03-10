#pragma once
#include "../../dsp_fv/APFstructures.h"
#include "../../dsp_fv/classicFilters.h"

struct ReverbControlParameters
{
	double mix;

	double inputDiffusion1;
	double inputDiffusion2;

	double decayDiffusion1;
	double decayDiffusion2;
	double decay;

	double damping;
	double bandwidth;

	
};

struct ReverbStructureParameters
{
	// input diffusers
	APFParameters inputDiffuser1Param = { 4.93, 0.75, true };
	APFParameters inputDiffuser2Param = { 3.6, 0.75, true };
	APFParameters inputDiffuser3Param = { 12.7, 0.625, true };
	APFParameters inputDiffuser4Param = { 9.3, 0.625, true };
	
	//modulated APF
	APF_modulationParameters modulatedAPF1_lfoParam = { 1, 0.536, true };
	APF_modulationParameters modulatedAPF2_lfoParam = { 1, 0.536, true };
	APFParameters modulatedAPF1Param = { 22.55, 0.7, true };
	APFParameters modulatedAPF2Param = { 30.46, 0.7, true };

	// simple delay lines
	delayLineParameters delayLine1Param = { 149.4,true };
	delayLineParameters delayLine2Param = { 124.8,true };
	delayLineParameters delayLine3Param = { 141.5,true };
	delayLineParameters delayLine4Param = { 105.2,true };

	// simple alternate APF
	APFParameters alternateAPF5Param = { 60.4, 0.50, true };
	APFParameters alternateAPF6Param = { 89.12, 0.50, true };

};

class DattorroPlateReverb
{
public:
	void setParameters(ReverbControlParameters pControlParameters)
	{
		controlParameters.mix = pControlParameters.mix;
		controlParameters.inputDiffusion1 = pControlParameters.inputDiffusion1;
		controlParameters.inputDiffusion2 = pControlParameters.inputDiffusion2;

		controlParameters.decayDiffusion1 = pControlParameters.decayDiffusion1;
		controlParameters.decayDiffusion2 = pControlParameters.decayDiffusion2;
		controlParameters.decay = pControlParameters.decay;

		controlParameters.damping = pControlParameters.damping;
		controlParameters.bandwidth = pControlParameters.bandwidth;

		// add additionnal control parameters here 
	}

	void updateParameters(ReverbControlParameters pControlParameters)
	{
		modulatedAPF1.reset(sampleRate);
		modulatedAPF2.reset(sampleRate);

		setParameters(pControlParameters);
		// update inputDiffusers inputDiffusion 1 and 2
		structureParameters.inputDiffuser1Param.feedbackGain = -controlParameters.inputDiffusion1;
		structureParameters.inputDiffuser2Param.feedbackGain = -controlParameters.inputDiffusion1;
		structureParameters.inputDiffuser3Param.feedbackGain = -controlParameters.inputDiffusion2;
		structureParameters.inputDiffuser4Param.feedbackGain = -controlParameters.inputDiffusion2;
		
		inputDiffuser1.setParameters(structureParameters.inputDiffuser1Param);
		inputDiffuser2.setParameters(structureParameters.inputDiffuser2Param);
		inputDiffuser3.setParameters(structureParameters.inputDiffuser3Param);
		inputDiffuser4.setParameters(structureParameters.inputDiffuser4Param);
		// update decayDiffusion1 and 2
		structureParameters.modulatedAPF1Param.feedbackGain = controlParameters.decayDiffusion1;
		structureParameters.modulatedAPF2Param.feedbackGain = controlParameters.decayDiffusion1;
		structureParameters.alternateAPF5Param.feedbackGain = controlParameters.decayDiffusion2;
		structureParameters.alternateAPF6Param.feedbackGain = controlParameters.decayDiffusion2;
		
		modulatedAPF1.setParameters(structureParameters.modulatedAPF1Param,structureParameters.modulatedAPF1_lfoParam);
		modulatedAPF2.setParameters(structureParameters.modulatedAPF2Param,structureParameters.modulatedAPF2_lfoParam);
		alternateAPF5.setParameters(structureParameters.alternateAPF5Param);
		alternateAPF6.setParameters(structureParameters.alternateAPF6Param);
		// update bandwidth and damping 
		bandwidthLPF.setCoefficients(controlParameters.bandwidth, 1.0, sampleRate);
		dampingLPF1.setCoefficients(controlParameters.damping, 1.0, sampleRate);
		dampingLPF2.setCoefficients(controlParameters.damping, 1.0, sampleRate);
	}

	void reset(double pSampleRate)
	{
		sampleRate = pSampleRate;

		// reset LFOs
		modulatedAPF1.reset(sampleRate); 
		modulatedAPF2.reset(sampleRate);

		// set parameters
		modulatedAPF1.setParameters(structureParameters.modulatedAPF1Param, structureParameters.modulatedAPF1_lfoParam);
		modulatedAPF2.setParameters(structureParameters.modulatedAPF2Param, structureParameters.modulatedAPF2_lfoParam);

		inputDiffuser1.setParameters(structureParameters.inputDiffuser1Param);
		inputDiffuser2.setParameters(structureParameters.inputDiffuser2Param);
		inputDiffuser3.setParameters(structureParameters.inputDiffuser3Param);
		inputDiffuser4.setParameters(structureParameters.inputDiffuser4Param);
		
		delayLine1.setParameters(structureParameters.delayLine1Param);
		delayLine2.setParameters(structureParameters.delayLine2Param);
		delayLine3.setParameters(structureParameters.delayLine3Param);
		delayLine4.setParameters(structureParameters.delayLine4Param);
		alternateAPF5.setParameters(structureParameters.alternateAPF5Param);
		alternateAPF6.setParameters(structureParameters.alternateAPF6Param);

		bandwidthLPF.setFilterType("LPF1");
		dampingLPF1.setFilterType("LPF1");
		dampingLPF2.setFilterType("LPF1");
		bandwidthLPF.	setCoefficients(20000.0, 1.0, sampleRate);
		dampingLPF1.	setCoefficients(0.0, 1.0, sampleRate);
		dampingLPF2.	setCoefficients(0.0, 1.0, sampleRate);


		// create buffers
		modulatedAPF1.createDelayBuffer(sampleRate);
		modulatedAPF2.createDelayBuffer(sampleRate);

		inputDiffuser1.createDelayBuffer(sampleRate);
		inputDiffuser2.createDelayBuffer(sampleRate);
		inputDiffuser3.createDelayBuffer(sampleRate);
		inputDiffuser4.createDelayBuffer(sampleRate);

		delayLine1.createDelayBuffer(sampleRate);
		delayLine2.createDelayBuffer(sampleRate);
		delayLine3.createDelayBuffer(sampleRate);
		delayLine4.createDelayBuffer(sampleRate);

		alternateAPF5.createDelayBuffer(sampleRate);
		alternateAPF6.createDelayBuffer(sampleRate);
	}

	vector<float> processAudioSample(vector<float> inputXn)
	{

		float input = 0.5f * (float)(inputXn[0] + inputXn[1]);
		float output = 0.0f;
		static float tank1_wet = 0.0f, tank2_wet = 0.0f;

		output = bandwidthLPF.	processAudioSample(input);

		output = inputDiffuser1.processAudioSample(output);
		output = inputDiffuser2.processAudioSample(output);
		output = inputDiffuser3.processAudioSample(output);
		output = inputDiffuser4.processAudioSample(output);


		//delay needs to be read !
		//tank1_wet = delayLine2.readDelayLine(4640);
		//tank2_wet = delayLine4.readDelayLine(5503);
		float tank1 = output + tank2_wet;
		float tank2 = output + tank1_wet;

		// process tank 1
		tank1 = modulatedAPF1.	processAudioSample(tank1);
		tank1 = delayLine1.		processAudioSample(tank1);
		tank1 = dampingLPF1.	processAudioSample(tank1);
		tank1 = alternateAPF5.	processAudioSample(tank1);
		tank1_wet = delayLine2.	processAudioSample(tank1) * controlParameters.decay;

		// process tank 2
		tank2 = modulatedAPF2.	processAudioSample(tank2);
		tank2 = delayLine3.		processAudioSample(tank2);
		tank2 = dampingLPF2.	processAudioSample(tank2);
		tank2 = alternateAPF6.	processAudioSample(tank2);
		tank2_wet = delayLine4.	processAudioSample(tank2) * controlParameters.decay;


		vector<float> outputYn = readOutputTaps();

		outputYn[0] = (1 - controlParameters.mix) * inputXn[0] + (controlParameters.mix) * outputYn[0];
		outputYn[1] = (1 - controlParameters.mix) * inputXn[1] + (controlParameters.mix) * outputYn[1];
		return outputYn;
	}

private:
	vector<float> readOutputTaps()
	{
		float yL = delayLine1.readDelayLine(394);
		yL += delayLine1.readDelayLine(4401);
		yL -= alternateAPF5.readDelayLine(2831);
		yL += delayLine2.readDelayLine(2954);
		yL -= delayLine3.readDelayLine(2945);
		yL -= alternateAPF6.readDelayLine(277);
		yL -= delayLine4.readDelayLine(1578);

		float yR = delayLine3.readDelayLine(522);
		yR += delayLine3.readDelayLine(5368);
		yR -= alternateAPF6.readDelayLine(1817);
		yR += delayLine4.readDelayLine(3956);
		yR -= delayLine1.readDelayLine(3124);
		yR -= alternateAPF5.readDelayLine(496);
		yR -= delayLine2.readDelayLine(179);

		vector<float> output = { yL, yR };

		return output;
	}

	double sampleRate;
	ReverbControlParameters controlParameters;
	ReverbStructureParameters structureParameters;

	alternateAllPassFilter inputDiffuser1, inputDiffuser2, inputDiffuser3, inputDiffuser4;
	alternateAllPassFilter_modulated  modulatedAPF1, modulatedAPF2;
	delayLine delayLine1, delayLine2, delayLine3, delayLine4;
	alternateAllPassFilter alternateAPF5, alternateAPF6;
	ClassicFilters bandwidthLPF, dampingLPF1, dampingLPF2;
};
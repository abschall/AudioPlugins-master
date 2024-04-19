#pragma once

#include "../../dsp_fv/APFstructures.h"
#include "../../dsp_fv/classicFilters.h"
#include "../../dsp_fv/vibrato.h"

/// <summary>
/// reverb Control parameters, linked to sliders
/// </summary>
struct ReverbControlParameters
{
	double mix = 0.5;
	double absorption = 20000;
	double earlyReflexions = 1.0;
	double decay = 0.0;
	double damping = 0.005;
	double modRate = 1.0;
	double modDepth = 0.0;

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
	APFParameters earlyReflexAPF2Param = { 6.38, -0.53, true };
	APFParameters earlyReflexAPF3Param = { 15.67, 0.45, true };
	APFParameters earlyReflexAPF4Param = { 16.75, 0.45, true };

	// early Reflexion low-pass comb filters
	FCombFilterParameters earlyReflexFcomb1Param = { 61.54, 0.35, true };
	FCombFilterParameters earlyReflexFcomb2Param = { 74.69, 0.35, true };

	// Reverberator modulated APFs
	vector<APF_modulationParameters> reverbModulatedAPF_lfoParam = {{0.73, 0.78, true },{ 0.71, 0.78, true },{ 1.12, 0.58, true },{ 1.57, 0.25, true }};
	vector<APFParameters> reverbModulatedAPF_Param = {{31.17,0.65,true},{8.87, 0.65, true}, {32.37, 0.75, true}, {9.45,0.75,true}};

	// Reverberator alternate APFs
	vector<APFParameters> reverbAlternateAPFParam = {{78,0.4,true}, {43, 0.4, true}, {64, 0.53, true}, {56,0.53,true}};

	// Reverberator Delay Lines 
	vector<delayLineParameters> reverbDelayLineParam = { {86, true}, {79, true}, {76, true}, {81, true}};
	
	// Reverberator vibratos
	vibratoParameters vibratoParam = { 0.5, 1.0, true }; // excursion_ms , rate then enableVibrato
};


class AbyssalPlateReverb
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

		structureParameters.vibratoParam = {controlParameters.modDepth,controlParameters.modRate,  true };
		reverbVibratoV1.setParameters(structureParameters.vibratoParam);
		reverbVibratoV2.setParameters(structureParameters.vibratoParam);

		//reverbVibratoV1.reset(sampleRate);
		//reverbVibratoV2.reset(sampleRate);

		setParameters(pControlParameters);
		// update absorption and damping 
		for (auto i = 0; i < absorptionFilter.size(); ++i)
		{
			absorptionFilter[i].setCoefficients(controlParameters.absorption, 1.0, sampleRate);
			reverbDampingFilter[i].updateParameters({ 1.0,0.0,0.0 }, { 1,(-1) * (float)controlParameters.damping,0.0 });
		}
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

		earlyReflexAPF1.createDelayBuffer(sampleRate);
		earlyReflexAPF2.createDelayBuffer(sampleRate);
		earlyReflexAPF3.createDelayBuffer(sampleRate);
		earlyReflexAPF4.createDelayBuffer(sampleRate);
		earlyReflexFcomb1.createDelayBuffer(sampleRate);
		earlyReflexFcomb2.createDelayBuffer(sampleRate);

		// Set absorption low pass filter type and initial cutoff frequency
		ClassicFilters filter;
		for (auto i = 0; i < 4; ++i)
		{
			filter.setFilterType("LPF1");
			filter.setCoefficients(20000, 1.0, sampleRate);
			absorptionFilter.push_back(filter);
		}

		alternateAllPassFilter_modulated rModAPF;
		alternateAllPassFilter rAPF;
		delayLine rDelayLine;
		Biquad rDampingFilter;

		for (auto i = 0; i < 4; ++i)
		{			
			rModAPF.setParameters(structureParameters.reverbModulatedAPF_Param[i], structureParameters.reverbModulatedAPF_lfoParam[i]);
			rModAPF.createDelayBuffer(sampleRate);

			rAPF.setParameters(structureParameters.reverbAlternateAPFParam[i]);
			rAPF.createDelayBuffer(sampleRate);

			rDelayLine.setParameters(structureParameters.reverbDelayLineParam[i]);
			rDelayLine.createDelayBuffer(sampleRate);

			rDampingFilter.setType("direct");
			rDampingFilter.updateParameters({ 1.0,0.0,0.0 }, { 1,(-1)*(float)controlParameters.damping,0.0 });

			reverbModAPF.push_back(std::move(rModAPF));
			reverbAPF.push_back(std::move(rAPF));
			reverbDelayLine.push_back(std::move(rDelayLine));
			reverbDampingFilter.push_back(rDampingFilter);
		}

		reverbVibratoV1.setParameters(structureParameters.vibratoParam);
		reverbVibratoV1.reset(sampleRate);
		reverbVibratoV2.reset(sampleRate);

		reverbVibratoV1.createDelayBuffer(sampleRate, 100.0);

		reverbVibratoV2.setParameters(structureParameters.vibratoParam);
		reverbVibratoV2.createDelayBuffer(sampleRate,100.0);
	}

	vector<float> processAudioSample(vector<float> inputXn)
	{
		auto temp = controlParameters.earlyReflexions * ( earlyReflexion_processAudioSample(inputXn));
		temp = controlParameters.earlyReflexions * temp + (1 - controlParameters.earlyReflexions) * 0.5 * (inputXn[0] + inputXn[1]);
		auto out= reverberator_processAudioSample(temp);
		return out;
	}

private:
	/// <summary>
	/// stereo IN to mono OUT
	/// </summary>
	/// <param name="inputXn"></param>
	/// <returns>mono output sample from the early reflexion </returns>
	float earlyReflexion_processAudioSample(vector<float> inputXn)
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

		float output = lateReflex + sumBranches;

		return output;
	}
	
	/// <summary>
	/// mono IN to stereo OUT
	/// </summary>
	/// <param name="inputXn"></param>
	/// <returns> the outputs for each channel </returns>
	vector<float> reverberator_processAudioSample(float inputXn)
	{
		static vector<float> branches = { 0.0, 0.0, 0.0, 0.0 };

		for (auto i = 0; i < branches.size(); ++i)
		{
			if (i == 0)
			{
				branches[i] = reverbModAPF[i].processAudioSample(inputXn + controlParameters.decay * branches[3]);
				branches[i] = reverbVibratoV1.processAudioSample(branches[i]);
			}
			else
			{
				branches[i] = reverbModAPF[i].processAudioSample(inputXn + controlParameters.decay * branches[i - 1]);

			}

			if (i == 1)
			{
				branches[i] = reverbVibratoV2.processAudioSample(branches[i]);
			}
			branches[i] = reverbAPF[i].processAudioSample(branches[i]);
			branches[i] = reverbDampingFilter[i].processAudioSample((1 - controlParameters.damping) * branches[i]);
			branches[i] = reverbDelayLine[i].processAudioSample(branches[i]);
		}

		vector<float> outputYn = readOutputTaps();

		outputYn[0] = (1 - controlParameters.mix) * inputXn + (controlParameters.mix) * outputYn[0];
		outputYn[1] = (1 - controlParameters.mix) * inputXn + (controlParameters.mix) * outputYn[1];
		return outputYn;

	}

	vector<float> readOutputTaps()
	{
		float yL = reverbDelayLine[0].readDelayLine(825);
		yL += reverbDelayLine[1].readDelayLine(2112);
		yL -= reverbDelayLine[2].readDelayLine(1630);
		yL -= reverbDelayLine[3].readDelayLine(3215);

		float yR = reverbDelayLine[2].readDelayLine(825);
		yR += reverbDelayLine[3].readDelayLine(2069);
		yR -= reverbDelayLine[1].readDelayLine(1679);
		yR -= reverbDelayLine[2].readDelayLine(2641);

		return {0.16f * yL, 0.16f * yR };
	}

	double sampleRate;
	ReverbControlParameters controlParameters;
	ReverbStructureParameters structureParameters;

	// Early Reflexions blocks
	vector<ClassicFilters> absorptionFilter;
	alternateAllPassFilter earlyReflexAPF1, earlyReflexAPF2, earlyReflexAPF3, earlyReflexAPF4;
	FCombFilter earlyReflexFcomb1, earlyReflexFcomb2;

	// Reverberator blocks
	vector<alternateAllPassFilter_modulated> reverbModAPF;
	vector<alternateAllPassFilter> reverbAPF;	
	vector<delayLine> reverbDelayLine;
	vector<Biquad> reverbDampingFilter;
	Vibrato reverbVibratoV1;
	Vibrato reverbVibratoV2;
};
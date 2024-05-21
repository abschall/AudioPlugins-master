#pragma once

/*
* Spring reverb with Downsampled Clf block 
*/
#include "../../dsp_fv/APFstructures.h"
#include "../../dsp_fv/classicFilters.h"
#include "IIR_10.h"
/// <summary>
/// reverb Control parameters, linked to sliders
/// </summary>
struct ReverbControlParameters
{
	double mix;
	double IR_level;
};

/// <summary>
/// Spring Reverb Model structure internal parameters 
/// </summary>
struct ReverbStructureParameters
{
	const double fC = 4300;			// Transition frequency
	const unsigned int Mlow = 100;		// Number of cascaded APF structures below fC
	const unsigned int Mhigh = 50;	// Number of cascaded APF structures above fC

	struct SpringModelParameters
	{
		double timeDelay_ms = 66;	// Delay Time in ms 
		float Nripple = 0.5;
		double ghf = -0.77;			// high - frequency chirps feedback gain
		double glf = -0.8;			// low - frequency chirps feedback gain
		double gmod_high = 6;		// Chf delay line modulation depth
		double gmod_low = 12;		// Clf delay line modulation depth

		double gripple = 0.05;		// Ripple filter feedforward coefficient
		double gecho = - 0.1;			// Pre - echo delay Line feedforward coefficient

		// Coupling coefficients
		double	gdry = 0.0;
		double	ghigh = 0.01;
		double	glow = 1.0;

		double a1 = 0.62;			// Low frequency nested First-order all pass filter coefficent 
	};

	// Derived variables 
	SpringModelParameters springModelParam;
	double defaultSampleRate = 44100;
	double defaultSamplesPerMs = 44.1/2 ;
	double fN = defaultSampleRate / 4;
	double K = fN / fC;
	int K1 = (int)(round(K) - 1);
	double d = K - K1;
	double a2 = (1 - d) / (1 + d);

	// Low frequency chirps delay struycture variables
	double groupDelay = K * Mlow * (1 - springModelParam.a1) / (1 + springModelParam.a1);
	int timeDelay_samples = springModelParam.timeDelay_ms * defaultSamplesPerMs;

	// Delay Line length are converted from samples to ms in order to fit the delayLine initialization (which utilizes ms)  
	double L = (timeDelay_samples - groupDelay) / defaultSamplesPerMs; // Total Multitap delay Length in ms
	double Lecho = L / 5; // Pre-echo delay line time in ms 
	double Lripple = (2 * K * springModelParam.Nripple) / defaultSamplesPerMs; // Ripple filter delay mine length in ms
	double L0 = L - Lecho - Lripple; // Clf Modulated delayLine length in ms 

	// High frequency chirps delay struycture length
	double Lhigh = L * 2 / 2.3;
	double ahigh = -0.6; // High-Frequency chirps 1st-order APF feedback coefficient

	// Clf DC high pass filter (40 Hz)
	double fDC_cutoff = 40; // Clf DC HPF cutoff frequency 
	double aDC = tan(3.1415 / 4 - fDC_cutoff / defaultSampleRate * 3.1415); // Clf DC HPF coefficient 
	double DC_scalingFactor = (1 + aDC) / 2; // Clf DC HPF scaling factor 

	// white noise Leaky Integrator
	double fint = 200;
	//float aint = tan(3.1415 / 4 - fint / defaultSampleRate * 3.1415); // Leaky Integrator coefficient 
	double aint = 0.93;
	// Cross-coupling coefficients
	double C1 = 0.1, C2 = 0.0;


	// Clf blocks, times in ms
	nestedAPFParameters ClfCascadedAPFParam = { K1 / defaultSamplesPerMs, springModelParam.a1, a2, true };
	delayLineParameters ClfDelayLineParam = { L0 * 1.3 , true };
	delayLineParameters preechoDelayLineParam = { Lecho, true };
	delayLineParameters rippleFilterDelayLineParam = { Lripple, true };

	//Chf block
	alternateAPF_1Parameters ChfCascadedAPFParam = { ahigh, true };
	delayLineParameters ChfDelayLineParam = { Lhigh * 1.3 , true }; // times in ms
};

/// <summary>
/// Low-Frequency Feedback Delay Structure
/// </summary>
class Clf_structure
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

	void reset(double pSampleRaten, double pDownSampleRate)
	{
		sampleRate = pSampleRaten;
		downSampleRate = pDownSampleRate;
		nestedAPF rNestedAPF;

		// Mlow-order stretched APF initialization
		for (auto i = 0; i < structureParameters.Mlow; ++i)
		{
			rNestedAPF.reset(downSampleRate);
			rNestedAPF.setParameters(structureParameters.ClfCascadedAPFParam);
			rNestedAPF.createDelayBuffer(downSampleRate);
			Clf_cascadedAPF.push_back(std::move(rNestedAPF));
		}

		// DC Filter initialization 
		DCFilter.updateParameters({ 1,-1, 0 },
			{ 1, -(float)structureParameters.aDC, 0 });
		DCFilter.setDryWetGain(0, 1.0);

		// MultiTap delay Line initialization 
		ClfDelayLine.setParameters(structureParameters.ClfDelayLineParam);
		ClfDelayLine.createDelayBuffer(downSampleRate);
		rippleFilterDelayLine.setParameters(structureParameters.rippleFilterDelayLineParam);
		rippleFilterDelayLine.createDelayBuffer(downSampleRate);
		preechoDelayLine.setParameters(structureParameters.preechoDelayLineParam);
		preechoDelayLine.createDelayBuffer(downSampleRate);

		// Leaky-Integrator which filters the modulation noise
		leakyIntegrator.updateParameters({ (1 - structureParameters.aint),0,0 }, { 1, -structureParameters.aint ,0 });
	}

	/// <summary>
	/// Processes the input sample, by the low-frequency feedback delay structure
	/// </summary>
	/// <param name="input"></param>
	/// <returns> Processed audio sample </returns>
	float processAudioSample(float input)
	{

		float output = 0.0f;
		static float ynD = 0.0f;

		auto temp = input - ynD;
		//temp = structureParameters.DC_scalingFactor *  DCFilter.processAudioSample(temp);
		temp = cascadedAPF_procesAudio(temp);
		output = temp;
		ynD = structureParameters.springModelParam.glf * multitapDelay_processAudio(temp);

		return output;
	}

	/// <summary>
	/// Processes the input sample, by the low-frequency feedback delay structure
	/// </summary>
	/// <param name="input"></param>
	/// <returns> Processed audio sample </returns>
	float processAudioSample_bis(float input)
	{
		float output = 0.0f;
		static float ynD = 0.0f;

		ynD = rippleFilterDelayLine.readDelayLine(structureParameters.Lripple * 44.1);
		auto temp = input - structureParameters.springModelParam.glf * ynD;
		//temp = structureParameters.DC_scalingFactor * DCFilter.processAudioSample(temp);
		temp = cascadedAPF_procesAudio(temp);
		output = temp;

		ynD = multitapDelay_processAudio(output);

		return output;
	}
private:
	/// <summary>
	/// Processes the input sample, noted x by Mlow cascaded nested APF structures 
	/// </summary>
	/// <param name="x"></param>
	/// <returns> Processed audio sample</returns>
	float cascadedAPF_procesAudio(float x)
	{
		float output = 0.0f;
		for (auto i = 0; i < structureParameters.Mlow; ++i)
		{
			if (i == 0)
				output = Clf_cascadedAPF[0].processAudioSample(x);
			else
				output = Clf_cascadedAPF[i].processAudioSample(output);

		}
		// Filter the signal above fC, keep only low frequency chirps
		return output;
	}

	float multitapDelay_processAudio(float x)
	{
		juce::Random rnd;
		auto noiseMod = leakyIntegrator.processAudioSample(rnd.nextFloat()) * structureParameters.springModelParam.gmod_low;
		ClfDelayLine.writeDelayLine(x);
		auto temp = ClfDelayLine.readDelayLine(structureParameters.L0 * structureParameters.defaultSamplesPerMs + noiseMod);

		bool combStyle = true;
		if (combStyle)
		{
			// Process using Preecho filter, comb filter style
			auto tempwet = preechoDelayLine.readDelayLine(structureParameters.Lecho);
			auto fullwet = temp + structureParameters.springModelParam.gecho * tempwet;
			preechoDelayLine.writeDelayLine(fullwet);
			temp = fullwet;

			// Process by Ripple Filter, comb filter style
			tempwet = rippleFilterDelayLine.readDelayLine(structureParameters.Lecho);
			fullwet = temp + structureParameters.springModelParam.gecho * tempwet;
			rippleFilterDelayLine.writeDelayLine(fullwet);
			temp = fullwet;

		}
		else
		{
			temp = preechoDelayLine.processAudioSample(temp) + structureParameters.springModelParam.gecho * temp;
			temp = rippleFilterDelayLine.processAudioSample(temp) + structureParameters.springModelParam.gripple * temp;
		}

		return temp;
	}

	double sampleRate;
	double downSampleRate;
	ReverbControlParameters controlParameters;
	ReverbStructureParameters structureParameters;
	vector<nestedAPF> Clf_cascadedAPF;
	Biquad DCFilter{ "canonical" };
	Biquad leakyIntegrator{ "canonical" };
	IIRfilter ellipticFilter;
	delayLine preechoDelayLine, rippleFilterDelayLine, ClfDelayLine;
	IIRFilterCoeff newCoeff;
};

/// <summary>
/// High-Frequency Feedback Delay Structure 
/// </summary>
class Chf_structure
{
public:
	void reset(double pSampleRate)
	{
		sampleRate = pSampleRate;
		stretchedAPF_2 rAPF_1(structureParameters.ChfCascadedAPFParam);
		structureParameters.fN = sampleRate / 2;

		// Mhigh-order stretched APF initialization
		for (auto i = 0; i < structureParameters.Mhigh; ++i)
		{
			Chf_cascadedAPF.push_back(rAPF_1);
		}

		// High-Frequency modulated delay Line initialization 
		ChfDelayLine.setParameters(structureParameters.ChfDelayLineParam);
		ChfDelayLine.createDelayBuffer(sampleRate);

		// Leaky-Integrator 
		leakyIntegrator.updateParameters({ (1 - structureParameters.aint),0,0 }, { 1, -structureParameters.aint ,0 });

	}

	/// <summary>
	/// Processes the input sample by the High-Frequency FB delay structure
	/// </summary>
	/// <param name="input"></param>
	/// <returns> Processed audio sample</returns>
	float processAudioSample(float input)
	{
		float output = 0.0f;
		static float ynD = 0.0f;
		juce::Random rnd;
		auto noiseMod = leakyIntegrator.processAudioSample(rnd.nextFloat()) * structureParameters.springModelParam.gmod_high;

		ynD = ChfDelayLine.readDelayLine(structureParameters.Lhigh * structureParameters.defaultSamplesPerMs + noiseMod);
		output = input - structureParameters.springModelParam.ghf * ynD;
		output = cascadedAPF_procesAudio(output);
		ChfDelayLine.writeDelayLine(output);

		return output;
	}

private:
	/// <summary>
	/// Processes the input sample, noted x by Mhigh cascaded nested APF structures 
	/// </summary>
	/// <param name="x"></param>
	/// <returns> Processed audio sample</returns>
	float cascadedAPF_procesAudio(float x)
	{
		float output = 0.0f;
		for (auto i = 0; i < structureParameters.Mhigh; ++i)
		{
			if (i == 0)
				output = Chf_cascadedAPF[0].processAudioSample(x);
			else
				output = Chf_cascadedAPF[i].processAudioSample(output);

		}
		return output;
	}

	double sampleRate;
	ReverbStructureParameters structureParameters;
	vector<stretchedAPF_2> Chf_cascadedAPF;
	delayLine  ChfDelayLine;
	Biquad leakyIntegrator{ "canonical" };
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
		//! Eventually remove setParameters function if not necessary in the Clf structure
		clf_structure.setParameters(controlParameters);
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
	/// Resets the sample rate and the Clf and Chf internal blocks 
	/// </summary>
	/// <param name="pSampleRate"></param>
	void reset(double pSampleRate)
	{
		sampleRate = pSampleRate;
		downsampleRate = sampleRate / decimationFactor;
		clf_structure.reset(sampleRate, downsampleRate);
		chf_structure.reset(sampleRate);
	}

	vector<double> processAudioSample(vector<float> inputXn)
	{
		auto input = 0.5f * (inputXn[0] + inputXn[1]);
		static auto clf_out = 0.0f, chf_out = 0.0f;

		// decimate input audio for the Clf block 
		if (decimationCounter % decimationFactor == 0) {
			decimationCounter = 1; // Reset the counter
			clf_out = clf_structure.processAudioSample(input + structureParameters.C1 * chf_out); // process input audio by Clf structure 
		}
		else {
			++decimationCounter;
			clf_out = 0.0; // Return zero for samples not taken
		}
		clf_out = ellipticFilter.processAudio(clf_out);

		chf_out = chf_structure.processAudioSample(input + structureParameters.C2 * clf_out);

		double mixedSignal = structureParameters.springModelParam.ghigh * chf_out +
			structureParameters.springModelParam.glow * clf_out +
			structureParameters.springModelParam.gdry * input;
		return { controlParameters.mix * mixedSignal + (1 - controlParameters.mix) * input,controlParameters.mix * mixedSignal + (1 - controlParameters.mix) * input };
	}


private:
	double sampleRate;
	double downsampleRate;
	int decimationFactor = 2;
	int decimationCounter = 1;
	IIRfilter ellipticFilter;
	ReverbControlParameters controlParameters;
	ReverbStructureParameters structureParameters;
	Clf_structure clf_structure;
	Chf_structure chf_structure;
};

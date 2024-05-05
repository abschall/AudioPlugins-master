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
	const double fC = 4300;			// Transition frequency
	const unsigned int Mlow = 100;		// Number of cascaded APF structures below fC
	const unsigned int Mhigh = 200;	// Number of cascaded APF structures above fC

	struct SpringModelParameters
	{
		double timeDelay_ms = 56;	// Delay Time in ms 
		float Nripple = 0.5;
		double ghf = -0.77;			// high - frequency chirps feedback gain
		double glf = -0.8;			// low - frequency chirps feedback gain
		double gmod_high = 0.0;		// Chf delay line modulation depth
		double gmod_low = 8;			// Clf delay line modulation depth

		double gripple = 0.1;		// Ripple filter feedforward coefficient
		double gecho = 0.1;			// Pre - echo delay Line feedforward coefficient

		// Coupling coefficients
		double	gdry = 0.0;
		double	ghigh = 0.005;
		double	glow = 1.0;

		double a1 = 0.62;			// First-order all pass filter coefficent 
	};

	// Derived variables 
	SpringModelParameters springModelParam;
	double defaultSampleRate = 44100; 
	double defaultSamplesPerMs = 44.1;
	double fN = defaultSampleRate / 2;
	double K = fN / fC;
	int K1 = (int)(round(K) - 1);
	double d = K - K1;
	double a2 = (1 - d) / (1 + d);

	// Low frequency chirps delay struycture variables
	double groupDelay = K * Mlow * (1 - springModelParam.a1) / (1 + springModelParam.a1);
	int timeDelay_samples = springModelParam.timeDelay_ms * defaultSamplesPerMs;
	
	// It is of importance to simplify for clearer more comprhensible code 
	// Delay Line length are converted from samples to ms in order to fit the delayLine initialization (which utilizes ms)  
	double L = (timeDelay_samples - groupDelay) / defaultSamplesPerMs;
	double Lecho = L / 5;
	double Lripple = (2 * K * springModelParam.Nripple) / defaultSamplesPerMs;
	double L0 = L - Lecho - Lripple;
	
	// High frequency chirps delay struycture length
	int Lhigh = L / 2.3;
	 
	// Clf DC high pass filter (40 Hz)
	double fDC_cutoff = 40;
	double aDC = tan(3.1415 / 4 - fDC_cutoff / defaultSampleRate * 3.1415);
	double DC_scalingFactor = (1 + aDC) / 2;

	// white noise Leaky Integrator
	double fint = 2000;
	double aint = tan(3.1415 / 4 - fint / defaultSampleRate * 3.1415);

	// Cross-coupling coefficients
	double C1 = 0.0, C2 = 0.0;


	// Clf blocks
	nestedAPFParameters ClfCascadedAPFParam = { K1 / defaultSampleRate * 1000, springModelParam.a1, a2, true };
	delayLineParameters ClfDelayLineParam = { 2 * L0 , true }; // times in ms
	delayLineParameters preechoDelayLineParam = { Lecho, true }; // times in ms
	delayLineParameters rippleFilterDelayLineParam = { Lripple, true }; // times in ms

};


/// <summary>
/// 10th-order low pass Elliptic Filter
/// N : 10
/// type : low pass
/// fc : 4 kHz
/// rp : 1 dB
/// rs : attenuation in the stop band 60 dB
/// </summary>
struct ellipticLPF
{
	//calculated using Python's scipy signal library
	// 2nd order cascaded filter structures coefficients
	vector<vector<float>> acoeff = { { 2.08707020e-03, 4.32795775e-04, 2.08707020e-03 },
		{1.00000000e+00, -1.35295140e+00, 1.00000000e+00},
		{1.00000000e+00, -1.58866341e+00, 1.00000000e+00},
		{1.00000000e+00, -1.64689418e+00, 1.00000000e+00},
		{1.00000000e+00, -1.66280686e+00, 1.00000000e+00}};

	vector<vector<float>> bcoeff = { {1.00000000e+00, -1.71423801e+00, 7.56257719e-01},
		{1.00000000e+00, -1.69763544e+00, 8.58196421e-01},
		{1.00000000e+00, -1.68441929e+00, 9.41018508e-01},
		{1.00000000e+00, -1.67922030e+00, 9.79415607e-01},
		{1.00000000e+00, -1.67993383e+00, 9.95127711e-01} };
};


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

	void reset(double pSampleRate)
	{
		sampleRate = pSampleRate;
		nestedAPF rNestedAPF;
		structureParameters.fN = sampleRate / 2;

		// Mlow-order stretched APF initialization
		for (auto i = 0; i < structureParameters.Mlow; ++i)
		{
			rNestedAPF.reset(sampleRate);
			rNestedAPF.setParameters(structureParameters.ClfCascadedAPFParam);
			rNestedAPF.createDelayBuffer(sampleRate);
			Clf_cascadedAPF.push_back(std::move(rNestedAPF));
		}

		// DC Filter initialization 
		DCFilter.updateParameters({ 1,-1, 0 },
			{ 1, -(float)structureParameters.aDC, 0 });
		DCFilter.setDryWetGain(0, 1.0);

		Biquad ellipticFilterBlock{ "direct" };
		for (auto i = 0; i < 5; ++i)
		{
			
			ellipticFilterBlock.updateParameters(ellipticFilterCoeff.acoeff[i], ellipticFilterCoeff.bcoeff[i]);
			ellipticFilterBlock.setDryWetGain(0, 1.0);
			ellipticFilter.push_back(ellipticFilterBlock);
		}

		// MultiTap delay Line initialization 
		ClfDelayLine.setParameters(structureParameters.ClfDelayLineParam);
		
		ClfDelayLine.createDelayBuffer(sampleRate);
		rippleFilterDelayLine.setParameters(structureParameters.rippleFilterDelayLineParam);
		rippleFilterDelayLine.createDelayBuffer(sampleRate);
		preechoDelayLine.setParameters(structureParameters.preechoDelayLineParam);
		preechoDelayLine.createDelayBuffer(sampleRate);

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
		static float ynD = 0.0f;

		auto temp = input - ynD;
		temp = structureParameters.DC_scalingFactor *  DCFilter.processAudioSample(temp);
		temp = cascadedAPF_procesAudio(temp);
		output = temp;
		ynD = structureParameters.springModelParam.glf *  multitapDelay_processAudio(temp);
		
		
		outputYn[0] = (1 - controlParameters.mix) * inputXn[0] + (controlParameters.mix) * output;
		outputYn[1] = (1 - controlParameters.mix) * inputXn[1] + (controlParameters.mix) * output;
		return outputYn;
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
		for (auto i = 0; i < 5; ++i)
		{
			output = ellipticFilter[i].processAudioSample(output);
		}
		return output;
	}

	float multitapDelay_processAudio(float x)
	{
		auto temp = ClfDelayLine.processAudioSample(x);
		juce::Random rnd;
		auto noiseMod = 0.33 * rnd.nextFloat() * structureParameters.springModelParam.gmod_low;
		temp = ClfDelayLine.readDelayLine(structureParameters.L0 * structureParameters.defaultSamplesPerMs + noiseMod );
		temp = preechoDelayLine.processAudioSample(temp) + structureParameters.springModelParam.gecho*temp;
		temp = rippleFilterDelayLine.processAudioSample(temp) + structureParameters.springModelParam.gripple * temp ;
		return temp;

	}

	double sampleRate;
	ReverbControlParameters controlParameters;
	ReverbStructureParameters structureParameters;
	vector<nestedAPF> Clf_cascadedAPF;
	Biquad DCFilter{ "direct" };

	ellipticLPF ellipticFilterCoeff;
	vector<Biquad> ellipticFilter;
	delayLine preechoDelayLine, rippleFilterDelayLine, ClfDelayLine;

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
	/// Resets reverb parameters 
	/// </summary>
	/// <param name="pSampleRate"></param>
	void reset(double pSampleRate)
	{
		sampleRate = pSampleRate;
		clf_structure.reset(sampleRate);
	}

	vector<float> processAudioSample(vector<float> inputXn)
	{
		return clf_structure.processAudioSample(inputXn);
	}

private:
	double sampleRate;
	ReverbControlParameters controlParameters;
	ReverbStructureParameters structureParameters;
	Clf_structure clf_structure;
};

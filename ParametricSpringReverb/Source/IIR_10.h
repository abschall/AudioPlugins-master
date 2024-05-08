#pragma once
/*
* Infinite Impulse Response filter class
* 
* Filters an input signal, sample by sample
* As for now this is only a test version, which in unoptimized 
* Calculation of a and b coefficients is performed externally using the "scipy.signal" library 
*/

/// <summary>
/// Naive IIR filter, up to the 10th order 
/// </summary>
class IIR_10
{
public:
	IIR_10() {
		aCoeffVector = { 0,0,0,0,0,0,0,0,0,0,0 };
		bCoeffVector = { 0,0,0,0,0,0,0,0,0,0,0 };
		form = "canonical";

	}

	IIR_10(const juce::String f) :form(f)
	{
		aCoeffVector = { 0,0,0,0,0,0,0,0,0,0,0 };
		bCoeffVector = { 0,0,0,0,0,0,0,0,0,0,0 };
	}

	/// <summary>
	/// Constructs a IIR filter with a specified form but with default coefficients.
	/// </summary>
	/// <param name="f">The form of the biquad filter (e.g., "direct", "canonical").</param>
	IIR_10(vector<double> aCoeff, vector<double> bCoeff, int numCoeff, juce::String f = "direct") :
		aCoeffVector(aCoeff), bCoeffVector(bCoeff), form(f) { }

	/// <summary>
	/// Updates the filter coefficients.
	/// </summary>
	/// <param name="aCoeff">New set of feedback coefficients.</param>
	/// <param name="bCoeff">New set of feedforward coefficients.</param>
	void updateParameters(vector<double> aCoeff, vector<double> bCoeff, int pOrder)
	{
		order = pOrder;
		// updates the biquad class a and b  parameters 
		for (int i = 0; i < order; ++i)
		{
			aCoeffVector[i] = aCoeff[i];
			bCoeffVector[i] = bCoeff[i];
		}
	}

	/// <summary>
	/// Resets all coefficients and internal state vectors to zero.
	/// </summary>    
	virtual void resetCoeff()
	{
		// flushes all the vectors values, resets to 0
		for (int i = 0; i < aCoeffVector.size(); ++i)
		{
			aCoeffVector[i] = 0.0f;
			bCoeffVector[i] = 0.0f;
		}

		for (int i = 0; i < aCoeffVector.size(); ++i)
		{
			xStateVector[i] = 0.0f;
			yStateVector[i] = 0.0f;
		}
	}

	/// <summary>
	/// Sets the dry/wet mix gain values.
	/// </summary>
	/// <param name="dry">Gain for the dry (unprocessed) signal.</param>
	/// <param name="processed">Gain for the wet (processed) signal.</param>
	void setDryWetGain(double dry, double processed)
	{
		dryCoeff = dry;
		processedCoeff = processed;
	}

	/// <summary>
	/// Processes an audio sample through the IIR filter and returns the filtered sample.
	/// </summary>
	/// <param name="xn">The input audio sample.</param>
	/// <returns>The filtered audio sample.</returns>
	virtual double processAudioSample(double xn)
	{
		double yn = 0;
		if (form == juce::String("direct"))
		{
			for (auto i = 0; i < order; ++i)
			{

				if (i == 0)
				{
					yn = aCoeffVector[0] * xn;
				}
				else
				{
					yn += aCoeffVector[i] * xStateVector[i - 1];
					yn -= bCoeffVector[i] * yStateVector[i - 1];
				}

			}
			for (auto i = order - 1; i >= 0; --i)
			{
				if (i == 0)
				{
					xStateVector[0] = xn;
					yStateVector[0] = yn;
				}
				else
				{
					xStateVector[i] = xStateVector[i - 1];
					yStateVector[i] = yStateVector[i - 1];
				}
			}
			return yn;
		}

		else if (form == juce::String("None"))
		{
			double yn = xn;
			return yn;
		}
	}

	/// <summary>
	/// Sets the form/type of the IIR filter.
	/// </summary>
	/// <param name="type">The form/type of the biquad filter  topology (e.g., "direct", "canonical").</param>
	void setType(juce::String type) {
		form = type;
	}

private:
	vector<double> aCoeffVector;
	vector<double> bCoeffVector;
	vector<double> xStateVector{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
	vector<double> yStateVector{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
	vector<double> wStateVector{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
	double dryCoeff = 0.0;
	double processedCoeff = 1.0;
	juce::String form;
	int order = 11;
};

/// <summary>
/// 10th-order low pass Elliptic Filter
/// N : 10
/// type : low pass
/// fc : 4 kHz and 5 kHz
/// rp : 1 dB
/// rs : attenuation in the stop band 60 dB
/// </summary>
/// 
struct IIRFilterCoeff
{
	//filter order
	unsigned int N = 10;

	//calculated using Python's scipy signal library
	// 2nd order cascaded filter structures coefficients
	//4KHz
	//vector<vector<double>> acoeff = { { 2.08707020e-03, 4.32795775e-04, 2.08707020e-03 },
	//	{1.00000000e+00, -1.35295140e+00, 1.00000000e+00},
	//	{1.00000000e+00, -1.58866341e+00, 1.00000000e+00},
	//	{1.00000000e+00, -1.64689418e+00, 1.00000000e+00},
	//	{1.00000000e+00, -1.66280686e+00, 1.00000000e+00}};

	//vector<vector<double>> bcoeff = { {1.00000000e+00, -1.71423801e+00, 7.56257719e-01},
	//	{1.00000000e+00, -1.69763544e+00, 8.58196421e-01},
	//	{1.00000000e+00, -1.68441929e+00, 9.41018508e-01},
	//	{1.00000000e+00, -1.67922030e+00, 9.79415607e-01},
	//	{1.00000000e+00, -1.67993383e+00, 9.95127711e-01} };
	//5 kHz
	vector<vector<double>> acoeff = { {  0.00289504, 0.00191265, 0.00289504 },
		{1., -1.05033069, 1.},
		{1., -1.37575949, 1.},
		{1., -1.45959129, 1.},
		{1., -1.48274757, 1.} };

	vector<vector<double>> bcoeff = { {1., -1.63691194,		0.7021348},
		{1., -1.57927007,		0.8274404},
		{1., -1.53319663,		0.92846835},
		{1., -1.51330346,		0.97507128},
		{1., -1.50934335,		0.99410041} };
};

/// <summary>
/// IIR filter based on 2nd order biquad filter structures, expects an even numbered filter order 
/// </summary>
class IIRfilter
{
public:
	IIRfilter() {
		initializeFilter();
	}

	/// <summary>
	/// Updates the filter's cascaded biquad filter structures 
	/// </summary>
	/// <param name="pacoeff"></param>
	/// <param name="pbcoeff"></param>
	void updateCoeff(vector<vector<double>> pacoeff, vector<vector<double>> pbcoeff, unsigned int pN, string form ="canonical")
	{
		coeff.N = pN;
		coeff.acoeff = pacoeff;
		coeff.bcoeff = pbcoeff;
		Biquad ellipticFilterBlock{ form };
		for (auto i = 0; i < int(coeff.N / 2); ++i)
		{
			ellipticFilterBlock.updateParameters(coeff.acoeff[i], coeff.bcoeff[i]);
			ellipticFilterBlock.setDryWetGain(0, 1.0);
			// Update the current filter's biquad structures
			filter[i] = ellipticFilterBlock;
		}
	}

	/// <summary>
	/// Processes the input sample by a N-th order IIR filter 
	/// </summary>
	/// <param name="x"></param>
	/// <returns> The processed audio sample</returns>
	double processAudio(double x)
	{
		double output = x;
		for (auto i = 0; i < int(coeff.N / 2); ++i)
		{
			output = filter[i].processAudioSample(output);
		}
		return output;
	}
private:
	/// <summary>
	/// sets 
	/// </summary>
	void initializeFilter()
	{
		Biquad ellipticFilterBlock{ "canonical" };
		for (auto i = 0; i < int(coeff.N / 2); ++i)
		{

			ellipticFilterBlock.updateParameters(coeff.acoeff[i], coeff.bcoeff[i]);
			ellipticFilterBlock.setDryWetGain(0, 1.0);
			filter.push_back(ellipticFilterBlock);
		}
	}
	// Member variables 
	IIRFilterCoeff coeff; 
	vector<Biquad> filter;
};
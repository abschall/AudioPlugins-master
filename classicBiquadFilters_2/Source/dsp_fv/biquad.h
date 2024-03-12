#pragma once 
#include <JuceHeader.h>
#include <vector>
using std::vector;

// =============================================================================
// Biquad  Class
// used to implement every kinf of second order filter, sample by sample reading
// =============================================================================


class Biquad
{
public:
    
    Biquad() {
        aCoeffVector = { 0,0,0 };
        bCoeffVector = { 0,0,0 };
        form = "canonical";

    }
    Biquad(const juce::String f):form(f)
    {
        aCoeffVector = { 0,0,0 };
        bCoeffVector = { 0,0,0 };
    }
    Biquad(vector<float> aCoeff, vector<float> bCoeff, int numCoeff, juce::String f = "direct") :
        aCoeffVector(aCoeff), bCoeffVector(bCoeff), numCoefficients(numCoeff), form(f) { }
    ~Biquad() {}

    void updateParameters(vector<float> aCoeff, vector<float> bCoeff)
    {
        // updates the biquad class a and b  parameters 
        for (int i = 0; i < 3; ++i)
        {
            aCoeffVector[i] = aCoeff[i];
            bCoeffVector[i] = bCoeff[i];
        }
    }
    
    virtual void resetCoeff()
    {
        // flushes all the vectors values, resets to 0
        for (int i = 0; i <aCoeffVector.size(); ++i)
        {
            aCoeffVector[i] = 0.0f;
            bCoeffVector[i] = 0.0f;
        }
        numCoefficients = aCoeffVector.size() + bCoeffVector.size();

        for (int i = 0; i < aCoeffVector.size(); ++i)
        {
            xStateVector[i] = 0.0f;
            yStateVector[i] = 0.0f;
        }
    }

    void setDryWetGain(float dry, float processed) 
    {
        dryCoeff = dry;
        processedCoeff = processed;
    }

    virtual float processAudioSample(float xn)
    {
        if (form == juce::String("direct"))
        {
            float yn = processedCoeff * (aCoeffVector[0] * xn +
                aCoeffVector[1] * xStateVector[0] +
                aCoeffVector[2] * xStateVector[1] -
                bCoeffVector[1] * yStateVector[0] -
                bCoeffVector[2] * yStateVector[1]) + 
                dryCoeff * xn;


            xStateVector[1] = xStateVector[0];
            xStateVector[0] = xn;

            yStateVector[1] = yStateVector[0];
            yStateVector[0] = yn;

            return yn;
        }
        else if (form == juce::String("canonical"))
        {
            auto wn = (float) ( xn - aCoeffVector[0] * wStateVector[0] - 
                                bCoeffVector[1] * wStateVector[1]);
            auto ynUnprocessed = (float) ( aCoeffVector[0] * wn     + 
                                aCoeffVector[1] * wStateVector[0]   + 
                                aCoeffVector[2] * wStateVector[1]);

            
            wStateVector[1] = wStateVector[0];
            wStateVector[0] = wn;
            return (float) (processedCoeff * ynUnprocessed +dryCoeff * xn); //yn

        }
        else if (form == juce::String("None"))
        {
            float yn = xn;
            return yn;
        }
    }

    void setType(juce::String type)
    {
        form = type;
    }

private:
    // =============================================================================
    // Private members 

    int numCoefficients;
    vector<float> aCoeffVector;
    vector<float> bCoeffVector;

    vector<float>  xStateVector{ 0.0f, 0.0f };
    vector<float>  yStateVector{ 0.0f, 0.0f };
    vector<float>  wStateVector{ 0.0f, 0.0f };
    float dryCoeff = 0.0;
    float processedCoeff = 1.0;
    juce::String form;

};

// =============================================================================
// ClassicFilters : public Biquad Class
// used to implement every kind of second order filter, sample by sample reading
// =============================================================================

/// <summary>
/// ClassicFilters class, used to implement the following filters :
/// LPF1, LPF2, HPF1, HPF2, more filters to come ...
/// </summary>
class ClassicFilters : private Biquad
{
public:

    ClassicFilters()
    {
        biquadStruct.setType("direct");

    }
    /// <summary>
    /// resets the biquadStruct members coefficients 
    /// </summary>
    void resetCoeff() override
    {
        biquadStruct.resetCoeff();
    }
    /// <summary>
    /// sets the coefficients of the second order Biquad biquadSruct member, 
    /// using the desired corner Frequency and Quality factor.
    /// a and b coefficients of the biquadStruct member depend upon the sample Rate as well.
    /// </summary>
    /// <param name="cornerFreq"></param>
    /// <param name="qualityFactor"></param>
    /// <param name="sampleFreq"></param>
    void setCoefficients(float cornerFreq, float qualityFactor,float sampleFreq) 
    {
       
        vector<float> aCoeff;
        vector<float> bCoeff;

        if (filterType == juce::String("LPF1"))
        {
            auto theta_c = juce::MathConstants<float>::twoPi * cornerFreq / sampleFreq;
            auto gamma = std::cos(theta_c) / (1 + std::sin(theta_c));
            aCoeff.push_back((1 - gamma) / 2);
            aCoeff.push_back((1 - gamma) / 2);
            aCoeff.push_back(0.0f);

            bCoeff.push_back(0);
            bCoeff.push_back(-gamma);
            bCoeff.push_back(0);

        }
        else if (filterType == juce::String("HPF1"))
        {
            auto theta_c = juce::MathConstants<float>::twoPi * cornerFreq / sampleFreq;
            auto gamma = std::cos(theta_c) / (1 + std::sin(theta_c));

            aCoeff.push_back((1 + gamma) / 2);
            aCoeff.push_back(-(1 + gamma) / 2);
            aCoeff.push_back(0.0f);

            bCoeff.push_back(0);
            bCoeff.push_back(-gamma);
            bCoeff.push_back(0);
        }
        else if (filterType == juce::String("LPF2"))
        {
            auto theta_c = juce::MathConstants<float>::twoPi * cornerFreq / sampleFreq;
            auto d = 1 / qualityFactor;
            auto beta = 0.5 * (1 - d / 2 * std::sin(theta_c)) / (1 + d / 2 * std::sin(theta_c));
            auto gamma = std::cos(theta_c) * (0.5 + beta);

            aCoeff.push_back((0.5 + beta - gamma) / 2);
            aCoeff.push_back((0.5 + beta - gamma));
            aCoeff.push_back((0.5 + beta - gamma) / 2);

            bCoeff.push_back(1);
            bCoeff.push_back(-2 * gamma);
            bCoeff.push_back(2 * beta);

        }
        else if (filterType == juce::String("HPF2"))
        {
            auto theta_c = juce::MathConstants<float>::twoPi * cornerFreq / sampleFreq;
            auto d = 1 / qualityFactor;
            auto beta = 0.5 * (1 - d / 2 * std::sin(theta_c)) / (1 + d / 2 * std::sin(theta_c));
            auto gamma = std::cos(theta_c) * (0.5 + beta);

            aCoeff.push_back((0.5 + beta + gamma) / 2.0);
            aCoeff.push_back(-(0.5 + beta + gamma));
            aCoeff.push_back((0.5 + beta + gamma) / 2.0);

            bCoeff.push_back(1);
            bCoeff.push_back(-2 * gamma);
            bCoeff.push_back(2 * beta);
        }
        else if (filterType == juce::String("None"))
        {
            biquadStruct.setType(juce::String("None"));
            aCoeff.push_back(1);
            aCoeff.push_back(0);
            aCoeff.push_back(0);

            bCoeff.push_back(1);
            bCoeff.push_back(0);
            bCoeff.push_back(0);

        }
        biquadStruct.updateParameters(aCoeff, bCoeff);
    }

    /// <summary>
    /// sets te Dry and Wet parameters of the underlying biquadStruct member,
    /// Wet (processed) = (float) Gain, Dry  = 1 - (float) Gain
    /// </summary>
    /// <param name="gain"></param>
    void setFilterGain(float gain)
    {
        biquadStruct.setDryWetGain(1.0f - gain, gain);
    }
    /// <summary>
    /// sets the filter type. Supported are :
    /// LPF1, LPF2, HPF1, HPF 2. more to come...
    /// </summary>
    /// <param name="type"></param>
    void setFilterType(juce::String type) 
    {
        filterType = type;
    }
    /// <summary>
    /// function used to process the incoming sample x(n),
    ///  override the biquadStruct::processAudioSample() method
    /// </summary>
    /// <param name="xn"></param>
    /// <returns></returns>
    float processAudioSample(float xn) override
    {
        auto yn = (float) biquadStruct.processAudioSample(xn);
        return yn;
    }

private:
    juce::String filterType;
    Biquad biquadStruct;
};


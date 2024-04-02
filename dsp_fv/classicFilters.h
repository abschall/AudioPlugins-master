#pragma once
#include <JuceHeader.h>
#include <vector>
#include "biquad.h"

using std::vector;

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
    void setCoefficients(float cornerFreq, float qualityFactor, float sampleFreq, float gain = 1.0f /*full Wet*/)
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
        setFilterGain(gain);
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
    /// Sets the filter type. Supported are :
    /// LPF1, LPF2, HPF1, HPF 2. more to come...
    /// </summary>
    /// <param name="type"></param>
    void setFilterType(juce::String type)
    {
        filterType = type;
    }

    /// <summary>
    /// Processes the incoming sample x(n),
    ///  override the biquadStruct::processAudioSample() method
    /// </summary>
    /// <param name="xn"></param>
    /// <returns> returns the output sample</returns>
    float processAudioSample(float xn) override
    {
        auto yn = (float)biquadStruct.processAudioSample(xn);
        return yn;
    }

private:
    juce::String filterType;
    Biquad biquadStruct;
};

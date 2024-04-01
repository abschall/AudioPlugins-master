#pragma once 
#include <JuceHeader.h>
#include <vector>
using std::vector;

// =============================================================================
// Biquad  Class
// used to implement every kinf of second order filter, sample by sample reading
// =============================================================================

/// <summary>
/// Represents a biquad filter, which is a fundamental element in digital signal processing for audio.
/// This class allows for the creation and manipulation of various filter types through the adjustment
/// of coefficients and provides methods for processing audio samples.
/// </summary>
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
    /// <summary>
    /// Constructs a Biquad filter with a specified form but with default coefficients.
    /// </summary>
    /// <param name="f">The form of the biquad filter (e.g., "direct", "canonical").</param>
    Biquad(vector<float> aCoeff, vector<float> bCoeff, int numCoeff, juce::String f = "direct") :
        aCoeffVector(aCoeff), bCoeffVector(bCoeff), form(f) { }

    /// <summary>
    /// Updates the filter coefficients.
    /// </summary>
    /// <param name="aCoeff">New set of feedback coefficients.</param>
    /// <param name="bCoeff">New set of feedforward coefficients.</param>
    void updateParameters(vector<float> aCoeff, vector<float> bCoeff)
    {
        // updates the biquad class a and b  parameters 
        for (int i = 0; i < 3; ++i)
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
        for (int i = 0; i <aCoeffVector.size(); ++i)
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
    void setDryWetGain(float dry, float processed) 
    {
        dryCoeff = dry;
        processedCoeff = processed;
    }

    /// <summary>
    /// Processes an audio sample through the biquad filter and returns the filtered sample.
    /// </summary>
    /// <param name="xn">The input audio sample.</param>
    /// <returns>The filtered audio sample.</returns>
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

   /// <summary>
   /// Sets the form/type of the biquad filter.
   /// </summary>
   /// <param name="type">The form/type of the filter (e.g., "direct", "canonical").</param>
    void setType(juce::String type) {
        form = type;
    }

private:
    vector<float> aCoeffVector;
    vector<float> bCoeffVector;
    vector<float> xStateVector{ 0.0f, 0.0f };
    vector<float> yStateVector{ 0.0f, 0.0f };
    vector<float> wStateVector{ 0.0f, 0.0f };
    float dryCoeff = 0.0;
    float processedCoeff = 1.0;
    juce::String form;
};
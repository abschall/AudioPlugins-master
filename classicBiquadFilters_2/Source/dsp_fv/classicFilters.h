#pragma once
#include <JuceHeader.h>
#include <vector>
#include "biquad.h"

/// <summary>
/// A class representing classic filters derived from the Biquad filter.
/// </summary>
class ClassicFilters : private Biquad
{
public:
	Biquad() {}

    /// <summary>
    /// Sets the parameters of the classic filter.
    /// </summary>
    /// <param name="fc">The corner frequency of the filter.</param>
    /// <param name="Q">The quality factor of the filter.</param>
    /// <param name="filterType">The type of filter.</param>
    void setParameters(float fc, float Q, juce::String filterType) override
    {

        Biquad::setParameters();
    }

private:
    float cornerFreq, qualityFactor, sampleFreq;
    float gainControl;
    
};
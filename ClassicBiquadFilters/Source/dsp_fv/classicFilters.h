#pragma once
#include <JuceHeader.h>
#include <vector>
#include "biquad.h"

using std::vector;

class ClassicFilters : private Biquad
{
public:
	Biquad() {}
	void setParameters(float fc, float Q, juce::String filterType) override
	{

		Biquad::setParameters()
	}
private:
	float cornerFreq, qualityFactor, sampleFreq;
	float gainControl;
	
};
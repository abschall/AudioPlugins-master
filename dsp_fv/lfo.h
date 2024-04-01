#pragma once
#include <iostream>
using namespace std;

const double M_PI = 3.1415; ///< Value of pi.
const double B = 4.0 / M_PI; ///< Coefficient B for parabolic sine computation.
const double C = -4.0 / (M_PI * M_PI); ///< Coefficient C for parabolic sine computation.
const double P = 0.225; ///< Parabolic sine constant.

/// <summary>
/// Converts a unipolar value to a bipolar value.
/// </summary>
/// <param name="xUni">The input unipolar value.</param>
/// <returns>The converted bipolar value.</returns>
inline double unipolarToBipolar(double xUni) {
    return (2.0 * xUni - 1.0);
}

/// <summary>
/// Converts a bipolar value to a unipolar value.
/// </summary>
/// <param name="xBi">The input bipolar value.</param>
/// <returns>The converted unipolar value.</returns>
inline double bipolarToUnipolar(double xBi) {
    return ((xBi + 1.0) / 2);
}

/// <summary>
/// Computes the parabolic sine function.
/// </summary>
/// <param name="x">The input value.</param>
/// <returns>The computed parabolic sine value.</returns>
inline double parabolicSine(double x) {
    // designed to take an input from -pi to pi
    auto y = B * x + C * x * fabs(x);
    y = P * (y * fabs(y) - y) + y;
    return y;
}

/// Enumerates waveform types for the oscillator.

enum class generatorWaveform { kTriangle, kSin, kSaw };

/// Represents parameters for an oscillator.

struct OscillatorParameters {
    generatorWaveform waveform; ///< Waveform type.
    double oscillatorFrequency_Hz; ///< Oscillator frequency in Hz.
};


/// <summary>
/// Represents a phasor for generating oscillations.
/// </summary>
class Phasor
{
public:
    /// <summary>
    /// Resets the phasor to a specified sample rate and optional reset value.
    /// </summary>
    /// <param name="pSampleRate">The sample rate.</param>
    /// <param name="phasorResetValue">The optional reset value (default is 0.0).</param>
    /// <returns>True if the reset is successful, false otherwise.</returns>
    bool reset(double pSampleRate, double phasorResetValue = 0.0)
    {
        currentSampleRate = pSampleRate;
        phasorInc = phasorFrequency / pSampleRate;

        if ((phasorResetValue < 0.0) || (phasorResetValue > 1.0))
            phasorOutput = 0.0;
        else
            phasorOutput = phasorResetValue;

        return true;
    }

    /// <summary>
    /// Sets the frequency of the phasor.
    /// </summary>
    /// <param name="pFrequency">The frequency value to set.</param>
    void setFrequency(double pFrequency)
    {
        phasorFrequency = pFrequency;
    }

    /// <summary>
    /// Gets the frequency of the phasor.
    /// </summary>
    /// <returns>The frequency of the phasor.</returns>
    double getFrequency()
    {
        return phasorFrequency;
    }

    /// <summary>
    /// Advances the phasor and wraps around if necessary.
    /// </summary>
    /// <returns>True if the phasor wrapped around, false otherwise.</returns>
    virtual bool advanceAndWrap()
    {
        phasorOutput += phasorInc;
        if ((phasorInc > 0) && (phasorOutput >= 1.0))
        {
            phasorOutput -= 1.0;
            return true;
        }

        if (phasorInc < 0 && phasorOutput <= 0.0)
        {
            phasorOutput += 1.0;
            return true;
        }
        return false;
    }

    /// <summary>
    /// Gets the current output value of the phasor.
    /// </summary>
    /// <returns>The current output value of the phasor.</returns>
    double getPhasorOutput()
    {
        return phasorOutput;
    }

private:
    double currentSampleRate; ///< Current sample rate.
    double phasorFrequency; ///< Frequency of the phasor.
    double phasorInc; ///< Increment value for advancing the phasor.
    double phasorOutput; ///< Current output value of the phasor.
};

/// <summary>
/// different LFO signal outputs
/// </summary>
struct LFOSignalOutput
{
    double normalOutput;
    double invertedOutput;
    double quarterPhaseOutput; ///< Quarter phase
    double invertedQuarterPhaseOutput; ///< Quarter phase inverted
};

/// <summary>
/// Represents a low-frequency oscillator (LFO) using a phasor.
/// </summary>
class LFO : private Phasor
{
public:
    /// <summary>
    /// Gets the parameters of the oscillator.
    /// </summary>
    /// <returns>The parameters of the oscillator.</returns>
    OscillatorParameters getParameters()
    {
        return oscillatorParameters;
    }

    /// <summary>
    /// Sets the parameters of the oscillator.
    /// </summary>
    /// <param name="pOscillatorParameters">The parameters to set.</param>
    void setParameters(OscillatorParameters pOscillatorParameters)
    {
        oscillatorParameters.waveform = pOscillatorParameters.waveform;
        oscillatorParameters.oscillatorFrequency_Hz = pOscillatorParameters.oscillatorFrequency_Hz;

        phasorNormal.setFrequency(oscillatorParameters.oscillatorFrequency_Hz);
        phasorQuarterPhase.setFrequency(oscillatorParameters.oscillatorFrequency_Hz);
    }

    /// <summary>
    /// Resets the LFO with the given sample rate.
    /// </summary>
    /// <param name="pSampleRate">The sample rate.</param>
    /// <returns>True if the reset is successful, false otherwise.</returns>
    bool reset(double pSampleRate)
    {
        currentSampleRate = pSampleRate;
        phasorNormal.reset(currentSampleRate, 0.0);
        phasorQuarterPhase.reset(currentSampleRate, 0.25);

        return true;
    }

    /// <summary>
    /// Advances the LFO and wraps around if necessary.
    /// </summary>
    /// <returns>True if the LFO wrapped around, false otherwise.</returns>
    bool advanceAndWrap() override
    {
        auto state = phasorNormal.advanceAndWrap();
        phasorQuarterPhase.advanceAndWrap();
        return state;
    }

    /// <summary>
    /// Renders the audio output of the LFO.
    /// </summary>
    /// <returns>The audio output of the LFO.</returns>
    LFOSignalOutput renderAudioOuput()
    {
        // Retrieve phasor outputs
        auto xNorm = phasorNormal.getPhasorOutput();
        auto xQP = phasorQuarterPhase.getPhasorOutput();

        // Calculate LFO waveform based on oscillator waveform type
        if (oscillatorParameters.waveform == generatorWaveform::kSaw)
        {
            // Convert phasor outputs to bipolar and assign to normal and quarter-phase outputs
            lfoSignalOutput.normalOutput = unipolarToBipolar(xNorm);
            lfoSignalOutput.quarterPhaseOutput = unipolarToBipolar(xQP);
        }
        else if (oscillatorParameters.waveform == generatorWaveform::kTriangle)
        {
            // Apply triangular waveform transformation to phasor outputs and assign to normal and quarter-phase outputs
            lfoSignalOutput.normalOutput = (2.0 * fabs(unipolarToBipolar(xNorm)) - 1.0);
            lfoSignalOutput.quarterPhaseOutput = 2.0 * fabs(unipolarToBipolar(xQP)) - 1.0;
        }
        else if (oscillatorParameters.waveform == generatorWaveform::kSin)
        {
            // Adjust phasor outputs to range [-pi, pi], apply parabolic sine function, and assign to normal and quarter-phase outputs
            xNorm = 2.0 * M_PI * xNorm - M_PI;
            xQP = 2.0 * M_PI * xQP - M_PI;
            lfoSignalOutput.normalOutput = parabolicSine(-xNorm);
            lfoSignalOutput.quarterPhaseOutput = parabolicSine(-xQP);
        }

        // Calculate inverted outputs
        lfoSignalOutput.invertedOutput = lfoSignalOutput.normalOutput * -1.0;
        lfoSignalOutput.invertedQuarterPhaseOutput = lfoSignalOutput.quarterPhaseOutput * -1.0;

        // Advance phasors to the next sample and return the resulting LFO signal output
        advanceAndWrap();
        return lfoSignalOutput;
    }

private:
    double currentSampleRate;
    Phasor phasorNormal;
    Phasor phasorQuarterPhase;

    // Oscillator parameters
    OscillatorParameters oscillatorParameters;

    LFOSignalOutput lfoSignalOutput;

};

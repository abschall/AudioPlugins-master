#pragma once
#include <iostream>
using namespace std;

const double M_PI = 3.1415;
const double B = 4.0 / M_PI;
const double C = -4.0 / (M_PI * M_PI);
const double P = 0.225;

inline double unipolarToBipolar(double xUni)
{
    return (2.0 * xUni - 1.0);
}

inline double bipolarToUnipolar(double xBi)
{
    return((xBi + 1.0) / 2);

}
inline double parabolicSine(double x)
{
    // designed to take an input from -pi to pi
    auto y = B * x + C * x * fabs(x);
    y = P * (y * fabs(y) - y) + y;
    return y;
}
enum class generatorWaveform { kTriangle, kSin, kSaw };

struct OscillatorParameters
{
    generatorWaveform waveform;
    double oscillatorFrequency_Hz;
};

class Phasor
{

public:
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
    void setFrequency(double pFrequency)
    {
        phasorFrequency = pFrequency;
    }

    double getFrequency()
    {
        return phasorFrequency;
    }

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

    double getPhasorOutput()
    {
        return phasorOutput;
    }

private:

    double currentSampleRate;
    double phasorFrequency;
    double phasorInc;
    double phasorOutput;
};

struct LFOSignalOutput
{
    double normalOutput;
    double invertedOutput;
    double quarterPhaseOutput;
    double invertedQuarterPhaseOutput;
};

class LFO : private Phasor
{

public:
    OscillatorParameters getParameters()
    {
        return oscillatorParameters;
    }

    void setParameters(OscillatorParameters pOscillatorParameters)
    {

        oscillatorParameters.waveform = pOscillatorParameters.waveform;
        oscillatorParameters.oscillatorFrequency_Hz = pOscillatorParameters.oscillatorFrequency_Hz;

        phasorNormal.setFrequency(oscillatorParameters.oscillatorFrequency_Hz);
        phasorQuarterPhase.setFrequency(oscillatorParameters.oscillatorFrequency_Hz);
    }


    bool reset(double pSampleRate)
    {
        currentSampleRate = pSampleRate;
        phasorNormal.reset(currentSampleRate, 0.0);
        phasorQuarterPhase.reset(currentSampleRate, 0.25);

        return true;
    }

     bool advanceAndWrap() override
    {
        auto state = phasorNormal.advanceAndWrap();
        phasorQuarterPhase.advanceAndWrap();
        return state;
    }


    LFOSignalOutput renderAudioOuput()
    {
        auto xNorm  = phasorNormal.getPhasorOutput();
        auto xQP = phasorQuarterPhase.getPhasorOutput();

        if (oscillatorParameters.waveform == generatorWaveform::kSaw)
        {
            lfoSignalOutput.normalOutput = unipolarToBipolar( xNorm);
            lfoSignalOutput.quarterPhaseOutput = unipolarToBipolar(xQP);

        }
        else if (oscillatorParameters.waveform == generatorWaveform::kTriangle)
        {

            lfoSignalOutput.normalOutput = (2.0 * fabs(unipolarToBipolar(xNorm)) - 1.0);
            lfoSignalOutput.quarterPhaseOutput = 2.0 * fabs(unipolarToBipolar(xQP)) - 1.0;
        }
        else if (oscillatorParameters.waveform == generatorWaveform::kSin)
        {
            xNorm =  2.0 * M_PI * xNorm - M_PI;
            xQP =    2.0 * M_PI * xQP   - M_PI;
            lfoSignalOutput.normalOutput = parabolicSine(-xNorm);
            lfoSignalOutput.quarterPhaseOutput = parabolicSine(-xQP);
        }

        lfoSignalOutput.invertedOutput = lfoSignalOutput.normalOutput * -1.0;
        lfoSignalOutput.invertedQuarterPhaseOutput = lfoSignalOutput.quarterPhaseOutput * -1.0;

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

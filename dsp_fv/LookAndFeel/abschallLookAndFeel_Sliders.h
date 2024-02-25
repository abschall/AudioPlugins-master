#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class abschallLookAndFeel_Sliders : public juce::LookAndFeel_V4

{
public:
    abschallLookAndFeel_Sliders(juce::Colour pMainColour) : mainColour(pMainColour)
    {

    }
    void drawRotarySlider(juce::Graphics& g,
        int x,
        int y,
        int width,
        int height,
        float sliderPos,
        const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider&) override
    {
        auto radius = (float)juce::jmin(width / 2, height / 2) - 4.0f;
        auto centreX = (float)x + (float)width * 0.5f;
        auto centreY = (float)y + (float)height * 0.5f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        //juce::Colour mainColour = juce::Colours::limegreen;
        juce::Colour shadow = juce::Colours::transparentBlack;

        g.setColour(mainColour);
        g.fillEllipse(rx, ry, rw, rw);

        juce::ColourGradient shadowGradient = juce::ColourGradient::ColourGradient(shadow, centreX - radius / 4, centreY - radius / 4, juce::Colours::black, 0, 2 * radius, true);
        g.setGradientFill(shadowGradient);
        g.fillEllipse(rx, ry, rw, rw);

        // Pointer
        juce::Path p;
        auto pointerRadius = radius * 0.3f;
        p.addEllipse(-pointerRadius / 2, -radius + pointerRadius / 2, pointerRadius, pointerRadius);

        juce::Colour pointerColour = juce::Colours::black;
        juce::ColourGradient pointerGradient = juce::ColourGradient::horizontal(pointerColour, 0, pointerColour.darker(5.0f), width);

        g.setGradientFill(pointerGradient);
        p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
        g.fillPath(p);
    }

    juce::Colour mainColour;

};
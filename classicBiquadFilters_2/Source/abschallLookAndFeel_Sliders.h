#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "font/ArialCE.h"

using namespace juce;
class valueLabel : public juce::Label, public juce::LookAndFeel_V4
{
public:

    valueLabel() {}
    void drawLabel(Graphics& g, Label& label) override
    {
        g.fillAll(label.findColour(Label::backgroundColourId));

        if (!label.isBeingEdited())
        {
            auto alpha = label.isEnabled() ? 1.0f : 0.5f;

            g.setColour(fontLabelColour.withMultipliedAlpha(alpha));
            g.setFont(customLabelFont);

            auto textArea = getLabelBorderSize(label).subtractedFrom(label.getLocalBounds());

            g.drawFittedText(label.getText(), textArea, label.getJustificationType(),
                jmax(1, (int)((float)textArea.getHeight() / customLabelFont.getHeight())),
                label.getMinimumHorizontalScale());
            label.setJustificationType(juce::Justification::centred);

            g.setColour(label.findColour(Label::outlineColourId).withMultipliedAlpha(alpha));
        }
        else if (label.isEnabled())
        {
            g.setColour(label.findColour(Label::outlineColourId));
        }

        g.drawRect(label.getLocalBounds());
    }

    void componentMovedOrResized(Component& component, bool /* wasMoved */, bool /* wasResized */)
    {
        const Font f(getLookAndFeel().getLabelFont(*this));

        setSize(component.getWidth(), roundToInt(f.getHeight()));
        setTopLeftPosition(component.getX(), component.getY() + component.getHeight());
    }

    static const Typeface::Ptr getCustomFont()
    {
        static auto typeface = Typeface::createSystemTypefaceFor(ArialCE::ArialCE_ttf, ArialCE::ArialCE_ttfSize);
        return typeface;
    }
    juce::Colour fontLabelColour = juce::Colours::white;
private:
    Typeface::Ptr getTypefaceForFont(const Font& f) override
    {
        return getCustomFont();
    }
    juce::Font customLabelFont = getCustomFont();
};

class rotaryPot : public juce::Slider
{
public:
    rotaryPot()
    {
        setSliderStyle(rotaryPot::RotaryVerticalDrag);
        setTextBoxStyle(rotaryPot::NoTextBox, false, 50, getTextBoxHeight() );

        valueLab.attachToComponent(this, false);
        onValueChange = [this] {
            if (getMaximum() > 100)
                valueLab.setText(String(ceil(getValue())) + suffix, juce::dontSendNotification);
            else
                valueLab.setText(String(ceil(getValue() * 100) / 100) + suffix, juce::dontSendNotification);
        };
        valueLab.setJustificationType(juce::Justification::verticallyCentred);
        this->getX();
    }

    void componentMovedOrResized(Component& component, bool /* wasMoved */, bool /* wasResized */)
    {

        setSize(component.getWidth(), getHeight());
        setTopLeftPosition(component.getX(), component.getY() + getHeight());
        valueLab.componentMovedOrResized(*this, false, false);

    }

    void createPot(double pInitialValue, double pMinVal, double pMaxVal, juce::String pValueSuffix, int size)
    {
        setValue(pInitialValue);
        setRange(pMinVal, pMaxVal);
        setTextValueSuffix(pValueSuffix);
        setSuffix(pValueSuffix);
        setSize(size, size);
    }


    void setSuffix(juce::String pSuffix)
    {
        suffix = pSuffix;
    }
    valueLabel valueLab;
private:
    juce::String suffix = "";
};

class abschallLookAndFeel_Sliders : public juce::LookAndFeel_V4, public juce::DeletedAtShutdown
{
public:
    abschallLookAndFeel_Sliders(juce::Colour pMainColour, juce::Colour pPointerColour) : 
        mainColour(pMainColour), pointerColour(pPointerColour)
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
        auto radius = (float)juce::jmin(width / 2, height / 2) - 15.0f;
        auto centreX = (float)x + (float)width * 0.5f;
        auto centreY = (float)y + (float)height * 0.5f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto lineThickness = radius / 25.0f;


        juce::Colour shadow = juce::Colours::transparentBlack;
        juce::ColourGradient shadowGradient = juce::ColourGradient::ColourGradient(shadow, centreX - radius / 4, centreY - radius / 4, mainColour, 0, 2 * radius, true);
        g.setColour(mainColour.brighter(.8f));
        g.drawEllipse(rx, ry, rw, rw, lineThickness);

       // g.setGradientFill(shadowGradient);
        g.drawEllipse(rx, ry, rw, rw, lineThickness);
        g.setColour(mainColour);
        g.fillEllipse(rx, ry, rw, rw);

        //g.setGradientFill(shadowGradient);
        g.fillEllipse(rx, ry, rw, rw);

        //  Pointer
        juce::Path p;
        auto pointerRadius = radius * 0.3f;
        p.addEllipse(-pointerRadius / 2, -radius + pointerRadius / 2, pointerRadius, pointerRadius);


        juce::ColourGradient pointerGradient = juce::ColourGradient::horizontal(pointerColour, 0, pointerColour.darker(5.0f), width);

        g.setGradientFill(pointerGradient);
        g.setColour(pointerColour);
        p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
        g.fillPath(p);
    }


    void drawLabel(Graphics& g, Label& label) override
    {
        g.fillAll(label.findColour(Label::backgroundColourId));

        if (!label.isBeingEdited())
        {
            auto alpha = label.isEnabled() ? 1.0f : 0.5f;

            g.setColour(fontLabelColour.withMultipliedAlpha(alpha));
            g.setFont(customTitleFont);
            
            auto textArea = getLabelBorderSize(label).subtractedFrom(label.getLocalBounds());
            customTitleFont.setHeight(15.0f);

            g.drawFittedText(label.getText(), textArea, label.getJustificationType(),
                jmax(1, (int)((float)textArea.getHeight() / customTitleFont.getHeight())),
                label.getMinimumHorizontalScale());

            label.setJustificationType(juce::Justification::centred);

            g.setColour(label.findColour(Label::outlineColourId).withMultipliedAlpha(alpha));
        }
        else if (label.isEnabled())
        {
            g.setColour(label.findColour(Label::outlineColourId));
        }

        g.drawRect(label.getLocalBounds());
    }

    juce::Colour mainColour;        
    juce::Colour pointerColour;
    juce::Colour fontLabelColour = juce::Colours::white;


    static const Typeface::Ptr getCustomFont()
    {
        static auto typeface = Typeface::createSystemTypefaceFor(ArialCE::ArialCEBold_ttf, ArialCE::ArialCEBold_ttfSize);
        return typeface;
    }
private:


    Typeface::Ptr getTypefaceForFont(const Font& f) override
    {
        return getCustomFont();
    }
    juce::Font customTitleFont = getCustomFont();
};

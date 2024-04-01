#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include "font/ArialCE.h" 

using namespace juce;

/// <summary>
/// Define a class titleLabel which inherits from juce::Label
/// </summary>
class titleLabel : public juce::Label
{
public:
    titleLabel() {
        setLookAndFeel(&lookAndFeel_Titles); // Set the custom look and feel for the label
    }
    ~titleLabel() {
        setLookAndFeel(nullptr); // Reset the look and feel when the object is destroyed
    }

    // Inner class defining the custom look and feel for titleLabel
    class LookAndFeel_Titles : public juce::LookAndFeel_V4, public juce::DeletedAtShutdown
    {
    public:
        // Override the drawLabel function to customize label drawing
        void drawLabel(Graphics& g, Label& label) override
        {
            // Fill the label background
            g.fillAll(label.findColour(Label::backgroundColourId));

            if (!label.isBeingEdited()) // If label is not being edited
            {
                auto alpha = label.isEnabled() ? 1.0f : 0.5f; // Set alpha based on label's enabled state

                // Set the color and font for the title label
                g.setColour(titleLabelColour.withMultipliedAlpha(alpha));
                g.setFont(customTitleFont);

                // Calculate text area and draw fitted text
                auto textArea = getLabelBorderSize(label).subtractedFrom(label.getLocalBounds());
                customTitleFont.setHeight(20.0f); // Set custom font height
                g.drawFittedText(label.getText(), textArea, label.getJustificationType(),
                    jmax(1, (int)((float)textArea.getHeight() / customTitleFont.getHeight())),
                    label.getMinimumHorizontalScale());

                label.setJustificationType(juce::Justification::centred);

                // Set outline color
                g.setColour(label.findColour(Label::outlineColourId).withMultipliedAlpha(alpha));
            }
            else if (label.isEnabled()) // If label is being edited and enabled
            {
                g.setColour(label.findColour(Label::outlineColourId));
            }

            // Draw label outline
            g.drawRect(label.getLocalBounds());
        }

        // Static function to get custom font
        static const Typeface::Ptr getCustomFont()
        {
            static auto typeface = Typeface::createSystemTypefaceFor(ArialCE::ArialCEBold_ttf, ArialCE::ArialCEBold_ttfSize);
            return typeface;
        }

    private:
        juce::Colour titleLabelColour = juce::Colours::dodgerblue.darker(1.0f);
        juce::Font customTitleFont = getCustomFont();
    };
    LookAndFeel_Titles lookAndFeel_Titles;

};

/// <summary>
///  Define a class valueLabel which inherits from juce::Label and juce::LookAndFeel_V4
/// </summary>
class valueLabel : public juce::Label, public juce::LookAndFeel_V4
{
public:

    valueLabel() {}

    // Override the drawLabel function to customize label drawing
    void drawLabel(Graphics& g, Label& label) override
    {
        // Fill the label background
        g.fillAll(label.findColour(Label::backgroundColourId));

        if (!label.isBeingEdited()) // If label is not being edited
        {
            auto alpha = label.isEnabled() ? 1.0f : 0.5f; // Set alpha based on label's enabled state

            // Set the color and font for the value label
            g.setColour(fontLabelColour.withMultipliedAlpha(alpha));
            g.setFont(customLabelFont);

            // Calculate text area and draw fitted text
            auto textArea = getLabelBorderSize(label).subtractedFrom(label.getLocalBounds());
            g.drawFittedText(label.getText(), textArea, label.getJustificationType(),
                jmax(1, (int)((float)textArea.getHeight() / customLabelFont.getHeight())),
                label.getMinimumHorizontalScale());

            // Center the label text horizontally
            label.setJustificationType(juce::Justification::centred);

            // Set outline color
            g.setColour(label.findColour(Label::outlineColourId).withMultipliedAlpha(alpha));
        }
        else if (label.isEnabled()) // If label is being edited and enabled
        {
            g.setColour(label.findColour(Label::outlineColourId));
        }

        // Draw label outline
        g.drawRect(label.getLocalBounds());
    }

    // Callback function invoked when the component is moved or resized
    void componentMovedOrResized(Component& component, bool /* wasMoved */, bool /* wasResized */) override
    {
        setSize(component.getWidth(), roundToInt(getLookAndFeel().getLabelFont(*this).getHeight()));
        setTopLeftPosition(component.getX(), component.getY() + component.getHeight());
    }

    // Static function to get custom font
    static const Typeface::Ptr getCustomFont()
    {
        static auto typeface = Typeface::createSystemTypefaceFor(ArialCE::ArialCE_ttf, ArialCE::ArialCE_ttfSize);
        return typeface;
    }

    juce::Colour fontLabelColour = juce::Colours::dodgerblue.darker(0.5f); // Default color for valueLabel
private:
    //Override getTypefaceForFont to set custom font for the label
    Typeface::Ptr getTypefaceForFont(const Font& f) override
    {
        return getCustomFont();
    }
    juce::Font customLabelFont = getCustomFont();
};

/// <summary>
/// Define a class rotaryPot which inherits from juce::Slider
/// </summary>
class rotaryPot : public juce::Slider
{
public:
    rotaryPot()
    {
        setSliderStyle(rotaryPot::Rotary); // Set the slider style to Rotary
        setTextBoxStyle(rotaryPot::NoTextBox, false, 50, getTextBoxHeight() - 2); // Set the text box style
        valueLab.attachToComponent(this, false); // Attach the valueLabel to the rotaryPot
        onValueChange = [this] {
            // Update the valueLabel text when the slider value changes
            if (getMaximum() > 100)
                valueLab.setText(String(ceil(getValue())) + suffix, juce::dontSendNotification);
            else
                valueLab.setText(String(ceil(getValue() * 100) / 100) + suffix, juce::dontSendNotification);
        };
        valueLab.setJustificationType(juce::Justification::verticallyCentred);
        this->getX(); // Get the x-coordinate of the rotaryPot
    }

    // Callback function invoked when the component is moved or resized
    void componentMovedOrResized(Component& component, bool /* wasMoved */, bool /* wasResized */)
    {

        setSize(component.getWidth(), getHeight()); // Set the size of the rotaryPot
        setTopLeftPosition(component.getX(), component.getY() + getHeight()); // Set the position of the rotaryPot
        valueLab.componentMovedOrResized(*this, false, false); // Update the position of the valueLabel
    }

    /// <summary>
    /// Function to create the rotary pot
    /// </summary>
    /// <param name="pInitialValue"></param>
    /// <param name="pMinVal"></param>
    /// <param name="pMaxVal"></param>
    /// <param name="pValueSuffix"></param>
    /// <param name="size"></param>
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
        suffix = pSuffix; // Set suffix
    }
    valueLabel valueLab; // Instance of valueLabel
private:
    juce::String suffix = ""; // Suffix for the rotary pot
};

/// <summary>
///  Define a class abschallLookAndFeel_Sliders which inherits from juce::LookAndFeel_V4
/// </summary>
class abschallLookAndFeel_Sliders : public juce::LookAndFeel_V4, public juce::DeletedAtShutdown
{
public:

    abschallLookAndFeel_Sliders(juce::Colour pMainColour, juce::Colour pPointerColour) :
        mainColour(pMainColour), pointerColour(pPointerColour)
    {   }

    // Override the drawRotarySlider function to customize slider drawing
    void drawRotarySlider(juce::Graphics& g,
        int x,
        int y,
        int width,
        int height,
        float sliderPos,
        const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider&) override
    {
        // Calculate necessary parameters for drawing
        auto radius = (float)juce::jmin(width / 2, height / 2) - 15.0f;
        auto centreX = (float)x + (float)width * 0.5f;
        auto centreY = (float)y + (float)height * 0.5f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto lineThickness = radius / 10.0f;

        // Define colors for shadow and pointer
        juce::Colour shadow = juce::Colours::transparentBlack;
        juce::ColourGradient shadowGradient = juce::ColourGradient::ColourGradient(shadow, centreX - radius / 4, centreY - radius / 4, mainColour, 0, 2 * radius, true);

        // Draw outer circle
        g.setColour(mainColour.brighter(.05f));
        g.drawEllipse(rx, ry, rw, rw, lineThickness);

        // Draw inner circle
        g.drawEllipse(rx, ry, rw, rw, lineThickness);
        g.setColour(mainColour);
        g.fillEllipse(rx, ry, rw, rw);

        g.fillEllipse(rx, ry, rw, rw);

        // Draw pointer
        juce::Path p;
        auto pointerRadius = radius * 0.3f;
        p.addEllipse(-pointerRadius / 2, -radius + pointerRadius / 2, pointerRadius, pointerRadius);

        // Define pointer gradient
        juce::ColourGradient pointerGradient = juce::ColourGradient::horizontal(pointerColour, 0, pointerColour.darker(5.0f), width);

        // Fill pointer path with gradient and draw it
        g.setGradientFill(pointerGradient);
        g.setColour(pointerColour);
        p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
        g.fillPath(p);
    }

    /// <summary>
    ///  Override the drawLabel function to customize label drawing
    /// </summary>
    /// <param name="g"></param>
    /// <param name="label"></param>
    void drawLabel(Graphics& g, Label& label) override
    {
        // Fill the label background
        g.fillAll(label.findColour(Label::backgroundColourId));

        if (!label.isBeingEdited()) // If label is not being edited
        {
            auto alpha = label.isEnabled() ? 1.0f : 0.5f; // Set alpha based on label's enabled state

            // Set the color and font for the label
            g.setColour(fontLabelColour.withMultipliedAlpha(alpha));
            g.setFont(customLabelFont);

            // Calculate text area and draw fitted text
            auto textArea = getLabelBorderSize(label).subtractedFrom(label.getLocalBounds());
            customLabelFont.setHeight(16.0f);
            g.drawFittedText(label.getText(), textArea, label.getJustificationType(),
                jmax(1, (int)((float)textArea.getHeight() / customLabelFont.getHeight())),
                label.getMinimumHorizontalScale());

            // Center the label text horizontally
            label.setJustificationType(juce::Justification::centred);

            // Set outline color
            g.setColour(label.findColour(Label::outlineColourId).withMultipliedAlpha(alpha));
        }
        else if (label.isEnabled()) // If label is being edited and enabled
        {
            g.setColour(label.findColour(Label::outlineColourId));
        }

        // Draw label outline
        g.drawRect(label.getLocalBounds());
    }

    juce::Colour mainColour;
    juce::Colour pointerColour;
    juce::Colour fontLabelColour = juce::Colours::dodgerblue.darker(0.05f);

    /// <summary>
    /// Static function to get custom font
    /// </summary>
    /// <returns></returns>
    static const Typeface::Ptr getCustomFont()
    {
        static auto typeface = Typeface::createSystemTypefaceFor(ArialCE::ArialCEBold_ttf, ArialCE::ArialCEBold_ttfSize);
        return typeface;
    }
private:

    /// <summary>
    /// Override getTypefaceForFont to set custom font for the label
    /// </summary>
    /// <param name="f"></param>
    /// <returns></returns>
    Typeface::Ptr getTypefaceForFont(const Font& f) override
    {
        return getCustomFont();
    }

    juce::Font customLabelFont = getCustomFont();
};
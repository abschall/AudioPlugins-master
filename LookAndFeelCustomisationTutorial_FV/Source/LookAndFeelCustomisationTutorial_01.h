/*
  ==============================================================================

   This file is part of the JUCE tutorials.
   Copyright (c) 2020 - Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             LookAndFeelCustomisationTutorial
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Explores look and feel customisations.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics
 exporters:        xcode_mac, vs2019, linux_make, xcode_iphone, androidstudio

 type:             Component
 mainClass:        MainContentComponent

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/


#pragma once
class abschallLookAndFeel_Sliders : public juce::LookAndFeel_V4

{
public:
    abschallLookAndFeel_Sliders()
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
       
        g.setColour(juce::Colours::white);
        g.drawEllipse(rx, ry, rw, rw,.5f) ;

        juce::Colour colour = juce::Colours::white;
        juce::ColourGradient cg = juce::ColourGradient::horizontal(colour, 0, colour.darker(5.0), width);

        g.setGradientFill(cg);
        g.fillEllipse(rx, ry, rw, rw);

        juce::Path p;
        //auto pointerLength = radius * 0.4f;
        //auto pointerWidth = 4.0f;
        //p.addRectangle(-pointerWidth / 2, -radius, pointerWidth, pointerLength);
        auto pointerRadius = radius * 0.3f;
        p.addEllipse(-pointerRadius / 2, -radius + pointerRadius/2, pointerRadius, pointerRadius);
        p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
        
        juce::Colour pointerGreen = juce::Colours::forestgreen;
        juce::ColourGradient pointerGradient = juce::ColourGradient::horizontal(colour, 0 , colour.darker(10.0f), width);
        g.setGradientFill(pointerGradient);
        g.setColour(pointerGreen);
        g.fillPath(p);
    }
};
//==============================================================================
class MainContentComponent   : public juce::Component
{
public:
    MainContentComponent()
    {
        getLookAndFeel().setColour (juce::Slider::thumbColourId, juce::Colours::red);
        newLook.setColour(juce::Slider::thumbColourId, juce::Colours::green);

        dial2.setLookAndFeel(&newLook);

        dial1.setSliderStyle (juce::Slider::Rotary);
        dial1.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible (dial1);

        dial2.setSliderStyle (juce::Slider::Rotary);
        dial2.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible (dial2);

        button1.setButtonText ("Button 1");
        addAndMakeVisible (button1);

        button2.setButtonText ("Button 2");
        addAndMakeVisible (button2);

        setSize (300, 200);
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    }

    void resized() override
    {
        auto border = 4;

        auto area = getLocalBounds();

        auto dialArea = area.removeFromTop (area.getHeight() / 2);
        dial1.setBounds (dialArea.removeFromLeft (dialArea.getWidth() / 2).reduced (border));
        dial2.setBounds (dialArea.reduced (border));

        auto buttonHeight = 30;

        button1.setBounds (area.removeFromTop (buttonHeight).reduced (border));
        button2.setBounds (area.removeFromTop (buttonHeight).reduced (border));
    }

private:
    juce::Slider dial1;
    juce::Slider dial2;
    juce::TextButton button1;
    juce::TextButton button2;
    abschallLookAndFeel_Sliders newLook;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};

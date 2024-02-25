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
#include "abschallLookAndFeel_Sliders.h"

//==============================================================================
class MainContentComponent   : public juce::Component
{
public:
    MainContentComponent()
    {

        setLookAndFeel(new abschallLookAndFeel_Sliders(juce::Colours::white));

        //dial1.setSliderStyle (juce::Slider::Rotary);
        //dial1.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible (dial1);
        addAndMakeVisible(dial2);
        dial1.setValue(0.0f);
        dial1.setRange(10, 15000);
        dial1.setSuffix("Hz");
        

        dial2.setValue(0.0f);
        dial2.setRange(-0.5f, 0.5f);
        dial2.setSuffix("%");

        dial2Label.attachToComponent(&dial2, false);
        addAndMakeVisible(dial2Label);
        dial1Label.attachToComponent(&dial1, false);
        addAndMakeVisible(dial1Label);

        button1.setButtonText ("Button 1");
        addAndMakeVisible (button1);

        button2.setButtonText ("Button 2");
        addAndMakeVisible (button2);

        addAndMakeVisible(dial1Label);
        dial1Label.setText("FREQ",juce::dontSendNotification);

        addAndMakeVisible(dial2Label);
        dial2Label.setText("MIX", juce::dontSendNotification);
        
        setSize (300, 300);
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    }

    void resized() override
    {
        auto border = 5;

        auto area = getLocalBounds();

        auto dialArea = area.removeFromTop (area.getHeight() / 2);
        dialArea = dialArea.removeFromBottom(dialArea.getHeight() - 30);

        dial1.setBounds (dialArea.removeFromLeft (dialArea.getWidth() / 2).reduced (border));
        dial2.setBounds (dialArea.reduced (border));

        auto buttonHeight = 30;

        button1.setBounds (area.removeFromTop (buttonHeight).reduced (border));
        button2.setBounds (area.removeFromTop (buttonHeight).reduced (border));
    }

private:

    juce::Label dial1Label;
    juce::Label dial2Label;
    juce::TextButton button1;
    juce::TextButton button2;
    rotaryPot dial1, dial2;
    //abschallLookAndFeel_Sliders newLook;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};

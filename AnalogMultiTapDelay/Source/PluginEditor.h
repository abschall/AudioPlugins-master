/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class rotaryPot : public juce::Slider
{
public:
    rotaryPot() {
        setSliderStyle(rotaryPot ::Rotary);
        setTextBoxStyle(rotaryPot ::TextBoxBelow, false, 50, getTextBoxHeight() - 2);
    }

    void createPot(double pInitialValue, double pMinVal, double pMaxVal, juce::String pValueSuffix, int size)
    {
        setValue(pInitialValue);
        setRange(pMinVal, pMaxVal);
        setTextValueSuffix(pValueSuffix);
        setSize(size, size);
    }
private:

};
class AnalogMultiTapDelayAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    AnalogMultiTapDelayAudioProcessorEditor (AnalogMultiTapDelayAudioProcessor&, juce::AudioProcessorValueTreeState& );
    ~AnalogMultiTapDelayAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;


private:
    AnalogMultiTapDelayAudioProcessor& audioProcessor;
    juce::AudioProcessorValueTreeState& valueTreeState;

    // GUI elements
    rotaryPot  delayPot;
    rotaryPot  timeRatioPot;
    rotaryPot  tapLevel_1Pot;
    rotaryPot  tapLevel_2Pot;
    rotaryPot  tapLevel_3Pot;
    rotaryPot  tapLevel_4Pot;
    rotaryPot  mixPot;
    rotaryPot  widthPot;
    rotaryPot  inputLevelPot;
    rotaryPot  feedbackPot;
    rotaryPot  noiseLevelPot;
    rotaryPot  saturationPot;
    rotaryPot  lowPassPot;
    rotaryPot  highPassPot;

    std::unique_ptr<SliderAttachment> delayAttachement;
    std::unique_ptr<SliderAttachment> timeRatioAttachement;
    std::unique_ptr<SliderAttachment> tapLevel_1Attachement;
    std::unique_ptr<SliderAttachment> tapLevel_2Attachement;
    std::unique_ptr<SliderAttachment> tapLevel_3Attachement;
    std::unique_ptr<SliderAttachment> tapLevel_4Attachement;
    std::unique_ptr<SliderAttachment> mixAttachement;
    std::unique_ptr<SliderAttachment> widthAttachement;
    std::unique_ptr<SliderAttachment> inputLevelAttachement;
    std::unique_ptr<SliderAttachment> feedbackAttachement;
    std::unique_ptr<SliderAttachment> noiseLevelAttachement;
    std::unique_ptr<SliderAttachment> saturationAttachement;
    std::unique_ptr<SliderAttachment> lowPassAttachement;
    std::unique_ptr<SliderAttachment> highPassAttachement;

    juce::Label delayLabel;
    juce::Label timeRatioLabel;
    juce::Label tapLevel_1Label;
    juce::Label tapLevel_2Label;
    juce::Label tapLevel_3Label;
    juce::Label tapLevel_4Label;
    juce::Label mixLabel;
    juce::Label widthLabel;
    juce::Label inputLevelLabel;
    juce::Label feedbackLabel;
    juce::Label noiseLevelLabel;
    juce::Label saturationLabel;
    juce::Label lowPassLabel;
    juce::Label highPassLabel;

    // Sizes
    int wPot = 150, heightPot = 120;
    int wLabel = 150, heightLabel = 30;
    int widthBox = wPot;
    int heightBox = heightPot + heightLabel;
    int mediumPotsWidth = 120, mediumPotsHeight = 100, mediumPotsLabelHeight = 20;
    int smallPotsWidth = 100, smallPotsHeight = 75, smallPotsLabelHeight = 20;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalogMultiTapDelayAudioProcessorEditor)
};

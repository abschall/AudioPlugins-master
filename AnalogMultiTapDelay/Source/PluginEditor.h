/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "abschallLookAndFeel_Sliders.h"


//==============================================================================
/**
*/
//class rotaryPot : public juce::Slider
//{
//public:
//    rotaryPot() {
//        setSliderStyle(rotaryPot ::Rotary);
//        setTextBoxStyle(rotaryPot ::TextBoxBelow, false, 50, getTextBoxHeight() - 2);
//    }
//
//    void createPot(double pInitialValue, double pMinVal, double pMaxVal, juce::String pValueSuffix, int size)
//    {
//        setValue(pInitialValue);
//        setRange(pMinVal, pMaxVal);
//        setTextValueSuffix(pValueSuffix);
//        setSize(size, size);
//    }
//private:
//
//};
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

   //vector<rotaryPot*> testPots = {&delayPot,&timeRatioPot};

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

    titleLabel delayLabel;
    titleLabel timeRatioLabel;
    titleLabel tapLevel_1Label;
    titleLabel tapLevel_2Label;
    titleLabel tapLevel_3Label;
    titleLabel tapLevel_4Label;
    titleLabel mixLabel;
    titleLabel widthLabel;
    titleLabel inputLevelLabel;
    titleLabel feedbackLabel;
    titleLabel noiseLevelLabel;
    titleLabel saturationLabel;
    titleLabel lowPassLabel;
    titleLabel highPassLabel;

    // Sizes
    int wPot = 150, heightPot = 120;
    int wLabel = 150, heightLabel = 50;
    int widthBox = wPot;
    int heightBox = heightPot + heightLabel;
    int mediumPotsWidth = 120, mediumPotsHeight = 100, mediumPotsLabelHeight = 50;
    int smallPotsWidth = 100, smallPotsHeight = 75, smallPotsLabelHeight = 40;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalogMultiTapDelayAudioProcessorEditor)
};

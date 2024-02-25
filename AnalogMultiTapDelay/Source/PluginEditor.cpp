/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
AnalogMultiTapDelayAudioProcessorEditor::AnalogMultiTapDelayAudioProcessorEditor (AnalogMultiTapDelayAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState(vts)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
   setSize (800, 360);
   setLookAndFeel(new abschallLookAndFeel_Sliders(juce::Colours::limegreen,juce::Colours::green));
   //==============================================================================
   addAndMakeVisible( delayPot);
   addAndMakeVisible( timeRatioPot);
   addAndMakeVisible( tapLevel_1Pot);
   addAndMakeVisible( tapLevel_2Pot);
   addAndMakeVisible( tapLevel_3Pot);
   addAndMakeVisible( tapLevel_4Pot);
   addAndMakeVisible( mixPot);
   addAndMakeVisible( widthPot);
   //addAndMakeVisible( inputLevelPot);
   addAndMakeVisible( feedbackPot);
   addAndMakeVisible( noiseLevelPot);
   //addAndMakeVisible( saturationPot);
   addAndMakeVisible( lowPassPot);
   addAndMakeVisible( highPassPot);
   
   //timeRatioPot.setLookAndFeel(&lookAndFeelTapDelay);
   // method for automatic scripting of delay functions 
   //for (auto i = 0; i < testPots.size(); ++i)
   //{
   //    addAndMakeVisible(testPots[i]);
   //     testPots[i]->setLookAndFeel(&lookAndFeelTapDelay);
   //}

   delayPot.createPot(10, 10, 10000, " ms", heightPot);
   delayPot.setSkewFactorFromMidPoint(2000);
   timeRatioPot.createPot(1.62, 1, 2, " ", heightPot);
   tapLevel_1Pot.createPot(0, 0, 1, " %", mediumPotsHeight);
   tapLevel_2Pot.createPot(0, 0, 1, " %", mediumPotsHeight);
   tapLevel_3Pot.createPot(0, 0, 1, " %", mediumPotsHeight);
   tapLevel_4Pot.createPot(0, 0, 1, " %", mediumPotsHeight);
   mixPot.createPot(50, 0, 100, " %", heightPot);
   widthPot.createPot(0, 0, 100, "",smallPotsHeight);
   //inputLevelPot.createPot(100, 0, 100, "%", heightPot);
   feedbackPot.createPot(100, 0, 100, " %", heightPot);
   noiseLevelPot.createPot(0, 0, 1, "", smallPotsHeight);
   //saturationPot.createPot(0, 0, 1, "", smallPotsHeight);
   lowPassPot.createPot(15000, 20, 15000, " Hz", smallPotsHeight);
   highPassPot.createPot(20, 20, 15000, " Hz", smallPotsHeight);

   delayAttachement.reset(new SliderAttachment(valueTreeState, "delay", delayPot));
   timeRatioAttachement.reset(new SliderAttachment(valueTreeState, "timeRatio", timeRatioPot));
   tapLevel_1Attachement.reset(new SliderAttachment(valueTreeState, "tapLevel_1", tapLevel_1Pot));
   tapLevel_2Attachement.reset(new SliderAttachment(valueTreeState, "tapLevel_2", tapLevel_2Pot));
   tapLevel_3Attachement.reset(new SliderAttachment(valueTreeState, "tapLevel_3", tapLevel_3Pot));
   tapLevel_4Attachement.reset(new SliderAttachment(valueTreeState, "tapLevel_4", tapLevel_4Pot));
   mixAttachement.reset(new SliderAttachment(valueTreeState, "mix", mixPot));
   widthAttachement.reset(new SliderAttachment(valueTreeState, "width", widthPot));
   inputLevelAttachement.reset(new SliderAttachment(valueTreeState, "inputLevel", inputLevelPot));
   feedbackAttachement.reset(new SliderAttachment(valueTreeState, "feedback", feedbackPot));
   noiseLevelAttachement.reset(new SliderAttachment(valueTreeState, "noiseLevel",noiseLevelPot));
   saturationAttachement.reset(new SliderAttachment(valueTreeState, "saturation", saturationPot));
   lowPassAttachement.reset(new SliderAttachment(valueTreeState, "lowPass", lowPassPot));
   highPassAttachement.reset(new SliderAttachment(valueTreeState, "highPass", highPassPot));

  //==============================================================================
  addAndMakeVisible( delayLabel);
  addAndMakeVisible( timeRatioLabel);
  addAndMakeVisible( tapLevel_1Label);
  addAndMakeVisible( tapLevel_2Label);
  addAndMakeVisible( tapLevel_3Label);
  addAndMakeVisible( tapLevel_4Label);
  addAndMakeVisible( mixLabel);
  addAndMakeVisible( widthLabel);
 // addAndMakeVisible( inputLevelLabel);
  addAndMakeVisible( feedbackLabel);
  addAndMakeVisible( noiseLevelLabel);
  //addAndMakeVisible( saturationLabel);
  addAndMakeVisible( lowPassLabel);
  addAndMakeVisible( highPassLabel);

  //==============================================================================
  delayLabel.       attachToComponent(&delayPot,false);
  timeRatioLabel.   attachToComponent(&timeRatioPot,false);
  tapLevel_1Label.  attachToComponent(&tapLevel_1Pot,false);
  tapLevel_2Label.  attachToComponent(&tapLevel_2Pot,false);
  tapLevel_3Label.  attachToComponent(&tapLevel_3Pot,false);
  tapLevel_4Label.  attachToComponent(&tapLevel_4Pot,false);
  mixLabel.         attachToComponent(&mixPot,false);
  widthLabel.       attachToComponent(&widthPot,false);
  inputLevelLabel.  attachToComponent(&inputLevelPot,false);
  feedbackLabel.    attachToComponent(&feedbackPot,false);
  noiseLevelLabel.  attachToComponent(&noiseLevelPot,false);
  saturationLabel.  attachToComponent(&saturationPot,false);
  lowPassLabel.     attachToComponent(&lowPassPot,false);
  highPassLabel.    attachToComponent(&highPassPot,false);

  //==============================================================================
  delayLabel.       setText("Delay", juce::dontSendNotification);
  timeRatioLabel.   setText("Time Ratio", juce::dontSendNotification);
  tapLevel_1Label.  setText("Tap 1", juce::dontSendNotification);
  tapLevel_2Label.  setText("Tap 2", juce::dontSendNotification);
  tapLevel_3Label.  setText("Tap 3", juce::dontSendNotification);
  tapLevel_4Label.  setText("Tap 4", juce::dontSendNotification);
  mixLabel.         setText("Mix", juce::dontSendNotification);
  widthLabel.       setText("Width", juce::dontSendNotification);
  inputLevelLabel.  setText("Input Level", juce::dontSendNotification);
  feedbackLabel.    setText("Feedback", juce::dontSendNotification);
  noiseLevelLabel.  setText("Noise", juce::dontSendNotification);
  saturationLabel.  setText("Saturation", juce::dontSendNotification);
  lowPassLabel.     setText("Low-Pass", juce::dontSendNotification);
  highPassLabel.    setText("High-Pass", juce::dontSendNotification);

  //==============================================================================
}

AnalogMultiTapDelayAudioProcessorEditor::~AnalogMultiTapDelayAudioProcessorEditor()
{
}

//==============================================================================
void AnalogMultiTapDelayAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colours::green.darker(5.0));

    g.setColour (juce::Colours::white);
}

void AnalogMultiTapDelayAudioProcessorEditor::resized()
{

    delayPot.setBounds(0, heightLabel, wPot, heightPot);

    timeRatioPot.setBounds(0, heightBox + heightLabel, wPot, heightPot);

    tapLevel_1Pot.setBounds(widthBox,           mediumPotsLabelHeight, mediumPotsWidth, mediumPotsHeight);
    tapLevel_2Pot.setBounds(widthBox,           2* mediumPotsLabelHeight + mediumPotsHeight , mediumPotsWidth, mediumPotsHeight);
    tapLevel_3Pot.setBounds(2 * widthBox + 20,  mediumPotsLabelHeight, mediumPotsWidth , mediumPotsHeight);
    tapLevel_4Pot.setBounds(2 * widthBox + 20,  2 * mediumPotsLabelHeight + mediumPotsHeight , mediumPotsWidth, mediumPotsHeight);

    mixPot.setBounds(3 * widthBox, heightLabel, wPot, heightPot);

    feedbackPot.setBounds(3 * widthBox , heightBox + heightLabel, wPot, heightPot);


    //inputLevelPot.setBounds(1 * widthBox, 2 * heightBox , wPot, heightPot);

   // feedbackPot.setBounds(2 * widthBox, 2 * heightBox , wPot, heightPot);

    noiseLevelPot.setBounds(4 * widthBox, heightLabel,smallPotsWidth,smallPotsHeight);
    widthPot.setBounds(4 * widthBox, heightBox , smallPotsWidth, smallPotsHeight);

    lowPassPot.setBounds(4* widthBox + smallPotsWidth, heightLabel, smallPotsWidth, smallPotsHeight);;
    highPassPot.    setBounds(4 * widthBox + smallPotsWidth, heightBox, smallPotsWidth, smallPotsHeight);;
}

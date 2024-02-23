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
    setSize (600, 500);

   //==============================================================================
   addAndMakeVisible( delayPot);
   addAndMakeVisible( timeRatioPot);
   addAndMakeVisible( tapLevel_1Pot);
   addAndMakeVisible( tapLevel_2Pot);
   addAndMakeVisible( tapLevel_3Pot);
   addAndMakeVisible( tapLevel_4Pot);
   addAndMakeVisible( mixPot);
   addAndMakeVisible( widthPot);
   addAndMakeVisible( inputLevelPot);
   addAndMakeVisible( feedbackPot);
   addAndMakeVisible( noiseLevelPot);
   addAndMakeVisible( saturationPot);
   addAndMakeVisible( lowPassPot);
   addAndMakeVisible( highPassPot);

   delayPot.createPot(10, 10, 2000, "ms", heightPot);
   timeRatioPot.createPot(1.62, 1, 2, " ", heightPot);
   tapLevel_1Pot.createPot(0, 0, 1, " ", mediumPotsHeight);
   tapLevel_2Pot.createPot(0, 0, 1, " ", mediumPotsHeight);
   tapLevel_3Pot.createPot(0, 0, 1, " ", mediumPotsHeight);
   tapLevel_4Pot.createPot(0, 0, 1, " ", mediumPotsHeight);
   mixPot.createPot(50, 0, 100, "%", heightPot);
   widthPot.createPot(0, 0, 100, "%", heightPot);
   inputLevelPot.createPot(100, 0, 100, "%", heightPot);
   feedbackPot.createPot(100, 0, 100, "%", heightPot);
   noiseLevelPot.createPot(0, 0, 1, "", smallPotsHeight);
   saturationPot.createPot(0, 0, 1, "", smallPotsHeight);
   lowPassPot.createPot(15000, 20, 15000, "Hz", smallPotsHeight);
   highPassPot.createPot(20, 20, 15000, "Hz", smallPotsHeight);

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
  addAndMakeVisible( inputLevelLabel);
  addAndMakeVisible( feedbackLabel);
  addAndMakeVisible( noiseLevelLabel);
  addAndMakeVisible( saturationLabel);
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
  delayLabel.       setJustificationType(juce::Justification::centred);
  timeRatioLabel.   setJustificationType(juce::Justification::centred);
  tapLevel_1Label.  setJustificationType(juce::Justification::centred);
  tapLevel_2Label.  setJustificationType(juce::Justification::centred);
  tapLevel_3Label.  setJustificationType(juce::Justification::centred);
  tapLevel_4Label.  setJustificationType(juce::Justification::centred);
  mixLabel.         setJustificationType(juce::Justification::centred);
  widthLabel.       setJustificationType(juce::Justification::centred);
  inputLevelLabel.  setJustificationType(juce::Justification::centred);
  feedbackLabel.    setJustificationType(juce::Justification::centred);
  noiseLevelLabel.  setJustificationType(juce::Justification::centred);
  saturationLabel.  setJustificationType(juce::Justification::centred);
  lowPassLabel.     setJustificationType(juce::Justification::centred);
  highPassLabel.    setJustificationType(juce::Justification::centred);
  
}

AnalogMultiTapDelayAudioProcessorEditor::~AnalogMultiTapDelayAudioProcessorEditor()
{
}

//==============================================================================
void AnalogMultiTapDelayAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
}

void AnalogMultiTapDelayAudioProcessorEditor::resized()
{

    //delayLabel.setBounds(0, 0, wLabel, heightLabel);
    delayPot.setBounds(0, heightLabel, wPot, heightPot);

    //timeRatioLabel.setBounds(0, heightBox, wLabel, heightLabel);
    timeRatioPot.setBounds(0, heightBox + heightLabel, wPot, heightPot);

    tapLevel_1Pot.setBounds(widthBox,           mediumPotsLabelHeight+heightLabel, mediumPotsWidth, mediumPotsHeight);
    tapLevel_2Pot.setBounds(widthBox,           2* mediumPotsLabelHeight + mediumPotsHeight + heightLabel, mediumPotsWidth, mediumPotsHeight);
    tapLevel_3Pot.setBounds(2 * widthBox + 20,  mediumPotsLabelHeight + heightLabel, mediumPotsWidth , mediumPotsHeight);
    tapLevel_4Pot.setBounds(2 * widthBox + 20,  2 * mediumPotsLabelHeight + mediumPotsHeight + heightLabel, mediumPotsWidth, mediumPotsHeight);

    //mixLabel.setBounds(3 * widthBox , 0, wLabel, heightLabel);
    mixPot.setBounds(3 * widthBox, heightLabel, wPot, heightPot);

   // widthLabel.setBounds(3 * widthBox , heightBox, wLabel, heightLabel);
    widthPot.setBounds(3 * widthBox , heightBox + heightLabel, wPot, heightPot);

    //inputLevelLabel.setBounds(0, 2*heightBox, wLabel, heightLabel);
    inputLevelPot.setBounds(0, 2 * heightBox + heightLabel, wPot, heightPot);

  //  feedbackLabel.setBounds(widthBox, 2 * heightBox, wLabel, heightLabel);
    feedbackPot.setBounds(widthBox, 2 * heightBox + heightLabel, wPot, heightPot);

   // noiseLevelLabel.setBounds(2 * widthBox, 2 * heightBox, smallPotsWidth, smallPotsLabelHeight);
    noiseLevelPot.setBounds(2 * widthBox, 2 * heightBox+smallPotsLabelHeight,smallPotsWidth,smallPotsHeight);
    //saturationLabel.setBounds(2 * widthBox,  2 * heightBox + smallPotsHeight + smallPotsLabelHeight, smallPotsWidth, smallPotsLabelHeight);
    saturationPot.setBounds(2 * widthBox, 2 * heightBox + smallPotsHeight + 2 * smallPotsLabelHeight, smallPotsWidth, smallPotsHeight);
    
    //lowPassLabel.setBounds(2 * widthBox + smallPotsWidth, 2 * heightBox, smallPotsWidth, smallPotsLabelHeight);
    lowPassPot.setBounds(2 * widthBox + smallPotsWidth, 2 * heightBox + smallPotsLabelHeight, smallPotsWidth, smallPotsHeight);;
    //highPassLabel.  setBounds(2 * widthBox + smallPotsWidth, 2 * heightBox + smallPotsHeight + smallPotsLabelHeight, smallPotsWidth, smallPotsLabelHeight);
    highPassPot.    setBounds(2 * widthBox + smallPotsWidth, 2 * heightBox + smallPotsHeight + 2 * smallPotsLabelHeight, smallPotsWidth, smallPotsHeight);;
}

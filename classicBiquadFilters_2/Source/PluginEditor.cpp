/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

void ClassicBiquadFilters_2AudioProcessorEditor::filterStyleSelection()
{
    juce::AudioProcessorParameterWithID* param = valueTreeState.getParameter("filtertype");
    switch (filterTypeChoice.getSelectedId()) {
    case 1:
        param->setValueNotifyingHost(0.0);
        break;
    case 2:
        param->setValueNotifyingHost(0.25);
        break;
    case 3:
        param->setValueNotifyingHost(0.5);
        break;
    case 4:
        param->setValueNotifyingHost(0.75);
        break;
    default:
        param->setValueNotifyingHost(0.0);
        break;
    }
}
//==============================================================================
ClassicBiquadFilters_2AudioProcessorEditor::ClassicBiquadFilters_2AudioProcessorEditor (ClassicBiquadFilters_2AudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState(vts)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (480, 300);

    // create Sliders
    addAndMakeVisible(dBSlider);
    dBSlider.setValue(0);
    dBSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 90, dBSlider.getTextBoxHeight() - 2);
    dBSlider.setSize(dBSlider.getTextBoxWidth() + 150, dBSlider.getTextBoxHeight());
    dBSlider.setRange(-100.0f, 0);
    dBSlider.setTextValueSuffix("dB");

    addAndMakeVisible(dBLabel);
    dBLabel.setText("Noise level (dB)", juce::dontSendNotification);

    addAndMakeVisible(cutOffSlider);
    cutOffSlider.setValue(10.0f);
    cutOffSlider.setRange(10.0f, 15000.0f);
    cutOffSlider.setSkewFactorFromMidPoint(5000.0f);
    cutOffSlider.setTextValueSuffix("Hz");
    cutOffSlider.setSliderStyle(juce::Slider::Rotary);
    cutOffSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, dBSlider.getTextBoxHeight() - 2);
    cutOffSlider.setSize(90, 90);
    cutOffAttachment.reset(new SliderAttachment(valueTreeState, "fc", cutOffSlider));


    addAndMakeVisible(qSlider);
    qSlider.setValue(1);
    qSlider.setRange(0.001f, 20.0f);
    qSlider.setSkewFactorFromMidPoint(1);
    qSlider.setSliderStyle(juce::Slider::Rotary);
    qSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, qSlider.getTextBoxHeight() - 2);
    qSlider.setSize(90, 90);
    qAttachment.reset(new SliderAttachment(valueTreeState, "qfactor", qSlider));


    addAndMakeVisible(cutOffLabel);
    cutOffLabel.setText("Cut-off Freq", juce::dontSendNotification);
    cutOffLabel.attachToComponent(&cutOffSlider, false);
    cutOffLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(qLabel);
    qLabel.setText("Q factor", juce::dontSendNotification);
    qLabel.attachToComponent(&qSlider, false);
    qLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(dryWetSlider);
    dryWetSlider.setValue(1.0f);
    dryWetSlider.setRange(0.0f, 1.0f);
    dryWetSlider.setSliderStyle(juce::Slider::Rotary);
    dryWetSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, qSlider.getTextBoxHeight() - 2);
    dryWetSlider.setSize(90, 90);
    dryWetAttachment.reset(new SliderAttachment(valueTreeState, "dryWet", dryWetSlider));

    addAndMakeVisible(dryWetLabel);
    dryWetLabel.setText("Dry / Wet", juce::dontSendNotification);
    dryWetLabel.attachToComponent(&dryWetSlider, false);
    dryWetLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(filterTypeChoice);
    filterTypeChoice.addItem("LPF1", 1);
    filterTypeChoice.addItem("LPF2", 2);
    filterTypeChoice.addItem("HPF1", 3);
    filterTypeChoice.addItem("HPF2", 4);
    //filterTypeChoice.onChange = [this] { filterStyleSelection(); };// first method to update valueTreeParameter
    //filterTypeChoice.setSelectedId(1);
    // second method using an attachment
    filterTypeChoiceAttachment.reset(new ComboBoxAttachment(valueTreeState, "filtertype", filterTypeChoice));


}


ClassicBiquadFilters_2AudioProcessorEditor::~ClassicBiquadFilters_2AudioProcessorEditor()
{
}

//==============================================================================
void ClassicBiquadFilters_2AudioProcessorEditor::paint(juce::Graphics& g)
{

}

void ClassicBiquadFilters_2AudioProcessorEditor::resized()
{
    int widthTri = 120;
    int heightTri = 120;

    cutOffLabel.setBounds(0, 0, widthTri, 30);
    qLabel.setBounds(widthTri, 0, widthTri, 30);
    dryWetLabel.setBounds(2 * widthTri, 0, widthTri, 30);

    cutOffSlider.setBounds(0, 30, widthTri, 90);
    qSlider.setBounds(widthTri, 30, widthTri, 90);
    dryWetSlider.setBounds(2 * widthTri, 30, widthTri, 90);

    dBLabel.setBounds(0, heightTri, widthTri, 30);
    dBSlider.setBounds(widthTri, heightTri, 250 + dBSlider.getTextBoxWidth(), 30);

    filterTypeChoice.setBounds(3 * widthTri + 10, 10, 100, heightTri/3);
}

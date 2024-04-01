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

    setSize (480, 170);
    
    // set custom LookAndFeel, which applies to Pots and Sliders Only 
    setLookAndFeel(new abschallLookAndFeel_Sliders(juce::Colours::navy.darker(20.0f), juce::Colours::dodgerblue.darker(1.0f)));

    cutOffAttachment.reset(new SliderAttachment(valueTreeState, "fc", cutOffPot));
    qAttachment.reset(new SliderAttachment(valueTreeState, "qfactor", qPot));
    dryWetAttachment.reset(new SliderAttachment(valueTreeState, "dryWet", dryWetPot));

    // addAndMakeVisible and attachToComponent
    for (auto i = 0; i < pots.size(); ++i)
    {
        addAndMakeVisible(pots[i]);

        addAndMakeVisible(titleLabels[i]);
        titleLabels[i]->attachToComponent(pots[i],false);
    }

    // set pot titles 
    qLabel.setText("RES", juce::dontSendNotification);
    qLabel.setJustificationType(juce::Justification::centred);

    cutOffLabel.setText("CUT-OFF", juce::dontSendNotification);
    cutOffLabel.setJustificationType(juce::Justification::centred);

    dryWetLabel.setText("AMOUNT", juce::dontSendNotification);
    dryWetLabel.setJustificationType(juce::Justification::centred);

    // set pot value suffixes
    cutOffPot.setSuffix(" Hz");
    cutOffPot.setSkewFactorFromMidPoint(2000.0);
    // filter choiceComboBox
    addAndMakeVisible(filterTypeChoice);
    filterTypeChoice.addItem("LPF1", 1);
    filterTypeChoice.addItem("LPF2", 2);
    filterTypeChoice.addItem("HPF1", 3);
    filterTypeChoice.addItem("HPF2", 4);
    filterTypeChoiceAttachment.reset(new ComboBoxAttachment(valueTreeState, "filtertype", filterTypeChoice));
}


ClassicBiquadFilters_2AudioProcessorEditor::~ClassicBiquadFilters_2AudioProcessorEditor()
{
}

//==============================================================================
void ClassicBiquadFilters_2AudioProcessorEditor::paint(juce::Graphics& g)
{
    // set background colour
    g.setColour(juce::Colours::navy.darker(2.0f));
    g.fillAll();
}

void ClassicBiquadFilters_2AudioProcessorEditor::resized()
{
    int widthTri = 120;
    int heightTri = 120;

    // set components position
    // would reauire a ore flexible of setting their positions
    cutOffPot.setBounds(0, heightLabel, wPot, heightPot);
    qPot.setBounds(widthBox, heightLabel, wPot, heightPot);
    dryWetPot.setBounds(2*widthBox, heightLabel, wPot, heightPot);
    filterTypeChoice.setBounds(3 * widthBox + 10, heightLabel, 80, heightBox/3);
}

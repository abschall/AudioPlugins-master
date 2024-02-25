#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    // Make sure you set the size of the component after
    // you add any child components.
    setSize(480,150);
    setAudioChannels(0, 1); // setting two output channels and 0 inout channels 

    // create Sliders
    addAndMakeVisible(dBSlider);
    dBSlider.setValue(0);
    dBSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 90 , dBSlider.getTextBoxHeight()-2);
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
    cutOffSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, dBSlider.getTextBoxHeight()-2);
    cutOffSlider.setSize(90, 90);
    cutOffSlider.onValueChange= [this] () {
        updateFilterParameters();
        };


    addAndMakeVisible(qSlider);
    qSlider.setValue(1);
    qSlider.setRange(0.001f, 20.0f);
    qSlider.setSkewFactorFromMidPoint(1);
    qSlider.setSliderStyle(juce::Slider::Rotary);
    qSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, qSlider.getTextBoxHeight() - 2);
    qSlider.setSize(90, 90);
    qSlider.onValueChange = [this]() {
        updateFilterParameters();
        };

    addAndMakeVisible(cutOffLabel);
    cutOffLabel.setText("Cut-off Freq", juce::dontSendNotification);
    cutOffLabel.attachToComponent(&cutOffSlider,false);
    cutOffLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(qLabel);
    qLabel.setText("Q factor", juce::dontSendNotification);
    qLabel.attachToComponent(& qSlider,false);
    qLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(dryWetSlider);
    dryWetSlider.setValue(1.0f);
    dryWetSlider.setRange(0.0f,1.0f);
    dryWetSlider.setSliderStyle(juce::Slider::Rotary);
    dryWetSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, qSlider.getTextBoxHeight() - 2);
    dryWetSlider.setSize(90, 90);
    dryWetSlider.onValueChange = [this]() {
        updateFilterParameters();
        };

    addAndMakeVisible(dryWetLabel);
    dryWetLabel.setText("Dry / Wet", juce::dontSendNotification);
    dryWetLabel.attachToComponent(&dryWetSlider, false);
    dryWetLabel.setJustificationType(juce::Justification::centred);

    // Create Buttons
    addAndMakeVisible(LPF1textButton);
    LPF1textButton.setSize(100, 20);
    LPF1textButton.setButtonText("LPF1");
    LPF1textButton.addListener(this);

    addAndMakeVisible(LPF2textButton);
    LPF2textButton.setSize(100, 20);
    LPF2textButton.setButtonText("LPF2");
    LPF2textButton.addListener(this);

    addAndMakeVisible(HPF1textButton);
    HPF1textButton.setSize(100, 20);
    HPF1textButton.setButtonText("HPF1");
    HPF1textButton.addListener(this);

    addAndMakeVisible(HPF2textButton);
    HPF2textButton.setSize(100, 20);
    HPF2textButton.setButtonText("HPF2");
    HPF2textButton.addListener(this);


}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    dBSlider.setValue(0.01);
    currentSampleRate = sampleRate;
    filter.setFilterType("LPF1");
    filter.setFilterGain(1.0f);

    osc.oscillatorFrequency_Hz = 1;
    osc.waveform = generatorWaveform::kTriangle;

    lfo.setParameters(osc);
    lfo.reset(sampleRate);

}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{

    // the level control does not use a listener on the slider ( why ?) 
    level = juce::Decibels::decibelsToGain((float)dBSlider.getValue());// auto : lets the compiler decide which type to use 

    auto level_scale = level * 2;

    
    for (auto channel = 0; channel != bufferToFill.buffer->getNumChannels(); ++channel)
    {

        auto* buffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);

        for (auto sample = bufferToFill.startSample; sample < bufferToFill.numSamples; ++sample)
        {
            lfoSignalOutput = lfo.renderAudioOuput();
            buffer[sample] = filter.processAudioSample(lfoSignalOutput.normalOutput * level_scale*level_scale * random.nextFloat() - level);
        }
    }

    // This is a very basic and crude way of scaling the output level of the noise generator,
    // applied to sinusoid would introduce zipper noise.
    // The level would be jumping from one value to the next one inbetween audio blocks,
    // this introduces some crackling to the output sound. 
}

void MainComponent::releaseResources()
{
    juce::Logger::getCurrentLogger()->writeToLog("Releasing Audio ressources ");

    LPF1textButton.removeListener(this);
    LPF2textButton.removeListener(this);
    HPF1textButton.removeListener(this);
    HPF2textButton.removeListener(this);
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // You can add your drawing code here!
}

void MainComponent::resized()
{
    int widthTri = 120;
    int heightTri = 120;

    cutOffLabel.setBounds(0, 0, widthTri, 30);
    qLabel.     setBounds(widthTri,0, widthTri, 30);
    dryWetLabel.setBounds(2 * widthTri, 0, widthTri, 30);

    cutOffSlider.setBounds(0, 30, widthTri, 90);
    qSlider.    setBounds(widthTri, 30, widthTri, 90);
    dryWetSlider.setBounds(2 * widthTri, 30, widthTri, 90);

    dBLabel.    setBounds(0, heightTri, widthTri, 30);
    dBSlider.   setBounds(widthTri, heightTri, 250+dBSlider.getTextBoxWidth(), 30);


    LPF1textButton.setBounds(3 * widthTri + 10, 10,            100, 20);
    LPF2textButton.setBounds(3 * widthTri + 10, 10 + 1 * (25), 100, 20);
    HPF1textButton.setBounds(3 * widthTri + 10, 10 + 2 * (25), 100, 20);
    HPF2textButton.setBounds(3 * widthTri + 10, 10 + 3 * (25), 100, 20);
}

//==============================================================================
void MainComponent::updateFilterParameters()
{
    fc = cutOffSlider.getValue();
    Q = qSlider.getValue();
    K = dryWetSlider.getValue();
    filter.setCoefficients(fc, Q, currentSampleRate);
    filter.setFilterGain(K);
}

void MainComponent::buttonClicked(juce::Button* button)
{
    if (button == &LPF1textButton)
        filter.setFilterType("LPF1");
    if (button == &LPF2textButton)
        filter.setFilterType("LPF2");
    if (button == &HPF1textButton)
        filter.setFilterType("HPF1");
    if (button == &HPF2textButton)
        filter.setFilterType("HPF2");

    updateFilterParameters();
}

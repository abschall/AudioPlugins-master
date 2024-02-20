#pragma once

#include <JuceHeader.h>

// !!! child components have to be placed before the parent component, code is compiled sequentially  
class RoofComponent : public juce::Component
{
public:

    void paint(juce::Graphics& g) override
    {
        g.setColour(juce::Colours::red);

        juce::Path roof;
        roof.addTriangle(0.0f, (float)getHeight(), (float)getWidth(), (float)getHeight(), (float)getWidth() / 2.0f, 0.0f);
        g.fillPath(roof);
    }
private:

};
class WallComponent : public juce::Component
{
public:
    WallComponent() {}
    void paint(juce::Graphics& g) override
    {
        g.fillCheckerBoard(getLocalBounds().toFloat(), 30, 10, juce::Colours::sandybrown, juce::Colours::saddlebrown);
    }
private:

};
class HouseComponent : public juce::Component
{
public:
    HouseComponent()
    {
        addAndMakeVisible(wall);
        addAndMakeVisible(roof);
    }

    void resized() override
    {
        auto separation = juce::jlimit(2, 10, getHeight() / 20);                                                               // [1]

        roof.setBounds(0, 0, getWidth(), (int)(getHeight() * 0.2) - separation / 2);                                          // [2]
        wall.setBounds(0, (int)(getHeight() * 0.20) + separation / 2, getWidth(), (int)(getHeight() * 0.80) - separation);   // [3]

    /*
    [1]: First we calculate the separation between the roof and wall. Let's make this 1⁄20 of the height of 
    the house but make it no smaller than 2 pixels — using the jlimit() function. This so that there is always
    a gap between the roof and the wall even when the height is small. When the height is large then the gap
    remains proportional to the height.
    [2]: The roof is set to be the full width of the house and 1⁄5 of the height of the house. This is adjusted 
    to account for the separation.
    [3]: The wall is positioned under the roof occupying 4⁄5 of the height of the house. This is also adjusted 
    to account for the separation.
    */
    }
private:
    WallComponent wall;
    RoofComponent roof;
};
class FloorComponent : public juce::Component
{
public:
    FloorComponent() = default;
    void paint(juce::Graphics& g) override
    {
        g.setColour(juce::Colours::green);
        g.drawLine(0.0f, (float)getHeight() / 2.0f, (float)getWidth(), (float)getHeight() / 2.0f, 5.0f);
    }
private:

};
class SceneComponent : public juce::Component
{
public:
    SceneComponent()
    {
        //child components are made visible in the parent Constructor class
        addAndMakeVisible(house);
        addAndMakeVisible(floor);
        addAndMakeVisible(house2);
    }
    void paint(juce::Graphics& g) override
    {
        // painting the Scene component with the desired color 
        // the paint method applies to the subsequent Component class, ans is used within the addAndMakeVisible() method
        g.fillAll(juce::Colours::aqua);
    }

    void resized() override
    {
        // child components' positions and bounds are set in the parent's resized function 
        floor.setBounds(10, 297, 580, 5);
        house.setBounds(300, 70, 200, 220);
        house2.setBounds(200, 35, 40, 40);
    }

private:
    // house and roof are child components of the paraent Scene(class SceneComponent)
    HouseComponent  house;
    FloorComponent floor;
    HouseComponent house2;
};

class MainComponent  : public juce::Component
{
public:
    //==============================================================================
    MainComponent()
    {
        addAndMakeVisible(scene);
        // in order to have a child component to be displayed, it also needs to be made visible.
        // both theses actions are performed by te addAndMakeVisible() method 
        setSize(300, 300);
    }
    //~MainComponent() override;

    //==============================================================================
    void paint (juce::Graphics&) override {}
    void resized() override
    {
        //scene.setBounds(0, 0, getWidth(), getHeight());
        //is equivalent to
        scene.setBounds(getLocalBounds());
    }

private:
    //==============================================================================
    // Your private member variables go here...
    SceneComponent  scene;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

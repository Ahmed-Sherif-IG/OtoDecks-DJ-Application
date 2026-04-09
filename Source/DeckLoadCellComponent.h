#pragma once

#include <JuceHeader.h>
#include "../JuceLibraryCode/JuceHeader.h"
#include "CustomLookAndFeel.h"

using namespace std;
using namespace juce;

class DeckLoadCellComponent : public juce::Component,
    public juce::Button::Listener
{
public:
    DeckLoadCellComponent(std::function<void(int, int)> onLoadTrack,
        std::function<void(int)> onRemoveTrack)
        : loadTrackCallback(onLoadTrack),
        removeTrackCallback(onRemoveTrack)
    {
        addAndMakeVisible(deck1Button);
        addAndMakeVisible(deck2Button);
        addAndMakeVisible(removeButton);

        deck1Button.setButtonText("Deck 1");
        deck2Button.setButtonText("Deck 2");
        removeButton.setButtonText("Remove");

        deck1Button.addListener(this);
        deck2Button.addListener(this);
        removeButton.addListener(this);

       
        deck1Button.setLookAndFeel(&customLook);
        deck2Button.setLookAndFeel(&customLook);
        removeButton.setLookAndFeel(&customLook);


    }

    ~DeckLoadCellComponent() override
    {
        
        deck1Button.setLookAndFeel(nullptr);
        deck2Button.setLookAndFeel(nullptr);
        removeButton.setLookAndFeel(nullptr);
    }

    void setRow(int newRow)
    {
        rowNumber = newRow;
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(4);  
        int buttonWidth = (area.getWidth() - 8) / 3; 

        deck1Button.setBounds(area.removeFromLeft(buttonWidth));
        area.removeFromLeft(4); 

        deck2Button.setBounds(area.removeFromLeft(buttonWidth));
        area.removeFromLeft(4); 

        removeButton.setBounds(area.removeFromLeft(buttonWidth));
    }



    void buttonClicked(juce::Button* button) override
    {
        if (rowNumber < 0)
            return;

        if (button == &deck1Button && loadTrackCallback)
            loadTrackCallback(rowNumber, 1);
        else if (button == &deck2Button && loadTrackCallback)
            loadTrackCallback(rowNumber, 2);
        else if (button == &removeButton && removeTrackCallback)
            removeTrackCallback(rowNumber);
    }

private:
    int rowNumber = -1;
    juce::TextButton deck1Button;
    juce::TextButton deck2Button;
    juce::TextButton removeButton;

    std::function<void(int, int)> loadTrackCallback;
    std::function<void(int)> removeTrackCallback;

    CustomLookAndFeel customLook;
};

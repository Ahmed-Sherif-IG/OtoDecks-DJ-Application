#pragma once

#include <JuceHeader.h>
#include "../shared/Callbacks.h"

class DeckLoadCellComponent : public juce::Component,
                               public juce::Button::Listener
{
public:
    DeckLoadCellComponent(LoadTrackByRowFn onLoadTrack,
                          RemoveTrackFn   onRemoveTrack)
        : loadTrackCallback(std::move(onLoadTrack)),
          removeTrackCallback(std::move(onRemoveTrack))
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
    }

    ~DeckLoadCellComponent() override = default;

    void setRow(int newRow) { rowNumber = newRow; }

    void resized() override
    {
        auto area        = getLocalBounds().reduced(4);
        int  buttonWidth = (area.getWidth() - 8) / 3;

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

    LoadTrackByRowFn loadTrackCallback;
    RemoveTrackFn    removeTrackCallback;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeckLoadCellComponent)
};

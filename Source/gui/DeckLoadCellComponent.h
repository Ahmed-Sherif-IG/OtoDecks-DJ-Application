#pragma once

#include <JuceHeader.h>
#include "../shared/Callbacks.h"
#include "../shared/CustomLookAndFeel.h"

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

        deck1Button.setButtonText("LOAD A");
        deck2Button.setButtonText("LOAD B");
        removeButton.setButtonText("DELETE");

        deck1Button.setColour(juce::TextButton::buttonColourId,
                              CustomLookAndFeel::colour(CustomLookAndFeel::accentBlueValue).withAlpha(0.78f));
        deck2Button.setColour(juce::TextButton::buttonColourId,
                              CustomLookAndFeel::colour(CustomLookAndFeel::accentOrangeValue).withAlpha(0.78f));
        removeButton.setColour(juce::TextButton::buttonColourId,
                               CustomLookAndFeel::colour(CustomLookAndFeel::panelRaisedColourValue));

        for (auto* button : { &deck1Button, &deck2Button, &removeButton })
        {
            button->setColour(juce::TextButton::textColourOffId,
                              CustomLookAndFeel::colour(CustomLookAndFeel::textColourValue));
        }

        deck1Button.addListener(this);
        deck2Button.addListener(this);
        removeButton.addListener(this);
    }

    ~DeckLoadCellComponent() override = default;

    void setRow(int newRow) { rowNumber = newRow; }

    void resized() override
    {
        auto area = getLocalBounds().reduced(3, 3);
        const int gap = 5;
        const int buttonWidth = (area.getWidth() - gap * 2) / 3;

        deck1Button.setBounds(area.removeFromLeft(buttonWidth));
        area.removeFromLeft(gap);
        deck2Button.setBounds(area.removeFromLeft(buttonWidth));
        area.removeFromLeft(gap);
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

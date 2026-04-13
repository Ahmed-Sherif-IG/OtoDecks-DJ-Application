#pragma once

#include <JuceHeader.h>

class CustomLookAndFeel : public juce::LookAndFeel_V4 {
public:
    CustomLookAndFeel() {
        // Flat, grayscale design
        setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
        setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        setColour(juce::Slider::thumbColourId, juce::Colours::lightgrey);
        setColour(juce::Slider::trackColourId, juce::Colours::black);
        setColour(juce::Label::textColourId, juce::Colours::lightgrey);
        setColour(juce::ListBox::backgroundColourId, juce::Colours::darkgrey);
    }

    juce::Font getTextButtonFont(juce::TextButton&, int) override {
        return juce::Font(juce::FontOptions(14.0f));
    }

    juce::Typeface::Ptr getTypefaceForFont(const juce::Font& font) override
    {
        return juce::LookAndFeel_V4::getTypefaceForFont(font);
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
        const juce::Colour& backgroundColour,
        bool isMouseOverButton, bool isButtonDown) override
    {
        auto bounds = button.getLocalBounds().toFloat();

        // Use flat color without gradients or rounded edges
        auto baseColour = backgroundColour.withAlpha(isButtonDown ? 0.9f : (isMouseOverButton ? 0.7f : 1.0f));

        g.setColour(baseColour);
        g.fillRect(bounds);  // No rounded corners
    }
};

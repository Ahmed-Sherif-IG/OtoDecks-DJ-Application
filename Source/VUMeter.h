#pragma once

#include <JuceHeader.h>
#include <functional>

class VUMeter : public juce::Component,
                private juce::Timer
{
public:
    explicit VUMeter(std::function<float()> getLevelFn)
        : getLevel(std::move(getLevelFn))
    {
        startTimerHz(30);
    }

    ~VUMeter() override { stopTimer(); }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        float level = juce::jlimit(0.0f, 1.0f, getLevel ? getLevel() : 0.0f);

        // Background
        g.setColour(juce::Colour::fromRGB(15, 15, 15));
        g.fillRect(bounds);
        g.setColour(juce::Colours::darkgrey);
        g.drawRect(bounds, 1.0f);

        // Level bar — green → yellow → red
        float fillH = bounds.getHeight() * level;
        auto  fill  = bounds.removeFromBottom(fillH);

        juce::Colour barColour = level > 0.85f ? juce::Colours::red
                               : level > 0.6f  ? juce::Colours::gold
                                               : juce::Colours::limegreen;
        g.setColour(barColour.withAlpha(0.85f));
        g.fillRect(fill.reduced(2.0f, 0.0f));

        // Peak line
        float peakY = bounds.getY() + (1.0f - peakLevel_) * bounds.getHeight();
        g.setColour(juce::Colours::white.withAlpha(0.7f));
        g.drawHorizontalLine(static_cast<int>(peakY),
                              bounds.getX() + 2.0f,
                              bounds.getRight() - 2.0f);
    }

    void resized() override {}

private:
    void timerCallback() override
    {
        float current = getLevel ? getLevel() : 0.0f;

        // Peak hold with decay
        if (current >= peakLevel_)
        {
            peakLevel_      = current;
            peakHoldFrames_ = 30;
        }
        else if (peakHoldFrames_ > 0)
        {
            --peakHoldFrames_;
        }
        else
        {
            peakLevel_ = juce::jmax(0.0f, peakLevel_ - 0.02f);
        }

        repaint();
    }

    std::function<float()> getLevel;
    float peakLevel_      = 0.0f;
    int   peakHoldFrames_ = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VUMeter)
};

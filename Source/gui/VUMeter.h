#pragma once

#include <JuceHeader.h>
#include <functional>
#include "../shared/CustomLookAndFeel.h"

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
        auto frame = bounds.reduced(1.0f);
        const float level = juce::jlimit(0.0f, 1.0f, getLevel ? getLevel() : 0.0f);

        g.setColour(CustomLookAndFeel::colour(CustomLookAndFeel::panelAltColourValue));
        g.fillRoundedRectangle(frame, 8.0f);
        g.setColour(CustomLookAndFeel::colour(CustomLookAndFeel::outlineColourValue));
        g.drawRoundedRectangle(frame, 8.0f, 1.0f);

        auto inner = frame.reduced(4.0f);
        for (int i = 1; i < 8; ++i)
        {
            const float y = inner.getY() + inner.getHeight() * (static_cast<float>(i) / 8.0f);
            g.setColour(CustomLookAndFeel::colour(CustomLookAndFeel::outlineColourValue).withAlpha(0.45f));
            g.drawHorizontalLine(static_cast<int>(y), inner.getX(), inner.getRight());
        }

        juce::ColourGradient meterGradient(CustomLookAndFeel::colour(CustomLookAndFeel::accentGreenValue), inner.getBottomLeft(),
                                           CustomLookAndFeel::colour(CustomLookAndFeel::accentOrangeValue), inner.withY(inner.getHeight() * 0.35f).getTopLeft(), false);
        meterGradient.addColour(0.9, CustomLookAndFeel::colour(CustomLookAndFeel::accentRedValue));

        const float fillHeight = inner.getHeight() * level;
        auto fill = inner.removeFromBottom(fillHeight);
        g.setGradientFill(meterGradient);
        g.fillRoundedRectangle(fill, 5.0f);

        const float peakY = inner.getY() + (1.0f - peakLevel_) * inner.getHeight();
        g.setColour(CustomLookAndFeel::colour(CustomLookAndFeel::textColourValue).withAlpha(0.9f));
        g.drawHorizontalLine(static_cast<int>(peakY), inner.getX(), inner.getRight());
    }

private:
    void timerCallback() override
    {
        const float current = getLevel ? getLevel() : 0.0f;

        if (current >= peakLevel_)
        {
            peakLevel_      = current;
            peakHoldFrames_ = 24;
        }
        else if (peakHoldFrames_ > 0)
        {
            --peakHoldFrames_;
        }
        else
        {
            peakLevel_ = juce::jmax(0.0f, peakLevel_ - 0.018f);
        }

        repaint();
    }

    std::function<float()> getLevel;
    float peakLevel_      = 0.0f;
    int   peakHoldFrames_ = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VUMeter)
};

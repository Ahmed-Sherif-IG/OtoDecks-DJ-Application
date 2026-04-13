#pragma once

#include <JuceHeader.h>
#include "DJAudioPlayer.h"
#include "VUMeter.h"

class MixerPanel : public juce::Component,
                   public juce::Slider::Listener
{
public:
    MixerPanel(DJAudioPlayer& player1,
               DJAudioPlayer& player2,
               std::atomic<float>& masterGain);
    ~MixerPanel() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void sliderValueChanged(juce::Slider*) override;

private:
    void applyCrossfader(double pos);  // pos in [0,1]: 0=full D1, 1=full D2

    DJAudioPlayer& player1_;
    DJAudioPlayer& player2_;
    std::atomic<float>& masterGain_;

    // Crossfader
    juce::Slider crossfader;
    juce::Label  crossfaderLabel;

    // Master volume
    juce::Slider masterSlider;
    juce::Label  masterLabel;

    // Per-deck trim
    juce::Slider trim1Slider, trim2Slider;
    juce::Label  trim1Label,  trim2Label;

    // EQ deck 1
    juce::Slider low1Slider,  mid1Slider,  high1Slider;
    juce::Label  low1Label,   mid1Label,   high1Label;

    // EQ deck 2
    juce::Slider low2Slider,  mid2Slider,  high2Slider;
    juce::Label  low2Label,   mid2Label,   high2Label;

    // VU meters
    VUMeter vuMeter1;
    VUMeter vuMeter2;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MixerPanel)
};

#pragma once

#include <JuceHeader.h>
#include <functional>

class WaveformDisplay : public juce::Component,
                        public juce::ChangeListener
{
public:
    WaveformDisplay(juce::AudioFormatManager& formatManagerToUse,
                    juce::AudioThumbnailCache& cacheToUse);
    ~WaveformDisplay() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    void loadURL(juce::URL audioURL);
    void setPositionRelative(double pos);

    // M2: deck-specific waveform colour (default white)
    void setWaveformColour(juce::Colour colour);

    // M2: visualise loop region [start,end] in relative coords [0,1]
    void setLoopRegion(double start, double end, bool active);

    // M2: visualise cue marker at relative position [0,1]
    void setCueMarker(double pos);

    // M2: callback invoked when user clicks/drags to seek
    std::function<void(double)> onSeek;

    // M5: beat positions in relative coords for grid overlay
    void setBeatPositions(const std::vector<double>& beats);

private:
    void seekToX(int x);

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;

    juce::AudioThumbnail audioThumb;
    bool   fileLoaded    = false;
    double position      = 0.0;

    juce::Colour waveformColour{ juce::Colours::white };

    double loopStartRel_ = 0.0;
    double loopEndRel_   = 0.0;
    bool   loopActive_   = false;
    double cueMarker_    = 0.0;

    std::vector<double> beatPositions_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformDisplay)
};

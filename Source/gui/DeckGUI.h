#pragma once

#include <JuceHeader.h>
#include <array>
#include "../audio/DJAudioPlayer.h"
#include "WaveformDisplay.h"
#include "../audio/BPMAnalyser.h"
#include "../shared/Callbacks.h"

class DeckGUI : public juce::Component,
                public juce::Button::Listener,
                public juce::Slider::Listener,
                public juce::FileDragAndDropTarget,
                public juce::Timer
{
public:
    DeckGUI(DJAudioPlayer& player,
            juce::AudioFormatManager& formatManagerToUse,
            juce::AudioThumbnailCache& cacheToUse,
            const juce::String& deckTitleText,
            juce::Colour deckColour);
    ~DeckGUI() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    void buttonClicked(juce::Button*) override;
    void sliderValueChanged(juce::Slider*) override;

    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    void timerCallback() override;

    void loadFile(const juce::File& file);
    void triggerSync();   // M6: called by keyboard shortcut

    // M4: show "Artist - Title" in deck header
    void setNowPlaying(const juce::String& info);

    GetOtherDeckSpeedFn getOtherDeckSpeed;
    std::function<double()> getOtherDeckBPM;  // M5: BPM-based sync

    float getSpeed() const;

private:
    static juce::String formatTime(double seconds);
    void  recordTap();
    void resetVolumeToDefault();
    void resetSpeedToDefault();
    juce::Colour getDeckGlowColour() const;

    juce::AudioFormatManager& formatManager_;
    juce::FileChooser fChooser{ "Select a file..." };

    // Transport
    juce::TextButton playButton{ "Play" };
    juce::TextButton stopButton{ "Stop" };
    juce::TextButton loadButton{ "Load" };
    juce::TextButton syncButton{ "Sync" };

    // Loop (M2)
    juce::TextButton loopInButton   { "Loop In"  };
    juce::TextButton loopOutButton  { "Loop Out" };
    juce::TextButton clearLoopButton{ "Clr Loop" };

    // Loop bar-length buttons
    juce::TextButton loopBar1_4Button { "1/4" };
    juce::TextButton loopBar1_2Button { "1/2" };
    juce::TextButton loopBar1Button   { "1"   };
    juce::TextButton loopBar2Button   { "2"   };
    juce::TextButton loopBar4Button   { "4"   };
    juce::TextButton loopBar8Button   { "8"   };

    // Cue (M2)
    juce::TextButton setCueButton{ "Set Cue" };
    juce::TextButton goCueButton { "Go Cue"  };

    // M6: filter toggles
    juce::TextButton lpfButton { "LP" };
    juce::TextButton hpfButton { "HP" };

    // M5: TAP BPM
    juce::TextButton tapButton { "TAP" };

    // Hotcue pads (8 per deck)
    static constexpr int kNumHotcues = 8;
    std::array<juce::TextButton, kNumHotcues> hotcuePads;
    juce::TextButton clearHotcuesButton { "CLR" };
    void updateHotcuePadColors();
    void clearAllHotcues();

    // Sliders
    juce::Slider volSlider;
    juce::Slider speedSlider;
    juce::Slider posSlider;
    juce::TextButton resetVolumeButton{ "VOL RESET" };
    juce::TextButton resetSpeedButton{ "SPD RESET" };

    // Labels
    juce::Label volLabel, speedLabel, posLabel;
    juce::Label deckTitle;
    juce::Label stateLabel;
    juce::Label timeLabel;
    juce::Label bpmLabel;    // M5

    WaveformDisplay waveformDisplay;
    BPMAnalyser     bpmAnalyser_;   // M5

    DJAudioPlayer&   player;
    juce::Colour     deckColour_;

    // TAP BPM state (M5)
    std::vector<double> tapTimes_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeckGUI)
};

#pragma once

#include <JuceHeader.h>
#include "../JuceLibraryCode/JuceHeader.h"
#include "DJAudioPlayer.h"
#include "WaveformDisplay.h"
#include "CustomLookAndFeel.h"

using namespace juce;

class DeckGUI : public Component,
    public Button::Listener,
    public Slider::Listener,
    public FileDragAndDropTarget,
    public Timer
{
public:
    DeckGUI(DJAudioPlayer* player,
        juce::AudioFormatManager& formatManagerToUse,
        juce::AudioThumbnailCache& cacheToUse,
        const juce::String& deckTitleText);
    ~DeckGUI() override;

    void paint(Graphics&) override;
    void resized() override;

    void buttonClicked(Button*) override;
    void sliderValueChanged(Slider*) override;

    bool isInterestedInFileDrag(const StringArray& files) override;
    void filesDropped(const StringArray& files, int x, int y) override;

    void timerCallback() override;

    void loadFile(const File& file);

    std::function<void()> onSyncRequested; 

    std::function<float()> getOtherDeckSpeed;

    float getSpeed() const;

private:
    juce::FileChooser fChooser{ "Select a file..." };

    TextButton playButton{ "Play" };
    TextButton stopButton{ "Stop" };
    TextButton loadButton{ "Load" };
    TextButton syncButton{ "SYNC" }; 

    Slider volSlider;
    Slider speedSlider;
    Slider posSlider;

    Label volLabel, speedLabel, posLabel;
    Label deckTitle;

    WaveformDisplay waveformDisplay;

    DJAudioPlayer* player;

    CustomLookAndFeel customLook;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeckGUI)
};

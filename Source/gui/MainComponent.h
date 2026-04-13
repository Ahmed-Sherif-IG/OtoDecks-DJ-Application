#pragma once

#include <JuceHeader.h>
#include "../audio/DJAudioPlayer.h"
#include "DeckGUI.h"
#include "MixerPanel.h"
#include "../library/PlaylistComponent.h"
#include "../shared/CustomLookAndFeel.h"

class MainComponent : public juce::AudioAppComponent,
                      public juce::KeyListener
{
public:
    MainComponent();
    ~MainComponent() override;

    // AudioAppComponent
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    // Component
    void paint(juce::Graphics& g) override;
    void resized() override;

    // KeyListener (M6)
    bool keyPressed(const juce::KeyPress& key, juce::Component* origin) override;

private:
    void showAboutDialog();  // M6

    CustomLookAndFeel customLook;

    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbnailCache{ 100 };

    DJAudioPlayer player1{ formatManager };
    DJAudioPlayer player2{ formatManager };

    juce::MixerAudioSource mixerSource;

    // Applied in getNextAudioBlock — set by MixerPanel
    std::atomic<float> masterGain_{ 0.8f };

    std::unique_ptr<DeckGUI>    deckGUI1;
    std::unique_ptr<DeckGUI>    deckGUI2;
    std::unique_ptr<MixerPanel> mixerPanel;

    PlaylistComponent playlistComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

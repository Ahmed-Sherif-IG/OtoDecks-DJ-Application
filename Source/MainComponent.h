#pragma once
#include <JuceHeader.h>
#include "../JuceLibraryCode/JuceHeader.h"
#include "DJAudioPlayer.h"
#include "DeckGUI.h"
#include "PlaylistComponent.h"
#include "CustomLookAndFeel.h"

using namespace juce;

class MainComponent : public AudioAppComponent
{
public:
    MainComponent();
    ~MainComponent();

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void paint(Graphics& g) override;
    void resized() override;

private:
    std::unique_ptr<DeckGUI> deckGUI1;
    std::unique_ptr<DeckGUI> deckGUI2;

    AudioFormatManager formatManager;
    AudioThumbnailCache thumbnailCache{ 100 };

    DJAudioPlayer player1{ formatManager };
    DJAudioPlayer player2{ formatManager };

    MixerAudioSource mixerSource;

    PlaylistComponent playlistComponent;

    CustomLookAndFeel customLook;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

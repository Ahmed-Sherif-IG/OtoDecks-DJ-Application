#pragma once

#include <JuceHeader.h>
#include "../audio/DJAudioPlayer.h"
#include "DeckGUI.h"
#include "MixerPanel.h"
#include "../library/PlaylistComponent.h"
#include "../shared/CustomLookAndFeel.h"

class MainComponent : public juce::AudioAppComponent,
                      public juce::KeyListener,
                      public juce::Button::Listener,
                      public juce::Timer
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
    void buttonClicked(juce::Button* button) override;
    void timerCallback() override;

private:
    void showAboutDialog();  // M6
    void startRecording();
    void stopRecording();
    void updateRecordingUI();
    void logTrackHistory(int deckNumber, const juce::File& file);

    CustomLookAndFeel customLook;

    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbnailCache{ 100 };

    DJAudioPlayer player1{ formatManager };
    DJAudioPlayer player2{ formatManager };

    juce::MixerAudioSource mixerSource;

    // Applied in getNextAudioBlock — set by MixerPanel
    std::atomic<float> masterGain_{ 0.8f };

    juce::TextButton recordButton{ "REC" };
    juce::Label      recordStatusLabel;
    juce::TimeSliceThread recordingThread_{ "OtoDeck Recorder" };
    std::unique_ptr<juce::AudioFormatWriter::ThreadedWriter> threadedWriter_;
    std::atomic<juce::AudioFormatWriter::ThreadedWriter*> activeWriter_{ nullptr };
    juce::int64 recordingStartMs_ = 0;
    double recordingSampleRate_ = 44100.0;

    std::unique_ptr<DeckGUI>    deckGUI1;
    std::unique_ptr<DeckGUI>    deckGUI2;
    std::unique_ptr<MixerPanel> mixerPanel;

    PlaylistComponent playlistComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

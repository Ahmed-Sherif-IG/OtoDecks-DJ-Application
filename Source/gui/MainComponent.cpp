#include "MainComponent.h"

MainComponent::MainComponent()
{
    setLookAndFeel(&customLook);

    if (juce::RuntimePermissions::isRequired(juce::RuntimePermissions::recordAudio)
        && !juce::RuntimePermissions::isGranted(juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio,
            [&](bool granted) { if (granted) setAudioChannels(2, 2); });
    }
    else
    {
        setAudioChannels(0, 2);
    }

    formatManager.registerBasicFormats();

    deckGUI1 = std::make_unique<DeckGUI>(player1, formatManager, thumbnailCache,
                                          "Deck 1", juce::Colour(0x3A7BD5FF));
    deckGUI2 = std::make_unique<DeckGUI>(player2, formatManager, thumbnailCache,
                                          "Deck 2", juce::Colour(0xF5A623FF));

    deckGUI1->getOtherDeckSpeed = [this]() { return deckGUI2->getSpeed(); };
    deckGUI2->getOtherDeckSpeed = [this]() { return deckGUI1->getSpeed(); };

    deckGUI1->getOtherDeckBPM = [this]() { return player2.getBPM(); };
    deckGUI2->getOtherDeckBPM = [this]() { return player1.getBPM(); };

    mixerPanel = std::make_unique<MixerPanel>(player1, player2, masterGain_);

    addAndMakeVisible(deckGUI1.get());
    addAndMakeVisible(mixerPanel.get());
    addAndMakeVisible(deckGUI2.get());
    addAndMakeVisible(playlistComponent);

    playlistComponent.loadTrackToDeck = [this](juce::File file, int deckNumber)
    {
        if (deckNumber == 1) { player1.loadURL(juce::URL{ file }); deckGUI1->loadFile(file); }
        else if (deckNumber == 2) { player2.loadURL(juce::URL{ file }); deckGUI2->loadFile(file); }
    };

    playlistComponent.onNowPlaying = [this](int deck, const juce::String& info)
    {
        if (deck == 1 && deckGUI1) deckGUI1->setNowPlaying(info);
        if (deck == 2 && deckGUI2) deckGUI2->setNowPlaying(info);
    };

    addKeyListener(this);
    setWantsKeyboardFocus(true);

    setSize(1100, 720);
}

MainComponent::~MainComponent()
{
    removeKeyListener(this);
    deckGUI1   = nullptr;
    deckGUI2   = nullptr;
    mixerPanel = nullptr;
    setLookAndFeel(nullptr);
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    player1.prepareToPlay(samplesPerBlockExpected, sampleRate);
    player2.prepareToPlay(samplesPerBlockExpected, sampleRate);

    mixerSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    mixerSource.addInputSource(&player1, false);
    mixerSource.addInputSource(&player2, false);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    mixerSource.getNextAudioBlock(bufferToFill);
    bufferToFill.buffer->applyGain(bufferToFill.startSample,
                                    bufferToFill.numSamples,
                                    masterGain_.load());
}

void MainComponent::releaseResources()
{
    player1.releaseResources();
    player2.releaseResources();
    mixerSource.releaseResources();
}

//==============================================================================
void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(14, 14, 14));
}

void MainComponent::resized()
{
    // Fully proportional — works from 900×600 to 1920×1080
    auto area = getLocalBounds().reduced(
        juce::jmax(4, getWidth() / 120),
        juce::jmax(4, getHeight() / 80));

    int deckAreaH = static_cast<int>(area.getHeight() * 0.65f);
    auto deckArea = area.removeFromTop(deckAreaH);

    int mixerW = juce::jmax(150, deckArea.getWidth() / 6);
    int deckW  = (deckArea.getWidth() - mixerW) / 2;

    if (deckGUI1)   deckGUI1->setBounds(deckArea.removeFromLeft(deckW).reduced(4));
    if (mixerPanel) mixerPanel->setBounds(deckArea.removeFromLeft(mixerW).reduced(4));
    if (deckGUI2)   deckGUI2->setBounds(deckArea.reduced(4));

    playlistComponent.setBounds(area.reduced(4));
}

//==============================================================================
// M6: full keyboard shortcut table
bool MainComponent::keyPressed(const juce::KeyPress& key, juce::Component*)
{
    using KP = juce::KeyPress;

    // Play/Stop deck 1
    if (key == KP('1'))
    {
        auto s = player1.getState();
        if (s.isPlaying) player1.stop(); else player1.start();
        return true;
    }
    // Play/Stop deck 2
    if (key == KP('2'))
    {
        auto s = player2.getState();
        if (s.isPlaying) player2.stop(); else player2.start();
        return true;
    }
    // Space = toggle deck 1
    if (key == KP(KP::spaceKey))
    {
        auto s = player1.getState();
        if (s.isPlaying) player1.stop(); else player1.start();
        return true;
    }
    // Q = Set Cue Deck 1
    if (key == KP('q') || key == KP('Q'))
    { player1.setCuePoint(); return true; }
    // W = Go Cue Deck 1
    if (key == KP('w') || key == KP('W'))
    { player1.jumpToCue(); return true; }
    // O = Set Cue Deck 2
    if (key == KP('o') || key == KP('O'))
    { player2.setCuePoint(); return true; }
    // P = Go Cue Deck 2
    if (key == KP('p') || key == KP('P'))
    { player2.jumpToCue(); return true; }
    // S = Sync deck 2 to deck 1
    if (key == KP('s') || key == KP('S'))
    {
        if (deckGUI2) deckGUI2->triggerSync();
        return true;
    }

    return false;
}

void MainComponent::showAboutDialog()
{
    auto* content = new juce::Component();
    content->setSize(320, 130);

    auto* lbl = new juce::Label();
    juce::String aboutText = "OtoDecks DJ v1.0\n\nBuilt with JUCE "
                           + juce::String(JUCE_VERSION)
                           + "\n"
                           + juce::String(__DATE__);
    lbl->setText(aboutText, juce::dontSendNotification);
    lbl->setJustificationType(juce::Justification::centred);
    lbl->setBounds(content->getLocalBounds());
    lbl->setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    content->addAndMakeVisible(lbl);

    juce::DialogWindow::LaunchOptions opts;
    opts.content.setOwned(content);
    opts.dialogTitle                  = "About OtoDecks";
    opts.dialogBackgroundColour       = juce::Colour::fromRGB(30, 30, 30);
    opts.escapeKeyTriggersCloseButton = true;
    opts.useNativeTitleBar            = false;
    opts.resizable                    = false;
    opts.launchAsync();
}

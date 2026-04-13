#include "MainComponent.h"
#include "../shared/CustomLookAndFeel.h"

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
                                          "Deck A", CustomLookAndFeel::colour(CustomLookAndFeel::accentBlueValue));
    deckGUI2 = std::make_unique<DeckGUI>(player2, formatManager, thumbnailCache,
                                          "Deck B", CustomLookAndFeel::colour(CustomLookAndFeel::accentOrangeValue));

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

    setSize(1180, 780);
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
    auto bounds = getLocalBounds().toFloat();
    juce::ColourGradient background(juce::Colour(0xFF0C1017), bounds.getTopLeft(),
                                    juce::Colour(0xFF121826), bounds.getBottomLeft(), false);
    g.setGradientFill(background);
    g.fillAll();

    g.setColour(CustomLookAndFeel::colour(CustomLookAndFeel::accentBlueValue).withAlpha(0.08f));
    g.fillEllipse(-120.0f, -80.0f, 360.0f, 220.0f);
    g.setColour(CustomLookAndFeel::colour(CustomLookAndFeel::accentOrangeValue).withAlpha(0.08f));
    g.fillEllipse(static_cast<float>(getWidth() - 220), static_cast<float>(getHeight() - 220), 320.0f, 220.0f);
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced(14, 12);
    const int sectionGap = 10;
    const int deckAreaH = static_cast<int>(area.getHeight() * 0.64f);
    auto deckArea = area.removeFromTop(deckAreaH);

    const int mixerW = juce::jlimit(200, 280, deckArea.getWidth() / 5);
    const int deckW  = (deckArea.getWidth() - mixerW - sectionGap * 2) / 2;

    if (deckGUI1)   deckGUI1->setBounds(deckArea.removeFromLeft(deckW));
    deckArea.removeFromLeft(sectionGap);
    if (mixerPanel) mixerPanel->setBounds(deckArea.removeFromLeft(mixerW));
    deckArea.removeFromLeft(sectionGap);
    if (deckGUI2)   deckGUI2->setBounds(deckArea);

    area.removeFromTop(12);
    playlistComponent.setBounds(area);
}

//==============================================================================
bool MainComponent::keyPressed(const juce::KeyPress& key, juce::Component*)
{
    using KP = juce::KeyPress;

    if (key == KP('1'))
    {
        auto s = player1.getState();
        if (s.isPlaying) player1.stop(); else player1.start();
        return true;
    }
    if (key == KP('2'))
    {
        auto s = player2.getState();
        if (s.isPlaying) player2.stop(); else player2.start();
        return true;
    }
    if (key == KP(KP::spaceKey))
    {
        auto s = player1.getState();
        if (s.isPlaying) player1.stop(); else player1.start();
        return true;
    }
    if (key == KP('q') || key == KP('Q'))
    { player1.setCuePoint(); return true; }
    if (key == KP('w') || key == KP('W'))
    { player1.jumpToCue(); return true; }
    if (key == KP('o') || key == KP('O'))
    { player2.setCuePoint(); return true; }
    if (key == KP('p') || key == KP('P'))
    { player2.jumpToCue(); return true; }
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

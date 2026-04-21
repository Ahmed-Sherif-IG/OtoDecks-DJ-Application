#include "MainComponent.h"
#include "../shared/CustomLookAndFeel.h"

namespace
{
    juce::File getProjectRootFromExecutable()
    {
        auto dir = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                       .getParentDirectory();

        for (int i = 0; i < 5 && dir.getParentDirectory() != dir; ++i)
            dir = dir.getParentDirectory();

        return dir.getChildFile("Source").isDirectory()
            ? dir
            : juce::File::getCurrentWorkingDirectory();
    }
}

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
    deckGUI1->getOtherDeckPositionSeconds = [this]() { return player2.getCurrentPositionSeconds(); };
    deckGUI2->getOtherDeckPositionSeconds = [this]() { return player1.getCurrentPositionSeconds(); };

    mixerPanel = std::make_unique<MixerPanel>(player1, player2, masterGain_);

    addAndMakeVisible(deckGUI1.get());
    addAndMakeVisible(mixerPanel.get());
    addAndMakeVisible(deckGUI2.get());
    addAndMakeVisible(playlistComponent);
    addAndMakeVisible(recordButton);
    addAndMakeVisible(recordStatusLabel);

    recordButton.addListener(this);
    recordButton.setColour(juce::TextButton::buttonColourId,
                           CustomLookAndFeel::colour(CustomLookAndFeel::accentRedValue).withAlpha(0.78f));
    recordButton.setColour(juce::TextButton::textColourOffId,
                           CustomLookAndFeel::colour(CustomLookAndFeel::textColourValue));

    recordStatusLabel.setText("READY", juce::dontSendNotification);
    recordStatusLabel.setFont(juce::Font(juce::FontOptions(10.5f).withStyle("Bold")));
    recordStatusLabel.setJustificationType(juce::Justification::centred);
    recordStatusLabel.setColour(juce::Label::textColourId,
                                CustomLookAndFeel::colour(CustomLookAndFeel::mutedTextColourValue));
    recordingThread_.startThread();
    startTimerHz(4);

    playlistComponent.loadTrackToDeck = [this](juce::File file, int deckNumber)
    {
        if (deckNumber == 1) { player1.loadURL(juce::URL{ file }); deckGUI1->loadFile(file); }
        else if (deckNumber == 2) { player2.loadURL(juce::URL{ file }); deckGUI2->loadFile(file); }
        logTrackHistory(deckNumber, file);
        playlistComponent.setNowPlayingFile(file);
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
    stopTimer();
    stopRecording();
    recordingThread_.stopThread(1000);
    recordButton.removeListener(this);
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
    recordingSampleRate_ = sampleRate;
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

    if (auto* writer = activeWriter_.load())
    {
        const float* channels[2] = {
            bufferToFill.buffer->getReadPointer(0, bufferToFill.startSample),
            bufferToFill.buffer->getNumChannels() > 1
                ? bufferToFill.buffer->getReadPointer(1, bufferToFill.startSample)
                : bufferToFill.buffer->getReadPointer(0, bufferToFill.startSample)
        };
        writer->write(channels, bufferToFill.numSamples);
    }
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
    juce::ColourGradient background(CustomLookAndFeel::colour(CustomLookAndFeel::panelColourValue).brighter(0.03f),
                                    bounds.getTopLeft(),
                                    CustomLookAndFeel::colour(CustomLookAndFeel::panelColourValue).darker(0.22f),
                                    bounds.getBottomLeft(), false);
    g.setGradientFill(background);
    g.fillAll();

    auto canvas = bounds.reduced(10.0f, 10.0f);
    if (deckGUI1 != nullptr && mixerPanel != nullptr && deckGUI2 != nullptr)
    {
        auto performanceSurface = deckGUI1->getBounds().getUnion(mixerPanel->getBounds()).getUnion(deckGUI2->getBounds())
            .expanded(8, 8).toFloat();
        juce::ColourGradient deckWash(juce::Colours::white.withAlpha(0.018f), performanceSurface.getTopLeft(),
                                      juce::Colours::transparentWhite, performanceSurface.getBottomLeft(), false);
        g.setGradientFill(deckWash);
        g.fillRoundedRectangle(performanceSurface, 18.0f);

        g.setColour(juce::Colours::black.withAlpha(0.14f));
        g.drawRoundedRectangle(performanceSurface, 18.0f, 0.9f);
        g.setColour(juce::Colours::white.withAlpha(0.024f));
        g.drawRoundedRectangle(performanceSurface.reduced(0.8f), 17.2f, 0.6f);
    }

    auto librarySurface = playlistComponent.getBounds().expanded(6, 6).toFloat();
    juce::ColourGradient libraryWash(juce::Colours::white.withAlpha(0.014f), librarySurface.getTopLeft(),
                                     juce::Colours::transparentWhite, librarySurface.getBottomLeft(), false);
    g.setGradientFill(libraryWash);
    g.fillRoundedRectangle(librarySurface, 16.0f);

    g.setColour(juce::Colours::white.withAlpha(0.015f));
    g.drawHorizontalLine(static_cast<int>(canvas.getY() + 78.0f), canvas.getX() + 10.0f, canvas.getRight() - 10.0f);
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced(16, 14);
    const bool compact = getWidth() < 1380 || getHeight() < 760;
    const int sectionGap = compact ? 10 : 14;
    const int deckAreaH = static_cast<int>(area.getHeight() * (compact ? 0.70f : 0.72f));
    auto deckArea = area.removeFromTop(deckAreaH);

    const int mixerW = juce::jlimit(compact ? 188 : 196, compact ? 224 : 232, deckArea.getWidth() / 5);
    const int deckW  = (deckArea.getWidth() - mixerW - sectionGap * 2) / 2;

    if (deckGUI1)   deckGUI1->setBounds(deckArea.removeFromLeft(deckW));
    deckArea.removeFromLeft(sectionGap);
    auto mixerBounds = deckArea.removeFromLeft(mixerW);
    if (mixerPanel) mixerPanel->setBounds(mixerBounds);
    deckArea.removeFromLeft(sectionGap);
    if (deckGUI2)   deckGUI2->setBounds(deckArea);

    juce::Rectangle<int> recordArea;
    if (mixerPanel)
        recordArea = mixerPanel->getRecordSlotBounds().translated(mixerBounds.getX(), mixerBounds.getY());
    else
        recordArea = juce::Rectangle<int>(mixerBounds.getCentreX() - 76, mixerBounds.getBottom() - 32, 152, 24);

    const int recordGap = compact ? 6 : 8;
    const int recordW = compact ? 66 : 72;
    const int statusW = juce::jmax(44, juce::jmin(compact ? 58 : 72,
                                                  recordArea.getWidth() - recordW - recordGap));
    auto contentArea = recordArea.withSizeKeepingCentre(recordW + recordGap + statusW, recordArea.getHeight());
    recordButton.setBounds(contentArea.removeFromLeft(recordW));
    contentArea.removeFromLeft(recordGap);
    recordStatusLabel.setBounds(contentArea.removeFromLeft(statusW));

    area.removeFromTop(compact ? 10 : 12);
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

void MainComponent::buttonClicked(juce::Button* button)
{
    if (button == &recordButton)
    {
        if (activeWriter_.load() == nullptr)
            startRecording();
        else
            stopRecording();
    }
}

void MainComponent::timerCallback()
{
    updateRecordingUI();
}

void MainComponent::startRecording()
{
    if (activeWriter_.load() != nullptr)
        return;

    auto folder = getProjectRootFromExecutable().getChildFile("Recordings");
    folder.createDirectory();

    auto timestamp = juce::Time::getCurrentTime().formatted("%Y-%m-%d_%H-%M-%S");
    auto file = folder.getChildFile("OtoDecks_Mix_" + timestamp + ".wav");

    juce::WavAudioFormat wavFormat;
    auto fileOutput = file.createOutputStream();
    if (fileOutput == nullptr || !fileOutput->openedOk())
    {
        recordStatusLabel.setText("REC ERROR", juce::dontSendNotification);
        return;
    }
    std::unique_ptr<juce::OutputStream> output(std::move(fileOutput));

    auto options = juce::AudioFormatWriterOptions()
                       .withSampleRate(recordingSampleRate_)
                       .withNumChannels(2)
                       .withBitsPerSample(24);
    auto writer = wavFormat.createWriterFor(output, options);

    if (writer == nullptr)
    {
        recordStatusLabel.setText("REC ERROR", juce::dontSendNotification);
        return;
    }

    threadedWriter_ = std::make_unique<juce::AudioFormatWriter::ThreadedWriter>(writer.release(),
                                                                                 recordingThread_,
                                                                                 32768);
    activeWriter_.store(threadedWriter_.get());
    recordingStartMs_ = juce::Time::currentTimeMillis();
    updateRecordingUI();
}

void MainComponent::stopRecording()
{
    activeWriter_.store(nullptr);
    threadedWriter_.reset();
    recordingStartMs_ = 0;
    updateRecordingUI();
}

void MainComponent::updateRecordingUI()
{
    const bool recording = activeWriter_.load() != nullptr;
    const auto red = CustomLookAndFeel::colour(CustomLookAndFeel::accentRedValue);
    recordButton.setButtonText(recording ? "STOP" : "REC");
    recordButton.setToggleState(recording, juce::dontSendNotification);

    if (!recording)
    {
        recordButton.setColour(juce::TextButton::buttonColourId, red.withAlpha(0.72f));
        recordStatusLabel.setText("READY", juce::dontSendNotification);
        recordStatusLabel.setColour(juce::Label::textColourId,
                                    CustomLookAndFeel::colour(CustomLookAndFeel::mutedTextColourValue));
        return;
    }

    const auto elapsedMs = juce::Time::currentTimeMillis() - recordingStartMs_;
    const int totalSeconds = static_cast<int>(elapsedMs / 1000);
    const int minutes = totalSeconds / 60;
    const int seconds = totalSeconds % 60;
    const bool flashOn = ((elapsedMs / 500) % 2) == 0;
    recordButton.setColour(juce::TextButton::buttonColourId,
                           flashOn ? red.brighter(0.12f)
                                   : red.withAlpha(0.84f));
    recordStatusLabel.setText("REC " + juce::String(minutes) + ":" + juce::String(seconds).paddedLeft('0', 2),
                              juce::dontSendNotification);
    recordStatusLabel.setColour(juce::Label::textColourId,
                                flashOn ? red.brighter(0.25f)
                                        : red.withAlpha(0.86f));
}

void MainComponent::logTrackHistory(int deckNumber, const juce::File& file)
{
    if (!file.existsAsFile())
        return;

    auto historyFile = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                           .getChildFile("OtoDecksTrackHistory.csv");
    const bool needsHeader = !historyFile.existsAsFile() || historyFile.getSize() == 0;

    juce::FileOutputStream out(historyFile);
    if (!out.openedOk())
        return;

    out.setPosition(historyFile.getSize());
    if (needsHeader)
        out << "Timestamp,Deck,Title,File Path\n";

    auto escapeCsv = [](juce::String value)
    {
        value = value.replace("\"", "\"\"");
        return "\"" + value + "\"";
    };

    out << escapeCsv(juce::Time::getCurrentTime().toISO8601(true)) << ","
        << escapeCsv("Deck " + juce::String(deckNumber)) << ","
        << escapeCsv(file.getFileNameWithoutExtension()) << ","
        << escapeCsv(file.getFullPathName()) << "\n";
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

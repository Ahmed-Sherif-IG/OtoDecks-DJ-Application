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
    juce::ColourGradient background(juce::Colour(0xFF040710), bounds.getTopLeft(),
                                    juce::Colour(0xFF08101C), bounds.getBottomLeft(), false);
    g.setGradientFill(background);
    g.fillAll();

    // Deck A (blue) ambient glow
    g.setColour(CustomLookAndFeel::colour(CustomLookAndFeel::accentBlueValue).withAlpha(0.13f));
    g.fillEllipse(-140.0f, -90.0f, 460.0f, 280.0f);
    // Deck B (orange) ambient glow
    g.setColour(CustomLookAndFeel::colour(CustomLookAndFeel::accentOrangeValue).withAlpha(0.11f));
    g.fillEllipse(static_cast<float>(getWidth() - 300), -60.0f, 420.0f, 260.0f);
    // Subtle separator lines
    g.setColour(juce::Colours::white.withAlpha(0.018f));
    g.drawHorizontalLine(82, 20.0f, static_cast<float>(getWidth() - 20));
    g.drawHorizontalLine(static_cast<int>(getHeight() * 0.66f), 20.0f, static_cast<float>(getWidth() - 20));
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced(16, 14);
    const int sectionGap = 14;
    const int deckAreaH = static_cast<int>(area.getHeight() * 0.73f);
    auto deckArea = area.removeFromTop(deckAreaH);

    const int mixerW = juce::jlimit(196, 232, deckArea.getWidth() / 5);
    const int deckW  = (deckArea.getWidth() - mixerW - sectionGap * 2) / 2;

    if (deckGUI1)   deckGUI1->setBounds(deckArea.removeFromLeft(deckW));
    deckArea.removeFromLeft(sectionGap);
    auto mixerBounds = deckArea.removeFromLeft(mixerW);
    if (mixerPanel) mixerPanel->setBounds(mixerBounds);
    deckArea.removeFromLeft(sectionGap);
    if (deckGUI2)   deckGUI2->setBounds(deckArea);

    const int recordW = 72;
    const int statusW = 72;
    const int recordGap = 8;
    auto recordArea = juce::Rectangle<int>(mixerBounds.getCentreX() - (recordW + statusW + recordGap) / 2,
                                           mixerBounds.getBottom() - 50,
                                           recordW + statusW + recordGap,
                                           26);
    recordButton.setBounds(recordArea.removeFromLeft(recordW));
    recordArea.removeFromLeft(recordGap);
    recordStatusLabel.setBounds(recordArea.removeFromLeft(statusW));

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
    recordButton.setButtonText(recording ? "STOP" : "REC");
    recordButton.setColour(juce::TextButton::buttonColourId,
                           recording ? CustomLookAndFeel::colour(CustomLookAndFeel::accentRedValue)
                                     : CustomLookAndFeel::colour(CustomLookAndFeel::accentRedValue).withAlpha(0.72f));

    if (!recording)
    {
        recordStatusLabel.setText("READY", juce::dontSendNotification);
        recordStatusLabel.setColour(juce::Label::textColourId,
                                    CustomLookAndFeel::colour(CustomLookAndFeel::mutedTextColourValue));
        return;
    }

    const auto elapsedMs = juce::Time::currentTimeMillis() - recordingStartMs_;
    const int totalSeconds = static_cast<int>(elapsedMs / 1000);
    const int minutes = totalSeconds / 60;
    const int seconds = totalSeconds % 60;
    recordStatusLabel.setText("REC " + juce::String(minutes) + ":" + juce::String(seconds).paddedLeft('0', 2),
                              juce::dontSendNotification);
    recordStatusLabel.setColour(juce::Label::textColourId,
                                CustomLookAndFeel::colour(CustomLookAndFeel::accentRedValue).brighter(0.2f));
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

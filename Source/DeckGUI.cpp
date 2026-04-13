#include "DeckGUI.h"

static juce::String fmtTime(double totalSecs)
{
    if (totalSecs < 0.0) totalSecs = 0.0;
    int m = static_cast<int>(totalSecs) / 60;
    int s = static_cast<int>(totalSecs) % 60;
    return juce::String(m) + ":" + juce::String(s).paddedLeft('0', 2);
}

//==============================================================================
DeckGUI::DeckGUI(DJAudioPlayer& _player,
                 juce::AudioFormatManager& fmgr,
                 juce::AudioThumbnailCache& cache,
                 const juce::String& deckTitleText,
                 juce::Colour colour)
    : player(_player),
      formatManager_(fmgr),
      waveformDisplay(fmgr, cache),
      deckColour_(colour)
{
    // Title
    deckTitle.setText(deckTitleText, juce::dontSendNotification);
    deckTitle.setFont(juce::Font(juce::FontOptions(16.0f).withStyle("Bold")));
    deckTitle.setColour(juce::Label::textColourId, colour);
    addAndMakeVisible(deckTitle);

    // State / time / BPM labels
    stateLabel.setText("EMPTY", juce::dontSendNotification);
    stateLabel.setFont(juce::Font(juce::FontOptions(11.0f)));
    stateLabel.setColour(juce::Label::textColourId, juce::Colours::grey);
    addAndMakeVisible(stateLabel);

    timeLabel.setText("0:00 / 0:00", juce::dontSendNotification);
    timeLabel.setFont(juce::Font(juce::FontOptions(11.0f)));
    timeLabel.setJustificationType(juce::Justification::centredRight);
    timeLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(timeLabel);

    bpmLabel.setText("BPM: ---", juce::dontSendNotification);
    bpmLabel.setFont(juce::Font(juce::FontOptions(11.0f).withStyle("Bold")));
    bpmLabel.setJustificationType(juce::Justification::centred);
    bpmLabel.setColour(juce::Label::textColourId, colour.brighter(0.3f));
    addAndMakeVisible(bpmLabel);

    // Transport
    for (auto* b : { &playButton, &stopButton, &loadButton, &syncButton })
    { b->addListener(this); addAndMakeVisible(b); }

    // Loop / cue
    for (auto* b : { &loopInButton, &loopOutButton, &clearLoopButton,
                     &setCueButton, &goCueButton })
    { b->addListener(this); addAndMakeVisible(b); }

    // Effects
    for (auto* b : { &lpfButton, &hpfButton })
    { b->addListener(this); addAndMakeVisible(b); }

    // TAP
    tapButton.addListener(this);
    addAndMakeVisible(tapButton);

    // Sliders
    volSlider.setRange(0.0, 1.0);    volSlider.setValue(0.5);
    speedSlider.setRange(0.0, 2.5);  speedSlider.setValue(1.0);
    posSlider.setRange(0.0, 1.0);

    for (auto* s : { &volSlider, &speedSlider, &posSlider })
    {
        s->setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
        s->addListener(this);
        addAndMakeVisible(s);
    }

    volLabel.setText("Vol",   juce::dontSendNotification);
    speedLabel.setText("Spd", juce::dontSendNotification);
    posLabel.setText("Pos",   juce::dontSendNotification);

    for (auto* l : { &volLabel, &speedLabel, &posLabel })
    {
        l->setFont(juce::Font(juce::FontOptions(11.0f)));
        l->setColour(juce::Label::textColourId, juce::Colours::lightgrey);
        addAndMakeVisible(l);
    }

    // Waveform
    waveformDisplay.setWaveformColour(colour);
    waveformDisplay.onSeek = [this](double pos) { player.setPositionRelative(pos); };
    addAndMakeVisible(waveformDisplay);

    // BPM analyser result callback
    bpmAnalyser_.onResult = [this](double bpm)
    {
        player.setBPM(bpm);
        if (bpm > 0.0)
        {
            bpmLabel.setText("BPM: " + juce::String(bpm, 1), juce::dontSendNotification);

            // Build beat grid for waveform overlay
            double len = player.getTotalLength();
            if (len > 0.0)
            {
                double beatInterval = 60.0 / bpm;
                std::vector<double> beats;
                for (double t = 0.0; t < len; t += beatInterval)
                    beats.push_back(t / len);
                waveformDisplay.setBeatPositions(beats);
            }
        }
        else
        {
            bpmLabel.setText("BPM: ???", juce::dontSendNotification);
        }
    };

    startTimerHz(20);
}

DeckGUI::~DeckGUI()
{
    stopTimer();
    bpmAnalyser_.cancel();

    for (auto* b : { &playButton, &stopButton, &loadButton, &syncButton,
                     &loopInButton, &loopOutButton, &clearLoopButton,
                     &setCueButton, &goCueButton,
                     &lpfButton, &hpfButton, &tapButton })
        b->removeListener(this);

    for (auto* s : { &volSlider, &speedSlider, &posSlider })
        s->removeListener(this);
}

//==============================================================================
void DeckGUI::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(20, 20, 20));
    g.setColour(deckColour_.withAlpha(0.35f));
    g.drawRect(getLocalBounds(), 2);
}

void DeckGUI::resized()
{
    auto area   = getLocalBounds().reduced(5);
    int  rowH   = 30;
    int  lbW    = 38;
    int  gap    = 3;

    // Title row
    auto titleRow = area.removeFromTop(rowH);
    deckTitle.setBounds(titleRow);
    area.removeFromTop(gap);

    // Info row: state | BPM | time
    auto infoRow = area.removeFromTop(18);
    stateLabel.setBounds(infoRow.removeFromLeft(60));
    bpmLabel.setBounds(infoRow.removeFromLeft(70));
    timeLabel.setBounds(infoRow);
    area.removeFromTop(gap);

    // Transport: Play | Stop | Sync
    auto tr = area.removeFromTop(rowH);
    {
        int bw = (tr.getWidth() - gap * 2) / 3;
        playButton.setBounds(tr.removeFromLeft(bw)); tr.removeFromLeft(gap);
        stopButton.setBounds(tr.removeFromLeft(bw)); tr.removeFromLeft(gap);
        syncButton.setBounds(tr);
    }
    area.removeFromTop(gap);

    // Vol / Speed / Pos rows
    auto volRow = area.removeFromTop(rowH);
    volLabel.setBounds(volRow.removeFromLeft(lbW));
    volSlider.setBounds(volRow);

    auto spdRow = area.removeFromTop(rowH);
    speedLabel.setBounds(spdRow.removeFromLeft(lbW));
    speedSlider.setBounds(spdRow);

    auto posRow = area.removeFromTop(rowH);
    posLabel.setBounds(posRow.removeFromLeft(lbW));
    posSlider.setBounds(posRow);
    area.removeFromTop(gap);

    // Waveform — give it a healthy chunk
    int waveH = juce::jmin(area.getHeight() - rowH * 3 - gap * 4, rowH * 3);
    waveformDisplay.setBounds(area.removeFromTop(juce::jmax(waveH, rowH)));
    area.removeFromTop(gap);

    // Cue row
    auto cueRow = area.removeFromTop(rowH);
    {
        int bw = (cueRow.getWidth() - gap * 2) / 3;
        setCueButton.setBounds(cueRow.removeFromLeft(bw)); cueRow.removeFromLeft(gap);
        goCueButton.setBounds(cueRow.removeFromLeft(bw));  cueRow.removeFromLeft(gap);
        tapButton.setBounds(cueRow);
    }
    area.removeFromTop(gap);

    // Loop row
    auto loopRow = area.removeFromTop(rowH);
    {
        int bw = (loopRow.getWidth() - gap * 2) / 3;
        loopInButton.setBounds(loopRow.removeFromLeft(bw));   loopRow.removeFromLeft(gap);
        loopOutButton.setBounds(loopRow.removeFromLeft(bw));  loopRow.removeFromLeft(gap);
        clearLoopButton.setBounds(loopRow);
    }
    area.removeFromTop(gap);

    // Effect row + load
    if (area.getHeight() > 0)
    {
        auto effRow = area.removeFromTop(rowH);
        int  bw     = (effRow.getWidth() - gap * 2) / 3;
        lpfButton.setBounds(effRow.removeFromLeft(bw)); effRow.removeFromLeft(gap);
        hpfButton.setBounds(effRow.removeFromLeft(bw)); effRow.removeFromLeft(gap);
        loadButton.setBounds(effRow);
    }
}

//==============================================================================
void DeckGUI::buttonClicked(juce::Button* b)
{
    if      (b == &playButton)      player.start();
    else if (b == &stopButton)      player.stop();
    else if (b == &loadButton)
    {
        fChooser.launchAsync(juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& c)
            {
                auto f = c.getResult();
                if (f.existsAsFile()) loadFile(f);
            });
    }
    else if (b == &syncButton)
    {
        if (getOtherDeckSpeed)
        {
            double myBPM    = player.getBPM();
            double otherBPM = getOtherDeckBPM ? getOtherDeckBPM() : 0.0;

            if (myBPM > 0.0 && otherBPM > 0.0)
            {
                // BPM-based sync: adjust speed ratio so our BPM matches theirs
                double ratio = otherBPM / myBPM;
                ratio = juce::jlimit(0.5, 2.0, ratio);
                double newSpeed = speedSlider.getValue() * ratio;
                speedSlider.setValue(newSpeed);
                player.setSpeed(newSpeed);
            }
            else
            {
                // Fall back to speed-ratio sync
                float otherSpeed = getOtherDeckSpeed();
                speedSlider.setValue(otherSpeed);
                player.setSpeed(otherSpeed);
            }
        }
    }
    // Loop
    else if (b == &loopInButton)    player.setLoopIn();
    else if (b == &loopOutButton)   player.setLoopOut();
    else if (b == &clearLoopButton) player.clearLoop();
    // Cue
    else if (b == &setCueButton)    player.setCuePoint();
    else if (b == &goCueButton)     player.jumpToCue();
    // TAP
    else if (b == &tapButton)       recordTap();
    // M6 effects
    else if (b == &lpfButton)       player.setLowPassEnabled(!player.isLowPassEnabled());
    else if (b == &hpfButton)       player.setHighPassEnabled(!player.isHighPassEnabled());
}

void DeckGUI::sliderValueChanged(juce::Slider* s)
{
    if      (s == &volSlider)   player.setGain(s->getValue());
    else if (s == &speedSlider) player.setSpeed(s->getValue());
    else if (s == &posSlider)   player.setPositionRelative(s->getValue());
}

//==============================================================================
bool DeckGUI::isInterestedInFileDrag(const juce::StringArray&) { return true; }

void DeckGUI::filesDropped(const juce::StringArray& files, int, int)
{
    if (files.size() == 1)
    {
        juce::File f{ files[0] };
        if (f.existsAsFile()) loadFile(f);
    }
}

//==============================================================================
void DeckGUI::timerCallback()
{
    auto state = player.getState();
    double pos = player.getPositionRelative();

    // Position slider
    if (!posSlider.isMouseButtonDown())
        posSlider.setValue(pos, juce::dontSendNotification);

    // Waveform
    waveformDisplay.setPositionRelative(pos);
    waveformDisplay.setCueMarker(state.cuePoint);

    // Loop region
    double len = player.getTotalLength();
    if (state.loopActive && state.loopEndRel > state.loopStartRel)
        waveformDisplay.setLoopRegion(state.loopStartRel, state.loopEndRel, true);
    else
        waveformDisplay.setLoopRegion(0.0, 0.0, false);

    // Loop monitoring
    player.checkAndLoopIfNeeded();

    // Time display
    double elapsed = pos * len;
    timeLabel.setText(fmtTime(elapsed) + " / " + fmtTime(len),
                      juce::dontSendNotification);

    // State indicator
    if (!state.isLoaded)
    {
        stateLabel.setText("EMPTY",   juce::dontSendNotification);
        stateLabel.setColour(juce::Label::textColourId, juce::Colours::dimgrey);
    }
    else if (state.isPlaying)
    {
        stateLabel.setText("PLAYING", juce::dontSendNotification);
        stateLabel.setColour(juce::Label::textColourId, juce::Colours::limegreen);
    }
    else
    {
        stateLabel.setText("STOPPED", juce::dontSendNotification);
        stateLabel.setColour(juce::Label::textColourId, juce::Colours::gold);
    }

    // Button visual state (M6)
    playButton.setAlpha(state.isPlaying ? 1.0f : 0.6f);
    stopButton.setAlpha(!state.isLoaded ? 0.4f : 1.0f);
    lpfButton.setAlpha(player.isLowPassEnabled()  ? 1.0f : 0.5f);
    hpfButton.setAlpha(player.isHighPassEnabled() ? 1.0f : 0.5f);
}

//==============================================================================
void DeckGUI::loadFile(const juce::File& file)
{
    player.loadURL(juce::URL{ file });
    waveformDisplay.loadURL(juce::URL{ file });
    waveformDisplay.setCueMarker(0.0);
    waveformDisplay.setLoopRegion(0.0, 0.0, false);
    deckTitle.setText(file.getFileNameWithoutExtension(), juce::dontSendNotification);
    bpmLabel.setText("BPM: ...", juce::dontSendNotification);
    bpmAnalyser_.analyse(file, formatManager_);
}

void DeckGUI::triggerSync()
{
    buttonClicked(&syncButton);
}

void DeckGUI::setNowPlaying(const juce::String& info)
{
    if (info.isNotEmpty())
        deckTitle.setText(info, juce::dontSendNotification);
}

float DeckGUI::getSpeed() const
{
    return static_cast<float>(speedSlider.getValue());
}

void DeckGUI::recordTap()
{
    double now = juce::Time::getMillisecondCounterHiRes() / 1000.0;
    tapTimes_.push_back(now);

    // Only keep last 8 taps
    if (tapTimes_.size() > 8)
        tapTimes_.erase(tapTimes_.begin());

    if (tapTimes_.size() >= 2)
    {
        double totalInterval = tapTimes_.back() - tapTimes_.front();
        double avgInterval   = totalInterval / static_cast<double>(tapTimes_.size() - 1);
        double tappedBPM     = 60.0 / avgInterval;

        // Fold into [60,180]
        while (tappedBPM < 60.0)  tappedBPM *= 2.0;
        while (tappedBPM > 180.0) tappedBPM /= 2.0;

        tappedBPM = std::round(tappedBPM * 10.0) / 10.0;
        player.setBPM(tappedBPM);
        bpmLabel.setText("BPM: " + juce::String(tappedBPM, 1),
                          juce::dontSendNotification);
    }
}

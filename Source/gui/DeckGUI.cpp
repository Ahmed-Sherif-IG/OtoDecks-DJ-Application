#include "DeckGUI.h"
#include "../shared/CustomLookAndFeel.h"

static juce::String fmtTime(double totalSecs)
{
    if (totalSecs < 0.0) totalSecs = 0.0;
    const int m = static_cast<int>(totalSecs) / 60;
    const int s = static_cast<int>(totalSecs) % 60;
    return juce::String(m) + ":" + juce::String(s).paddedLeft('0', 2);
}

static void styleDeckButton(juce::TextButton& button, juce::Colour colour)
{
    button.setColour(juce::TextButton::buttonColourId, colour);
    button.setColour(juce::TextButton::buttonOnColourId, colour.brighter(0.15f));
    button.setColour(juce::TextButton::textColourOffId, CustomLookAndFeel::colour(CustomLookAndFeel::textColourValue));
    button.setClickingTogglesState(false);
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
    deckTitle.setText(deckTitleText, juce::dontSendNotification);
    deckTitle.setFont(juce::Font(juce::FontOptions(18.0f).withStyle("Bold")));
    deckTitle.setColour(juce::Label::textColourId, CustomLookAndFeel::colour(CustomLookAndFeel::textColourValue));
    deckTitle.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(deckTitle);

    stateLabel.setText("READY", juce::dontSendNotification);
    stateLabel.setFont(juce::Font(juce::FontOptions(10.5f).withStyle("Bold")));
    stateLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(stateLabel);

    timeLabel.setText("0:00 / 0:00", juce::dontSendNotification);
    timeLabel.setFont(juce::Font(juce::FontOptions(11.0f)));
    timeLabel.setJustificationType(juce::Justification::centredRight);
    timeLabel.setColour(juce::Label::textColourId, CustomLookAndFeel::colour(CustomLookAndFeel::mutedTextColourValue));
    addAndMakeVisible(timeLabel);

    bpmLabel.setText("BPM: ---", juce::dontSendNotification);
    bpmLabel.setFont(juce::Font(juce::FontOptions(11.0f).withStyle("Bold")));
    bpmLabel.setJustificationType(juce::Justification::centred);
    bpmLabel.setColour(juce::Label::textColourId, colour.brighter(0.2f));
    addAndMakeVisible(bpmLabel);

    styleDeckButton(playButton, colour.brighter(0.1f));
    styleDeckButton(stopButton, CustomLookAndFeel::colour(CustomLookAndFeel::panelAltColourValue).brighter(0.12f));
    styleDeckButton(loadButton, CustomLookAndFeel::colour(CustomLookAndFeel::panelAltColourValue).brighter(0.05f));
    styleDeckButton(syncButton, colour.withAlpha(0.85f));
    styleDeckButton(loopInButton, colour.darker(0.18f));
    styleDeckButton(loopOutButton, colour.darker(0.10f));
    styleDeckButton(clearLoopButton, CustomLookAndFeel::colour(CustomLookAndFeel::panelAltColourValue));
    styleDeckButton(setCueButton, CustomLookAndFeel::colour(CustomLookAndFeel::panelAltColourValue));
    styleDeckButton(goCueButton, colour.withAlpha(0.75f));
    styleDeckButton(tapButton, colour.withAlpha(0.82f));
    styleDeckButton(lpfButton, CustomLookAndFeel::colour(CustomLookAndFeel::panelAltColourValue));
    styleDeckButton(hpfButton, CustomLookAndFeel::colour(CustomLookAndFeel::panelAltColourValue));

    for (auto* b : { &playButton, &stopButton, &loadButton, &syncButton,
                     &loopInButton, &loopOutButton, &clearLoopButton,
                     &setCueButton, &goCueButton,
                     &lpfButton, &hpfButton })
    {
        b->addListener(this);
        addAndMakeVisible(b);
    }

    tapButton.addListener(this);
    addAndMakeVisible(tapButton);

    volSlider.setRange(0.0, 1.0);    volSlider.setValue(0.5);
    speedSlider.setRange(0.0, 2.5);  speedSlider.setValue(1.0);
    posSlider.setRange(0.0, 1.0);

    volSlider.setColour(juce::Slider::trackColourId, colour.withAlpha(0.95f));
    speedSlider.setColour(juce::Slider::trackColourId, colour.brighter(0.2f));
    posSlider.setColour(juce::Slider::trackColourId, CustomLookAndFeel::colour(CustomLookAndFeel::textColourValue).withAlpha(0.9f));

    for (auto* s : { &volSlider, &speedSlider, &posSlider })
    {
        s->setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
        s->addListener(this);
        addAndMakeVisible(s);
    }

    volLabel.setText("GAIN",   juce::dontSendNotification);
    speedLabel.setText("SPEED", juce::dontSendNotification);
    posLabel.setText("SEEK",   juce::dontSendNotification);

    for (auto* l : { &volLabel, &speedLabel, &posLabel })
    {
        l->setFont(juce::Font(juce::FontOptions(10.5f).withStyle("Bold")));
        l->setColour(juce::Label::textColourId, CustomLookAndFeel::colour(CustomLookAndFeel::mutedTextColourValue));
        addAndMakeVisible(l);
    }

    waveformDisplay.setWaveformColour(colour);
    waveformDisplay.onSeek = [this](double pos) { player.setPositionRelative(pos); };
    addAndMakeVisible(waveformDisplay);

    bpmAnalyser_.onResult = [this](double bpm)
    {
        player.setBPM(bpm);
        if (bpm > 0.0)
        {
            bpmLabel.setText("BPM: " + juce::String(bpm, 1), juce::dontSendNotification);

            const double len = player.getTotalLength();
            if (len > 0.0)
            {
                const double beatInterval = 60.0 / bpm;
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
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);
    juce::ColourGradient background(deckColour_.withAlpha(0.18f), bounds.getTopLeft(),
                                    CustomLookAndFeel::colour(CustomLookAndFeel::panelColourValue), bounds.getBottomLeft(), false);
    g.setGradientFill(background);
    g.fillRoundedRectangle(bounds, 18.0f);

    g.setColour(CustomLookAndFeel::colour(CustomLookAndFeel::outlineColourValue).withAlpha(0.95f));
    g.drawRoundedRectangle(bounds, 18.0f, 1.2f);

    auto accent = bounds.removeFromTop(6.0f).reduced(14.0f, 0.0f);
    g.setColour(deckColour_.withAlpha(0.95f));
    g.fillRoundedRectangle(accent, 3.0f);

    juce::ColourGradient gloss(juce::Colours::white.withAlpha(0.06f), getLocalBounds().toFloat().getTopLeft(),
                               juce::Colours::transparentBlack, getLocalBounds().toFloat().getCentre(), false);
    g.setGradientFill(gloss);
    g.fillRoundedRectangle(getLocalBounds().toFloat().reduced(1.0f), 18.0f);
}

void DeckGUI::resized()
{
    auto area = getLocalBounds().reduced(12);
    const int gap = 6;
    const int rowH = 32;
    const int labelW = 50;

    auto titleRow = area.removeFromTop(34);
    auto statePill = titleRow.removeFromLeft(84);
    stateLabel.setBounds(statePill.reduced(0, 4));
    titleRow.removeFromLeft(10);
    deckTitle.setBounds(titleRow.removeFromLeft(static_cast<int>(titleRow.getWidth() * 0.52f)));
    bpmLabel.setBounds(titleRow.removeFromLeft(100));
    timeLabel.setBounds(titleRow);

    area.removeFromTop(gap);

    auto transportRow = area.removeFromTop(rowH);
    const int transportGap = 6;
    const int transportButtonWidth = (transportRow.getWidth() - transportGap * 3) / 4;
    playButton.setBounds(transportRow.removeFromLeft(transportButtonWidth)); transportRow.removeFromLeft(transportGap);
    stopButton.setBounds(transportRow.removeFromLeft(transportButtonWidth)); transportRow.removeFromLeft(transportGap);
    loadButton.setBounds(transportRow.removeFromLeft(transportButtonWidth)); transportRow.removeFromLeft(transportGap);
    syncButton.setBounds(transportRow);

    area.removeFromTop(gap);

    auto sliderRow = [&](juce::Label& label, juce::Slider& slider)
    {
        auto row = area.removeFromTop(rowH);
        label.setBounds(row.removeFromLeft(labelW));
        slider.setBounds(row.reduced(4, 4));
        area.removeFromTop(gap - 1);
    };

    sliderRow(volLabel, volSlider);
    sliderRow(speedLabel, speedSlider);
    sliderRow(posLabel, posSlider);

    const int lowerSectionReserved = rowH * 3 + gap * 3;
    const int waveformHeight = juce::jmax(130, area.getHeight() - lowerSectionReserved);
    waveformDisplay.setBounds(area.removeFromTop(waveformHeight));

    area.removeFromTop(gap);

    auto cueRow = area.removeFromTop(rowH);
    const int buttonGap = 6;
    const int cueButtonW = (cueRow.getWidth() - buttonGap * 2) / 3;
    setCueButton.setBounds(cueRow.removeFromLeft(cueButtonW)); cueRow.removeFromLeft(buttonGap);
    goCueButton.setBounds(cueRow.removeFromLeft(cueButtonW));  cueRow.removeFromLeft(buttonGap);
    tapButton.setBounds(cueRow);

    area.removeFromTop(gap);

    auto loopRow = area.removeFromTop(rowH);
    const int loopButtonW = (loopRow.getWidth() - buttonGap * 2) / 3;
    loopInButton.setBounds(loopRow.removeFromLeft(loopButtonW)); loopRow.removeFromLeft(buttonGap);
    loopOutButton.setBounds(loopRow.removeFromLeft(loopButtonW)); loopRow.removeFromLeft(buttonGap);
    clearLoopButton.setBounds(loopRow);

    area.removeFromTop(gap);

    auto fxRow = area.removeFromTop(rowH);
    const int fxButtonW = (fxRow.getWidth() - buttonGap) / 2;
    lpfButton.setBounds(fxRow.removeFromLeft(fxButtonW)); fxRow.removeFromLeft(buttonGap);
    hpfButton.setBounds(fxRow.removeFromLeft(fxButtonW));
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
            const double myBPM    = player.getBPM();
            const double otherBPM = getOtherDeckBPM ? getOtherDeckBPM() : 0.0;

            if (myBPM > 0.0 && otherBPM > 0.0)
            {
                double ratio = otherBPM / myBPM;
                ratio = juce::jlimit(0.5, 2.0, ratio);
                const double newSpeed = speedSlider.getValue() * ratio;
                speedSlider.setValue(newSpeed);
                player.setSpeed(newSpeed);
            }
            else
            {
                const float otherSpeed = getOtherDeckSpeed();
                speedSlider.setValue(otherSpeed);
                player.setSpeed(otherSpeed);
            }
        }
    }
    else if (b == &loopInButton)    player.setLoopIn();
    else if (b == &loopOutButton)   player.setLoopOut();
    else if (b == &clearLoopButton) player.clearLoop();
    else if (b == &setCueButton)    player.setCuePoint();
    else if (b == &goCueButton)     player.jumpToCue();
    else if (b == &tapButton)       recordTap();
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
    const auto state = player.getState();
    const double pos = player.getPositionRelative();

    if (!posSlider.isMouseButtonDown())
        posSlider.setValue(pos, juce::dontSendNotification);

    waveformDisplay.setPositionRelative(pos);
    waveformDisplay.setCueMarker(state.cuePoint);

    const double len = player.getTotalLength();
    if (state.loopActive && state.loopEndRel > state.loopStartRel)
        waveformDisplay.setLoopRegion(state.loopStartRel, state.loopEndRel, true);
    else
        waveformDisplay.setLoopRegion(0.0, 0.0, false);

    player.checkAndLoopIfNeeded();

    const double elapsed = pos * len;
    timeLabel.setText(fmtTime(elapsed) + " / " + fmtTime(len), juce::dontSendNotification);

    juce::Colour stateColour = CustomLookAndFeel::colour(CustomLookAndFeel::mutedTextColourValue);
    juce::String stateText = "READY";

    if (!state.isLoaded)
    {
        stateText = "EMPTY";
        stateColour = CustomLookAndFeel::colour(CustomLookAndFeel::accentRedValue).withAlpha(0.8f);
    }
    else if (state.isPlaying)
    {
        stateText = "LIVE";
        stateColour = CustomLookAndFeel::colour(CustomLookAndFeel::accentGreenValue);
    }
    else
    {
        stateText = "CUE";
        stateColour = deckColour_.brighter(0.15f);
    }

    stateLabel.setText(stateText, juce::dontSendNotification);
    stateLabel.setColour(juce::Label::textColourId, stateColour);

    playButton.setButtonText(state.isPlaying ? "Playing" : "Play");
    playButton.setToggleState(state.isPlaying, juce::dontSendNotification);
    playButton.setColour(juce::TextButton::buttonColourId,
                         state.isPlaying ? deckColour_.brighter(0.1f) : deckColour_.withAlpha(0.9f));
    stopButton.setEnabled(state.isLoaded);
    stopButton.setAlpha(!state.isLoaded ? 0.4f : 0.95f);

    lpfButton.setToggleState(player.isLowPassEnabled(), juce::dontSendNotification);
    hpfButton.setToggleState(player.isHighPassEnabled(), juce::dontSendNotification);
    lpfButton.setColour(juce::TextButton::buttonColourId,
                        player.isLowPassEnabled() ? deckColour_.withAlpha(0.85f)
                                                  : CustomLookAndFeel::colour(CustomLookAndFeel::panelAltColourValue));
    hpfButton.setColour(juce::TextButton::buttonColourId,
                        player.isHighPassEnabled() ? deckColour_.withAlpha(0.85f)
                                                   : CustomLookAndFeel::colour(CustomLookAndFeel::panelAltColourValue));
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
    const double now = juce::Time::getMillisecondCounterHiRes() / 1000.0;
    tapTimes_.push_back(now);

    if (tapTimes_.size() > 8)
        tapTimes_.erase(tapTimes_.begin());

    if (tapTimes_.size() >= 2)
    {
        const double totalInterval = tapTimes_.back() - tapTimes_.front();
        const double avgInterval   = totalInterval / static_cast<double>(tapTimes_.size() - 1);
        double tappedBPM           = 60.0 / avgInterval;

        while (tappedBPM < 60.0)  tappedBPM *= 2.0;
        while (tappedBPM > 180.0) tappedBPM /= 2.0;

        tappedBPM = std::round(tappedBPM * 10.0) / 10.0;
        player.setBPM(tappedBPM);
        bpmLabel.setText("BPM: " + juce::String(tappedBPM, 1), juce::dontSendNotification);
    }
}

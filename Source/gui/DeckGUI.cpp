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
    button.setColour(juce::TextButton::textColourOnId, CustomLookAndFeel::colour(CustomLookAndFeel::textColourValue));
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
    deckTitle.setFont(juce::Font(juce::FontOptions(22.0f).withStyle("Bold")));
    deckTitle.setColour(juce::Label::textColourId, CustomLookAndFeel::colour(CustomLookAndFeel::textColourValue));
    deckTitle.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(deckTitle);

    stateLabel.setText("READY", juce::dontSendNotification);
    stateLabel.setFont(juce::Font(juce::FontOptions(10.5f).withStyle("Bold")));
    stateLabel.setJustificationType(juce::Justification::centred);
    stateLabel.setColour(juce::Label::backgroundColourId, CustomLookAndFeel::colour(CustomLookAndFeel::panelRaisedColourValue));
    addAndMakeVisible(stateLabel);

    timeLabel.setText("0:00 / 0:00", juce::dontSendNotification);
    timeLabel.setFont(juce::Font(juce::FontOptions(14.0f).withStyle("Bold")));
    timeLabel.setJustificationType(juce::Justification::centredRight);
    timeLabel.setColour(juce::Label::textColourId, CustomLookAndFeel::colour(CustomLookAndFeel::textColourValue));
    addAndMakeVisible(timeLabel);

    bpmLabel.setText("BPM: ---", juce::dontSendNotification);
    bpmLabel.setFont(juce::Font(juce::FontOptions(13.0f).withStyle("Bold")));
    bpmLabel.setJustificationType(juce::Justification::centred);
    bpmLabel.setColour(juce::Label::textColourId, colour.brighter(0.3f));
    addAndMakeVisible(bpmLabel);

    playButton.setButtonText("PLAY");
    stopButton.setButtonText("PAUSE");
    loadButton.setButtonText("LOAD TRACK");
    syncButton.setButtonText("BEAT SYNC");
    setCueButton.setButtonText("SET CUE");
    goCueButton.setButtonText("JUMP TO CUE");
    loopInButton.setButtonText("LOOP START");
    loopOutButton.setButtonText("LOOP END");
    clearLoopButton.setButtonText("CLEAR LOOP");
    tapButton.setButtonText("TAP BPM");
    lpfButton.setButtonText("LOW CUT");
    hpfButton.setButtonText("HIGH CUT");

    styleDeckButton(playButton, colour.brighter(0.1f));
    styleDeckButton(stopButton, CustomLookAndFeel::colour(CustomLookAndFeel::panelRaisedColourValue));
    styleDeckButton(loadButton, CustomLookAndFeel::colour(CustomLookAndFeel::panelRaisedColourValue).brighter(0.05f));
    styleDeckButton(syncButton, colour.withAlpha(0.85f));
    styleDeckButton(loopInButton, colour.darker(0.18f));
    styleDeckButton(loopOutButton, colour.darker(0.10f));
    styleDeckButton(clearLoopButton, CustomLookAndFeel::colour(CustomLookAndFeel::panelRaisedColourValue));

    for (auto* lb : { &loopBar1_4Button, &loopBar1_2Button, &loopBar1Button,
                      &loopBar2Button,   &loopBar4Button,   &loopBar8Button })
    {
        styleDeckButton(*lb, colour.withAlpha(0.62f));
        lb->addListener(this);
        addAndMakeVisible(lb);
    }
    styleDeckButton(setCueButton, CustomLookAndFeel::colour(CustomLookAndFeel::panelRaisedColourValue));
    styleDeckButton(goCueButton, colour.withAlpha(0.75f));
    styleDeckButton(tapButton, colour.withAlpha(0.82f));
    styleDeckButton(lpfButton, CustomLookAndFeel::colour(CustomLookAndFeel::panelRaisedColourValue));
    styleDeckButton(hpfButton, CustomLookAndFeel::colour(CustomLookAndFeel::panelRaisedColourValue));
    styleDeckButton(resetVolumeButton, CustomLookAndFeel::colour(CustomLookAndFeel::panelRaisedColourValue).darker(0.05f));
    styleDeckButton(resetSpeedButton, CustomLookAndFeel::colour(CustomLookAndFeel::panelRaisedColourValue).darker(0.05f));

    for (auto* b : { &playButton, &stopButton, &loadButton, &syncButton,
                     &loopInButton, &loopOutButton, &clearLoopButton,
                     &setCueButton, &goCueButton,
                     &lpfButton, &hpfButton,
                     &resetVolumeButton, &resetSpeedButton })
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

    volLabel.setText("VOLUME",   juce::dontSendNotification);
    speedLabel.setText("SPEED",   juce::dontSendNotification);
    posLabel.setText("POSITION", juce::dontSendNotification);

    for (auto* l : { &volLabel, &speedLabel, &posLabel })
    {
        l->setFont(juce::Font(juce::FontOptions(10.5f).withStyle("Bold")));
        l->setColour(juce::Label::textColourId, CustomLookAndFeel::colour(CustomLookAndFeel::mutedTextColourValue));
        addAndMakeVisible(l);
    }

    waveformDisplay.setWaveformColour(colour);
    waveformDisplay.onSeek = [this](double pos) { player.setPositionRelative(pos); };
    addAndMakeVisible(waveformDisplay);

    // Hotcue pads
    for (int i = 0; i < kNumHotcues; ++i)
    {
        hotcuePads[static_cast<size_t>(i)].setButtonText(juce::String(i + 1));
        hotcuePads[static_cast<size_t>(i)].setColour(juce::TextButton::buttonColourId,
                                                      CustomLookAndFeel::colour(CustomLookAndFeel::panelRaisedColourValue));
        hotcuePads[static_cast<size_t>(i)].setColour(juce::TextButton::textColourOffId,
                                                      CustomLookAndFeel::colour(CustomLookAndFeel::mutedTextColourValue));
        hotcuePads[static_cast<size_t>(i)].addListener(this);
        addAndMakeVisible(hotcuePads[static_cast<size_t>(i)]);
    }

    // Clear all hotcues button
    clearHotcuesButton.setColour(juce::TextButton::buttonColourId,
                                 CustomLookAndFeel::colour(CustomLookAndFeel::accentRedValue).withAlpha(0.55f));
    clearHotcuesButton.setColour(juce::TextButton::textColourOffId,
                                 CustomLookAndFeel::colour(CustomLookAndFeel::textColourValue));
    clearHotcuesButton.addListener(this);
    addAndMakeVisible(clearHotcuesButton);

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
                     &lpfButton, &hpfButton, &tapButton,
                     &resetVolumeButton, &resetSpeedButton })
        b->removeListener(this);

    for (auto* lb : { &loopBar1_4Button, &loopBar1_2Button, &loopBar1Button,
                      &loopBar2Button,   &loopBar4Button,   &loopBar8Button })
        lb->removeListener(this);

    for (auto& pad : hotcuePads)
        pad.removeListener(this);
    clearHotcuesButton.removeListener(this);

    for (auto* s : { &volSlider, &speedSlider, &posSlider })
        s->removeListener(this);
}

//==============================================================================
void DeckGUI::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);

    juce::DropShadow shadow(juce::Colours::black.withAlpha(0.45f), 18, { 0, 8 });
    shadow.drawForRectangle(g, bounds.toNearestInt());

    juce::ColourGradient background(deckColour_.withAlpha(0.14f), bounds.getTopLeft(),
                                    CustomLookAndFeel::colour(CustomLookAndFeel::panelColourValue), bounds.getBottomLeft(), false);
    g.setGradientFill(background);
    g.fillRoundedRectangle(bounds, 20.0f);

    auto inner = bounds.reduced(1.5f);
    juce::ColourGradient surface(CustomLookAndFeel::colour(CustomLookAndFeel::panelAltColourValue).brighter(0.10f), inner.getTopLeft(),
                                 CustomLookAndFeel::colour(CustomLookAndFeel::panelColourValue).darker(0.04f), inner.getBottomLeft(), false);
    g.setGradientFill(surface);
    g.fillRoundedRectangle(inner, 18.0f);

    auto glow = inner.reduced(6.0f);
    g.setColour(getDeckGlowColour());
    g.drawRoundedRectangle(glow, 16.0f, 1.4f);

    g.setColour(CustomLookAndFeel::colour(CustomLookAndFeel::outlineColourValue).withAlpha(0.95f));
    g.drawRoundedRectangle(inner, 18.0f, 1.1f);

    auto accent = inner.removeFromTop(5.0f).reduced(16.0f, 0.0f);
    juce::ColourGradient accentFill(deckColour_.brighter(0.35f), accent.getTopLeft(),
                                    deckColour_.darker(0.1f), accent.getTopRight(), false);
    g.setGradientFill(accentFill);
    g.fillRoundedRectangle(accent, 2.5f);

    juce::ColourGradient gloss(juce::Colours::white.withAlpha(0.075f), bounds.getTopLeft(),
                               juce::Colours::transparentBlack, bounds.getCentre(), false);
    g.setGradientFill(gloss);
    g.fillRoundedRectangle(bounds, 20.0f);

    // Section panel backgrounds for visual grouping
    auto sectionBounds = getLocalBounds().toFloat().reduced(14.0f);
    sectionBounds.removeFromTop(42.0f + 10.0f); // title row + gap

    // Transport section panel
    auto transportPanel = sectionBounds.removeFromTop(34.0f).expanded(6.0f, 4.0f);
    g.setColour(juce::Colours::white.withAlpha(0.032f));
    g.fillRoundedRectangle(transportPanel, 10.0f);
    g.setColour(CustomLookAndFeel::colour(CustomLookAndFeel::outlineColourValue).withAlpha(0.28f));
    g.drawRoundedRectangle(transportPanel, 10.0f, 0.8f);

    sectionBounds.removeFromTop(10.0f); // gap

    // Controls section panel (sliders)
    auto controlsPanel = sectionBounds.removeFromTop(118.0f).expanded(6.0f, 4.0f);
    g.setColour(juce::Colours::white.withAlpha(0.022f));
    g.fillRoundedRectangle(controlsPanel, 10.0f);

    sectionBounds.removeFromTop(10.0f); // gap after controls

    // Hotcue pads row
    sectionBounds.removeFromTop(26.0f + 10.0f); // hotcue + gap

    // Skip waveform, then effects section
    const int bottomControlHeight = 130;
    const int waveformH = juce::jmax(120, static_cast<int>(sectionBounds.getHeight()) - bottomControlHeight - 10);
    sectionBounds.removeFromTop(static_cast<float>(waveformH) + 10.0f); // waveform + gap

    // Effects / cue / loop section panel
    auto effectsPanel = sectionBounds.expanded(6.0f, 4.0f);
    g.setColour(juce::Colours::white.withAlpha(0.028f));
    g.fillRoundedRectangle(effectsPanel, 10.0f);
    g.setColour(CustomLookAndFeel::colour(CustomLookAndFeel::outlineColourValue).withAlpha(0.22f));
    g.drawRoundedRectangle(effectsPanel, 10.0f, 0.8f);
}

void DeckGUI::resized()
{
    auto area = getLocalBounds().reduced(14);
    const int gap = 10;
    const int labelW = 74;

    auto titleRow = area.removeFromTop(38);
    auto statePill = titleRow.removeFromLeft(64);
    stateLabel.setBounds(statePill.reduced(0, 6));
    titleRow.removeFromLeft(8);
    auto rightInfo = titleRow.removeFromRight(190);
    deckTitle.setBounds(titleRow);
    bpmLabel.setBounds(rightInfo.removeFromLeft(90));
    timeLabel.setBounds(rightInfo);

    area.removeFromTop(gap);

    auto transportRow = area.removeFromTop(34);
    const int transportGap = 8;
    const int transportButtonWidth = (transportRow.getWidth() - transportGap * 3) / 4;
    playButton.setBounds(transportRow.removeFromLeft(transportButtonWidth)); transportRow.removeFromLeft(transportGap);
    stopButton.setBounds(transportRow.removeFromLeft(transportButtonWidth)); transportRow.removeFromLeft(transportGap);
    loadButton.setBounds(transportRow.removeFromLeft(transportButtonWidth)); transportRow.removeFromLeft(transportGap);
    syncButton.setBounds(transportRow);

    area.removeFromTop(gap);

    auto controlsRow = area.removeFromTop(118);
    const int sliderRowH = 28;
    const int sliderGap = 10;

    auto layoutSliderRow = [&](juce::Label& label, juce::Slider& slider, juce::TextButton* resetButton)
    {
        auto row = controlsRow.removeFromTop(sliderRowH);
        label.setBounds(row.removeFromLeft(labelW));
        if (resetButton != nullptr)
        {
            auto resetArea = row.removeFromRight(86);
            resetButton->setBounds(resetArea.reduced(0, 2));
            row.removeFromRight(8);
        }
        slider.setBounds(row.reduced(4, 4));
        controlsRow.removeFromTop(sliderGap);
    };

    layoutSliderRow(volLabel, volSlider, &resetVolumeButton);
    layoutSliderRow(speedLabel, speedSlider, &resetSpeedButton);
    layoutSliderRow(posLabel, posSlider, nullptr);

    area.removeFromTop(gap);

    // Hotcue pads row (8 pads + CLR button)
    auto hotcueRow = area.removeFromTop(26);
    const int padGap = 3;
    const int clrW   = 34;
    const int padsW  = hotcueRow.getWidth() - clrW - padGap;
    const int padW   = (padsW - padGap * 7) / 8;
    for (int i = 0; i < kNumHotcues; ++i)
    {
        hotcuePads[static_cast<size_t>(i)].setBounds(hotcueRow.removeFromLeft(padW));
        if (i < kNumHotcues - 1) hotcueRow.removeFromLeft(padGap);
    }
    hotcueRow.removeFromLeft(padGap);
    clearHotcuesButton.setBounds(hotcueRow);

    area.removeFromTop(gap);

    const int bottomControlHeight = 130;
    const int waveformHeight = juce::jmax(120, area.getHeight() - bottomControlHeight - gap);
    waveformDisplay.setBounds(area.removeFromTop(waveformHeight));

    area.removeFromTop(gap);

    auto bottomControls = area.removeFromTop(bottomControlHeight).reduced(0, 2);
    auto topActionRow = bottomControls.removeFromTop(28);
    const int buttonGap = 8;
    const int actionButtonW = (topActionRow.getWidth() - buttonGap * 2) / 3;
    setCueButton.setBounds(topActionRow.removeFromLeft(actionButtonW)); topActionRow.removeFromLeft(buttonGap);
    goCueButton.setBounds(topActionRow.removeFromLeft(actionButtonW));  topActionRow.removeFromLeft(buttonGap);
    tapButton.setBounds(topActionRow);

    bottomControls.removeFromTop(8);

    auto secondActionRow = bottomControls.removeFromTop(24);
    const int secondButtonW = (secondActionRow.getWidth() - buttonGap * 2) / 3;
    loopInButton.setBounds(secondActionRow.removeFromLeft(secondButtonW)); secondActionRow.removeFromLeft(buttonGap);
    loopOutButton.setBounds(secondActionRow.removeFromLeft(secondButtonW)); secondActionRow.removeFromLeft(buttonGap);
    clearLoopButton.setBounds(secondActionRow);

    bottomControls.removeFromTop(6);

    // Loop bar-length buttons row
    auto barRow = bottomControls.removeFromTop(22);
    const int barGap = 4;
    const int barW = (barRow.getWidth() - barGap * 5) / 6;
    for (auto* lb : { &loopBar1_4Button, &loopBar1_2Button, &loopBar1Button,
                      &loopBar2Button,   &loopBar4Button,   &loopBar8Button })
    {
        lb->setBounds(barRow.removeFromLeft(barW));
        if (lb != &loopBar8Button) barRow.removeFromLeft(barGap);
    }

    bottomControls.removeFromTop(6);

    auto filterRow = bottomControls.removeFromTop(24);
    const int filterButtonW = (filterRow.getWidth() - buttonGap) / 2;
    lpfButton.setBounds(filterRow.removeFromLeft(filterButtonW)); filterRow.removeFromLeft(buttonGap);
    hpfButton.setBounds(filterRow.removeFromLeft(filterButtonW));
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
    else if (b == &loopBar1_4Button || b == &loopBar1_2Button || b == &loopBar1Button
          || b == &loopBar2Button   || b == &loopBar4Button   || b == &loopBar8Button)
    {
        const double bpm = player.getBPM();
        if (bpm > 0.0)
        {
            double bars = 1.0;
            if      (b == &loopBar1_4Button) bars = 0.25;
            else if (b == &loopBar1_2Button) bars = 0.5;
            else if (b == &loopBar2Button)   bars = 2.0;
            else if (b == &loopBar4Button)   bars = 4.0;
            else if (b == &loopBar8Button)   bars = 8.0;

            const double secondsPerBar = (60.0 / bpm) * 4.0;  // 4/4 time
            player.setLoopFromCurrentPosition(bars * secondsPerBar);
        }
    }
    else if (b == &setCueButton)      player.setCuePoint();
    else if (b == &goCueButton)       player.jumpToCue();
    else if (b == &tapButton)         recordTap();
    else if (b == &resetVolumeButton) resetVolumeToDefault();
    else if (b == &resetSpeedButton)  resetSpeedToDefault();
    else if (b == &lpfButton)            player.setLowPassEnabled(!player.isLowPassEnabled());
    else if (b == &hpfButton)            player.setHighPassEnabled(!player.isHighPassEnabled());
    else if (b == &clearHotcuesButton)   clearAllHotcues();

    // Hotcue pads (standalone check — not part of the if-else chain)
    for (int i = 0; i < kNumHotcues; ++i)
    {
        if (b == &hotcuePads[static_cast<size_t>(i)])
        {
            if (juce::ModifierKeys::getCurrentModifiers().isShiftDown())
                player.clearHotcue(i);
            else if (player.isHotcueSet(i))
                player.jumpToHotcue(i);
            else
                player.setHotcue(i);

            updateHotcuePadColors();
            break;
        }
    }
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
    stateLabel.setColour(juce::Label::backgroundColourId,
                         CustomLookAndFeel::colour(CustomLookAndFeel::panelRaisedColourValue).interpolatedWith(stateColour.withAlpha(0.18f), 0.35f));

    playButton.setButtonText(state.isPlaying ? "PLAYING" : "PLAY");
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

juce::Colour DeckGUI::getDeckGlowColour() const
{
    const auto state = player.getState();
    auto glow = deckColour_.withAlpha(state.isPlaying ? 0.55f : 0.22f);

    if (!state.isLoaded)
        glow = CustomLookAndFeel::colour(CustomLookAndFeel::outlineColourValue).withAlpha(0.45f);
    else if (state.loopActive)
        glow = glow.brighter(0.22f);

    return glow;
}

//==============================================================================
void DeckGUI::loadFile(const juce::File& file)
{
    player.loadURL(juce::URL{ file });
    waveformDisplay.loadURL(juce::URL{ file });
    waveformDisplay.setCueMarker(0.0);
    waveformDisplay.setLoopRegion(0.0, 0.0, false);
    waveformDisplay.setTotalDuration(player.getTotalLength());
    deckTitle.setText(file.getFileNameWithoutExtension(), juce::dontSendNotification);
    bpmLabel.setText("BPM: ...", juce::dontSendNotification);
    bpmAnalyser_.analyse(file, formatManager_);

    // Auto-clear all hotcues when a new track is loaded
    clearAllHotcues();
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

void DeckGUI::resetVolumeToDefault()
{
    volSlider.setValue(0.5, juce::sendNotificationSync);
}

void DeckGUI::resetSpeedToDefault()
{
    speedSlider.setValue(1.0, juce::sendNotificationSync);
}

void DeckGUI::clearAllHotcues()
{
    for (int i = 0; i < kNumHotcues; ++i)
        player.clearHotcue(i);
    updateHotcuePadColors();
}

void DeckGUI::updateHotcuePadColors()
{
    // Cycle through 4 accent colours for the 8 pads
    const juce::Colour setColours[4] = {
        deckColour_.brighter(0.2f),
        CustomLookAndFeel::colour(CustomLookAndFeel::accentGreenValue).withAlpha(0.85f),
        CustomLookAndFeel::colour(CustomLookAndFeel::accentOrangeValue).withAlpha(0.85f),
        CustomLookAndFeel::colour(CustomLookAndFeel::accentRedValue).withAlpha(0.80f)
    };

    for (int i = 0; i < kNumHotcues; ++i)
    {
        const bool isSet = player.isHotcueSet(i);
        hotcuePads[static_cast<size_t>(i)].setColour(
            juce::TextButton::buttonColourId,
            isSet ? setColours[i % 4]
                  : CustomLookAndFeel::colour(CustomLookAndFeel::panelRaisedColourValue));
        hotcuePads[static_cast<size_t>(i)].setColour(
            juce::TextButton::textColourOffId,
            isSet ? CustomLookAndFeel::colour(CustomLookAndFeel::textColourValue)
                  : CustomLookAndFeel::colour(CustomLookAndFeel::mutedTextColourValue));
    }
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

#include "DeckGUI.h"
#include "../shared/CustomLookAndFeel.h"
#include <cmath>

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
    deckTitle.setFont(juce::Font(juce::FontOptions(18.5f).withStyle("Bold")));
    deckTitle.setColour(juce::Label::textColourId, CustomLookAndFeel::colour(CustomLookAndFeel::textColourValue));
    deckTitle.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(deckTitle);
    deckTitle.setVisible(false);

    stateLabel.setText("", juce::dontSendNotification);
    stateLabel.setFont(juce::Font(juce::FontOptions(10.5f).withStyle("Bold")));
    stateLabel.setJustificationType(juce::Justification::centred);
    stateLabel.setColour(juce::Label::backgroundColourId, CustomLookAndFeel::colour(CustomLookAndFeel::accentRedValue).withAlpha(0.85f));
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
    resetVolumeButton.setButtonText("RESET VOL");
    resetSpeedButton.setButtonText("RESET");
    lpfButton.setButtonText("LOW CUT");
    hpfButton.setButtonText("HIGH CUT");
    delayButton.setButtonText("DELAY");

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
    styleDeckButton(delayButton, CustomLookAndFeel::colour(CustomLookAndFeel::panelRaisedColourValue));
    styleDeckButton(nudgeDownButton, CustomLookAndFeel::colour(CustomLookAndFeel::panelRaisedColourValue));
    styleDeckButton(nudgeUpButton, CustomLookAndFeel::colour(CustomLookAndFeel::panelRaisedColourValue));
    styleDeckButton(resetVolumeButton, CustomLookAndFeel::colour(CustomLookAndFeel::panelRaisedColourValue).darker(0.05f));
    styleDeckButton(resetSpeedButton, CustomLookAndFeel::colour(CustomLookAndFeel::panelRaisedColourValue).darker(0.05f));
    styleDeckButton(tempoRangeButton, colour.withAlpha(0.65f));
    styleDeckButton(trackLoopButton, CustomLookAndFeel::colour(CustomLookAndFeel::panelRaisedColourValue).darker(0.05f));

    for (auto* b : { &playButton, &stopButton, &loadButton, &syncButton,
                     &loopInButton, &loopOutButton, &clearLoopButton,
                     &setCueButton, &goCueButton,
                     &lpfButton, &hpfButton, &delayButton, &nudgeDownButton, &nudgeUpButton,
                     &resetVolumeButton, &resetSpeedButton, &tempoRangeButton, &trackLoopButton })
    {
        b->addListener(this);
        addAndMakeVisible(b);
    }

    tapButton.addListener(this);
    addAndMakeVisible(tapButton);

    volSlider.setRange(0.0, 1.0);    volSlider.setValue(0.5);
    speedSlider.setRange(-tempoRangePercent_, tempoRangePercent_);
    speedSlider.setValue(0.0);
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
    speedLabel.setText("TEMPO",   juce::dontSendNotification);
    posLabel.setText("SEEK", juce::dontSendNotification);

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
                     &lpfButton, &hpfButton, &delayButton, &nudgeDownButton, &nudgeUpButton, &tapButton,
                     &resetVolumeButton, &resetSpeedButton, &tempoRangeButton, &trackLoopButton })
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
    const float outerRadius = 18.0f;
    const auto state = player.getState();
    bool hasHotcue = false;
    for (int i = 0; i < kNumHotcues; ++i)
        hasHotcue = hasHotcue || player.isHotcueSet(i);

    juce::DropShadow shadow(juce::Colours::black.withAlpha(0.42f), 16, { 0, 8 });
    shadow.drawForRectangle(g, bounds.toNearestInt());

    juce::ColourGradient background(deckColour_.withAlpha(0.13f), bounds.getTopLeft(),
                                    CustomLookAndFeel::colour(CustomLookAndFeel::panelColourValue), bounds.getBottomLeft(), false);
    g.setGradientFill(background);
    g.fillRoundedRectangle(bounds, outerRadius);

    auto inner = bounds.reduced(1.5f);
    juce::ColourGradient surface(CustomLookAndFeel::colour(CustomLookAndFeel::panelAltColourValue).brighter(0.08f), inner.getTopLeft(),
                                 CustomLookAndFeel::colour(CustomLookAndFeel::panelColourValue).darker(0.02f), inner.getBottomLeft(), false);
    g.setGradientFill(surface);
    g.fillRoundedRectangle(inner, outerRadius - 1.5f);

    auto glow = inner.reduced(6.0f);
    g.setColour(getDeckGlowColour());
    g.drawRoundedRectangle(glow, 14.0f, 1.2f);

    g.setColour(CustomLookAndFeel::colour(CustomLookAndFeel::outlineColourValue).withAlpha(0.95f));
    g.drawRoundedRectangle(inner, outerRadius - 1.5f, 1.0f);

    auto accent = inner.removeFromTop(5.0f).reduced(16.0f, 0.0f);
    juce::ColourGradient accentFill(deckColour_.brighter(0.35f), accent.getTopLeft(),
                                    deckColour_.darker(0.1f), accent.getTopRight(), false);
    g.setGradientFill(accentFill);
    g.fillRoundedRectangle(accent, 2.5f);

    juce::ColourGradient gloss(juce::Colours::white.withAlpha(0.055f), bounds.getTopLeft(),
                               juce::Colours::transparentBlack, bounds.getCentre(), false);
    g.setGradientFill(gloss);
    g.fillRoundedRectangle(bounds, outerRadius);

    auto innerSurface = bounds.reduced(12.0f, 12.0f);

    auto topWash = innerSurface.removeFromTop(72.0f);
    juce::ColourGradient topFill(juce::Colours::white.withAlpha(0.020f), topWash.getTopLeft(),
                                 juce::Colours::transparentWhite, topWash.getBottomLeft(), false);
    g.setGradientFill(topFill);
    g.fillRoundedRectangle(topWash, 10.0f);

    auto leftRail = bounds.reduced(8.0f);
    auto deckAccent = leftRail.removeFromLeft(2.5f);
    g.setColour(deckColour_.withAlpha(0.22f));
    g.fillRoundedRectangle(deckAccent, 1.25f);

    auto grooveBounds = bounds.reduced(14.0f, 14.0f);
    auto drawSoftGroove = [&](juce::Rectangle<float> area, float alpha)
    {
        if (area.isEmpty())
            return;

        g.setColour(juce::Colours::black.withAlpha(alpha));
        g.drawRoundedRectangle(area, 8.0f, 1.0f);
        g.setColour(juce::Colours::white.withAlpha(alpha * 0.45f));
        g.drawRoundedRectangle(area.reduced(0.6f), 7.4f, 0.7f);
    };

    const auto headerBand = stateLabel.getBounds().getUnion(deckTitle.getBounds())
                                         .getUnion(bpmLabel.getBounds()).getUnion(timeLabel.getBounds())
                                         .expanded(8, 6).toFloat();
    drawSoftGroove(headerBand.withX(18.0f).withWidth(static_cast<float>(getWidth() - 36)), 0.10f);

    auto lampArea = stateLabel.getBounds().toFloat();
    const auto lampColour = stateLabel.findColour(juce::Label::backgroundColourId);
    auto lampGlow = lampArea.expanded(3.0f, 3.0f);
    juce::ColourGradient lampFill(lampColour.withAlpha(0.22f), lampGlow.getCentre(),
                                  juce::Colours::transparentBlack, lampGlow.getBottomRight(), true);
    g.setGradientFill(lampFill);
    g.fillEllipse(lampGlow);
    g.setColour(lampColour);
    g.fillEllipse(lampArea);
    auto highlight = lampArea.reduced(lampArea.getWidth() * 0.32f, lampArea.getHeight() * 0.32f);
    highlight = highlight.withHeight(lampArea.getHeight() * 0.24f).translated(-0.5f, -1.0f);
    g.setColour(juce::Colours::white.withAlpha(0.22f));
    g.fillEllipse(highlight);
    g.setColour(lampColour.brighter(0.22f).withAlpha(0.60f));
    g.drawEllipse(lampArea, 1.0f);

    {
        juce::Graphics::ScopedSaveState save(g);
        auto titleArea = deckTitle.getBounds();
        g.reduceClipRegion(titleArea);
        g.setColour(deckTitle.findColour(juce::Label::textColourId));
        g.setFont(deckTitle.getFont());

        if (!titleNeedsMarquee_)
        {
            g.drawText(deckTitle.getText(), titleArea, juce::Justification::centredLeft, true);
        }
        else
        {
            const int gap = 36;
            const int startX = titleArea.getX() - static_cast<int>(std::round(titleScrollOffset_));
            const int textW = static_cast<int>(std::ceil(titleTextWidth_)) + 8;
            g.drawText(deckTitle.getText(),
                       juce::Rectangle<int>(startX, titleArea.getY(), textW, titleArea.getHeight()),
                       juce::Justification::centredLeft, false);

            const int secondX = startX + textW + gap;
            g.drawText(deckTitle.getText(),
                       juce::Rectangle<int>(secondX, titleArea.getY(), textW, titleArea.getHeight()),
                       juce::Justification::centredLeft, false);
        }
    }

    const auto transportBand = playButton.getBounds().getUnion(stopButton.getBounds())
                                         .getUnion(loadButton.getBounds()).getUnion(syncButton.getBounds())
                                         .expanded(6, 6).toFloat();
    drawSoftGroove(transportBand, 0.09f);

    const auto hotcueBand = hotcuePads.front().getBounds().getUnion(clearHotcuesButton.getBounds())
                                      .expanded(5, 5).toFloat();
    drawSoftGroove(hotcueBand, 0.07f);
    if (hasHotcue)
    {
        g.setColour(deckColour_.withAlpha(0.20f));
        g.drawRoundedRectangle(hotcueBand.reduced(0.8f), 8.0f, 1.0f);
    }

    const auto performanceBand = setCueButton.getBounds().getUnion(nudgeUpButton.getBounds())
                                            .expanded(6, 6).toFloat();
    drawSoftGroove(performanceBand, 0.08f);

    auto loopBand = loopInButton.getBounds().getUnion(loopOutButton.getBounds())
                                .getUnion(clearLoopButton.getBounds())
                                .getUnion(loopBar1_4Button.getBounds())
                                .getUnion(loopBar8Button.getBounds())
                                .expanded(5, 5).toFloat();
    if (state.loopActive)
    {
        g.setColour(CustomLookAndFeel::colour(CustomLookAndFeel::accentOrangeValue).withAlpha(0.24f));
        g.drawRoundedRectangle(loopBand, 8.0f, 1.0f);
    }

    auto subtleLine = [&](float y, float inset, float alpha)
    {
        g.setColour(juce::Colours::white.withAlpha(alpha));
        g.drawHorizontalLine(static_cast<int>(y), inset, static_cast<float>(getWidth()) - inset);
    };

    subtleLine(transportBand.getBottom() + 8.0f, 22.0f, 0.030f);
    subtleLine(hotcueBand.getBottom() + 8.0f, 22.0f, 0.022f);
    subtleLine(waveformDisplay.getBottom() + 10.0f, 22.0f, 0.028f);

    auto waveformShelf = waveformDisplay.getBounds().toFloat().expanded(6.0f, 8.0f);
    g.setColour(deckColour_.withAlpha(0.08f));
    g.drawRoundedRectangle(waveformShelf, 10.0f, 1.0f);
    if (state.isPlaying)
    {
        g.setColour(deckColour_.withAlpha(0.18f));
        g.drawRoundedRectangle(waveformShelf.reduced(1.1f), 9.0f, 1.2f);
    }
    if (state.loopActive)
    {
        g.setColour(CustomLookAndFeel::colour(CustomLookAndFeel::accentOrangeValue).withAlpha(0.26f));
        g.drawRoundedRectangle(waveformShelf.reduced(2.3f), 8.0f, 1.0f);
    }
    if (player.isTrackLoopEnabled())
    {
        const float y = waveformShelf.getY() + 4.0f;
        g.setColour(deckColour_.withAlpha(0.45f));
        g.drawHorizontalLine(static_cast<int>(y), waveformShelf.getX() + 10.0f, waveformShelf.getRight() - 10.0f);
    }

    auto bottomGlow = bounds.removeFromBottom(108.0f).reduced(18.0f, 0.0f);
    juce::ColourGradient lowerFill(deckColour_.withAlpha(0.04f), bottomGlow.getBottomLeft(),
                                   juce::Colours::transparentBlack, bottomGlow.getTopLeft(), false);
    g.setGradientFill(lowerFill);
    g.fillRoundedRectangle(bottomGlow, 14.0f);
}

void DeckGUI::resized()
{
    auto area = getLocalBounds().reduced(14);
    const bool compact = getWidth() < 500 || getHeight() < 560;
    const int gap = compact ? 8 : 10;
    const int labelW = compact ? 66 : 74;

    auto titleRow = area.removeFromTop(38);
    auto rightInfo = titleRow.removeFromRight(compact ? 168 : 178);
    const int lampSize = compact ? 10 : 12;
    const int lampGap = compact ? 6 : 8;
    auto titleArea = titleRow.reduced(0, 2);
    const int titleInset = compact ? 18 : 22;
    const int lampX = titleArea.getX() + titleInset;
    stateLabel.setBounds(lampX, titleArea.getCentreY() - lampSize / 2, lampSize, lampSize);
    auto titleBounds = titleArea;
    titleBounds.setX(stateLabel.getRight() + lampGap);
    const int infoReserve = compact ? 170 : 186;
    titleBounds.setRight(getWidth() - 14 - infoReserve);
    deckTitle.setBounds(titleBounds);
    updateTitleMarquee();
    bpmLabel.setBounds(rightInfo.removeFromLeft(compact ? 76 : 82));
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
    const int sliderRowH = compact ? 26 : 28;
    const int sliderGap = compact ? 8 : 10;

    auto layoutSliderRow = [&](juce::Label& label, juce::Slider& slider,
                               juce::TextButton* resetButton,
                               juce::TextButton* rangeButton = nullptr)
    {
        auto row = controlsRow.removeFromTop(sliderRowH);
        label.setBounds(row.removeFromLeft(labelW));
        if (rangeButton != nullptr)
        {
            auto rangeArea = row.removeFromRight(compact ? 54 : 58);
            rangeButton->setBounds(rangeArea.reduced(0, 2));
            row.removeFromRight(compact ? 4 : 6);
        }
        if (resetButton != nullptr)
        {
            auto resetArea = row.removeFromRight(rangeButton != nullptr ? (compact ? 54 : 58) : (compact ? 78 : 86));
            resetButton->setBounds(resetArea.reduced(0, 2));
            row.removeFromRight(compact ? 6 : 8);
        }
        slider.setBounds(row.reduced(4, 4));
        controlsRow.removeFromTop(sliderGap);
    };

    layoutSliderRow(volLabel, volSlider, &resetVolumeButton);
    layoutSliderRow(speedLabel, speedSlider, &resetSpeedButton, &tempoRangeButton);
    layoutSliderRow(posLabel, posSlider, &trackLoopButton);

    area.removeFromTop(gap);

    // Hotcue pads row (8 pads + CLR)
    auto hotcueRow = area.removeFromTop(26);
    const int padGap = compact ? 2 : 3;
    const int clearWidth = 34;
    const int padsWidth = hotcueRow.getWidth() - clearWidth - padGap;
    const int padW = (padsWidth - padGap * 7) / 8;
    for (int i = 0; i < kNumHotcues; ++i)
    {
        hotcuePads[static_cast<size_t>(i)].setBounds(hotcueRow.removeFromLeft(padW));
        hotcueRow.removeFromLeft(padGap);
    }
    clearHotcuesButton.setBounds(hotcueRow.removeFromLeft(clearWidth));

    area.removeFromTop(gap);

    const int bottomControlHeight = compact ? 136 : 138;
    const int waveformHeight = juce::jmax(compact ? 94 : 120, area.getHeight() - bottomControlHeight - gap);
    waveformDisplay.setBounds(area.removeFromTop(waveformHeight));

    area.removeFromTop(gap);

    auto bottomControls = area.removeFromTop(bottomControlHeight).reduced(0, 2);
    auto topActionRow = bottomControls.removeFromTop(28);
    const int buttonGap = compact ? 6 : 8;
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
    const int barGap = compact ? 3 : 4;
    const int barW = (barRow.getWidth() - barGap * 5) / 6;
    for (auto* lb : { &loopBar1_4Button, &loopBar1_2Button, &loopBar1Button,
                      &loopBar2Button,   &loopBar4Button,   &loopBar8Button })
    {
        lb->setBounds(barRow.removeFromLeft(barW));
        if (lb != &loopBar8Button) barRow.removeFromLeft(barGap);
    }

    bottomControls.removeFromTop(6);

    auto filterRow = bottomControls.removeFromTop(24);
    const int filterButtonW = (filterRow.getWidth() - buttonGap * 4) / 5;
    lpfButton.setBounds(filterRow.removeFromLeft(filterButtonW)); filterRow.removeFromLeft(buttonGap);
    hpfButton.setBounds(filterRow.removeFromLeft(filterButtonW)); filterRow.removeFromLeft(buttonGap);
    delayButton.setBounds(filterRow.removeFromLeft(filterButtonW)); filterRow.removeFromLeft(buttonGap);
    nudgeDownButton.setBounds(filterRow.removeFromLeft(filterButtonW)); filterRow.removeFromLeft(buttonGap);
    nudgeUpButton.setBounds(filterRow.removeFromLeft(filterButtonW));
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
                setSpeedRatio(getSpeed() * ratio);
                alignBeatPhaseToOtherDeck();
            }
            else
            {
                const float otherSpeed = getOtherDeckSpeed();
                setSpeedRatio(otherSpeed);
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
    else if (b == &tempoRangeButton)  cycleTempoRange();
    else if (b == &trackLoopButton)   player.setTrackLoopEnabled(!player.isTrackLoopEnabled());
    else if (b == &lpfButton)         player.setLowPassEnabled(!player.isLowPassEnabled());
    else if (b == &hpfButton)         player.setHighPassEnabled(!player.isHighPassEnabled());
    else if (b == &delayButton)
    {
        player.setDelayTime(0.28);
        player.setDelayFeedback(0.35);
        player.setDelayWetDry(0.28);
        player.setDelayEnabled(!player.isDelayEnabled());
    }
    else if (b == &nudgeDownButton)   togglePitchNudge(-1);
    else if (b == &nudgeUpButton)     togglePitchNudge(1);
    else if (b == &clearHotcuesButton) clearAllHotcues();

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
    else if (s == &speedSlider) applySpeedSliderToPlayer();
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
    const bool loopActive = state.loopActive;
    const bool trackRepeat = player.isTrackLoopEnabled();
    const bool lowCutActive = player.isLowPassEnabled();
    const bool highCutActive = player.isHighPassEnabled();
    const bool delayActive = player.isDelayEnabled();
    bool hasHotcue = false;
    for (int i = 0; i < kNumHotcues; ++i)
        hasHotcue = hasHotcue || player.isHotcueSet(i);
    const bool hasCue = state.cuePoint > 0.0005 || hasHotcue;
    updateTitleMarquee();

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
    speedLabel.setText(formatTempoPercent(), juce::dontSendNotification);

    juce::Colour stateColour = CustomLookAndFeel::colour(CustomLookAndFeel::mutedTextColourValue);
    juce::String stateText = "READY";

    const double remaining = len - elapsed;

    if (!state.isLoaded)
    {
        stateText = "EMPTY";
        stateColour = CustomLookAndFeel::colour(CustomLookAndFeel::accentRedValue).withAlpha(0.8f);
    }
    else if (state.isPlaying && remaining <= 30.0 && len > 0.0)
    {
        stateText = "ENDING";
        stateColour = remaining <= 10.0
            ? CustomLookAndFeel::colour(CustomLookAndFeel::accentRedValue)
            : CustomLookAndFeel::colour(CustomLookAndFeel::accentOrangeValue);
    }
    else if (state.isPlaying && loopActive)
    {
        stateText = "LOOP";
        stateColour = CustomLookAndFeel::colour(CustomLookAndFeel::accentOrangeValue);
    }
    else if (state.isPlaying && trackRepeat)
    {
        stateText = "REPEAT";
        stateColour = deckColour_.brighter(0.20f);
    }
    else if (state.isPlaying)
    {
        stateText = "LIVE";
        stateColour = CustomLookAndFeel::colour(CustomLookAndFeel::accentGreenValue);
    }
    else if (loopActive)
    {
        stateText = "LOOP SET";
        stateColour = CustomLookAndFeel::colour(CustomLookAndFeel::accentOrangeValue);
    }
    else if (trackRepeat)
    {
        stateText = "REPEAT";
        stateColour = deckColour_.brighter(0.18f);
    }
    else if (hasCue)
    {
        stateText = "CUE SET";
        stateColour = deckColour_.brighter(0.18f);
    }
    else
    {
        stateText = "PAUSED";
        stateColour = CustomLookAndFeel::colour(CustomLookAndFeel::mutedTextColourValue).brighter(0.18f);
    }

    const auto lampColour = state.isLoaded
        ? CustomLookAndFeel::colour(CustomLookAndFeel::accentGreenValue).withAlpha(state.isPlaying ? 0.96f : 0.84f)
        : CustomLookAndFeel::colour(CustomLookAndFeel::accentRedValue).withAlpha(0.88f);
    stateLabel.setText("", juce::dontSendNotification);
    stateLabel.setTooltip(stateText);
    stateLabel.setColour(juce::Label::backgroundColourId, lampColour);
    timeLabel.setColour(juce::Label::textColourId,
                        (state.isLoaded && remaining <= 30.0 && len > 0.0)
                            ? stateColour
                            : (loopActive ? CustomLookAndFeel::colour(CustomLookAndFeel::accentOrangeValue).brighter(0.08f)
                                          : CustomLookAndFeel::colour(CustomLookAndFeel::textColourValue)));

    playButton.setButtonText(state.isPlaying ? "PLAYING" : "PLAY");
    playButton.setToggleState(state.isPlaying, juce::dontSendNotification);
    playButton.setColour(juce::TextButton::buttonColourId,
                         state.isPlaying ? deckColour_.brighter(0.1f) : deckColour_.withAlpha(0.9f));
    stopButton.setButtonText(state.isPlaying ? "PAUSE" : "PAUSED");
    stopButton.setEnabled(state.isLoaded);
    stopButton.setAlpha(!state.isLoaded ? 0.4f : 0.95f);

    const bool loaded = state.isLoaded;
    deckTitle.setColour(juce::Label::textColourId,
                        loaded ? CustomLookAndFeel::colour(CustomLookAndFeel::textColourValue)
                               : CustomLookAndFeel::colour(CustomLookAndFeel::mutedTextColourValue).withAlpha(0.85f));
    bpmLabel.setColour(juce::Label::textColourId,
                       loaded ? deckColour_.brighter(0.30f)
                              : CustomLookAndFeel::colour(CustomLookAndFeel::mutedTextColourValue));

    for (auto* button : { &playButton, &stopButton, &syncButton, &setCueButton, &goCueButton,
                          &loopInButton, &loopOutButton, &clearLoopButton,
                          &lpfButton, &hpfButton, &delayButton, &nudgeDownButton, &nudgeUpButton,
                          &tapButton, &resetVolumeButton, &resetSpeedButton, &tempoRangeButton, &trackLoopButton,
                          &clearHotcuesButton })
    {
        if (button != &playButton)
            button->setEnabled(loaded);
    }

    for (auto* pad : { &hotcuePads[0], &hotcuePads[1], &hotcuePads[2], &hotcuePads[3],
                       &hotcuePads[4], &hotcuePads[5], &hotcuePads[6], &hotcuePads[7] })
        pad->setEnabled(loaded);

    volSlider.setEnabled(loaded);
    speedSlider.setEnabled(loaded);
    posSlider.setEnabled(loaded);
    playButton.setEnabled(loaded);
    goCueButton.setColour(juce::TextButton::buttonColourId,
                          hasCue ? deckColour_.withAlpha(0.82f)
                                 : CustomLookAndFeel::colour(CustomLookAndFeel::panelRaisedColourValue));
    setCueButton.setColour(juce::TextButton::buttonColourId,
                           hasCue ? deckColour_.withAlpha(0.42f)
                                  : CustomLookAndFeel::colour(CustomLookAndFeel::panelRaisedColourValue));

    const bool volumeOffset = std::abs(volSlider.getValue() - 0.5) > 0.001;
    resetVolumeButton.setColour(juce::TextButton::buttonColourId,
                                volumeOffset ? deckColour_.withAlpha(0.40f)
                                             : CustomLookAndFeel::colour(CustomLookAndFeel::panelRaisedColourValue).darker(0.05f));

    const bool tempoOffset = std::abs(speedSlider.getValue()) > 0.01 || pitchNudgeDirection_ != 0;
    resetSpeedButton.setColour(juce::TextButton::buttonColourId,
                               tempoOffset ? deckColour_.withAlpha(0.45f)
                                           : CustomLookAndFeel::colour(CustomLookAndFeel::panelRaisedColourValue).darker(0.05f));

    juce::Colour rangeColour = deckColour_.withAlpha(0.65f);
    if (tempoRangePercent_ <= 8.0)
        rangeColour = deckColour_.withAlpha(0.52f);
    else if (tempoRangePercent_ >= 50.0)
        rangeColour = CustomLookAndFeel::colour(CustomLookAndFeel::accentOrangeValue).withAlpha(0.78f);
    tempoRangeButton.setColour(juce::TextButton::buttonColourId, rangeColour);

    loopInButton.setColour(juce::TextButton::buttonColourId,
                           loopActive ? CustomLookAndFeel::colour(CustomLookAndFeel::accentOrangeValue).withAlpha(0.78f)
                                      : deckColour_.darker(0.18f));
    loopOutButton.setColour(juce::TextButton::buttonColourId,
                            loopActive ? CustomLookAndFeel::colour(CustomLookAndFeel::accentOrangeValue).withAlpha(0.72f)
                                       : deckColour_.darker(0.10f));
    clearLoopButton.setColour(juce::TextButton::buttonColourId,
                              loopActive ? CustomLookAndFeel::colour(CustomLookAndFeel::accentRedValue).withAlpha(0.62f)
                                         : CustomLookAndFeel::colour(CustomLookAndFeel::panelRaisedColourValue));
    clearLoopButton.setEnabled(loaded && loopActive);

    lpfButton.setToggleState(lowCutActive, juce::dontSendNotification);
    hpfButton.setToggleState(highCutActive, juce::dontSendNotification);
    lpfButton.setColour(juce::TextButton::buttonColourId,
                        lowCutActive ? deckColour_.withAlpha(0.85f)
                                                  : CustomLookAndFeel::colour(CustomLookAndFeel::panelAltColourValue));
    hpfButton.setColour(juce::TextButton::buttonColourId,
                        highCutActive ? deckColour_.withAlpha(0.85f)
                                                   : CustomLookAndFeel::colour(CustomLookAndFeel::panelAltColourValue));
    delayButton.setToggleState(delayActive, juce::dontSendNotification);
    delayButton.setColour(juce::TextButton::buttonColourId,
                          delayActive ? deckColour_.withAlpha(0.85f)
                                                  : CustomLookAndFeel::colour(CustomLookAndFeel::panelAltColourValue));
    refreshNudgeButtons();

    trackLoopButton.setToggleState(trackRepeat, juce::dontSendNotification);
    trackLoopButton.setColour(juce::TextButton::buttonColourId,
                              trackRepeat ? deckColour_.withAlpha(0.85f)
                                          : CustomLookAndFeel::colour(CustomLookAndFeel::panelAltColourValue));
    updateHotcuePadColors();
    repaint();
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
    resetTitleMarquee();
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
    {
        deckTitle.setText(info, juce::dontSendNotification);
        resetTitleMarquee();
    }
}

float DeckGUI::getSpeed() const
{
    return static_cast<float>(speedSliderValueToRatio());
}

void DeckGUI::resetVolumeToDefault()
{
    volSlider.setValue(0.5, juce::sendNotificationSync);
}

void DeckGUI::resetSpeedToDefault()
{
    pitchNudgeDirection_ = 0;
    refreshNudgeButtons();
    speedSlider.setValue(0.0, juce::sendNotificationSync);
}

void DeckGUI::cycleTempoRange()
{
    if (tempoRangePercent_ < 10.0)
        setTempoRange(16.0);
    else if (tempoRangePercent_ < 20.0)
        setTempoRange(50.0);
    else
        setTempoRange(8.0);
}

void DeckGUI::setTempoRange(double rangePercent)
{
    const double currentRatio = getSpeed();
    tempoRangePercent_ = rangePercent;
    speedSlider.setRange(-tempoRangePercent_, tempoRangePercent_, 0.01);
    tempoRangeButton.setButtonText("RNG " + juce::String(static_cast<int>(tempoRangePercent_)) + "%");
    setSpeedRatio(currentRatio);
}

void DeckGUI::setSpeedRatio(double ratio)
{
    ratio = juce::jlimit(0.5, 1.5, ratio);
    pitchNudgeDirection_ = 0;
    refreshNudgeButtons();
    speedSlider.setValue(speedRatioToSliderValue(ratio), juce::sendNotificationSync);
}

void DeckGUI::applySpeedSliderToPlayer()
{
    double ratio = speedSliderValueToRatio();

    if (pitchNudgeDirection_ < 0)
        ratio *= 0.96;
    else if (pitchNudgeDirection_ > 0)
        ratio *= 1.04;

    player.setSpeed(juce::jlimit(0.5, 1.5, ratio));
}

void DeckGUI::alignBeatPhaseToOtherDeck()
{
    if (!getOtherDeckPositionSeconds)
        return;

    const double bpm = player.getBPM();
    const double otherBpm = getOtherDeckBPM ? getOtherDeckBPM() : 0.0;
    if (bpm <= 0.0 || otherBpm <= 0.0)
        return;

    const double secondsPerBeat = 60.0 / otherBpm;
    const double current = player.getCurrentPositionSeconds();
    const double other = getOtherDeckPositionSeconds();
    const double length = player.getTotalLength();
    if (secondsPerBeat <= 0.0 || length <= 0.0)
        return;

    const double myPhase = std::fmod(current, secondsPerBeat);
    const double otherPhase = std::fmod(other, secondsPerBeat);
    double offset = otherPhase - myPhase;
    if (offset < 0.0)
        offset += secondsPerBeat;

    if (offset > secondsPerBeat * 0.5)
        offset -= secondsPerBeat;

    const double target = juce::jlimit(0.0, length, current + offset);
    player.setPosition(target);
}

void DeckGUI::togglePitchNudge(int direction)
{
    pitchNudgeDirection_ = (pitchNudgeDirection_ == direction) ? 0 : direction;
    applySpeedSliderToPlayer();
    refreshNudgeButtons();
}

void DeckGUI::refreshNudgeButtons()
{
    const auto active = deckColour_.withAlpha(0.85f);
    const auto inactive = CustomLookAndFeel::colour(CustomLookAndFeel::panelAltColourValue);

    nudgeDownButton.setToggleState(pitchNudgeDirection_ < 0, juce::dontSendNotification);
    nudgeUpButton.setToggleState(pitchNudgeDirection_ > 0, juce::dontSendNotification);
    nudgeDownButton.setColour(juce::TextButton::buttonColourId,
                              pitchNudgeDirection_ < 0 ? active : inactive);
    nudgeUpButton.setColour(juce::TextButton::buttonColourId,
                            pitchNudgeDirection_ > 0 ? active : inactive);
}

float DeckGUI::measureTitleTextWidth() const
{
    juce::GlyphArrangement glyphs;
    glyphs.addLineOfText(deckTitle.getFont(), deckTitle.getText(), 0.0f, 0.0f);
    return glyphs.getBoundingBox(0, -1, true).getWidth();
}

void DeckGUI::resetTitleMarquee()
{
    titleScrollOffset_ = 0.0f;
    titlePauseTicks_ = 28;
    updateTitleMarquee();
}

void DeckGUI::updateTitleMarquee()
{
    const float previousOffset = titleScrollOffset_;
    const bool previousNeedsMarquee = titleNeedsMarquee_;
    titleTextWidth_ = measureTitleTextWidth();
    const float availableWidth = static_cast<float>(juce::jmax(0, deckTitle.getWidth() - 6));
    titleNeedsMarquee_ = titleTextWidth_ > availableWidth;

    if (!titleNeedsMarquee_)
    {
        titleScrollOffset_ = 0.0f;
        titlePauseTicks_ = 24;
        if (previousOffset != titleScrollOffset_ || previousNeedsMarquee != titleNeedsMarquee_)
            repaint(deckTitle.getBounds());
        return;
    }

    if (titlePauseTicks_ > 0)
    {
        --titlePauseTicks_;
        return;
    }

    const float cycleWidth = titleTextWidth_ + 44.0f;
    titleScrollOffset_ += 0.65f;
    if (titleScrollOffset_ >= cycleWidth)
    {
        titleScrollOffset_ = 0.0f;
        titlePauseTicks_ = 28;
    }

    if (previousOffset != titleScrollOffset_ || previousNeedsMarquee != titleNeedsMarquee_)
        repaint(deckTitle.getBounds());
}

double DeckGUI::speedSliderValueToRatio() const
{
    return juce::jlimit(0.5, 1.5, 1.0 + speedSlider.getValue() / 100.0);
}

double DeckGUI::speedRatioToSliderValue(double ratio) const
{
    return juce::jlimit(-tempoRangePercent_, tempoRangePercent_, (ratio - 1.0) * 100.0);
}

juce::String DeckGUI::formatTempoPercent() const
{
    const double percent = speedSlider.getValue();
    const juce::String sign = percent >= 0.0 ? "+" : "";
    return "TEMPO " + sign + juce::String(percent, 1) + "%";
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

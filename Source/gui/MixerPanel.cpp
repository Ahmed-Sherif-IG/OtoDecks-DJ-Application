#include "MixerPanel.h"
#include "../shared/CustomLookAndFeel.h"

static void styleMixerButton(juce::TextButton& button)
{
    button.setColour(juce::TextButton::buttonColourId, CustomLookAndFeel::colour(CustomLookAndFeel::panelRaisedColourValue));
    button.setColour(juce::TextButton::textColourOffId, CustomLookAndFeel::colour(CustomLookAndFeel::textColourValue));
    button.setColour(juce::TextButton::textColourOnId, CustomLookAndFeel::colour(CustomLookAndFeel::textColourValue));
}

static void styleSlider(juce::Slider& s, juce::Slider::SliderStyle style,
                        double lo, double hi, double def, juce::Colour colour)
{
    s.setSliderStyle(style);
    s.setRange(lo, hi);
    s.setValue(def, juce::dontSendNotification);
    s.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    s.setColour(juce::Slider::trackColourId, colour);
    s.setColour(juce::Slider::thumbColourId, CustomLookAndFeel::colour(CustomLookAndFeel::textColourValue));
}

static void styleLabel(juce::Label& l, const juce::String& text, juce::Colour colour)
{
    l.setText(text, juce::dontSendNotification);
    const float fontSize = text.length() > 4 ? 8.5f : 10.5f;
    l.setFont(juce::Font(juce::FontOptions(fontSize).withStyle("Bold")));
    l.setJustificationType(juce::Justification::centred);
    l.setColour(juce::Label::textColourId, colour);
}

//==============================================================================
MixerPanel::MixerPanel(DJAudioPlayer& p1, DJAudioPlayer& p2, std::atomic<float>& master)
    : player1_(p1),
      player2_(p2),
      masterGain_(master),
      vuMeter1([&p1]() { return p1.getRMSLevel(); }),
      vuMeter2([&p2]() { return p2.getRMSLevel(); })
{
    const auto blue   = CustomLookAndFeel::colour(CustomLookAndFeel::accentBlueValue);
    const auto orange = CustomLookAndFeel::colour(CustomLookAndFeel::accentOrangeValue);
    const auto neutral = CustomLookAndFeel::colour(CustomLookAndFeel::mutedTextColourValue);

    styleSlider(crossfader, juce::Slider::LinearHorizontal, 0.0, 1.0, 0.5, CustomLookAndFeel::colour(CustomLookAndFeel::textColourValue));
    crossfader.addListener(this);
    styleLabel(crossfaderLabel, "BALANCE", neutral);
    addAndMakeVisible(crossfader);
    addAndMakeVisible(crossfaderLabel);

    styleSlider(masterSlider, juce::Slider::LinearVertical, 0.0, 1.0, 0.8, CustomLookAndFeel::colour(CustomLookAndFeel::accentGreenValue));
    masterSlider.addListener(this);
    styleLabel(masterLabel, "MASTER", neutral.brighter(0.1f));
    addAndMakeVisible(masterSlider);
    addAndMakeVisible(masterLabel);

    styleSlider(trim1Slider, juce::Slider::LinearVertical, 0.0, 2.0, 1.0, blue);
    trim1Slider.addListener(this);
    styleLabel(trim1Label, "A", blue.brighter(0.1f));
    addAndMakeVisible(trim1Slider);
    addAndMakeVisible(trim1Label);

    styleSlider(trim2Slider, juce::Slider::LinearVertical, 0.0, 2.0, 1.0, orange);
    trim2Slider.addListener(this);
    styleLabel(trim2Label, "B", orange.brighter(0.1f));
    addAndMakeVisible(trim2Slider);
    addAndMakeVisible(trim2Label);

    juce::Slider* eqSliders1[] = { &low1Slider, &mid1Slider, &high1Slider };
    juce::Label* eqLabels1[]   = { &low1Label,  &mid1Label,  &high1Label };
    const char* eqTexts1[]     = { "BASS", "MIDS", "TREBLE" };

    juce::Slider* eqSliders2[] = { &low2Slider, &mid2Slider, &high2Slider };
    juce::Label* eqLabels2[]   = { &low2Label,  &mid2Label,  &high2Label };
    const char* eqTexts2[]     = { "BASS", "MIDS", "TREBLE" };

    for (int i = 0; i < 3; ++i)
    {
        styleSlider(*eqSliders1[i], juce::Slider::Rotary, -12.0, 12.0, 0.0, blue);
        eqSliders1[i]->setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                            juce::MathConstants<float>::pi * 2.75f, true);
        eqSliders1[i]->addListener(this);
        styleLabel(*eqLabels1[i], eqTexts1[i], blue.withAlpha(0.88f));
        addAndMakeVisible(*eqSliders1[i]);
        addAndMakeVisible(*eqLabels1[i]);

        styleSlider(*eqSliders2[i], juce::Slider::Rotary, -12.0, 12.0, 0.0, orange);
        eqSliders2[i]->setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                            juce::MathConstants<float>::pi * 2.75f, true);
        eqSliders2[i]->addListener(this);
        styleLabel(*eqLabels2[i], eqTexts2[i], orange.withAlpha(0.88f));
        addAndMakeVisible(*eqSliders2[i]);
        addAndMakeVisible(*eqLabels2[i]);
    }

    for (auto* button : { &low1KillButton, &mid1KillButton, &high1KillButton,
                          &low2KillButton, &mid2KillButton, &high2KillButton })
    {
        styleMixerButton(*button);
        button->addListener(this);
        addAndMakeVisible(*button);
    }

    styleMixerButton(resetMixerButton);
    styleMixerButton(resetEq1Button);
    styleMixerButton(resetEq2Button);
    resetMixerButton.addListener(this);
    resetEq1Button.addListener(this);
    resetEq2Button.addListener(this);
    addAndMakeVisible(resetMixerButton);
    addAndMakeVisible(resetEq1Button);
    addAndMakeVisible(resetEq2Button);

    // Crossfader curve buttons
    auto setupCurveButton = [&](juce::TextButton& btn, bool active)
    {
        btn.setClickingTogglesState(false);
        btn.setColour(juce::TextButton::buttonColourId,
                      active ? CustomLookAndFeel::colour(CustomLookAndFeel::accentBlueValue).withAlpha(0.8f)
                             : CustomLookAndFeel::colour(CustomLookAndFeel::panelRaisedColourValue));
        btn.setColour(juce::TextButton::textColourOffId, CustomLookAndFeel::colour(CustomLookAndFeel::textColourValue));
        btn.addListener(this);
        addAndMakeVisible(btn);
    };
    setupCurveButton(curveLinearButton, false);
    setupCurveButton(curveEqPowButton,  true);   // default is EqualPower
    setupCurveButton(curveCutButton,    false);

    addAndMakeVisible(vuMeter1);
    addAndMakeVisible(vuMeter2);

    applyCrossfader(0.5);
    refreshKillButtons();
    masterGain_.store(0.8f);
}

MixerPanel::~MixerPanel()
{
    juce::Slider* sliders[] = { &crossfader, &masterSlider, &trim1Slider, &trim2Slider,
                                &low1Slider, &mid1Slider, &high1Slider,
                                &low2Slider, &mid2Slider, &high2Slider };
    for (auto* slider : sliders)
        slider->removeListener(this);

    resetMixerButton.removeListener(this);
    resetEq1Button.removeListener(this);
    resetEq2Button.removeListener(this);
    low1KillButton.removeListener(this);
    mid1KillButton.removeListener(this);
    high1KillButton.removeListener(this);
    low2KillButton.removeListener(this);
    mid2KillButton.removeListener(this);
    high2KillButton.removeListener(this);
    curveLinearButton.removeListener(this);
    curveEqPowButton.removeListener(this);
    curveCutButton.removeListener(this);
}

//==============================================================================
void MixerPanel::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    juce::DropShadow shadow(juce::Colours::black.withAlpha(0.40f), 14, { 0, 8 });
    shadow.drawForRectangle(g, bounds.toNearestInt());

    juce::ColourGradient background(CustomLookAndFeel::colour(CustomLookAndFeel::panelRaisedColourValue).brighter(0.05f),
                                    bounds.getTopLeft(),
                                    CustomLookAndFeel::colour(CustomLookAndFeel::panelColourValue).darker(0.12f),
                                    bounds.getBottomLeft(),
                                    false);
    g.setGradientFill(background);
    g.fillRoundedRectangle(bounds.reduced(1.0f), 18.0f);

    auto inner = bounds.reduced(2.0f);
    g.setColour(juce::Colours::white.withAlpha(0.04f));
    g.fillRoundedRectangle(inner.removeFromTop(70.0f), 18.0f);

    g.setColour(CustomLookAndFeel::colour(CustomLookAndFeel::outlineColourValue).withAlpha(0.95f));
    g.drawRoundedRectangle(bounds.reduced(1.0f), 18.0f, 1.2f);

    const auto blue = CustomLookAndFeel::colour(CustomLookAndFeel::accentBlueValue).withAlpha(0.35f);
    const auto orange = CustomLookAndFeel::colour(CustomLookAndFeel::accentOrangeValue).withAlpha(0.35f);
    g.setColour(blue);
    g.fillRoundedRectangle(bounds.removeFromLeft(7.0f).reduced(1.0f), 4.0f);
    g.setColour(orange);
    g.fillRoundedRectangle(bounds.removeFromRight(7.0f).reduced(1.0f), 4.0f);
}

void MixerPanel::resized()
{
    auto area = getLocalBounds().reduced(8);
    const int labelH = 16;

    auto titleRow = area.removeFromTop(18);
    const int titleGap = 6;
    auto leftTitle = titleRow.removeFromLeft((titleRow.getWidth() - titleGap) / 2);
    titleRow.removeFromLeft(titleGap);
    auto rightTitle = titleRow;
    masterLabel.setBounds(leftTitle);
    crossfaderLabel.setBounds(rightTitle);

    area.removeFromTop(8);

    auto upperSection = area.removeFromTop(static_cast<int>(area.getHeight() * 0.74f));
    auto lowerSection = area;

    const int sideTrimW = 24;
    const int centerGap = 8;
    const int meterW = 14;
    const int masterW = 24;

    auto leftTrimLane = upperSection.removeFromLeft(sideTrimW);
    trim1Label.setBounds(leftTrimLane.removeFromTop(labelH));
    trim1Slider.setBounds(leftTrimLane.reduced(2, 4));
    upperSection.removeFromLeft(centerGap);

    auto rightTrimLane = upperSection.removeFromRight(sideTrimW);
    trim2Label.setBounds(rightTrimLane.removeFromTop(labelH));
    trim2Slider.setBounds(rightTrimLane.reduced(2, 4));
    upperSection.removeFromRight(centerGap);

    const int centerWidth = meterW * 2 + masterW + centerGap * 2;
    const int eqZoneWidth = juce::jmax(0, upperSection.getWidth() - centerWidth - centerGap * 2);
    const int eqColumnW = eqZoneWidth / 2;

    auto leftEq = upperSection.removeFromLeft(eqColumnW);
    upperSection.removeFromLeft(centerGap);
    auto centerLane = upperSection.removeFromLeft(centerWidth);
    upperSection.removeFromLeft(centerGap);
    auto rightEq = upperSection;

    auto layoutEqColumn = [&](juce::Rectangle<int> eqArea, juce::Slider& low, juce::Slider& mid, juce::Slider& high,
                              juce::Label& lowLabel, juce::Label& midLabel, juce::Label& highLabel,
                              juce::TextButton& lowKill, juce::TextButton& midKill, juce::TextButton& highKill)
    {
        const int bandGap = 4;
        const int bandHeight = (eqArea.getHeight() - bandGap * 2) / 3;
        const int killH = 16;
        const int knobSize = juce::jmin(eqArea.getWidth(), bandHeight - labelH - killH);

        auto placeBand = [&](juce::Rectangle<int> bandArea, juce::Slider& slider, juce::Label& label, juce::TextButton& killButton)
        {
            auto killArea = bandArea.removeFromBottom(killH);
            killButton.setBounds(killArea.reduced(1, 1));
            auto labelArea = bandArea.removeFromBottom(labelH);
            label.setBounds(labelArea);
            const int sz = juce::jmin(bandArea.getWidth(), bandArea.getHeight(), knobSize);
            const int cx = bandArea.getCentreX() - sz / 2;
            const int cy = bandArea.getCentreY() - sz / 2;
            slider.setBounds(cx, cy, sz, sz);
        };

        auto lowArea = eqArea.removeFromTop(bandHeight);
        eqArea.removeFromTop(bandGap);
        auto midArea = eqArea.removeFromTop(bandHeight);
        eqArea.removeFromTop(bandGap);
        auto highArea = eqArea;

        placeBand(lowArea, low, lowLabel, lowKill);
        placeBand(midArea, mid, midLabel, midKill);
        placeBand(highArea, high, highLabel, highKill);
    };

    layoutEqColumn(leftEq, low1Slider, mid1Slider, high1Slider, low1Label, mid1Label, high1Label,
                   low1KillButton, mid1KillButton, high1KillButton);
    layoutEqColumn(rightEq, low2Slider, mid2Slider, high2Slider, low2Label, mid2Label, high2Label,
                   low2KillButton, mid2KillButton, high2KillButton);

    auto vuLeft = centerLane.removeFromLeft(meterW);
    vuMeter1.setBounds(vuLeft.reduced(1, 8));
    centerLane.removeFromLeft(centerGap);
    auto masterLane = centerLane.removeFromLeft(masterW);
    masterSlider.setBounds(masterLane.reduced(2, 8));
    centerLane.removeFromLeft(centerGap);
    auto vuRight = centerLane.removeFromLeft(meterW);
    vuMeter2.setBounds(vuRight.reduced(1, 8));

    lowerSection.removeFromTop(8);
    auto resetRow = lowerSection.removeFromTop(24);
    const int resetGap = 4;
    const int resetWidth = (resetRow.getWidth() - resetGap * 2) / 3;
    resetEq1Button.setBounds(resetRow.removeFromLeft(resetWidth));
    resetRow.removeFromLeft(resetGap);
    resetMixerButton.setBounds(resetRow.removeFromLeft(resetWidth));
    resetRow.removeFromLeft(resetGap);
    resetEq2Button.setBounds(resetRow);

    lowerSection.removeFromTop(6);

    // Crossfader curve buttons
    auto curveRow = lowerSection.removeFromTop(20);
    const int curveGap = 3;
    const int curveW = (curveRow.getWidth() - curveGap * 2) / 3;
    curveLinearButton.setBounds(curveRow.removeFromLeft(curveW));
    curveRow.removeFromLeft(curveGap);
    curveEqPowButton.setBounds(curveRow.removeFromLeft(curveW));
    curveRow.removeFromLeft(curveGap);
    curveCutButton.setBounds(curveRow);

    lowerSection.removeFromTop(4);
    crossfader.setBounds(lowerSection.removeFromTop(28).reduced(2, 2));
}

//==============================================================================
void MixerPanel::sliderValueChanged(juce::Slider* s)
{
    if (s == &crossfader)         applyCrossfader(s->getValue());
    else if (s == &masterSlider)
    {
        masterGain_.store(static_cast<float>(s->getValue()));
        applyCrossfader(crossfader.getValue());
    }
    else if (s == &trim1Slider)   player1_.setTrimGain(s->getValue());
    else if (s == &trim2Slider)   player2_.setTrimGain(s->getValue());
    else if (s == &low1Slider)    { player1_.setEQLowKill(false);  player1_.setEQLow(s->getValue());  refreshKillButtons(); }
    else if (s == &mid1Slider)    { player1_.setEQMidKill(false);  player1_.setEQMid(s->getValue());  refreshKillButtons(); }
    else if (s == &high1Slider)   { player1_.setEQHighKill(false); player1_.setEQHigh(s->getValue()); refreshKillButtons(); }
    else if (s == &low2Slider)    { player2_.setEQLowKill(false);  player2_.setEQLow(s->getValue());  refreshKillButtons(); }
    else if (s == &mid2Slider)    { player2_.setEQMidKill(false);  player2_.setEQMid(s->getValue());  refreshKillButtons(); }
    else if (s == &high2Slider)   { player2_.setEQHighKill(false); player2_.setEQHigh(s->getValue()); refreshKillButtons(); }
}

void MixerPanel::buttonClicked(juce::Button* button)
{
    if (button == &resetMixerButton)
        resetMixerDefaults();
    else if (button == &resetEq1Button)
        resetDeck1EQ();
    else if (button == &resetEq2Button)
        resetDeck2EQ();
    else if (button == &low1KillButton)  { player1_.setEQLowKill (!player1_.isEQLowKilled());  refreshKillButtons(); }
    else if (button == &mid1KillButton)  { player1_.setEQMidKill (!player1_.isEQMidKilled());  refreshKillButtons(); }
    else if (button == &high1KillButton) { player1_.setEQHighKill(!player1_.isEQHighKilled()); refreshKillButtons(); }
    else if (button == &low2KillButton)  { player2_.setEQLowKill (!player2_.isEQLowKilled());  refreshKillButtons(); }
    else if (button == &mid2KillButton)  { player2_.setEQMidKill (!player2_.isEQMidKilled());  refreshKillButtons(); }
    else if (button == &high2KillButton) { player2_.setEQHighKill(!player2_.isEQHighKilled()); refreshKillButtons(); }
    else if (button == &curveLinearButton || button == &curveEqPowButton || button == &curveCutButton)
    {
        if (button == &curveLinearButton)
            crossfaderCurve_ = CrossfaderCurve::Linear;
        else if (button == &curveEqPowButton)
            crossfaderCurve_ = CrossfaderCurve::EqualPower;
        else
            crossfaderCurve_ = CrossfaderCurve::Cut;

        // Update button highlight
        const auto activeColour   = CustomLookAndFeel::colour(CustomLookAndFeel::accentBlueValue).withAlpha(0.8f);
        const auto inactiveColour = CustomLookAndFeel::colour(CustomLookAndFeel::panelRaisedColourValue);
        curveLinearButton.setColour(juce::TextButton::buttonColourId,
                                    crossfaderCurve_ == CrossfaderCurve::Linear ? activeColour : inactiveColour);
        curveEqPowButton.setColour(juce::TextButton::buttonColourId,
                                    crossfaderCurve_ == CrossfaderCurve::EqualPower ? activeColour : inactiveColour);
        curveCutButton.setColour(juce::TextButton::buttonColourId,
                                    crossfaderCurve_ == CrossfaderCurve::Cut ? activeColour : inactiveColour);

        applyCrossfader(crossfader.getValue());
    }
}

void MixerPanel::resetDeck1EQ()
{
    player1_.setEQLowKill(false);
    player1_.setEQMidKill(false);
    player1_.setEQHighKill(false);
    low1Slider.setValue(0.0, juce::sendNotificationSync);
    mid1Slider.setValue(0.0, juce::sendNotificationSync);
    high1Slider.setValue(0.0, juce::sendNotificationSync);
    refreshKillButtons();
}

void MixerPanel::resetDeck2EQ()
{
    player2_.setEQLowKill(false);
    player2_.setEQMidKill(false);
    player2_.setEQHighKill(false);
    low2Slider.setValue(0.0, juce::sendNotificationSync);
    mid2Slider.setValue(0.0, juce::sendNotificationSync);
    high2Slider.setValue(0.0, juce::sendNotificationSync);
    refreshKillButtons();
}

void MixerPanel::resetMixerDefaults()
{
    trim1Slider.setValue(1.0, juce::sendNotificationSync);
    trim2Slider.setValue(1.0, juce::sendNotificationSync);
    masterSlider.setValue(0.8, juce::sendNotificationSync);
    crossfader.setValue(0.5, juce::sendNotificationSync);
    resetDeck1EQ();
    resetDeck2EQ();
}

void MixerPanel::applyCrossfader(double pos)
{
    double g1 = 1.0, g2 = 1.0;

    switch (crossfaderCurve_)
    {
        case CrossfaderCurve::Linear:
            g1 = 1.0 - pos;
            g2 = pos;
            break;

        case CrossfaderCurve::EqualPower:
        {
            const double angle = pos * juce::MathConstants<double>::halfPi;
            g1 = std::cos(angle);
            g2 = std::sin(angle);
            break;
        }

        case CrossfaderCurve::Cut:
            // Hard cut: each side at full until centre, then silence
            g1 = (pos <= 0.5) ? 1.0 : 0.0;
            g2 = (pos >= 0.5) ? 1.0 : 0.0;
            break;
    }

    const double masterBoost = juce::jmap(masterSlider.getValue(), 0.0, 1.0, 0.65, 1.45);
    player1_.setCrossfaderGain(juce::jlimit(0.0, 2.0, g1 * masterBoost));
    player2_.setCrossfaderGain(juce::jlimit(0.0, 2.0, g2 * masterBoost));
}

void MixerPanel::refreshKillButtons()
{
    const auto activeA = CustomLookAndFeel::colour(CustomLookAndFeel::accentBlueValue).withAlpha(0.85f);
    const auto activeB = CustomLookAndFeel::colour(CustomLookAndFeel::accentOrangeValue).withAlpha(0.85f);
    const auto inactive = CustomLookAndFeel::colour(CustomLookAndFeel::panelRaisedColourValue);

    low1KillButton.setColour(juce::TextButton::buttonColourId,  player1_.isEQLowKilled()  ? activeA : inactive);
    mid1KillButton.setColour(juce::TextButton::buttonColourId,  player1_.isEQMidKilled()  ? activeA : inactive);
    high1KillButton.setColour(juce::TextButton::buttonColourId, player1_.isEQHighKilled() ? activeA : inactive);

    low2KillButton.setColour(juce::TextButton::buttonColourId,  player2_.isEQLowKilled()  ? activeB : inactive);
    mid2KillButton.setColour(juce::TextButton::buttonColourId,  player2_.isEQMidKilled()  ? activeB : inactive);
    high2KillButton.setColour(juce::TextButton::buttonColourId, player2_.isEQHighKilled() ? activeB : inactive);
}

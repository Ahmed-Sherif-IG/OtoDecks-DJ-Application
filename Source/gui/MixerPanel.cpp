#include "MixerPanel.h"
#include "../shared/CustomLookAndFeel.h"

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
    l.setFont(juce::Font(juce::FontOptions(10.5f).withStyle("Bold")));
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
    styleLabel(crossfaderLabel, "CROSSFADER", neutral);
    addAndMakeVisible(crossfader);
    addAndMakeVisible(crossfaderLabel);

    styleSlider(masterSlider, juce::Slider::LinearVertical, 0.0, 1.0, 0.8, CustomLookAndFeel::colour(CustomLookAndFeel::accentGreenValue));
    masterSlider.addListener(this);
    styleLabel(masterLabel, "MASTER", neutral.brighter(0.1f));
    addAndMakeVisible(masterSlider);
    addAndMakeVisible(masterLabel);

    styleSlider(trim1Slider, juce::Slider::LinearVertical, 0.0, 2.0, 1.0, blue);
    trim1Slider.addListener(this);
    styleLabel(trim1Label, "TRIM A", blue.brighter(0.1f));
    addAndMakeVisible(trim1Slider);
    addAndMakeVisible(trim1Label);

    styleSlider(trim2Slider, juce::Slider::LinearVertical, 0.0, 2.0, 1.0, orange);
    trim2Slider.addListener(this);
    styleLabel(trim2Label, "TRIM B", orange.brighter(0.1f));
    addAndMakeVisible(trim2Slider);
    addAndMakeVisible(trim2Label);

    juce::Slider* eqSliders1[] = { &low1Slider, &mid1Slider, &high1Slider };
    juce::Label* eqLabels1[]   = { &low1Label,  &mid1Label,  &high1Label };
    const char* eqTexts1[]     = { "LOW", "MID", "HIGH" };

    juce::Slider* eqSliders2[] = { &low2Slider, &mid2Slider, &high2Slider };
    juce::Label* eqLabels2[]   = { &low2Label,  &mid2Label,  &high2Label };
    const char* eqTexts2[]     = { "LOW", "MID", "HIGH" };

    for (int i = 0; i < 3; ++i)
    {
        styleSlider(*eqSliders1[i], juce::Slider::LinearVertical, -12.0, 12.0, 0.0, blue);
        eqSliders1[i]->addListener(this);
        styleLabel(*eqLabels1[i], eqTexts1[i], blue.withAlpha(0.88f));
        addAndMakeVisible(*eqSliders1[i]);
        addAndMakeVisible(*eqLabels1[i]);

        styleSlider(*eqSliders2[i], juce::Slider::LinearVertical, -12.0, 12.0, 0.0, orange);
        eqSliders2[i]->addListener(this);
        styleLabel(*eqLabels2[i], eqTexts2[i], orange.withAlpha(0.88f));
        addAndMakeVisible(*eqSliders2[i]);
        addAndMakeVisible(*eqLabels2[i]);
    }

    addAndMakeVisible(vuMeter1);
    addAndMakeVisible(vuMeter2);

    applyCrossfader(0.5);
    masterGain_.store(0.8f);
}

MixerPanel::~MixerPanel()
{
    juce::Slider* sliders[] = { &crossfader, &masterSlider, &trim1Slider, &trim2Slider,
                                &low1Slider, &mid1Slider, &high1Slider,
                                &low2Slider, &mid2Slider, &high2Slider };
    for (auto* slider : sliders)
        slider->removeListener(this);
}

//==============================================================================
void MixerPanel::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    juce::ColourGradient background(CustomLookAndFeel::colour(CustomLookAndFeel::panelAltColourValue).brighter(0.04f),
                                    bounds.getTopLeft(),
                                    CustomLookAndFeel::colour(CustomLookAndFeel::panelColourValue).darker(0.18f),
                                    bounds.getBottomLeft(),
                                    false);
    g.setGradientFill(background);
    g.fillRoundedRectangle(bounds.reduced(1.0f), 16.0f);

    g.setColour(CustomLookAndFeel::colour(CustomLookAndFeel::outlineColourValue).withAlpha(0.95f));
    g.drawRoundedRectangle(bounds.reduced(1.0f), 16.0f, 1.2f);

    const auto blue = CustomLookAndFeel::colour(CustomLookAndFeel::accentBlueValue).withAlpha(0.22f);
    const auto orange = CustomLookAndFeel::colour(CustomLookAndFeel::accentOrangeValue).withAlpha(0.22f);
    g.setColour(blue);
    g.fillRoundedRectangle(bounds.removeFromLeft(8.0f).reduced(1.0f), 4.0f);
    g.setColour(orange);
    g.fillRoundedRectangle(bounds.removeFromRight(8.0f).reduced(1.0f), 4.0f);
}

void MixerPanel::resized()
{
    auto area = getLocalBounds().reduced(10);
    const int labelH = 16;
    const int topSectionH = static_cast<int>(area.getHeight() * 0.68f);
    auto top = area.removeFromTop(topSectionH);

    auto leftStrip = top.removeFromLeft(30);
    trim1Label.setBounds(leftStrip.removeFromBottom(labelH));
    trim1Slider.setBounds(leftStrip);
    top.removeFromLeft(8);

    auto rightStrip = top.removeFromRight(30);
    trim2Label.setBounds(rightStrip.removeFromBottom(labelH));
    trim2Slider.setBounds(rightStrip);
    top.removeFromRight(8);

    auto center = top;
    const int laneGap = 8;
    const int meterWidth = 22;
    const int eqWidth = 32;
    const int centerWidth = 42;

    auto leftEq = center.removeFromLeft(eqWidth * 3 + laneGap * 2);
    auto centerBlock = center.removeFromLeft(centerWidth * 3 + laneGap * 2);
    center.removeFromLeft(6);
    auto rightEq = center.removeFromLeft(eqWidth * 3 + laneGap * 2);

    auto layoutEq = [labelH, laneGap](juce::Rectangle<int> areaRect, juce::Slider* sliders[3], juce::Label* labels[3])
    {
        for (int i = 0; i < 3; ++i)
        {
            auto slot = areaRect.removeFromLeft((areaRect.getWidth() - laneGap * (2 - i)) / (3 - i));
            if (i < 2)
                areaRect.removeFromLeft(laneGap);
            labels[i]->setBounds(slot.removeFromBottom(labelH));
            sliders[i]->setBounds(slot.reduced(2, 2));
        }
    };

    juce::Slider* eq1[] = { &low1Slider, &mid1Slider, &high1Slider };
    juce::Label*  el1[] = { &low1Label,  &mid1Label,  &high1Label };
    juce::Slider* eq2[] = { &low2Slider, &mid2Slider, &high2Slider };
    juce::Label*  el2[] = { &low2Label,  &mid2Label,  &high2Label };
    layoutEq(leftEq, eq1, el1);
    layoutEq(rightEq, eq2, el2);

    auto meter1Area = centerBlock.removeFromLeft(meterWidth);
    vuMeter1.setBounds(meter1Area.reduced(1));
    centerBlock.removeFromLeft(laneGap);

    auto masterArea = centerBlock.removeFromLeft(centerWidth);
    masterLabel.setBounds(masterArea.removeFromBottom(labelH));
    masterSlider.setBounds(masterArea.reduced(2, 2));
    centerBlock.removeFromLeft(laneGap);

    auto meter2Area = centerBlock.removeFromLeft(meterWidth);
    vuMeter2.setBounds(meter2Area.reduced(1));

    area.removeFromTop(10);
    auto bottom = area;
    crossfaderLabel.setBounds(bottom.removeFromTop(labelH));
    crossfader.setBounds(bottom.removeFromTop(28).reduced(8, 2));
}

//==============================================================================
void MixerPanel::sliderValueChanged(juce::Slider* s)
{
    if (s == &crossfader)         applyCrossfader(s->getValue());
    else if (s == &masterSlider)  masterGain_.store(static_cast<float>(s->getValue()));
    else if (s == &trim1Slider)   player1_.setTrimGain(s->getValue());
    else if (s == &trim2Slider)   player2_.setTrimGain(s->getValue());
    else if (s == &low1Slider)    player1_.setEQLow(s->getValue());
    else if (s == &mid1Slider)    player1_.setEQMid(s->getValue());
    else if (s == &high1Slider)   player1_.setEQHigh(s->getValue());
    else if (s == &low2Slider)    player2_.setEQLow(s->getValue());
    else if (s == &mid2Slider)    player2_.setEQMid(s->getValue());
    else if (s == &high2Slider)   player2_.setEQHigh(s->getValue());
}

void MixerPanel::applyCrossfader(double pos)
{
    const double angle = pos * juce::MathConstants<double>::halfPi;
    const double g1 = std::cos(angle);
    const double g2 = std::sin(angle);
    player1_.setCrossfaderGain(g1);
    player2_.setCrossfaderGain(g2);
}

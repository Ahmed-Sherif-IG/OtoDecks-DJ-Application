#include "MixerPanel.h"

static void styleSlider(juce::Slider& s, juce::Slider::SliderStyle style,
                        double lo, double hi, double def)
{
    s.setSliderStyle(style);
    s.setRange(lo, hi);
    s.setValue(def, juce::dontSendNotification);
    s.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
}

static void styleLabel(juce::Label& l, const juce::String& text)
{
    l.setText(text, juce::dontSendNotification);
    l.setFont(juce::Font(juce::FontOptions(10.0f)));
    l.setJustificationType(juce::Justification::centred);
    l.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
}

//==============================================================================
MixerPanel::MixerPanel(DJAudioPlayer& p1, DJAudioPlayer& p2, std::atomic<float>& master)
    : player1_(p1),
      player2_(p2),
      masterGain_(master),
      vuMeter1([&p1]() { return p1.getRMSLevel(); }),
      vuMeter2([&p2]() { return p2.getRMSLevel(); })
{
    styleSlider(crossfader, juce::Slider::LinearHorizontal, 0.0, 1.0, 0.5);
    crossfader.addListener(this);
    styleLabel(crossfaderLabel, "XFADE");
    addAndMakeVisible(crossfader);
    addAndMakeVisible(crossfaderLabel);

    styleSlider(masterSlider, juce::Slider::LinearVertical, 0.0, 1.0, 0.8);
    masterSlider.addListener(this);
    styleLabel(masterLabel, "MASTER");
    addAndMakeVisible(masterSlider);
    addAndMakeVisible(masterLabel);

    styleSlider(trim1Slider, juce::Slider::LinearVertical, 0.0, 2.0, 1.0);
    trim1Slider.addListener(this);
    styleLabel(trim1Label, "TRIM 1");
    addAndMakeVisible(trim1Slider);
    addAndMakeVisible(trim1Label);

    styleSlider(trim2Slider, juce::Slider::LinearVertical, 0.0, 2.0, 1.0);
    trim2Slider.addListener(this);
    styleLabel(trim2Label, "TRIM 2");
    addAndMakeVisible(trim2Slider);
    addAndMakeVisible(trim2Label);

    juce::Slider* eqSliders1[] = { &low1Slider, &mid1Slider, &high1Slider };
    juce::Label* eqLabels1[]   = { &low1Label,  &mid1Label,  &high1Label };
    const char* eqTexts1[]     = { "LO", "MID", "HI" };

    juce::Slider* eqSliders2[] = { &low2Slider, &mid2Slider, &high2Slider };
    juce::Label* eqLabels2[]   = { &low2Label,  &mid2Label,  &high2Label };
    const char* eqTexts2[]     = { "LO", "MID", "HI" };

    for (int i = 0; i < 3; ++i)
    {
        styleSlider(*eqSliders1[i], juce::Slider::LinearVertical, -12.0, 12.0, 0.0);
        eqSliders1[i]->addListener(this);
        styleLabel(*eqLabels1[i], eqTexts1[i]);
        addAndMakeVisible(*eqSliders1[i]);
        addAndMakeVisible(*eqLabels1[i]);

        styleSlider(*eqSliders2[i], juce::Slider::LinearVertical, -12.0, 12.0, 0.0);
        eqSliders2[i]->addListener(this);
        styleLabel(*eqLabels2[i], eqTexts2[i]);
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
    g.fillAll(juce::Colour::fromRGB(25, 25, 25));
    g.setColour(juce::Colours::dimgrey);
    g.drawRect(getLocalBounds(), 1);
    g.setColour(juce::Colours::darkgrey);
    g.drawHorizontalLine(getHeight() / 2, 0.0f, static_cast<float>(getWidth()));
}

void MixerPanel::resized()
{
    auto area = getLocalBounds().reduced(4);
    const int width = area.getWidth();
    const int height = area.getHeight();
    const int col = (width - 8) / 3;
    const int labelH = 14;

    auto top = area.removeFromTop(height / 2 - 10);

    auto d1eq = top.removeFromLeft(col);
    {
        const int slotWidth = d1eq.getWidth() / 3;
        juce::Slider* sliders[] = { &low1Slider, &mid1Slider, &high1Slider };
        juce::Label* labels[]   = { &low1Label,  &mid1Label,  &high1Label };

        for (int i = 0; i < 3; ++i)
        {
            auto slot = d1eq.removeFromLeft(slotWidth);
            labels[i]->setBounds(slot.removeFromBottom(labelH));
            sliders[i]->setBounds(slot);
        }
    }

    auto center = top.removeFromLeft(col);
    {
        const int vuWidth = center.getWidth() / 3;
        vuMeter1.setBounds(center.removeFromLeft(vuWidth).reduced(2));
        auto masterArea = center.removeFromLeft(vuWidth);
        masterLabel.setBounds(masterArea.removeFromBottom(labelH));
        masterSlider.setBounds(masterArea);
        vuMeter2.setBounds(center.reduced(2));
    }

    auto d2eq = top;
    {
        const int slotWidth = d2eq.getWidth() / 3;
        juce::Slider* sliders[] = { &low2Slider, &mid2Slider, &high2Slider };
        juce::Label* labels[]   = { &low2Label,  &mid2Label,  &high2Label };

        for (int i = 0; i < 3; ++i)
        {
            auto slot = d2eq.removeFromLeft(slotWidth);
            labels[i]->setBounds(slot.removeFromBottom(labelH));
            sliders[i]->setBounds(slot);
        }
    }

    area.removeFromTop(8);

    auto bottom = area;
    auto trim1Area = bottom.removeFromLeft(col / 2);
    trim1Label.setBounds(trim1Area.removeFromBottom(labelH));
    trim1Slider.setBounds(trim1Area);

    auto trim2Area = bottom.removeFromRight(col / 2);
    trim2Label.setBounds(trim2Area.removeFromBottom(labelH));
    trim2Slider.setBounds(trim2Area);

    auto xfArea = bottom;
    crossfaderLabel.setBounds(xfArea.removeFromBottom(labelH));
    crossfader.setBounds(xfArea.withSizeKeepingCentre(xfArea.getWidth(),
                                                      juce::jmin(30, xfArea.getHeight())));
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

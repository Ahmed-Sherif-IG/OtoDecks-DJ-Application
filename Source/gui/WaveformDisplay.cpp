#include "WaveformDisplay.h"
#include "../shared/CustomLookAndFeel.h"

WaveformDisplay::WaveformDisplay(juce::AudioFormatManager& formatManagerToUse,
                                 juce::AudioThumbnailCache& cacheToUse)
    : audioThumb(1000, formatManagerToUse, cacheToUse)
{
    audioThumb.addChangeListener(this);
}

WaveformDisplay::~WaveformDisplay()
{
    audioThumb.removeChangeListener(this);
}

//==============================================================================
void WaveformDisplay::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    const float outerRadius = 10.0f;

    juce::ColourGradient background(CustomLookAndFeel::colour(CustomLookAndFeel::panelAltColourValue).brighter(0.02f),
                                    bounds.getTopLeft(),
                                    juce::Colour(0xFF03060C),
                                    bounds.getBottomLeft(),
                                    false);
    g.setGradientFill(background);
    g.fillRoundedRectangle(bounds, outerRadius);

    auto screen = bounds.reduced(1.5f);
    juce::ColourGradient screenFill(juce::Colour(0xFF050911).withAlpha(0.96f), screen.getTopLeft(),
                                    juce::Colour(0xFF010203).withAlpha(0.99f), screen.getBottomLeft(), false);
    g.setGradientFill(screenFill);
    g.fillRoundedRectangle(screen, outerRadius - 1.2f);

    g.setColour(CustomLookAndFeel::colour(CustomLookAndFeel::outlineColourValue).withAlpha(0.72f));
    g.drawRoundedRectangle(bounds.reduced(0.5f), outerRadius, 0.9f);

    auto bezel = screen.reduced(8.0f, 9.0f);
    auto titleStrip = bezel.removeFromTop(18.0f);
    g.setColour(CustomLookAndFeel::colour(CustomLookAndFeel::mutedTextColourValue).brighter(0.24f).withAlpha(0.85f));
    g.setFont(juce::Font(juce::FontOptions(10.0f).withStyle("Bold")));
    g.drawText("WAVEFORM DISPLAY", titleStrip.toNearestInt(), juce::Justification::centredLeft, false);

    auto topStatus = titleStrip.removeFromRight(88.0f);
    g.setColour(waveformColour.withAlpha(fileLoaded ? 0.80f : 0.34f));
    g.drawText(fileLoaded ? "TRACK READY" : "NO TRACK", topStatus.toNearestInt(), juce::Justification::centredRight, false);

    auto inner = screen.reduced(10.0f, 12.0f);
    inner.removeFromTop(14.0f);

    if (!fileLoaded)
    {
        auto centerArea = inner.reduced(12.0f, 6.0f);
        auto iconArea = centerArea.removeFromTop(42.0f);
        iconArea.removeFromBottom(8.0f);

        juce::Path icon;
        const float cx = iconArea.getCentreX();
        const float cy = iconArea.getCentreY();
        icon.startNewSubPath(cx - 28.0f, cy + 10.0f);
        icon.lineTo(cx - 10.0f, cy - 6.0f);
        icon.lineTo(cx + 2.0f, cy + 4.0f);
        icon.lineTo(cx + 18.0f, cy - 14.0f);
        icon.lineTo(cx + 30.0f, cy + 10.0f);
        g.setColour(waveformColour.withAlpha(0.48f));
        g.strokePath(icon, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        g.setFont(juce::Font(juce::FontOptions(17.0f).withStyle("Bold")));
        g.setColour(CustomLookAndFeel::colour(CustomLookAndFeel::textColourValue).withAlpha(0.74f));
        g.drawText("Drop or Load a Track", centerArea.removeFromTop(28.0f).toNearestInt(),
                   juce::Justification::centred, true);

        g.setFont(juce::Font(juce::FontOptions(11.5f)));
        g.setColour(CustomLookAndFeel::colour(CustomLookAndFeel::mutedTextColourValue).withAlpha(0.92f));
        g.drawText("The deck display will show waveform, cue, loop, and beat markers here.",
                   centerArea.removeFromTop(24.0f).toNearestInt(), juce::Justification::centred, true);
        return;
    }

    juce::Rectangle<float> overview;
    if (inner.getHeight() > 88.0f)
    {
        overview = inner.removeFromTop(24.0f);
        inner.removeFromTop(8.0f);

        g.setColour(juce::Colour(0xFF02050A).withAlpha(0.78f));
        g.fillRoundedRectangle(overview, 6.0f);
        g.setColour(CustomLookAndFeel::colour(CustomLookAndFeel::outlineColourValue).withAlpha(0.72f));
        g.drawRoundedRectangle(overview, 6.0f, 1.0f);

        auto overviewWave = overview.reduced(5.0f, 4.0f);
        g.setColour(CustomLookAndFeel::colour(CustomLookAndFeel::mutedTextColourValue).withAlpha(0.75f));
        g.setFont(juce::Font(juce::FontOptions(9.0f).withStyle("Bold")));
        g.drawText("OVERVIEW", juce::Rectangle<int>(static_cast<int>(overview.getX()) + 6,
                                                    static_cast<int>(overview.getY()) + 2,
                                                    58, 10),
                   juce::Justification::centredLeft, false);

        if (loopActive_ && loopEndRel_ > loopStartRel_)
        {
            auto loopRect = juce::Rectangle<float>(
                overviewWave.getX() + static_cast<float>(loopStartRel_ * overviewWave.getWidth()), overviewWave.getY(),
                static_cast<float>((loopEndRel_ - loopStartRel_) * overviewWave.getWidth()), overviewWave.getHeight());
            g.setColour(waveformColour.withAlpha(0.16f));
            g.fillRoundedRectangle(loopRect, 4.0f);
        }

        g.setColour(waveformColour.withAlpha(0.48f));
        audioThumb.drawChannel(g, overviewWave.toNearestInt(), 0.0, audioThumb.getTotalLength(), 0, 0.72f);

        if (cueMarker_ > 0.0)
        {
            const float cx = overviewWave.getX() + static_cast<float>(cueMarker_ * overviewWave.getWidth());
            g.setColour(CustomLookAndFeel::colour(CustomLookAndFeel::accentOrangeValue).withAlpha(0.9f));
            g.drawVerticalLine(static_cast<int>(cx), overviewWave.getY(), overviewWave.getBottom());
        }

        const float opx = overviewWave.getX() + static_cast<float>(position * overviewWave.getWidth());
        g.setColour(juce::Colours::white.withAlpha(0.92f));
        g.drawLine(opx, overview.getY() + 2.0f, opx, overview.getBottom() - 2.0f, 1.8f);
    }

    for (int i = 1; i < 4; ++i)
    {
        const float y = inner.getY() + inner.getHeight() * (static_cast<float>(i) / 4.0f);
        g.setColour(CustomLookAndFeel::colour(CustomLookAndFeel::outlineColourValue).withAlpha(0.24f));
        g.drawHorizontalLine(static_cast<int>(y), inner.getX(), inner.getRight());
    }

    for (int i = 1; i < 16; ++i)
    {
        const float x = inner.getX() + inner.getWidth() * (static_cast<float>(i) / 16.0f);
        g.setColour(juce::Colours::white.withAlpha(0.035f));
        g.drawVerticalLine(static_cast<int>(x), inner.getY(), inner.getBottom());
    }

    if (loopActive_ && loopEndRel_ > loopStartRel_)
    {
        auto loopRect = juce::Rectangle<float>(
            inner.getX() + static_cast<float>(loopStartRel_ * inner.getWidth()), inner.getY(),
            static_cast<float>((loopEndRel_ - loopStartRel_) * inner.getWidth()), inner.getHeight());
        g.setColour(waveformColour.withAlpha(0.15f));
        g.fillRoundedRectangle(loopRect, 8.0f);
    }

    g.setColour(waveformColour.withAlpha(0.98f));
    audioThumb.drawChannel(g, inner.toNearestInt(), 0.0, audioThumb.getTotalLength(), 0, 0.95f);

    g.setColour(waveformColour.withAlpha(0.08f));
    g.fillRect(inner.withHeight(inner.getHeight() * 0.5f).withY(inner.getCentreY() - inner.getHeight() * 0.25f));

    if (!beatPositions_.empty())
    {
        g.setColour(juce::Colours::white.withAlpha(0.12f));
        for (double beat : beatPositions_)
        {
            const float bx = inner.getX() + static_cast<float>(beat * inner.getWidth());
            g.drawVerticalLine(static_cast<int>(bx), inner.getY(), inner.getBottom());
        }
    }

    if (cueMarker_ > 0.0)
    {
        const float cx = inner.getX() + static_cast<float>(cueMarker_ * inner.getWidth());
        g.setColour(CustomLookAndFeel::colour(CustomLookAndFeel::accentOrangeValue));
        g.drawVerticalLine(static_cast<int>(cx), inner.getY(), inner.getBottom());
        juce::Path tri;
        tri.addTriangle(cx - 5.0f, inner.getY() + 1.0f,
                        cx + 5.0f, inner.getY() + 1.0f,
                        cx, inner.getY() + 10.0f);
        g.fillPath(tri);
    }

    if (loopStartRel_ > 0.0 || loopEndRel_ > 0.0)
    {
        const auto markerColour = loopActive_ ? waveformColour.brighter(0.35f)
                                              : CustomLookAndFeel::colour(CustomLookAndFeel::mutedTextColourValue);
        g.setColour(markerColour.withAlpha(0.95f));
        const float sx = inner.getX() + static_cast<float>(loopStartRel_ * inner.getWidth());
        const float ex = inner.getX() + static_cast<float>(loopEndRel_ * inner.getWidth());
        g.drawVerticalLine(static_cast<int>(sx), inner.getY(), inner.getBottom());
        g.drawVerticalLine(static_cast<int>(ex), inner.getY(), inner.getBottom());
    }

    const float px = inner.getX() + static_cast<float>(position * inner.getWidth());
    g.setColour(juce::Colours::white.withAlpha(0.18f));
    g.drawLine(px, inner.getY(), px, inner.getBottom(), 5.0f);
    g.setColour(juce::Colours::white.withAlpha(0.95f));
    g.drawLine(px, inner.getY(), px, inner.getBottom(), 2.2f);
    g.fillEllipse(px - 5.5f, inner.getCentreY() - 5.5f, 11.0f, 11.0f);

    juce::ColourGradient gloss(juce::Colours::white.withAlpha(0.035f), bounds.getTopLeft(),
                               juce::Colours::transparentBlack, bounds.getCentre(), false);
    g.setGradientFill(gloss);
    g.fillRoundedRectangle(bounds.reduced(1.0f), outerRadius - 1.0f);

    // Hover time tooltip
    if (mouseInside_ && hoverX_ >= 0 && fileLoaded && totalDuration_ > 0.0)
    {
        const float hx = static_cast<float>(hoverX_);
        const double hoverPos = juce::jlimit(0.0, 1.0, static_cast<double>(hoverX_) / getWidth());
        const double hoverSec = hoverPos * totalDuration_;

        const int hm = static_cast<int>(hoverSec) / 60;
        const int hs = static_cast<int>(hoverSec) % 60;
        const juce::String tooltip = juce::String(hm) + ":" + juce::String(hs).paddedLeft('0', 2);

        // Semi-transparent vertical line at hover position
        g.setColour(juce::Colours::white.withAlpha(0.25f));
        g.drawLine(hx, bounds.getY() + 4.0f, hx, bounds.getBottom() - 4.0f, 1.0f);

        // Tooltip pill
        const int tipW = 44;
        const int tipH = 20;
        float tipX = static_cast<float>(hoverX_) - static_cast<float>(tipW) / 2.0f;
        tipX = juce::jlimit(bounds.getX() + 4.0f, bounds.getRight() - tipW - 4.0f, tipX);
        const float tipY = bounds.getY() + 6.0f;
        const juce::Rectangle<float> tipRect(tipX, tipY, tipW, tipH);

        g.setColour(juce::Colours::black.withAlpha(0.72f));
        g.fillRoundedRectangle(tipRect, 6.0f);
        g.setColour(juce::Colours::white.withAlpha(0.85f));
        g.setFont(juce::Font(juce::FontOptions(11.0f).withStyle("Bold")));
        g.drawText(tooltip, tipRect.toNearestInt(), juce::Justification::centred, false);
    }
}

void WaveformDisplay::resized() {}

//==============================================================================
void WaveformDisplay::loadURL(juce::URL audioURL)
{
    beatPositions_.clear();
    loopStartRel_ = 0.0;
    loopEndRel_ = 0.0;
    loopActive_ = false;
    cueMarker_ = 0.0;
    position = 0.0;
    audioThumb.clear();
    fileLoaded = audioThumb.setSource(new juce::URLInputSource(audioURL));
    repaint();
}

void WaveformDisplay::changeListenerCallback(juce::ChangeBroadcaster*)
{
    repaint();
}

void WaveformDisplay::setPositionRelative(double pos)
{
    if (pos != position)
    {
        position = pos;
        repaint();
    }
}

void WaveformDisplay::setWaveformColour(juce::Colour colour)
{
    waveformColour = colour;
    repaint();
}

void WaveformDisplay::setLoopRegion(double start, double end, bool active)
{
    loopStartRel_ = start;
    loopEndRel_   = end;
    loopActive_   = active;
    repaint();
}

void WaveformDisplay::setCueMarker(double pos)
{
    cueMarker_ = pos;
    repaint();
}

void WaveformDisplay::setBeatPositions(const std::vector<double>& beats)
{
    beatPositions_ = beats;
    repaint();
}

//==============================================================================
void WaveformDisplay::seekToX(int x)
{
    if (!fileLoaded || !onSeek) return;
    const double pos = juce::jlimit(0.0, 1.0, static_cast<double>(x) / getWidth());
    onSeek(pos);
}

void WaveformDisplay::mouseDown(const juce::MouseEvent& e)
{
    seekToX(e.x);
}

void WaveformDisplay::mouseDrag(const juce::MouseEvent& e)
{
    seekToX(e.x);
}

void WaveformDisplay::mouseMove(const juce::MouseEvent& e)
{
    hoverX_      = e.x;
    mouseInside_ = true;
    repaint();
}

void WaveformDisplay::mouseExit(const juce::MouseEvent&)
{
    mouseInside_ = false;
    hoverX_      = -1;
    repaint();
}

void WaveformDisplay::setTotalDuration(double seconds)
{
    totalDuration_ = seconds;
}

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

    juce::ColourGradient background(CustomLookAndFeel::colour(CustomLookAndFeel::panelRaisedColourValue).brighter(0.04f),
                                    bounds.getTopLeft(),
                                    juce::Colour(0xFF090E15),
                                    bounds.getBottomLeft(),
                                    false);
    g.setGradientFill(background);
    g.fillRoundedRectangle(bounds, 14.0f);

    auto screen = bounds.reduced(2.0f);
    g.setColour(juce::Colour(0xFF05080D).withAlpha(0.55f));
    g.fillRoundedRectangle(screen, 12.0f);

    g.setColour(CustomLookAndFeel::colour(CustomLookAndFeel::outlineColourValue).withAlpha(0.95f));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 14.0f, 1.0f);

    auto inner = bounds.reduced(10.0f, 12.0f);

    if (!fileLoaded)
    {
        g.setFont(juce::Font(juce::FontOptions(16.0f).withStyle("Bold")));
        g.setColour(CustomLookAndFeel::colour(CustomLookAndFeel::mutedTextColourValue));
        g.drawText("Drop or Load a Track", inner.toNearestInt(), juce::Justification::centred, true);
        return;
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

    g.setColour(waveformColour.withAlpha(0.10f));
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

    juce::ColourGradient gloss(juce::Colours::white.withAlpha(0.07f), bounds.getTopLeft(),
                               juce::Colours::transparentBlack, bounds.getCentre(), false);
    g.setGradientFill(gloss);
    g.fillRoundedRectangle(bounds.reduced(1.0f), 12.0f);

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

#include "WaveformDisplay.h"

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
    auto bounds = getLocalBounds();
    int  w      = bounds.getWidth();
    int  h      = bounds.getHeight();

    g.fillAll(juce::Colour::fromRGB(10, 10, 10));
    g.setColour(juce::Colour::fromRGB(60, 60, 60));
    g.drawRect(bounds, 1);

    if (!fileLoaded)
    {
        g.setFont(juce::Font(juce::FontOptions(16.0f)));
        g.setColour(juce::Colours::dimgrey);
        g.drawText("Drop or Load a file", bounds, juce::Justification::centred, true);
        return;
    }

    // Loop region shading
    if (loopActive_)
    {
        auto loopRect = juce::Rectangle<float>(
            static_cast<float>(loopStartRel_ * w), 0.0f,
            static_cast<float>((loopEndRel_ - loopStartRel_) * w), static_cast<float>(h));
        g.setColour(waveformColour.withAlpha(0.15f));
        g.fillRect(loopRect);
    }

    // Waveform
    g.setColour(waveformColour);
    audioThumb.drawChannel(g, bounds, 0.0, audioThumb.getTotalLength(), 0, 1.0f);

    // Beat grid overlay (M5)
    if (!beatPositions_.empty())
    {
        g.setColour(juce::Colours::white.withAlpha(0.18f));
        for (double beat : beatPositions_)
        {
            float bx = static_cast<float>(beat * w);
            g.drawVerticalLine(static_cast<int>(bx), 0.0f, static_cast<float>(h));
        }
    }

    // Cue marker
    if (cueMarker_ > 0.0)
    {
        float cx = static_cast<float>(cueMarker_ * w);
        g.setColour(juce::Colours::yellow);
        g.drawVerticalLine(static_cast<int>(cx), 0.0f, static_cast<float>(h));
        // small triangle indicator at top
        juce::Path tri;
        tri.addTriangle(cx - 4.0f, 0.0f, cx + 4.0f, 0.0f, cx, 8.0f);
        g.fillPath(tri);
    }

    // Loop in/out markers
    if (loopStartRel_ > 0.0 || loopEndRel_ > 0.0)
    {
        auto markerColour = loopActive_ ? waveformColour.brighter(0.5f)
                                        : juce::Colours::grey;
        g.setColour(markerColour);
        float sx = static_cast<float>(loopStartRel_ * w);
        float ex = static_cast<float>(loopEndRel_ * w);
        g.drawVerticalLine(static_cast<int>(sx), 0.0f, static_cast<float>(h));
        g.drawVerticalLine(static_cast<int>(ex), 0.0f, static_cast<float>(h));
    }

    // Playhead
    float px = static_cast<float>(position * w);
    g.setColour(juce::Colours::white);
    g.drawLine(px, 0.0f, px, static_cast<float>(h), 2.0f);
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
    double pos = juce::jlimit(0.0, 1.0, static_cast<double>(x) / getWidth());
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

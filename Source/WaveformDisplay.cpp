
#include<JuceHeader.h>
#include "../JuceLibraryCode/JuceHeader.h"
#include "WaveformDisplay.h"
using namespace juce;
//==============================================================================
WaveformDisplay::WaveformDisplay(AudioFormatManager & 	formatManagerToUse,
                                 AudioThumbnailCache & 	cacheToUse) :
                                 audioThumb(1000, formatManagerToUse, cacheToUse), 
                                 fileLoaded(false), 
                                 position(0)
                          
{


  audioThumb.addChangeListener(this);
}

WaveformDisplay::~WaveformDisplay()
{
}

void WaveformDisplay::paint (Graphics& g)
{
    g.fillAll(Colour::fromRGB(0, 0, 0));
    g.setColour(Colour::fromRGB(100, 100, 100)); 
    g.drawRect(getLocalBounds(), 2);

    if (fileLoaded)
    {
        g.setColour(Colour::fromRGB(255, 255, 255));
        audioThumb.drawChannel(g, getLocalBounds(), 0, audioThumb.getTotalLength(), 0, 1.0f);

        g.setColour(Colour::fromRGB(0, 200, 83)); 
        g.drawRect(position * getWidth(), 0, 3, getHeight());
    }
    else
    {
        g.setFont(18.0f);
        g.setColour(Colours::grey);
        g.drawText("Drop or Load a file", getLocalBounds(), Justification::centred, true);
    }
}

void WaveformDisplay::resized()
{


}

void WaveformDisplay::loadURL(URL audioURL)
{
  audioThumb.clear();
  fileLoaded  = audioThumb.setSource(new URLInputSource(audioURL));
  if (fileLoaded)
  {
    std::cout << "wfd: loaded! " << std::endl;
    repaint();
  }
  else {
    std::cout << "wfd: not loaded! " << std::endl;
  }

}

void WaveformDisplay::changeListenerCallback (ChangeBroadcaster *source)
{
    std::cout << "wfd: change received! " << std::endl;

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





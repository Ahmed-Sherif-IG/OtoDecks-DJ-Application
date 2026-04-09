#include <JuceHeader.h>
#include "DeckGUI.h"

DeckGUI::DeckGUI(DJAudioPlayer* _player,
    AudioFormatManager& formatManagerToUse,
    AudioThumbnailCache& cacheToUse,
    const juce::String& deckTitleText)
    : player(_player),
    waveformDisplay(formatManagerToUse, cacheToUse)
{
    deckTitle.setText(deckTitleText, dontSendNotification);
    deckTitle.setFont(Font(20.0f, Font::bold));
    deckTitle.setJustificationType(Justification::centred);
    deckTitle.setColour(Label::textColourId, Colours::white);
    addAndMakeVisible(deckTitle);

    addAndMakeVisible(playButton);
    addAndMakeVisible(stopButton);
    addAndMakeVisible(loadButton);
    addAndMakeVisible(syncButton);

    playButton.addListener(this);
    stopButton.addListener(this);
    loadButton.addListener(this);
    syncButton.addListener(this);

    playButton.setButtonText("Play");
    stopButton.setButtonText("Stop");
    loadButton.setButtonText("Load");
    syncButton.setButtonText("Sync");

    volSlider.setRange(0.0, 1.0);
    speedSlider.setRange(0.0, 2.5);
    posSlider.setRange(0.0, 1.0);

    volSlider.setValue(0.5);
    speedSlider.setValue(1.0);

    for (auto* slider : { &volSlider, &speedSlider, &posSlider }) {
        slider->setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
        slider->addListener(this);
        addAndMakeVisible(slider);
    }

    volLabel.setText("Volume", dontSendNotification);
    speedLabel.setText("Speed", dontSendNotification);
    posLabel.setText("Position", dontSendNotification);

    for (auto* label : { &volLabel, &speedLabel, &posLabel }) {
        label->setColour(Label::textColourId, Colours::white);
        label->setFont(Font(14.0f));
        addAndMakeVisible(label);
    }

    addAndMakeVisible(waveformDisplay);

    startTimer(500);
}

DeckGUI::~DeckGUI()
{
    playButton.removeListener(this);
    stopButton.removeListener(this);
    loadButton.removeListener(this);

    volSlider.removeListener(this);
    speedSlider.removeListener(this);
    posSlider.removeListener(this);
    stopTimer();
    player = nullptr;
}

void DeckGUI::paint(Graphics& g)
{
    g.fillAll(Colour::fromRGB(20, 20, 20));
}

void DeckGUI::resized()
{
    auto area = getLocalBounds().reduced(4);
    int rowH = 40;
    int labelW = 60;
    int spacing = 4;

    deckTitle.setBounds(area.removeFromTop(rowH));

    // Button Row (Play / Stop)
    auto buttonRow = area.removeFromTop(rowH);
    int buttonWidth = (buttonRow.getWidth() - spacing * 2) / 3;
    playButton.setBounds(buttonRow.removeFromLeft(buttonWidth));
    buttonRow.removeFromLeft(spacing);
    stopButton.setBounds(buttonRow.removeFromLeft(buttonWidth));
    buttonRow.removeFromLeft(spacing);
    syncButton.setBounds(buttonRow);

    // Volume Row
    auto volRow = area.removeFromTop(rowH);
    volLabel.setBounds(volRow.removeFromLeft(labelW));
    volSlider.setBounds(volRow);

    // Speed Row
    auto speedRow = area.removeFromTop(rowH);
    speedLabel.setBounds(speedRow.removeFromLeft(labelW));
    speedSlider.setBounds(speedRow);

    // Position Row
    auto posRow = area.removeFromTop(rowH);
    posLabel.setBounds(posRow.removeFromLeft(labelW));
    posSlider.setBounds(posRow);

    // Waveform
    waveformDisplay.setBounds(area.removeFromTop(rowH * 2));

    // Load Button
    loadButton.setBounds(area.removeFromTop(rowH));
}

void DeckGUI::buttonClicked(Button* button)
{
    if (button == &playButton)
        player->start();
    else if (button == &stopButton)
        player->stop();
    else if (button == &loadButton)
    {
        fChooser.launchAsync(FileBrowserComponent::canSelectFiles, [this](const FileChooser& chooser)
            {
                auto file = chooser.getResult();
                if (file.existsAsFile())
                {
                    player->loadURL(URL{ file });
                    waveformDisplay.loadURL(URL{ file });
                }
            });
    }
    else if (button == &syncButton)
    {
        if (getOtherDeckSpeed)
        {
            float otherSpeed = getOtherDeckSpeed();
            speedSlider.setValue(otherSpeed);  
            player->setSpeed(otherSpeed);      
        }
    }
}

void DeckGUI::sliderValueChanged(Slider* slider)
{
    if (slider == &volSlider)
        player->setGain(slider->getValue());
    else if (slider == &speedSlider)
        player->setSpeed(slider->getValue());
    else if (slider == &posSlider)
        player->setPositionRelative(slider->getValue());
}

bool DeckGUI::isInterestedInFileDrag(const StringArray& files)
{
    return true;
}

void DeckGUI::filesDropped(const StringArray& files, int, int)
{
    if (files.size() == 1)
    {
        File file{ files[0] };
        if (file.existsAsFile())
        {
            player->loadURL(URL{ file });
            waveformDisplay.loadURL(URL{ file });
        }
    }
}

void DeckGUI::timerCallback()
{
    if (player != nullptr)
        waveformDisplay.setPositionRelative(player->getPositionRelative());
}

void DeckGUI::loadFile(const File& file)
{
    player->loadURL(URL{ file });
    waveformDisplay.loadURL(URL{ file });
}

float DeckGUI::getSpeed() const
{
    return speedSlider.getValue();
}

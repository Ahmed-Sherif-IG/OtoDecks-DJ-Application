#include "MainComponent.h"

MainComponent::MainComponent()
    : thumbnailCache(100),
    player1(formatManager),
    player2(formatManager)
{
    setLookAndFeel(&customLook);

    if (RuntimePermissions::isRequired(RuntimePermissions::recordAudio)
        && !RuntimePermissions::isGranted(RuntimePermissions::recordAudio))
    {
        RuntimePermissions::request(RuntimePermissions::recordAudio,
            [&](bool granted) { if (granted) setAudioChannels(2, 2); });
    }
    else
    {
        setAudioChannels(0, 2);
    }

    deckGUI1 = std::make_unique<DeckGUI>(&player1, formatManager, thumbnailCache, "Deck 1");
    deckGUI2 = std::make_unique<DeckGUI>(&player2, formatManager, thumbnailCache, "Deck 2");

    deckGUI1->getOtherDeckSpeed = [this]() { return deckGUI2->getSpeed(); };
    deckGUI2->getOtherDeckSpeed = [this]() { return deckGUI1->getSpeed(); };

    addAndMakeVisible(deckGUI1.get());
    addAndMakeVisible(deckGUI2.get());
    addAndMakeVisible(playlistComponent);

    
    formatManager.registerBasicFormats();

  
    playlistComponent.loadTrackToDeck = [this](File file, int deckNumber)
        {
            if (deckNumber == 1)
            {
                player1.loadURL(URL{ file });
                deckGUI1->loadFile(file);
            }
            else if (deckNumber == 2)
            {
                player2.loadURL(URL{ file });
                deckGUI2->loadFile(file);
            }
        };

    setSize(800, 600); 
}

MainComponent::~MainComponent()
{

    deckGUI1 = nullptr;
    deckGUI2 = nullptr;
    setLookAndFeel(nullptr);
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    player1.prepareToPlay(samplesPerBlockExpected, sampleRate);
    player2.prepareToPlay(samplesPerBlockExpected, sampleRate);

    mixerSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    mixerSource.addInputSource(&player1, false);
    mixerSource.addInputSource(&player2, false);
}

void MainComponent::getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill)
{
    mixerSource.getNextAudioBlock(bufferToFill);
}

void MainComponent::releaseResources()
{
    player1.releaseResources();
    player2.releaseResources();
    mixerSource.releaseResources();
}

void MainComponent::paint(Graphics& g)
{
    g.fillAll(Colour::fromRGB(18, 18, 18));
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced(10);
    auto deckArea = area.removeFromTop(proportionOfHeight(0.6f));
    auto deckWidth = deckArea.getWidth() / 2;

    if (deckGUI1)
        deckGUI1->setBounds(deckArea.removeFromLeft(deckWidth).reduced(5));

    if (deckGUI2)
        deckGUI2->setBounds(deckArea.reduced(5));

    playlistComponent.setBounds(area.reduced(5));
}


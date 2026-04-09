
#include <JuceHeader.h>
#include "../JuceLibraryCode/JuceHeader.h"
#include "PlaylistComponent.h"
#include "DeckLoadCellComponent.h"

using namespace std;
using namespace juce;

PlaylistComponent::PlaylistComponent()
{
    tableComponent.getHeader().addColumn("Track title", 1, 400);
    tableComponent.getHeader().addColumn("Load", 2, 200);
    tableComponent.setModel(this);

    addAndMakeVisible(tableComponent);
    addAndMakeVisible(addTracksButton);
    addAndMakeVisible(trackCountLabel);
    addTracksButton.addListener(this);


    addTracksButton.setLookAndFeel(&customLook);   
    trackCountLabel.setLookAndFeel(&customLook);
    addTracksButton.setButtonText(CharPointer_UTF8("\xe2\x9e\x95  Add Tracks"));  

    tableComponent.getHeader().setColour(TableHeaderComponent::backgroundColourId, Colours::black);
    tableComponent.getHeader().setColour(TableHeaderComponent::textColourId, Colours::lightgrey);



    loadLibrary();
}

PlaylistComponent::~PlaylistComponent()
{
    addTracksButton.setLookAndFeel(nullptr);
    trackCountLabel.setLookAndFeel(nullptr);
    tableComponent.setModel(nullptr);
    
}

void PlaylistComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);  

    g.setColour(juce::Colours::grey);
    g.drawRect(getLocalBounds(), 1);

}

void PlaylistComponent::resized()
{
    auto area = getLocalBounds().reduced(10);
    auto topRow = area.removeFromTop(50); 
    addTracksButton.setBounds(topRow.removeFromLeft(140));
    trackCountLabel.setBounds(topRow.removeFromLeft(100));
    tableComponent.setBounds(area);  

}


int PlaylistComponent::getNumRows()
{
    return static_cast<int>(tracks.size());
}

void PlaylistComponent::paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
{
    g.fillAll(rowIsSelected ? Colours::darkgrey : Colour(30, 30, 30));
    tableComponent.getHeader().setColumnWidth(1, 300); 

}

void PlaylistComponent::paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool)
{
    g.setColour(Colours::white);
    g.setFont(Font(14.0f));
    g.drawText(tracks[rowNumber].title, 10, 0, width - 20, height, Justification::centredLeft, true);
}


Component* PlaylistComponent::refreshComponentForCell(int rowNumber,
    int columnId,
    bool isRowSelected,
    Component* existingComponentToUpdate)
{
    if (rowNumber >= tracks.size())
        return nullptr;

    if (columnId == 2)
    {
        auto* cell = dynamic_cast<DeckLoadCellComponent*>(existingComponentToUpdate);

        if (cell == nullptr)
        {
            cell = new DeckLoadCellComponent(
                [this](int row, int deck)
                {
                    if (row >= 0 && row < tracks.size() && loadTrackToDeck)
                        loadTrackToDeck(tracks[row].file, deck);
                },
                [this](int row)
                {
                    if (row >= 0 && row < tracks.size())
                    {
                        tracks.erase(tracks.begin() + row);
                        saveLibrary();
                        updateTrackCountLabel();
                        tableComponent.deselectAllRows();
                        MessageManager::callAsync([this]() {
                            if (tableComponent.isShowing())
                            {
                                tableComponent.updateContent();
                            }
                            tableComponent.repaint();
                            });
                    }
                });
        }

        cell->setRow(rowNumber);
        return cell;
    }

    return nullptr;
}

void PlaylistComponent::buttonClicked(Button* button)
{
    if (button == &addTracksButton)
    {
        DBG("Add Tracks button clicked!");

        fileChooser.reset(new FileChooser("Select audio files...",
            {},
            "*.mp3;*.wav;*.aiff;*.flac",
            true));

        fileChooser->launchAsync(FileBrowserComponent::openMode
            | FileBrowserComponent::canSelectFiles
            | FileBrowserComponent::canSelectMultipleItems,
            [this](const FileChooser& fc)
            {
                DBG("FileChooser callback triggered");

                Array<File> selectedFiles = fc.getResults();
                DBG("Number of files selected: " + String(selectedFiles.size()));

                for (auto& file : selectedFiles)
                {
                    DBG("File selected: " + file.getFullPathName());

                    if (file.existsAsFile())
                    {
                        Track newTrack{ file.getFileNameWithoutExtension(), file };
                        tracks.push_back(newTrack);
                    }
                }

                tableComponent.updateContent();
                tableComponent.repaint();
                saveLibrary();
                updateTrackCountLabel();
            });
    }




    String id = button->getComponentID();
    if (id.startsWith("deck1_"))
    {
        int index = id.fromFirstOccurrenceOf("_", false, false).getIntValue();
        if (loadTrackToDeck) loadTrackToDeck(tracks[index].file, 1);
    }
    else if (id.startsWith("deck2_"))
    {
        int index = id.fromFirstOccurrenceOf("_", false, false).getIntValue();
        if (loadTrackToDeck) loadTrackToDeck(tracks[index].file, 2);
    }
}

void PlaylistComponent::saveLibrary()
{
    std::ofstream outFile(libraryFile.getFullPathName().toStdString());
    for (auto& track : tracks)
    {
        outFile << track.title.toStdString() << "," << track.file.getFullPathName().toStdString() << std::endl;
    }
}

void PlaylistComponent::loadLibrary()
{
    std::ifstream inFile(libraryFile.getFullPathName().toStdString());
    std::string line;
    while (std::getline(inFile, line))
    {
        std::stringstream ss(line);
        std::string title, path;
        if (std::getline(ss, title, ',') && std::getline(ss, path))
        {
            File f(path);
            if (f.existsAsFile())
            {
                tracks.push_back({ String(title), f });
            }
        }
    }
    updateTrackCountLabel();
}
void PlaylistComponent::updateTrackCountLabel()
{
    trackCountLabel.setText("Tracks: " + juce::String(tracks.size()), juce::dontSendNotification);
}
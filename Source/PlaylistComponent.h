
#pragma once

#include <JuceHeader.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include "CustomLookAndFeel.h"

using namespace std;
using namespace juce;

struct Track {
    String title;
    File file;
};

class PlaylistComponent : public juce::Component,
    public TableListBoxModel,
    public Button::Listener
{
public:
    PlaylistComponent();
    ~PlaylistComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    int getNumRows() override;

    void paintRowBackground(Graphics&,
        int rowNumber,
        int width,
        int height,
        bool rowIsSelected) override;

    void paintCell(Graphics&,
        int rowNumber,
        int columnId,
        int width,
        int height,
        bool rowIsSelected) override;

    Component* refreshComponentForCell(int rowNumber,
        int columnId,
        bool isRowSelected,
        Component* existingComponentToUpdate) override;

    void buttonClicked(Button* button) override;

    void saveLibrary();
    void loadLibrary();

    void updateTrackCountLabel();

    std::function<void(File, int)> loadTrackToDeck;
    std::unique_ptr<FileChooser> fileChooser;

private:
    TableListBox tableComponent;
    TextButton addTracksButton{ "Add Tracks" };
    vector<Track> tracks;
    File libraryFile = File::getSpecialLocation(File::userDocumentsDirectory)
        .getChildFile("trackLibrary.csv");
    juce::Label trackCountLabel;  

    CustomLookAndFeel customLook;



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlaylistComponent)
};

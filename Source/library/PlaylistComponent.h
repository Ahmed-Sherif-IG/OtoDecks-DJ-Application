#pragma once

#include <JuceHeader.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "../shared/Callbacks.h"

struct Track
{
    juce::String title;
    juce::String artist;
    juce::File   file;
    double       durationSeconds = 0.0;
    double       bpm             = 0.0;
    juce::int64  fileSize        = 0;
};

class PlaylistComponent : public juce::Component,
                          public juce::TableListBoxModel,
                          public juce::Button::Listener,
                          public juce::TextEditor::Listener
{
public:
    PlaylistComponent();
    ~PlaylistComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    // TableListBoxModel
    int getNumRows() override;

    void paintRowBackground(juce::Graphics&,
                            int rowNumber, int width, int height,
                            bool rowIsSelected) override;

    void paintCell(juce::Graphics&,
                   int rowNumber, int columnId,
                   int width, int height,
                   bool rowIsSelected) override;

    juce::Component* refreshComponentForCell(int rowNumber, int columnId,
                                              bool isRowSelected,
                                              juce::Component* existingComponentToUpdate) override;

    void sortOrderChanged(int newSortColumnId, bool isForwards) override;

    // Listeners
    void buttonClicked(juce::Button* button) override;
    void textEditorTextChanged(juce::TextEditor&) override;

    // Library persistence
    void saveLibrary();
    void loadLibrary();
    void updateTrackCountLabel();

    // Called by MainComponent to wire track loading into decks
    LoadTrackToDeckFn loadTrackToDeck;

    // Called after a track is loaded — supplies "Artist - Title" for the deck header
    std::function<void(int deckNumber, const juce::String& nowPlayingText)> onNowPlaying;

    // Mark a file as now playing to highlight its row
    void setNowPlayingFile(const juce::File& file);
    juce::File nowPlayingFile_;

    std::unique_ptr<juce::FileChooser> fileChooser;

private:
    struct SortKey
    {
        juce::String text;
        double number = 0.0;
    };

    void readMetadata(Track& track);
    std::vector<int> getFilteredIndices() const;
    static juce::String sanitiseDisplayText(const juce::String& text);
    static juce::String escapeCsvField(const juce::String& text);
    static bool parseCsvLine(const std::string& line, std::vector<std::string>& fields);
    static double parseDoubleSafe(const juce::String& text);
    static SortKey buildSortKey(const Track& track, int columnId);

    // Columns
    enum ColumnIds { ColTitle = 1, ColArtist, ColDuration, ColBPM, ColLoad };

    juce::TableListBox tableComponent;
    juce::TextButton   addTracksButton{ "Add Tracks" };
    juce::Label        trackCountLabel;
    juce::TextEditor   searchBox;    // M4: filter tracks
    juce::TextEditor   bpmMinBox;
    juce::TextEditor   bpmMaxBox;
    juce::TextEditor   durationMaxBox;

    std::vector<Track> tracks;
    juce::String       searchText_;
    juce::String       bpmMinText_;
    juce::String       bpmMaxText_;
    juce::String       durationMaxText_;

    // Sort state
    int  sortColumnId_  = ColTitle;
    bool sortForwards_  = true;

    juce::File libraryFile = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                                 .getChildFile("trackLibrary.csv");

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlaylistComponent)
};

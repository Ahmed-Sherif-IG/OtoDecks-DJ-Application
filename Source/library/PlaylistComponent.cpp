#include "PlaylistComponent.h"
#include "../gui/DeckLoadCellComponent.h"
#include "../shared/CustomLookAndFeel.h"

namespace
{
    constexpr auto libraryHeader = "# OtoDecks track library v2";
}

//==============================================================================
PlaylistComponent::PlaylistComponent()
{
    auto& header = tableComponent.getHeader();
    header.addColumn("Title",    ColTitle,    260, 100, -1, juce::TableHeaderComponent::defaultFlags);
    header.addColumn("Artist",   ColArtist,   170, 80, -1, juce::TableHeaderComponent::defaultFlags);
    header.addColumn("Duration", ColDuration,  84, 60, -1, juce::TableHeaderComponent::defaultFlags);
    header.addColumn("BPM",      ColBPM,       68, 50, -1, juce::TableHeaderComponent::defaultFlags);
    header.addColumn("Actions",  ColLoad,     220, 160, -1, juce::TableHeaderComponent::notSortable);
    header.setSortColumnId(ColTitle, true);

    header.setColour(juce::TableHeaderComponent::backgroundColourId, CustomLookAndFeel::colour(CustomLookAndFeel::panelAltColourValue));
    header.setColour(juce::TableHeaderComponent::textColourId,       CustomLookAndFeel::colour(CustomLookAndFeel::textColourValue));

    tableComponent.setModel(this);
    tableComponent.setRowHeight(28);
    tableComponent.setColour(juce::ListBox::backgroundColourId, CustomLookAndFeel::colour(CustomLookAndFeel::panelColourValue));
    tableComponent.setColour(juce::ListBox::outlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(tableComponent);

    addTracksButton.addListener(this);
    addTracksButton.setButtonText("+ Add Tracks");
    addTracksButton.setColour(juce::TextButton::buttonColourId,
                              CustomLookAndFeel::colour(CustomLookAndFeel::accentBlueValue));
    addAndMakeVisible(addTracksButton);

    trackCountLabel.setFont(juce::Font(juce::FontOptions(12.0f).withStyle("Bold")));
    trackCountLabel.setColour(juce::Label::textColourId,
                              CustomLookAndFeel::colour(CustomLookAndFeel::mutedTextColourValue));
    trackCountLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(trackCountLabel);

    searchBox.setTextToShowWhenEmpty("Search tracks, artists, BPM...", CustomLookAndFeel::colour(CustomLookAndFeel::mutedTextColourValue));
    searchBox.addListener(this);
    searchBox.setColour(juce::TextEditor::backgroundColourId, CustomLookAndFeel::colour(CustomLookAndFeel::panelAltColourValue));
    searchBox.setColour(juce::TextEditor::textColourId,       CustomLookAndFeel::colour(CustomLookAndFeel::textColourValue));
    searchBox.setColour(juce::TextEditor::outlineColourId,    CustomLookAndFeel::colour(CustomLookAndFeel::outlineColourValue));
    searchBox.setColour(juce::TextEditor::focusedOutlineColourId, CustomLookAndFeel::colour(CustomLookAndFeel::accentBlueValue));
    searchBox.setFont(juce::Font(juce::FontOptions(13.0f)));
    addAndMakeVisible(searchBox);

    loadLibrary();
}

PlaylistComponent::~PlaylistComponent()
{
    tableComponent.setModel(nullptr);
}

//==============================================================================
void PlaylistComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);

    juce::DropShadow shadow(juce::Colours::black.withAlpha(0.38f), 16, { 0, 8 });
    shadow.drawForRectangle(g, bounds.toNearestInt());

    juce::ColourGradient background(CustomLookAndFeel::colour(CustomLookAndFeel::panelRaisedColourValue).brighter(0.03f),
                                    bounds.getTopLeft(),
                                    CustomLookAndFeel::colour(CustomLookAndFeel::panelColourValue),
                                    bounds.getBottomLeft(),
                                    false);
    g.setGradientFill(background);
    g.fillRoundedRectangle(bounds, 18.0f);

    auto topGlow = bounds.removeFromTop(42.0f);
    g.setColour(juce::Colours::white.withAlpha(0.035f));
    g.fillRoundedRectangle(topGlow, 18.0f);

    g.setColour(CustomLookAndFeel::colour(CustomLookAndFeel::outlineColourValue).withAlpha(0.95f));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1.0f), 18.0f, 1.2f);
}

void PlaylistComponent::resized()
{
    auto area   = getLocalBounds().reduced(12);
    auto topRow = area.removeFromTop(40);

    addTracksButton.setBounds(topRow.removeFromLeft(148));
    topRow.removeFromLeft(10);
    searchBox.setBounds(topRow.removeFromLeft(320));
    topRow.removeFromLeft(10);
    trackCountLabel.setBounds(topRow);

    area.removeFromTop(8);
    tableComponent.setBounds(area);
}

//==============================================================================
int PlaylistComponent::getNumRows()
{
    return static_cast<int>(getFilteredIndices().size());
}

std::vector<int> PlaylistComponent::getFilteredIndices() const
{
    std::vector<int> indices;
    juce::String query = searchText_.toLowerCase().trim();

    for (int i = 0; i < static_cast<int>(tracks.size()); ++i)
    {
        if (query.isEmpty()
            || tracks[i].title.toLowerCase().contains(query)
            || tracks[i].artist.toLowerCase().contains(query)
            || juce::String(tracks[i].bpm, 1).contains(query))
        {
            indices.push_back(i);
        }
    }
    return indices;
}

void PlaylistComponent::paintRowBackground(juce::Graphics& g,
                                           int rowNumber,
                                           int width, int height,
                                           bool rowIsSelected)
{
    const auto evenColour    = CustomLookAndFeel::colour(CustomLookAndFeel::panelColourValue).brighter(0.03f);
    const auto oddColour     = CustomLookAndFeel::colour(CustomLookAndFeel::panelAltColourValue).darker(0.10f);
    const auto selectedColor = CustomLookAndFeel::colour(CustomLookAndFeel::accentBlueValue).withAlpha(0.24f);

    // Check if this row is the now-playing track
    bool isNowPlaying = false;
    if (nowPlayingFile_.existsAsFile())
    {
        auto fi = getFilteredIndices();
        if (rowNumber < static_cast<int>(fi.size()))
        {
            const int idx = fi[static_cast<size_t>(rowNumber)];
            if (idx >= 0 && idx < static_cast<int>(tracks.size()))
                isNowPlaying = (tracks[static_cast<size_t>(idx)].file == nowPlayingFile_);
        }
    }

    const auto baseColour = rowIsSelected ? selectedColor : ((rowNumber % 2 == 0) ? evenColour : oddColour);
    g.fillAll(baseColour);

    if (isNowPlaying)
    {
        // Accent underline at bottom of row
        g.setColour(CustomLookAndFeel::colour(CustomLookAndFeel::accentGreenValue).withAlpha(0.85f));
        g.fillRect(0, height - 2, width, 2);

        // Subtle green tint overlay
        g.setColour(CustomLookAndFeel::colour(CustomLookAndFeel::accentGreenValue).withAlpha(0.08f));
        g.fillAll();
    }
}

void PlaylistComponent::setNowPlayingFile(const juce::File& file)
{
    nowPlayingFile_ = file;
    tableComponent.repaint();
}

void PlaylistComponent::paintCell(juce::Graphics& g,
                                  int rowNumber,
                                  int columnId,
                                  int width, int height,
                                  bool /*rowIsSelected*/)
{
    auto filtered = getFilteredIndices();
    if (rowNumber >= static_cast<int>(filtered.size())) return;

    const Track& t = tracks[static_cast<size_t>(filtered[rowNumber])];
    juce::String text;
    juce::Justification just = juce::Justification::centredLeft;

    switch (columnId)
    {
        case ColTitle:
            text = sanitiseDisplayText(t.title);
            break;
        case ColArtist:
            text = t.artist.isEmpty() ? "--" : sanitiseDisplayText(t.artist);
            break;
        case ColDuration:
            just = juce::Justification::centred;
            if (t.durationSeconds > 0.0)
            {
                const int m = static_cast<int>(t.durationSeconds) / 60;
                const int s = static_cast<int>(t.durationSeconds) % 60;
                text = juce::String(m) + ":" + juce::String(s).paddedLeft('0', 2);
            }
            else
            {
                text = "--";
            }
            break;
        case ColBPM:
            just = juce::Justification::centred;
            text = (t.bpm > 0.0) ? juce::String(t.bpm, 1) : "--";
            break;
        default:
            return;
    }

    g.setColour(CustomLookAndFeel::colour(CustomLookAndFeel::textColourValue));
    g.setFont(juce::Font(juce::FontOptions(columnId == ColTitle ? 13.5f : 12.5f)
                         .withStyle(columnId == ColTitle ? "Bold" : "Regular")));
    g.drawText(text, juce::Rectangle<int>(6, 0, width - 12, height), just, true);
}

juce::Component* PlaylistComponent::refreshComponentForCell(int rowNumber,
                                                            int columnId,
                                                            bool /*isRowSelected*/,
                                                            juce::Component* existing)
{
    if (columnId != ColLoad) return nullptr;

    auto filtered = getFilteredIndices();
    if (rowNumber >= static_cast<int>(filtered.size())) return nullptr;

    auto* cell = dynamic_cast<DeckLoadCellComponent*>(existing);
    if (cell == nullptr)
    {
        cell = new DeckLoadCellComponent(
            [this](int row, int deck)
            {
                auto fi = getFilteredIndices();
                if (row < 0 || row >= static_cast<int>(fi.size())) return;
                const int idx = fi[row];
                if (idx < 0 || idx >= static_cast<int>(tracks.size())) return;
                const Track& t = tracks[static_cast<size_t>(idx)];
                if (loadTrackToDeck)
                    loadTrackToDeck(t.file, deck);
                if (onNowPlaying)
                {
                    const juce::String display = t.artist.isEmpty() ? t.title : t.artist + " - " + t.title;
                    onNowPlaying(deck, display);
                }
            },
            [this](int row)
            {
                auto fi = getFilteredIndices();
                if (row < 0 || row >= static_cast<int>(fi.size())) return;
                const int idx = fi[row];
                tracks.erase(tracks.begin() + idx);
                saveLibrary();
                updateTrackCountLabel();
                tableComponent.deselectAllRows();
                juce::MessageManager::callAsync([this]()
                {
                    if (tableComponent.isShowing())
                        tableComponent.updateContent();
                    tableComponent.repaint();
                });
            });
    }

    cell->setRow(rowNumber);
    return cell;
}

void PlaylistComponent::sortOrderChanged(int newSortColumnId, bool isForwards)
{
    sortColumnId_ = newSortColumnId;
    sortForwards_ = isForwards;

    auto cmp = [&](const Track& a, const Track& b) -> bool
    {
        const auto keyA = buildSortKey(a, newSortColumnId);
        const auto keyB = buildSortKey(b, newSortColumnId);

        int comparison = 0;
        if (newSortColumnId == ColDuration || newSortColumnId == ColBPM)
        {
            if (keyA.number < keyB.number) comparison = -1;
            else if (keyA.number > keyB.number) comparison = 1;
        }
        else
        {
            comparison = keyA.text.compareNatural(keyB.text, true);
        }

        if (comparison == 0)
            comparison = a.file.getFullPathName().compareIgnoreCase(b.file.getFullPathName());

        return isForwards ? (comparison < 0) : (comparison > 0);
    };

    std::stable_sort(tracks.begin(), tracks.end(), cmp);
    tableComponent.updateContent();
    tableComponent.repaint();
}

//==============================================================================
void PlaylistComponent::buttonClicked(juce::Button* button)
{
    if (button != &addTracksButton) return;

    fileChooser.reset(new juce::FileChooser("Select audio files...",
                                             {},
                                             "*.mp3;*.wav;*.aiff;*.flac;*.ogg",
                                             true));

    fileChooser->launchAsync(
        juce::FileBrowserComponent::openMode
        | juce::FileBrowserComponent::canSelectFiles
        | juce::FileBrowserComponent::canSelectMultipleItems,
        [this](const juce::FileChooser& fc)
        {
            for (auto& file : fc.getResults())
            {
                if (!file.existsAsFile()) continue;

                bool exists = false;
                for (auto& t : tracks)
                    if (t.file == file) { exists = true; break; }
                if (exists) continue;

                Track newTrack;
                newTrack.file     = file;
                newTrack.title    = sanitiseDisplayText(file.getFileNameWithoutExtension());
                newTrack.fileSize = file.getSize();
                readMetadata(newTrack);
                tracks.push_back(std::move(newTrack));
            }

            tableComponent.updateContent();
            tableComponent.repaint();
            saveLibrary();
            updateTrackCountLabel();
        });
}

void PlaylistComponent::textEditorTextChanged(juce::TextEditor& editor)
{
    if (&editor == &searchBox)
    {
        searchText_ = editor.getText();
        tableComponent.updateContent();
        tableComponent.repaint();
    }
}

//==============================================================================
void PlaylistComponent::readMetadata(Track& track)
{
    juce::AudioFormatManager afm;
    afm.registerBasicFormats();

    std::unique_ptr<juce::AudioFormatReader> reader(afm.createReaderFor(track.file));

    if (reader != nullptr)
    {
        track.durationSeconds = static_cast<double>(reader->lengthInSamples) / reader->sampleRate;

        const auto& meta = reader->metadataValues;
        if (meta.size() > 0)
        {
            static const char* titleKeys[]  = { "title", "TITLE", "TIT2", nullptr };
            static const char* artistKeys[] = { "artist", "ARTIST", "TPE1", nullptr };

            for (int i = 0; titleKeys[i] != nullptr; ++i)
            {
                juce::String v = meta[titleKeys[i]];
                if (v.isNotEmpty())
                {
                    track.title = sanitiseDisplayText(v);
                    break;
                }
            }

            for (int i = 0; artistKeys[i] != nullptr; ++i)
            {
                juce::String v = meta[artistKeys[i]];
                if (v.isNotEmpty())
                {
                    track.artist = sanitiseDisplayText(v);
                    break;
                }
            }
        }
    }
}

//==============================================================================
void PlaylistComponent::saveLibrary()
{
    std::ofstream out(libraryFile.getFullPathName().toStdString(), std::ios::trunc);
    if (!out.is_open())
        return;

    out << libraryHeader << "\n";

    for (const auto& t : tracks)
    {
        out << escapeCsvField(sanitiseDisplayText(t.title)).toStdString() << ","
            << escapeCsvField(sanitiseDisplayText(t.artist)).toStdString() << ","
            << juce::String(t.durationSeconds, 6).toStdString() << ","
            << juce::String(t.bpm, 3).toStdString() << ","
            << escapeCsvField(t.file.getFullPathName()).toStdString() << "\n";
    }
}

void PlaylistComponent::loadLibrary()
{
    tracks.clear();

    std::ifstream in(libraryFile.getFullPathName().toStdString());
    if (!in.is_open())
    {
        updateTrackCountLabel();
        return;
    }

    std::string line;
    while (std::getline(in, line))
    {
        if (line.empty() || line == libraryHeader)
            continue;

        std::vector<std::string> fields;
        if (parseCsvLine(line, fields) && fields.size() >= 5)
        {
            const juce::File file(fields[4]);
            if (!file.existsAsFile())
                continue;

            Track t;
            t.title           = sanitiseDisplayText(juce::String::fromUTF8(fields[0].c_str()));
            t.artist          = sanitiseDisplayText(juce::String::fromUTF8(fields[1].c_str()));
            t.durationSeconds = parseDoubleSafe(juce::String::fromUTF8(fields[2].c_str()));
            t.bpm             = parseDoubleSafe(juce::String::fromUTF8(fields[3].c_str()));
            t.file            = file;
            t.fileSize        = file.getSize();

            if (t.title.isEmpty())
                t.title = sanitiseDisplayText(file.getFileNameWithoutExtension());

            tracks.push_back(std::move(t));
            continue;
        }

        std::stringstream ss(line);
        std::string title, path;
        if (!std::getline(ss, title, ','))
            continue;
        if (!std::getline(ss, path))
            continue;

        juce::File file(juce::String::fromUTF8(path.c_str()));
        if (!file.existsAsFile())
            continue;

        Track t;
        t.title = sanitiseDisplayText(juce::String::fromUTF8(title.c_str()));
        if (t.title.isEmpty())
            t.title = sanitiseDisplayText(file.getFileNameWithoutExtension());
        t.file = file;
        t.fileSize = file.getSize();
        tracks.push_back(std::move(t));
    }

    updateTrackCountLabel();
}

juce::String PlaylistComponent::sanitiseDisplayText(const juce::String& text)
{
    auto cleaned = text.trim().replaceCharacters("\r\n\t", "   ");
    cleaned = cleaned.replaceCharacter(juce::juce_wchar(0x2026), '.');

    juce::String result;
    result.preallocateBytes(cleaned.getNumBytesAsUTF8());

    bool previousWasSpace = false;
    for (auto ch : cleaned)
    {
        const bool keep = ch >= 32 || ch == juce::juce_wchar(0x060C) || ch == juce::juce_wchar(0x061B)
                       || ch == juce::juce_wchar(0x061F) || ch == juce::juce_wchar(0x066A);
        const auto value = keep ? ch : juce::juce_wchar(' ');
        const bool isSpace = juce::CharacterFunctions::isWhitespace(value);

        if (isSpace)
        {
            if (!previousWasSpace)
                result << ' ';
        }
        else
        {
            result << value;
        }

        previousWasSpace = isSpace;
    }

    return result.trim();
}

juce::String PlaylistComponent::escapeCsvField(const juce::String& text)
{
    auto escaped = text;
    escaped = escaped.replace("\"", "\"\"");
    return "\"" + escaped + "\"";
}

bool PlaylistComponent::parseCsvLine(const std::string& line, std::vector<std::string>& fields)
{
    fields.clear();
    std::string current;
    bool inQuotes = false;

    for (size_t i = 0; i < line.size(); ++i)
    {
        const char c = line[i];
        if (c == '"')
        {
            if (inQuotes && i + 1 < line.size() && line[i + 1] == '"')
            {
                current.push_back('"');
                ++i;
            }
            else
            {
                inQuotes = !inQuotes;
            }
        }
        else if (c == ',' && !inQuotes)
        {
            fields.push_back(current);
            current.clear();
        }
        else
        {
            current.push_back(c);
        }
    }

    if (inQuotes)
        return false;

    fields.push_back(current);
    return !fields.empty();
}

double PlaylistComponent::parseDoubleSafe(const juce::String& text)
{
    return text.trim().getDoubleValue();
}

PlaylistComponent::SortKey PlaylistComponent::buildSortKey(const Track& track, int columnId)
{
    SortKey key;
    switch (columnId)
    {
        case ColArtist:
            key.text = sanitiseDisplayText(track.artist);
            if (key.text.isEmpty()) key.text = "~";
            break;
        case ColDuration:
            key.number = track.durationSeconds;
            break;
        case ColBPM:
            key.number = track.bpm;
            break;
        case ColTitle:
        default:
            key.text = sanitiseDisplayText(track.title);
            break;
    }

    return key;
}

void PlaylistComponent::updateTrackCountLabel()
{
    trackCountLabel.setText("Tracks: " + juce::String(static_cast<int>(tracks.size())),
                            juce::dontSendNotification);
}

#include "PlaylistComponent.h"
#include "../gui/DeckLoadCellComponent.h"

namespace
{
    constexpr auto libraryHeader = "# OtoDecks track library v2";
}

//==============================================================================
PlaylistComponent::PlaylistComponent()
{
    // Table columns
    auto& header = tableComponent.getHeader();
    header.addColumn("Title",    ColTitle,    220, 80, -1, juce::TableHeaderComponent::defaultFlags);
    header.addColumn("Artist",   ColArtist,   140, 60, -1, juce::TableHeaderComponent::defaultFlags);
    header.addColumn("Duration", ColDuration,  70, 50, -1, juce::TableHeaderComponent::defaultFlags);
    header.addColumn("BPM",      ColBPM,       55, 40, -1, juce::TableHeaderComponent::defaultFlags);
    header.addColumn("Load",     ColLoad,     180, 120,-1, juce::TableHeaderComponent::notSortable);
    header.setSortColumnId(ColTitle, true);

    header.setColour(juce::TableHeaderComponent::backgroundColourId, juce::Colours::black);
    header.setColour(juce::TableHeaderComponent::textColourId,       juce::Colours::lightgrey);

    tableComponent.setModel(this);
    addAndMakeVisible(tableComponent);

    addTracksButton.addListener(this);
    addTracksButton.setButtonText("+  Add Tracks");
    addAndMakeVisible(addTracksButton);
    addAndMakeVisible(trackCountLabel);

    searchBox.setTextToShowWhenEmpty("Search tracks...", juce::Colours::grey);
    searchBox.addListener(this);
    searchBox.setColour(juce::TextEditor::backgroundColourId, juce::Colour::fromRGB(35,35,35));
    searchBox.setColour(juce::TextEditor::textColourId,       juce::Colours::white);
    searchBox.setColour(juce::TextEditor::outlineColourId,    juce::Colours::darkgrey);
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
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::grey);
    g.drawRect(getLocalBounds(), 1);
}

void PlaylistComponent::resized()
{
    auto area   = getLocalBounds().reduced(8);
    auto topRow = area.removeFromTop(36);

    addTracksButton.setBounds(topRow.removeFromLeft(130));
    topRow.removeFromLeft(8);
    searchBox.setBounds(topRow.removeFromLeft(220));
    topRow.removeFromLeft(8);
    trackCountLabel.setBounds(topRow.removeFromLeft(100));

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
            || tracks[i].artist.toLowerCase().contains(query))
        {
            indices.push_back(i);
        }
    }
    return indices;
}

void PlaylistComponent::paintRowBackground(juce::Graphics& g,
                                            int /*rowNumber*/,
                                            int /*width*/, int /*height*/,
                                            bool rowIsSelected)
{
    g.fillAll(rowIsSelected ? juce::Colour(50, 80, 120) : juce::Colour(28, 28, 28));
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

    switch (columnId)
    {
        case ColTitle:
            text = sanitiseDisplayText(t.title);
            break;
        case ColArtist:
            text = t.artist.isEmpty() ? "--" : sanitiseDisplayText(t.artist);
            break;
        case ColDuration:
            if (t.durationSeconds > 0.0)
            {
                int m = static_cast<int>(t.durationSeconds) / 60;
                int s = static_cast<int>(t.durationSeconds) % 60;
                text = juce::String(m) + ":" + juce::String(s).paddedLeft('0', 2);
            }
            else
                text = "--";
            break;
        case ColBPM:
            text = (t.bpm > 0.0) ? juce::String(t.bpm, 1) : "--";
            break;
        default:
            return;
    }

    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(juce::FontOptions(13.0f)));
    g.drawText(text, 6, 0, width - 10, height, juce::Justification::centredLeft, true);
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
                int idx = fi[row];
                if (idx < 0 || idx >= static_cast<int>(tracks.size())) return;
                const Track& t = tracks[static_cast<size_t>(idx)];
                if (loadTrackToDeck)
                    loadTrackToDeck(t.file, deck);
                if (onNowPlaying)
                {
                    juce::String display = t.artist.isEmpty()
                                         ? t.title
                                         : t.artist + " - " + t.title;
                    onNowPlaying(deck, display);
                }
            },
            [this](int row)
            {
                auto fi = getFilteredIndices();
                if (row < 0 || row >= static_cast<int>(fi.size())) return;
                int idx = fi[row];
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

                // Skip duplicates
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

    std::unique_ptr<juce::AudioFormatReader> reader(
        afm.createReaderFor(track.file));

    if (reader != nullptr)
    {
        track.durationSeconds = static_cast<double>(reader->lengthInSamples)
                                / reader->sampleRate;

        // Read metadata if available
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

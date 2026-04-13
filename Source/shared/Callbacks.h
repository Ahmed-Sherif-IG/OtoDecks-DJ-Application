#pragma once

#include <JuceHeader.h>
#include <functional>

// Load a file into a specific deck (1 or 2). Set on PlaylistComponent by MainComponent.
using LoadTrackToDeckFn   = std::function<void(juce::File, int)>;

// Returns the current playback speed of the opposite deck. Set on DeckGUI by MainComponent.
using GetOtherDeckSpeedFn = std::function<float()>;

// Load a playlist row into a specific deck. Used internally by DeckLoadCellComponent.
using LoadTrackByRowFn    = std::function<void(int rowIndex, int deckNumber)>;

// Remove a track row from the playlist. Used internally by DeckLoadCellComponent.
using RemoveTrackFn       = std::function<void(int rowIndex)>;

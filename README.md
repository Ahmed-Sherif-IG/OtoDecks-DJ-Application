# OtoDecks DJ Application

OtoDecks is a Windows desktop DJ application built with JUCE and C++.

The app now has a full dual-deck workflow, a central mixer, an integrated recording path, a searchable library, and the polished single-surface UI direction we built across the project.

## What The App Does Right Now

OtoDecks is organized into four main areas:

- `Deck A`
- `Mixer`
- `Deck B`
- `Track Library`

Each deck can load and play a track, set cues, create loops, trigger hotcues, sync tempo, and shape playback with filters and nudging. The mixer blends both decks together and can record the live output to WAV files. The library stores tracks, supports search and filters, and lets you load tracks directly to either deck.

## Current Feature Set

### Deck Workflow

Each deck includes:

- `PLAY`, `PAUSE`, `LOAD TRACK`, and `BEAT SYNC`
- `VOLUME` slider with `RESET VOL`
- `TEMPO` slider with reset and tempo range cycling
- `SEEK` slider
- `REPEAT` button to loop the loaded track when playback reaches the end
- drag-and-drop file loading
- BPM display
- elapsed / total time display

### Cue And Loop Controls

Each deck supports:

- `SET CUE`
- `JUMP TO CUE`
- manual loop start / loop end / clear loop
- quick loop buttons for `1/4`, `1/2`, `1`, `2`, `4`, and `8` bars

### Hotcues

Each deck has:

- 8 hotcue pads
- one `CLR` button to clear all hotcues on that deck
- hotcue state coloring for set vs empty pads

### Performance Controls

Each deck includes:

- `LOW CUT`
- `HIGH CUT`
- `DELAY`
- `NUDGE -`
- `NUDGE +`
- `TAP BPM`

Notes:

- `NUDGE -` and `NUDGE +` are toggle-style now, not hold-only
- the tempo range button cycles the slider range (`RNG 8%`, `RNG 16%`, `RNG 50%`)
- the tempo range button changes how wide the `TEMPO` slider can move, not the speed by itself

### Waveform Display

Each deck waveform area supports:

- waveform rendering through `juce::AudioThumbnail`
- playhead position indicator
- seek by clicking / dragging
- cue marker rendering
- loop region rendering
- beat marker overlay when BPM is available
- empty-state screen when no track is loaded

### Header Status Feedback

Each deck header now includes:

- a small circular status lamp beside the deck title
- `red` lamp when the deck is empty
- `green` lamp when a track is loaded
- scrolling marquee for long track titles so the full name can be read without overlapping the BPM / time area

### Mixer

The central mixer includes:

- crossfader
- crossfader curve modes: `LIN`, `EQP`, `CUT`
- master output control
- Deck A trim and Deck B trim
- 3-band EQ for each deck:
  - `BASS`
  - `MIDS`
  - `TREBLE`
- per-band `KILL` buttons
- `RESET A`
- `RESET MIX`
- `RESET B`
- VU metering for both decks

### Recording

The app can record the final mixed output:

- `REC` button lives inside the mixer, below the crossfader
- recording writes `.wav` files into the project `Recordings` folder
- live recording state updates in the mixer while recording

### Track Library

The lower library area supports:

- `+ Add Tracks`
- search by text
- BPM min filter
- BPM max filter
- max duration filter in minutes
- sortable table columns
- `LOAD A`
- `LOAD B`
- `DELETE`
- now-playing row highlight

## Persistence And Generated Files

The app currently writes to these locations:

- Library CSV: `Documents/trackLibrary.csv`
- Track history CSV: `Documents/OtoDecksTrackHistory.csv`
- Recordings: `Recordings/*.wav` inside the project folder

## Running The App

### Option 1: Run The built Release executable

After a Release build, the Visual Studio output is:

`Builds/VisualStudio2022/x64/Release/App/OtoDeck.exe`

### Option 2: Package A ready-to-run local executable

There is now a helper script:

[`package-otodeck.ps1`](C:\Ahmed Sherif Files\Work\Git Project Files\OtoDecks DJ Application\package-otodeck.ps1)

Run it from PowerShell:

```powershell
.\package-otodeck.ps1
```

That copies the Release build into a simple top-level app file:

`OtoDeck DJ.exe`

This is the easiest way to launch the app locally without reopening Visual Studio every time.

## Build Requirements

- Windows
- Visual Studio 2022
- JUCE
- x64 build environment

## Build Steps

1. Open [`OtoDeck.jucer`](C:\Ahmed Sherif Files\Work\Git Project Files\OtoDecks DJ Application\OtoDeck.jucer) in Projucer if you need to regenerate project files.
2. Open [`Builds/VisualStudio2022/OtoDeck.sln`](C:\Ahmed Sherif Files\Work\Git Project Files\OtoDecks DJ Application\Builds\VisualStudio2022\OtoDeck.sln)
3. Build `Release | x64`
4. Run the built app directly, or run [`package-otodeck.ps1`](C:\Ahmed Sherif Files\Work\Git Project Files\OtoDecks DJ Application\package-otodeck.ps1) to refresh `OtoDeck DJ.exe`

## Project Structure

```text
Source/
  audio/
    DJAudioPlayer.h / .cpp
    BPMAnalyser.h / .cpp

  gui/
    MainComponent.h / .cpp
    DeckGUI.h / .cpp
    WaveformDisplay.h / .cpp
    MixerPanel.h / .cpp
    VUMeter.h
    DeckLoadCellComponent.h

  library/
    PlaylistComponent.h / .cpp

  shared/
    CustomLookAndFeel.h
    DeckState.h
    Callbacks.h

  Main.cpp
```

## Key Files

- [`Source/audio/DJAudioPlayer.cpp`](C:\Ahmed Sherif Files\Work\Git Project Files\OtoDecks DJ Application\Source\audio\DJAudioPlayer.cpp) - deck audio engine
- [`Source/gui/DeckGUI.cpp`](C:\Ahmed Sherif Files\Work\Git Project Files\OtoDecks DJ Application\Source\gui\DeckGUI.cpp) - deck controls and deck UI behavior
- [`Source/gui/MixerPanel.cpp`](C:\Ahmed Sherif Files\Work\Git Project Files\OtoDecks DJ Application\Source\gui\MixerPanel.cpp) - mixer layout and mixer controls
- [`Source/gui/MainComponent.cpp`](C:\Ahmed Sherif Files\Work\Git Project Files\OtoDecks DJ Application\Source\gui\MainComponent.cpp) - top-level layout, audio routing, and recording
- [`Source/gui/WaveformDisplay.cpp`](C:\Ahmed Sherif Files\Work\Git Project Files\OtoDecks DJ Application\Source\gui\WaveformDisplay.cpp) - waveform display and interaction
- [`Source/library/PlaylistComponent.cpp`](C:\Ahmed Sherif Files\Work\Git Project Files\OtoDecks DJ Application\Source\library\PlaylistComponent.cpp) - library browsing, filters, and persistence
- [`Source/shared/CustomLookAndFeel.h`](C:\Ahmed Sherif Files\Work\Git Project Files\OtoDecks DJ Application\Source\shared\CustomLookAndFeel.h) - app-wide visual styling

## UI Direction Status

The app has already gone through the planned UI foundation pass:

- shared design system pass
- deck redesign pass
- mixer redesign pass
- playlist polish pass
- layout pass
- state feedback pass

The detailed UI roadmap is kept in:

- [`UI update.md`](C:\Ahmed Sherif Files\Work\Git Project Files\OtoDecks DJ Application\UI%20update.md)

## Current Keyboard Shortcuts

- `1` - play / pause Deck A
- `2` - play / pause Deck B
- `Space` - play / pause Deck A
- `Q` - set cue on Deck A
- `W` - jump to cue on Deck A
- `O` - set cue on Deck B
- `P` - jump to cue on Deck B
- `S` - trigger Deck B beat sync

## Notes

- The packaged `.exe` is intentionally not committed to git.
- Recorded WAV files are also ignored by git.
- The app is designed around the current Windows desktop layout and Release build flow.

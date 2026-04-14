# OtoDecks DJ Application

OtoDecks is a desktop DJ application built with **JUCE** in **C++**.

It is structured around a dual-deck workflow:
- **Deck A** and **Deck B** for playback and performance controls
- a central **Mixer Panel** for trim, EQ, crossfader, master level, and metering
- a lower **Library / Playlist** section for loading and managing tracks

The project started as an academic JUCE DJ app and is being actively pushed toward a more production-quality desktop DJ tool with clearer architecture, better UX, stronger audio controls, and a more professional UI.

---

## Current Feature Set

### Playback / Deck Controls
Each deck supports:
- Play
- Pause / Stop
- Load Track
- Position slider
- Volume slider
- Speed slider
- BPM-aware sync fallback logic
- Cue point set / jump
- Loop start / loop end / clear loop
- Tap BPM
- Reset volume to default
- Reset speed to default
- Drag-and-drop track loading

### Waveform
Each deck includes a waveform display with:
- waveform rendering via `juce::AudioThumbnail`
- playback position indicator
- click / drag seek support
- loop region overlay
- cue marker overlay
- beat grid overlay when BPM is available
- per-deck colour styling

### Mixer
The mixer panel currently includes:
- Crossfader
- Master level
- Deck trim controls
- 3-band EQ per deck
- Per-deck VU meters
- Reset EQ A / Reset EQ B / Reset mixer controls

### Audio Features
`DJAudioPlayer` currently supports:
- file loading through JUCE readers
- transport playback
- resampling-based speed control
- gain, trim, and crossfader gain stacking
- loop handling
- cue points
- 3-band EQ
- low-cut / high-cut style filter toggles
- delay support scaffolding
- RMS metering
- BPM storage

### Library / Playlist
The lower browser supports:
- adding tracks from disk
- metadata extraction (where available)
- search filtering
- column sorting
- CSV persistence
- loading tracks to Deck A or Deck B
- delete action per row
- now-playing text updates back to the deck header

---

## Project Structure

```text
Source/
├── audio/
│   ├── DJAudioPlayer.h / .cpp
│   ├── BPMAnalyser.h / .cpp
│
├── gui/
│   ├── MainComponent.h / .cpp
│   ├── DeckGUI.h / .cpp
│   ├── WaveformDisplay.h / .cpp
│   ├── MixerPanel.h / .cpp
│   ├── VUMeter.h
│   └── DeckLoadCellComponent.h
│
├── library/
│   ├── PlaylistComponent.h / .cpp
│
├── shared/
│   ├── CustomLookAndFeel.h
│   ├── DeckState.h
│   └── Callbacks.h
│
└── Main.cpp
```

---

## How The App Works

### 1. Application startup
Entry point:
- `Source/Main.cpp`

`OtoDecksApplication` creates the main window and sets `MainComponent` as the content component.

### 2. Main UI composition
Main UI root:
- `Source/gui/MainComponent.h`
- `Source/gui/MainComponent.cpp`

`MainComponent` owns the main runtime objects:
- two `DJAudioPlayer` instances
- two `DeckGUI` instances
- one `MixerPanel`
- one `PlaylistComponent`
- one `juce::MixerAudioSource`
- one shared `CustomLookAndFeel`

At a high level:
- **DeckGUI 1** controls **player1**
- **DeckGUI 2** controls **player2**
- **MixerPanel** controls trim, EQ, crossfader, and master interaction across both players
- **PlaylistComponent** loads tracks into either deck

### 3. Audio flow
Audio flow is centered in `MainComponent`.

- `player1` and `player2` are added into `mixerSource`
- `mixerSource` writes into the output buffer in `MainComponent::getNextAudioBlock(...)`
- `masterGain_` is applied to the final output buffer after the two deck sources are mixed

### 4. Deck playback model
Playback engine:
- `Source/audio/DJAudioPlayer.h`
- `Source/audio/DJAudioPlayer.cpp`

Each deck player contains:
- a `juce::AudioTransportSource`
- a `juce::ResamplingAudioSource`
- filter / EQ stages
- loop state
- cue state
- RMS metering
- BPM state

Core responsibilities of `DJAudioPlayer`:
- load a file
- start / stop playback
- set gain / trim / crossfader gain
- change speed
- move playback position
- manage cue and loop state
- update EQ and filter coefficients
- expose current deck state via `DeckState`

### 5. Deck UI layer
Deck UI:
- `Source/gui/DeckGUI.h`
- `Source/gui/DeckGUI.cpp`

Each `DeckGUI` is a control surface bound to one `DJAudioPlayer`.

Responsibilities:
- presents deck title, status, BPM, and time
- handles transport buttons
- handles volume / speed / position sliders
- handles cue and loop actions
- handles filter toggle buttons
- shows waveform
- reacts to playback state on a timer
- triggers BPM analysis and updates waveform beat markers

### 6. Waveform system
Waveform component:
- `Source/gui/WaveformDisplay.h`
- `Source/gui/WaveformDisplay.cpp`

Responsibilities:
- render waveform thumbnail
- render playback position marker
- render cue marker
- render loop region
- render beat grid markers
- allow seek by clicking or dragging in the waveform area

### 7. Mixer system
Mixer component:
- `Source/gui/MixerPanel.h`
- `Source/gui/MixerPanel.cpp`

Responsibilities:
- crossfader control
- master output control
- deck trim controls
- deck EQ controls
- reset controls for neutral/default state
- VU meter display per deck

Crossfader logic currently uses equal-power style blending and then applies a master-related scaling layer before sending gain values to both deck players.

### 8. Library system
Library / browser:
- `Source/library/PlaylistComponent.h`
- `Source/library/PlaylistComponent.cpp`

Responsibilities:
- maintain the track list
- read track metadata
- display searchable/sortable rows
- save/load CSV library state
- expose callbacks for loading tracks into Deck A or Deck B
- notify decks of now-playing display text

### 9. Shared types
Shared support files:
- `Source/shared/DeckState.h`
- `Source/shared/Callbacks.h`
- `Source/shared/CustomLookAndFeel.h`

These provide:
- a consistent state snapshot for deck UI updates
- callback typedefs between components
- centralized project styling/theme values

---

## Important Classes

### `DJAudioPlayer`
Main audio engine for one deck.

Key areas:
- playback transport
- resampling / speed
- cue / loop state
- EQ / filters
- level metering
- BPM storage

### `DeckGUI`
Deck-facing control surface.

Key areas:
- user interaction for one deck
- timer-driven UI refresh
- waveform integration
- sync / cue / loop controls
- filter toggles and slider controls

### `MixerPanel`
Shared center mixing surface.

Key areas:
- crossfader
- trim
- EQ
- master level
- VU monitoring
- reset/default actions

### `PlaylistComponent`
Track browser and persistence layer.

Key areas:
- track management
- searching / sorting
- loading to deck
- CSV persistence

### `BPMAnalyser`
Background-thread BPM analysis helper.

Key areas:
- async analysis on a worker thread
- result callback on message thread

---

## Build Information

### Project file
Main JUCE project:
- `OtoDeck.jucer`

### Visual Studio solution
Generated build target:
- `Builds/VisualStudio2022/OtoDeck.sln`

### JUCE modules
The project uses JUCE modules referenced in `OtoDeck.jucer`.
If source files are moved, **re-save the `.jucer` file in Projucer** to regenerate Visual Studio project paths correctly.

This matters because stale generated Visual Studio paths previously caused build errors after source-tree restructuring.

### Typical Windows workflow
1. Open `OtoDeck.jucer` in Projucer
2. Re-save the project if build files are stale
3. Open `Builds/VisualStudio2022/OtoDeck.sln`
4. Build `Debug | x64` or `Release | x64`
5. Run the generated executable

---

## Runtime Usage

### Loading tracks
You can load tracks by:
- pressing **LOAD TRACK** on a deck
- dragging a file onto a deck
- using **LOAD A** or **LOAD B** from the library table

### Mixing
Use the center mixer to:
- control trim for each deck
- adjust LOW / MID / HIGH EQ
- view deck levels on VU meters
- adjust master level
- blend decks with the crossfader

### Performance workflow
Typical flow:
1. Add tracks to the library
2. Load one track into each deck
3. Start playback on a deck
4. Use waveform seek, cue, loop, and speed controls
5. Blend between decks with trim, EQ, and crossfader
6. Monitor levels via VU meters

---

## Keyboard Shortcuts
Current shortcuts implemented in `MainComponent`:

- `1` → play/stop Deck 1
- `2` → play/stop Deck 2
- `Space` → play/stop Deck 1
- `Q` → set cue Deck 1
- `W` → jump cue Deck 1
- `O` → set cue Deck 2
- `P` → jump cue Deck 2
- `S` → sync active deck logic

---

## Persistence

### Library file
The library currently persists to:
- `Documents/trackLibrary.csv`

The CSV stores track/library information so the track list can be restored across runs.

---

## Current Design / Architecture Notes

This project is mid-transition from a coursework-style prototype to a more production-oriented DJ desktop application.

Important current realities:
- the app works as a dual-deck DJ tool already
- the UI is actively being redesigned
- layout and wording are being improved through runtime screenshot iteration
- the project structure has already been reorganized into `audio`, `gui`, `library`, and `shared`
- generated Visual Studio files may need regeneration after structural moves

---

## Known Limitations / Active Work

This project is still under active overhaul.

Current active areas of improvement include:
- making the mixer layout more professional and readable
- shrinking the library footprint so the performance surface dominates
- refining deck hierarchy and visual identity
- validating filter behavior against UI labels
- improving polish and responsiveness across different window sizes

Potential technical caveats while iterating:
- some UI changes may be ahead of documentation if the project is in active design work
- generated Visual Studio project files can drift if `.jucer` is not re-saved after moves
- some features are present in scaffold form and still need deeper polish or validation

---

## File Reference Summary

### Entry / App
- `Source/Main.cpp`

### Audio
- `Source/audio/DJAudioPlayer.h`
- `Source/audio/DJAudioPlayer.cpp`
- `Source/audio/BPMAnalyser.h`
- `Source/audio/BPMAnalyser.cpp`

### GUI
- `Source/gui/MainComponent.h`
- `Source/gui/MainComponent.cpp`
- `Source/gui/DeckGUI.h`
- `Source/gui/DeckGUI.cpp`
- `Source/gui/WaveformDisplay.h`
- `Source/gui/WaveformDisplay.cpp`
- `Source/gui/MixerPanel.h`
- `Source/gui/MixerPanel.cpp`
- `Source/gui/VUMeter.h`
- `Source/gui/DeckLoadCellComponent.h`

### Library
- `Source/library/PlaylistComponent.h`
- `Source/library/PlaylistComponent.cpp`

### Shared
- `Source/shared/DeckState.h`
- `Source/shared/Callbacks.h`
- `Source/shared/CustomLookAndFeel.h`

### Project / Build
- `OtoDeck.jucer`
- `Builds/VisualStudio2022/OtoDeck.sln`

---

## Suggested Next Documentation To Add Later

If you want, the next documentation pass can split this into:
- `README.md` → user/developer overview
- `ARCHITECTURE.md` → deep internal structure
- `BUILD.md` → build and regeneration workflow
- `UI.md` → UI overhaul brief and design direction

For now, this README is meant to be the single practical overview of how the project works.

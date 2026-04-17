# OtoDecks DJ Application

OtoDecks is a desktop DJ application built with **JUCE** in **C++**.

Dual-deck workflow:
- **Deck A** and **Deck B** — full playback and performance controls
- Central **Mixer Panel** — trim, EQ, crossfader, master level, metering
- Lower **Library / Playlist** — track management and loading

---

## Current Feature Set

### Playback / Deck Controls
Each deck supports:
- Play / Pause / Stop
- Load Track (button, drag-and-drop, or library)
- Position slider with seek
- Volume slider with reset
- Speed slider with reset
- Drag-and-drop file loading
- BPM-aware sync (matches other deck's BPM or speed)
- Tap BPM

### Cue System
- Set cue point at current position
- Jump to cue point
- Cue marker rendered on waveform

### Loop System
- Manual loop in / loop out / clear loop
- Bar-length loop buttons: **1/4, 1/2, 1, 2, 4, 8 bars** — auto-calculated from BPM
- Loop region rendered on waveform

### Hotcue Pads (8 per deck)
- 8 numbered pad buttons per deck
- **Left-click** unset pad → sets hotcue at current position
- **Left-click** set pad → jumps to stored position
- **Shift + click** → clears that pad
- **CLR button** → clears all 8 pads at once
- Pads auto-clear when a new track is loaded
- Set pads light up in colour; empty pads dimmed

### Waveform Display
- Waveform rendering via `juce::AudioThumbnail`
- Playback position indicator
- Click / drag seek support
- Loop region overlay
- Cue marker overlay
- Beat grid overlay (when BPM is available)
- Per-deck colour styling
- **Hover time tooltip** — shows MM:SS position under cursor

### Filters
- Low-pass filter toggle (LP button)
- High-pass filter toggle (HP button)
- Active filters highlighted in deck colour

### Mixer
- **Crossfader** with three curve modes:
  - **LIN** — linear blend
  - **EQP** — equal-power blend (default)
  - **CUT** — hard cut (scratch mode)
- Master level slider
- Per-deck trim sliders
- **3-band EQ per deck** (LOW / MID / HIGH) displayed as rotary knobs
- Reset EQ A / Reset EQ B / Reset mixer buttons
- Per-deck **VU meters** with:
  - RMS level bar
  - Peak-hold indicator
  - **Clip LED** — latches red on peak ≥ 0.98, click to reset

### Audio Engine (`DJAudioPlayer`)
- File loading via JUCE readers
- Transport playback
- Resampling-based speed control with smooth ramping
- Gain / trim / crossfader gain stacking
- Loop state with `setLoopFromCurrentPosition(durationSeconds)`
- Cue points
- 8-slot hotcue storage per deck
- 3-band IIR EQ (LOW 100 Hz / MID 1 kHz / HIGH 10 kHz)
- Low-pass and high-pass filter toggles
- Delay effect scaffold
- RMS metering
- BPM storage

### Library / Playlist
- Add tracks from disk (multi-select)
- Metadata extraction (title, artist, duration, BPM)
- Text search filter
- Column sorting
- **Now-playing row highlight** — green underline on currently loaded track
- Row height 28px for readability
- CSV persistence (`Documents/trackLibrary.csv`)
- Load to Deck A or Deck B
- Delete track from library

### UI & Theming
- Custom dark theme — `CustomLookAndFeel`
- Color palette: deep near-black panels, electric cyan-blue (Deck A), vivid orange (Deck B)
- Section panel backgrounds grouping Transport / Controls / Effects per deck
- Rotary EQ knobs with centre-zero arc and gradient body
- Minimum window size: **1200 × 700**
- Ambient glow ellipses on main background per deck color

---

## Project Structure

```
Source/
├── audio/
│   ├── DJAudioPlayer.h / .cpp      — audio engine for one deck
│   ├── BPMAnalyser.h / .cpp        — background BPM detection
│
├── gui/
│   ├── MainComponent.h / .cpp      — root UI, audio callback, keyboard shortcuts
│   ├── DeckGUI.h / .cpp            — deck control surface
│   ├── WaveformDisplay.h / .cpp    — waveform rendering and interaction
│   ├── MixerPanel.h / .cpp         — crossfader, EQ, VU meters
│   ├── VUMeter.h                   — level metering component
│   └── DeckLoadCellComponent.h     — table cell for library load/delete actions
│
├── library/
│   └── PlaylistComponent.h / .cpp  — track browser and CSV persistence
│
├── shared/
│   ├── CustomLookAndFeel.h         — theme colours and custom draw overrides
│   ├── DeckState.h                 — deck state snapshot struct
│   └── Callbacks.h                 — callback typedefs
│
└── Main.cpp                        — JUCE app entry point
```

---

## How The App Works

### 1. Startup
`Source/Main.cpp` → `OtoDecksApplication` → creates `MainWindow` → sets `MainComponent` as content.

### 2. Main UI composition (`MainComponent`)
Owns:
- `DJAudioPlayer player1, player2`
- `DeckGUI deckGUI1, deckGUI2`
- `MixerPanel mixerPanel`
- `PlaylistComponent playlistComponent`
- `juce::MixerAudioSource mixerSource`
- `CustomLookAndFeel customLook`
- `std::atomic<float> masterGain_`

Layout: `[DeckGUI1] [MixerPanel] [DeckGUI2]` across top 73% of window, `[PlaylistComponent]` below.

### 3. Audio signal flow
```
DJAudioPlayer (player1)  ──┐
                            ├── MixerAudioSource ── masterGain_ ── audio output
DJAudioPlayer (player2)  ──┘
```
`MainComponent::getNextAudioBlock()` calls `mixerSource`, then applies `masterGain_` to the final buffer.

### 4. Deck audio engine (`DJAudioPlayer`)
Chain per deck:
```
File → AudioFormatReaderSource → AudioTransportSource → ResamplingAudioSource
     → EQ filters (IIR low/mid/high) → LP/HP filters → delay → gain → output
```
Gain stacking: `effectiveGain = userGain × trimGain × crossfaderGain`

### 5. Deck UI (`DeckGUI`)
- Timer at 20 Hz polls `player.getState()` and updates all labels, button states, waveform position
- Hotcue pads: click routes through `buttonClicked()` → `player.setHotcue/jumpToHotcue/clearHotcue`
- Bar-length loops: calculate `bars × (60/bpm × 4)` seconds → `player.setLoopFromCurrentPosition()`
- BPM analysis runs on background thread via `BPMAnalyser`; result callback updates `bpmLabel` and waveform beat grid

### 6. Waveform (`WaveformDisplay`)
- `juce::AudioThumbnail` renders waveform
- `mouseMove` → updates `hoverX_` → `paint()` draws MM:SS tooltip pill
- `mouseDown/mouseDrag` → `seekToX()` → `onSeek` callback → `player.setPositionRelative()`

### 7. Mixer (`MixerPanel`)
Crossfader curves in `applyCrossfader(pos)`:
- **Linear:** `g1 = 1-pos, g2 = pos`
- **EqualPower:** `g1 = cos(pos × π/2), g2 = sin(pos × π/2)`
- **Cut:** `g1 = (pos ≤ 0.5) ? 1 : 0, g2 = (pos ≥ 0.5) ? 1 : 0`

EQ sliders use `Rotary` style — drawn by `CustomLookAndFeel::drawRotarySlider()`.

### 8. VU Meter (`VUMeter`)
- Timer at 30 Hz reads RMS via `getLevel()`
- Peak-hold: hold 24 frames, then decay 0.018/frame
- Clip LED: latches `clipped_ = true` when level ≥ 0.98; `mouseDown` resets

### 9. Library (`PlaylistComponent`)
- `getFilteredIndices()` applies `searchText_` filter on every render
- `setNowPlayingFile(file)` stores file path → `paintRowBackground()` draws green underline on matching row
- Persists to `Documents/trackLibrary.csv` on every add/delete

---

## Important Classes

| Class | Role |
|-------|------|
| `DJAudioPlayer` | Audio engine — playback, EQ, filters, cues, loops, hotcues, metering |
| `DeckGUI` | Deck UI surface — controls, hotcue pads, bar-loop buttons, waveform |
| `WaveformDisplay` | Waveform render, seek, hover tooltip, loop/cue/beat overlays |
| `MixerPanel` | Crossfader (3 curves), EQ rotary knobs, trim, master, VU meters |
| `PlaylistComponent` | Library browser, search, sort, now-playing highlight, CSV persistence |
| `BPMAnalyser` | Background BPM detection, result callback |
| `VUMeter` | RMS bar, peak-hold, clip LED |
| `CustomLookAndFeel` | Theme colours, button draw, linear slider draw, rotary knob draw |

---

## Keyboard Shortcuts

| Key | Action |
|-----|--------|
| `1` | Play/stop Deck A |
| `2` | Play/stop Deck B |
| `Space` | Play/stop Deck A |
| `Q` | Set cue — Deck A |
| `W` | Jump to cue — Deck A |
| `O` | Set cue — Deck B |
| `P` | Jump to cue — Deck B |
| `S` | BPM sync Deck B to Deck A |

---

## Build

### Requirements
- JUCE 8.x
- Visual Studio 2022
- Windows x64

### Steps
1. Open `OtoDeck.jucer` in Projucer
2. Re-save if source files have changed (regenerates VS project)
3. Open `Builds/VisualStudio2022/OtoDeck.sln`
4. Build `Debug | x64` or `Release | x64`

---

## Runtime Usage

### Loading tracks
- **LOAD TRACK** button on deck
- Drag a file onto a deck
- **Load A / Load B** from library table

### Performance workflow
1. Add tracks to library
2. Load one track per deck
3. Play both decks
4. Use hotcue pads, loop bar buttons, cue set/jump for performance
5. Blend with crossfader (pick curve: LIN / EQP / CUT)
6. Shape sound with 3-band EQ and LP/HP filters
7. Monitor levels on VU meters — clip LED shows if hitting hard

---

## Persistence

Library saves to: `Documents/trackLibrary.csv`

Format: `Title, Artist, Duration (seconds), BPM, File Path`

Loaded automatically on startup. Saved on every track add or delete.

---

## File Reference

| File | Role |
|------|------|
| `Source/Main.cpp` | App entry point |
| `Source/audio/DJAudioPlayer.h/cpp` | Audio engine |
| `Source/audio/BPMAnalyser.h/cpp` | BPM detection |
| `Source/gui/MainComponent.h/cpp` | Root UI + audio callback |
| `Source/gui/DeckGUI.h/cpp` | Deck control surface |
| `Source/gui/WaveformDisplay.h/cpp` | Waveform component |
| `Source/gui/MixerPanel.h/cpp` | Mixer panel |
| `Source/gui/VUMeter.h` | VU meter |
| `Source/gui/DeckLoadCellComponent.h` | Library row action cell |
| `Source/library/PlaylistComponent.h/cpp` | Library browser |
| `Source/shared/CustomLookAndFeel.h` | Theme |
| `Source/shared/DeckState.h` | Deck state snapshot |
| `Source/shared/Callbacks.h` | Callback typedefs |
| `OtoDeck.jucer` | JUCE project file |
| `Builds/VisualStudio2022/OtoDeck.sln` | VS solution |

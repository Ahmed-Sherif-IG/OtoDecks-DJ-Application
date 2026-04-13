# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

**IDE:** Visual Studio 2022 — open `Builds/VisualStudio2022/OtoDeck.sln`.

**Projucer:** Open `OtoDeck.jucer` in Projucer to regenerate VS project files after adding/removing source files. JUCE 8 modules expected at `../../../JUCE/juce-8.0.8-windows/JUCE/modules`.

**Configurations:** Debug and Release — output `OtoDeck.exe` under `Builds/VisualStudio2022/x64/{Debug|Release}/App/`.

No test suite. No CLI build system.

## Architecture

Dual-deck DJ app built on JUCE 8. All source files live flat in `Source/`.

### Audio pipeline (`DJAudioPlayer`)

Chain per deck: `AudioFormatReaderSource → AudioTransportSource → ResamplingAudioSource → MixerAudioSource`

Each player owns:
- 3-band EQ (`IIRFilter` low-shelf / peak / high-shelf) applied in `getNextAudioBlock`
- LP/HP filter toggles (M6)
- Feedback delay line (M6)
- `std::atomic<float> rmsLevel_` updated per block for VU metering
- `std::atomic<double> bpm_` set by `BPMAnalyser` on the message thread
- Three separate gain stages: `userGain_` (channel fader) × `trimGain_` (MixerPanel trim) × `crossfaderGain_` (equal-power crossfader)
- Loop region (checked on GUI timer thread via `checkAndLoopIfNeeded()`)
- Cue point (relative position [0,1])

**Audio thread safety:** EQ/filter coefficients updated via `std::atomic<bool> eqDirty_` / `filterDirty_` flags — dirty-checked at start of each `getNextAudioBlock`. No locks or allocations in the audio thread.

### GUI layer

- **`MainComponent`** — top-level `AudioAppComponent`. Owns both players, both `DeckGUI`, the `MixerPanel`, `PlaylistComponent`, and a `std::atomic<float> masterGain_` applied in `getNextAudioBlock`. Also implements `KeyListener` for the global keyboard shortcut table.
- **`DeckGUI`** — owns the full per-deck UI: play/stop/sync/cue/loop/LP/HP/tap buttons, vol/speed/pos sliders, time label, state label, BPM label, and a `WaveformDisplay`. Runs a 20Hz timer for position/state/time refresh and loop monitoring. Owns a `BPMAnalyser` instance — analysis starts automatically on `loadFile()`.
- **`WaveformDisplay`** — `AudioThumbnail`-based renderer. Supports deck-specific colour, click/drag-to-seek (`onSeek` callback), loop region shading, cue marker, beat grid overlay (M5), and playhead line.
- **`MixerPanel`** — sits between the two decks. Contains crossfader (equal-power: `g1 = cos(x·π/2)`, `g2 = sin(x·π/2)`), per-deck trim sliders, per-deck VU meters, master volume, and 3-band EQ sliders per deck. Calls player methods directly (passed by reference in constructor).
- **`VUMeter`** — header-only. Self-contained 30Hz-timer-driven component. Gets level from a `std::function<float()>` callback into `DJAudioPlayer::getRMSLevel()`. Draws green/yellow/red bar with peak-hold.
- **`PlaylistComponent`** — `TableListBoxModel` with columns: Title / Artist / Duration / BPM / Load. Reads ID3/Vorbis metadata via `AudioFormatReader::metadataValues` on add. Filters via `TextEditor` search box (case-insensitive substring). Column-sort via `sortOrderChanged`. Persists to `~/Documents/trackLibrary.csv` (format: `title,artist,duration,bpm,path`). Exposes `loadTrackToDeck` and `onNowPlaying` callbacks wired by `MainComponent`.
- **`DeckLoadCellComponent`** — custom table cell with Deck 1 / Deck 2 / Remove buttons. Callbacks injected by `PlaylistComponent::refreshComponentForCell`.
- **`BPMAnalyser`** — `juce::Thread` subclass. Energy-based onset detection over first 60s of audio (10ms RMS windows, local-average threshold). Folds result into [60,180] BPM. Invokes `onResult` callback on message thread. TAP BPM in `DeckGUI` averages intervals of last 8 taps and overrides auto-detected value.
- **`CustomLookAndFeel`** — flat grayscale `LookAndFeel_V4`. Owned exclusively by `MainComponent`; `setLookAndFeel(&customLook)` on self propagates to all children via JUCE inheritance. `setLookAndFeel(nullptr)` called in `MainComponent` destructor before `customLook` is destroyed.

### Cross-component wiring

All wiring happens in `MainComponent` constructor after all objects exist:
- `deckGUI1/2->getOtherDeckSpeed` → lambda returning other deck's speed slider value
- `deckGUI1/2->getOtherDeckBPM` → lambda calling `player2/1.getBPM()`
- `playlistComponent.loadTrackToDeck` → loads file into player + DeckGUI
- `playlistComponent.onNowPlaying` → calls `DeckGUI::setNowPlaying()`
- `MixerPanel` holds references to both players directly

### Keyboard shortcuts (M6)

| Key | Action |
|-----|--------|
| `Space` | Play/Stop Deck 1 |
| `1` | Play/Stop Deck 1 |
| `2` | Play/Stop Deck 2 |
| `Q` | Set Cue Deck 1 |
| `W` | Go Cue Deck 1 |
| `O` | Set Cue Deck 2 |
| `P` | Go Cue Deck 2 |
| `S` | Sync Deck 2 to Deck 1 |

### Layout

`MainComponent::resized()` uses proportional math (`getWidth()` / `getHeight()` ratios). Top 65% = deck area; mixer panel takes ~1/6 of deck width; bottom 35% = playlist.

### Audio-thread rules

- No locks, no heap allocation in `getNextAudioBlock`.
- EQ/filter coefficient updates use atomic dirty flags.
- Delay buffer is pre-allocated in `prepareToPlay`.
- Loop re-trigger and cue jump called from the GUI timer (message thread), which is safe since `AudioTransportSource::setPosition` is message-thread-only.

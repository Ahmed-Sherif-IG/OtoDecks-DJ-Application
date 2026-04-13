# OtoDecks — Production Refactor Plan

## Goal
Transform the current academic JUCE DJ app into a production-quality desktop DJ application with professional architecture, polished UX, real audio features, and a solid library system — while preserving every working piece of the current foundation.

## Current State Summary
- Dual-deck playback via JUCE audio pipeline (works)
- Volume / speed / position sliders per deck (works)
- Waveform display via AudioThumbnail (works)
- Deck sync via speed-ratio callback (works, crude)
- Playlist with CSV persistence (works, no metadata)
- Flat grayscale CustomLookAndFeel (works, needs polish)
- Drag-and-drop file loading (works)

## Known Weaknesses to Fix
- `using namespace juce` / `using namespace std` in headers — pollutes every include
- CustomLookAndFeel owned per-component — causes dangling-pointer risk at shutdown
- No audio state struct — state scattered across DeckGUI, DJAudioPlayer, sliders
- No crossfader
- No BPM detection or real beat sync
- No track metadata (title, artist, duration)
- No waveform click-to-seek
- No search / sort in playlist
- No keyboard shortcuts
- Layout not responsive to window resize

---

## Milestones

---

### Milestone 1 — Architecture Cleanup (Foundation)
**Goal:** Clean up the codebase so every future milestone builds on solid ground. No new features. No visible UI changes. Just structural correctness.

**Tasks:**
1. Remove all `using namespace` from `.h` files — qualify everything explicitly
2. Centralize LookAndFeel: one `CustomLookAndFeel` instance owned exclusively by `MainComponent`, passed as pointer to all children
3. Introduce `DeckState` struct in `DJAudioPlayer.h`:
   - `bool isPlaying`, `double bpm`, `double gain`, `double speed`, `bool isLoaded`
4. Add `getState() const` to `DJAudioPlayer` returning `DeckState`
5. Replace raw `DJAudioPlayer*` in `DeckGUI` with a reference
6. Move all `std::function` callback declarations to a shared `Callbacks.h` header
7. Ensure every component calling `setLookAndFeel(&x)` calls `setLookAndFeel(nullptr)` in its destructor
8. Fix double-include of `JuceHeader.h` in all files (currently included twice)

**Test:** App compiles and runs identically to before. No regressions.

---

### Milestone 2 — Enhanced Waveform & Playback Controls
**Goal:** Make the waveform interactive and the deck controls feel professional.

**Tasks:**
1. **Click-to-seek on waveform** — `WaveformDisplay` handles `mouseDown` / `mouseDrag`, maps x-position to relative position and calls `player.setPositionRelative()`
2. **Deck-colored waveforms** — Deck 1 draws in blue (`#3A7BD5`), Deck 2 in orange (`#F5A623`). Background stays dark.
3. **Playback state indicator** — small colored dot or label next to deck title: green = playing, yellow = paused, red = stopped/empty
4. **Time display label** — show elapsed / remaining time (MM:SS) updated by the existing 500ms timer
5. **Waveform playhead line** — a visible thin vertical line at current position (already partially done, refine thickness + color)
6. **Loop region** (basic) — two buttons "Loop In" / "Loop Out" on DeckGUI; `DJAudioPlayer` stores `loopStart` and `loopEnd` and re-triggers position when end is hit
7. **Cue point** — single "Set Cue" button; "Go Cue" jumps to stored cue position. Stored in `DeckState`.

**Test:** Click waveform, playhead jumps. Time label updates live. Loop plays correctly.

---

### Milestone 3 — Crossfader & Mixer Panel
**Goal:** Add the core mixing interface that turns this into a real DJ tool.

**New Component:** `MixerPanel` — sits between the two decks in the layout.

**Tasks:**
1. **Crossfader slider** — horizontal slider in `MixerPanel`. Left = full Deck 1, Right = full Deck 2, center = equal blend. Formula: `deck1gain = cos(x * π/2)`, `deck2gain = sin(x * π/2)` (equal-power).
2. **Per-deck channel gain (trim)** — small vertical slider above each deck section in MixerPanel for pre-fader gain trim.
3. **Per-deck VU meter** — simple `Component` subclass that paints a vertical bar driven by RMS level from the audio thread. Refresh at 30fps via a timer.
4. **Master volume** — single slider in `MixerPanel` applied to `MixerAudioSource` output.
5. **3-band EQ per deck** — Low / Mid / High sliders (-12dB to +12dB). Implement via JUCE `IIRFilter` (low-shelf, peak, high-shelf). Store filters in `DJAudioPlayer`.
6. `MainComponent::resized()` update to accommodate `MixerPanel` between the two decks.

**Audio thread safety note:** All parameter changes to EQ and gain must use `std::atomic<float>` or JUCE's `SmoothedValue` — never call `Slider::getValue()` from the audio thread.

**Test:** Crossfader smoothly transitions audio. VU meters react. EQ changes audible. No audio glitches.

---

### Milestone 4 — Library & Metadata
**Goal:** Replace the bare CSV playlist with a real track library that shows metadata and supports search.

**Tasks:**
1. **Track metadata struct** — extend `Track` to include: `String artist`, `String title`, `double durationSeconds`, `double bpm` (0 if unknown), `int64 fileSize`
2. **Metadata reader** — on file add, use `juce::AudioFormatReader` to get duration; use `juce::ID3Reader` (or manual MP3 tag read via `juce::MemoryBlock`) for title/artist
3. **Playlist columns** — add columns: Title | Artist | Duration | BPM | Load Deck 1 | Load Deck 2
4. **Search bar** — `TextEditor` above the table. Filters visible rows by matching title or artist (case-insensitive substring). Does not delete tracks, just hides non-matching rows.
5. **Column sort** — clicking column header sorts by that field. Second click reverses order.
6. **Persist metadata in CSV** — extend the CSV format to include artist, duration, bpm fields. Handle legacy files gracefully (missing fields = empty string).
7. **"Now Playing" label** — deck title area shows `Artist - Title` after a track is loaded (pulled from the Track metadata).

**Test:** Add 10 tracks, verify metadata appears, search filters correctly, sort works, library reloads after restart with full metadata.

---

### Milestone 5 — BPM Detection & Real Sync
**Goal:** Replace the crude speed-ratio sync with actual BPM-based sync.

**Tasks:**
1. **BPM analyser class** — `BPMAnalyser` runs on a background thread (`juce::Thread`). Reads audio samples from the loaded file, runs an onset-detection algorithm (energy-based onset detection is sufficient for this scope), accumulates beat intervals, outputs estimated BPM.
2. **BPM display per deck** — label showing detected BPM next to deck title. Shows "---" while analysing, then updates.
3. **Sync rewrite** — when SYNC is clicked: compute target BPM from the other deck's `DeckState.bpm`, set `player.setSpeed(myBPM / targetBPM)`. Speed slider updates to reflect new ratio.
4. **Beat grid overlay on waveform** — once BPM is known, draw faint vertical lines at beat positions in `WaveformDisplay`.
5. **BPM tap button** — "TAP" button: calculates BPM from successive taps (average interval of last 4 taps). Overrides auto-detected BPM.

**Test:** Load two tracks, SYNC aligns playback speed correctly. Tap BPM produces accurate result.

---

### Milestone 6 — Effects, Keyboard Shortcuts & Final Polish
**Goal:** Add effects, hotkeys, and professional-grade UI responsiveness.

**Tasks:**

#### Effects
1. **Low-pass filter toggle** — button on each deck cuts highs (useful for transitions). Uses `juce::IIRFilter` with cutoff ~800Hz.
2. **High-pass filter toggle** — cuts lows (~300Hz). Standard DJ kill-switch behavior.
3. **Echo/delay effect** — simple feedback delay line (`juce::AbstractFifo` based). Wet/dry knob. Time synced to BPM (1/4 note or 1/8 note).

#### Keyboard Shortcuts
| Key | Action |
|-----|--------|
| `Space` | Play/Stop focused deck |
| `1` | Play/Stop Deck 1 |
| `2` | Play/Stop Deck 2 |
| `Q` | Set Cue Deck 1 |
| `W` | Go Cue Deck 1 |
| `O` | Set Cue Deck 2 |
| `P` | Go Cue Deck 2 |
| `S` | Sync active deck to other |

Implement via `juce::KeyListener` registered on `MainComponent`.

#### UI Polish
1. **Responsive layout** — all layout math based on `getWidth()` / `getHeight()` ratios in `resized()`. Test from 900×600 to 1920×1080.
2. **Deck color accent** — Deck 1 border/header tinted blue, Deck 2 orange. Consistent with waveform colors.
3. **Button states** — Play button visually depressed when playing. Stop grayed when nothing loaded.
4. **Smooth slider response** — use `juce::SmoothedValue` for volume and speed to avoid zipper noise on rapid slider movement.
5. **About dialog** — Help > About shows version string and build date. (`juce::DialogWindow`)

**Test:** All shortcuts work. UI scales properly at multiple window sizes. No audio artifacts from slider movement.

---

## File Structure After Refactor

```
Source/
├── audio/
│   ├── DJAudioPlayer.h / .cpp
│   ├── BPMAnalyser.h / .cpp
│   └── AudioEngine.h / .cpp       ← optional wrapper
├── gui/
│   ├── MainComponent.h / .cpp
│   ├── DeckGUI.h / .cpp
│   ├── WaveformDisplay.h / .cpp
│   ├── MixerPanel.h / .cpp         ← NEW (Milestone 3)
│   ├── VUMeter.h / .cpp            ← NEW (Milestone 3)
│   └── DeckLoadCellComponent.h
├── library/
│   ├── PlaylistComponent.h / .cpp
│   └── TrackLibrary.h / .cpp       ← NEW (Milestone 4)
├── effects/
│   ├── EQProcessor.h / .cpp        ← NEW (Milestone 3 / 6)
│   └── DelayEffect.h / .cpp        ← NEW (Milestone 6)
├── shared/
│   ├── DeckState.h                 ← NEW (Milestone 1)
│   ├── Callbacks.h                 ← NEW (Milestone 1)
│   └── CustomLookAndFeel.h / .cpp
└── Main.cpp
```

---

## Constraints & Rules

- Never break the existing audio pipeline. Refactor around it, not through it.
- All audio-thread code must be lock-free. No mutexes, no allocations in `getNextAudioBlock`.
- LookAndFeel must be owned by MainComponent only.
- One milestone at a time. Do not mix milestone work in one commit.
- Commit after each milestone with a descriptive message.
- Update this file only when architecture or priorities change.

---

## Priority Order
1. Milestone 1 — must do first (everything depends on clean architecture)
2. Milestone 2 — high value, high visibility
3. Milestone 3 — makes it actually a DJ app
4. Milestone 4 — makes library usable
5. Milestone 5 — impressive differentiator
6. Milestone 6 — polish, ship-ready

# OtoDecks DJ Application — Update Plan

## Context

OtoDecks is a JUCE/C++ dual-deck DJ application currently transitioning from academic prototype to production-quality software. Core audio engine is solid (EQ, filters, BPM, loops, cues, crossfader, VU metering, library persistence). UI needs polish, several features need upgrades, and new professional features are missing. This plan documents what to build next.

---

## 1. UI Improvements

### 1.1 Deck Layout Hierarchy
- **Problem:** Deck sections lack clear visual grouping — controls feel flat
- **Fix:** Add section panels (rounded rect backgrounds) to group: Transport | EQ | Effects | Waveform
- **File:** `Source/gui/DeckGUI.cpp` — `resized()` and `paint()`

### 1.2 Waveform Display
- **Problem:** Waveform area too small relative to screen; seek interaction feedback lacking
- **Fix:**
  - Increase waveform height allocation in `DeckGUI::resized()`
  - Add hover highlight on mouse-over
  - Show time cursor tooltip on hover (current position in MM:SS)
- **File:** `Source/gui/WaveformDisplay.h/cpp`

### 1.3 Button States & Feedback
- **Problem:** Buttons don't clearly show active/inactive state (e.g., loop active, filter on)
- **Fix:**
  - Toggle buttons use accent color fill when active
  - Play button switches icon glyph (▶ → ❚❚) via `CustomLookAndFeel`
  - Filter buttons (LP/HP) glow accent blue when enabled
- **File:** `Source/shared/CustomLookAndFeel.h`, `Source/gui/DeckGUI.cpp`

### 1.4 Typography & Labels
- **Problem:** BPM, time, and track title labels all same size/weight — hard to scan
- **Fix:**
  - Track title: larger bold font
  - BPM: monospace accent-colored label
  - Elapsed/total time: large monospace counter
  - Section headers: small caps muted text
- **File:** `Source/gui/DeckGUI.cpp`

### 1.5 Mixer Panel Polish
- **Problem:** EQ sliders and crossfader look identical — no visual hierarchy
- **Fix:**
  - Crossfader: wider, centered, distinct color
  - EQ knobs: switch from sliders to rotary knobs via `CustomLookAndFeel::drawRotarySlider()`
  - Add dB scale tick marks beside EQ sliders
  - VU meters: taller, cleaner gradient segments
- **File:** `Source/gui/MixerPanel.cpp`, `Source/shared/CustomLookAndFeel.h`

### 1.6 Playlist / Library
- **Problem:** Table rows tight; no album art placeholder; search box plain
- **Fix:**
  - Increase row height to 28px
  - Rounded search box with magnifier icon prefix
  - Alternating row colors using `panelAlt` color
  - "Now Playing" row highlight with accent underline
- **File:** `Source/library/PlaylistComponent.cpp`

### 1.7 Responsive Layout
- **Problem:** Window resize breaks proportions
- **Fix:** Define minimum window size (1200×700); set proportional flex layout in `MainComponent::resized()`
- **File:** `Source/gui/MainComponent.cpp`

---

## 2. New Features

### 2.1 Hotcue Pads (8 per deck)
- 8 pad buttons per deck, each stores/recalls a position
- Pads light up color when set, dim when empty
- Right-click pad → clear
- **Files:** `Source/gui/DeckGUI.h/cpp`, `Source/audio/DJAudioPlayer.h/cpp`

### 2.2 Pitch/Key Lock
- Toggle to maintain pitch while changing speed (formant preservation)
- JUCE `StretcherAudioSource` or rubber-band integration
- **File:** `Source/audio/DJAudioPlayer.h/cpp`

### 2.3 Track Waveform Overview Strip
- Mini full-track waveform bar above main waveform
- Click to jump anywhere; shows loop region at scale
- **File:** `Source/gui/WaveformDisplay.h/cpp`

### 2.4 Auto-Mix / BPM Sync Button
- Extend existing SYNC to also sync phase (beat alignment)
- Visual countdown to next beat before sync fires
- **File:** `Source/gui/DeckGUI.cpp`, `Source/gui/MainComponent.cpp`

### 2.5 Crossfader Curve Select
- Crossfader curve selector: Linear | Equal Power | Cut (scratch)
- Stored as enum in `MixerPanel`
- **File:** `Source/gui/MixerPanel.h/cpp`

### 2.6 Recording Output
- Record master output to WAV file
- Record button in `MainComponent` toolbar
- Uses `juce::AudioFormatWriter`
- **File:** `Source/gui/MainComponent.h/cpp`

### 2.7 Tempo Range Control
- Slider range toggle: ±8% | ±16% | ±50% | ×2
- **File:** `Source/gui/DeckGUI.cpp`, `Source/audio/DJAudioPlayer.h`

---

## 3. Existing Feature Upgrades

### 3.1 BPM Analyser
- **Current:** Energy-based; accuracy ~±2 BPM
- **Upgrade:** Add autocorrelation pass; discard outlier sub-ranges
- **File:** `Source/audio/BPMAnalyser.h/cpp`

### 3.2 3-Band EQ
- **Current:** Fixed frequencies (100Hz / 1kHz / 10kHz)
- **Upgrade:** Add kill switch (−∞ dB) per band; adjustable crossover frequencies via right-click
- **File:** `Source/audio/DJAudioPlayer.h/cpp`, `Source/gui/MixerPanel.h/cpp`

### 3.3 Loop System
- **Current:** Manual in/out set only
- **Upgrade:** Bar-length buttons (1/4, 1/2, 1, 2, 4, 8 bars) auto-calculated from BPM; loop halve/double
- **File:** `Source/gui/DeckGUI.h/cpp`, `Source/audio/DJAudioPlayer.h/cpp`

### 3.4 Effects Scaffold → Real Effects
- **Current:** Delay scaffolded but not wired to audio path
- **Upgrade:** Wire delay fully; add reverb via `juce::Reverb`; add flanger LFO; wet/dry knob per effect
- **File:** `Source/audio/DJAudioPlayer.h/cpp`, `Source/gui/DeckGUI.h/cpp`

### 3.5 VU Metering
- **Current:** RMS only
- **Upgrade:** True peak metering; clip indicator LED latches until clicked
- **File:** `Source/gui/VUMeter.h`

### 3.6 Library Search
- **Current:** Text filter only
- **Upgrade:** BPM range filter slider; duration filter
- **File:** `Source/library/PlaylistComponent.h/cpp`

### 3.7 Keyboard Shortcuts
- **Current:** 9 hardcoded shortcuts
- **Upgrade:** Preferences dialog with full rebindable mapping; export/import profile
- **File:** `Source/gui/MainComponent.cpp`

---

## 4. Ideas / Future Improvements

| Idea | Notes |
|------|-------|
| **Sampler pad bank** | 4–8 one-shot sample pads per deck; load WAV files |
| **Spectrum analyzer** | FFT display in waveform area or separate panel |
| **MIDI controller mapping** | Map any MIDI CC/note to any control via learn mode |
| **Track history log** | Timestamped play history, exportable to text |
| **Stem separation** | Load stems separately; mute individual stems |
| **Auto-gain normalize** | Analyze loudness on load; normalize to −14 LUFS |
| **Dark/light theme toggle** | Switch between dark and light studio theme |
| **OSC/network control** | Control OtoDecks from phone/tablet over LAN |
| **VST/AU plugin effects** | Load third-party effects into effect chain |
| **Waveform color by frequency** | Color waveform by spectral content (bass=red, hi=blue) |

---

## Critical Files

| File | Role |
|------|------|
| `Source/gui/MainComponent.h/cpp` | Root container, audio callback, keyboard shortcuts |
| `Source/gui/DeckGUI.h/cpp` | Deck surface — most UI work lands here |
| `Source/gui/MixerPanel.h/cpp` | Crossfader, EQ, VU — mixer UI |
| `Source/gui/WaveformDisplay.h/cpp` | Waveform rendering and interaction |
| `Source/audio/DJAudioPlayer.h/cpp` | Audio engine — all DSP changes |
| `Source/audio/BPMAnalyser.h/cpp` | BPM detection algorithm |
| `Source/library/PlaylistComponent.h/cpp` | Library browser and CSV persistence |
| `Source/shared/CustomLookAndFeel.h` | All visual styling — theme changes |

---

## Implementation Order

1. UI — `CustomLookAndFeel.h` + `DeckGUI.cpp` + `MixerPanel.cpp` (highest visual impact)
2. Loop upgrades — bar-length buttons (self-contained, easy win)
3. Hotcue pads — GUI + audio changes
4. Wire effects — delay/reverb already scaffolded
5. Recording — new feature, isolated to `MainComponent`
6. BPM analyser accuracy
7. Library search upgrades
8. Remaining new features

---

## Verification

- Build Debug config in Visual Studio 2022 after each major change
- Load MP3/WAV/FLAC on both decks — confirm playback unaffected
- Test crossfader, EQ, and loop with audio running
- Resize window — confirm layout scales correctly
- Confirm library CSV loads on startup and saves on change
- All 9 existing keyboard shortcuts must still fire

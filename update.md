# OtoDecks — Remaining Work

Things not yet built. Anything already shipped is removed from this list.

---

## New Features

### Pitch / Key Lock
- Toggle to hold pitch while changing speed (formant preservation)
- Needs `juce::TimeStretchAudioSource` or Rubber Band library integration
- **File:** `Source/audio/DJAudioPlayer.h/cpp`

### Waveform Overview Strip
- Mini full-track waveform bar above main waveform
- Click to jump anywhere; shows loop region at scale
- **File:** `Source/gui/WaveformDisplay.h/cpp`

### Phase-Aligned Sync
- Extend existing SYNC button to align beat phase, not just BPM
- Visual countdown to next beat before sync fires
- **File:** `Source/gui/DeckGUI.cpp`, `Source/gui/MainComponent.cpp`

### Master Recording
- Record master output to WAV file
- Record button + stop in `MainComponent` toolbar
- Uses `juce::AudioFormatWriter` + writer thread
- **File:** `Source/gui/MainComponent.h/cpp`

### Tempo Range Toggle
- Buttons to switch speed slider range: ±8% | ±16% | ±50% | ×2
- Currently hardcoded range in `DJAudioPlayer`
- **File:** `Source/gui/DeckGUI.cpp`, `Source/audio/DJAudioPlayer.h`

### Spectrum Analyzer
- Real-time FFT display — either overlay on waveform or separate panel
- Uses `juce::dsp::FFT`
- **File:** new `Source/gui/SpectrumAnalyser.h/cpp`, wire into `DeckGUI`

### Sampler Pad Bank
- 4–8 one-shot WAV pads per deck
- Same pad UI pattern as hotcues
- **File:** new `Source/audio/SamplerPlayer.h/cpp`, `Source/gui/DeckGUI.h/cpp`

### Auto-Gain Normalize
- Analyze track loudness on load, set trim to target −14 LUFS
- Uses `juce::AudioFormatReader` + RMS scan
- **File:** `Source/audio/DJAudioPlayer.h/cpp`, `Source/gui/DeckGUI.cpp`

---

## Existing Feature Upgrades

### BPM Analyser Accuracy
- **Current:** Energy-based, accuracy ~±2 BPM
- **Upgrade:** Add autocorrelation confirmation pass; discard outlier sub-ranges
- **File:** `Source/audio/BPMAnalyser.h/cpp`

### EQ Kill Switches
- **Current:** ±12 dB range per band
- **Upgrade:** Per-band kill button (−∞ dB); right-click knob for adjustable crossover frequency
- **File:** `Source/audio/DJAudioPlayer.h/cpp`, `Source/gui/MixerPanel.h/cpp`

### Wire Delay + Add Reverb / Flanger
- **Current:** Delay scaffold exists but not connected to audio path
- **Upgrade:** Wire delay; add `juce::Reverb`; add flanger LFO; wet/dry knob per effect
- **File:** `Source/audio/DJAudioPlayer.h/cpp`, `Source/gui/DeckGUI.h/cpp`

### Library Search Filters
- **Current:** Text filter only
- **Upgrade:** BPM range slider filter; track duration filter
- **File:** `Source/library/PlaylistComponent.h/cpp`

### Rebindable Keyboard Shortcuts
- **Current:** 8 hardcoded shortcuts in `MainComponent`
- **Upgrade:** Preferences dialog with full rebind table; export/import keybinding profile
- **File:** `Source/gui/MainComponent.cpp`

---

## Future Ideas

| Idea | Notes |
|------|-------|
| **MIDI controller mapping** | Map any MIDI CC/note to any control via learn mode |
| **Track history log** | Timestamped play history, exportable to text |
| **Stem separation** | Load stems separately; mute per stem |
| **Waveform color by frequency** | Spectral coloring — bass=red, treble=blue |
| **Dark/light theme toggle** | Switch between dark and light theme |
| **OSC/network control** | Control app from phone/tablet over LAN |
| **VST/AU plugin effects** | Load third-party effects into effect chain |
| **Key detection** | Detect musical key on load; show Camelot wheel compatibility |
| **Pitch nudge buttons** | Tiny ±2% nudge buttons for manual beat matching |
| **Remaining time warning** | Waveform position indicator turns red when <30s left |

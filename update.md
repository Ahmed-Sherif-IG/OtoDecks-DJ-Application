# OtoDecks — Claude Code Upgrade Roadmap

This file is the working roadmap for improving OtoDecks beyond its current shipped state.

It is written so a coding agent such as Claude Code can use it as a planning and execution brief.

---

## Project Intent

OtoDecks is being upgraded from a functional JUCE DJ coursework app into a more professional desktop DJ application.

The goal is not to add random features.
The goal is to improve the app in a way that makes it feel more like a real DJ product:
- stronger performance workflow
- better audio tools
- more useful deck controls
- improved browser/library usability
- more professional polish
- features added in sensible stages without destabilizing the existing app

---

## Important Rules For Any Agent Updating This Project

1. Do not break existing core playback.
2. Prefer milestone-sized changes over giant mixed changes.
3. Keep code structure aligned with the current source layout:
   - `Source/audio`
   - `Source/gui`
   - `Source/library`
   - `Source/shared`
4. If project files move or new files are added, ensure the JUCE project / generated build files stay in sync.
5. Preserve current working features while extending the app.
6. Prioritize real DJ workflow improvements over novelty features.
7. When uncertain, improve the highest-value, lowest-risk item first.

---

# Recommended Upgrade Order

The items below are grouped by implementation priority.

---

## PHASE 1 — High-Value Upgrades (Best Next Work)

These are the best immediate improvements because they add real value without exploding complexity.

### 1. Master Recording
**Priority:** Very High  
**Impact:** High  
**Difficulty:** Medium

#### Why it matters
A DJ app feels much more real when the user can record the master output.

#### Build target
- Add master output recording to WAV file
- Add record / stop controls in a sensible global UI location
- Use a writer thread so recording does not block audio

#### Likely files
- `Source/gui/MainComponent.h`
- `Source/gui/MainComponent.cpp`
- possibly a new helper in `Source/audio/`

#### Notes for implementation
- Use `juce::AudioFormatWriter`
- Avoid allocations in the audio callback
- Make start/stop safe and obvious

---

### 2. Tempo Range Toggle
**Priority:** Very High  
**Impact:** High  
**Difficulty:** Low to Medium

#### Why it matters
Real DJ workflows often need different tempo slider ranges depending on style and precision needs.

#### Build target
Add range options for tempo/speed control, such as:
- ±8%
- ±16%
- ±50%
- wider fallback / x2 mode if desired

#### Likely files
- `Source/gui/DeckGUI.h`
- `Source/gui/DeckGUI.cpp`
- `Source/audio/DJAudioPlayer.h`
- `Source/audio/DJAudioPlayer.cpp`

#### Notes for implementation
- Separate the displayed slider range from the internal resampling limit cleanly
- Keep current default behavior safe

---

### 3. Waveform Overview Strip
**Priority:** High  
**Impact:** High  
**Difficulty:** Medium

#### Why it matters
A mini overview waveform makes navigation much faster and more professional.

#### Build target
- Add a compact full-track overview strip above or around the main waveform
- Allow click-to-jump anywhere in the track
- Show loop region and current playhead at a glance

#### Likely files
- `Source/gui/WaveformDisplay.h`
- `Source/gui/WaveformDisplay.cpp`

#### Notes for implementation
- Keep it visually clean
- Do not make the deck feel crowded again

---

### 4. Wire Delay Properly + Add One More Musical Effect
**Priority:** High  
**Impact:** High  
**Difficulty:** Medium

#### Why it matters
Effects are core to DJ performance, but they must be musically useful, not just present in name.

#### Build target
- fully wire existing delay into the audible path
- expose usable control(s)
- then add one additional effect such as:
  - reverb, or
  - flanger

#### Likely files
- `Source/audio/DJAudioPlayer.h`
- `Source/audio/DJAudioPlayer.cpp`
- `Source/gui/DeckGUI.h`
- `Source/gui/DeckGUI.cpp`

#### Notes for implementation
- Keep effects subtle and performance-friendly
- avoid cluttering the deck UI with too many knobs at once

---

### 5. EQ Kill Switches
**Priority:** High  
**Impact:** High  
**Difficulty:** Medium

#### Why it matters
Quick bass/mid/high kills are very DJ-appropriate and more useful live than only having continuous EQ sliders.

#### Build target
- Add kill buttons for LOW / MID / HIGH per deck
- Kill should behave like real full-cut behavior, not a weak reduction

#### Likely files
- `Source/audio/DJAudioPlayer.h`
- `Source/audio/DJAudioPlayer.cpp`
- `Source/gui/MixerPanel.h`
- `Source/gui/MixerPanel.cpp`

#### Notes for implementation
- Ensure the kill state and slider state interact predictably

---

## PHASE 2 — Strong Product Upgrades

These are excellent next steps once Phase 1 is stable.

### 6. Pitch / Key Lock
**Priority:** High  
**Impact:** High  
**Difficulty:** Medium to High

#### Why it matters
Changing speed without changing musical pitch is a major DJ quality upgrade.

#### Build target
- Add a pitch/key lock toggle
- Preserve pitch while tempo changes

#### Likely files
- `Source/audio/DJAudioPlayer.h`
- `Source/audio/DJAudioPlayer.cpp`

#### Notes for implementation
- JUCE alone may not provide the best result here
- Rubber Band integration may be the right long-term path

---

### 7. Phase-Aligned Sync
**Priority:** High  
**Impact:** High  
**Difficulty:** High

#### Why it matters
Current sync behavior is useful but limited.
Real sync should consider beat phase, not just tempo/BPM.

#### Build target
- extend SYNC to align beat phase
- optionally provide timing feedback before sync fires

#### Likely files
- `Source/gui/DeckGUI.cpp`
- `Source/gui/MainComponent.cpp`
- possibly waveform / BPM support files

#### Notes for implementation
- only worth doing if BPM estimation is reliable enough

---

### 8. Library Search Filters
**Priority:** Medium-High  
**Impact:** Medium-High  
**Difficulty:** Medium

#### Why it matters
A DJ library becomes much more useful when filtering by more than just text.

#### Build target
Add filters for:
- BPM range
- duration range
- possibly artist/title combined quick filtering remains as-is

#### Likely files
- `Source/library/PlaylistComponent.h`
- `Source/library/PlaylistComponent.cpp`

---

### 9. BPM Analyser Accuracy Upgrade
**Priority:** Medium-High  
**Impact:** Medium-High  
**Difficulty:** Medium

#### Why it matters
Several advanced features become better if BPM detection becomes more trustworthy.

#### Build target
- improve BPM detection beyond the current energy-based estimate
- add a confirmation pass or outlier rejection

#### Likely files
- `Source/audio/BPMAnalyser.h`
- `Source/audio/BPMAnalyser.cpp`

---

### 10. Key Detection
**Priority:** Medium  
**Impact:** Medium-High  
**Difficulty:** Medium to High

#### Why it matters
Useful for harmonic mixing and makes the library more powerful.

#### Build target
- detect musical key on load
- display key in library and/or deck header
- optional Camelot notation support later

#### Likely files
- `Source/audio/`
- `Source/library/PlaylistComponent.*`
- deck/library display code

---

### 11. Pitch Nudge Buttons
**Priority:** Medium  
**Impact:** Medium  
**Difficulty:** Low to Medium

#### Why it matters
Good for manual beat matching and subtle correction.

#### Build target
- add small temporary nudge buttons
- nudge should not permanently rewrite the speed slider value unless designed intentionally

#### Likely files
- `Source/gui/DeckGUI.h`
- `Source/gui/DeckGUI.cpp`
- maybe `DJAudioPlayer`

---

### 12. Remaining Time Warning
**Priority:** Medium  
**Impact:** Medium  
**Difficulty:** Low

#### Why it matters
Small feature, but useful live.

#### Build target
- warning visual when track is nearly over
- for example under 30 seconds remaining

#### Likely files
- `Source/gui/DeckGUI.cpp`
- `Source/gui/WaveformDisplay.cpp`

---

## PHASE 3 — Bigger Expansions

These are good ideas, but should come after the core DJ workflow is stronger.

### 13. Spectrum Analyzer
**Priority:** Medium  
**Impact:** Medium  
**Difficulty:** Medium to High

#### Why it matters
Looks professional and can help visually, but is not core compared to recording, effects, and better sync.

#### Build target
- FFT-based spectrum display
- either overlay or separate compact panel

#### Likely files
- new `Source/gui/SpectrumAnalyser.h/.cpp`
- `DeckGUI` integration

---

### 14. Sampler Pad Bank
**Priority:** Medium  
**Impact:** Medium-High  
**Difficulty:** High

#### Why it matters
Can be great for performance, but adds major complexity.

#### Build target
- 4 to 8 one-shot sample pads
- separate from hotcues

#### Likely files
- new `Source/audio/SamplerPlayer.h/.cpp`
- `Source/gui/DeckGUI.h/.cpp`

---

### 15. Auto-Gain Normalize
**Priority:** Medium  
**Impact:** Medium  
**Difficulty:** Medium

#### Why it matters
Helpful for more consistent perceived loudness across tracks.

#### Build target
- scan loudness / RMS on load
- normalize trim target sensibly

#### Likely files
- `Source/audio/DJAudioPlayer.h/.cpp`
- `Source/gui/DeckGUI.cpp`

---

### 16. Rebindable Keyboard Shortcuts
**Priority:** Medium  
**Impact:** Medium  
**Difficulty:** Medium

#### Why it matters
Good usability feature, but not more important than core DJ workflow upgrades.

#### Build target
- shortcut preferences dialog
- configurable mapping
- save/load bindings

#### Likely files
- `Source/gui/MainComponent.cpp`
- maybe new settings helper files

---

## PHASE 4 — Advanced / Long-Term Ideas

These are valid, but should not be tackled before the project feels stable and polished.

### 17. MIDI Controller Mapping
**Priority:** Long-Term  
**Impact:** Very High  
**Difficulty:** High

#### Why it matters
Huge value for real DJ use, but implementation complexity is significant.

---

### 18. Track History Log
**Priority:** Long-Term  
**Impact:** Medium  
**Difficulty:** Low to Medium

#### Why it matters
Useful for set tracking and exports.

---

### 19. Stem Separation
**Priority:** Long-Term  
**Impact:** High  
**Difficulty:** Very High

#### Why it matters
Very modern and powerful, but far beyond core app maturity right now.

---

### 20. Waveform Color By Frequency
**Priority:** Long-Term  
**Impact:** Medium  
**Difficulty:** High

---

### 21. Dark / Light Theme Toggle
**Priority:** Long-Term  
**Impact:** Medium  
**Difficulty:** Medium

---

### 22. OSC / Network Control
**Priority:** Long-Term  
**Impact:** Medium-High  
**Difficulty:** High

---

### 23. VST / AU Plugin Effects
**Priority:** Long-Term  
**Impact:** High  
**Difficulty:** Very High

---

## Best Immediate Recommendation For Claude Code

If Claude Code is going to improve this project, the recommended order is:

1. **Master Recording**
2. **Tempo Range Toggle**
3. **Waveform Overview Strip**
4. **Wire Delay Properly + Add One More Musical Effect**
5. **EQ Kill Switches**
6. **Pitch / Key Lock**
7. **Library Search Filters**
8. **BPM Accuracy Upgrade**
9. **Phase-Aligned Sync**
10. **Key Detection**

This order gives the best balance of:
- user-visible value
- technical realism
- product maturity
- manageable risk

---

## How Claude Code Should Work Through This File

For each selected item:
1. inspect current implementation first
2. identify all impacted files
3. make a small implementation plan
4. implement in a milestone-sized chunk
5. avoid mixing unrelated upgrades in one commit
6. keep UI labels clear and DJ-appropriate
7. preserve build stability and existing working features

---

## Final Guidance

The project does not need every possible DJ feature immediately.
It needs the right features in the right order.

The best upgrades are the ones that:
- improve live usability
- improve audio control quality
- improve workflow clarity
- make the app feel more like a serious DJ tool

Use this file as a roadmap, not a dump list.

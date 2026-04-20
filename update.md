# OtoDecks Professional Upgrade Roadmap

This roadmap replaces the earlier feature dump with a product-focused plan for turning
OtoDecks from a functional coursework-style JUCE DJ app into a more polished desktop DJ
application.

The goal is not to add every possible DJ feature. The goal is to improve the app in a
stable order so each milestone makes the app feel more useful, more intentional, and more
professional.

---

## Product Direction

OtoDecks should become a clean two-deck DJ application with:

- strong deck workflow
- clearer tempo, cue, loop, and sync controls
- a more professional waveform-led interface
- a mixer that feels like a real center console
- a more useful music library
- reliable recording and performance tools
- restrained visual polish instead of busy coursework-style controls

Every change should protect the existing core playback behavior.

---

## Working Rules

1. Preserve core playback, loading, cueing, looping, and mixing.
2. Prefer milestone-sized improvements over large mixed changes.
3. Keep the current source structure:
   - `Source/audio`
   - `Source/gui`
   - `Source/library`
   - `Source/shared`
4. Keep the JUCE project and generated Visual Studio project in sync when adding files.
5. Make labels understandable to a non-technical DJ user.
6. Avoid adding advanced features before the deck, mixer, and library feel solid.
7. Verify each milestone with a build and a quick app launch.

---

# Recommended Upgrade Order

## Milestone 1 - Professional Deck And Mixer Polish

This should be the next major milestone. It improves the app's first impression and makes
the existing features easier to understand.

### 1. Tempo Range Toggle

**Priority:** Very High  
**Impact:** High  
**Difficulty:** Low to Medium

Current `SPEED` control feels generic. Professional DJ apps usually expose tempo/pitch
ranges such as:

- `+/-8%`
- `+/-16%`
- `+/-50%`

#### Build Target

- Rename `SPEED` to `TEMPO`.
- Add range buttons or a compact selector per deck.
- Default to a safe DJ-style range, probably `+/-16%`.
- Keep internal speed/resampling limits separate from the displayed user range.
- Show the current tempo value as a percentage, for example `+4.2%`.

#### Likely Files

- `Source/gui/DeckGUI.h`
- `Source/gui/DeckGUI.cpp`
- `Source/audio/DJAudioPlayer.h`
- `Source/audio/DJAudioPlayer.cpp`

---

### 2. Waveform Overview Strip

**Priority:** Very High  
**Impact:** High  
**Difficulty:** Medium

The waveform should be the visual center of each deck. A compact overview strip makes the
app feel much closer to professional DJ software.

#### Build Target

- Add a small full-track waveform overview above or below the main waveform.
- Show current playhead.
- Show cue marker.
- Show loop region.
- Allow click-to-jump anywhere in the track.
- Keep the visual clean and avoid crowding the deck.

#### Likely Files

- `Source/gui/WaveformDisplay.h`
- `Source/gui/WaveformDisplay.cpp`
- possibly `Source/gui/DeckGUI.cpp`

---

### 3. Deck Label And Control Cleanup

**Priority:** High  
**Impact:** Medium-High  
**Difficulty:** Low

Some labels still feel like implementation names rather than DJ product language.

#### Recommended Label Changes

- `SPEED` -> `TEMPO`
- `POSITION` -> `SEEK`
- `SPD RESET` -> `RESET TEMPO`
- `VOL RESET` -> `RESET VOL`
- Keep `BASS`, `MIDS`, `TREBLE` for EQ.
- Keep filter labels clear:
  - `LOW CUT` means removing bass/low frequencies.
  - `HIGH CUT` means removing treble/high frequencies.

#### Build Target

- Rename controls consistently.
- Make button text fit at the minimum supported window size.
- Keep primary actions visually stronger than secondary actions.

#### Likely Files

- `Source/gui/DeckGUI.cpp`
- `Source/gui/MixerPanel.cpp`
- `Source/shared/CustomLookAndFeel.h`

---

### 4. EQ Kill Switches

**Priority:** High  
**Impact:** High  
**Difficulty:** Medium

EQ kill buttons are very DJ-appropriate. They make the mixer more useful for live
performance than continuous EQ knobs alone.

#### Build Target

- Add kill buttons for each EQ band:
  - Bass kill
  - Mids kill
  - Treble kill
- Add the buttons per deck in the mixer.
- Kills should behave like full cuts, not weak reductions.
- Make slider and kill state interaction predictable:
  - If kill is active, the band is muted/cut.
  - Moving the EQ knob can either keep kill active until toggled off, or automatically clear kill.
  - Pick one behavior and keep it consistent.

#### Likely Files

- `Source/audio/DJAudioPlayer.h`
- `Source/audio/DJAudioPlayer.cpp`
- `Source/gui/MixerPanel.h`
- `Source/gui/MixerPanel.cpp`

---

### 5. Mixer Visual Refinement

**Priority:** High  
**Impact:** Medium-High  
**Difficulty:** Medium

The mixer should feel like a center console, not a narrow column of controls.

#### Build Target

- Make VU meters more prominent.
- Improve spacing around EQ knobs and labels.
- Make the crossfader wider and more tactile.
- Keep curve selector buttons smaller than performance controls.
- Use clearer deck color ownership:
  - Deck A controls use blue accents.
  - Deck B controls use orange accents.
  - Master/recording controls use green/red accents where appropriate.

#### Likely Files

- `Source/gui/MixerPanel.h`
- `Source/gui/MixerPanel.cpp`
- `Source/gui/VUMeter.h`
- `Source/shared/CustomLookAndFeel.h`

---

## Milestone 2 - Recording And Performance Tools

This milestone adds features that make the app useful for real sessions, not only playback
testing.

### 6. Master Recording

**Priority:** Very High  
**Impact:** High  
**Difficulty:** Medium

A DJ app feels much more serious when the user can record the master output.

#### Build Target

- Add global `REC` / `STOP REC` control near the mixer or master section.
- Record the final master output to a WAV file.
- Show recording time.
- Use a writer thread so recording never blocks the audio callback.
- Choose a sensible default save location, such as the user's Music or Documents folder.

#### Likely Files

- `Source/gui/MainComponent.h`
- `Source/gui/MainComponent.cpp`
- possibly a new helper in `Source/audio`

---

### 7. Remaining Time Warning

**Priority:** Medium-High  
**Impact:** Medium  
**Difficulty:** Low

This is a small feature, but it makes the deck feel alive and performance-aware.

#### Build Target

- When a loaded track has less than 30 seconds remaining:
  - change the time label color
  - optionally show `ENDING`
  - optionally add a subtle waveform warning color
- Do not trigger the warning for an empty deck.

#### Likely Files

- `Source/gui/DeckGUI.cpp`
- `Source/gui/WaveformDisplay.cpp`

---

### 8. Delay Effect UI

**Priority:** High  
**Impact:** Medium-High  
**Difficulty:** Medium

The audio engine already contains delay support, but the UI does not expose it in a useful
way.

#### Build Target

- Add a compact FX section per deck.
- Start with one reliable effect: delay.
- Include:
  - Delay on/off
  - Time control
  - Mix/wet control
- Keep feedback conservative to avoid runaway sound.
- Avoid cluttering the deck with too many knobs.

#### Likely Files

- `Source/audio/DJAudioPlayer.h`
- `Source/audio/DJAudioPlayer.cpp`
- `Source/gui/DeckGUI.h`
- `Source/gui/DeckGUI.cpp`

---

### 9. Pitch Nudge Buttons

**Priority:** Medium  
**Impact:** Medium  
**Difficulty:** Low to Medium

Pitch nudge helps manual beat matching.

#### Build Target

- Add small temporary nudge buttons per deck.
- Nudge should temporarily speed up or slow down playback.
- Releasing the button should return to the current tempo slider value.

#### Likely Files

- `Source/gui/DeckGUI.h`
- `Source/gui/DeckGUI.cpp`
- possibly `Source/audio/DJAudioPlayer.h`
- possibly `Source/audio/DJAudioPlayer.cpp`

---

## Milestone 3 - Library Upgrade

The current playlist works, but still looks and feels like a basic table. This milestone
makes the library more useful for DJ workflow.

### 10. Library Filters

**Priority:** High  
**Impact:** Medium-High  
**Difficulty:** Medium

#### Build Target

- Keep text search.
- Add BPM min/max filter.
- Add duration filter.
- Improve sorting behavior.
- Keep `Load A` and `Load B` visually tied to deck colors.

#### Likely Files

- `Source/library/PlaylistComponent.h`
- `Source/library/PlaylistComponent.cpp`

---

### 11. Better Track Browser Presentation

**Priority:** Medium-High  
**Impact:** Medium  
**Difficulty:** Low to Medium

#### Build Target

- Improve row selection styling.
- Add clearer now-playing state.
- Make missing-file handling visible if a saved track path no longer exists.
- Consider a selected-track info area.
- Keep the library compact so it does not overpower the decks.

#### Likely Files

- `Source/library/PlaylistComponent.h`
- `Source/library/PlaylistComponent.cpp`
- `Source/gui/DeckLoadCellComponent.h`

---

### 12. Key Detection

**Priority:** Medium  
**Impact:** Medium-High  
**Difficulty:** Medium to High

Musical key detection helps harmonic mixing, but it should come after the library has
better filtering and presentation.

#### Build Target

- Detect musical key on load or during analysis.
- Display key in the library.
- Optionally display key in the deck header.
- Consider Camelot notation later.

#### Likely Files

- `Source/audio`
- `Source/library/PlaylistComponent.h`
- `Source/library/PlaylistComponent.cpp`
- `Source/gui/DeckGUI.cpp`

---

## Milestone 4 - Analysis And Sync Quality

These upgrades improve the intelligence of the app. They should come after the UI and
core workflow feel better.

### 13. BPM Analyser Accuracy Upgrade

**Priority:** Medium-High  
**Impact:** Medium-High  
**Difficulty:** Medium

Current BPM detection is energy-based. It is useful, but not reliable enough for advanced
sync behavior.

#### Build Target

- Improve onset detection.
- Add outlier rejection.
- Add a confirmation pass.
- Avoid unstable BPM values.
- Keep analysis on a background thread.

#### Likely Files

- `Source/audio/BPMAnalyser.h`
- `Source/audio/BPMAnalyser.cpp`

---

### 14. Phase-Aligned Sync

**Priority:** Medium  
**Impact:** High  
**Difficulty:** High

This is valuable, but only worth doing after BPM detection becomes more trustworthy.

#### Build Target

- Extend sync beyond matching BPM/speed.
- Estimate beat phase.
- Align the target deck to the source deck's beat grid.
- Provide timing feedback before or after sync.

#### Likely Files

- `Source/gui/DeckGUI.cpp`
- `Source/gui/MainComponent.cpp`
- `Source/gui/WaveformDisplay.cpp`
- possibly `Source/audio/BPMAnalyser.cpp`

---

## Milestone 5 - Advanced Features

These are good long-term ideas, but should not be built before the deck, mixer, recording,
library, and analysis workflows are solid.

### 15. Pitch / Key Lock

**Priority:** Medium-Later  
**Impact:** High  
**Difficulty:** Medium to High

Changing tempo without changing musical pitch is a major professional feature. However,
high-quality key lock usually needs better time-stretching than basic JUCE resampling.

#### Notes

- Research Rubber Band or another time-stretching approach.
- Avoid shipping a low-quality version that sounds broken.

---

### 16. Spectrum Analyzer

**Priority:** Later  
**Impact:** Medium  
**Difficulty:** Medium to High

A spectrum analyzer can look professional, but it should not come before recording,
tempo polish, waveform polish, and library upgrades.

---

### 17. Sampler Pad Bank

**Priority:** Later  
**Impact:** Medium-High  
**Difficulty:** High

A sampler is useful, but it adds new audio-player complexity. Build it only after the main
deck workflow is stable.

---

### 18. Auto-Gain Normalize

**Priority:** Later  
**Impact:** Medium  
**Difficulty:** Medium

Useful for consistency across tracks, but it depends on reliable analysis and careful gain
staging.

---

### 19. Rebindable Keyboard Shortcuts

**Priority:** Later  
**Impact:** Medium  
**Difficulty:** Medium

Good usability feature, but less important than improving deck, mixer, and library
workflow.

---

### 20. MIDI Controller Mapping

**Priority:** Long-Term  
**Impact:** Very High  
**Difficulty:** High

Very valuable for a real DJ product, but it is a major architecture and UX project.

---

### 21. Track History Log

**Priority:** Long-Term  
**Impact:** Medium  
**Difficulty:** Low to Medium

Can be added after recording and library improvements.

---

### 22. Stem Separation

**Priority:** Long-Term  
**Impact:** High  
**Difficulty:** Very High

Too large for the current maturity of the project.

---

### 23. VST / AU Plugin Effects

**Priority:** Long-Term  
**Impact:** High  
**Difficulty:** Very High

Powerful, but far beyond the next practical upgrade steps.

---

# UI Direction

The app should feel less like a coursework submission and more like a compact DJ console.
The most important UI change is hierarchy: make the user understand what matters first.

## Deck UI Principles

- The waveform should be the hero of each deck.
- Transport buttons should be obvious and larger than secondary controls.
- Tempo controls should show meaningful DJ values, not only raw slider movement.
- Hotcues and loops should feel like performance pads.
- Repeat, filters, and effects should be grouped as deck tools.
- Avoid making every control the same visual weight.

## Mixer UI Principles

- The mixer should feel like the center console.
- VU meters should be easier to read.
- EQ knobs should have clear names: `BASS`, `MIDS`, `TREBLE`.
- Kill buttons should sit close to their EQ bands.
- Crossfader should be visually wide and important.
- Curve selector should be compact.

## Library UI Principles

- The browser should feel like a music library, not just a table.
- Search and filters should be obvious.
- Deck load actions should use deck colors.
- Now-playing state should be clear.
- Missing-file states should be handled gracefully.

---

# Best Next Work

The best next implementation order is:

1. Tempo range toggle and tempo label cleanup.
2. Waveform overview strip.
3. EQ kill switches.
4. Mixer visual refinement.
5. Master recording.
6. Remaining time warning.
7. Delay effect UI.
8. Library BPM/duration filters.
9. BPM analyser accuracy upgrade.
10. Phase-aligned sync.

This order improves the visible product quickly, keeps risk manageable, and avoids jumping
into large advanced features before the app feels polished at its core.


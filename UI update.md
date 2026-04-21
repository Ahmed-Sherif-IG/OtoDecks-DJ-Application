# OtoDecks UI Upgrade Plan

This document is the UI-only roadmap for the next phase of the OtoDecks DJ Application. The goal is to make the application feel like a polished DJ tool while keeping all current audio logic, deck behavior, playlist behavior, recording behavior, and feature functionality unchanged.

## Main Goal

Make the app look and feel more professional, cohesive, and performance-focused without changing what the app does.

The current app works well, but visually it still has some coursework-style signs:

- Many controls have similar weight, so the most important actions do not stand out enough.
- The decks, mixer, and library are functional but not yet visually unified.
- The interface has good colors, but the spacing, borders, hierarchy, and component polish can be improved.
- The waveform area, mixer, and playlist need stronger visual structure.
- Some controls still feel like generic buttons instead of parts of a DJ console.

## Strict Scope

This phase is only for UI and visual structure.

Allowed:

- Layout improvements
- Better spacing and alignment
- Better panel styling
- Better typography
- Better button styling
- Better slider styling
- Better waveform container styling
- Better playlist/table styling
- Better visual states for existing controls
- Reorganising existing controls visually, as long as behavior does not change
- Improving `CustomLookAndFeel`
- Adding helper drawing functions for UI consistency

Not allowed in this phase:

- No new audio features
- No changes to playback logic
- No changes to recording logic
- No changes to BPM detection logic
- No changes to playlist loading behavior
- No changes to shortcuts
- No changes to file formats
- No major new controls unless they are purely visual wrappers around existing controls

## Visual Direction

The app should feel like a modern compact DJ controller:

- Dark professional base
- Strong but controlled Deck A blue accent
- Strong but controlled Deck B orange accent
- Central mixer should feel like hardware
- Decks should feel like performance surfaces
- Library should feel like a clean music browser
- Active states should be clear without looking noisy
- Borders should be thinner and more intentional
- Buttons should have consistent height and radius
- Sliders should feel more custom and less default
- Empty waveform areas should feel intentional, not unfinished

The visual style should avoid:

- Overly glossy or toy-like UI
- Too many glowing effects
- Too many gradients
- Large rounded “coursework card” panels
- Crowded controls
- One-color monotony
- Text that is too small to read
- Decorative elements that do not help the DJ workflow

## Design Foundation Rules

These rules should guide every UI change.

### 1. Consistent Spacing

Use a clear spacing scale:

- Small gap: 4 px
- Normal gap: 8 px
- Section gap: 12-16 px
- Outer margin: 16 px

Avoid random one-off spacing unless the layout needs it.

### 2. Consistent Radius

Use restrained corner radius:

- Small controls: 6 px
- Panels: 8-10 px
- Large containers: 10-12 px maximum

Avoid very rounded panels unless they represent a physical console area.

### 3. Strong Hierarchy

Important controls should be visually stronger:

- `PLAY`
- `BEAT SYNC`
- `REC`
- Loaded/playing deck state

Secondary controls should be calmer:

- Reset buttons
- Clear buttons
- Nudge buttons when inactive
- EQ kill buttons when inactive

### 4. Clear Active States

Every toggle should have a visible active state:

- Repeat
- Delay
- Low cut
- High cut
- Nudge -
- Nudge +
- EQ kill buttons
- Crossfader curve
- Recording

Active states should use accent color, brighter text, or a subtle glow.

### 5. Better Text System

Use a clearer text hierarchy:

- Deck title: largest and bold
- BPM/time: medium and readable
- Section labels: small uppercase
- Button text: bold but not too large
- Playlist rows: readable and consistent

Avoid labels that feel cramped or clipped.

## Milestone 1: Shared UI System

### Goal

Build the visual foundation inside `CustomLookAndFeel` so the rest of the UI can reuse consistent colors, typography, borders, and component styling.

### Work

- Clean up the main color palette.
- Define stronger panel, raised panel, outline, text, muted text, blue, orange, red, and green colors.
- Improve default button drawing.
- Improve slider drawing if practical.
- Add reusable drawing helpers if useful.
- Make disabled buttons look intentional instead of faded randomly.
- Make hover/pressed states clearer but subtle.

### Files Likely Involved

- `Source/shared/CustomLookAndFeel.h`
- `Source/shared/CustomLookAndFeel.cpp`

### Acceptance Check

- App builds successfully.
- Existing controls still work.
- Buttons and sliders look more consistent across the whole app.
- No layout or behavior changes yet unless required by styling.

## Milestone 2: Deck Surface Redesign

### Goal

Make Deck A and Deck B feel like professional deck surfaces with clearer sections and better rhythm.

### Work

- Improve the deck panel background.
- Make the top title/status area cleaner.
- Improve spacing in the transport row.
- Make the waveform area feel like a deliberate display.
- Make hotcue pads more polished.
- Group cue/loop/effect controls more clearly.
- Improve empty-track state inside the waveform area.
- Keep Deck A and Deck B visually symmetrical.

### Files Likely Involved

- `Source/gui/DeckGUI.h`
- `Source/gui/DeckGUI.cpp`
- `Source/gui/WaveformDisplay.h`
- `Source/gui/WaveformDisplay.cpp`

### Acceptance Check

- Decks still load tracks.
- Waveforms still render.
- All existing deck buttons still work.
- Text does not clip at the current app size.
- Decks look like the main focus of the app.

## Milestone 3: Mixer Hardware Pass

### Goal

Make the mixer panel feel more like the center hardware section of a DJ controller.

### Work

- Improve mixer panel shape and internal alignment.
- Make master and balance labels clearer.
- Make the VU meters more polished.
- Improve EQ knob spacing and labels.
- Make EQ kill buttons visually consistent.
- Keep the record button below the crossfader, inside the mixer.
- Give the record button a polished but obvious recording state.
- Improve reset and crossfader curve button styling.

### Files Likely Involved

- `Source/gui/MixerPanel.h`
- `Source/gui/MixerPanel.cpp`
- `Source/gui/MainComponent.cpp`

### Acceptance Check

- Mixer controls do not overlap.
- Record button remains inside the mixer below the white balance slider.
- Sliders and EQ controls still work.
- The mixer feels like the visual anchor between both decks.

## Milestone 4: Playlist and Browser Polish

### Goal

Make the track library feel like a modern music browser rather than a plain table.

### Work

- Improve the playlist panel background and border.
- Improve the search and filter row.
- Improve table header styling.
- Improve row spacing and selected row state.
- Make `LOAD A`, `LOAD B`, and `DELETE` buttons cleaner.
- Make the track count less visually loud.
- Improve scrollbar appearance if possible.

### Files Likely Involved

- `Source/library/PlaylistComponent.h`
- `Source/library/PlaylistComponent.cpp`
- `Source/gui/DeckLoadCellComponent.h`

### Acceptance Check

- Add track still works.
- Search and filters still work.
- Load buttons still load tracks to the correct deck.
- Delete still works.
- Table is easier to scan.

## Milestone 5: Whole-App Layout and Proportion Pass

### Goal

Make the full app layout feel balanced at the current desktop size.

### Work

- Refine the vertical relationship between decks and library.
- Make sure the mixer width feels right.
- Improve outer margins and section gaps.
- Reduce wasted space without crowding controls.
- Make the app feel intentional when no tracks are loaded.
- Check loaded-track layout with waveform and playlist data visible.

### Files Likely Involved

- `Source/gui/MainComponent.h`
- `Source/gui/MainComponent.cpp`
- Possibly deck, mixer, and playlist files if small layout adjustments are needed.

### Acceptance Check

- No controls overlap.
- The app looks balanced when empty.
- The app looks balanced when tracks are loaded.
- The library is useful but does not visually overpower the decks.

## Milestone 6: State Feedback and Final Polish

### Goal

Make the app feel alive and responsive through better visual states, without changing behavior.

### Work

- Improve playing/paused/empty/ending deck states.
- Improve active loop/repeat states.
- Improve active nudge states.
- Improve active delay/cut states.
- Improve recording state.
- Improve hover/pressed states where useful.
- Remove visual clutter.
- Fix any text clipping or awkward spacing.

### Files Likely Involved

- `Source/shared/CustomLookAndFeel.*`
- `Source/gui/DeckGUI.*`
- `Source/gui/MixerPanel.*`
- `Source/library/PlaylistComponent.*`

### Acceptance Check

- Active states are clear.
- Inactive states are calm.
- The app feels coherent as one product.
- Build succeeds.
- App is opened and checked visually.

## Verification Plan

After every milestone:

1. Build the Release app.
2. Open the app.
3. Take a screenshot.
4. Check for:
   - Text clipping
   - Button overlap
   - Slider overlap
   - Bad spacing
   - Too much visual noise
   - Controls that look disabled when they are not
   - Controls that look active when they are not
5. Load at least one track when checking deck and playlist UI.
6. Confirm no intended functionality changed.

## First Implementation Step

Start with Milestone 1.

The first code pass should focus on `CustomLookAndFeel` only, unless a tiny related adjustment is needed elsewhere. This lets the app get a stronger foundation before changing the deck, mixer, and playlist layouts.

## Notes for Review

Before implementation starts, review this document and decide:

- Do we want the app to look more like club hardware, broadcast software, or a modern dark desktop app?
- Should the mixer stay narrow and hardware-like, or become wider and more premium?
- Should waveform displays be the strongest visual element on each deck?
- Should the library stay compact, or become more like a full browser?
- Should the app keep the blue/orange deck identity strongly, or make accents more subtle?

Once these direction choices are clear, implementation can begin milestone by milestone.

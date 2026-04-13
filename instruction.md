# OtoDecks Next-Phase Instructions

## Current Status
Runtime verification looks good from manual testing:
- Arabic-heavy track loading is working in both decks
- Mixed Arabic and Latin metadata rendering appears stable
- Load, play, seek, loop, and cue workflows appear functional
- Playlist add, remove, search, sort, save, and reload appear functional

That means the current stabilization and cleanup phase is effectively complete.

## Immediate Next Step
Before adding new features, clean up the project structure to match the intended layout described in `PROJECT.md`.

### Goal
Refactor the repository structure so it is easier to maintain, easier to scale, and looks like a serious production codebase rather than a course project.

### Scope for the project-structure pass
- Reorganize files and folders to match `PROJECT.md`
- Group related components logically
- Reduce root-level clutter
- Make naming more consistent where needed
- Update project references/build files so the app still builds cleanly after restructuring
- Do not introduce new features during the structure pass unless a tiny supporting change is required
- Keep behavior unchanged during restructuring

## After Structure Cleanup
Once the structure pass is complete, move to the next major phase:

# Phase: Professional UI and Feature Expansion

## Product Direction
The app should evolve from a functional student-style DJ project into something that feels more polished, dynamic, and professional.

Current weakness:
- the interface looks boring
- the layout feels static
- the visual language feels like coursework instead of a real DJ product

Next work should focus on making the app feel alive, modern, and performance-oriented.

## UI/UX Goals
- Make the UI feel more dynamic and responsive
- Improve visual hierarchy and spacing
- Add stronger visual identity and deck distinction
- Make interaction states feel intentional and polished
- Improve the perceived quality of the mixer, decks, waveform area, and playlist
- Introduce motion/visual feedback where appropriate
- Make the app feel like a professional DJ application, not a demo

## Future Enhancement Priorities
After the project structure cleanup, prioritize work in roughly this order:

1. **UI modernization**
   - improve layout polish
   - improve typography
   - improve deck visual treatment
   - improve button styling and interaction states
   - improve mixer visual design
   - make waveform and playback state feel more alive
   - improve playlist readability and density

2. **Dynamic visual feedback**
   - better active-state indicators
   - clearer playback and sync feedback
   - more responsive loop/cue/transport visuals
   - subtle animation or state transitions where they improve quality

3. **Feature growth**
   - add carefully chosen DJ-focused features after the structure cleanup is done
   - only add features that support a more professional DJ workflow
   - avoid random feature creep

## Working Rules for the Next Phase
- First finish repository/project structure cleanup according to `PROJECT.md`
- Then improve the UI to feel modern and professional
- Then add features in controlled milestones
- Keep changes milestone-based and reviewable
- Prefer production polish over rushed feature quantity
- Avoid breaking working runtime behavior while redesigning

## Recommended Milestone Order
### Milestone 1
Project structure cleanup and build-file alignment

### Milestone 2
Visual design system pass
- colors
- spacing
- typography
- component consistency

### Milestone 3
Deck and mixer redesign
- more professional control presentation
- stronger active states
- better visual rhythm

### Milestone 4
Playlist and waveform polish
- cleaner data presentation
- better readability
- more modern playback visualization

### Milestone 5
Feature expansion
- only after the app structure and UI quality are solid

## Summary
The app is stable enough to move forward.
The best next move is:
1. clean up the project file structure based on `PROJECT.md`
2. then redesign the UI so it feels dynamic and professional
3. then add features in controlled milestones

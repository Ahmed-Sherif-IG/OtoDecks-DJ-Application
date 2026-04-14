# OtoDecks UI Overhaul Notes

## Why this file exists
This file captures the UI problems discovered during live testing, the redesign goals, and the execution plan for a proper professional overhaul.

It is not a cosmetic tweak list.
It is the working brief for rebuilding the OtoDecks interface into something closer to a real DJ product.

---

## Current Verified Problems

### 1) Mixer structure is still not solved
Observed in runtime screenshots and user feedback.

Problems:
- The center mixer strip is still too cramped.
- Deck 2 mixer/EQ controls can be clipped or visually compressed.
- Important labels and controls are too close together.
- The mixer still feels like a narrow technical strip, not a deliberate control hub.

Conclusion:
- The current mixer layout should not be incrementally patched anymore.
- It needs a new structure, not just spacing adjustments.

### 2) Deck control layout still follows the old app mindset
Problems:
- Controls are still arranged like rows of generic form widgets.
- Action buttons feel packed under the waveform rather than designed into functional groups.
- The deck header and utility controls still feel academic instead of product-grade.

Conclusion:
- The deck UI should be regrouped by function and visual importance.

### 3) Labels needed usability cleanup
Already identified by user.

Preferred wording:
- `Gain` -> `Volume`
- `Seek` -> `Position`

Also maintain clear, descriptive action labels when needed.
Avoid cryptic wording unless the surrounding UI makes it obvious.

### 4) Need reset/default controls
User explicitly requested buttons to restore default values.

Needed reset/default actions:
- Deck volume reset to default
- Deck speed reset to default
- Mixer controls reset to default values
- EQ reset to neutral values
- Crossfader reset to center
- Master reset to default

Conclusion:
- Serious DJ workflow benefits from fast recovery to known default states.
- This should be part of the redesign, not an afterthought.

### 5) Bass problem was real and now fixed
Verified outcome:
- Bass EQ was too weak / not audibly convincing.
- Audio path was updated so bass now works and is audibly affecting sound.

This should be preserved during future UI/architecture work.

### 6) Button rendering flaw was real
Observed issue:
- A thin white line / split across buttons made them look like two joined ovals.

Status:
- This rendering issue was identified and targeted for removal.
- Future button styling must avoid fake gloss seams or split-cap illusions.

---

## Design Direction Going Forward

Do not keep the existing layout structure just because it already exists.
The redesign is allowed to change layout and component composition significantly.

### Core direction
The app should feel closer to professional DJ products such as:
- Rekordbox
- Serato DJ Pro
- Traktor Pro
- VirtualDJ

### Shared patterns seen in professional DJ apps
- Decks own most of the screen width.
- Mixer is compact, readable, and intentional.
- Controls are grouped by workflow, not by implementation convenience.
- High-frequency controls are visually prioritized.
- Transport, performance, EQ, and browsing are clearly separated.
- Visual hierarchy is obvious at a glance.
- Default/reset behavior is easy and fast.
- Labels are short but unmistakable.

---

## Redesign Goals

### Goal A: Rebuild the mixer as a compact professional center module
Requirements:
- No clipped controls on either side.
- Clear left deck / right deck ownership.
- Crossfader visually separated from EQ area.
- Master section clearly distinct from deck EQ sections.
- Better spacing and stronger grouping.
- VU meters should remain readable without dominating width.

### Goal B: Rebuild each deck into clear control zones
Suggested zones:
1. Header zone
   - deck status
   - track title / artist
   - BPM / time

2. Transport zone
   - play
   - pause/stop
   - load
   - sync

3. Playback adjustment zone
   - volume
   - speed
   - position
   - reset/default buttons for key values

4. Waveform zone
   - largest area in the deck
   - visually premium and uncluttered

5. Performance / utility zone
   - cue
   - jump cue
   - loop start
   - loop end
   - clear loop
   - tap BPM
   - filter toggles

### Goal C: Add reset/default affordances
Must include:
- volume reset
- speed reset
- EQ reset
- mixer reset
- crossfader center reset
- master default reset

### Goal D: Make the product feel designed, not assembled
Requirements:
- stronger hierarchy
- fewer cramped micro-elements
- more breathing room
- more intentional panel proportions
- better deck vs mixer balance
- no dependence on the previous classroom-style arrangement

---

## Concrete Next-Step Overhaul Plan

### Phase 1: Structural redesign
- Redesign `MainComponent::resized()` around a new layout ratio.
- Give decks more ownership of horizontal space.
- Redesign `MixerPanel::resized()` from scratch.
- Stop trying to fit all mixer elements into a narrow strip without hierarchy.

### Phase 2: Deck control redesign
- Rework `DeckGUI::resized()` into functional sections.
- Add reset buttons for volume and speed.
- Consider a compact utility row or icon/action row for reset/default actions.
- Preserve readability under normal desktop sizes.

### Phase 3: Mixer defaults and recovery controls
- Add reset-to-default actions for:
  - deck trims
  - EQ bands
  - master out
  - crossfader center
- Make reset actions obvious and safe.

### Phase 4: Visual identity pass
- Revisit panel shape, spacing, and contrast after layout is correct.
- Keep professional DJ-app references in mind.
- Avoid fake decorative effects that hurt clarity.
- Prioritize visual confidence and readability over excessive ornament.

### Phase 5: Validation pass
- Test both decks loaded and unloaded.
- Test long track titles.
- Test Arabic/Latin mixed metadata.
- Test smaller and larger window sizes.
- Verify no control clipping.
- Verify reset actions work and return expected defaults.

---

## Hard Rules For The Overhaul
- Do not preserve bad layout decisions just because they are already implemented.
- Do not compress important controls until they become unreadable.
- Do not let the mixer crowd out the decks.
- Do not use unclear labels where a better term exists.
- Do not reintroduce decorative seams or split-gloss artifacts on buttons.
- Keep the waveform visually important.
- Preserve working audio behavior while changing the UI.

---

## Screenshot Review: Current UI Errors Confirmed In Runtime

### Error 1: Mixer is visually too narrow and overloaded
Observed from runtime screenshot:
- The center mixer still looks squeezed between the decks.
- Labels are cramped and some are truncated (`BA...`, `TR...`, `G...`).
- Reset buttons compete with the actual audio controls for very limited vertical space.
- The mixer reads like a compressed control strip, not a professional mixer module.

Fix direction:
- Widen the mixer slightly, but reduce visual clutter inside it.
- Stop stacking too many distinct functions in the same top section.
- Move to a simpler vertical rhythm: header, EQ/level body, crossfader footer.
- Prevent any label truncation.

### Error 2: Mixer labels are being clipped or abbreviated
Observed:
- `BASS`, `TREBLE`, and gain labels are visibly cut off.
- The UI currently looks broken rather than dense.

Fix direction:
- Use shorter or better-positioned labels inside the mixer.
- Give each mixer lane enough width or convert label placement.
- If needed, move labels above controls or use side captions rather than forcing them into narrow bottoms.

### Error 3: Reset buttons are useful but currently jammed into the wrong place
Observed:
- `RESET A`, `RESET MIX`, `RESET B` are packed at the top of the mixer.
- They visually dominate a very small center section.
- They reduce room for the actual mixing controls.

Fix direction:
- Keep reset functionality, but relocate it more intelligently.
- Consider one compact defaults row at the bottom or a slimmer utility band.
- The main mixer body should prioritize live mixing controls, not recovery buttons.

### Error 4: Deck bottom filter row is clipped / partially hidden
Observed:
- The bottom-most controls are too close to the deck edge.
- `LOW CUT` / `HIGH CUT` are not fully readable in the screenshot.
- This confirms the deck vertical spacing is still too tight.

Fix direction:
- Reallocate vertical space inside each deck.
- Reduce button height or row count pressure near the bottom.
- Give the deck footer more bottom padding.

### Error 5: Deck control hierarchy is improved but still too busy
Observed:
- The decks are better than before, but there are still many equal-weight buttons.
- The UI does not yet clearly separate primary actions from utility actions.
- Everything still feels a little like stacked controls rather than a designed workflow.

Fix direction:
- Promote transport and waveform as the dominant deck elements.
- De-emphasize less frequently used utility controls.
- Consider grouping utility actions into a smaller/lighter band.

### Error 6: The center section visually interrupts the app instead of anchoring it
Observed:
- The mixer feels like a narrow wall inserted between two decks.
- The whole app still lacks a strong, intentional global composition.

Fix direction:
- Treat the mixer as a clean center spine, not a crowded sidebar.
- Improve proportion balance between deck area and center area.
- Use clearer spacing so the full app reads as one system.

### Error 7: Library section is taking too much performance space
Observed:
- The library/browser area is taller than it needs to be for the current workflow.
- It steals attention from the decks and mixer, which should be the main focus.
- For a DJ-focused performance layout, the live mixing surface should dominate the screen.

Fix direction:
- Reduce the library height so the performance surface gets more vertical room.
- Keep the library usable, but visually secondary.
- Consider a future collapsible or compact browser mode.

---

## Immediate Fix Plan From This Screenshot
- [ ] Remove mixer label truncation completely
- [ ] Rebuild mixer internal spacing so EQ/gain/master are readable
- [ ] Reposition reset controls so they do not crowd the mixer body
- [ ] Increase deck footer breathing room so filter buttons are fully visible
- [ ] Reduce visual busyness of utility controls
- [ ] Reduce library footprint so decks/mixer dominate the layout
- [ ] Rebalance overall proportions again after mixer cleanup
- [ ] Verify HIGH CUT audio behavior matches the label
- [ ] Re-test with a fresh runtime screenshot

---

## Immediate Action Items
- [ ] Redesign mixer structure from scratch
- [ ] Add reset/default buttons for deck playback controls
- [ ] Add reset/default controls for mixer values
- [ ] Rework deck layout into professional control groups
- [ ] Rebalance overall app proportions
- [ ] Re-test with real tracks loaded in both decks
- [ ] Commit the overhaul as its own milestone when stable

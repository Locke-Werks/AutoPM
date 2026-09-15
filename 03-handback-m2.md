# Handback to the PM — AutoPM, Work Package M2 (and beyond)

**From:** Delivery team
**To:** Nyx (project manager and sponsor)
**Date:** 2026-09-15
**Against:** `02-handoff-m2.md`

---

## 1. What was built

The handoff scoped the two Initiating screens. During the session you expanded
scope twice, and both expansions are logged as change requests in the record
rather than absorbed quietly.

**All eleven lifecycle screens**, generated from `definitions/*.pmdef`:

| Phase | Screens |
|---|---|
| Initiating | Project Charter (16 fields) · Stakeholder Register |
| Planning | Requirements · Scope and WBS · Schedule · Risk Register |
| Executing | Sprint Board |
| Monitoring and Controlling | Change Log · Issues and Decisions · Status Reports |
| Closing | Closeout |

**The walkthrough.** After your 14:2x message the product changed shape: it now
teaches rather than files. Each screen opens with a lesson (what the artifact
is, what you should be able to explain afterwards, what has to be true before
you start it), then asks one question per step with the teaching content in
front of the input rather than in a side panel, then closes with a recap of
what is still thin.

**The coach.** Each field declares rules its answers are checked against, and
the app says what is missing: a milestone with no date, a risk with no owner, a
decision log with fewer than three entries. Nothing blocks; you are told and
you decide.

**Lifecycle order is taught, not enforced.** A screen declares what should come
before it. Opening one out of order shows what you are risking and a Next
button.

**Provenance on every entry.** Three states plus untagged, on every field and
every table row, with an evidence line. The overview counts decisions and
unanswered proposals separately.

**The record.** One plain-text file per project under `Documents\AutoPM`, one
line per cell so a diff is readable. Record #1 is this project's own charter,
risks, decisions, WBS, schedule and board, pre-loaded on first run.

**House style.** The Locke Werks material language as a desktop application:
near-black ground, violet-warmed text, hairlines that are never white, one
accent per project spent on emission rather than fill.

**Icon and installer.** A multi-resolution icon drawn rather than downscaled,
and a per-user Forge installer that builds clean.

## 2. Against the M2 acceptance checklist

| Check | Status |
|---|---|
| Installs per-user, Start Menu shortcut, clean uninstall | **Built, not verified.** The installer builds (10.9 MB, unsigned). Nobody has run it. |
| Launches to the project list with record #1 present | Met |
| Charter shows 16 fields, Stakeholder shows register and notes, all from definition files | Met |
| Changing a label or "why" in a definition file changes the app with no code change | Met |
| Focusing any field shows source, why, if-blank, example; `yours` fields visibly labelled | Met |
| Every field and row has a source tag and an evidence line | Met |
| Table rows add and remove; choice columns offer only their listed values | Met |
| Save writes a readable record; unsaved changes shown; warned on close | Met |
| New project creates an empty record | Met |

The PM check — you open record #1, click into every field, and understand what
the panel says — is yours, and is the thing that actually accepts M2.

## 3. Questions for you, not decisions I made

1. **Repo name.** `Locke-Werks/AutoPM`, private. You said "for now", so a rename
   later costs one command. Say the word if you want a different one.
2. ~~**House fonts.**~~ Answered 09-15: fetched and bundled. All three are in
   `assets/fonts` under the SIL Open Font License and ship with the installer.
   `AutoPM.exe --fonts report.txt` confirms the app resolves Chakra Petch,
   Outfit and Instrument Serif rather than falling back.
3. **Signing.** No certificate, so the installer is unsigned and Windows will
   warn on first run. Accepted for a personal install unless you say otherwise.

   **Licence** is settled: all rights reserved (09-15). `LICENSE` also states
   the terms of the two things AutoPM cannot claim, the Qt libraries it links
   (LGPLv3) and the three fonts it bundles (OFL 1.1).
4. **Archon's review timing.** Still unanswered from 09-15 (Q10 in the record).
   It is the only open thing between here and a defensible closeout date.

## 4. What differs from the handoff, and why

- **Scope.** Nine screens beyond the two specified, on your instruction. Logged
  as CR1. No milestone date moved.
- **The product's shape.** The handoff described guided input forms with a help
  panel. Your message about teaching rather than tracking changed that into a
  walkthrough with a coach. Logged as CR5.
- **Repository.** The handoff said no repository until the product was named.
  The name arrived in the same session. Logged as CR2 and CR3.
- **A `--screen` and `--walk` argument** were added so a screen or the
  walkthrough can be opened directly. Not in the spec; they cost nothing and
  made the build verifiable.
- **Code location.** `C:\Users\trist\projects\pm-tool`, alongside the written
  record rather than in a separate folder, so the project and its documentation
  stay together. Say if you want them split.

## 5. How to check it yourself

Open it:

```bash
C:\Users\trist\projects\pm-tool\build\Release\AutoPM.exe
```

Then:

1. The overview leads with **what to do next**. Press **Walk me through it**.
2. Read a lesson page, press Start, and answer two or three questions. Watch
   what the coach says under a thin answer.
3. Leave the walkthrough and open **Project Charter** from the rail. Click into
   every field and read the panel on the right. That is the M2 acceptance check.
4. Open **Sprint Board** and drag a card. Watch the WIP count on Doing.
5. Open the record file the status bar names and read it. It should be
   something Archon can diff.

To prove no field is hard-coded: edit the `why:` block of any field in
`definitions/10-charter.pmdef`, restart the app, and read the panel again.

<div align="center">

<img src="assets/autopm.ico" width="96" alt="AutoPM">

# AutoPM

**Learn the project manager's job by doing it, one question at a time.**

[![licence](https://img.shields.io/badge/licence-all%20rights%20reserved-b76bff?style=flat-square)](LICENSE)
[![platform](https://img.shields.io/badge/platform-Windows%2011-b76bff?style=flat-square)](#requirements)
[![stack](https://img.shields.io/badge/C%2B%2B17-Qt%206.8-b76bff?style=flat-square)](#requirements)

</div>

---

AutoPM walks you through the project management lifecycle and asks you the
questions a PM has to answer, in the order a PM has to answer them. Every
question says where it comes from in the standard, why a PM answers it, what
breaks when nobody does, and what a good answer looks like. Then it reads your
answer back and tells you what is thin about it.

It keeps the record too. That is the side effect, not the point.

## The walkthrough

One question per screen, in lifecycle order: Initiating, Planning, Executing,
Monitoring and Controlling, Closing. Each artifact opens with a lesson (what it
is, what you should be able to explain afterwards, what has to be true before
you start) and closes with a recap of what is still thin.

The order is taught, not enforced. Starting the schedule before the charter is
signed gets you a sentence explaining what you are risking, and a Next button.

## The coach

A record can be complete and still be bad PM work. Milestones with no dates.
Risks with no owner. A decision log where everything is "unobjected". Each
field declares the rules its answers have to meet, and the coach says what is
missing:

    check rows>=3: Fewer than three checkpoints is a start date and a hope.
    check column:owner: A risk with no owner is a worry. Name the person who watches it.

Nothing blocks. Being told and carrying on anyway is a PM decision; the tool's
job is to make sure it was one.

## Provenance

Every field and every table row carries a source tag: **specified** (someone
said it, quote them), **agreed** (they approved your summary), or
**unobjected** (you proposed it and nobody answered). Only the first two count
as decisions. The overview counts them separately, so a project running on
unanswered proposals cannot hide.

## Screens

| Phase | Screens |
|---|---|
| Initiating | Project Charter · Stakeholder Register |
| Planning | Requirements · Scope and WBS · Schedule · Risk Register |
| Executing | Sprint Board |
| Monitoring and Controlling | Change Log · Issues and Decisions · Status Reports |
| Closing | Closeout |

The board drags and drops with WIP limits. The schedule draws a timeline with
milestone diamonds. The WBS draws a tree. Risks and stakeholders draw a
probability-and-impact grid. Every one of them is a view over the same table
the grid edits, one toggle away.

## Nothing is hard-coded

Every screen is generated from a file in `definitions/`. A field's label, type,
allowed values, help text, question and checks all live there. Adding a screen
means adding a file; rewording an explanation means editing a line and
restarting.

```
field milestones
  prompt: What are the dated checkpoints between now and done?
  check rows>=3: Fewer than three checkpoints is a start date and a hope.
  label: Summary milestone schedule
  type: table
  view: timeline
  why: |
    Milestones are dated checkpoints, not tasks. They are what turn
    "in progress" into something that can be on time or late.
```

Records are plain text too, one file per project under
`Documents\AutoPM`, one line per changed cell so a diff is readable.

## Requirements

Windows 11, and for building: CMake 3.21+, Qt 6.8 msvc2022_64, MSVC 2022.

```bash
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="C:/Qt/6.8.3/msvc2022_64"
```

```bash
cmake --build build --config Release
```

Run `windeployqt.exe --release build/Release/AutoPM.exe` once before launching:
a bare Qt executable will not start.

`AutoPM.exe --screen <id>` opens straight onto one screen and
`AutoPM.exe --walk [id]` opens the walkthrough.

## Type

The three house faces ship with the product, in `assets/fonts`: Chakra Petch
for labels, Outfit for body, Instrument Serif for display. They are loaded at
startup, so no installation is needed. `AutoPM.exe --fonts report.txt` writes
out which faces actually resolved, which is how you find out that a machine
quietly fell back to Segoe UI.

## Licence

All rights reserved. See [LICENSE](LICENSE), which also covers the bundled Qt
libraries (LGPLv3) and the three fonts (SIL Open Font License 1.1).

## Source basis

Field lists and process numbers follow the PMBOK Guide, 6th edition, which
still carries the prescriptive artifact contents later editions moved away
from. The sprint board comes from Kanban practice and the Agile Practice Guide
published alongside it. Fields marked **your own** in the app are not from the
standard.

## Project documentation

AutoPM is itself run as a managed project, and its own record is loaded as
record #1 on first run. The written record lives beside the code:

| File | What it is |
|---|---|
| `00-project-record.md` | The running record: decisions with evidence, principles, open questions, risks, changelog |
| `01-charter.md` | The approved charter (v1.0, 2026-09-15) |
| `02-handoff-m2.md` | The work package the first build was made from |
| `03-handback-m2.md` | What was built against it, and what differs |


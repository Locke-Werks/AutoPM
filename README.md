<div align="center">

<img src="assets/autopm.ico" width="96" alt="AutoPM">

# AutoPM

**Learn the project manager's job by doing it, one question at a time.**

[![release](https://img.shields.io/github/v/release/Locke-Werks/AutoPM?style=flat-square&color=B05CF6)](https://github.com/Locke-Werks/AutoPM/releases)
[![licence](https://img.shields.io/badge/licence-all%20rights%20reserved-B05CF6?style=flat-square)](LICENSE)
[![platform](https://img.shields.io/badge/platform-Windows%2011-B05CF6?style=flat-square)](#install)

</div>

---

AutoPM walks you through the project management lifecycle and asks the
questions a PM has to answer, in the order a PM has to answer them. Every
question says where it comes from in the standard, why a PM answers it, what
breaks when nobody does, and what a good answer looks like. Then it reads your
answer back and tells you what is thin about it.

It keeps the record too. That is the side effect, not the point.

It exists because PM work done by intuition does not show you how it maps onto
the job, and because nothing else on the desk held project-level truth.
ProjectMan tracks git state. MemoryBook holds memory and loops. Neither holds
intent: the charter, the scope, the decisions, the risks, and the status
against a plan.

## Where it stands

v0.3.0 is released. AutoPM is also its own first project: it is being run
inside itself, charter through closeout, and its record ships as the example.

- **M1**, charter approved: reached 2026-09-15.
- **M2 to M4**, the Initiating, Planning and Executing screens: built, and
  accepted 2026-09-23. Accepted means the sponsor used each screen on this
  project and understood every field, not that it compiled.
- **S3**, a second real project managed in it: met 2026-09-23. Ten other
  projects had their charters answered and signed off in AutoPM, not imported.
- **Next: M5, closeout**, targeted 2026-10-15. Then one review by the reviewer, whose
  verdict decides whether the product is right.

The last success criterion is that the PM can explain every artifact, and when
it is used, with the tool closed. A study guide you need open during the exam
has failed.

## Install

Download `AutoPM-Setup.exe` from the
[latest release](https://github.com/Locke-Werks/AutoPM/releases/latest) and run
it. It installs for the current user only, into
`%LOCALAPPDATA%\Programs\AutoPM`, with a Start Menu shortcut, and needs no
administrator rights. The installer and every binary in it are signed.

To upgrade, run the newer installer over the old one. Close AutoPM first, and
stop anything running `autopm-mcp.exe`, or the installer cannot replace it.

Uninstall from Settings, Apps. Your records are not in the install directory
and are left alone.

Windows 11, 64-bit.

## Using it

**Projects** are listed down the far left, by name, each with its colour and
its current phase. Click one to switch to it; you stay on the screen you were
on, so the risks of one project and the risks of the next are one click apart.
Ctrl+PageUp and Ctrl+PageDown step through the list. **new project** at the
bottom, or Ctrl+N, starts one.

**The project button** at the top of the rail holds the open project's
settings: its colour, its name, and the records folder.

**The rail** lists the screens in lifecycle order. **The panel on the right**
explains whatever field has focus.

Ctrl+S saves. The title bar shows a dot while there are unsaved changes, and
switching project or closing asks first.

## The walkthrough

One question per screen, in lifecycle order. Each artifact opens with a lesson
(what it is, what you should be able to explain afterwards, what has to be true
before you start) and closes with a recap of what is still thin.

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
| Before the project | Intake and Business Case |
| Initiating | Project Charter · Stakeholder Register |
| Planning | Requirements · Scope and WBS · Quality Management · Schedule · Risk Register |
| Executing | Sprint Planning · Sprint Board · Review and Retrospective |
| Monitoring and Controlling | Change Log · Issues and Decisions · Status Reports |
| Closing | Closeout |

The overview sits above them all. Its four counts are buttons: each opens the
first entry it counted.

### The agile half

The charter picks a hybrid framework: predictive phases, with sprints inside
Executing.

**Sprint Planning** is where cards get committed. Drag a card from the backlog
into the sprint and a bar shows the commitment against the capacity you set.
Going over is shown rather than blocked, because deciding what to drop is the
planning. A velocity strip underneath shows what each closed sprint finished
against what it planned, which is where the next capacity number comes from.
There is one backlog, so a card pulled into a sprint is on the board a moment
later. Pulling in work that does not meet the definition of ready gets a
warning.

**Sprint Board** runs the sprint, with WIP limits from the definition file.

**Review and Retrospective** closes it: what was demonstrated and whether the
sponsor accepted it, then what worked, what did not, and the one thing changing
next sprint.

### The other views

The schedule draws a timeline with milestone diamonds. The WBS draws a tree.
Risks and stakeholders draw a probability and impact grid. Each is a view over
the same table the grid edits, one toggle away.

## The MCP server

`autopm-mcp.exe` installs beside the app. It is the same core with no Qt, so an
agent can read and write records in the conversation where the decisions are
being made, instead of someone retyping them into the tool later.

| Tool | |
|---|---|
| `autopm_projects` | What projects exist and how far through each one is |
| `autopm_create` | Start a new project: name, accent, and the repo whose history documents it |
| `autopm_screens` | The fields a project can hold, and what each is for |
| `autopm_read` | What a record claims, and where each claim came from |
| `autopm_set` | Write a field, with its provenance |
| `autopm_add_row` | Add a decision, risk, change, issue, card or lesson |
| `autopm_update_row` | Move a card, close a risk, answer a change request |
| `autopm_review` | Run the coach: what is blank, what is thin |
| `autopm_git_activity` | What was committed in a window, for evidence |
| `autopm_reconcile` | The board's claims against the commits behind them |

**The provenance rule is enforced.** A write claiming something was
*specified* or *agreed* is refused without evidence:

```
provenance "specified" needs evidence: the quote and date it rests on.
Use "unobjected" if you are proposing it.
```

An agent cannot put words in anybody's mouth through this API.

The window and the server can hold the same record at once. If the server
writes a record while the window has it open, saving in the window asks
whether to reload theirs or overwrite with yours rather than silently
discarding either.

### Git, for evidence

AutoPM reads git for two things a record cannot get any other way: pinning an
entry to a commit so anyone can check it, and holding the board's claims up
against what was actually committed in each sprint window.

```
Sprint 1  2026-09-15 to 2026-09-15
  record: 9 cards done, 35 points
  git:    8 commits in that window
```

It never writes, never fetches, and never moves a card. What a difference means
is the PM's to decide.

### Registering it with Claude Code

At user scope, so it is available in every project:

```bash
claude mcp add --scope user autopm "%LOCALAPPDATA%\Programs\AutoPM\autopm-mcp.exe"
```

Or in `mcpServers` in `~/.claude.json`:

```json
"autopm": {
  "type": "stdio",
  "command": "C:\\Users\\<you>\\AppData\\Local\\Programs\\AutoPM\\autopm-mcp.exe"
}
```

`--records <dir>` and `--definitions <dir>` point it somewhere other than the
defaults.

## Records and definitions

Records are plain text, one `.pmproj` file per project in `Documents\AutoPM`,
one line per changed cell so a diff is readable. On first run AutoPM's own
project record is copied in, so there is a filled-in example to read.

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

## Command line

| Flag | |
|---|---|
| `--project <name>` | Open that project |
| `--screen <id>` | Open on that screen |
| `--walk [id]` | Open the walkthrough, optionally at a screen |
| `--colour` | Open the project's colour picker |
| `--records <dir>` | Read and write records somewhere other than `Documents\AutoPM` |
| `--fonts <file>` | Write which font faces actually resolved, then exit |

They combine:

```bash
AutoPM.exe --project SmallHours --walk charter
```

## What this is not

- **Not an accounting package.** Budget is one field on the charter. Cost
  tracking beyond that is out of scope.
- **Not multi-user.** Each person runs their own install, with their own
  records. There are no permissions, because there is nobody to deny.
- **Not a git tracker.** It reads git for evidence and nothing else. Tracking
  repository state is ProjectMan's job.
- **Not a scheduling engine.** No resource leveling, no Gantt math. The
  schedule draws a timeline of the dates you gave it and does not argue with
  them.

## Building

CMake 3.21 or later, Qt 6.8 `msvc2022_64`, and Visual Studio 2022.

```bash
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="C:/Qt/6.8.3/msvc2022_64"
```

```bash
cmake --build build --config Release
```

```bash
ctest --test-dir build -C Release --output-on-failure
```

Run `windeployqt --release build/Release/AutoPM.exe` once before launching; a
bare Qt executable will not start. The build copies `definitions/` and
`assets/` next to the executables, so the build directory runs exactly like an
install.

Releases are built by `.github/workflows/release.yml` on a `v*` tag. The tag
has to match `version` in `forge/autopm.toml`; the job builds, tests, signs the
binaries, packs them with [Forge](https://github.com/Locke-Werks/Forge), signs
the installer and publishes it.

## Colour and type

The six colour values are borrowed from the Locke Werks house palette because
they are a set that already works on a dark ground. The values only: AutoPM is
not one of the suite apps and does not inherit that palette's rules.

**Status colours are fixed** and mean the same thing in every project:

| | | |
|---|---|---|
| Blue | `#3D7DFF` | Solid. Reached, done, specified. |
| Ember | `#FF5A2A` | Unresolved. Proposed and unanswered, watching. |
| Crimson | `#FF1E3C` | Wrong. Missed, blocked, overdue. |

**The accent is chosen per project**, from all six families, and stored in the
record. It lights the rail, focus, and primary buttons.

Surfaces step by lightness at a fixed hue, hairlines are tinted rather than
grey, and the ink tiers carry their measured contrast against the ground:
18:1, 12.9:1, 8.9:1, 7:1, and a 4.8:1 floor below which nothing carries
meaning.

The three house faces ship in `assets/fonts` and load at startup: Chakra Petch
for labels, Outfit for body, Instrument Serif for display.

## Source basis

Field lists and process numbers follow *Process Groups: A Practice Guide*
(PMI, 2022), which reproduces the 49 processes and their artifact contents.
Those contents came from the PMBOK Guide 6th edition, which PMI retired in
2022: the 7th edition replaced the processes with principles, and the 8th,
published in 2025, reorganizes the same ground into focus areas and performance
domains. Neither carries the prescriptive content lists these fields are built
from, so the practice guide is the citation that can still be checked.

The sprint screens come from Kanban practice and the Agile Practice Guide
published alongside the 6th edition. Fields marked **your own** in the app are
not from the standard.

## Project documentation

AutoPM is run as a managed project, and its own record ships as the first-run
example. The written record lives beside the code:

| File | What it is |
|---|---|
| `00-project-record.md` | Decisions with evidence, principles, open questions, risks, history |
| `01-charter.md` | The approved charter (v1.0, 2026-09-15) |
| `02-handoff-m2.md` | The work package the first build was made from |
| `03-handback-m2.md` | What was built against it, and what differs |
| `04-pm-review.md` | An outside PM review, and what it leaves open |

## Licence

All rights reserved. See [LICENSE](LICENSE), which also covers the bundled Qt
libraries (LGPLv3) and the three fonts (SIL Open Font License 1.1).

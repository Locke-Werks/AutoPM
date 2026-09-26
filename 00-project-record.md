# PM Tool — Project Record

**Status:** Charter approved (M1 reached, 09-15). M2 built and handed back (`03-handback-m2.md`); the PM acceptance check is outstanding. M3 and M4 content delivered early under CR1.
**Opened:** 2026-09-15
**Sponsor / PM:** Nyx
**Product name:** **AutoPM** (D20, 09-15, "for now")
**Repository:** `Locke-Werks/AutoPM`, public (D43)
**Files:** `00-project-record.md` (this file) · `01-charter.md` · `02-handoff-m2.md` · `03-handback-m2.md`

This file is the record for building your own project management software, run as a managed project from day one. Nothing below is a decision unless the Decision Log says so.

## 1. What you said (verbatim, 2026-09-15)

Opening:
> "lets start handing my projects properly. but first lets talk about building my own PM software, so that I can start keeping track of things the way I want to. But lets document doing it right the whole time"

Purpose:
> "I figured building the tool would help me understand the concept of the role better- currently im doing it intuitively, but i want to understand how it would fit into a job. The best way to do that is to build the virtual workflow as an imput field and learn how to use it"

On paper:
> "im not going to go buy a million postits- but I just might use the software"

On product and framework:
> "yes exatly- this project will produce a product that acts as a study guide, practical application, and future orginization. hybrid sounds like the best option for us at the current moment"

On the charter draft:
> "yes to all proposals"

On form factor, schedule and exit:
> "I want this to be personal installable software- like... project man / Milestones- I want to TRY to get this done by October 15th / M1- hopefully today / M2- Hopefully today / M3- tomorrow? / M4- Next week? / Closeout- October 15 / exit criteria approved"

On Archon:
> "Archon may use it, but he will definately review it"
> "he does a review when i think we are done, and he tells me if it is right or not"

Approval and stack:
> "charter approved- go with recommended stack"

On roles:
> "I expected you to doccument it" / "I expected a handoff" / "I am being the PM...."

## 2. Purpose

The product does three jobs: a **study guide**, **practical application**, and **future organization**. Building each part of the PM workflow as an input form makes the role explicit: you already do the work by intuition, and this shows how it fits into a PM job.

## 3. Decision Log

Every entry records where the decision came from, using one of these four sources:
**specified** (you said it; quote it) · **agreed** (you approved a summary) · **unobjected** (proposed, no reply). Only *specified* and *agreed* count as decisions.

| # | Date | Decision | Source | Evidence |
|---|------|----------|--------|----------|
| D1 | 09-15 | Build your own PM software | specified | opening quote |
| D2 | 09-15 | Document the build as it happens | specified | opening quote |
| D3 | 09-15 | Main purpose is learning the role by building its workflow as input forms | specified | purpose quote |
| D4 | 09-15 | Build first; no paper/manual phase | specified | purpose quote + "im not going to go buy a million postits" |
| D5 | 09-15 | **Hybrid framework**: predictive phases, agile sprints inside Executing. Open to revisiting later | specified | "hybrid sounds like the best option for us at the current moment" |
| D6 | 09-15 | Product has three roles: study guide, practical application, future organization | specified | framework quote |
| D7 | 09-15 | Adopt P1 (every field sourced) and P2 (every field explains itself) | agreed | "yes exatly", replying to the study-guide proposal |
| D9 | 09-15 | Personal, installable desktop software, modeled on ProjectMan | specified | "personal installable software- like... project man" |
| D10 | 09-15 | Schedule baseline: M1–M2 Sep 15, M3 Sep 16, M4 week of Sep 21, M5 Oct 15 (targets, with your hedges) | specified | milestones quote |
| D11 | 09-15 | Exit criteria approved (close: M5 + all success criteria; cancel: 30 days unused, or 2+ weeks pulling time from paid PM work) | specified | "exit criteria approved" |
| D12 | 09-15 | Archon is a definite reviewer and a possible user | specified | Archon quote |
| D13 | 09-15 | Archon reviews once, when you think it's done, and tells you whether it's right (final acceptance review) | specified | "he does a review when i think we are done, and he tells me if it is right or not" |
| D14 | 09-15 | **Charter approved** (M1 reached) | specified | "charter approved" |
| D15 | 09-15 | Stack: C++/Qt like ProjectMan, with field definitions in readable data files | agreed | "go with recommended stack" |
| D18 | 09-15 | **How the work is split:** you are the PM. This workstream documents and writes handoffs. The delivery team builds from those handoffs | specified | "I expected you to doccument it" · "I expected a handoff" · "I am being the PM...." |
| D8 | 09-15 | Adopt P3–P5 and every proposed charter entry in v0.1 (business case, success criteria, high-level requirements, out-of-scope list, milestones M1–M5, $0 budget, approval rule, PM authority) | agreed | "yes to all proposals" |
| D19 | 09-15 | **Build the full lifecycle, not just the Initiating screens.** Kanban boards included | specified | "yes I want the forms but I want a full PM tool, Kanban boards, everything" |
| D20 | 09-15 | **Product is named AutoPM**, provisionally. Closes Q7 | specified | "name it AutoPM for now" |
| D21 | 09-15 | Private repository in the Locke-Werks organisation | specified | "make a repo in the lockewerks org, private for now" |
| D22 | 09-15 | The house material language is applied to the application, translated from the web stylesheet rather than copied | specified | "make it pretty, use the house style translated into software instead of a webpage" |
| D23 | 09-15 | **The product teaches the role; it does not just record it.** Guided walkthrough, one question at a time, with coaching on the answers | specified | "I want it to walk me through the steps, not just keep track of it for me- it needs to teach me how to be a pm... not be my pm" |
| D24 | 09-15 | Bundle the three house faces (Chakra Petch, Outfit, Instrument Serif) with the product rather than depending on the machine | specified | "yes get the fonts" |
| D25 | 09-15 | **Licence: all rights reserved.** Not GPLv3, unlike the other Locke Werks repos | specified | "and all rights" |
| D26 | 09-15 | Accent is violet, not pink | specified | "Pink is soooo not the color- lets try... purple" |
| D27 | 09-15 | **Colour values are taken from the house palette families** ("House Palette - What The Colours Do", MindTether2 zip). The values only. AutoPM is not a suite app and is not bound by that sheet's rules, so crimson is not reserved here and there is no obligation to spend hue on a single axis | specified | "make the colors all from the color families listed in mindtether's second style zip" · corrected by "this is not directly mindtether- we are just stealing colors" |
| D28 | 09-15 | **The accent is a per-project choice**, from five families, set from the project menu and stored in the record | specified | "and make it a choice on a project" |
| D34 | 09-15 | **Releases are signed in CI, not locally.** The signing identity exists (Specter Point Intelligence, LLC, via Azure Trusted Signing, and it is what signs the Forge stub) but its credentials are not on this machine. Local builds stay `--dev` and unsigned; a GitHub Actions release job does the signing | agreed | Answers "why unsigned" (2026-09-15) with what was actually found on the machine |
| D32 | 09-15 | **AutoPM gets an MCP front end**, a third face on the same core, so a decision is written into the record where it is made instead of retyped afterwards. The provenance rule is enforced at that API: specified or agreed without evidence is refused | specified | "do the mcp" |
| D33 | 09-15 | **It reads git, for evidence and cross-checking, never for status.** ProjectMan keeps reporting; AutoPM documents. `autopm_reconcile` holds the board's claims up against the commits in each sprint window and reports the difference without changing anything | specified | "I do want it to read from git- just not in the same way Project Man does" · "im just using it to doccument" |
| D30 | 09-15 | **Build out the agile half**: Sprint Planning and Review and Retrospective, alongside the board. Thirteen screens | specified | "add tools like the kanban board, sprint planning" · "add agile" |
| D31 | 09-15 | Sprint planning moves the board's own cards rather than keeping a second list. A field may name another field's rows with `reads:` | agreed | Follows from D30; a sprint that does not contain the actual work is a spreadsheet |
| D29 | 09-15 | **Status colours are fixed across every project**: blue solid, ember unresolved, crimson wrong. The accent moves, these do not, so a colour means the same thing in every record | agreed | Practical, not inherited. An earlier version of this entry claimed the house sheet's one-axis rule applied to AutoPM; it does not |
| D35 | 09-15 | **The repository gets CI**: configure, build and test on a clean Windows runner for every push and pull request, plus a check that the app icon still carries all seven sizes. Before this nothing verified a build except this machine | agreed | "do all the things", pointing at a CI indicator that showed an empty circle because no workflow existed |
| D36 | 09-15 | **The core gets tests, the GUI does not.** The core is the half with no Qt in it and the half where a defect is silent: a record that quietly lost a field looks exactly like one nobody filled in. A GUI defect shows up on screen | agreed | Follows from D35. 231 checks; writing them found a third coach defect |
| D37 | 09-15 | **CI runs on windows-2022, pinned.** `windows-latest` now resolves to windows-2025-vs2026, which has no Visual Studio 2022, and Qt ships this application's binaries as win64_msvc2022_64 | agreed | The first CI run failed at configure with "could not find any instance of Visual Studio" |
| D38 | 09-15 | **Field lists are cited against Process Groups: A Practice Guide (PMI, 2022)**, not the PMBOK Guide 6th ed. The guide reproduces the same 49 processes and artifact contents, so no process number moves and no field list changes; the citation now names something in print | agreed | Archon's PM review, finding 1. The 6th ed. was retired in 2022 and the stated reason for citing it (later editions teach principles, not lists) stopped being true when the 8th reintroduced processes |
| D39 | 09-15 | **A project does not begin at its own charter.** New Intake screen in a phase before Initiating: the request, the needs assessment, options including doing nothing, benefits with measures and owners, and the selection decision. The charter now requires it | agreed | Archon's PM review, finding 2. The business case was filed as a charter field while its own source block said it is a business document and an input to 4.1 |
| D40 | 09-15 | **Success criteria are rows and closeout must answer all of them.** New `covers:` check kind; `reviewField` takes the record so a check can see the field it covers. Closeout rows trace to the criterion they answer | agreed | Archon's PM review, finding 3. Closeout claimed to answer the charter "one line at a time" against a paragraph, and a closeout omitting the two criteria that went badly passed the coach silently |
| D41 | 09-15 | **Quality gets a screen; procurement gets a written exclusion.** Quality was absent and unexcluded, which teaches that the knowledge area does not exist. Procurement is excluded on the record rather than left silent | specified | "Quality screen, procurement excluded" |
| D42 | 09-15 | **The repository was made public and then private again**, a window of about ten minutes on 09-15 for the purpose below, reversing and then restoring D21's "private for now". The purpose is read access for another AI, which can reach a public repository and not a private one. Archon agreed to it, which also covers his review being published. The licence does not change with it: D25 stands, so the source is readable and still all rights reserved. R9 is unchanged and still open: the PM review was explicitly not a security review | specified | "Make it public." · "he said we could go public- it's for my other ai to be able to see it" · "Ok she's seen it- back private" |
| D43 | 09-25 | **The repository is public**, reversing D21 for the second time and, unlike D42, with no return to private planned. The licence does not change: D25 stands, so the source is readable and still all rights reserved. R9 is unchanged and still open | specified | "then make the AutoPM repo Public" |

⚠ Your "yes exatly" answered a message that contained P1–P5. P1 and P2 were what the reply echoed ("study guide"), so only they are logged as agreed. P3–P5 are still proposed.

## 4. Design principles

- P1. ✅ **Every field traces to the standard.** Each input names its PM concept and source. Fields with no source are labeled "your own."
- P2. ✅ **Every field explains itself:** what it is, why a PM fills it in, and what breaks when it's left blank. The help text is the study guide.
- P3. ✅ **Use industry vocabulary** so the concepts carry over to Jira, Asana, MS Project and Smartsheet, and to interviews.
- P4. ✅ **Build order follows the lifecycle:** charter first.
- P5. ✅ **Dogfood:** this project's charter is record #1 (drafted as `01-charter.md`).

## 5. Open questions

- Q1. What breaks today when you track a project? *(Lower priority now.)*
- Q2. Which projects is it for: software, non-software, or both?
- ~~Q3. Who uses it?~~ → D12: you; Archon reviews and may run his own install
- ~~Q4. How does it relate to ProjectMan?~~ → D33: ProjectMan reports git state and starts chats; AutoPM documents intent and reads git only for evidence. The charter's "replacing ProjectMan's git tracking" exclusion stands. MemoryBook and MindTether still open.
- ~~Q5. Build first or paper first?~~ → D4
- ~~Q6. Which framework?~~ → D5
- ~~Q7. Charter gaps~~ → D10–D12. ~~Working title~~ → D20: AutoPM.
- ~~Q9. When does Archon review?~~ → D13: once, at the end
- Q10. Schedule his review after M4 instead of on Oct 15? Share the charter with him up front? (proposed 09-15, **still unanswered**)
- ~~Q15. Fetch the house fonts?~~ → D24: bundled, OFL, loaded at startup
- ~~Q17. Licence?~~ → D25: all rights reserved
- Q16. Keep the code and the written record in one folder, or split them? (built as one; ask)
- ~~Q11. Charter sign-off~~ → D14
- ~~Q8. Architecture~~ → D15
- ~~Q14. How is M2 delivered?~~ → D18: handoff here (`02-handoff-m2.md`); the delivery team builds

## 6. Tools that already exist (read 2026-09-15)

- **ProjectMan** (Archon's): git state and outstanding work across 24 folders; 33 open items on 09-15. Tracks code, not project intent.
- **MemoryBook** (yours): memory, loops, calendar, passive capture.
- **MindTether** (yours): daily continuity.
- **PM experience ledger** (Chronicle `alice-office/career/`): record of past projects.

## 7. Risks

- R1. **You learn the tool you designed instead of the job.** Mitigation: P1. *(This replaces the earlier "building replaces managing" risk, which D3 made much smaller.)*
- R2. **Yet another tracker.** It splits where the truth lives.
- R3. **Decisions attributed without evidence.** Mitigation: the §3 source column.
- R4. **Scope creep.** A full PM suite is huge. Mitigation: P4, plus explicit out-of-scope boundaries in the charter.

- R5. **One late review gate.** With a single review at the end, anything Archon finds wrong shows up at the most expensive moment, and his availability is outside the schedule's control. Mitigation (proposed): review after M4, inside the slack before Oct 15, and give him the charter up front.
- R6. **Aggressive front end.** M2 today needs the architecture decided today. Moving fast works against P2 (you understanding every field).

- R7. **The record looks finished before the learning has happened.** Record #1 was pre-loaded with this project's charter, risks, decisions and board, which makes the tool look 93% complete on first run. That is a demonstration, not your work. Mitigation: the walkthrough and the coach exist precisely so completeness is not the measure; success criterion 4 (explaining each artifact without opening the tool) is the real test.
- R9. **No security review has ever been done.** The block file parser reads records, the hand-written JSON parser reads whatever an MCP client sends, and the git module builds a shell command from a stored path; the MCP has write access to every record. All three surfaces were written in one session by the party that would be reviewing them. Mitigation: a review before the tool is used anywhere but this machine. **Not started**, tracked as card C16 and issue I4.

- R8. **Unsigned installer.** The certificate exists but its Azure Trusted Signing credentials are not on this machine, so local builds are unsigned and Windows warns on first run. Mitigation (D34): `.github/workflows/release.yml` now does the whole signed path on a `v*` tag, following Forge's `docs/using-forge-in-ci.md`. Everything but the two Azure signing steps was run locally against the real config. **Still blocked**, on three values that have to be set on this repository: `AZURE_TENANT_ID` and `AZURE_CLIENT_ID` as repository variables, `AZURE_CLIENT_SECRET` as a secret on the `release` environment. Until they exist the job fails at the first signing step, and the installer states its unsigned state on its first page rather than leaving it to be discovered.

## 8. Planned artifacts (these double as the tool's screens)

Charter → stakeholder register → requirements + acceptance criteria → scope / WBS → schedule → risk register → change log → status reports → closeout + lessons learned.

## 9. Architecture reference: ProjectMan (read from GitHub 2026-09-15)

`Locke-Werks/ProjectMan`: C++ with CMake; a Qt 6 Widgets GUI; a deliberately **Qt-free `src/core`** with separate `cli`, `gui` and `mcp` front ends; Forge installers (`installer.toml`, `installer-mcp.toml`). Its "one core, several faces" shape is worth copying whatever language this project uses.

## 10. Changelog

- 09-15 · Record opened (D1–D2)
- 09-15 · Purpose reframed to learning; build-first (D3–D4); P1–P5 proposed; R1 revised
- 09-15 · Hybrid chosen (D5); three product roles (D6); P1–P2 agreed (D7); charter v0.1 drafted
- 09-15 · All proposals agreed (D8); charter v0.2; exit-criteria candidates drafted
- 09-15 · Form factor, schedule, exit criteria, Archon's role (D9–D12); charter v0.3 ready for sign-off; ProjectMan architecture read
- 09-15 · Archon's review defined as a single final gate (D13); R5 revised; review timing proposed (Q10)
- 09-15 · Charter approved (D14); stack chosen (D15)
- 09-15 · Roles set (D18); M2 handoff written for the delivery team (`02-handoff-m2.md`)
- 09-15 · Scope expanded to the full lifecycle (D19, CR1); named AutoPM (D20, CR2); private repo created (D21, CR3); house style applied (D22, CR4)
- 09-15 · **Product reframed from recorder to teacher** (D23, CR5): guided walkthrough, per-field coaching, lifecycle order taught rather than enforced
- 09-15 · M2 built and handed back (`03-handback-m2.md`). Eleven screens, installer builds unsigned. PM acceptance check outstanding; R7 and R8 opened
- 09-15 · House faces bundled (D24) and licence set to all rights reserved (D25). Installer rebuilt at 11.2 MB
- 09-15 · Accent changed from pink to violet (D26). Icon, seed record, new-project default and README badges all follow it
- 09-15 · Whole palette rebuilt on the house family values (D27), status colours fixed (D29), accent made a per-project choice (D28). Two layout bugs fixed on the way: wrapped text was measured at one width and drawn at another
- 09-15 · Defect found by the sponsor and fixed: the overview picked what to do next by taking the first incomplete screen regardless of phase, so it proposed Closeout on a project still in Executing. It now only proposes screens in a phase the project has reached. The empty Closing screens were briefly listed in the review request as a gap; they are the correct state and the description was corrected
- 09-15 · **Installer run and verified on this machine.** Per-user install to `%LOCALAPPDATA%\Programs\AutoPM`, 48 files, Start Menu shortcut, uninstall entry registered, both the app and the MCP launch from the installed copy. The last outstanding M2 acceptance item, except uninstall itself, which has not been exercised because doing so removes the install
- 09-15 · Installer gained the licence page and the application's palette (R8 revised: the unsigned warning is now stated on the first page rather than discovered)
- 09-15 · MCP front end built (D32) with git reading for evidence and reconciliation (D33). Three faces on one core now: gui, mcp, and the core itself
- 09-15 · Agile half built (D30): Sprint Planning with capacity, velocity and a refinement warning; Review and Retrospective. Cards gained a `ready` column and a view can now read another field's rows (D31)
- 09-15 · **Correction.** The first pass imported the house sheet's rules along with its colours: crimson treated as reserved, a single-axis obligation, a mono-label invariant raised as an open question. AutoPM is not a suite app and none of that binds it. Crimson restored as both a status colour and an accent choice; the doctrine removed from the code, the dialog and this record
- 09-15 · **Taskbar icon fixed.** The `.ico` was drawn on an opaque tile, so at 16 and 24 pixels it read as a black square rather than a mark; it is drawn on transparency now and fills more of the frame. What kept it looking broken after that was a stale shell icon cache: the same running process showed the placeholder at 6:28 and the mark at 6:33 with nothing about it changed. `SetCurrentProcessExplicitAppUserModelID` was added while chasing this and removed again, since it never moved the fault and ProjectMan resolves its icon without one
- 09-15 · **CI exists** (D35, D37). Build and test on every push and pull request, plus a check that the icon still carries all seven sizes. The first run failed because `windows-latest` has moved to Visual Studio 2026
- 09-15 · **Core tests written** (D36). 231 checks over the block format, provenance, record round trips, the shipped definition files and the two coach defects already fixed. They found a third: `reviewScreen` listed every blank field as a note while `progressOf` skipped them, so the walkthrough's end-of-screen summary said "2 of 14 answered" and then printed twelve complaints about the twelve nobody had reached. The MCP's review tool already drew that line correctly; `reviewScreen` does now too
- 09-15 · **Signed release job written** (D34, R8 revised). Build, sign payload, forge, sign installer, verify, publish, on a `v*` tag. Validated locally end to end apart from the two Azure steps: the forged installer came out 11,883,408 bytes against the 11,871,712 of the one that shipped, and read its own container back. Blocked on three credential values being set on the repository
- 09-15 · Two defects found while validating the release path. The payload was carrying `opengl32sw.dll` and the D3D compiler, 24 MB that the working install has never had, and `assets/` is copied wholesale into the payload so the icon build scripts were shipping to end users. Scripts moved to `tools/`, deployment narrowed
- 09-15 · Uninstall still not exercised. The manifest shows its scope is 47 files inside the install directory plus one Start Menu shortcut, and names no path under Documents, so records are safe by construction. Running it was stopped short because the open window had unsaved changes in SmallHours
- 09-15 · **PM-domain review received from Archon** (`AutoPM-pm-review.md`, against commit cc975e4): 13 findings, a coverage table against the ten knowledge areas, and a list of what to leave alone. It passes M2 §8 on every line and passes the two charter success criteria that can be judged now. Every factual claim spot-checked against the files before anything was changed, and all of them held
- 09-15 · Four small findings fixed (7, 8, 9, 10): the issue log's help text described an escalation column that did not exist, the Issues screen showed an Executing process number under a Monitoring and Controlling phase, a risk had a planned response and nothing saying when to fire it, and the change log recorded when a change arrived but not when it was decided
- 09-15 · Citations moved to Process Groups: A Practice Guide (D38). Nothing renumbered
- 09-15 · **Intake screen added** (D39), and closeout gained a benefits field to match. The tool now holds the half of the lifecycle that decides whether a project should exist, not just the half after it is authorized
- 09-15 · Success criteria became rows and closeout became traceable to them (D40), through a new `covers:` check. Record #1 migrated by hand: the old paragraph is deleted rather than left beside the rows, which is the one way that change breaks quietly, and a test now loads the seeded record and checks both
- 09-15 · Quality screen added and procurement excluded on the record (D41). Eight of the ten knowledge areas now have an artifact; resource management (finding 4) and the project management plan baselines (finding 13) are the two still open
- 09-15 · **Repository made public** (D42). Scanned first: no credentials, no email, and the Actions workflows reference secret names rather than values. What went public with it is the record itself, which quotes the sponsor verbatim 114 times, names four other projects, and carries Archon's review, which he agreed to. All of it hers, and all of it already in the history, which a later change to visibility cannot take back. The reason for going public is read access for another AI rather than distribution, and the all-rights-reserved licence still says so
- 09-15 · **Uninstall exercised, the last M2 acceptance criterion.** Full cycle on this machine: saved the open work, closed the app, uninstalled, reinstalled from a freshly forged installer carrying the day's fixes. All three records came through byte for byte identical, which is the criterion that mattered. The Start Menu shortcut and the uninstall entry were both removed and both restored
- 09-15 · **Defect found by that test: uninstall leaves a running payload binary behind and says nothing.** `autopm-mcp.exe` was live as the registered MCP server, so the uninstaller could not delete it, removed the other 47 files, dropped the shortcut and the ARP entry, and reported success. The install directory was left holding one locked executable with nothing in the product saying so. Deleting it succeeded the moment the process was stopped, which confirms the lock was the only obstacle. An uninstall that cannot finish should say which files it could not remove and why
- 09-16 · **That defect is fixed in Forge**, merged as Locke-Werks/Forge#3. `delete_tree_entry` was falling back to a delayed-rename queue that lives under HKLM, which an unelevated per-user uninstall cannot write to, and it discarded the result and returned success either way. It now reports what happened and the uninstaller names every file it could not remove, in the message box interactively and as an `lwi: warning:` line under `/S`. Testing it turned up a second problem in the fix itself: the uninstaller's own image is always locked at that point and was being reported, which would have put a warning on every uninstall that ever runs. **It does not reach AutoPM yet.** The installer takes `lwstub.exe` from a Forge release and the release workflow pins v0.4.0, so this arrives when Forge cuts its next release and the pin moves
- 09-16 · **Forge v0.4.1 released and the pin moved.** Signed on the runner, both assets verified against the Specter Point certificate, and the shipped stub checked rather than assumed: a throwaway package forged with the released `lwforge.exe` and `lwstub.exe`, installed, a payload file held open, and the uninstaller named it. `release.yml` now pins v0.4.1, so the next AutoPM release embeds an uninstaller that reports what it could not remove. The installer sitting in `dist/` was forged against v0.4.0 and does not have it
- 09-17 · The overview's four stat tiles became controls, on the sponsor's reading that a number you cannot act on is decoration. Each opens the first entry it counted and scrolls to it, so "14 proposed, unanswered" lands on one of the fourteen rather than at the top of a page holding one. The completion tile is the exception and opens the next screen worth working on, because there is no single blank entry to point at and because the first screen with a blank field is how the overview used to propose Closeout on a running project. Cards are focusable and answer Enter or Space, not just the mouse
- 09-15 · Repository returned to private (D42). The public window was about ten minutes and served its purpose; anything fetched or indexed in it is still out, which is why the decision records the window rather than pretending it did not happen
- 09-25 · **v0.1.0 released**, the first release. The release job had never run: AutoPM carried the signing secret but not the tenant and client IDs, so they were copied from Forge's repository variables. Tagged at the stat-tiles commit; the far-left project list in the working tree is not in it. Payload and installer signed on the runner, uninstaller from Forge v0.4.1
- 09-25 · **Repository made public** (D43), after the release

## 11. Open from the PM review

Findings from `AutoPM-pm-review.md` not acted on yet, in the reviewer's own
order of priority. These should become cards on the board rather than living
only here.

- **Finding 4. Resource management has no artifact.** Knowledge area 9 has no
  screen. Ownership is recorded once per row in five places, all `type: line`,
  so nothing consolidates them and nothing separates who does the work from who
  answers for it. A roster and a responsibility assignment would fix it. The
  charter's exclusion covers resource *levelling*, which is optimization, not
  resource management, so this is absent rather than excluded.
- **Finding 13. Nothing names what is baselined.** The subsidiary plans are
  distributed to the screens that use them, which is better teaching than one
  "Project Management Plan" form. What is missing is the sentence saying which
  artifacts are baselined and as of when, and therefore which can only change
  through the change log. The tool currently teaches that distinction only for
  the schedule.
- **Finding 6. Status reports have no communications plan behind them.** 10.2 is
  cited, 10.1 is absent, so nothing says who receives a report or how often. Two
  columns on the stakeholder register would carry it.
- **Finding 7, second half. Reserves appear only inside an example.** Contingency
  reserve against identified risks and management reserve against the unknown is
  one of the distinctions the exam tests hardest, and the tool mentions it as
  sample text. A `fallback` column beside `response` is worth considering at the
  same time: the response is what you do to stop it, the fallback is what you do
  when the response did not work.
- **Finding 11. The assumption log is a charter output filed under Planning.**
  Its own `source:` says it is a 4.1 output, and it sits on a screen gated behind
  `requires: charter`. The cheapest fix is an assumptions field on the charter,
  with the requirements one kept as elaboration.
- **Closing.** 4.7 is Close Project *or Phase* and the tool only closes a
  project; a phase-closure table would make the second half real and would mean
  M1 to M4 get closed rather than passed. The final report is a 4.7 output with
  no field. Resource release belongs here and depends on finding 4.
- **`view: matrix` fails silently.** `MatrixView::paintEvent` returns early when
  either axis column is not a `choice` with declared options, drawing nothing,
  with no message and no fallback to the grid. Since the product's claim is that
  adding a screen means adding a file, this is the one place a plausible file
  produces a blank rectangle. Either document it where somebody will copy from,
  or fall back to the table.

Not in this list: the security review, which is R9 and card C16 and belongs to
Archon. The review says explicitly that it does not touch it.

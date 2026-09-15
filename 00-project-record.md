# PM Tool — Project Record

**Status:** Charter approved (M1 reached, 09-15). M2 built and handed back (`03-handback-m2.md`); the PM acceptance check is outstanding. M3 and M4 content delivered early under CR1.
**Opened:** 2026-09-15
**Sponsor / PM:** Nyx
**Product name:** **AutoPM** (D20, 09-15, "for now")
**Repository:** `Locke-Werks/AutoPM`, private (D21)
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
| D27 | 09-15 | **Every colour comes from the house palette families** (MindTether2 zip, "House Palette - What The Colours Do"). Ground, surfaces, hairlines and ink tiers follow the documented invariants. Crimson `#FF1E3C` is reserved family-wide and is not used anywhere in AutoPM, including as a project accent | specified | "make the colors all from the color families listed in mindtether's second style zip" |
| D28 | 09-15 | **The accent is a per-project choice**, from five families, set from the project menu and stored in the record | specified | "and make it a choice on a project" |
| D29 | 09-15 | **AutoPM's one axis for hue is how well-founded a claim is.** Blue solid, violet agreed, magenta unresolved, ember broken. Ember is taken as a signal, which the house sheet says is a decision to make explicitly rather than by drift | agreed | Follows from D27; the sheet requires each app to name its one axis |

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
- Q4. How does it relate to ProjectMan, MemoryBook and MindTether? (See §6.)
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
- R8. **Unsigned installer.** No signing certificate, so Windows warns on first run and Archon will see that warning too. Accepted for a personal install; revisit before any distribution.

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
- 09-15 · Whole palette rebuilt on the house families (D27), one hue axis named (D29), accent made a per-project choice (D28). Two layout bugs fixed on the way: wrapped text was measured at one width and drawn at another

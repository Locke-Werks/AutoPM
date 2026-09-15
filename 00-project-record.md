# PM Tool — Project Record

**Status:** Charter approved (M1 reached, 09-15). M2 not reached; the M2 handoff is ready (`02-handoff-m2.md`).
**Opened:** 2026-09-15
**Sponsor / PM:** Nyx
**Working name:** none yet
**Files:** `00-project-record.md` (this file) · `01-charter.md` · `02-handoff-m2.md`

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
- ~~Q7. Charter gaps~~ → D10–D12. Working title still open (doesn't block).
- ~~Q9. When does Archon review?~~ → D13: once, at the end
- Q10. Schedule his review after M4 instead of on Oct 15? Share the charter with him up front? (proposed)
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
